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

#include "sync_const.h"
#include "sync_raft_entry.h"

struct SSyncRaftEntry {
  SyncTerm term;
  SSyncBuffer buffer;
  unsigned int refCount;
};

struct SSyncRaftEntryArray {
  // Circular buffer of log entries
  SSyncRaftEntry *entries;

  // size of Circular buffer
  int size;

  // Indexes of used slots [front, back)
  int front, back;

  // Index of first entry is offset + 1
  SyncIndex offset;
};

static int locateEntryOfIndex(const SSyncRaftEntryArray*, SyncIndex index);
static SyncIndex indexAt(const SSyncRaftEntryArray*, int i);
static int positionAt(const SSyncRaftEntryArray*, int i);
static const SSyncRaftEntry* entryAt(const SSyncRaftEntryArray*, int i);
static void removePrefix(SSyncRaftEntryArray*, SyncIndex index);

static void resetEntries(SSyncRaftEntryArray*);

static int ensureCapacity(SSyncRaftEntryArray*, int n);

static int sliceEntries(SSyncRaftEntryArray* ents, SyncIndex from, SyncIndex to, SSyncRaftEntry **ppEntries, int* n);

static bool decrEntry(SSyncRaftEntry *entry);
static void incrEntry(SSyncRaftEntry *entry);

SSyncRaftEntryArray* syncRaftCreateEntryArray(SyncIndex firstIndex) {
  SSyncRaftEntry* entries = (SSyncRaftEntry*)malloc(sizeof(SSyncRaftEntry) * kEntryArrayInitSize);
  if (entries == NULL) {
    return NULL;
  }
  SSyncRaftEntryArray* ary = (SSyncRaftEntryArray*)malloc(sizeof(SSyncRaftEntryArray));
  if (ary == NULL) {
    free(entries);
    return NULL;
  }
  *ary = (SSyncRaftEntryArray) {
    .entries  = entries,
    .front    = 0,
    .back     = 0,
    .offset   = firstIndex - 1,
    .size     = kEntryArrayInitSize,
  };

  return ary;
}

int syncRaftNumOfEntries(const SSyncRaftEntryArray* ents) {
  // if circular buffer wrapped?
  if (ents->front > ents->back) {
    return ents->size + ents->back - ents->front;
  }

  return ents->back - ents->front;
}

SyncIndex syncRaftLastIndexOfEntries(const SSyncRaftEntryArray* ents) {
  return ents->offset + syncRaftNumOfEntries(ents);
}

SyncIndex syncRaftFirstIndexOfEntries(const SSyncRaftEntryArray* ents) {
  return ents->offset + 1;
}

SyncTerm syncRaftTermOfEntries(const SSyncRaftEntryArray* ents, SyncIndex i) {
  int i = locateEntryOfIndex(ents, i);
  if (i < 0) {
    return SYNC_NON_TERM;
  }

  return entryAt(ents, i)->term;
}

// delete all entries before the given raft index(included)
void syncRaftRemoveLogEntriesBefore(SSyncRaftEntryArray* ents, SyncIndex i) {
  removePrefix(ents, i);
}

void syncRaftRemoveLogEntriesAfter(SSyncRaftEntryArray* ents, SyncIndex i) {

}

int syncRaftAppendLogEntries(SSyncRaftEntryArray* ents, SyncIndex index, SSyncRaftEntry* entries, int n) {
  assert(index == syncRaftLastIndexOfEntries(ents) + 1);

  int ret = ensureCapacity(ents, n);
  if (ret < 0) {
    return ret;
  }

  SyncIndex firstIndex = syncRaftLastIndexOfEntries(ents) + 1;
  SSyncRaftEntry* entry;
  int i;
  for (i = 0; i < n; ++i) {
    entry = &ents->entries[ents->back];
    memcpy(entry, &entries[i], sizeof(SSyncRaftEntry*));
    ents->back = (ents->back + 1) % ents->size;
  }

  return RAFT_OK;
}

int syncRaftTruncateAndAppendLogEntries(SSyncRaftEntryArray* ents, SyncIndex index, SSyncRaftEntry* entries, int n) {
  SyncIndex afterIndex = index;
  SyncIndex lastIndex  = syncRaftLastIndexOfEntries(ents);
  if (afterIndex == lastIndex + 1) {
		// after is the next index in the u.entries
		// directly append    
    return syncRaftAppendLogEntries(ents, index, entries, n);
  }

  if (afterIndex <= ents->offset) {
		// The log is being truncated to before our current offset
		// portion, so set the offset and replace the entries
    resetEntries(ents);
    ents->offset = afterIndex;
    return syncRaftAppendLogEntries(ents, index, entries, n);
  }

	// truncate to after and copy to u.entries
	// then append
  SSyncRaftEntry* sliceEnts;
  int nSliceEnts;
  sliceEntries(ents, ents->offset, afterIndex, &sliceEnts, &nSliceEnts);
  resetEntries(ents);
  syncRaftAppendLogEntries(ents, index, sliceEnts, nSliceEnts);
  syncRaftAppendLogEntries(ents, index, entries, n);
}

// return the index of log entries with the given raft index
// return -1 if not found
static int locateEntryOfIndex(const SSyncRaftEntryArray* ents, SyncIndex index) {
  int size = syncRaftNumOfEntries(ents);
  if (size == 0 || index < indexAt(ents, 0) || index > indexAt(ents, size - 1)) {
    return -1;
  }

  return positionAt(ents, (int)(index - 1 - ents->offset));
}

// return raft index of i'th entry in the log
static SyncIndex indexAt(const SSyncRaftEntryArray* ents, int i) {
  return ents->offset + i + 1;
}

// return the index of circular buffer of the i'th entry in the log
static int positionAt(const SSyncRaftEntryArray* ents, int i) {
  return (ents->front + i) % ents->size;
}

static const SSyncRaftEntry* entryAt(const SSyncRaftEntryArray* ents, int i) {
  return &(ents->entries[positionAt(ents, i)]);
}

// Ensure that the entries array has enough free slots for adding n new entry.
static int ensureCapacity(SSyncRaftEntryArray* ents, int n) {
  int size = syncRaftNumOfEntries(ents);

  if (size + n < ents->size) {
    return RAFT_OK;
  }

  int newSize = size * 2;
  while (newSize < size + n) {
    newSize += size;
  }

  SSyncRaftEntry *entries = (SSyncRaftEntry*)calloc(newSize, sizeof(SSyncRaftEntry));
  if (entries == NULL) {
    return RAFT_NO_MEM;
  }

  int i;
  for (i = 0; i < size; ++i) {
    memcpy(&entries[i], entryAt(ents, i), sizeof(SSyncRaftEntry*));
  }
  free(ents->entries);

  ents->entries = entries;
  ents->size = newSize;
  ents->front = 0;
  ents->back = size;

  return RAFT_OK;
}

static int sliceEntries(SSyncRaftEntryArray* ents, SyncIndex from, SyncIndex to, SSyncRaftEntry **ppEntries, int* n) {
  return RAFT_OK;
}

// delete all entries before the given raft index(included)
static void removePrefix(SSyncRaftEntryArray* ents, SyncIndex index) {
  int i, n;
  SSyncRaftEntry* entry;
  bool removeFront;

  assert(ents != NULL);
  assert(index > 0);
  assert(index <= syncRaftLastIndexOfEntries(ents));

  // Number of entries to delete
  n = index - indexAt(ents, 0) + 1;

  for (i = 0, removeFront = true; i < n; i++) {
    entry = &(ents->entries[ents->front]);
    if (!decrEntry(entry)) {
      removeFront = false;
      continue;
    }
    if (!removeFront) {
      continue;
    }
    if (ents->front == ents->size - 1) {
      ents->front = 0;
    } else {
      ents->front++;
    }
    ents->offset += 1;
  }
}

static bool decrEntry(SSyncRaftEntry *entry) {
  entry->refCount -= 1;
  assert(entry->refCount >= 0);
  if (entry->refCount == 0) {
    free(entry->buffer.data);
    entry->buffer.len = 0;
    entry->term = SYNC_NON_TERM;
    return true;
  }

  return false;
}

static void incrEntry(SSyncRaftEntry *entry) {
  assert(entry->refCount >= 0);
  entry->refCount += 1;
}