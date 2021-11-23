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
#include "sync_raft_unstable_log.h"

struct SSyncRaftUnstableLog {

  // all entries that have not yet been written to storage.
  SSyncRaftEntryArray* entries;

  SyncIndex offset;
};

SSyncRaftUnstableLog* syncRaftCreateUnstableLog(SyncIndex lastIndex) {
  SSyncRaftUnstableLog* unstable = (SSyncRaftUnstableLog*)malloc(sizeof(SSyncRaftUnstableLog));
  if (unstable == NULL) {
    return NULL;
  }

  unstable->entries = syncRaftCreateEntryArray();
  if (unstable->entries == NULL) {
    free(unstable);
    return NULL;
  }

  unstable->offset = lastIndex + 1;

  return unstable;
}