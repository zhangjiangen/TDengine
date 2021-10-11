/*
 * Copyright (c) 2019 TAOS Data, Inc. <cli@taosdata.com>
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

#include "raft_configuration.h"
#include "raft_message.h"
#include "raft_impl.h"

extern int raftCreate(RaftNode *pNode, RaftCore** ppCore);
extern int raftStep(RaftCore* pCore, RaftMessage* pMsg);

static bool validateOptions(const RaftNodeOptions* options);
static void setDefaultOptions(RaftNodeOptions* options);
static void *nodeWorkerMain(void *pArgs);

int RaftStart(Raft* pRaft, 
              RaftId selfId,
              RaftGroupId selfGroupId,
              const RaftConfiguration* cluster,
              const RaftNodeOptions* options,              
              RaftNode **ppNode) {
  if (!validateOptions(options)) {
    return -1;
  }

  RaftNode *pNode = (RaftNode*)malloc(sizeof(RaftNode));
  if (pNode == NULL) {
    return -1;
  }

  if (raftConfigurationCopy(&pNode->cluster) < 0) {
    goto err;
  }

  pNode->options = *optios;

  // init log storage
  
  // init worker thread
  if (pthread_create(&pNode->thread, NULL, nodeWorkerMain, pNode) < 0) {
    goto err;
  }

  pNode->msgQueue = taosOpenQueue();
  if (pNode->msgQueue == NULL) {
    goto err;
  }

  pNode->msgQset = taosOpenQset();
  if (pNode->msgQset == NULL) {
    taosCloseQueue(Node->msgQueue);
    goto err;
  }
  taosAddIntoQset(pNode->msgQset, pNode->msgQueue, NULL);

  pNode->msgQall = taosAllocateQall();
  if (pNode->msgQall == NULL) {
    taosCloseQset(pNode->msgQset);
    taosCloseQueue(Node->msgQueue);
    return -1;
  }

  return 0;

err:
  free(pNode->cluster);
  free(pNode);

  return -1;
}

// stop a raft node
int RaftStop(RaftNode* pNode) {
  return 0;
}

// internal static functions
static void *nodeWorkerMain(void *pArgs) {
  int32_t  qtype;
  void *   unUsed;
  RaftMessage *pMsg;
  RaftNode *pNode = (RaftNode*)pArgs;

  setThreadName("raftNodeWorker");

  while (1) {
    int32_t numOfMsgs = taosReadAllQitemsFromQset(pNode->msgQset, pNode->msgQall, &unUsed);
    if (numOfMsgs == 0) {
      break;
    }

    for (int32_t i = 0; i < numOfMsgs; ++i) {
      taosGetQitem(pNode->msgQall, &qtype, (void **)&pMsg);

      raftStep(pNode->raftCore, pMsg);
    }
  }
  return NULL;
}