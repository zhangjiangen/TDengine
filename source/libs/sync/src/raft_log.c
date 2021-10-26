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

static int locateEntry(RaftLog* pLog, RaftIndex index);
static RaftIndex indexAt(RaftLog* pLog, int i);
static int positionAt(RaftLog* pLog, int i);
static int ensureCapacity(RaftLog* pLog);
static void decrEntry(RaftLog* pLog, RaftEntry *entry);
static void removeSuffix(RaftLog* pLog, RaftIndex index);
static void removePrefix(RaftLog* pLog, RaftIndex index);

void raftLogInit(RaftLog* pLog) {
  assert(pLog != NULL);

  pLog->entries = (RaftEntry*)malloc(sizeof(RaftEntry) * RAFT_LOG_INIT_SIZE);
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

int inline raftLogNumEntries(const RaftLog* pLog) {
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
  int i;

  assert(index > 0);
  assert(pLog->offset <= pLog->snapshot.lastIndex);

  if (index == pLog->snapshot.lastIndex) {
    assert(pLog->snapshot.lastTerm != 0);
    i = locateEntry(pLog, index);
    /**
     * if we still have the entry at last index, then its term MUST match to the snapshot last term
     **/
    if (i != l->size) {
      assert(pLog->entries[i].term == pLog->snapshot.lastTerm);
    }

    return pLog->snapshot.lastTerm;
  }

  /* is the log has been compacted? */
  if (index < pLog->offset + 1) {
    if (errCode) *errCode = RAFT_INDEX_COMPACTED;
    return 0;
  }

  /* is the log index bigger than the last log index? */
  if (index > raftLogLastIndex(pLog)) {
    if (errCode) *errCode = RAFT_INDEX_UNAVAILABLE;
    return 0;
  }
  
  i = locateEntry(pLog, index);
  assert(i < pLog->size);
  return pLog->entries[i].term;
}

RaftIndex raftLogSnapshotIndex(RaftLog* pLog) {
  return pLog->snapshot.lastIndex;
}

int raftLogAppend(RaftLog* pLog,
                  RaftTerm term,
                  const RaftBuffer *buf) {
  int ret = 0;
  RaftIndex index;
  RaftEntry* pEntry = NULL;

  assert(pLog != NULL);
  assert(buf  != NULL);
  assert(term != 0);

  /* ensure the log buffer capacity */
  ret = ensureCapacity(pLog);
  if (ret != 0) {
    return ret;
  }

  /* the log index is last index + 1 */
  index = raftLogLastIndex(pLog) + 1;

  /* init the log entry */
  pEntry = &(pLog->entries[pLog->back]);
  pEntry->term = term;
  pEntry->buffer = *buf;
  pEntry->refCount = 0;

  /* increament the back position of the circular buffer */
  pLog->back = (pLog->back + 1) % pLog->size;

  return 0;
}

int raftLogAcquire(RaftLog* pLog,
                  RaftIndex index,
                  RaftEntry **ppEntries,
                  int *n) {
  int i; 
  int j;

  assert(pLog != NULL);
  assert(ppEntries != NULL);
  assert(index > 0);

  /* Get the array index of the first entry to acquire. */
  i = locateEntry(pLog, index);

  if (i == pLog->size) {
    *ppEntries = NULL;
    *n = 0;
    return 0;
  }

  if (i < pLog->back) {
    /* the last entry does not wraps with respect to i, so the number of entries range from [i,back) */
    *n = pLog->back - i;
  } else {
    /* 
     * the last entry wraps with respect to i, so the number of entries is the sum of
     * [i,size) plus [0,back)
     */
    *n = pLog->size - i + pLog->back;
  }

  assert(*n > 0);
  *ppEntries = (RaftEntry*)malloc(sizeof(RaftEntry) * (*n) );
  if (ppEntries == NULL) {
    return RAFT_OOM;
  }

  for (j = 0; j < *n; ++j) {
    int k = (i + j) % l->size;
    RaftEntry* entry = &(*ppEntries)[j];
    *entry = pLog->entries[k];
    entry->refCount += 1;
  }

  return 0;
}

void raftLogRelease(RaftLog* pLog,
                    RaftIndex index,
                    RaftEntry *pEntries,
                    int n) {
  int i;

  assert(pLog != NULL);

  for (i = 0; i < n; i++) {
    RaftEntry* entry = pEntries[i];

    decrEntry(pLog, entry);
  }

  if (pEntries != NULL) {
    free(pEntries);
  }
}

void raftLogTruncate(RaftLog* pLog, RaftIndex index) {
  if (raftLogNumEntries(pLog) == 0) {
    return;
  }

  removeSuffix(pLog, index);
}

void raftLogSnapshot(RaftLog* pLog, RaftIndex lastIndex, RaftIndex trailing) {
  RaftTerm lastTerm = raftLogLastTerm(pLog, lastIndex);

  assert(lastTerm != 0);

  pLog->snapshot.lastTerm = lastTerm;
  pLog->snapshot.lastIndex = lastIndex;

  /* If we have not at least n entries preceeding the given last index, then
     * there's nothing to remove and we're done. */
  if (lastIndex <= trailing || locateEntry(pLog, lastIndex - trailing) == l->size) {
    return;
  }

  removePrefix(pLog, lastIndex - trailing);
}

/**
 * return the offset of the log entries with the given raft index.
 * if there is no log with the given index, return size of the entries.
 **/
static int locateEntry(RaftLog* pLog, RaftIndex index) {
  int n = raftLogNumEntries(pLog);

  /**
   * if there is no entry, or index out of entries index range, return size of entries
   **/
  if (n == 0 || index < indexAt(pLog, 0) || index > indexAt(pLog, n - 1)) {
    return pLog->size;
  }

  return positionAt(pLog, (int)(index - 1 - pLog->offset));
}

/**
 * return the raft index of the i-th entry in the log entries
 **/ 
static RaftIndex indexAt(RaftLog* pLog, int i) {
  return pLog->offset + i + 1;
}

/**
 * return the circular buffer position of the i'th entry in the log entries
 **/
static int positionAt(RaftLog* pLog, int i) {
  return (pLog->front + i) % pLog->size;
}

static int ensureCapacity(RaftLog* pLog) {

}

static void decrEntry(RaftLog* pLog, RaftEntry *entry) {
  --entry->refCount;

  if (entry->refCount > 0) {
    return;
  }

  free(entry->buffer.data);
}

/* removing all log entries from index(include) onward.*/
static void removeSuffix(RaftLog* pLog, RaftIndex index) {
  int i, n;
  RaftIndex start = index;
  RaftEntry* entry;

  assert(pLog != NULL);
  assert(index > pLog->offset);
  assert(index <= raftLogLastIndex(pLog));

  /* Number of entries to delete */
  n = raftLogLastIndex(pLog) - start + 1;

  for (i = 0; i < n; i++) {
    if (pLog->back == 0) {
      pLog->back = pLog->size - 1;
    } else {
      pLog->back--;
    }

    entry = &(pLog->entries[pLog->back]);
    decrEntry(pLog, entry);
  }
}

/* Delete all entries up to the given index (included). */
static void removePrefix(RaftLog* pLog, RaftIndex index) {
  int i, n;

  assert(pLog != NULL);
  assert(index > 0);
  assert(index <= raftLogLastIndex(pLog));

  /* Number of entries to delete */
  n = index - indexAt(pLog, 0) + 1;

  for (i = 0; i < n; i++) {
    entry = &(pLog->entries[pLog->front]);

    if (pLog->front == pLog->size - 1) {
      pLog->front = 0;
    } else {
      pLog->front++;
    }
    pLog->offset++;
    
    decrEntry(pLog, entry);
  }
}