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

#include "raft.h"
#include "syncInt.h"
#include "sync_raft_entry.h"
#include "sync_raft_proto.h"
#include "sync_raft_stable_log.h"

struct SSyncRaftStableLog {
  // owner raft
  SSyncRaft* pRaft;
  RaftSnapshotMeta snapshotMeta;

  // entries[i] has raft log position i+snapshotMeta.Index
  SSyncRaftEntryArray* entries;
};

SSyncRaftStableLog* syncRaftCreateStableLog(SSyncRaft* pRaft) {
  SSyncRaftStableLog* storage = (SSyncRaftStableLog*)malloc(sizeof(SSyncRaftStableLog));
  if (storage == NULL) {
    return NULL;
  }
  storage->entries = syncRaftCreateEntryArray();
  if (storage->entries == NULL) {
    free(storage);
    return NULL;
  }

  // When starting from scratch populate the list with a dummy entry at term zero.
  syncRaftAppendEmptyEntry(storage->entries);

  storage->pRaft = pRaft;

  return storage;
}

// Entries returns a slice of log entries in the range [lo,hi).
// MaxSize limits the total size of the log entries returned, but
// Entries returns at least one entry if any.
int syncRaftStableEntries(SSyncRaftStableLog* storage, SyncIndex lo, SyncIndex hi, SSyncRaftEntry** ppEntries, int* n) {
  SyncIndex offset = syncRaftFirstEntry(storage->entries)->index;
  if (lo <= offset) {
    return RAFT_COMPACTED;
  }

  SSyncRaft* pRaft = storage->pRaft;
  SyncIndex lastIndex = syncRaftStableLogLastIndex(storage);
  if (hi > lastIndex + 1) {
    syncFatal("[%d:%d]entries' hi(%" PRId64 ") is out of bound lastindex(%" PRId64 ")",
      pRaft->selfGroupId, pRaft->selfId, hi, lastIndex);
  }
  int num = syncRaftNumOfEntries(storage->entries);
  // only contains dummy entries.
  if (num == 1) {
    return RAFT_UNAVAILABLE;
  }

  return syncRaftSliceEntries(storage->entries, lo, hi, ppEntries, n);
}

// Term returns the term of entry i, which must be in the range
// [FirstIndex()-1, LastIndex()]. The term of the entry before
// FirstIndex is retained for matching purposes even though the
// rest of that entry may not be available.
SyncTerm syncRaftStableLogTerm(const SSyncRaftStableLog* storage, SyncIndex i, ESyncRaftCode* err) {
  SyncIndex offset = syncRaftFirstEntry(storage->entries)->index;
  if (i < offset) {
    *err = RAFT_COMPACTED;
    return SYNC_NON_TERM;
  }

  int num = syncRaftNumOfEntries(storage->entries);
  if ((int)(i - offset) >= num) {
    *err = RAFT_UNAVAILABLE;
    return SYNC_NON_TERM;
  }

  *err = RAFT_OK;
  return syncRaftEntryOfPosition(storage->entries, i - offset)->term;
}

// LastIndex returns the index of the last entry in the log.
SyncIndex syncRaftStableLogLastIndex(const SSyncRaftStableLog* storage) {
  return syncRaftFirstEntry(storage->entries)->index + syncRaftNumOfEntries(storage->entries) - 1;
}

// FirstIndex returns the index of the first log entry that is
// possibly available via Entries (older entries have been incorporated
// into the latest Snapshot; if storage only contains the dummy entry the
// first log entry is not available).
SyncIndex syncRaftStableLogFirstIndex(const SSyncRaftStableLog* storage) {
  return syncRaftFirstEntry(storage->entries)->index + 1;
}

// Append the new entries to storage.
// TODO (xiangli): ensure the entries are continuous and
// entries[0].Index > ms.entries[0].Index
int syncRaftStableAppendEntries(SSyncRaftStableLog* storage, SSyncRaftEntry* entries, int n) {
  if (n == 0) {
    return RAFT_OK;
  }

  SyncIndex firstIndex = syncRaftStableLogFirstIndex(storage);
  SyncIndex inFirstIndex = entries[0].index;
  SyncIndex inLastIndex = inFirstIndex + n - 1;

  // shortcut if there is no new entry.
  if (inLastIndex < firstIndex) {
    return RAFT_OK;
  }

  // truncate compacted entries
  SSyncRaftEntry* ents = entries;
  int nEnts = n;
  if (firstIndex > inFirstIndex) {
    ents = &entries[firstIndex - inFirstIndex];
    nEnts = n - (firstIndex - inFirstIndex);
  }

  inFirstIndex = ents[0].index;
  int offset = (int)inFirstIndex - firstIndex;
  int num = syncRaftNumOfEntries(storage->entries);
  if (num > offset) {
    SSyncRaftEntry* sliceEnts;
    int nSliceEnts;
    
    syncRaftSliceEntries(storage->entries, 0, offset, &sliceEnts, &nSliceEnts);

    syncRaftCleanEntryArray(storage->entries);
    syncRaftAppendEmptyEntry(storage->entries);
    syncRaftAppendEntries(storage->entries, sliceEnts, nSliceEnts);
    return syncRaftAppendEntries(storage->entries, ents, nEnts);
  }

  if (num == offset) {
    return syncRaftAppendEntries(storage->entries, ents, nEnts);
  }

  SSyncRaft* pRaft = storage->pRaft;
  SyncIndex lastIndex = syncRaftStableLogLastIndex(storage);
  syncFatal("[%d:%d]missing log entry [last: %" PRId64 ", append at: %" PRId64 "]",
    pRaft->selfGroupId, pRaft->selfId, lastIndex, firstIndex);

  return RAFT_OK;
}

// visit entries in [lo, hi - 1]
void syncRaftStableLogVisit(const SSyncRaftStableLog* storage, SyncIndex lo, SyncIndex hi, visitEntryFp visit, void* arg) {
  SyncIndex offsetIndex = syncRaftEntryOfPosition(storage->entries, 0)->index;
  if (lo <= offsetIndex) {
    return;
  }

  SyncIndex lastIndex = syncRaftStableLogLastIndex(storage);
  if (hi > lastIndex + 1) {
    return;
  }

  if (syncRaftNumOfEntries(storage->entries) == 1) {
    return;
  }

  while (lo < hi) {
    const SSyncRaftEntry* entry = syncRaftEntryOfPosition(storage->entries, lo - offsetIndex);
    visit(entry, arg);
  }
}
