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

#include "raft.h"
#include "raft_type.h"

/** 
 * below define message type which handled by Raft node thread
 * internal message, which communicate in threads, start with RAFT_MSG_INTERNAL_*,
 * internal message use pointer only, need not to be decode/encode
 * outter message start with RAFT_MSG_*, need to implement its decode/encode functions
 **/
typedef enum RaftMessageType {
  // client propose a cmd
  RAFT_MSG_INTERNAL_PROP = 1,

  RAFT_MSG_APPEND,
  RAFT_MSG_APPEND_RESP,

  RAFT_MSG_VOTE,
  RAFT_MSG_VOTE_RESP,

  RAFT_MSG_PRE_VOTE,
  RAFT_MSG_PRE_VOTE_RESP,

} RaftMessageType;

typedef struct RaftMsgInternalProp {
  const RaftBuffer *pBuf;
  RaftApplyFp applyCb;
} RaftMsgInternalProp;

typedef struct RaftMsgVote {
  // candidate's term
  RaftTerm term;
  // candidate's id
  RaftId id;
  // candidate's last log index
  RaftIndex lastLogIndex;
  // candidate's last log term
  RaftTerm lastLogTerm;
  // true if preVote request
  bool preVote;
} RaftMsgVote;

typedef struct RaftMessage {
  RaftMessageType msgType;
  RaftTerm term;
  RaftId from;
  RaftId to;

  union {
    RaftMsgInternalProp propose;
    RaftMsgVote vote;
  };
} RaftMessage;

static bool isInternalMsg(RaftMessageType msgType) {
  return msgType == RAFT_MSG_INTERNAL_PROP;
}

static bool isVoteMsg(RaftMessageType msgType) {
  return msgType == RAFT_MSG_VOTE || msgType == RAFT_MSG_PRE_VOTE;
}

int handleVoteMessage(Raft* raft, const RaftMessage* msg);

#endif  /* TD_RAFT_MESSAGE_H */