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
#include "sync_raft_log.h"
#include "sync_raft_progress.h"
#include "syncInt.h"
#include "sync_raft_progress_tracker.h"

static bool sendSnapshot(SSyncRaft* pRaft, SSyncRaftProgress* progress);
static bool sendAppendEntries(SSyncRaft* pRaft, SSyncRaftProgress* progress,
                              SyncIndex prevIndex, SyncTerm prevTerm,
                              SSyncRaftEntry *entries, int nEntry);

// maybeSendAppend sends an append RPC with new entries to the given peer,
// if necessary. Returns true if a message was sent. The sendIfEmpty
// argument controls whether messages with no entries will be sent
// ("empty" messages are useful to convey updated Commit indexes, but
// are undesirable when we're sending multiple messages in a batch).
bool syncRaftMaybeSendAppend(SSyncRaft* pRaft, SyncNodeId to, bool sendIfEmpty) {
  SSyncRaftProgress* progress = syncRaftFindProgressByNodeId(&pRaft->tracker->progressMap, to);
  if (progress == NULL) {
    return false;
  }
  assert(pRaft->state == TAOS_SYNC_STATE_LEADER);
  SyncNodeId nodeId = progress->id;

  if (syncRaftProgressIsPaused(progress)) {
    syncInfo("node [%d:%d] paused", pRaft->selfGroupId, nodeId);
    return false;
  }

  SyncIndex nextIndex = syncRaftProgressNextIndex(progress);
  SSyncRaftEntry *entries;
  int nEntry;   
  SyncIndex prevIndex;
  SyncTerm prevTerm;
  ESyncRaftCode err;

  prevIndex = nextIndex - 1;
  prevTerm = syncRaftLogTermOf(pRaft->log, prevIndex, &err);
  int ret = syncRaftLogEntries(pRaft->log, nextIndex, &entries, &nEntry);

  if (nEntry == 0 && !sendIfEmpty) {
    return false;
  }

  if (ret != RAFT_OK || err != RAFT_OK) {
    if (entries) free(entries);
    return sendSnapshot(pRaft, progress);
  }

  return sendAppendEntries(pRaft, progress, prevIndex, prevTerm, entries, nEntry);
}

static bool sendSnapshot(SSyncRaft* pRaft, SSyncRaftProgress* progress) {
  if (!syncRaftProgressRecentActive(progress)) {
    return false;
  }
  return true;
}

static bool sendAppendEntries(SSyncRaft* pRaft, SSyncRaftProgress* progress,
                              SyncIndex prevIndex, SyncTerm prevTerm,
                              SSyncRaftEntry *entries, int nEntry) {                                
  SSyncNodeInfo* pNode = syncRaftGetNodeById(pRaft, progress->id);
  if (pNode == NULL) {
    return false;
  }
  bool ret = false;
  SyncIndex lastIndex;
  SyncTerm logTerm = prevTerm;  
  SyncIndex commitIndex = syncRaftLogCommitIndex(pRaft->log);

  SSyncMessage* msg = syncNewAppendMsg(pRaft->selfGroupId, pRaft->selfId, pRaft->term,
                                      prevIndex, prevTerm, commitIndex,
                                      nEntry, entries);

  if (msg == NULL) {
    goto err_release_log;
  }

  if (nEntry != 0) {
    switch (progress->state) {
    // optimistically increase the next when in StateReplicate
    case PROGRESS_STATE_REPLICATE:
      lastIndex = entries[nEntry - 1].index;
      syncRaftProgressOptimisticNextIndex(progress, lastIndex);
      syncRaftInflightAdd(progress->inflights, lastIndex);
      break;
    case PROGRESS_STATE_PROBE:
      progress->probeSent = true;
      break;
    default:
      syncFatal("[%d:%d] is sending append in unhandled state %s", 
                pRaft->selfGroupId, pRaft->selfId, syncRaftProgressStateString(progress));
      break;
    }
  }
  syncRaftSend(pRaft, msg, pNode);
  ret = true;

err_release_log:
  if (entries) free(entries);
  return ret;
}
