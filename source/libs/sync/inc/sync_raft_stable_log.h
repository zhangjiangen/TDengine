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

#ifndef _TD_LIBS_SYNC_RAFT_STABLE_LOG_H
#define _TD_LIBS_SYNC_RAFT_STABLE_LOG_H

#include "sync.h"
#include "sync_raft_code.h"
#include "sync_type.h"

SSyncRaftStableLog* syncRaftCreateStableLog();

void syncRaftStableEntries(SSyncRaftStableLog* storage, SyncIndex lo, SyncIndex hi, int maxSize, SSyncRaftEntry** ppEntries, int* n);

SyncIndex syncRaftStableLogLastIndex(const SSyncRaftStableLog* storage);

// FirstIndex returns the index of the first log entry that is
// possibly available via Entries (older entries have been incorporated
// into the latest Snapshot; if storage only contains the dummy entry the
// first log entry is not available).
SyncIndex syncRaftStableLogFirstIndex(const SSyncRaftStableLog* storage);

int syncRaftStableAppendEntries(SSyncRaftStableLog* storage, SSyncRaftEntry* entries, int n);

#endif // _TD_LIBS_SYNC_RAFT_STABLE_LOG_H
