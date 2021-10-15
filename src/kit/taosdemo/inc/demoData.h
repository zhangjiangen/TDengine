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
#ifndef _DEMODATA_H_
#define _DEMODATA_H_
#include "taosdemo.h"
void    init_rand_data();
int32_t rand_bool();
int32_t rand_tinyint();
int32_t rand_smallint();
double  rand_double();
float   rand_float();
int64_t rand_bigint();
int32_t rand_int();
char *  rand_double_str();
void    rand_string(char *str, int size);
int     prepareSampleData();
void    freeDataResource();
char *  generateBinaryNCharTagValues(int64_t tableSeq, uint32_t len);
int64_t getTSRandTail(int64_t timeStampStep, int32_t seq, int disorderRatio,
                      int disorderRange);

int32_t generateStbDataTail(SSuperTable *stbInfo, uint32_t batch, char *buffer,
                            int64_t remainderBufLen, int64_t insertRows,
                            uint64_t recordFrom, int64_t startTime,
                            int64_t *pSamplePos, int64_t *dataLen);
int64_t generateStbRowData(SSuperTable *stbInfo, char *recBuf,
                           int64_t remainderBufLen, int64_t timestamp);
int32_t generateProgressiveDataWithoutStb(
    char *tableName, threadInfo *pThreadInfo, char *buffer, int64_t insertRows,
    uint64_t recordFrom, int64_t startTime, int64_t *pRemainderBufLen);
int32_t generateStbProgressiveData(SSuperTable *stbInfo, char *tableName,
                                   int64_t tableSeq, char *dbName, char *buffer,
                                   int64_t insertRows, uint64_t recordFrom,
                                   int64_t startTime, int64_t *pSamplePos,
                                   int64_t *pRemainderBufLen);
int64_t generateInterlaceDataWithoutStb(char *tableName, uint32_t batch,
                                        uint64_t tableSeq, char *dbName,
                                        char *buffer, int64_t insertRows,
                                        int64_t   startTime,
                                        uint64_t *pRemainderBufLen);
int32_t generateStbInterlaceData(threadInfo *pThreadInfo, char *tableName,
                                 uint32_t batchPerTbl, uint64_t i,
                                 uint32_t batchPerTblTimes, uint64_t tableSeq,
                                 char *buffer, int64_t insertRows,
                                 int64_t startTime, uint64_t *pRemainderBufLen);
int32_t prepareStmtWithoutStb(threadInfo *pThreadInfo, char *tableName,
                              uint32_t batch, int64_t insertRows,
                              int64_t recordFrom, int64_t startTime);
UNUSED_FUNC int32_t prepareStbStmtRand(threadInfo *pThreadInfo, char *tableName,
                                       int64_t tableSeq, uint32_t batch,
                                       uint64_t insertRows, uint64_t recordFrom,
                                       int64_t startTime);
int32_t             prepareStbStmt(threadInfo *pThreadInfo, char *tableName,
                                   int64_t tableSeq, uint32_t batch, uint64_t insertRows,
                                   uint64_t recordFrom, int64_t startTime,
                                   int64_t *pSamplePos);
char *generateTagValuesForStb(SSuperTable *stbInfo, int64_t tableSeq);
char *getTagValueFromTagSample(SSuperTable *stbInfo, int tagUsePos);
int   parseNtbSampleToStmtBatchForThread(threadInfo *pThreadInfo,
                                         uint32_t timePrec, uint32_t batch);
int   prepareSampleForNtb();
int   prepareSampleForStb(SSuperTable *stbInfo);

extern char *g_sampleDataBuf;
extern char *g_sampleBindBatchArray;
#endif