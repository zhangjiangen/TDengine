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
#include "raft_message.h"

int syncRaftStepLeader(SSyncRaft* pRaft, const SSyncMessage* pMsg) {
  // These message types do not require any progress for m.From.
  ESyncRaftMessageType msgType = pMsg->msgType;
  if (msgType == RAFT_MSG_INTERNAL_BEAT) {
    syncRaftBroadcastHeartbeat(pRaft);
    return 0;
  }

  if (msgType == RAFT_MSG_INTERNAL_PROP) {
    return syncRaftHandlePropMessage(pRaft, pMsg);
  }

  // All other message types require a progress for m.From (pr).
  
  return 0;
}