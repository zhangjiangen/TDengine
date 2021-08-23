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

#ifndef _TD_TRAFT_H_
#define _TD_TRAFT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "traftDefs.h"

#include "traftCfg.h"

#include "traftMsg.h"

// Global node ID definition
extern raft_node_id_t raftNodeId;
#define RAFT_SELF_NODE_ID() raftNodeId

// SRaftLog Definition
typedef struct SRaftLog SRaftLog;

log_index_t raftLogLastIndex(SRaftLog *pRaftLog);

// SRaft Definition
typedef struct SRaft SRaft;

// SLogEntry definition
typedef struct SLogEntry SLogEntry;

int raftProcessMsg(SRaft *pRaft, SRaftMsg *pMsg);

// ============== May need to hide info below
struct SLogEntry {
  raft_term_t  term;
  raft_index_t index;
  void *       data;
};

#ifdef __cplusplus
}
#endif

#endif /*_TD_TRAFT_H_*/