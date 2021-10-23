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

#ifndef TD_RAFT_H
#define TD_RAFT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "taosdef.h"

typedef unsigned int RaftId;
typedef unsigned int RaftGroupId;

// buffer holding data
typedef struct RaftBuffer {
  void* data;
  size_t len;
} RaftBuffer;

// a single server information in a cluster
typedef struct RaftServer {
  RaftId    id;
  char      fqdn[TSDB_FQDN_LEN];
  uint16_t  port;
} RaftServer;

// all servers in a cluster
typedef struct RaftConfiguration {
  RaftServer *servers;
  int nServer;
} RaftConfiguration;

// raft lib module
struct Raft;
typedef struct Raft Raft;

struct RaftNode;
typedef struct RaftNode RaftNode;

typedef struct RaftLogStore {
  void* data;

  int (*initLogStore)();

  int (*commitLog)(RaftIndex index);
  
  int (*loadLogFrom)(RaftIndex index, int limit, RaftBuffer** ppBuffer, int* n);

  int (*appendLog)(RaftBuffer* pBuffer, RaftIndex index);

  int (*removeLogBefore)(RaftIndex index);

  int (*removeLogAfter)(RaftIndex index);

} RaftLogStore;

// raft state machine
struct RaftFSM;
typedef struct RaftFSM {
  // statemachine user data
  void *data;

  // apply committed log, bufs will be free by raft module
  int (*applyLog)(struct RaftFSM *fsm, RaftIndex index, const RaftBuffer *buf, void *pData);

  // configuration commit callback 
  int (*onConfigurationCommit)(const RaftConfiguration* cluster, void *pData);

  // fsm return snapshot in ppBuf, bufs will be free by raft module
  // TODO: getSnapshot SHOULD be async?
  int (*getSnapshot)(struct RaftFSM *fsm, RaftBuffer **ppBuf, int* objId, bool *isLast);

  // fsm apply snapshot with pBuf data
  int (*applySnapshot)(struct RaftFSM *fsm, RaftBuffer *pBuf, int objId, bool isLast);

  // call when restore snapshot and log done
  int (*onRestoreDone)(struct RaftFSM *fsm);

  void (*onRollback)(struct RaftFSM *fsm, RaftIndex index, const RaftBuffer *buf);
π
  // fsm send data in buf to server,buf will be free by raft module
  int (*send)(struct RaftFSM* fsm, const RaftServer* server, const RaftBuffer *buf);
} RaftFSM;

typedef struct RaftNodeOptions {
  // user define state machine
  RaftFSM* pFSM;

  // log store, if it is NULL, use default implementation by WAL
  RaftLogStore* logStore;

  // election timeout(in ms)
  // by default: 1000
  int electionTimeoutMS;

  // heart timeout(in ms)
  // by default: 100
  int heartbeatTimeoutMS;

  // install snapshot timeout(in ms)
  int installSnapshotTimeoutMS;

  /** 
   * number of log entries before starting a new snapshot. 
   * by default: 1024
   */
  int snapshotThreshold;

  /**
   * Number of log entries to keep in the log after a snapshot has
   * been taken. 
   * by default: 128.
   */
  int snapshotTrailing;

  int maxSizePerMsg;

  int maxInflightMsgs;

  /**
   * Enable or disable pre-vote support.
   * by default: false
   */
  bool preVote;

} RaftNodeOptions;

// create raft lib
int RaftCreate(Raft** ppRaft);

int RaftDestroy(Raft* pRaft);

// start a raft node with options,node id,group id
int RaftStart(Raft* pRaft, 
              RaftId selfId,
              RaftGroupId selfGroupId,
              const RaftConfiguration* cluster,
              const RaftNodeOptions* options,              
              RaftNode **ppNode);

// stop a raft node
int RaftStop(RaftNode* pNode);

// client apply a cmd in buf
typedef void (*RaftApplyFp)(const RaftBuffer *pBuf, int result);

int RaftPropose(RaftNode *pNode,
              const RaftBuffer *pBuf,
              void *pData,
              bool isWeak);

// recv data from other servers in cluster,buf will be free in raft
int RaftRecv(RaftNode *pNode, const RaftBuffer* pBuf);

// change cluster servers API
typedef void (*RaftChangeFp)(const RaftServer* pServer, int result);

int RaftAddServer(RaftNode *pNode,
                  const RaftServer* pServer,
                  RaftChangeFp changeCb);

int RaftRemoveServer(RaftNode *pNode, 
                    const RaftServer* pServer,
                    void *pData,
                    RaftChangeFp changeCb);

// transfer leader to id
typedef void (*RaftTransferFp)(RaftId id, int result);
int RaftTransfer(RaftNode *pNode,
                 RaftId id,
                 void *pData,
                 RaftTransferFp transferCb);

#ifdef __cplusplus
}
#endif

#endif  /* TD_RAFT_H */ 