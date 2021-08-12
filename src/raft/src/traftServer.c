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
#include "traft.h"

struct SRaftServer {
  // TODO
};

static struct SRaftServer gRaftServer;
int (*msg_process_fn)(SRaftCtx *pCtx, SRaftMsg *pMsg);
static msg_process_fn msg_vtable[RAFT_MAX_MSGS] = {
    raftProcessClienReq,        // client_req
    raftProcessAppendEntryReq,  // append_entries_req,
    raftProcessAppendEntryRsp,  // append_entries_rsp,
    raftProcessRequestVoteReq,  // request_vote_req,
    raftProcessRequestVoteRsp   // request_vote_rsp,
};

static int raftSetMsgInfo(SRaftCtx *pCtx, SRaftMsg *pMsg) {
  // TODO
  return 0;
}

static bool raftCheckTerm(SRaftCtx *pCtx, raft_term_t term) {
  bool termUpdated = false;

  if (term > RAFT_CTX_TERM(pCtx)) {
    RAFT_CTX_UPDATE_TERM(pCtx, term);
    raftBecomeFollower(pCtx);
    termUpdated = true;
  }

  return termUpdated;
}

static int raftBecomeFollower(SRaftCtx *pCtx) {
  // TODO
  return 0;
}

static int raftProcessClienReq(SRaftCtx *pCtx, SRaftMsg *pMsg) {
  if (RAFT_CTX_ROLE(pCtx) != RAFT_ROLE_LEADER) {
    // TODO: Only leader can process client request
    return -1;
  }

  // Modify message infos like term, idx etc.
  raftSetMsgInfo(pCtx, pMsg);

  if (RAFT_MSG_IS_SYNC(pMsg)) {
    // Sync replication message
  } else {
    // Async replication message
  }

  return 0;
}

static int raftProcessAppendEntryReq(SRaftCtx *pCtx, SRaftMsg *pMsg) {
  bool termUpdated = raftCheckTerm(pCtx, RAFT_MSG_TERM(pMsg));

  if (termUpdated) {
    // TODO: do something or just print some log info
  }

  if (RAFT_CTX_TERM(pCtx) == RAFT_MSG_TERM(pMsg)) {
    if (RAFT_CTX_ROLE(pCtx) == RAFT_ROLE_FOLLOWER) {
      // TODO: process replication normally
    } else if (RAFT_CTX_ROLE(pCtx) == RAFT_ROLE_CANDIDATE) {
      // TODO
    } else {
      // TODO: there must be a bug here, need to fix
      ASSERT(0);
    }
  }

  if (RAFT_CTX_TERM(pCtx) < RAFT_MSG_TERM(pMsg)) {
    // TODO
    return -1;
  }

  return 0;
}

static int raftProcessAppendEntryRsp(SRaftCtx *pCtx, SRaftMsg *pMsg) {
  // TODO
  return 0;
}

static int raftProcessRequestVoteReq(SRaftCtx *pCtx, SRaftMsg *pMsg) {
  // TODO
  return 0;
}

static int raftProcessRequestVoteRsp(SRaftCtx *pCtx, SRaftMsg *pMsg) {
  // TODO
  return 0;
}

int traftInit() {
  // TODO
  return 0;
}

int traftClear() {
  // TODO
  return 0;
}

int traftProcessMsg(SRaftCtx *pCtx, SRaftMsg *pMsg) {
  int code = (*msg_vtable[RAFT_MSG_TYPE(pMsg)])(pCtx, pMsg);
  // TODO
  return 0;
}
