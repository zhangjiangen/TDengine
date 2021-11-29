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

#include "sync_raft_entry.h"
#include "sync_raft_proto.h"
#include "sync_raft_stable_log.h"

struct SSyncRaftStableLog {
  RaftSnapshotMeta snapshotMeta;

  // entries[i] has raft log position i+snapshotMeta.Index
  SSyncRaftEntryArray* entries;
};

SSyncRaftStableLog* syncRaftCreateStableLog() {
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

  return storage;
}

void syncRaftStableEntries(SSyncRaftStableLog* storage, SyncIndex lo, SyncIndex hi, int maxSize, SSyncRaftEntry** ppEntries, int* n) {

}

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
  SyncIndex firstInIndex = entries[0].index;
  SyncIndex lastInIndex = entries[0].index + n - 1;

  // shortcut if there is no new entry.
  if (lastInIndex < firstIndex) {
    return RAFT_OK;
  }

  // truncate compacted entries
  SSyncRaftEntry* ents = entries;
  int nEnts = n;
  if (firstIndex > firstInIndex) {
    ents = &entries[firstIndex - firstInIndex];
    nEnts = n - (firstIndex - firstInIndex);
  }

  firstInIndex = ents[0].index;
  int offset = (int)firstInIndex - firstIndex;
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

  
  return RAFT_OK;
}