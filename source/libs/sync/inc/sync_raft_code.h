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

#ifndef _TD_LIBS_SYNC_RAFT_CODE_H
#define _TD_LIBS_SYNC_RAFT_CODE_H

typedef enum ESyncRaftCode {
  RAFT_OK = 0,

  RAFT_OOM = 1,

  // ErrCompacted is returned by Storage.Entries/Compact when a requested
  // index is unavailable because it predates the last snapshot.
  RAFT_COMPACTED = 2,

  // ErrUnavailable is returned by Storage interface when the requested log entries
  // are unavailable.
  RAFT_UNAVAILABLE = 3,
} ESyncRaftCode;

const static const char* gSyncRaftCodeString[] = {
  "OK",
  "OOM",
  "COMPACTED",
  "UNAVAILABLE",
};

#endif // _TD_LIBS_SYNC_RAFT_CODE_H
