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

#ifndef _TD_LIBS_SYNC_RAFT_IMPL_H
#define _TD_LIBS_SYNC_RAFT_IMPL_H

#include "sync.h"
#include "sync_type.h"
#include "sync_raft_message.h"
#include "sync_raft_quorum.h"

void syncRaftBecomeFollower(SSyncRaft* pRaft, SyncTerm term, SyncNodeId leaderId);
void syncRaftBecomePreCandidate(SSyncRaft* pRaft);
void syncRaftBecomeCandidate(SSyncRaft* pRaft);
void syncRaftBecomeLeader(SSyncRaft* pRaft);

void syncRaftStartElection(SSyncRaft* pRaft, ESyncRaftElectionType cType);

void syncRaftCampaign(SSyncRaft* pRaft, ESyncRaftElectionType cType);

void syncRaftRandomizedElectionTimeout(SSyncRaft* pRaft);
bool syncRaftIsPromotable(SSyncRaft* pRaft);
bool syncRaftIsPastElectionTimeout(SSyncRaft* pRaft);

bool syncRaftMaybeCommit(SSyncRaft* pRaft);

int syncRaftStepLeader(SSyncRaft* pRaft, SSyncMessage* pMsg);
int syncRaftStepCandidate(SSyncRaft* pRaft, SSyncMessage* pMsg);
int syncRaftStepFollower(SSyncRaft* pRaft, SSyncMessage* pMsg);

int syncRaftSend(SSyncRaft* pRaft, SSyncMessage* pMsg, const SSyncNodeInfo* pNode);
bool syncRaftSendAppend(SSyncRaft* pRaft, SyncNodeId to);

// maybeSendAppend sends an append RPC with new entries to the given peer,
// if necessary. Returns true if a message was sent. The sendIfEmpty
// argument controls whether messages with no entries will be sent
// ("empty" messages are useful to convey updated Commit indexes, but
// are undesirable when we're sending multiple messages in a batch).
bool syncRaftMaybeSendAppend(SSyncRaft* pRaft, SyncNodeId to, bool sendIfEmpty);

ESyncRaftVoteResult  syncRaftPollVote(SSyncRaft* pRaft, SyncNodeId id, 
                                    bool preVote, bool accept, 
                                    int* rejectNum, int *granted);

static FORCE_INLINE bool syncRaftIsEmptyServerState(const SSyncServerState* serverState) {
  return serverState->commitIndex == 0 &&
         serverState->term == SYNC_NON_TERM &&
         serverState->voteFor == SYNC_NON_NODE_ID;
}

void syncRaftLoadState(SSyncRaft* pRaft, const SSyncServerState* serverState);

void syncRaftBroadcastAppend(SSyncRaft* pRaft);
void syncRaftBroadcastHeartbeat(SSyncRaft* pRaft);

bool syncRaftAppendEntry(SSyncRaft* pRaft, SSyncRaftEntry* entries, int n);

SSyncNodeInfo* syncRaftGetNodeById(SSyncRaft *pRaft, SyncNodeId id);

#endif /* _TD_LIBS_SYNC_RAFT_IMPL_H */
