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
#include "tmsg.h"

struct SRaftServer {
  // TODO
};

static struct SRaftServer gRaftServer;
int (*msg_process_fn)(SRaftHandle *pRafth, SRaftMsg *pMsg);
static msg_process_fn msg_vtable[RAFT_MAX_MSGS] = {
    NULL,  // client_req
    NULL,  // append_entries_req,
    NULL   // append_entries_rsp,
};

int traftInit() {
  // TODO
  return 0;
}

int traftClear() {
  // TODO
  return 0;
}

int traftProcessMsg(SRaftHandle *pRafth, SRaftMsg *pMsg) {
  int code = (*msg_vtable[RAFT_MSG_TYPE(pMsg)])(pRafth, pMsg);
  // TODO
  return 0;
}