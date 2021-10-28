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

#ifndef TD_RAFT_LOG_H
#define TD_RAFT_LOG_H

void raftLogInit(RaftLog* pLog);

void raftLogClose(RaftLog* pLog);

/**
 * When startup populating log entrues loaded from disk,
 * init raft memory log with snapshot index,term and log start idnex.
 **/ 
void raftLogStart(RaftLog* pLog,
                  RaftSnapshotMeta snapshot,
                  SyncIndex startIndex);

/** 
 * Get the number of entries the log. 
 **/
int raftLogNumEntries(const RaftLog* pLog);

/**
 * return index of last in memory log, return 0 if log is empty
 **/
SyncIndex raftLogLastIndex(RaftLog* pLog);

/**
 * return last term of in memory log, return 0 if log is empty
 **/
SSyncTerm raftLogLastTerm(RaftLog* pLog);

/**
 * return term of log with the given index, return 0 if the term of index cannot be found
 * , errCode will save the error code.
 **/
SSyncTerm raftLogTermOf(RaftLog* pLog, SyncIndex index, RaftCode* errCode);

/** 
 * Get the last index of the most recent snapshot. Return 0 if there are no *
 * snapshots. 
 **/
SyncIndex raftLogSnapshotIndex(RaftLog* pLog);

/* Append a new entry to the log. */
int raftLogAppend(RaftLog* pLog,
                  SSyncTerm term,
                  const SSyncBuffer *buf);

/**
 * acquire log from given index onwards.
 **/ 
int raftLogAcquire(RaftLog* pLog,
                  SyncIndex index,
                  RaftEntry **ppEntries,
                  int *n);

void raftLogRelease(RaftLog* pLog,
                    SyncIndex index,
                    RaftEntry *pEntries,
                    int n);

/* Delete all entries from the given index (included) onwards. */
void raftLogTruncate(RaftLog* pLog, SyncIndex index);

/** 
 * when taking a new snapshot, the function will update the last snapshot information and delete
 * all entries up last_index - trailing (included). If the log contains no entry
 * a last_index - trailing, then no entry will be deleted. 
 **/
void raftLogSnapshot(RaftLog* pLog, SyncIndex index, SyncIndex trailing);

#endif /* TD_RAFT_LOG_H */