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

  return storage;
}

SyncIndex syncRaftStableLogLastIndex(const SSyncRaftStableLog* storage) {
  //return syncRaftLastIndexOfEntries(storage->entries);
}

// FirstIndex returns the index of the first log entry that is
// possibly available via Entries (older entries have been incorporated
// into the latest Snapshot; if storage only contains the dummy entry the
// first log entry is not available).
