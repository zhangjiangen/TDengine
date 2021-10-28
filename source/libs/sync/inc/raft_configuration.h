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

#ifndef TD_RAFT_CONFIGURATION_H
#define TD_RAFT_CONFIGURATION_H

#include "raft.h"
#include "raft_type.h"

int raftConfigurationCopy(const RaftConfiguration* cluster, RaftConfiguration** pRet);

const char* raftConfigurationString(Raft* raft, int i);

#endif /* TD_RAFT_CONFIGURATION_H */