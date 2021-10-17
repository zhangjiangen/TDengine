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
#ifndef _DEMOUTIL_H_
#define _DEMOUTIL_H_
#include "demoData.h"
#include "demoOutput.h"
#include "taosdemo.h"
void prompt();
int  taosRandom();
void setupForAnsiEscape(void);
void resetAfterAnsiEscape(void);
int  queryDbExec(TAOS *taos, char *command, QUERY_TYPE type, bool quiet);
void postFreeResource();
int  readTagFromCsvFileToMem(SSuperTable *stbInfo);
void ERROR_EXIT(const char *msg);

/* Function to do regular expression check */
int regexMatch(const char *s, const char *reg, int cflags);

int isCommentLine(char *line);

bool  isStringNumber(char *input);
void  tmfclose(FILE *fp);
void  tmfree(char *buf);
char *formatTimestamp(char *buf, int64_t val, int precision);
int   getDbFromServer(TAOS *taos, SDbInfo **dbInfos);
int   postProceSql(char *host, struct sockaddr_in *pServAddr, uint16_t port,
                   char *sqlstr, threadInfo *pThreadInfo);
int   convertHostToServAddr(char *host, uint16_t port,
                            struct sockaddr_in *serv_addr);
void  replaceChildTblName(char *inSql, char *outSql, int tblIndex);
void  fetchResult(TAOS_RES *res, threadInfo *pThreadInfo);
void getTableName(char *pTblName, SNormalTable *tbInfo, threadInfo *pThreadInfo,
                  uint64_t tableSeq);
int  getAllChildNameOfSuperTable(TAOS *taos, char *dbName, char *stbName,
                                 char **  childTblNameOfSuperTbl,
                                 int64_t *childTblCountOfSuperTbl);
int  getChildNameOfSuperTableWithLimitAndOffset(TAOS *taos, char *dbName,
                                                char *   stbName,
                                                char **  childTblNameOfSuperTbl,
                                                int64_t *childTblCountOfSuperTbl,
                                                int64_t limit, uint64_t offset);
#endif