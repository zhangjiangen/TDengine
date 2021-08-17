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

#ifndef _TD_TRAFT_MSG_TYPE_H_
#define _TD_TRAFT_MSG_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

enum raft_msg_type_t : uint8_t {
  RAFT_CLIENT_REQ = 0,
  RAFT_APPEND_ENTRIES_REQ,
  RAFT_APPEND_ENTRIES_RSP,
  RAFT_REQUEST_VOTE_REQ,
  RAFT_REQUEST_VOTE_RSP,
  RAFT_PRE_VOTE_REQ,
  RAFT_PRE_VOTE_RSP,
  RAFT_MAX_MSGS
};

#define RAFT_IS_VALID_MSG(T) (((T) >= RAFT_CLIENT_REQ) && ((T) < RAFT_MAX_MSGS))

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
    default:
      return "invalid raft msg type";
  }
}

#ifdef __cplusplus
}
#endif

#endif /*_TD_TRAFT_MSG_TYPE_H_*/