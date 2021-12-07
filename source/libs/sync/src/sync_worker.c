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

#include "os.h"
#include "sync_const.h"
#include "syncInt.h"
#include "sync_raft_message.h"
#include "sync_worker.h"
#include "tqueue.h"
#include "taoserror.h"
#include "tworker.h"

typedef struct SSyncWorker {
  bool inited;

  SMWorkerPool *pWorkerPool;

  taos_queue writeQueue;
} SSyncWorker;

struct SSyncWorkerPool {
  SMWorkerPool workerPool;

  SSyncWorker* worker;
  int nWorker;
};

static void syncProcessSendMsg(void *unused, taos_qall qall, int32_t numOfMsgs);
static int initSyncWorker(SMWorkerPool *pWorkerPool, SSyncWorker* pWorker);

SSyncWorkerPool* syncOpenWorkerPool(int nWorker) {
  int i, ret = 0;

  SSyncWorkerPool* pPool = (SSyncWorkerPool*)malloc(sizeof(SSyncWorkerPool));
  if (pPool == NULL) {
    return NULL;
  }
  pPool->worker = (SSyncWorker*)malloc(nWorker * sizeof(SSyncWorker));
  if (pPool->worker == NULL) {
    return NULL;
  }
  pPool->nWorker = nWorker;
  SMWorkerPool *pWorkerPool = &pPool->workerPool;
  pWorkerPool->name = "sync-send-worker";
  pWorkerPool->max = nWorker;
  ret = tMWorkerInit(pWorkerPool);
  if (ret != 0) {
    return NULL;
  }

  for (i = 0; i < nWorker; ++i) {
    SSyncWorker* pWorker = &(pPool->worker[i]);
    if (initSyncWorker(pWorkerPool, pWorker) != 0) {
      ret = -1;
      break;
    }
  }

  if (ret != 0) {
    syncCloseWorkerPool(pPool);
    return NULL;
  }
  return pPool;
}

void syncCloseWorkerPool(SSyncWorkerPool* pPool) {
  int i;

  for (i = 0; i < pPool->nWorker; ++i) {
    SSyncWorker* pWorker = &(pPool->worker[i]);
    if (pWorker->inited) {
      tMWorkerFreeQueue(&pPool->workerPool, pWorker->writeQueue);   
    }    
  }

  tMWorkerCleanup(&pPool->workerPool);
  free(pPool->worker);
  free(pPool);
}

void syncWorkerSendMessage(SSyncWorkerPool* pPool, const SSyncMessage* pMsg, const SSyncNodeInfo* pNode) {
  // dispatch msg to worker using node hash
  int i = pNode->hash % pPool->nWorker;
  SSyncWorker* pWorker = &(pPool->worker[i]);

  int code = taosWriteQitem(pWorker->writeQueue, (void*)pMsg);
  if (code != TSDB_CODE_SUCCESS) {    
    syncFreeMessage(pMsg);
    syncError("taosWriteQitem to queue fail");
  }
}

static void syncProcessSendMsg(void *pArg, taos_qall qall, int32_t numOfMsgs) {
  SSyncWorker* pWorker = (SSyncWorker*)pArg;
  const SSyncMessage* pMsg = NULL;

  for (int32_t i = 0; i < numOfMsgs; ++i) {
    taosGetQitem(qall, (void **)&pMsg);
    syncFreeMessage(pMsg);
  }

}

static int initSyncWorker(SMWorkerPool *pWorkerPool, SSyncWorker* pWorker) {
  pWorker->inited = false;

  pWorker->writeQueue = tMWorkerAllocQueue(pWorkerPool, pWorker, (FProcessItems)syncProcessSendMsg);
  if (pWorker->writeQueue == NULL) {
    return -1;
  }

  pWorker->inited = true;

  return 0;
}