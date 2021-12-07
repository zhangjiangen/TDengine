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

#include "sync_raft.h"
#include "sync_raft_impl.h"
#include "sync_raft_log.h"
#include "sync_raft_progress_tracker.h"
#include "syncInt.h"

static int convertClear(SSyncRaft* pRaft);

static bool increaseUncommittedSize(SSyncRaft* pRaft, SSyncRaftEntry* entries, int n);

static void tickElection(SSyncRaft* pRaft);
static void tickHeartbeat(SSyncRaft* pRaft);

static void abortLeaderTransfer(SSyncRaft* pRaft);

static void resetRaft(SSyncRaft* pRaft, SyncTerm term);

void syncRaftBecomeFollower(SSyncRaft* pRaft, SyncTerm term, SyncNodeId leaderId) {
  convertClear(pRaft);

  pRaft->stepFp = syncRaftStepFollower;
  resetRaft(pRaft, term);
  pRaft->tickFp = tickElection;
  pRaft->leaderId = leaderId;
  pRaft->state = TAOS_SYNC_STATE_FOLLOWER;
  syncInfo("[%d:%d] became followe at term %" PRId64 "", pRaft->selfGroupId, pRaft->selfId, pRaft->term);
}

void syncRaftBecomePreCandidate(SSyncRaft* pRaft) {
  convertClear(pRaft);

  if (pRaft->state == TAOS_SYNC_STATE_LEADER) {
    syncFatal("[%d:%d]invalid transition [leader -> candidate]", pRaft->selfGroupId, pRaft->selfId);
  }
	/**
   * Becoming a pre-candidate changes our step functions and state,
	 * but doesn't change anything else. In particular it does not increase
	 * r.Term or change r.Vote.
   **/
  pRaft->stepFp = syncRaftStepCandidate;
  pRaft->tickFp = tickElection;
  pRaft->state  = TAOS_SYNC_STATE_CANDIDATE;
  pRaft->candidateState.inPreVote = true;
  syncInfo("[%d:%d] became pre-candidate at term %" PRId64 "", pRaft->selfGroupId, pRaft->selfId, pRaft->term);
}

void syncRaftBecomeCandidate(SSyncRaft* pRaft) {
  convertClear(pRaft);
  if (pRaft->state == TAOS_SYNC_STATE_LEADER) {
    syncFatal("[%d:%d]invalid transition [leader -> candidate]", pRaft->selfGroupId, pRaft->selfId);
  }
  pRaft->candidateState.inPreVote = false;
  pRaft->stepFp = syncRaftStepCandidate;
  // become candidate make term+1
  resetRaft(pRaft, pRaft->term + 1);
  pRaft->tickFp = tickElection;
  pRaft->voteFor = pRaft->selfId;
  pRaft->state  = TAOS_SYNC_STATE_CANDIDATE;
  syncInfo("[%d:%d] became candidate at term %" PRId64 "", pRaft->selfGroupId, pRaft->selfId, pRaft->term);
}

void syncRaftBecomeLeader(SSyncRaft* pRaft) {
  assert(pRaft->state != TAOS_SYNC_STATE_FOLLOWER);

  pRaft->stepFp = syncRaftStepLeader;
  resetRaft(pRaft, pRaft->term);
  pRaft->leaderId = pRaft->leaderId;
  pRaft->state  = TAOS_SYNC_STATE_LEADER;

  SSyncRaftProgress* progress = syncRaftFindProgressByNodeId(&pRaft->tracker->progressMap, pRaft->selfId);
  assert(progress != NULL);
	// Followers enter replicate mode when they've been successfully probed
	// (perhaps after having received a snapshot as a result). The leader is
	// trivially in this state. Note that r.reset() has initialized this
	// progress with the last index already.  
  syncRaftProgressBecomeReplicate(progress);

	// Conservatively set the pendingConfIndex to the last index in the
	// log. There may or may not be a pending config change, but it's
	// safe to delay any future proposals until we commit all our
	// pending log entries, and scanning the entire tail of the log
	// could be expensive.
  SyncIndex lastIndex = syncRaftLogLastIndex(pRaft->log);
  pRaft->pendingConfigIndex = lastIndex;

  // after become leader, send a no-op log
  SSyncRaftEntry emptyEntry = (SSyncRaftEntry) {
    .buffer = (SSyncBuffer) {
      .data = NULL,
      .len  = 0,
    },
    .index = 0,
    .term = 0,
    .type = SYNC_ENTRY_TYPE_LOG,
  };
  syncRaftAppendEntry(pRaft, &emptyEntry, 1);

  syncInfo("[%d:%d] became leader at term %" PRId64 "", pRaft->selfGroupId, pRaft->selfId, pRaft->term);
}

void syncRaftRandomizedElectionTimeout(SSyncRaft* pRaft) {
  // electionTimeoutTick in [3,6] tick
  pRaft->randomizedElectionTimeout = taosRand() % 4 + 3;
}

bool syncRaftIsPromotable(SSyncRaft* pRaft) {
  return pRaft->selfId != SYNC_NON_NODE_ID;
}

bool syncRaftIsPastElectionTimeout(SSyncRaft* pRaft) {
  return pRaft->electionElapsed >= pRaft->randomizedElectionTimeout;
}

ESyncRaftVoteResult  syncRaftPollVote(SSyncRaft* pRaft, SyncNodeId id, 
                                      bool preVote, bool grant, 
                                      int* rejected, int *granted) {
  SNodeInfo* pNode = syncRaftGetNodeById(pRaft, id);
  if (pNode == NULL) {
    return true;
  }

  if (grant) {
    syncInfo("[%d:%d] received grant (pre-vote %d) from %d at term %" PRId64 "", 
      pRaft->selfGroupId, pRaft->selfId, preVote, id, pRaft->term);
  } else {
    syncInfo("[%d:%d] received rejection (pre-vote %d) from %d at term %" PRId64 "", 
      pRaft->selfGroupId, pRaft->selfId, preVote, id, pRaft->term);
  }

  syncRaftRecordVote(pRaft->tracker, pNode->nodeId, grant);
  return syncRaftTallyVotes(pRaft->tracker, rejected, granted);
}

void syncRaftLoadState(SSyncRaft* pRaft, const SSyncServerState* serverState) {
  SyncIndex commitIndex = serverState->commitIndex;
  SyncIndex lastIndex = syncRaftLogLastIndex(pRaft->log);
  SyncIndex logCommitIndex = syncRaftLogCommitIndex(pRaft->log);

  if (commitIndex < logCommitIndex || commitIndex > lastIndex) {
    syncFatal("[%d:%d] state.commit %"PRId64" is out of range [%" PRId64 ",%" PRId64 "",
      pRaft->selfGroupId, pRaft->selfId, commitIndex, logCommitIndex, lastIndex);
    return;
  }

  syncSetRaftLogCommitIndex(pRaft->log, commitIndex);
  pRaft->term = serverState->term;
  pRaft->voteFor = serverState->voteFor;
}

// syncRaftSendAppend sends an append RPC with new entries (if any) and the
// current commit index to the given peer.
bool syncRaftSendAppend(SSyncRaft* pRaft, SyncNodeId to) {
  syncRaftMaybeSendAppend(pRaft, to, true);
}

static void visitProgressSendAppend(SSyncRaftProgress* progress, void* arg) {
  SSyncRaft* pRaft = (SSyncRaft*)arg;
  if (pRaft->selfId == progress->id) {
    return;
  }
  
  syncRaftSendAppend(pRaft, progress->id);
}

// bcastAppend sends RPC, with entries to all peers that are not up-to-date
// according to the progress recorded in r.prs.
void syncRaftBroadcastAppend(SSyncRaft* pRaft) {
  syncRaftProgressVisit(pRaft->tracker, visitProgressSendAppend, pRaft);
}

void syncRaftBroadcastHeartbeat(SSyncRaft* pRaft) {

}

SNodeInfo* syncRaftGetNodeById(SSyncRaft *pRaft, SyncNodeId id) {
  SNodeInfo **ppNode = taosHashGet(pRaft->nodeInfoMap, &id, sizeof(SyncNodeId*));
  if (ppNode != NULL) {
    return *ppNode;
  }

  return NULL;
}

static int convertClear(SSyncRaft* pRaft) {

}

// tickElection is run by followers and candidates after r.electionTimeout.
static void tickElection(SSyncRaft* pRaft) {
  pRaft->electionElapsed += 1;

  if (!syncRaftIsPromotable(pRaft)) {
    return;
  }

  if (!syncRaftIsPastElectionTimeout(pRaft)) {
    return;
  }

  // election timeout
  pRaft->electionElapsed = 0;
  SSyncMessage msg;
  syncRaftStep(pRaft, syncInitElectionMsg(&msg, pRaft->selfId));
}

// tickHeartbeat is run by leaders to send a MsgBeat after r.heartbeatTimeout.
static void tickHeartbeat(SSyncRaft* pRaft) {
  pRaft->heartbeatElapsed++;
  pRaft->electionElapsed++;

  if (pRaft->electionElapsed >= pRaft->electionTimeout) {
    pRaft->electionElapsed = 0;
    if (pRaft->checkQuorum) {
      // TODO
    }
    // If current leader cannot transfer leadership in electionTimeout, it becomes leader again.
    if (pRaft->state == TAOS_SYNC_STATE_LEADER && pRaft->leadTransferee != SYNC_NON_NODE_ID) {
      abortLeaderTransfer(pRaft);
    }
  }

  if (pRaft->state != TAOS_SYNC_STATE_LEADER) {
    return;
  }

  if (pRaft->heartbeatElapsed >= pRaft->heartbeatTimeout) {
    pRaft->heartbeatElapsed = 0;
    SSyncMessage msg;
    syncInitBeatMsg(&msg, pRaft->selfId);
    syncRaftStep(pRaft, &msg);
  }
}

// TODO
static bool increaseUncommittedSize(SSyncRaft* pRaft, SSyncRaftEntry* entries, int n) {
  return false;
}

bool syncRaftAppendEntry(SSyncRaft* pRaft, SSyncRaftEntry* entries, int n) {
  SyncIndex lastIndex = syncRaftLogLastIndex(pRaft->log);
  SyncTerm term = pRaft->term;
  int i;

  for (i = 0; i < n; ++i) {
    entries[i].term = term;
    entries[i].index = lastIndex + 1 + i;
  }

  // Track the size of this uncommitted proposal.
  if (!increaseUncommittedSize(pRaft, entries, n)) {
    // Drop the proposal.
    return false;    
  }

  // use latest "last" index after truncate/append
  lastIndex = syncRaftLogAppend(pRaft->log, entries, n);

  SSyncRaftProgress* progress = syncRaftFindProgressByNodeId(&pRaft->tracker->progressMap, pRaft->selfId);
  assert(progress != NULL);
  syncRaftProgressMaybeUpdate(progress, lastIndex);
  // Regardless of syncRaftMaybeCommit's return, our caller will call bcastAppend.
  syncRaftMaybeCommit(pRaft);

  return true;
}

// syncRaftMaybeCommit attempts to advance the commit index. Returns true if
// the commit index changed (in which case the caller should call
// r.bcastAppend).
bool syncRaftMaybeCommit(SSyncRaft* pRaft) {
  SyncIndex commitIndex = syncRaftCommittedIndex(pRaft->tracker);
  syncRaftLogMaybeCommit(pRaft->log, commitIndex, pRaft->term);
  return true;
}

// send schedules persisting state to a stable storage and AFTER that
// sending the message (as part of next Ready message processing).
int syncRaftSend(SSyncRaft* pRaft, SSyncMessage* pMsg, const SNodeInfo* pNode) {
  if (pMsg->from == SYNC_NON_NODE_ID) {
    pMsg->from = pRaft->selfId;
  }
  ESyncRaftMessageType msgType = pMsg->msgType;
  if (msgType == RAFT_MSG_VOTE || msgType == RAFT_MSG_VOTE_RESP || syncIsPreVoteRespMsg(pMsg)) {
		// All {pre-,}campaign messages need to have the term set when
		// sending.
		// - MsgVote: m.Term is the term the node is campaigning for,
		//   non-zero as we increment the term when campaigning.
		// - MsgVoteResp: m.Term is the new r.Term if the MsgVote was
		//   granted, non-zero for the same reason MsgVote is
		// - MsgPreVote: m.Term is the term the node will campaign,
		//   non-zero as we use m.Term to indicate the next term we'll be
		//   campaigning for
		// - MsgPreVoteResp: m.Term is the term received in the original
		//   MsgPreVote if the pre-vote was granted, non-zero for the
		//   same reasons MsgPreVote is
    if (pMsg->term == SYNC_NON_TERM) {
      syncFatal("[%d:%d]term should be set when sending vote msg", pRaft->selfGroupId, pRaft->selfId);
    }
  } else {
    if (pMsg->term != SYNC_NON_TERM) {
      syncFatal("[%d:%d]term should be not set when sending %s msg", 
        pRaft->selfGroupId, pRaft->selfId, gMsgString[pMsg->msgType]);
    }
		// do not attach term to MsgProp, MsgReadIndex
		// proposals are a way to forward to the leader and
		// should be treated as local message.
		// MsgReadIndex is also forwarded to leader.
    if (pMsg->msgType != RAFT_MSG_INTERNAL_PROP && pMsg->msgType != RAFT_MSG_READ_INDEX) {
      pMsg->term = pRaft->term;
    }
  }

  return pRaft->io.send(pMsg, pNode);
}

static void abortLeaderTransfer(SSyncRaft* pRaft) {
  pRaft->leadTransferee = SYNC_NON_NODE_ID;
}

static void resetProgress(SSyncRaftProgress* progress, void* arg) {
  syncRaftResetProgress((SSyncRaft*)arg, progress);
}

static void resetRaft(SSyncRaft* pRaft, SyncTerm term) {
  if (pRaft->term != term) {
    pRaft->term = term;
    pRaft->voteFor = SYNC_NON_NODE_ID;
  }

  pRaft->leaderId = SYNC_NON_NODE_ID;

  pRaft->electionElapsed = 0;
  pRaft->heartbeatElapsed = 0;

  syncRaftRandomizedElectionTimeout(pRaft);

  abortLeaderTransfer(pRaft);

  syncRaftResetVotes(pRaft->tracker);
  syncRaftProgressVisit(pRaft->tracker, resetProgress, pRaft);

  pRaft->pendingConfigIndex = 0;
  pRaft->uncommittedSize = 0;
}