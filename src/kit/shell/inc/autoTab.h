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

#ifndef __AUTOTAB__
#define __AUTOTAB__

#include "taos.h"
#include "shellCommand.h"

// main entry
void pressTabKey(TAOS* con, Command* pcmd);

// show help
void showHelp();
// auto filling match words with pre
void autoFilling(char* pre, TAOS* conn, Command * pcmd);

#endif
