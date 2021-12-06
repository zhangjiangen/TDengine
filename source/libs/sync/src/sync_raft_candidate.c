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
#include "raft.h"
#include "syncInt.h"
#include "raft_message.h"
#include "sync_raft_progress_tracker.h"

int syncRaftStepCandidate(SSyncRaft* pRaft, SSyncMessage* pMsg) {
  /**
   * Only handle vote responses corresponding to our candidacy (while in
	 * StateCandidate, we may get stale MsgPreVoteResp messages in this term from
	 * our pre-candidate state).
   **/
  ESyncRaftMessageType msgType = pMsg->msgType;

  if (msgType == RAFT_MSG_INTERNAL_PROP) {
    syncInfo("[%d:%d]no leader at term %"PRId64", dropping proposal",
      pRaft->selfGroupId, pRaft->selfId, pRaft->term);
    return RAFT_PROPOSAL_DROPPED;
  }

  if (msgType == RAFT_MSG_VOTE_RESP) {
    syncRaftHandleVoteRespMessage(pRaft, pMsg);    
  } else if (msgType == RAFT_MSG_APPEND) {
    syncRaftBecomeFollower(pRaft, pMsg->term, pMsg->from);
    syncRaftHandleAppendEntriesMessage(pRaft, pMsg);
  }
  return RAFT_OK;
}