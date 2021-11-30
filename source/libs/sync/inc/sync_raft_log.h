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

#ifndef _TD_LIBS_SYNC_RAFT_LOG_H
#define _TD_LIBS_SYNC_RAFT_LOG_H

#include "sync.h"
#include "sync_type.h"
#include "sync_raft_code.h"

SSyncRaftLog* syncCreateRaftLog(SSyncRaftStableLog* storage, uint64_t maxNextEntsSize, const SSyncRaft*);

bool syncRaftLogMaybeAppend(SSyncRaftLog* log, SyncIndex index, SyncTerm logTerm, 
                            SyncIndex committedIndex, const SSyncRaftEntry* entries, int n, SyncIndex* lastNewIndex);

SyncIndex syncRaftLogAppend(SSyncRaftLog*, const SSyncRaftEntry* entries, int n);

bool syncRaftLogMatchTerm(SSyncRaftLog* log, SyncIndex index, SyncTerm logTerm);

bool syncRaftLogCommitTo(SSyncRaftLog* log, SyncIndex toCommit);

SyncIndex syncRaftLogLastIndex(const SSyncRaftLog* log);

SyncTerm syncRaftLogTermOf(SSyncRaftLog* log, SyncIndex index, ESyncRaftCode* errCode);

SyncIndex syncRaftLogFirstIndex(const SSyncRaftLog* log);

SyncIndex syncRaftLogLastIndex(const SSyncRaftLog* log);

SyncIndex syncRaftLogFindConflictByTerm(const SSyncRaftLog* log, SyncIndex index, SyncTerm term);

SyncTerm syncRaftLogLastTerm(const SSyncRaftLog* pLog);

void syncRaftLogAppliedTo(SSyncRaftLog* pLog, SyncIndex appliedIndex);

void syncRaftLogStableTo(SSyncRaftLog* log, SyncIndex, SyncTerm);

bool syncRaftLogIsUptodate(const SSyncRaftLog* pLog, SyncIndex index, SyncTerm term);

bool syncRaftHasUnappliedLog(const SSyncRaftLog* pLog);

int syncRaftLogSlice(SSyncRaftLog* pLog, SyncIndex lo, SyncIndex hi, SSyncRaftEntry** ppEntries, int* n);

int syncRaftLogNumOfPendingConf(SSyncRaftLog* pLog);

int syncRaftLogEntries(SSyncRaftLog* pLog, SyncIndex index,
                      SSyncRaftEntry **ppEntries, int *n);

bool syncRaftMaybeCommit(SSyncRaftLog* log, SyncIndex maxIndex, SyncTerm term);





int syncRaftLogAcquire(SSyncRaftLog* pLog, SyncIndex index, int maxMsgSize,
                      SSyncRaftEntry **ppEntries, int *n);

void syncRaftLogRelease(SSyncRaftLog* pLog, SyncIndex index,
                      SSyncRaftEntry *pEntries, int n);


#endif  /* _TD_LIBS_SYNC_RAFT_LOG_H */
