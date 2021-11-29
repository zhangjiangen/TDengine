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

static SyncIndex findConflict(SSyncRaftLog*, const SSyncRaftEntry*, int n);

struct SSyncRaftLog {
  // owner Raft
  SSyncRaft* pRaft;

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

SSyncRaftLog* syncCreateRaftLog(SSyncRaftStableLog* storage, uint64_t maxNextEntsSize) {
  SSyncRaftLog* log = (SSyncRaftLog*)malloc(sizeof(SSyncRaftLog));
  if (log == NULL) {
    return NULL;
  }

  log->storage = storage;
  log->maxNextEntsSize = maxNextEntsSize;

  SyncIndex firstIndex = syncRaftStableLogFirstIndex(storage);
  SyncIndex lastIndex = syncRaftStableLogLastIndex(storage);

  SSyncRaftUnstableLog* unstable = syncRaftCreateUnstableLog(lastIndex);
  if (unstable == NULL) {
    free(log);
    return NULL;
  }
  log->unstable = unstable;

  // Initialize our committed and applied pointers to the time of the last compaction.
  log->committedIndex = firstIndex - 1;
  log->committedIndex = firstIndex - 1;
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
    syncFatal("[%d:%d]entry %d conflict with committed entry [committed(%d)]", 
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

}

bool syncRaftLogCommitTo(SSyncRaftLog* log, SyncIndex toCommit) {
  // never decrease commit
  if (log->committedIndex < toCommit) {
    SyncIndex lastIndex = syncRaftLogLastIndex(log);
    if (lastIndex < toCommit) {
      syncFatal("[%d:%d]entry %d conflict with committed entry [committed(%d)]",
        log->pRaft->selfGroupId, log->pRaft->selfId, ci, log->committedIndex);
    }
    log->committedIndex = toCommit;
  }
}

SyncIndex syncRaftLogLastIndex(const SSyncRaftLog* log) {

}

SyncTerm syncRaftLogTermOf(SSyncRaftLog* pLog, SyncIndex index, ESyncRaftCode* errCode) {
  // the valid term range is [index of dummy entry, last index]

}

SyncIndex syncRaftLogFirstIndex(SSyncRaftLog* log) {
  SyncIndex firstIndex;
  if (syncRaftUnstableLogMaybeFirstIndex(log->unstable, &firstIndex)) {
    return firstIndex;
  }
  return syncRaftStableLogFirstIndex(log->storage);
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
static SyncIndex findConflict(SSyncRaftLog* log, const SSyncRaftEntry* entries, int n) {
  int i;
  SyncIndex lastIndex = syncRaftLogLastIndex(log);
  for (i = 0; i < n; ++i) {
    const SSyncRaftEntry* entry = &entries[i];
    if (!syncRaftLogMatchTerm(log, entry->index, entry->term)) {
      if (entry->index <= lastIndex) {
        syncFatal("[%d:%d]found conflict at index %d [existing term: %d, conflicting term: %d]",
          log->pRaft->selfGroupId, log->pRaft->selfId, entry->index);
      }
      return entry->index;
    }
  }

  return 0;
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

  }
  return logTerm;
}

void syncRaftLogAppliedTo(SSyncRaftLog* log, SyncIndex index) {
  if (index == 0) {
    return;
  }

  if (log->committedIndex < index || index < log->appliedIndex) {

  }
  log->appliedIndex = index;
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
void syncRaftLogSlice(SSyncRaftLog* log, SyncIndex lo, SyncIndex hi, SSyncRaftEntry** ppEntries, int* n, int limit) {
  if (lo == hi) {
    *n = 0;
    return;
  }

}

static visitNumOfPendingConf(const SSyncRaftEntry* entry, void* arg) {
  int* n = (int*)arg;
  if (syncRaftIsConfEntry(entry)) *n = *n + 1;
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