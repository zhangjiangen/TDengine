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

#ifndef _TD_TMSG_H_
#define _TD_TMSG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "tlogstore.h"
#include "tmsgType.h"

#define RAFT_MSG_REQ 0
#define RAFT_MSG_RSP 1

typedef struct {
  raft_msg_type_t type;
  raft_term_t     term;
  log_idx_t       idx;
  char            body[];
  bool            isSync;
} SRaftMsg;

typedef struct SRaftMsgBatch {
  // TODO
} SRaftMsgBatch;

#define RAFT_MSG_TYPE(m) ((m)->type)
#define RAFT_MSG_TERM(m) ((m)->term)
#define RAFT_MSG_BODY(m) ((m)->body)
#define RAFT_MSG_IS_SYNC(m) ((m)->isSync)

SRaftMsg *traftNewMsg(uint8_t type, size_t size);
void      traftFreeMsg(SRaftMsg *pMsg);

typedef struct SRaftReq {
  // TODO
} SRaftReq;

typedef struct SRaftRsp {
  // TODO
} SRaftRsp;

#ifdef __cplusplus
}
#endif

#endif /*_TD_TMSG_H_*/