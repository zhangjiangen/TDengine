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

static int acceptAppendEntriesResp(SSyncRaft* pRaft, const SSyncMessage* pMsg,SSyncRaftProgress* progress);
static int rejectAppendEntriesResp(SSyncRaft* pRaft, const SSyncMessage* pMsg,SSyncRaftProgress* progress);
static void releasePendingReadIndexMessages(SSyncRaft* pRaft);

int syncRaftHandleAppendEntriesRespMessage(SSyncRaft* pRaft, const SSyncMessage* pMsg,SSyncRaftProgress* progress) {
  syncRaftProgressRecentActive(progress);
  const RaftMsg_Append_Resp* resp = &(pMsg->appendResp);
  if (resp->reject) {
    return rejectAppendEntriesResp(pRaft, pMsg, progress);
  } else {
    return acceptAppendEntriesResp(pRaft, pMsg, progress);
  }
}

static int acceptAppendEntriesResp(SSyncRaft* pRaft, const SSyncMessage* pMsg,SSyncRaftProgress* progress) {
  bool oldPaused = syncRaftProgressIsPaused(progress);
  const RaftMsg_Append_Resp* resp = &(pMsg->appendResp);
  if (!syncRaftProgressMaybeUpdate(progress, resp->index)) {
		return RAFT_OK;
	}

  if (progress->state == PROGRESS_STATE_PROBE) {
    syncRaftProgressBecomeReplicate(progress);
  } else if (progress->state == PROGRESS_STATE_SNAPSHOT && progress->matchIndex >= progress->pendingSnapshotIndex) {
    // received that is below pr.PendingSnapshot but which makes it
    // possible to use the log again.
    syncDebug("[%d:%d]recovered from needing snapshot, resumed sending replication messages to %d",
        pRaft->selfGroupId, pRaft->selfId, pMsg->from);
    // Transition back to replicating state via probing state
    // (which takes the snapshot into account). If we didn't
    // move to replicating state, that would only happen with
    // the next round of appends (but there may not be a next
    // round for a while, exposing an inconsistent RaftStatus).
    syncRaftProgressBecomeProbe(progress);
    syncRaftProgressBecomeReplicate(progress);
  } else if (progress->state == PROGRESS_STATE_REPLICATE) {
    syncRaftInflightFreeLE(progress->inflights, resp->index);
  }

	if (syncRaftMaybeCommit(pRaft)) {
    // committed index has progressed for the term, so it is safe
    // to respond to pending read index requests
		releasePendingReadIndexMessages(pRaft);
		syncRaftBroadcastAppend(pRaft);
	} else if (oldPaused) {
    // If we were paused before, this node may be missing the
    // latest commit index, so send it.
		syncRaftSendAppend(pRaft, pMsg->from);
	}

  // We've updated flow control information above, which may
  // allow us to send multiple (size-limited) in-flight messages
  // at once (such as when transitioning from probe to
  // replicate, or when freeTo() covers multiple messages). If
  // we have more entries to send, send as many messages as we
  // can (without sending empty messages for the commit index)
	while (syncRaftMaybeSendAppend(pRaft, pMsg->from, false)) {
	}

	// Transfer leadership is in progress.
	// TODO
}

static int rejectAppendEntriesResp(SSyncRaft* pRaft, const SSyncMessage* pMsg,SSyncRaftProgress* progress) {
  // RejectHint is the suggested next base entry for appending (i.e.
  // we try to append entry RejectHint+1 next), and LogTerm is the
  // term that the follower has at index RejectHint. Older versions
  // of this library did not populate LogTerm for rejections and it
  // is zero for followers with an empty log.
  //
  // Under normal circumstances, the leader's log is longer than the
  // follower's and the follower's log is a prefix of the leader's
  // (i.e. there is no divergent uncommitted suffix of the log on the
  // follower). In that case, the first probe reveals where the
  // follower's log ends (RejectHint=follower's last index) and the
  // subsequent probe succeeds.
  //
  // However, when networks are partitioned or systems overloaded,
  // large divergent log tails can occur. The naive attempt, probing
  // entry by entry in decreasing order, will be the product of the
  // length of the diverging tails and the network round-trip latency,
  // which can easily result in hours of time spent probing and can
  // even cause outright outages. The probes are thus optimized as
  // described below.
  const RaftMsg_Append_Resp* resp = &(pMsg->appendResp);

	syncDebug("[%d:%d]received MsgAppResp(rejected, hint: (index %"PRId64", term %"PRId64")) from %d for index %"PRId64"",
		pRaft->selfGroupId, pRaft->selfId, resp->rejectHint, resp->logTerm, pMsg->from, resp->index);
  SyncIndex nextProbeIndex = resp->rejectHint;
  if (resp->logTerm > 0) {
    // If the follower has an uncommitted log tail, we would end up
    // probing one by one until we hit the common prefix.
    //
    // For example, if the leader has:
    //
    //   idx        1 2 3 4 5 6 7 8 9
    //              -----------------
    //   term (L)   1 3 3 3 5 5 5 5 5
    //   term (F)   1 1 1 1 2 2
    //
    // Then, after sending an append anchored at (idx=9,term=5) we
    // would receive a RejectHint of 6 and LogTerm of 2. Without the
    // code below, we would try an append at index 6, which would
    // fail again.
    //
    // However, looking only at what the leader knows about its own
    // log and the rejection hint, it is clear that a probe at index
    // 6, 5, 4, 3, and 2 must fail as well:
    //
    // For all of these indexes, the leader's log term is larger than
    // the rejection's log term. If a probe at one of these indexes
    // succeeded, its log term at that index would match the leader's,
    // i.e. 3 or 5 in this example. But the follower already told the
    // leader that it is still at term 2 at index 9, and since the
    // log term only ever goes up (within a log), this is a contradiction.
    //
    // At index 1, however, the leader can draw no such conclusion,
    // as its term 1 is not larger than the term 2 from the
    // follower's rejection. We thus probe at 1, which will succeed
    // in this example. In general, with this approach we probe at
    // most once per term found in the leader's log.
    //
    // There is a similar mechanism on the follower (implemented in
    // handleAppendEntries via a call to findConflictByTerm) that is
    // useful if the follower has a large divergent uncommitted log
    // tail[1], as in this example:
    //
    //   idx        1 2 3 4 5 6 7 8 9
    //              -----------------
    //   term (L)   1 3 3 3 3 3 3 3 7
    //   term (F)   1 3 3 4 4 5 5 5 6
    //
    // Naively, the leader would probe at idx=9, receive a rejection
    // revealing the log term of 6 at the follower. Since the leader's
    // term at the previous index is already smaller than 6, the leader-
    // side optimization discussed above is ineffective. The leader thus
    // probes at index 8 and, naively, receives a rejection for the same
    // index and log term 5. Again, the leader optimization does not improve
    // over linear probing as term 5 is above the leader's term 3 for that
    // and many preceding indexes; the leader would have to probe linearly
    // until it would finally hit index 3, where the probe would succeed.
    //
    // Instead, we apply a similar optimization on the follower. When the
    // follower receives the probe at index 8 (log term 3), it concludes
    // that all of the leader's log preceding that index has log terms of
    // 3 or below. The largest index in the follower's log with a log term
    // of 3 or below is index 3. The follower will thus return a rejection
    // for index=3, log term=3 instead. The leader's next probe will then
    // succeed at that index.
    //
    // [1]: more precisely, if the log terms in the large uncommitted
    // tail on the follower are larger than the leader's. At first,
    // it may seem unintuitive that a follower could even have such
    // a large tail, but it can happen:
    //
    // 1. Leader appends (but does not commit) entries 2 and 3, crashes.
    //   idx        1 2 3 4 5 6 7 8 9
    //              -----------------
    //   term (L)   1 2 2     [crashes]
    //   term (F)   1
    //   term (F)   1
    //
    // 2. a follower becomes leader and appends entries at term 3.
    //              -----------------
    //   term (x)   1 2 2     [down]
    //   term (F)   1 3 3 3 3
    //   term (F)   1
    //
    // 3. term 3 leader goes down, term 2 leader returns as term 4
    //    leader. It commits the log & entries at term 4.
    //
    //              -----------------
    //   term (L)   1 2 2 2
    //   term (x)   1 3 3 3 3 [down]
    //   term (F)   1
    //              -----------------
    //   term (L)   1 2 2 2 4 4 4
    //   term (F)   1 3 3 3 3 [gets probed]
    //   term (F)   1 2 2 2 4 4 4
    //
    // 4. the leader will now probe the returning follower at index
    //    7, the rejection points it at the end of the follower's log
    //    which is at a higher log term than the actually committed
    //    log.
		syncDebug("[]");
    nextProbeIndex = syncRaftLogFindConflictByTerm(pRaft->log, resp->rejectHint, resp->logTerm);
  }
  if (syncRaftProgressMaybeDecrTo(progress, resp->index, nextProbeIndex)) {
		syncDebug("[%d:%d]decreased progress of %d to %d",
			pRaft->selfGroupId, pRaft->selfId, pMsg->from, progress->id);
		if (progress->state == PROGRESS_STATE_REPLICATE) {
			syncRaftProgressBecomeProbe(progress);
		}
		syncRaftSendAppend(pRaft, pMsg->from);
  }
}

// TODO
static void releasePendingReadIndexMessages(SSyncRaft* pRaft) {

}
