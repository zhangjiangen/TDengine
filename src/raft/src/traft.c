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

// Function declarations
static int  followerProcessMsg(SRaft *pRaft, SRaftMsg *pMsg);
static int  candidateProcessMsg(SRaft *pRaft, SRaftMsg *pMsg);
static int  leaderProcessMsg(SRaft *pRaft, SRaftMsg *pMsg);
static void raftBecomeFollower(SRaft *pRaft);
static void raftBecomeCandidate(SRaft *pRaft);
static void raftBecomeLeader(SRaft *pRaft);

// Message process table
typedef int (*raft_process_msg_fn)(SRaft *pRaft, SRaftMsg *pMsg);
static raft_process_msg_fn raftProcessMsgTable[RAFT_ROLE_MAX] = {
    followerProcessMsg,   // for follower
    candidateProcessMsg,  // for candidate
    leaderProcessMsg      // for leader
};

SRaft *traftNew(const SRaftCfg *pCfg) {
  SRaft *pRaft = NULL;
  // TODO
  return pRaft;
}

SRaft *traftFree(SRaft *pRaft) {
  // TODO
  return NULL;
}

int raftProcessMsg(SRaft *pRaft, SRaftMsg *pMsg) {
  // TODO: preprocess the message

  // Check term part
  ASSERT(RAFT_TERM_IS_VLD(RAFT_MSG_TERM(pMsg)));
  if (RAFT_MSG_TERM(pMsg) > RAFT_TERM(pRaft)) {
    if (false) {
    } else {
      // Become FOLLOWER by default
      raftBecomeFollower(pRaft);
    }
  } else if (RAFT_MSG_TERM(pMsg) < RAFT_TERM(pRaft)) {
    // Just ignore the message
    return 0;
  }

  // call corresponding process function
  int ret = (*raftProcessMsgTable[RAFT_ROLE(pRaft)])(pRaft, pMsg);
  if (ret < 0) {
    //   TODO: deal with the error here
  }

  // TODO: postprocess the message

  return 0;
}

static int followerProcessMsg(SRaft *pRaft, SRaftMsg *pMsg) {
  // TODO
  return 0;
}

static int candidateProcessMsg(SRaft *pRaft, SRaftMsg *pMsg) {
  // TODO
  return 0;
}

static int leaderProcessMsg(SRaft *pRaft, SRaftMsg *pMsg) {
  switch (RAFT_MSG_TYPE(pMsg)) {
    case RAFT_CLIENT_REQ:
      break;
    case RAFT_APPEND_ENTRIES_REQ:
      break;
    case RAFT_APPEND_ENTRIES_RSP:
      break;
    case RAFT_REQUEST_VOTE_REQ:
      break;
    case RAFT_REQUEST_VOTE_RSP:
      break;
    case RAFT_PRE_VOTE_REQ:
      break;
    case RAFT_PRE_VOTE_RSP:
      break;
    default:
      break;
  }
  return 0;
}

static void raftBecomeFollower(SRaft *pRaft, raft_term_t term) {
  pRaft->term = term;
  pRaft->role = RAFT_ROLE_FOLLOWER;
  // TODO: need to reset timer
}

static void raftBecomeCandidate(SRaft *pRaft) {
  pRaft->role = RAFT_ROLE_CANDIDATE;
  // TODO
}

static void raftBecomeLeader(SRaft *pRaft) {
  pRaft->role = RAFT_ROLE_LEADER;
  // TODO
}