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

#include <assert.h>
#include "raft_log.h"

#define RAFT_LOG_INIT_SIZE 1024

void raftLogInit(RaftLog* pLog) {
  assert(pLog != NULL);

  pLog->entries = (RaftRefEntry*)malloc(sizeof(RaftRefEntry) * RAFT_LOG_INIT_SIZE);
  pLog->size = RAFT_LOG_INIT_SIZE;
  pLog->front = pLog->back = 0;
  pLog->offset = 0;
  pLog->snapshot.lastIndex = 0;
  pLog->snapshot.lastTerm = 0;
}

void raftLogClose(RaftLog* pLog) {

}

void raftLogStart(RaftLog* pLog,
                  RaftSnapshotMeta snapshot,
                  RaftIndex startIndex) {
  pLog->snapshot = snapshot;
  pLog->offset   = startIndex - 1; 
}

size_t inline raftLogNumEntries(const RaftLog* pLog) {
  assert(pLog != NULL);

  /* if circular buffer wrapped? */
  if (pLog->front > pLog->back) {
    return pLog->size + pLog->back - pLog->front;
  }

  return pLog->back - pLog->front;
}

RaftIndex raftLogLastIndex(RaftLog* pLog) {
  /**
   * if there are no entries and there is a snapshot,
   * check that the last index of snapshot is not smaller than log offset
   **/
  if (raftLogNumEntries(pLog) && pLog->snapshot.lastIndex != 0) {
    assert(pLog->offset <= pLog->snapshot.lastIndex);
  }

  return pLog->offset + raftLogNumEntries(pLog);
}

RaftTerm raftLogLastTerm(RaftLog* pLog) {
  RaftIndex lastIndex;

  lastIndex = raftLogLastIndex(pLog);

  if (lastIndex > 0) {
    return raftLogTermOf(pLog, lastIndex, NULL);
  }

  return 0;
}

RaftTerm raftLogTermOf(RaftLog* pLog, RaftIndex index, RaftCode* errCode) {
  size_t i;

  assert(index > 0);
  assert(pLog->offset <= pLog->snapshot.lastIndex);

  if ((index < pLog->offset + 1 && index != pLog->snapshot.lastIndex) ||
      (index > raftLogLastIndex(pLog)) {
    return 0;
  }
  
}