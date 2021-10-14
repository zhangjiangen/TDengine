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

#include "raft_impl.h"
#include "raft_log.h"
#include "raft_progress.h"

static resetProgressState(RaftProgress* progress, RaftProgressState state);
static resumeProgress(RaftProgress* progress);
static pauseProgress(RaftProgress* progress);

int raftProgressCreate(RaftCore* raft) {

/*
  inflights->buffer = (RaftIndex*)malloc(sizeof(RaftIndex) * raft->maxInflightMsgs);
  if (inflights->buffer == NULL) {
    return RAFT_OOM;
  }
  inflights->size  = raft->maxInflightMsgs;
*/
}

int raftProgressRecreate(RaftCore* raft, const RaftConfiguration* configuration) {

}

bool raftProgressIsUptodate(RaftCore* raft, int i) {
  RaftProgress* progress = &(raft->leaderState.progress[i]);
  return raftLogLastIndex(raft->log) + 1== progress->nextIndex;
}

bool raftProgressIsPaused(RaftCore* raft, int i) {
  RaftProgress* progress = &(raft->leaderState.progress[i]);
  RaftTime now = raft->io.time(raft);
  bool needHeartbeat = now - progress->lastSend >= raft->heartbeatTimeoutMS;
  RaftIndex lastIndex = raftLogLastIndex(raft->log);

  assert(progress->nextIndex <= lastIndex + 1);
  
  if (progress->state == PROGRESS_SNAPSHOT) {
    if (raft->installSnapShotTimeoutMS > 0 &&
        now - progress->lastSendSnapshot >= raft->installSnapShotTimeoutMS) {
      raftError("snapshot timed out for %s", raftConfigurationString(raft, i));      
      raftProgressAbortSnapshot(raft, i);
      return false;
    } else {
      return !needHeartbeat;
    }

    return true;
  }

  if (progress->state == PROGRESS_PROBE) {
    return progress->paused;
  }

  if (progress->state == PROGRESS_REPLICATE) {
    return raftInflightFull(progress->inflights);
  }

  /* never reach here */
  assert(NULL);
}

bool raftProgressNeedAbortSnapshot(RaftCore*, int i) {
  RaftProgress* progress = &(raft->leaderState.progress[i]);

  return progress->state == PROGRESS_SNAPSHOT && progress->matchIndex >= progress->pendingSnapshotIndex;
}

RaftIndex raftProgressNextIndex(RaftCore* raft, int i) {
  return raft->leaderState.progress[i].nextIndex;
}

RaftIndex raftProgressMatchIndex(RaftCore* raft, int i) {
  return raft->leaderState.progress[i].matchIndex;
}

void raftProgressUpdateLastSend(RaftCore* raft, int i) {
  raft->leaderState.progress[i].lastSend = raft->io.time(raft);
}

void raftProgressUpdateSnapshotLastSend(RaftCore* raft, int i) {
  raft->leaderState.progress[i].lastSendSnapshot = raft->io.time(raft);
}

bool raftProgressResetRecentRecv(RaftCore* raft, int i) {
  RaftProgress* progress = &(raft->leaderState.progress[i]);
  bool prev = progress->recentRecv;
  progress->recentRecv = false;
  return prev;
}

void raftProgressMarkRecentRecv(RaftCore* raft, int i) {
  raft->leaderState.progress[i].recentRecv = true;
}

bool raftProgressGetRecentRecv(RaftCore* raft, int i) {
  return raft->leaderState.progress[i].recentRecv;
}

void raftProgressBecomeSnapshot(RaftCore* raft, int i) {
  RaftProgress* progress = &(raft->leaderState.progress[i]);
  resetProgressState(progress, PROGRESS_SNAPSHOT);
  progress->pendingSnapshotIndex = raftLogSnapshotIndex(raft->log);
}

void raftProgressBecomeProbe(RaftCore* raft, int i) {
  RaftProgress* progress = &(raft->leaderState.progress[i]);

  if (progress->state == PROGRESS_SNAPSHOT) {
    assert(progress->pendingSnapshotIndex > 0);
    RaftIndex pendingSnapshotIndex = progress->pendingSnapshotIndex;
    resetProgressState(progress, PROGRESS_PROBE);
    progress->nextIndex = max(progress->matchIndex + 1, pendingSnapshotIndex);
  } else {
    resetProgressState(progress, PROGRESS_PROBE);
    progress->nextIndex = progress->matchIndex + 1;
  }
}

void raftProgressBecomeReplicate(RaftCore* raft, int i) {
  resetProgressState(progress, PROGRESS_REPLICATE);
  progress->nextIndex = progress->matchIndex + 1;
}

void raftProgressAbortSnapshot(RaftCore* raft, int i) {
  RaftProgress* progress = &(raft->leaderState.progress[i]);
  progress->pendingSnapshotIndex = 0;
  progress->state = PROGRESS_PROBE;
}

RaftProgressState raftProgressState(RaftCore* raft, int i) {
  return raft->leaderState.progress[i].state;
}

void raftProgressOptimisticNextIndex(RaftCore* raft,
                                    int i,
                                    RaftIndex nextIndex) {
  raft->leaderState.progress[i].nextIndex = nextIndex + 1;
}

bool raftProgressMaybeUpdate(RaftCore* raft, int i, RaftIndex lastIndex) {
  RaftProgress* progress = &(raft->leaderState.progress[i]);
  bool updated = false;

  if (progress->matchIndex < lastIndex) {
    progress->matchIndex = lastIndex;
    updated = true;
    resumeProgress(progress);
  }
  if (progress->nextIndex < lastIndex + 1) { 
    progress->nextIndex = lastIndex + 1;
  }

  return updated;
}

bool raftProgressMaybeDecrTo(RaftCore* raft,
                            int i,
                            RaftIndex rejected,
                            RaftIndex lastIndex) {
  RaftProgress* progress = &(raft->leaderState.progress[i]);

  if (progress->state == PROGRESS_REPLICATE) {
		/** 
     * the rejection must be stale if the progress has matched and "rejected"
		 * is smaller than "match".
     **/
    if (rejected <= progress->matchIndex) {
      tsdbDebug("match index is up to date,ignore");
      return false;
    }

    /* directly decrease next to match + 1 */
    progress->nextIndex = progress->matchIndex + 1;
    //raftProgressBecomeProbe(raft, i);
    return true;
  }

/*
  if (progress->state == PROGRESS_SNAPSHOT) {
    if (rejected != progress->pendingSnapshotIndex) {
      return false;
    }

    raftProgressAbortSnapshot(raft, i);

    return true;
  }
*/

  if (rejected != progress->nextIndex - 1) {
    tsdbDebug("rejected index %llu different from next index %lld -> ignore"
      , rejected, progress->nextIndex);
    return false;
  }

  progress->nextIndex = min(rejected, lastIndex + 1);
  progress->nextIndex = max(1, progress->nextIndex);

  resumeProgress(progress);
  return true;
}

static resetProgressState(RaftProgress* progress, RaftProgressState state) {
  progress->paused = false;
  progress->pendingSnapshotIndex = 0;
  progress->state = state;
  raftInflightReset(progress->inflights);
}

static resumeProgress(RaftProgress* progress) {
  progress->paused = false;
}

static pauseProgress(RaftProgress* progress) {
  progress->paused = true;
}

int raftInflightReset(RaftInflights* inflights) {  
  inflights->count = 0;
  inflights->start = 0;
  
  return 0;
}

bool raftInflightFull(RaftInflights* inflights) {
  return inflights->count == inflights->size;
}

void raftInflightAdd(RaftInflights* inflights, RaftIndex inflightIndex) {
  assert(!raftInflightFull(inflights));

  int next = inflights->start + inflights->count;
  int size = inflights->size;
  /* is next wrapped around buffer? */
  if (next >= size) {
    next -= size;
  }

  inflights->buffer[next] = inflightIndex;
  inflights->count++;
}

void raftInflightFreeTo(RaftInflights* inflights, RaftIndex toIndex) {
  if (inflights->count == 0 || toIndex < inflights->buffer[inflights->startIndex]) {
    return;
  }

  int i, idx;
  for (i = 0, idx = inflights->start; i < inflights->count; i++) {
    if (toIndex < inflights->buffer[idx]) {
      break;
    }

    int size = inflights->size;
    idx++;
    if (idx >= size) {
      idx -= size;
    }
  }

  inflights->count -= i;
  inflights->start  = idx;
  assert(inflights->count >= 0);
  if (inflights->count == 0) {
    inflights->start = 0;
  }
}

void raftInflightFreeFirstOne(RaftInflights* inflights) {
  raftInflightFreeTo(inflights, inflights->buffer[inflights->start]);
}