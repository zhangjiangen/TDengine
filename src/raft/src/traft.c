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

typedef struct {
  raft_node_id_t id;
  raft_index_t   next;
  raft_index_t   match;
  bool           isLearner;
} SRaftPeer;

#define RAFT_PEER_IS_LEARNER(p) ((p)->isLearner)

struct SRaft {
  raft_term_t    term;
  raft_node_id_t vote;
  raft_node_id_t leader;
  raft_role_t    role;
  bool           isLearner;
  SRaftLog *     log;
  raft_index_t   commitIdx;
  raft_index_t   appliedIdx;
  int            nPeers;
  SRaftPeer *    peers;  // TODO use a hash table or other container to implement this fiel
};

#define RAFT_TERM(r) ((r)->term)
#define RAFT_VOTE(r) ((r)->vote)
#define RAFT_LEADER(r) ((r)->leader)
#define RAFT_ROLE(r) ((r)->role)
#define RAFT_IS_LEARNER(r) ((r)->isLearner)
#define RAFT_LOG(r) ((r)->log)

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

#if 0
SRaft *traftNew(const SRaftCfg *pCfg) {
  SRaft *pRaft = NULL;
  // TODO
  return pRaft;
}

SRaft *traftFree(SRaft *pRaft) {
  // TODO
  return NULL;
}
#endif

int raftProcessMsg(SRaft *pRaft, SRaftMsg *pMsg) {
  // TODO: preprocess the message

  // Check the msg term. The reason that the work is here is to
  // call the corresponding handle function since the role may
  // change.

  int ret = raftCheckMsgTerm(pRaft, pMsg);
  if (ret) {
    // TODO
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
  switch (RAFT_MSG_TYPE(pRaft)) {
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
    case RAFT_HEARTBEAT_REQ:
      break;
    case RAFT_HEARTBEAT_RSP:
      break;
    default:
      NOT_POSSIBLE();
  }
  return 0;
}

static int candidateProcessMsg(SRaft *pRaft, SRaftMsg *pMsg) {
  switch (RAFT_MSG_TYPE(pRaft)) {
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
    case RAFT_HEARTBEAT_REQ:
      break;
    case RAFT_HEARTBEAT_RSP:
      break;
    default:
      NOT_POSSIBLE();
  }
  return 0;
}

static int leaderProcessMsg(SRaft *pRaft, SRaftMsg *pMsg) {
  int code = 0;

  switch (RAFT_MSG_TYPE(pMsg)) {
    case RAFT_CLIENT_REQ:
      code = raftHandleClientReq(pRaft, pMsg);
      break;
    case RAFT_APPEND_ENTRIES_RSP:
      code = raftHandleAppendEntriesRsp(pRaft, pMsg);
      break;
    case RAFT_REQUEST_VOTE_REQ:
      code = raftHandleRequestVoteReq(pRaft, pMsg);
      break;
    case RAFT_REQUEST_VOTE_RSP:
      break;
    case RAFT_PRE_VOTE_REQ:
      break;
    case RAFT_PRE_VOTE_RSP:
      break;
    case RAFT_HEARTBEAT_REQ:
      code = raftHandleHeartbeatReq(pRaft, pMsg);
      break;
    case RAFT_HEARTBEAT_RSP:
      break;
    default:
      NOT_POSSIBLE();
  }
  return code;
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

static int raftHandleClientReq(SRaft *pRaft, SRaftMsg *pMsg) {
  ASSERT(RAFT_ROLE(pRaft) == RAFT_ROLE_LEADER);

  // Set request term and indices
  SAppendEntriesReq *pReq = RAFT_MSG_CONTENT(pMsg);
  log_index_t lidx = raftLogLastIndex(pRaft->log);
  for (int i = 0; i < pReq->nEntries; i++) {
    SLogEntry *pEnt = pReq->entries + i;
    pEnt->term = RAFT_TERM(pRaft);
    pEnt->index = lidx + i + 1;
  }

  // Append entry to raft log
  if (raftLogAppendEntries(RAFT_LOG(pRaft), pMsg->nEntries, pMsg->entries) < 0) {
    // TODO: deal with error
    return -1;
  }

  // broadcast to other nodes
  RAFT_MSG_TYPE(pMsg) = RAFT_APPEND_ENTRIES_REQ;
  raftBroadcastMsg(pRaft, pMsg);

  return 0;
}

static void raftBroadcastMsg(SRaft *pRaft, SRaftMsg *pMsg) {
  for (size_t i = 0; i < taosArrayGetSize(pRaft->peers); i++) {
    SRaftPeer *pPeer = taosArrayGet(pRaft->peers, i);

    raftSendMsgToPeer(RAFT_PEER_ID(pPeer), pMsg);
  }
}

static int raftLeaderCheckMsgTerm(SRaft *pRaft, SRaftMsg *pMsg) {
  switch (RAFT_MSG_TYPE(pMsg)) {
    case RAFT_CLIENT_REQ:
      return 0;
    case RAFT_APPEND_ENTRIES_REQ:
      if (RAFT_MSG_TERM(pMsg) > RAFT_TERM(pRaft)) {
        // Now I am an out-of-date leader, there exists a leader
        // with higher term, I need to become the follower, and
        // process the message as a follower.
        raftBecomeFollower(pRaft, RAFT_MSG_TERM(pMsg));
        return 0;
      } else if (RAFT_MSG_TERM(pMsg) < RAFT_TERM(pRaft)) {
        // I am the leader with higher term, just ignre the message.
        // But I have the responsibility to tell the peer that I am
        // the new leader, just give up the leader role.

        // TODO: code here need refact, also, we may set an error code here
        SRaftMsg msg = {.term = RAFT_TERM(pRaft),
                        .type = RAFT_APPEND_ENTRIES_RSP,
                        .from = RAFT_SELF_NODE_ID(),
                        .to = RAFT_MSG_TO(pMsg),
                        reqs = NULL};
        raftSendMsg(&msg);
        return -1;
      } else {
        // There is another leader with the same term as me. That is
        // no possible. There must be a bug.
        NOT_POSSIBLE();
      }
      break;
    case RAFT_APPEND_ENTRIES_RSP:
      if (RAFT_MSG_TERM(pMsg) > RAFT_TERM(pRaft)) {
        // TODO: Is it valid here ????????
        NOT_POSSIBLE();
      } else if (RAFT_MSG_TERM(pMsg) < RAFT_TERM(pRaft)) {
        // TODO: how to handle this senario ??????
      } else {
        // I am the master, I need to handle this senario as normal
        return 0;
      }
      break;
    case RAFT_REQUEST_VOTE_REQ:
      if (RAFT_MSG_TERM(pMsg) > RAFT_TERM(pRaft)) {
        // I am the master but I recv a request vote msg with higher term. I need to become
        // a follower and handle the request
        raftBecomeFollower(pRaft, RAFT_MSG_TERM(pMsg));
        return 0;
      } else if (RAFT_MSG_TERM(pMsg) < RAFT_TERM(pRaft)) {
        // Just ignore the message. Should I tell the peer that I am the master?
        return -1;
      } else {
        // Another candidate with the same term ask for vote. I need to tell the peer that I
        // am the leader now, jut be a follower.
        return 0;
      }
    case RAFT_REQUEST_VOTE_RSP:
      if (RAFT_MSG_TERM(pMsg) > RAFT_TERM(pRaft)) {
        raftBecomeFollower(pRaft, RAFT_MSG_TERM(pMsg));
        return 0;
      } else if (RAFT_MSG_TERM(pMsg) < RAFT_TERM(pRaft)) {
        // Do we need to tell the node that it is rejected?
        return -1;
      } else {
        return 0;
      }
      break;
    case RAFT_PRE_VOTE_REQ:
      if (RAFT_MSG_TERM(pMsg) > RAFT_TERM(pRaft)) {
        // TODO
      } else if (RAFT_MSG_TERM(pMsg) < RAFT_TERM(pRaft)) {
        // TODO
      } else {
        // TODO
      }
      break;
    case RAFT_PRE_VOTE_RSP:
      if (RAFT_MSG_TERM(pMsg) > RAFT_TERM(pRaft)) {
        // TODO
      } else if (RAFT_MSG_TERM(pMsg) < RAFT_TERM(pRaft)) {
        // TODO
      } else {
        // TODO
      }
      break;
    case RAFT_HEARTBEAT_REQ:
      if (RAFT_MSG_TERM(pMsg) > RAFT_TERM(pRaft)) {
        raftBecomeFollower(pRaft, RAFT_MSG_TERM(pMsg));
        return 0;
      } else if (RAFT_MSG_TERM(pMsg) < RAFT_TERM(pRaft)) {
        // Just ignore the HB
        return -1;
      } else {
        NOT_POSSIBLE();
      }
      break;
    case RAFT_HEARTBEAT_RSP:
      if (RAFT_MSG_TERM(pMsg) > RAFT_TERM(pRaft)) {
        raftBecomeFollower(pRaft, RAFT_MSG_TERM(pMsg));
        return 0;
      } else if (RAFT_MSG_TERM(pMsg) < RAFT_TERM(pRaft)) {
        // ignore
        return -1;
      } else {
        // need to handle the HB rsp
        return 0;
      }
      break;
    default:
      NOT_POSSIBLE();
  }
}

static int raftFollwerCheckMsgTerm(SRaft *pRaft, SRaftMsg *pMsg) {
  switch (RAFT_MSG_TYPE(pMsg)) {
    case RAFT_CLIENT_REQ:
      // I am a follower, I cannot handle client request
      // I need to tell the client that I am not the leader
      // and tell who is the leader now if I know (I must know), [TODO]
      return -1;
      break;
    case RAFT_APPEND_ENTRIES_REQ:
      if (RAFT_MSG_TERM(pMsg) > RAFT_TERM(pRaft)) {
        // A leader with higher term exists, need to update
        // self term, and handle the request as needed
        raftBecomeFollower(pRaft, RAFT_TERM(pRaft));
        return 0;
      } else if (RAFT_MSG_TERM(pMsg) < RAFT_TERM(pRaft)) {
        // No need to handle this kind of message
        // TODO: should I tell the node that it is out of date????
        return -1;
      } else {
        return 0;
      }
    case RAFT_APPEND_ENTRIES_RSP:
      // I should not recv this kind of msg
      NOT_POSSIBLE();
      break;
    case RAFT_REQUEST_VOTE_REQ:
      break;
    case RAFT_REQUEST_VOTE_RSP:
      break;
    case RAFT_PRE_VOTE_REQ:
      break;
    case RAFT_PRE_VOTE_RSP:
      break;
    case RAFT_HEARTBEAT_REQ:
      break;
    case RAFT_HEARTBEAT_RSP:
      break;
    default:
      NOT_POSSIBLE();
  }

  return 0;
}

static int raftCandidateCheckMsgTerm(SRaft *pRaft, SRaftMsg *pMsg) {
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
    case RAFT_HEARTBEAT_REQ:
      break;
    case RAFT_HEARTBEAT_RSP:
      break;
    default:
      NOT_POSSIBLE();
  }
  return 0;
}

static int raftCheckMsgTerm(SRaft *pRaft, SRaftMsg *pMsg) {
  ASSERT((RAFT_MSG_TYPE(pMsg) != RAFT_CLIENT_REQ) || (RAFT_MSG_TERM(pMsg) == RAFT_TERM_NONE));

  switch (RAFT_ROLE(pRaft)) {
    case RAFT_ROLE_LEADER:
      return raftLeaderCheckMsgTerm(pRaft, pMsg);
    case RAFT_ROLE_FOLLOWER:
      return raftFollwerCheckMsgTerm(pRaft, pMsg);
    case RAFT_ROLE_CANDIDATE:
      return raftCandidateCheckMsgTerm(pRaft, pMsg);
    default:
      NOT_POSSIBLE();
  }
}

// TODO: this interface need to refact and take a decision
static void raftSendMsg(SRaftMsg *pMsg) {
  // TODO
}

static void raftSendMsgToPeer(raft_node_id_t id, SRaftMsg *pMsg) {
  RAFT_MSG_TO(pMsg) = id;
  raftSendMsg(pMsg);
}

static int raftHandleAppendEntriesRsp(SRaft *pRaft, SRaftMsg *pMsg) {
  ASSERT(RAFT_ROLE(pRaft) == RAFT_ROLE_LEADER);

  SAppendEntriesRsp *pRsp = (SAppendEntriesRsp *)RAFT_MSG_CONTENT(pMsg);

  if (pRsp->success) {
    // TODO
  } else {
    // TODO: not success, decrement nextIndex and retry
  }

  return 0;
}

static int raftHandleRequestVoteReq(SRaft *pRaft, SRaftMsg *pMsg) {
  // TODO
  return 0;
}

static SRaftPeer *raftGetPeer(SRaft *pRaft, raft_node_id_t id) {
  for (int i = 0; i < pRaft->nPeers; i++) {
    SRaftPeer *pPeer = pRaft->peers + i;

    if (pPeer->id == id) {
      return pPeer;
    }
  }

  return NULL;
}