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
#include "raft.h"

static int sendMsg(RaftNode *pNode, RaftMessage* pMsg);
static int handleMsg(Raft* raft, RaftMessage* pMsg);
static int prehandleOutterMsg(Raft* raft, RaftMessage* pMsg);

int RaftCreate(Raft** ppRaft) {
  *ppRaft = (Raft*)malloc(sizeof(Raft));

  return *ppRaft != NULL ? 0 : -1;
}

int RaftDestroy(Raft* pRaft) {
  free(pRaft);

  return 0;
}

int RaftApply(RaftNode *pNode,
              const SSyncBuffer *pBuf,
              RaftApplyFp applyCb) {
  RaftMessage* pMsg = (RaftMessage*)malloc(sizeof(RaftMessage));
  if (pMsg == NULL) {
    return RAFT_OOM;
  }

  pMsg->msgType = RAFT_MSG_INTERNAL_PROP;
  pMsg->from = pNode->selfId;

  pMsg->propose.pBuf = pBuf;
  pMsg->propose.applyCb = applyCb;

  return sendMsg(pNode, pMsg);
}

static int sendMsg(RaftNode *pNode, RaftMessage* pMsg) {

}

static int handleMsg(Raft* raft, RaftMessage* pMsg) {
  raftDebug("from");
  RaftMessageType msgType = pMsg->msgType;
  RaftId from = pMsg->from;
  RaftId to = pMsg->to;
  SSyncTerm term = pMsg->term;
  RaftId leader;
  int err;

  if (!isInternalMessage(msgType)) {
    err = prehandleOutterMsg(raft, pMsg);
    if (err != RAFT_OK) {
      return err;
    }
  }

  if (isVoteMsg(msgType)) {
    return handleVoteMessage(raft, pMsg);
  }

  return raft->stepFp(raft, pMsg);
}

static int prehandleOutterMsg(Raft* raft, RaftMessage* pMsg) {
  SSyncTerm term = pMsg->term;
  RaftId from = pMsg->from;
  RaftMessageType msgType = pMsg->msgType;
  RaftId leader;

  if (term > raft->currentTerm) {
    leader = from;
    if (isVoteMsg(msgType)) {

    }
  } else if (term < raft->currentTerm) {
    
    return RAFT_IGNORED;
  }

  return RAFT_OK;
}