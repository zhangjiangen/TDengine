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

#include "traftCfg.h"
#include "traftNodeID.h"
#include "traftRole.h"
#include "traftTerm.h"

typedef struct {
  raft_term_t    term;    // current term
  raft_node_id_t vote;    // voted for in this term
  raft_node_id_t leader;  // leader ID of current term
  raft_role_t    role;
  bool           isLearner;

  // Raft configuts
  RAFT_CFGS
} SRaft;

#define RAFT_TERM(r) ((r)->term)
#define RAFT_VOTE(r) ((r)->vote)
#define RAFT_LEADER(r) ((r)->leader)
#define RAFT_ROLE(r) ((r)->role)

int raftProcessMsg(SRaft *pRaft, SRaftMsg *pMsg);

#ifdef __cplusplus
}
#endif

#endif /*_TD_TRAFT_H_*/