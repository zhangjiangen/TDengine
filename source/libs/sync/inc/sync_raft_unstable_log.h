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

#ifndef _TD_LIBS_SYNC_RAFT_UNSTABLE_LOG_H
#define _TD_LIBS_SYNC_RAFT_UNSTABLE_LOG_H

#include "sync.h"
#include "sync_type.h"

SSyncRaftUnstableLog* syncRaftCreateUnstableLog(const SSyncRaft*, SyncIndex lastIndex);

// maybeFirstIndex returns the index of the first possible entry in entries
// if it has a snapshot.
bool syncRaftUnstableLogMaybeFirstIndex(const SSyncRaftUnstableLog* unstable, SyncIndex* index);

// maybeLastIndex returns the last index if it has at least one
// unstable entry or snapshot.
bool syncRaftUnstableLogMaybeLastIndex(const SSyncRaftUnstableLog* unstable, SyncIndex* index);

// maybeTerm returns the term of the entry at index i, if there
// is any.
bool syncRaftUnstableLogMaybeTerm(const SSyncRaftUnstableLog* unstable, SyncIndex i, SyncTerm* term);

void syncRaftUnstableLogStableTo(SSyncRaftUnstableLog* unstable, SyncIndex i, SyncTerm term);

int syncRaftUnstableLogTruncateAndAppend(SSyncRaftUnstableLog* unstable, SSyncRaftEntry* entries, int n);

void syncRaftUnstableLogSlice(const SSyncRaftUnstableLog* unstable, SyncIndex lo, SyncIndex hi, SSyncRaftEntry** ppEntries, int* n);

void syncRaftUnstableLogVisit(const SSyncRaftUnstableLog* storage, SyncIndex lo, SyncIndex hi, visitEntryFp visit, void* arg);

SyncIndex syncRaftUnstableLogOffset(const SSyncRaftUnstableLog* unstable);

#endif // _TD_LIBS_SYNC_RAFT_UNSTABLE_LOG_H
