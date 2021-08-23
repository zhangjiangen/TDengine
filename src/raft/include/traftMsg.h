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

#ifndef _TD_TRAFT_MSG_H_
#define _TD_TRAFT_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "traftDefs.h"

typedef struct {
  raft_msg_type_t type;
  raft_node_id_t  from;
  raft_node_id_t  to;
  raft_term_t     term;
  char            content[];
} SRaftMsg;

#define RAFT_MSG_TYPE(m) ((m)->type)
#define RAFT_MSG_FROM(m) ((m)->from)
#define RAFT_MSG_TO(m) ((m)->to)
#define RAFT_MSG_TERM(m) ((m)->term)
#define RAFT_MSG_CONTENT(m) ((m)->content)

typedef struct {
  raft_node_id_t leader;
  raft_index_t   prevLogIdx;
  raft_term_t    prevLogTerm;
  raft_index_t   leaderCommit;
  int            nEntries;
  SLogEntry*     entries;
} SAppendEntriesReq;

typedef struct {
  bool        success;
} SAppendEntriesRsp;

typedef struct {
  raft_node_id_t candidateId;
  raft_index_t   lastLogIdx;
  raft_term_t    lastLogTerm;
} SRequestVoteReq;

typedef struct {
  bool        granted;
} SRequestVoteRsp;

#ifdef __cplusplus
}
#endif

#endif /*_TD_TRAFT_MSG_H_*/