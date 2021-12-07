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
#include "sync_raft.h"
#include "sync_raft_log.h"
#include "sync_raft_impl.h"
#include "sync_raft_message.h"

int syncRaftHandleHeartbeat(SSyncRaft* pRaft, SSyncMessage* pMsg) {
  RaftMsg_Heartbeat* ht = (RaftMsg_Heartbeat*)(&pMsg->heartbeat);
  syncRaftLogCommitTo(pRaft->log, ht->commmitIndex);

  // TODO: resp heartbeat
  //syncRaftSend(pRaft, pRespMsg, pNode);

  return RAFT_OK;
}