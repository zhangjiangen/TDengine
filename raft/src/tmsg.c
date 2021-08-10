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

SRaftMsg *traftNewMsg(uint8_t type, size_t size) {
  SRaftMsg *pMsg;

  size_t tsize = sizeof(SRaftMsg) + size + (type == RAFT_MSG_REQ) ? sizeof(SRaftReq) : sizeof(SRaftRsp);
  if ((pMsg = malloc(tsize)) == NULL) {
    // TODO
    return NULL;
  }

  return pMsg;
}

void traftFreeMsg(SRaftMsg *pMsg) {
  if (pMsg) {
    free(pMsg);
  }
}