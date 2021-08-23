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

#ifndef _TD_TRAFT_DEFS_H_
#define _TD_TRAFT_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

// raft_term_t
typedef uint64_t raft_term_t;
#define RAFT_TERM_NONE ((raft_term_t)0)
#define RAFT_TERM_IS_VLD(term) ((term) != RAFT_TERM_NONE)

// raft_node_id_t
typedef uint32_t raft_node_id_t;
#define RAFT_NODE_ID_NONE UINT32_MAX
#define RAFT_NODE_ID_IS_NONE(id) ((id) == RAFT_NODE_ID_NONE)

// raft_role_t
typedef enum {
  RAFT_ROLE_FOLLOWER = 0,  // follower
  RAFT_ROLE_CANDIDATE,     // candidate
  RAFT_ROLE_LEADER,        // leader
  RAFT_ROLE_MAX
} raft_role_t;

#define RAFT_ROLE_IS_VLD(r) ((r) >= RAFT_ROLE_FOLLOWER) && ((r) < RAFT_ROLE_MAX)

static FORCE_INLINE const char *raftRoleToString(raft_role_t role) {
  switch (role) {
    case RAFT_ROLE_FOLLOWER:
      return "follower";
    case RAFT_ROLE_CANDIDATE:
      return "candidate";
    case RAFT_ROLE_LEADER:
      return "leader";
    default:
      return "invalid role";
  }
}

// raft_index_t
typedef uint64_t raft_index_t;

// raft_msg_type_t
typedef enum {
  RAFT_CLIENT_REQ = 0,      // client req, only LEADER can handle this kind of msg req
  RAFT_APPEND_ENTRIES_REQ,  // append entry request sent from LEADER, to FOLLOWER and CANDIDATE
  RAFT_APPEND_ENTRIES_RSP,  // append entry response
  RAFT_REQUEST_VOTE_REQ,    // vote req
  RAFT_REQUEST_VOTE_RSP,    // vote rsp
  RAFT_PRE_VOTE_REQ,        // pre-vote req
  RAFT_PRE_VOTE_RSP,        // pre-vote rsp
  RAFT_HEARTBEAT_REQ,       // heartbeat req
  RAFT_HEARTBEAT_RSP,       // heartbeat rsp
  RAFT_MAX_MSGS
} raft_msg_type_t;

#define RAFT_MSG_IS_VLD(T) (((T) >= 0) && ((T) < RAFT_MAX_MSGS))

static FORCE_INLINE const char *raftMsgToString(raft_msg_type_t type) {
  switch (type) {
    case RAFT_CLIENT_REQ:
      return "raft client request";
    case RAFT_APPEND_ENTRIES_REQ:
      return "raft append entries request";
    case RAFT_APPEND_ENTRIES_RSP:
      return "raft append entries response";
    case RAFT_REQUEST_VOTE_REQ:
      return "raft request vote request";
    case RAFT_REQUEST_VOTE_RSP:
      return "raft request vote response";
    case RAFT_PRE_VOTE_REQ:
      return "raft pre-vote request";
    case RAFT_PRE_VOTE_RSP:
      return "raft pre-vote response";
    case RAFT_HEARTBEAT_REQ:
      return "raft heartbeat request";
    case RAFT_HEARTBEAT_RSP:
      return "raft heartbeat response";
    default:
      return "invalid raft msg type";
  }
}

// Other definitions
#define NOT_POSSIBLE() ASSERT(0)
#define IGNORE()

#ifdef __cplusplus
}
#endif

#endif /*_TD_TRAFT_DEFS_H_*/