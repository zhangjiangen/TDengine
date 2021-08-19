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

#define RAFT_MSG_BASE   \
  raft_term_t     term; \
  raft_msg_type_t type;

typedef struct {
  RAFT_MSG_BASE

  raft_node_id_t from;
  raft_node_id_t to;
  SArray*        reqs;
} SRaftMsg;

#define RAFT_MSG_TERM(m) ((m)->term)
#define RAFT_MSG_TYPE(m) ((m)->type)
#define RAFT_MSG_FROM(m) ((m)->from)
#define RAFT_MSG_TO(m) ((m)->to)
#define RAFT_MSG_NUM_OF_REQS(m) taosArrayGetSize((m)->reqa)

#ifdef __cplusplus
}
#endif

#endif /*_TD_TRAFT_MSG_H_*/