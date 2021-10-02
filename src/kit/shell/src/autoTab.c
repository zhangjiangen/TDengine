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

#define _BSD_SOURCE
#define _GNU_SOURCE
#define _XOPEN_SOURCE
#define _DEFAULT_SOURCE

#include "autoTab.h"
#include "os.h"
#include "shellCommand.h"


// main entry
void pressTabKey(TAOS* con, Command* pcmd) {
  if(pcmd->commandSize == 0) {
    // insert help
    showHelp();
    showOnScreen(pcmd);
  } else {
    autoFilling(pcmd->command, con, pcmd);
  }

}

void autoFilling(char* pre, TAOS* conn, Command * pcmd) {
  // first level

  // second level

  

}

void showHelp() {
    printf("You can run below command: \n");
    printf("  create database \n");
    printf("  create table \n");
    printf("  show databases \n");
    printf("  use databases \n");
    printf("  describe table \n");
}
