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

#ifndef TD_RAFT_PROGRESS_H
#define TD_RAFT_PROGRESS_H

#include "raft.h"
#include "raft_type.h"

/** 
 * RaftInflights is a sliding window for the inflight messages.
 * Thus inflight effectively limits both the number of inflight messages
 * and the bandwidth each Progress can use.
 * When inflights is full, no more message should be sent.
 * When a leader sends out a message, the index of the last
 * entry should be added to inflights. The index MUST be added
 * into inflights in order.
 * When a leader receives a reply, the previous inflights should
 * be freed by calling raftInflightFreeTo with the index of the last
 * received entry.
 **/ 
typedef struct RaftInflights {
  /* the starting index in the buffer */
  int start;

  /* number of inflights in the buffer */
  int count;

  /* the size of the buffer */
  int size;

	/** 
   * buffer contains the index of the last entry
	 * inside one message.
   **/
  RaftIndex* buffer;
} RaftInflights;

/** 
 * State defines how the leader should interact with the follower.
 *
 * When in PROGRESS_PROBE, leader sends at most one replication message
 * per heartbeat interval. It also probes actual progress of the follower.
 * 
 * When in PROGRESS_REPLICATE, leader optimistically increases next
 * to the latest entry sent after sending replication message. This is
 * an optimized state for fast replicating log entries to the follower.
 *
 * When in PROGRESS_SNAPSHOT, leader should have sent out snapshot
 * before and stops sending any replication message.
 * 
 * PROGRESS_PROBE is the initial state.
 **/
typedef enum RaftProgressState {
  PROGRESS_PROBE = 0,
  PROGRESS_REPLICATE,
  PROGRESS_SNAPSHOT,
} RaftProgressState;

/**
 * Progress represents a follower’s progress in the view of the leader. Leader maintains
 * progresses of all followers, and sends entries to the follower based on its progress.
 **/
struct RaftProgress {
  RaftIndex nextIndex;

  RaftIndex matchIndex;

  RaftProgressState state;

	/**
   * paused is used in PROGRESS_PROBE.
	 * When paused is true, raft should pause sending replication message to this peer. 
   **/ 
  bool paused;

	/** 
   * pendingSnapshotIndex is used in PROGRESS_SNAPSHOT.
	 * If there is a pending snapshot, the pendingSnapshotIndex will be set to the
	 * index of the snapshot. If pendingSnapshotIndex is set, the replication process of
	 * this Progress will be paused. raft will not resend snapshot until the pending one
	 * is reported to be failed.
   **/
  RaftIndex pendingSnapshotIndex;

  /** 
   * Timestamp of last AppendEntries RPC. 
   **/
  RaftTime lastSend;

  /** 
   * Timestamp of last InstallSnaphot RPC. 
   **/
  RaftTime lastSendSnapshot;

  /** 
   * A msg was received within election timeout. 
   **/
  bool recentRecv;

  /**
   * flow control sliding window
   **/
  RaftInflights inflights;
};

int raftProgressCreate(RaftCore* raft);
int raftProgressRecreate(RaftCore* raft, const RaftConfiguration* configuration);

bool raftProgressIsUptodate(RaftCore* raft, int i);

/** 
 * raftProgressIsPaused returns whether sending log entries to this node has been
 * paused. A node may be paused because it has rejected recent
 * MsgApps, is currently waiting for a snapshot, or has reached the
 * MaxInflightMsgs limit.
 **/
bool raftProgressIsPaused(RaftCore* raft, int i);

bool raftProgressNeedAbortSnapshot(RaftCore*, int i);

void raftProgressAbortSnapshot(RaftCore* raft, int i);

RaftIndex raftProgressNextIndex(RaftCore* raft, int i);

RaftIndex raftProgressMatchIndex(RaftCore* raft, int i);

void raftProgressUpdateLastSend(RaftCore* raft, int i);

void raftProgressUpdateSnapshotLastSend(RaftCore* raft, int i);

bool raftProgressResetRecentRecv(RaftCore* raft, int i);

void raftProgressMarkRecentRecv(RaftCore* raft, int i);

bool raftProgressGetRecentRecv(RaftCore* raft, int i);

void raftProgressBecomeSnapshot(RaftCore* raft, int i);

void raftProgressBecomeProbe(RaftCore* raft, int i);

void raftProgressBecomeReplicate(RaftCore* raft, int i);

void raftProgressAbortSnapshot(RaftCore* raft, int i);

RaftProgressState raftProgressState(RaftCore* raft, int i);

void raftProgressOptimisticNextIndex(RaftCore* raft,
                                    int i,
                                    RaftIndex nextIndex);

/**
 *  raftProgressMaybeUpdate returns false if the given n index comes from an outdated message.
 * Otherwise it updates the progress and returns true.
 **/
bool raftProgressMaybeUpdate(RaftCore* raft, int i, RaftIndex lastIndex);

/** 
 * raftProgressMaybeDecrTo returns false if the given to index comes from an out of order message.
 * Otherwise it decreases the progress next index to min(rejected, last) and returns true.
 **/
bool raftProgressMaybeDecrTo(RaftCore* raft,
                            int i,
                            RaftIndex rejected,
                            RaftIndex lastIndex);


int raftInflightReset(RaftInflights* inflights);
bool raftInflightFull(RaftInflights* inflights);
void raftInflightAdd(RaftInflights* inflights, RaftIndex inflightIndex);
void raftInflightFreeTo(RaftInflights* inflights, RaftIndex toIndex);
void raftInflightFreeFirstOne(RaftInflights* inflights);

#endif /* TD_RAFT_PROGRESS_H */