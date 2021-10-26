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

#include "raft.h"

int handleVoteMessage(Raft* raft, const RaftMessage* msg) {
  RaftMsgVote* pVoteMsg = &(msg->vote);

	/** 
   * The m.Term > r.Term clause is for MsgPreVote. For MsgVote m.Term should
	 *always equal r.Term.
   **/
  if ((raft->votedFor == 0 || pVoteMsg->term > raft->currentTerm || raft->voted == pMsg->from) &&
    )
  return RAFT_OK;
}