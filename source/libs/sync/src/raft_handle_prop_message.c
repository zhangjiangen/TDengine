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

#include "syncInt.h"
#include "raft.h"
#include "sync_raft_log.h"
#include "sync_raft_impl.h"
#include "raft_message.h"
#include "sync_raft_progress_tracker.h"

int syncRaftHandlePropMessage(SSyncRaft* pRaft, SSyncMessage* pMsg) {
  const RaftMsgInternal_Prop *propose = &(pMsg->propose);
  if (propose->pData == NULL) {

  }

  if (!syncRaftFindProgressByNodeId(&pRaft->tracker->progressMap, pRaft->selfId)) {
		// If we are not currently a member of the range (i.e. this node
		// was removed from the configuration while serving as leader),
		// drop any new proposals.
    return RAFT_PROPOSAL_DROPPED;
  }

  if (pRaft->leadTransferee != SYNC_NON_NODE_ID) {
    return RAFT_PROPOSAL_DROPPED;
  }

  SSyncRaftEntry entry = (SSyncRaftEntry) {
    .buffer = propose->pData,
    .type = propose->eType,
  };
  if (!syncRaftAppendEntry(pRaft, &entry, 1)) {
    return RAFT_PROPOSAL_DROPPED;
  }

  syncRaftBroadcastAppend(pRaft);

  return RAFT_OK;
}