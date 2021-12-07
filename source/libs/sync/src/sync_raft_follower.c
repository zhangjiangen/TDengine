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

#include "sync_raft_impl.h"
#include "sync_raft.h"
#include "syncInt.h"
#include "sync_raft_message.h"
#include "sync_raft_progress_tracker.h"

static int followerHandleProp(SSyncRaft* pRaft, SSyncMessage* pMsg);
static int followerHandleAppendEntries(SSyncRaft* pRaft, SSyncMessage* pMsg);
static int followerHandleHeartbeat(SSyncRaft* pRaft, SSyncMessage* pMsg);

int syncRaftStepFollower(SSyncRaft* pRaft, SSyncMessage* pMsg) {
  ESyncRaftMessageType msgType = pMsg->msgType;

  if (msgType == RAFT_MSG_INTERNAL_PROP) {
    return followerHandleProp(pRaft, pMsg);
  } 
  
  if (msgType == RAFT_MSG_APPEND) {
    return followerHandleAppendEntries(pRaft, pMsg);
  }

  if (msgType == RAFT_MSG_HEARTBEAT) {
    return followerHandleHeartbeat(pRaft, pMsg);
  }

  return RAFT_OK;
}

static int followerHandleProp(SSyncRaft* pRaft, SSyncMessage* pMsg) {
  if (pRaft->leaderId == SYNC_NON_NODE_ID) {
    syncInfo("[%d:%d]no leader at term %"PRId64", dropping proposal",
      pRaft->selfGroupId, pRaft->selfId, pRaft->term);

    return RAFT_PROPOSAL_DROPPED;
  } else if (pRaft->disableProposalForwarding) {
    syncInfo("[%d:%d]not forwarding to leader %d at term %"PRId64"; dropping proposal",
      pRaft->selfGroupId, pRaft->selfId, pRaft->leaderId, pRaft->term);

    return RAFT_PROPOSAL_DROPPED;
  }

  SSyncNodeInfo* pNode = syncRaftGetNodeById(pRaft, pRaft->leaderId);
  assert(pNode);
  syncRaftSend(pRaft, pMsg, pNode);
  return RAFT_OK;
}

static int followerHandleAppendEntries(SSyncRaft* pRaft, SSyncMessage* pMsg) {
  pRaft->electionElapsed = 0;
  pRaft->leaderId = pMsg->from;
  syncRaftHandleAppendEntriesMessage(pRaft, pMsg);

  return RAFT_OK;
}

static int followerHandleHeartbeat(SSyncRaft* pRaft, SSyncMessage* pMsg) {
  pRaft->electionElapsed = 0;
  pRaft->leaderId = pMsg->from;

  return RAFT_OK;
}