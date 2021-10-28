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

#include "sync.h"
#include "raft_message.h"
#include "raft_type.h"
#include "tqueue.h"

typedef enum RaftCode {
  RAFT_OK = 0,

  /**
   * RAFT_INDEX_COMPACTED is returned when a request index is predates the last snapshot
   **/
  RAFT_INDEX_COMPACTED = -1,

  /**
   * RAFT_INDEX_UNAVAILABLE is returned when a request index is unavailable
   **/
  RAFT_INDEX_UNAVAILABLE = -2,

  /* out of memory */
  RAFT_OOM = -3,


  RAFT_IGNORED = -4,
} RaftCode;

/* raft log entry with reference count */
typedef struct RaftEntry {
  SSyncTerm term;
  SSyncBuffer buffer;
  unsigned int refCount;
} RaftEntry;

/* meta data about snapshot */
typedef struct RaftSnapshotMeta {
  SyncIndex lastIndex;
  SSyncTerm  lastTerm;
} RaftSnapshotMeta;

/* in-memory raft log storage */
typedef struct RaftLog {
  /* Circular buffer of log entries */
  RaftEntry *entries;

  /* size of Circular buffer */
  int size;

  /* Indexes of used slots [front, back) */
  int front, back;

  /* Index of first entry is offset + 1 */
  SyncIndex offset;

  /* meta data of snapshot */
  RaftSnapshotMeta snapshot;
} RaftLog;

typedef struct RaftIOMethods {
  RaftTime (*time)(Raft*);

} RaftIOMethods;

typedef struct RaftLeaderState {
  RaftProgress* progress;
} RaftLeaderState;

typedef int (*RaftStepFp)(Raft* raft, const RaftMessage* pMsg);

// raft core algorithm
struct Raft {
  RaftRole role;

	/** 
   * maxInflightMsgs limits the max number of in-flight append messages during
	 * optimistic replication phase. The application transportation layer usually
	 * has its own sending buffer over TCP/UDP. Setting MaxInflightMsgs to avoid
	 * overflowing that sending buffer. 
   **/
  int maxInflightMsgs;

  // user define state machine
  SSyncFSM fsm;

  SSyncServerState hardState;

  RaftStepFp stepFp;

  RaftLog*  log;

  RaftTime heartbeatTimeoutMS;

  RaftTime installSnapShotTimeoutMS;

  RaftLeaderState leaderState;
};

struct RaftNode {
  // the cluster configuration
  RaftConfiguration* cluster;

  // node options
  RaftNodeOptions options;

  RaftId selfId;

  // node worker thread
  pthread_t thread;

  // node worker message queue
  taos_qset  msgQset;
  taos_qall  msgQall;
  taos_queue msgQueue;

  // raft core algorithm
  Raft* raftCore;
};

struct Raft {
  RaftNode* nodes;
};

extern int32_t sDebugFlag;

#define raftFatal(...) do { if (sDebugFlag & DEBUG_FATAL) { taosPrintLog("RAFT FATAL ", 255, __VA_ARGS__); }}     while(0)
#define raftError(...) do { if (sDebugFlag & DEBUG_ERROR) { taosPrintLog("RAFT ERROR ", 255, __VA_ARGS__); }}     while(0)
#define raftWarn(...)  do { if (sDebugFlag & DEBUG_WARN)  { taosPrintLog("RAFT WARN ", 255, __VA_ARGS__); }}      while(0)
#define raftInfo(...)  do { if (sDebugFlag & DEBUG_INFO)  { taosPrintLog("RAFT ", 255, __VA_ARGS__); }}           while(0)
#define raftDebug(...) do { if (sDebugFlag & DEBUG_DEBUG) { taosPrintLog("RAFT ", sDebugFlag, __VA_ARGS__); }} while(0)
#define raftTrace(...) do { if (sDebugFlag & DEBUG_TRACE) { taosPrintLog("RAFT ", sDebugFlag, __VA_ARGS__); }} while(0)

#endif /* TD_RAFT_H */