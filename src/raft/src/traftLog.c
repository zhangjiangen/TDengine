/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
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

#include <stdlib.h>

#include "traft.h"

// The log store is implemented using a ring array
struct SRaftLog {
  // Capacity of the array
  raft_index_t capacity;
  raft_index_t count;
  raft_index_t head;
  raft_index_t tail;
  SLogEntry*   entries;
};

static void raftLogAppendEntryImpl(SRaftLog *pRaftLog, SLogEntry *pLogEntry);
static bool raftLogHasCapacity(SRaftLog *pRaftLog, log_index_t nEntries);

SRaftLog* raftLogNew(raft_index_t capacity) {
  SRaftLog* pRaftLog = calloc(1, sizeof(*pRaftLog));
  if (pRaftLog == NULL) {
    // TODO: deal with error
    return NULL;
  }

  pRaftLog->entries = malloc(sizeof(SLogEntry) * capacity);
  if (pRaftLog->entries == NULL) {
    // TODO: deal with error
    free(pRaftLog);
    return NULL;
  }

  pRaftLog->capacity = capacity;

  return pRaftLog;
}

SRaftLog *rafgLogFree(SRaftLog *pRaftLog) {
  if (pRaftLog) {
    if (pRaftLog->entries) {
      free(pRaftLog->entries);
      pRaftLog->entries = NULL;
    }

    free(pRaftLog);
  }

  return NULL;
}

int raftLogAppendEntry(SRaftLog *pRaftLog, SLogEntry *pLogEntry) {
  if (!raftLogHasCapacity(pRaftLog, 1)) {
    // TODO: deal with error, need to return a error code showing
    // that the log store has no capacity any more
    return -1;
  }

  raftLogAppendEntryImpl(pRaftLog, pLogEntry);
  return 0;
}

int raftLogAppendEntries(SRaftLog *pRaftLog, log_index_t nEntries, SLogEntry *entries) {
  if (!raftLogHasCapacity(pRaftLog, nEntries)) {
    // TODO: deal with error, need to return a error code showing
    // that the log store has no capacity any more
    return -1;
  }

  for (log_index_t i = 0; i < nEntries; i++) {
    raftLogAppendEntryImpl(pRaftLog, entries + i);
  }

  return 0;
}

log_index_t raftLogLastIndex(SRaftLog *pRaftLog) {
  // TODO
}

static void raftLogAppendEntryImpl(SRaftLog *pRaftLog, SLogEntry *pLogEntry) {
  pRaftLog->entries[pRaftLog->tail++] = *pLogEntry;
  pRaftLog->tail = pRaftLog->tail % pRaftLog->capacity;
  pRaftLog->count++;
}

static bool raftLogHasCapacity(SRaftLog *pRaftLog, log_index_t nEntries) {
  if (pRaftLog->count + nEntries <= pRaftLog->capacity) {
    return true;
  } else {
    return false;
  }
}