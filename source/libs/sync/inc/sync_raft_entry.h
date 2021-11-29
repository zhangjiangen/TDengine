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

#ifndef _TD_LIBS_SYNC_RAFT_ENTRY__H
#define _TD_LIBS_SYNC_RAFT_ENTRY__H

#include "sync.h"
#include "sync_type.h"

SSyncRaftEntryArray* syncRaftCreateEntryArray();

void syncRaftDestroyEntryArray(SSyncRaftEntryArray*);

void syncRaftCleanEntryArray(SSyncRaftEntryArray*);

int syncRaftNumOfEntries(const SSyncRaftEntryArray* ents);

const SSyncRaftEntry* syncRaftEntryOfPosition(const SSyncRaftEntryArray* ents, int pos);

SyncTerm syncRaftTermOfPosition(const SSyncRaftEntryArray* ents, int pos);

// delete all entries before the given position(included)
void syncRaftRemoveEntriesBeforePosition(SSyncRaftEntryArray* ents, int pos);

int syncRaftAppendEntries(SSyncRaftEntryArray* ents, SSyncRaftEntry* entries, int n);

int syncRaftAppendEmptyEntry(SSyncRaftEntryArray* ents);

int syncRaftAssignEntries(SSyncRaftEntryArray* ents, SSyncRaftEntry* entries, int n);

// return entries between [lo,hi]
int syncRaftSliceEntries(SSyncRaftEntryArray* ents, int lo, int hi, SSyncRaftEntry** ppEntries, int* n);

const SSyncRaftEntry* syncRaftFirstEntry(const SSyncRaftEntryArray* ents);

const SSyncRaftEntry* syncRaftLastEntry(const SSyncRaftEntryArray* ents);

#endif // _TD_LIBS_SYNC_RAFT_STABLE_LOG_H
