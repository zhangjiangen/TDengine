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
#include "taosdef.h"
#include "tutil.h"
#include "tulog.h"
#include "tsched.h"
#include "ttimer.h"

#define DUMP_SCHEDULER_TIME_WINDOW 30000 //every 30sec, take a snap shot of task queue.

typedef struct {
  char            label[TSDB_LABEL_LEN];
  pthread_mutex_t queueMutex;
  pthread_cond_t  rsignal;
  pthread_cond_t  wsignal;

  int             front;
  int             rear;
  int             queueSize;

  int             numOfThreads;
  pthread_t *     qthread;
  SSchedMsg *     queue;
  bool            stop;
} SSchedQueue;

static void *taosProcessSchedQueue(void *param);
//static void taosDumpSchedulerStatus(void *qhandle, void *tmrId);

bool taosSchedQueueIsFull(void *param) {
  SSchedQueue *pSched = param; 
  return (pSched->rear + 1)% pSched->queueSize == pSched->front;
} 
bool taosSchedQueueIsEmpty(void *param) {
  SSchedQueue *pSched = param; 
  return pSched->front == pSched->rear;
}
void *taosInitScheduler(int queueSize, int numOfThreads, const char *label) {
  SSchedQueue *pSched = (SSchedQueue *)calloc(sizeof(SSchedQueue), 1);
  if (pSched == NULL) {
    uError("%s: no enough memory for pSched", label);
    return NULL;
  }

  pSched->queue = (SSchedMsg *)calloc(sizeof(SSchedMsg), queueSize);
  if (pSched->queue == NULL) {
    uError("%s: no enough memory for queue", label);
    taosCleanUpScheduler(pSched);
    return NULL;
  }

  pSched->qthread = calloc(sizeof(pthread_t), numOfThreads);
  if (pSched->qthread == NULL) {
    uError("%s: no enough memory for qthread", label);
    taosCleanUpScheduler(pSched);
    return NULL;
  }
  pSched->front     = 0;
  pSched->rear      = 0;
  pSched->queueSize = queueSize;
  tstrncpy(pSched->label, label, sizeof(pSched->label)); // fix buffer overflow

  if (pthread_mutex_init(&pSched->queueMutex, NULL) < 0) {
    uError("init %s:queueMutex failed(%s)", label, strerror(errno));
    taosCleanUpScheduler(pSched);
    return NULL;
  }
  if (pthread_cond_init(&pSched->rsignal, NULL) < 0) {
    uError("init %s:rsignal failed(%s)", label, strerror(errno));
    taosCleanUpScheduler(pSched);
  }
  if (pthread_cond_init(&pSched->wsignal, NULL) < 0) {
    uError("init %s:wsignal failed(%s)", label, strerror(errno));
    taosCleanUpScheduler(pSched);
  }

  pSched->stop = false;
  for (int i = 0; i < numOfThreads; ++i) {
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int code = pthread_create(pSched->qthread + i, &attr, taosProcessSchedQueue, (void *)pSched);
    pthread_attr_destroy(&attr);
    if (code != 0) {
      uError("%s: failed to create rpc thread(%s)", label, strerror(errno));
      taosCleanUpScheduler(pSched);
      return NULL;
    }
    ++pSched->numOfThreads;
  }

  uDebug("%s scheduler is initialized, numOfThreads:%d", label, pSched->numOfThreads);

  return (void *)pSched;
}

void *taosProcessSchedQueue(void *scheduler) {
  SSchedMsg    msg;
  SSchedQueue *pSched = (SSchedQueue *)scheduler;

  char name[16] = {0};
  snprintf(name, tListLen(name), "%s-taskQ", pSched->label);
  setThreadName(name);
  while (!pSched->stop) {
    pthread_mutex_lock(&pSched->queueMutex); 
    while (taosSchedQueueIsEmpty(pSched) && !pSched->stop) {
      pthread_cond_wait(&pSched->rsignal, &pSched->queueMutex); 
    }  
    if (pSched->stop == true) {
      pthread_mutex_unlock(&pSched->queueMutex); 
      break;
    } 
    msg = pSched->queue[pSched->front];  
    pSched->front =(pSched->front + 1)%pSched->queueSize;

    pthread_cond_signal(&pSched->wsignal);
    pthread_mutex_unlock(&pSched->queueMutex); 
    if (msg.fp)
      (*(msg.fp))(&msg);
    else if (msg.tfp)
      (*(msg.tfp))(msg.ahandle, msg.thandle);
  } 
  return NULL;
}

void taosScheduleTask(void *queueScheduler, SSchedMsg *pMsg) {
  SSchedQueue *pSched = (SSchedQueue *)queueScheduler;

  if (pSched == NULL) {
    uError("sched is not ready, msg:%p is dropped", pMsg);
    return;
  }
  pthread_mutex_lock(&pSched->queueMutex);
  while (taosSchedQueueIsFull(pSched) && !pSched->stop) {
    pthread_cond_wait(&pSched->wsignal, &pSched->queueMutex); 
  } 
  if (!pSched->stop) {
    pSched->queue[pSched->rear] = *pMsg; 
    pSched->rear = (pSched->rear + 1)%pSched->queueSize;
    pthread_cond_signal(&pSched->rsignal);
  }
  
  pthread_mutex_unlock(&pSched->queueMutex);
}

void taosCleanUpScheduler(void *param) {
  SSchedQueue *pSched = (SSchedQueue *)param;
  if (pSched == NULL) return;
  
  pSched->stop = true;

  pthread_cond_broadcast(&pSched->rsignal);
  pthread_cond_broadcast(&pSched->wsignal);
  for (int i = 0; i < pSched->numOfThreads; ++i) {
    pthread_join(pSched->qthread[i], NULL);
  }

  pthread_mutex_destroy(&pSched->queueMutex);
  pthread_cond_destroy(&pSched->wsignal);
  pthread_cond_destroy(&pSched->rsignal);
  
  if (pSched->queue) free(pSched->queue);
  if (pSched->qthread) free(pSched->qthread);
  free(pSched); // fix memory leak
}
