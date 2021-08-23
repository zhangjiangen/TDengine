/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
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

#ifndef _TD_TRAFT_LOG_H_
#define _TD_TRAFT_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t raft_index_t;

typedef struct {
  SArray *     logs;       // need another data structure to support the log array
  raft_index_t committed;  // highest committed log index
  raft_index_t applied;    // highest applied log index
} SRaftLog;

#define RAFT_LOG_COMMITTED(l) ((l)->committed)
#define RAFT_LOG_APPLIED(l) ((l)->applied)
#define RAFT_LOG_NUM_OF_LOGS(l)

void         raftLogInit(SRaftLog *pRaftLog, size_t maxSize);
void         raftLogClear(SRaftLog *pRaftLog);
void         raftLogAppendEntries(SRaftLog *pRaftLog, SArray *logs);
raft_index_t raftLogFirstIndex(SRaftLog *pRaftLog);
raft_index_t raftLogLastIndex(SRaftLog *pRaftLog);

#ifdef __cplusplus
}
#endif

#endif /*_TD_TRAFT_LOG_H_*/