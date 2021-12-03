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

int syncRaftHandleAppendEntriesMessage(SSyncRaft* pRaft, const SSyncMessage* pMsg) {
  const RaftMsg_Append_Entries *appendEntries = &(pMsg->appendEntries);
  
  SNodeInfo* pNode = syncRaftGetNodeById(pRaft, pMsg->from);
  if (pNode == NULL) {
    return RAFT_OK;
  }

  SSyncMessage* pRespMsg = syncNewEmptyAppendRespMsg(pRaft->selfGroupId, pRaft->selfId, pRaft->term);
  if (pRespMsg == NULL) {
    return RAFT_OK;
  }

  RaftMsg_Append_Resp *appendResp = &(pRespMsg->appendResp);
  // ignore committed logs
  if (syncRaftLogIsCommitted(pRaft->log, appendEntries->index)) {
    appendResp->index = syncRaftLogCommitIndex(pRaft->log);
    goto out;
  }

  SyncIndex lastIndex;
  bool ok = syncRaftLogMaybeAppend(pRaft->log, appendEntries->index, appendEntries->term, 
                                  appendEntries->commitIndex, appendEntries->entries, appendEntries->nEntries, &lastIndex);
  if (ok) {
    appendResp->index = lastIndex;
    appendResp->reject = false;
    goto out;
  } else {
    ESyncRaftCode err;
    SyncTerm logTerm = syncRaftLogTermOf(pRaft->log, appendEntries->index, &err);

    syncDebug("[%d:%d][logterm: %"PRId64", index: %"PRId64"] rejected MsgApp [logterm: %"PRId64", index: %"PRId64"] from %d",
      pRaft->selfGroupId, pRaft->selfId, syncRaftLogZeroTermOnErrCompacted(pRaft->log, logTerm, err),
      appendEntries->index, appendEntries->term, appendEntries->index, pMsg->from);

		// Return a hint to the leader about the maximum index and term that the
		// two logs could be divergent at. Do this by searching through the
		// follower's log for the maximum (index, term) pair with a term <= the
		// MsgApp's LogTerm and an index <= the MsgApp's Index. This can help
		// skip all indexes in the follower's uncommitted tail with terms
		// greater than the MsgApp's LogTerm.
		//
		// See the other caller for findConflictByTerm (in stepLeader) for a much
		// more detailed explanation of this mechanism.
    SyncIndex hintIndex = MIN(appendEntries->index, syncRaftLogLastIndex(pRaft->log));
    hintIndex = syncRaftLogFindConflictByTerm(pRaft->log, hintIndex, appendEntries->term);
    SyncTerm hintTerm = syncRaftLogTermOf(pRaft->log, hintIndex, &err);
    if (err != RAFT_OK) {
      syncFatal("[%d:%d]term(%"PRId64") must be valid, but got %s",
        pRaft->selfGroupId, pRaft->selfId, hintIndex, gSyncRaftCodeString[err]);
    }
    *appendResp = (RaftMsg_Append_Resp) {
      .index = appendEntries->index,
      .reject = true,
      .rejectHint = hintIndex,
      .logTerm = hintTerm,
    };
  }

out:
  syncRaftSend(pRaft, pRespMsg, pNode);
  return RAFT_OK;
}