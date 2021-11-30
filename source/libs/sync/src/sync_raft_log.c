/*
 * Copyright (c) 2019 TAOS Data, Inc. <cli@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "syncInt.h"
#include "sync_raft_log.h"
#include "sync_raft_proto.h"
#include "sync_raft_stable_log.h"
#include "sync_raft_unstable_log.h"

static void visitNumOfPendingConf(const SSyncRaftEntry* entry, void* arg);
static SyncIndex findConflict(const SSyncRaftLog*, const SSyncRaftEntry*, int n);
static int mustCheckOutOfBounds(const SSyncRaftLog*, SyncIndex lo, SyncIndex hi);
static SyncTerm zeroTermOnErrCompacted(const SSyncRaftLog*, SyncTerm, ESyncRaftCode err);

struct SSyncRaftLog {
  // owner Raft
  const SSyncRaft* pRaft;

  // storage contains all stable entries since the last snapshot.
  SSyncRaftStableLog* storage;

	// unstable contains all unstable entries and snapshot.
	// they will be saved into storage.
  SSyncRaftUnstableLog* unstable;

	// committed is the highest log position that is known to be in
	// stable storage on a quorum of nodes.
  SyncIndex committedIndex;

	// applied is the highest log position that the application has
	// been instructed to apply to its state machine.
	// Invariant: applied <= committed
  SyncIndex appliedIndex;

	// maxNextEntsSize is the maximum number aggregate byte size of the messages
	// returned from calls to nextEnts.
  uint64_t maxNextEntsSize;
};

SSyncRaftLog* syncCreateRaftLog(SSyncRaftStableLog* storage, uint64_t maxNextEntsSize, const SSyncRaft* pRaft) {
  SSyncRaftLog* log = (SSyncRaftLog*)malloc(sizeof(SSyncRaftLog));
  if (log == NULL) {
    return NULL;
  }

  log->storage = storage;
  log->maxNextEntsSize = maxNextEntsSize;

  SyncIndex firstIndex = syncRaftStableLogFirstIndex(storage);
  SyncIndex lastIndex = syncRaftStableLogLastIndex(storage);

  SSyncRaftUnstableLog* unstable = syncRaftCreateUnstableLog(pRaft, lastIndex);
  if (unstable == NULL) {
    free(log);
    return NULL;
  }
  log->unstable = unstable;

  // Initialize our committed and applied pointers to the time of the last compaction.
  log->committedIndex = firstIndex - 1;
  log->committedIndex = firstIndex - 1;

  log->pRaft = pRaft;
  return log;
}

// maybeAppend returns (0, false) if the entries cannot be appended. Otherwise,
// it returns (last index of new entries, true).
bool syncRaftLogMaybeAppend(SSyncRaftLog* log, SyncIndex index, SyncTerm logTerm, 
                            SyncIndex committedIndex, const SSyncRaftEntry* entries, int n, SyncIndex* lastNewIndex) {
  if (!syncRaftLogMatchTerm(log, index, logTerm)) {
    return false;
  }

  *lastNewIndex = index + n;
  SyncIndex ci = findConflict(log, entries, n);
  if (ci <= log->committedIndex) {
    syncFatal("[%d:%d]entry %" PRId64 " conflict with committed entry [committed(%" PRId64 ")]", 
      log->pRaft->selfGroupId, log->pRaft->selfId, ci, log->committedIndex);
    return false;
  }

  if (ci == 0) {
    syncRaftLogCommitTo(log, MIN(committedIndex, *lastNewIndex));
    return true;
  }
  SyncIndex offset = index + 1;
  syncRaftLogAppend(log, &entries[ci-offset], n - (ci - offset));
  syncRaftLogCommitTo(log, MIN(committedIndex, *lastNewIndex));
  return true;
}

SyncIndex syncRaftLogAppend(SSyncRaftLog* log, const SSyncRaftEntry* entries, int n) {
  if (n == 0) {
    return syncRaftLogLastIndex(log);
  }
  SyncIndex afterIndex = entries[0].index - 1;
  if (afterIndex < log->committedIndex) {
    return 0;
  }
  syncRaftUnstableLogTruncateAndAppend(log->unstable, entries, n);
  return syncRaftLogLastIndex(log);
}

bool syncRaftLogMatchTerm(SSyncRaftLog* log, SyncIndex index, SyncTerm logTerm) {
  ESyncRaftCode err;
  SyncTerm t = syncRaftLogTermOf(log, index, &err);
  if (err != RAFT_OK) {
    return false;
  }

  return t == logTerm;
}

SyncTerm syncRaftLogTermOf(SSyncRaftLog* log, SyncIndex index, ESyncRaftCode* errCode) {
  // the valid term range is [index of dummy entry, last index]
  SyncIndex dummyIndex = syncRaftLogFirstIndex(log) - 1;
  SyncIndex lastIndex = syncRaftLogLastIndex(log);
  if (errCode) *errCode = RAFT_OK;
  if (index < dummyIndex || index > lastIndex) {
    return 0;
  }

  SyncTerm term;
  if (syncRaftUnstableLogMaybeTerm(log->unstable, index, &term)) {
    return term;
  }

  term = syncRaftStableLogTerm(log->storage, index, errCode);
  if (*errCode == RAFT_OK) {
    return term;
  }

  if (*errCode == RAFT_COMPACTED || *errCode == RAFT_UNAVAILABLE) {
    return SYNC_NON_TERM;
  }

  SSyncRaft* pRaft = log->pRaft;
  syncFatal("[%d:%d] syncRaftLogTermOf fatal", pRaft->selfGroupId, pRaft->selfId);
}

int syncRaftLogEntries(SSyncRaftLog* log, SyncIndex index, SSyncRaftEntry **ppEntries, int *n) {
  SyncIndex lastIndex = syncRaftLogLastIndex(log);
  if (index > lastIndex) {
    *n = 0;
    return RAFT_OK;
  }

  return syncRaftLogSlice(log, index, lastIndex + 1, ppEntries, n);
}

bool syncRaftMaybeCommit(SSyncRaftLog* log, SyncIndex maxIndex, SyncTerm term) {

  if (maxIndex > log->committedIndex) {
    ESyncRaftCode err;
    SyncTerm t = syncRaftLogTermOf(log, maxIndex, &err);
    if (zeroTermOnErrCompacted(log, t, &err) == term) {
      syncRaftLogCommitTo(log, maxIndex);
      return true;
    }
  }

  return false;
}

SyncIndex syncRaftLogFirstIndex(const SSyncRaftLog* log) {
  SyncIndex firstIndex;
  if (syncRaftUnstableLogMaybeFirstIndex(log->unstable, &firstIndex)) {
    return firstIndex;
  }
  return syncRaftStableLogFirstIndex(log->storage);
}

SyncIndex syncRaftLogLastIndex(const SSyncRaftLog* log) {
  SyncIndex lastIndex;
  if (syncRaftUnstableLogMaybeLastIndex(log->unstable, &lastIndex)) {
    return lastIndex;
  }
  return syncRaftStableLogLastIndex(log->storage);
}

bool syncRaftLogCommitTo(SSyncRaftLog* log, SyncIndex toCommit) {
  // never decrease commit
  if (log->committedIndex < toCommit) {
    SyncIndex lastIndex = syncRaftLogLastIndex(log);
    if (lastIndex < toCommit) {
      syncFatal("[%d:%d]entry %" PRId64 " conflict with committed entry [committed(%"PRId64")]",
        log->pRaft->selfGroupId, log->pRaft->selfId, ci, log->committedIndex);
    }
    log->committedIndex = toCommit;
  }
}

// findConflictByTerm takes an (index, term) pair (indicating a conflicting log
// entry on a leader/follower during an append) and finds the largest index in
// log l with a term <= `term` and an index <= `index`. If no such index exists
// in the log, the log's first index is returned.
//
// The index provided MUST be equal to or less than l.lastIndex(). Invalid
// inputs log a warning and the input index is returned.
SyncIndex syncRaftLogFindConflictByTerm(const SSyncRaftLog* log, SyncIndex index, SyncTerm term) {
  SyncIndex lastIndex = syncRaftLogLastIndex(log);
  if (index > lastIndex) {
		// NB: such calls should not exist, but since there is a straightfoward
		// way to recover, do it.
		//
		// It is tempting to also check something about the first index, but
		// there is odd behavior with peers that have no log, in which case
		// lastIndex will return zero and firstIndex will return one, which
		// leads to calls with an index of zero into this method.
    SSyncRaft* pRaft = log->pRaft;
    syncWarn("[%d:%d]index(%"PRId64") is out of range [0, lastIndex(%"PRId64")] in findConflictByTerm",
      pRaft->selfGroupId, pRaft->selfId, index, lastIndex);
    return index;
  }

  while (true) {
    ESyncRaftCode err = RAFT_OK;
    SyncTerm logTerm = syncRaftLogTermOf(log, index, &err);
    if (logTerm <= term || err != RAFT_OK) {
      return index;
    }
    index -= 1;
  }

  return index;
}

SyncTerm syncRaftLogLastTerm(const SSyncRaftLog* log) {
  ESyncRaftCode err = RAFT_OK;
  SyncIndex lastIndex = syncRaftLogLastIndex(log);
  SyncTerm logTerm = syncRaftLogTermOf(log, lastIndex, &err);
  if (err != RAFT_OK) {
    syncFatal("[%d:%d]unexpected error when getting the last term %s", gSyncRaftCodeString[err]);
  }
  return logTerm;
}

void syncRaftLogAppliedTo(SSyncRaftLog* log, SyncIndex index) {
  if (index == 0) {
    return;
  }

  if (log->committedIndex < index || index < log->appliedIndex) {
    syncFatal("[%d:%d]applied(%" PRId64 ") is out of range [prevApplied(%d" PRId64 "), committed(%" PRId64 ")]",
      pRaft->selfGroupId, pRaft->selfId, index, log->appliedIndex, log->committedIndex);
  }
  log->appliedIndex = index;
}

void syncRaftLogStableTo(SSyncRaftLog* log, SyncIndex i, SyncTerm t) {
  syncRaftUnstableLogStableTo(log->unstable, i, t);
}

// isUpToDate determines if the given (lastIndex,term) log is more up-to-date
// by comparing the index and term of the last entries in the existing logs.
// If the logs have last entries with different terms, then the log with the
// later term is more up-to-date. If the logs end with the same term, then
// whichever log has the larger lastIndex is more up-to-date. If the logs are
// the same, the given log is up-to-date.
bool syncRaftLogIsUptodate(const SSyncRaftLog* log, SyncIndex index, SyncTerm term) {
  SyncIndex lastIndex = syncRaftLogLastIndex(log);
  SyncTerm  lastTerm  = syncRaftLogLastTerm(log);

  return term > lastTerm || (term == lastTerm && index >= lastIndex);
}

bool syncRaftHasUnappliedLog(const SSyncRaftLog* log) {
  return log->committedIndex > log->appliedIndex;
}

// slice returns a slice of log entries from lo through hi-1, inclusive.
int syncRaftLogSlice(SSyncRaftLog* log, SyncIndex lo, SyncIndex hi, SSyncRaftEntry** ppEntries, int* n) {
  *n = 0;
  if (lo == hi) {    
    return RAFT_OK;
  }

  int err = mustCheckOutOfBounds(log, lo, hi);
  if (err != RAFT_OK) {
    return err;
  }

  SSyncRaft* pRaft = log->pRaft;
  SSyncRaftEntry *stableEnts = NULL, *unstableEnts = NULL, *pRet = NULL;
  int nStableEnts = 0, nUnstableEnts = 0, nRet = 0;

  SyncIndex unstableOffset = syncRaftUnstableLogOffset(log->unstable);
  if (lo < unstableOffset) {
    SyncIndex newHi = MIN(hi, unstableOffset);
    err = syncRaftStableEntries(log->storage, lo, newHi, &stableEnts, &nStableEnts);
    if (err == RAFT_COMPACTED) {
      return err;
    } else if (err == RAFT_UNAVAILABLE) {
      syncFatal("[%d:%d]entries[%"PRId64":%"PRId64") is unavailable from storage",
        pRaft->selfGroupId, pRaft->selfId, lo, newHi);
    } else {
      syncFatal("[%d:%d]syncRaftLogSlice fail:%s",
        pRaft->selfGroupId, pRaft->selfId, gSyncRaftCodeString[err]);
    }

    // check if ents has reached the size limitation
    if (nStableEnts < newHi) {
      *ppEntries = stableEnts;
      *n = nStableEnts;
      return RAFT_OK;
    }
    pRet = stableEnts;
    nRet = nStableEnts;
  }

  if (hi > unstableOffset) {
    SyncIndex newLo = MAX(lo, unstableOffset);
    syncRaftUnstableLogSlice(log->unstable, newLo, hi, &unstableEnts, &nUnstableEnts);

    if (nRet > 0) {
      SSyncRaftEntry* combined = (SSyncRaftEntry*)malloc((nRet + nUnstableEnts) * sizeof(SSyncRaftEntry));
      if (combined == NULL) {
        return RAFT_NO_MEM;
      }
      memcpy(combined, pRet, sizeof(SSyncRaftEntry) * nRet);
      memcpy(&combined[nRet], unstableEnts, sizeof(SSyncRaftEntry) * nUnstableEnts);

      pRet = combined;
      nRet = nRet + unstableEnts;
      if (stableEnts) free(stableEnts);
      if (unstableEnts) free(unstableEnts);
    } else {
      pRet = unstableEnts;
      nRet = nUnstableEnts;
    }
  }

  *ppEntries = pRet;
  *n = nRet;
  return RAFT_OK;
}

// return number of pending config entries
int syncRaftLogNumOfPendingConf(SSyncRaftLog* log) {
  SyncIndex lo = log->appliedIndex + 1;
  SyncIndex hi = log->committedIndex + 1;
  if (lo == hi) {
    return 0;
  }

  SyncIndex unstableOffset = syncRaftUnstableLogOffset(log->unstable);
  int n = 0;
  if (lo < unstableOffset) {
    syncRaftStableLogVisit(log->storage, lo, MIN(hi, unstableOffset), visitNumOfPendingConf, &n);
  }
  if (hi > unstableOffset) {
    syncRaftUnstableLogVisit(log->unstable, MAX(lo, unstableOffset), hi, visitNumOfPendingConf, &n);
  }
  
  return n;
}

bool syncRaftLogIsCommitted(SSyncRaftLog* log, SyncIndex index) {
  return log->committedIndex <= index;
}

static void visitNumOfPendingConf(const SSyncRaftEntry* entry, void* arg) {
  int* n = (int*)arg;
  if (syncRaftIsConfEntry(entry)) *n = *n + 1;
}

// findConflict finds the index of the conflict.
// It returns the first pair of conflicting entries between the existing
// entries and the given entries, if there are any.
// If there is no conflicting entries, and the existing entries contains
// all the given entries, zero will be returned.
// If there is no conflicting entries, but the given entries contains new
// entries, the index of the first new entry will be returned.
// An entry is considered to be conflicting if it has the same index but
// a different term.
// The index of the given entries MUST be continuously increasing.
static SyncIndex findConflict(const SSyncRaftLog* log, const SSyncRaftEntry* entries, int n) {
  int i;
  SyncIndex lastIndex = syncRaftLogLastIndex(log);
  SSyncRaft* pRaft = log->pRaft;
  for (i = 0; i < n; ++i) {
    const SSyncRaftEntry* entry = &entries[i];
    if (!syncRaftLogMatchTerm(log, entry->index, entry->term)) {
      if (entry->index <= lastIndex) {
        syncFatal("[%d:%d]found conflict at index %" PRId64 " [existing term: %" PRId64 ", conflicting term: %" PRId64 "]",
          pRaft->selfGroupId, pRaft->selfId, entry->index,
          zeroTermOnErrCompacted(log, syncRaftLogTermOf(log, entry->index)),
          entry->term);
      }
      return entry->index;
    }
  }

  return 0;
}

// l.firstIndex <= lo <= hi <= l.firstIndex + len(l.entries)
static int mustCheckOutOfBounds(const SSyncRaftLog* log, SyncIndex lo, SyncIndex hi) {
  SSyncRaft* pRaft = log->pRaft;

  if (lo > hi) {
    syncFatal("[%d:%d]invalid slice %" PRId64 " > %" PRId64 " ",
      pRaft->selfGroupId, pRaft->selfId, lo, hi);
  }

  SyncIndex firstIndex = syncRaftLogFirstIndex(log);
  if (lo < firstIndex) {
    return RAFT_COMPACTED;
  }
  SyncIndex lastIndex = syncRaftLogLastIndex(log);
  int length = (int)(lastIndex + 1 - firstIndex);
  if (hi > firstIndex + length) {
    syncFatal("[%d:%d]slice[%"PRId64",%"PRId64") out of bound [%"PRId64",%"PRId64"]",
      pRaft->selfGroupId, pRaft->selfId, lo, hi, firstIndex, lastIndex);
  }

  return RAFT_OK;
}

static SyncTerm zeroTermOnErrCompacted(const SSyncRaftLog* log, SyncTerm t, ESyncRaftCode err) {
  if (err == RAFT_OK) {
    return t;
  }

  if (err == RAFT_COMPACTED) {
    return SYNC_NON_TERM;
  }
  SSyncRaft* pRaft = log->pRaft;
  syncFatal("[%d:%d]unexpected error %s", pRaft->selfGroupId, pRaft->selfId, gSyncRaftCodeString[err]);
  return SYNC_NON_TERM;
}