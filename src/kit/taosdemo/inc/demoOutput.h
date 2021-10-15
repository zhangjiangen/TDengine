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

#ifndef _DEMOOUTPUT_H_
#define _DEMOOUTPUT_H_
#include "taosdemo.h"
void printfInsertMetaToFile(FILE *fp);
int  printfInsertMeta();
void printfQueryMeta();
void printHelp();
void printVersion();
void errorWrongValue(char *program, char *wrong_arg, char *wrong_value);
void errorUnrecognized(char *program, char *wrong_arg);
void errorPrintReqArg(char *program, char *wrong_arg);
void errorPrintReqArg2(char *program, char *wrong_arg);
void errorPrintReqArg3(char *program, char *wrong_arg);
void printStatPerThread(threadInfo *pThreadInfo);
void appendResultBufToFile(char *resultBuf, threadInfo *pThreadInfo);
void printfQuerySystemInfo(TAOS *taos);
#endif