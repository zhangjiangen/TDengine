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

#ifndef _TD_TRAFT_ROLE_H_
#define _TD_TRAFT_ROLE_H_

#ifdef __cplusplus
extern "C" {
#endif

enum raft_role_t : uint8_t {
  RAFT_ROLE_FOLLOWER = 0,  // follower
  RAFT_ROLE_CANDIDATE,     // candidate
  RAFT_ROLE_LEADER,        // leader
  RAFT_ROLE_MAX
};

#define RAFT_IS_VLD_ROLE(r) ((r) >= RAFT_ROLE_FOLLOWER) && ((r) < RAFT_ROLE_MAX)

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

#ifdef __cplusplus
}
#endif

#endif /*_TD_TRAFT_ROLE_H_*/