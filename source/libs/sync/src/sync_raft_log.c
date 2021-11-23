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

#include "sync_raft_log.h"
#include "sync_raft_stable_log.h"
#include "sync_raft_unstable_log.h"

struct SSyncRaftEntry {
  SyncTerm term;
  SSyncBuffer buffer;
  unsigned int refCount;
};

struct SSyncRaftLog {
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

  // Initialize our committed and applied pointers to the time of the last compaction.
  log->committedIndex = firstIndex - 1;
  log->committedIndex = firstIndex - 1;
  return log;
}

#if 0
static int raftLogNumEntries(const SSyncRaftLog* pLog);
static int locateEntry(SSyncRaftLog* pLog, SyncIndex index);

/* raft log entry with reference count */
struct SSyncRaftEntry {
  SyncTerm term;
  SSyncBuffer buffer;
  unsigned int refCount;
};

/* meta data about snapshot */
typedef struct RaftSnapshotMeta {
  SyncIndex lastIndex;
  SyncTerm  lastTerm;
} RaftSnapshotMeta;

/* in-memory raft log storage */
struct SSyncRaftLog {
  /* Circular buffer of log entries */
  SSyncRaftEntry *entries;

  /* size of Circular buffer */
  int size;

  /* Indexes of used slots [front, back) */
  int front, back;

  /* Index of first entry is offset + 1 */
  SyncIndex offset;

  /* meta data of snapshot */
  RaftSnapshotMeta snapshot;

  SyncIndex uncommittedConfigIndex;

  SyncIndex commitIndex;

  SyncIndex appliedIndex;
};

SSyncRaftLog* syncCreateRaftLog() {
  SSyncRaftLog* log = (SSyncRaftLog*)malloc(sizeof(SSyncRaftLog));
  if (log == NULL) {
    return NULL;
  }

  return log;
}

SyncIndex syncRaftLogLastIndex(SSyncRaftLog* pLog) {
  /**
   * if there are no entries and there is a snapshot,
   * check that the last index of snapshot is not smaller than log offset
   **/
  if (raftLogNumEntries(pLog) && pLog->snapshot.lastIndex != 0) {
    assert(pLog->offset <= pLog->snapshot.lastIndex);
  }

  return pLog->offset + raftLogNumEntries(pLog);
}

SyncIndex syncRaftLogFirstIndex(SSyncRaftLog* pLog) {
  return pLog->offset - 1;
}

SyncIndex syncRaftLogSnapshotIndex(SSyncRaftLog* pLog) {
  return pLog->snapshot.lastIndex;
}

SyncTerm syncRaftLogLastTerm(SSyncRaftLog* pLog) {
  SyncIndex lastIndex;

  lastIndex = syncRaftLogLastIndex(pLog);

  if (lastIndex > 0) {
    return syncRaftLogTermOf(pLog, lastIndex, NULL);
  }

  return SYNC_NON_TERM;
}

void syncRaftLogAppliedTo(SSyncRaftLog* pLog, SyncIndex appliedIndex) {

}

bool syncRaftLogIsUptodate(SSyncRaftLog* pLog, SyncIndex index, SyncTerm term) {
  return true;
}

int syncRaftLogNumOfPendingConf(SSyncRaftLog* pLog) {
  return 0;
}

bool syncRaftHasUnappliedLog(SSyncRaftLog* pLog) {
  return pLog->commitIndex > pLog->appliedIndex;
}

SyncTerm syncRaftLogTermOf(SSyncRaftLog* pLog, SyncIndex index, ESyncRaftCode* errCode) {
  int i;

  assert(index > 0);
  assert(pLog->offset <= pLog->snapshot.lastIndex);

  if (index == pLog->snapshot.lastIndex) {
    assert(pLog->snapshot.lastTerm != 0);
    i = locateEntry(pLog, index);
    /**
     * if we still have the entry at last index, then its term MUST match to the snapshot last term
     **/
    if (i != pLog->size) {
      assert(pLog->entries[i].term == pLog->snapshot.lastTerm);
    }

    return pLog->snapshot.lastTerm;
  }

  /* is the log has been compacted? */
  if (index < pLog->offset + 1) {
    if (errCode) *errCode = RAFT_INDEX_COMPACTED;
    return 0;
  }

  /* is the log index bigger than the last log index? */
  if (index > raftLogLastIndex(pLog)) {
    if (errCode) *errCode = RAFT_INDEX_UNAVAILABLE;
    return 0;
  }
  
  i = locateEntry(pLog, index);
  assert(i < pLog->size);
  return pLog->entries[i].term;
}

int syncRaftLogAppend(SSyncRaftLog* pLog, SSyncRaftEntry *pEntries, int n) {

}

int syncRaftLogAcquire(SSyncRaftLog* pLog, SyncIndex index, int maxMsgSize,
                      SSyncRaftEntry **ppEntries, int *n) {
  return 0;
}

void syncRaftLogRelease(SSyncRaftLog* pLog, SyncIndex index,
                      SSyncRaftEntry *pEntries, int n) {
  return;
}

#endif