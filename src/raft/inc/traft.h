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

#ifndef _TD_RAFT_H_
#define _TD_RAFT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tlogStore.h"
#include "tmsg.h"

typedef struct SRaftHandle SRaftHandle;

// Module Init/Clear APIs
int traftInit();
int traftClear();

// For multi-raft

// Process Message request
int traftProcessMsg(SRaftHandle *pRafth, SRaftMsg *pMsg);

/*----------------Do NOT use definitions below----------------*/
struct SRaftHandle {
  uint64_t  term;
  SLogStore logStore;
};

#ifdef __cplusplus
}
#endif

#endif /*_TD_RAFT_H_*/