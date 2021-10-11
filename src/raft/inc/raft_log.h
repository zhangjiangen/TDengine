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

#include "raft_impl.h"

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
                  RaftIndex startIndex);

/** 
 * Get the number of entries the log. 
 **/
size_t raftLogNumEntries(const RaftLog* pLog);

/**
 * return index of last in memory log, return 0 if log is empty
 **/
RaftIndex raftLogLastIndex(RaftLog* pLog);

/**
 * return last term of in memory log, return 0 if log is empty
 **/
RaftTerm raftLogLastTerm(RaftLog* pLog);

/**
 * return term of log with the given index, return 0 if the term of index cannot be found
 * , errCode will save the error code.
 **/
RaftTerm raftLogTermOf(RaftLog* pLog, RaftIndex index, RaftCode* errCode);

/* Append a new entry to the log. */
int raftLogAppend(RaftLog* pLog,
                  RaftTerm term,
                  const struct RaftBuffer *buf);


#endif /* TD_RAFT_LOG_H */