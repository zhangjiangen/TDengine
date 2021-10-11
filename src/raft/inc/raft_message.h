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

#ifndef TD_RAFT_MESSAGE_H
#define TD_RAFT_MESSAGE_H

/** 
 * below define message type which handled by Raft node thread
 * internal message, which communicate in threads, start with RaftInternalMessage_*,
 * internal message use pointer only, need not to be decode/encode
 * outter message start with RaftMessage_*, need to implement its decode/encode functions
 **/
enum RaftMessageType {

};

typedef struct RaftMessage {
  int type;

} RaftMessage;

#endif  /* TD_RAFT_MESSAGE_H */