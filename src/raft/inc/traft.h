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

#ifndef _TD_RAFT_H_
#define _TD_RAFT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tlogStore.h"
#include "tmsg.h"

typedef struct SRaftCtx SRaftCtx;

typedef uint64_t raft_term_t;

enum raft_role_t { RAFT_ROLE_FOLLOWER = 1, RAFT_ROLE_CANDIDATE, RAFT_ROLE_LEADER };

// Module Init/Clear APIs
int traftInit();
int traftClear();

// For multi-raft

// Process Message request
int traftProcessMsg(SRaftCtx *pRafth, SRaftMsg *pMsg);

/*----------------Do NOT use definitions below----------------*/
struct SRaftCtx {
  raft_term_t term;
  raft_role_t role;
  SLogStore   logStore;
};

#define RAFT_CTX_TERM(ctx) ((ctx)->term)
#define RAFT_CTX_ROLE(ctx) ((ctx)->role)
#define RAFT_CTX_UPDATE_TERM(ctx, term) (RAFT_CTX_TERM(ctx) = (term))

#ifdef __cplusplus
}
#endif

#endif /*_TD_RAFT_H_*/