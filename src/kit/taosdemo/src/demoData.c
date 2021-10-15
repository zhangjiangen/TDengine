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

#include "demoData.h"
#include "demoUtil.h"

void    getAndSetRowsFromCsvFile(SSuperTable *stbInfo);
int     generateSampleFromCsvForStb(SSuperTable *stbInfo);
void    getAndSetRowsFromCsvFile(SSuperTable *stbInfo);
int     generateSampleFromRand(char *sampleDataBuf, uint64_t lenOfOneRow,
                               int columnCount, StrColumn *columns);
int32_t generateDataTailWithoutStb(uint32_t batch, char *buffer,
                                   int64_t remainderBufLen, int64_t insertRows,
                                   uint64_t recordFrom, int64_t startTime,
                                   /* int64_t *pSamplePos, */ int64_t *dataLen);
int generateStbSQLHead(SSuperTable *stbInfo, char *tableName, int64_t tableSeq,
                       char *dbName, char *buffer, int remainderBufLen);
int generateSQLHeadWithoutStb(char *tableName, char *dbName, char *buffer,
                              int remainderBufLen);

/* Globale Variables */
char *g_sampleDataBuf = NULL;
#if STMT_BIND_PARAM_BATCH == 1
const char charset[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
// bind param batch
char *g_sampleBindBatchArray = NULL;
#endif

int32_t  g_randint[MAX_PREPARED_RAND];
uint32_t g_randuint[MAX_PREPARED_RAND];
int64_t  g_randbigint[MAX_PREPARED_RAND];
uint64_t g_randubigint[MAX_PREPARED_RAND];
float    g_randfloat[MAX_PREPARED_RAND];
double   g_randdouble[MAX_PREPARED_RAND];

char *g_randbool_buff = NULL;
char *g_randint_buff = NULL;
char *g_randuint_buff = NULL;
char *g_rand_voltage_buff = NULL;
char *g_randbigint_buff = NULL;
char *g_randubigint_buff = NULL;
char *g_randsmallint_buff = NULL;
char *g_randusmallint_buff = NULL;
char *g_randtinyint_buff = NULL;
char *g_randutinyint_buff = NULL;
char *g_randfloat_buff = NULL;
char *g_rand_current_buff = NULL;
char *g_rand_phase_buff = NULL;
char *g_randdouble_buff = NULL;

void init_rand_data() {
    g_randint_buff = calloc(1, INT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randint_buff);
    g_rand_voltage_buff = calloc(1, INT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_rand_voltage_buff);
    g_randbigint_buff = calloc(1, BIGINT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randbigint_buff);
    g_randsmallint_buff = calloc(1, SMALLINT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randsmallint_buff);
    g_randtinyint_buff = calloc(1, TINYINT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randtinyint_buff);
    g_randbool_buff = calloc(1, BOOL_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randbool_buff);
    g_randfloat_buff = calloc(1, FLOAT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randfloat_buff);
    g_rand_current_buff = calloc(1, FLOAT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_rand_current_buff);
    g_rand_phase_buff = calloc(1, FLOAT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_rand_phase_buff);
    g_randdouble_buff = calloc(1, DOUBLE_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randdouble_buff);
    g_randuint_buff = calloc(1, INT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randuint_buff);
    g_randutinyint_buff = calloc(1, TINYINT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randutinyint_buff);
    g_randusmallint_buff = calloc(1, SMALLINT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randusmallint_buff);
    g_randubigint_buff = calloc(1, BIGINT_BUFF_LEN * MAX_PREPARED_RAND);
    assert(g_randubigint_buff);

    for (int i = 0; i < MAX_PREPARED_RAND; i++) {
        g_randint[i] = (int)(taosRandom() % RAND_MAX - (RAND_MAX >> 1));
        g_randuint[i] = (int)(taosRandom());
        sprintf(g_randint_buff + i * INT_BUFF_LEN, "%d", g_randint[i]);
        sprintf(g_rand_voltage_buff + i * INT_BUFF_LEN, "%d",
                215 + g_randint[i] % 10);

        sprintf(g_randbool_buff + i * BOOL_BUFF_LEN, "%s",
                ((g_randint[i] % 2) & 1) ? "true" : "false");
        sprintf(g_randsmallint_buff + i * SMALLINT_BUFF_LEN, "%d",
                g_randint[i] % 32768);
        sprintf(g_randtinyint_buff + i * TINYINT_BUFF_LEN, "%d",
                g_randint[i] % 128);
        sprintf(g_randuint_buff + i * INT_BUFF_LEN, "%d", g_randuint[i]);
        sprintf(g_randusmallint_buff + i * SMALLINT_BUFF_LEN, "%d",
                g_randuint[i] % 65535);
        sprintf(g_randutinyint_buff + i * TINYINT_BUFF_LEN, "%d",
                g_randuint[i] % 255);

        g_randbigint[i] = (int64_t)(taosRandom() % RAND_MAX - (RAND_MAX >> 1));
        g_randubigint[i] = (uint64_t)(taosRandom());
        sprintf(g_randbigint_buff + i * BIGINT_BUFF_LEN, "%" PRId64 "",
                g_randbigint[i]);
        sprintf(g_randubigint_buff + i * BIGINT_BUFF_LEN, "%" PRId64 "",
                g_randubigint[i]);

        g_randfloat[i] =
            (float)(taosRandom() / 1000.0) * (taosRandom() % 2 > 0.5 ? 1 : -1);
        sprintf(g_randfloat_buff + i * FLOAT_BUFF_LEN, "%f", g_randfloat[i]);
        sprintf(g_rand_current_buff + i * FLOAT_BUFF_LEN, "%f",
                (float)(9.8 + 0.04 * (g_randint[i] % 10) +
                        g_randfloat[i] / 1000000000));
        sprintf(
            g_rand_phase_buff + i * FLOAT_BUFF_LEN, "%f",
            (float)((115 + g_randint[i] % 10 + g_randfloat[i] / 1000000000) /
                    360));

        g_randdouble[i] = (double)(taosRandom() / 1000000.0) *
                          (taosRandom() % 2 > 0.5 ? 1 : -1);
        sprintf(g_randdouble_buff + i * DOUBLE_BUFF_LEN, "%f", g_randdouble[i]);
    }
}

char *rand_bool_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randbool_buff + ((cursor % MAX_PREPARED_RAND) * BOOL_BUFF_LEN);
}

int32_t rand_bool() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randint[cursor % MAX_PREPARED_RAND] % 2;
}

char *rand_tinyint_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randtinyint_buff +
           ((cursor % MAX_PREPARED_RAND) * TINYINT_BUFF_LEN);
}

int32_t rand_tinyint() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randint[cursor % MAX_PREPARED_RAND] % 128;
}

char *rand_utinyint_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randutinyint_buff +
           ((cursor % MAX_PREPARED_RAND) * TINYINT_BUFF_LEN);
}

int32_t rand_utinyint() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randuint[cursor % MAX_PREPARED_RAND] % 255;
}

char *rand_smallint_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randsmallint_buff +
           ((cursor % MAX_PREPARED_RAND) * SMALLINT_BUFF_LEN);
}

int32_t rand_smallint() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randint[cursor % MAX_PREPARED_RAND] % 32768;
}

char *rand_usmallint_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randusmallint_buff +
           ((cursor % MAX_PREPARED_RAND) * SMALLINT_BUFF_LEN);
}

int32_t rand_usmallint() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randuint[cursor % MAX_PREPARED_RAND] % 65535;
}

char *rand_int_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randint_buff + ((cursor % MAX_PREPARED_RAND) * INT_BUFF_LEN);
}

int32_t rand_int() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randint[cursor % MAX_PREPARED_RAND];
}

char *rand_uint_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randuint_buff + ((cursor % MAX_PREPARED_RAND) * INT_BUFF_LEN);
}

int32_t rand_uint() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randuint[cursor % MAX_PREPARED_RAND];
}

char *rand_bigint_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randbigint_buff + ((cursor % MAX_PREPARED_RAND) * BIGINT_BUFF_LEN);
}

int64_t rand_bigint() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randbigint[cursor % MAX_PREPARED_RAND];
}

char *rand_ubigint_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randubigint_buff +
           ((cursor % MAX_PREPARED_RAND) * BIGINT_BUFF_LEN);
}

int64_t rand_ubigint() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randubigint[cursor % MAX_PREPARED_RAND];
}

char *rand_float_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randfloat_buff + ((cursor % MAX_PREPARED_RAND) * FLOAT_BUFF_LEN);
}

float rand_float() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randfloat[cursor % MAX_PREPARED_RAND];
}

char *demo_current_float_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_rand_current_buff +
           ((cursor % MAX_PREPARED_RAND) * FLOAT_BUFF_LEN);
}

float UNUSED_FUNC demo_current_float() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return (float)(9.8 + 0.04 * (g_randint[cursor % MAX_PREPARED_RAND] % 10) +
                   g_randfloat[cursor % MAX_PREPARED_RAND] / 1000000000);
}

char *demo_voltage_int_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_rand_voltage_buff + ((cursor % MAX_PREPARED_RAND) * INT_BUFF_LEN);
}

int32_t UNUSED_FUNC demo_voltage_int() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return 215 + g_randint[cursor % MAX_PREPARED_RAND] % 10;
}

char *demo_phase_float_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_rand_phase_buff + ((cursor % MAX_PREPARED_RAND) * FLOAT_BUFF_LEN);
}

float UNUSED_FUNC demo_phase_float() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return (float)((115 + g_randint[cursor % MAX_PREPARED_RAND] % 10 +
                    g_randfloat[cursor % MAX_PREPARED_RAND] / 1000000000) /
                   360);
}

int prepareSampleData() {
    for (int i = 0; i < g_Dbs.dbCount; i++) {
        for (int j = 0; j < g_Dbs.db[i].superTblCount; j++) {
            if (g_Dbs.db[i].superTbls[j].tagsFile[0] != 0) {
                if (readTagFromCsvFileToMem(&g_Dbs.db[i].superTbls[j]) != 0) {
                    return -1;
                }
            }
        }
    }

    return 0;
}

int generateSampleFromRandForNtb() {
    return generateSampleFromRand(g_sampleDataBuf, g_args.lenOfOneRow,
                                  g_args.columnCount, NULL);
}

int generateSampleFromRandForStb(SSuperTable *stbInfo) {
    return generateSampleFromRand(stbInfo->sampleDataBuf, stbInfo->lenOfOneRow,
                                  stbInfo->columnCount, stbInfo->columns);
}

int prepareSampleForNtb() {
    g_sampleDataBuf = calloc(g_args.lenOfOneRow * MAX_SAMPLES, 1);
    if (NULL == g_sampleDataBuf) {
        errorPrint2("%s() LN%d, Failed to calloc %" PRIu64
                    " Bytes, reason:%s\n",
                    __func__, __LINE__, g_args.lenOfOneRow * MAX_SAMPLES,
                    strerror(errno));
        return -1;
    }

    return generateSampleFromRandForNtb();
}

int generateSampleFromRand(char *sampleDataBuf, uint64_t lenOfOneRow,
                           int columnCount, StrColumn *columns) {
    char data[MAX_DATA_SIZE];
    memset(data, 0, MAX_DATA_SIZE);

    char *buff = malloc(lenOfOneRow);
    if (NULL == buff) {
        errorPrint2("%s() LN%d, memory allocation %" PRIu64 " bytes failed\n",
                    __func__, __LINE__, lenOfOneRow);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_SAMPLES; i++) {
        uint64_t pos = 0;
        memset(buff, 0, lenOfOneRow);

        for (int c = 0; c < columnCount; c++) {
            char *tmp = NULL;

            uint32_t dataLen;
            char     data_type =
                (columns) ? (columns[c].data_type) : g_args.data_type[c];

            switch (data_type) {
                case TSDB_DATA_TYPE_BINARY:
                    dataLen = (columns) ? columns[c].dataLen : g_args.binwidth;
                    rand_string(data, dataLen);
                    pos += sprintf(buff + pos, "%s,", data);
                    break;

                case TSDB_DATA_TYPE_NCHAR:
                    dataLen = (columns) ? columns[c].dataLen : g_args.binwidth;
                    rand_string(data, dataLen - 1);
                    pos += sprintf(buff + pos, "%s,", data);
                    break;

                case TSDB_DATA_TYPE_INT:
                    if ((g_args.demo_mode) && (c == 1)) {
                        tmp = demo_voltage_int_str();
                    } else {
                        tmp = rand_int_str();
                    }
                    pos += sprintf(buff + pos, "%s,", tmp);
                    break;

                case TSDB_DATA_TYPE_UINT:
                    pos += sprintf(buff + pos, "%s,", rand_uint_str());
                    break;

                case TSDB_DATA_TYPE_BIGINT:
                    pos += sprintf(buff + pos, "%s,", rand_bigint_str());
                    break;

                case TSDB_DATA_TYPE_UBIGINT:
                    pos += sprintf(buff + pos, "%s,", rand_ubigint_str());
                    break;

                case TSDB_DATA_TYPE_FLOAT:
                    if (g_args.demo_mode) {
                        if (c == 0) {
                            tmp = demo_current_float_str();
                        } else {
                            tmp = demo_phase_float_str();
                        }
                    } else {
                        tmp = rand_float_str();
                    }
                    pos += sprintf(buff + pos, "%s,", tmp);
                    break;

                case TSDB_DATA_TYPE_DOUBLE:
                    pos += sprintf(buff + pos, "%s,", rand_double_str());
                    break;

                case TSDB_DATA_TYPE_SMALLINT:
                    pos += sprintf(buff + pos, "%s,", rand_smallint_str());
                    break;

                case TSDB_DATA_TYPE_USMALLINT:
                    pos += sprintf(buff + pos, "%s,", rand_usmallint_str());
                    break;

                case TSDB_DATA_TYPE_TINYINT:
                    pos += sprintf(buff + pos, "%s,", rand_tinyint_str());
                    break;

                case TSDB_DATA_TYPE_UTINYINT:
                    pos += sprintf(buff + pos, "%s,", rand_utinyint_str());
                    break;

                case TSDB_DATA_TYPE_BOOL:
                    pos += sprintf(buff + pos, "%s,", rand_bool_str());
                    break;

                case TSDB_DATA_TYPE_TIMESTAMP:
                    pos += sprintf(buff + pos, "%s,", rand_bigint_str());
                    break;

                case TSDB_DATA_TYPE_NULL:
                    break;

                default:
                    errorPrint2(
                        "%s() LN%d, Unknown data type %s\n", __func__, __LINE__,
                        (columns) ? (columns[c].dataType) : g_args.dataType[c]);
                    exit(EXIT_FAILURE);
            }
        }

        *(buff + pos - 1) = 0;
        memcpy(sampleDataBuf + i * lenOfOneRow, buff, pos);
    }

    free(buff);
    return 0;
}

char *getTagValueFromTagSample(SSuperTable *stbInfo, int tagUsePos) {
    char *dataBuf = (char *)calloc(TSDB_MAX_SQL_LEN + 1, 1);
    if (NULL == dataBuf) {
        errorPrint2("%s() LN%d, calloc failed! size:%d\n", __func__, __LINE__,
                    TSDB_MAX_SQL_LEN + 1);
        return NULL;
    }

    int dataLen = 0;
    dataLen +=
        snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen, "(%s)",
                 stbInfo->tagDataBuf + stbInfo->lenOfTagOfOneRow * tagUsePos);

    return dataBuf;
}

char *generateBinaryNCharTagValues(int64_t tableSeq, uint32_t len) {
    char *buf = (char *)calloc(len, 1);
    if (NULL == buf) {
        printf("calloc failed! size:%d\n", len);
        return NULL;
    }

    if (tableSeq % 2) {
        tstrncpy(buf, "beijing", len);
    } else {
        tstrncpy(buf, "shanghai", len);
    }
    // rand_string(buf, stbInfo->tags[i].dataLen);

    return buf;
}

int prepareSampleForStb(SSuperTable *stbInfo) {
    stbInfo->sampleDataBuf = calloc(stbInfo->lenOfOneRow * MAX_SAMPLES, 1);
    if (NULL == stbInfo->sampleDataBuf) {
        errorPrint2("%s() LN%d, Failed to calloc %" PRIu64
                    " Bytes, reason:%s\n",
                    __func__, __LINE__, stbInfo->lenOfOneRow * MAX_SAMPLES,
                    strerror(errno));
        return -1;
    }

    int ret;
    if (0 == strncasecmp(stbInfo->dataSource, "sample", strlen("sample"))) {
        if (stbInfo->useSampleTs) {
            getAndSetRowsFromCsvFile(stbInfo);
        }
        ret = generateSampleFromCsvForStb(stbInfo);
    } else {
        ret = generateSampleFromRandForStb(stbInfo);
    }

    if (0 != ret) {
        errorPrint2("%s() LN%d, read sample from csv file failed.\n", __func__,
                    __LINE__);
        tmfree(stbInfo->sampleDataBuf);
        stbInfo->sampleDataBuf = NULL;
        return -1;
    }

    return 0;
}

void getAndSetRowsFromCsvFile(SSuperTable *stbInfo) {
    FILE *fp = fopen(stbInfo->sampleFile, "r");
    int   line_count = 0;
    if (fp == NULL) {
        errorPrint("Failed to open sample file: %s, reason:%s\n",
                   stbInfo->sampleFile, strerror(errno));
        exit(EXIT_FAILURE);
    }
    char *buf = calloc(1, stbInfo->maxSqlLen);
    while (fgets(buf, stbInfo->maxSqlLen, fp)) {
        line_count++;
    }
    fclose(fp);
    tmfree(buf);
    stbInfo->insertRows = line_count;
}

/*
   Read 10000 lines at most. If more than 10000 lines, continue to read after
   using
   */
int generateSampleFromCsvForStb(SSuperTable *stbInfo) {
    size_t  n = 0;
    ssize_t readLen = 0;
    char *  line = NULL;
    int     getRows = 0;

    FILE *fp = fopen(stbInfo->sampleFile, "r");
    if (fp == NULL) {
        errorPrint("Failed to open sample file: %s, reason:%s\n",
                   stbInfo->sampleFile, strerror(errno));
        return -1;
    }

    assert(stbInfo->sampleDataBuf);
    memset(stbInfo->sampleDataBuf, 0, MAX_SAMPLES * stbInfo->lenOfOneRow);
    while (1) {
        readLen = tgetline(&line, &n, fp);
        if (-1 == readLen) {
            if (0 != fseek(fp, 0, SEEK_SET)) {
                errorPrint("Failed to fseek file: %s, reason:%s\n",
                           stbInfo->sampleFile, strerror(errno));
                fclose(fp);
                return -1;
            }
            continue;
        }

        if (('\r' == line[readLen - 1]) || ('\n' == line[readLen - 1])) {
            line[--readLen] = 0;
        }

        if (readLen == 0) {
            continue;
        }

        if (readLen > stbInfo->lenOfOneRow) {
            printf("sample row len[%d] overflow define schema len[%" PRIu64
                   "], so discard this row\n",
                   (int32_t)readLen, stbInfo->lenOfOneRow);
            continue;
        }

        memcpy(stbInfo->sampleDataBuf + getRows * stbInfo->lenOfOneRow, line,
               readLen);
        getRows++;

        if (getRows == MAX_SAMPLES) {
            break;
        }
    }

    fclose(fp);
    tmfree(line);
    return 0;
}

int32_t generateStbInterlaceData(threadInfo *pThreadInfo, char *tableName,
                                 uint32_t batchPerTbl, uint64_t i,
                                 uint32_t batchPerTblTimes, uint64_t tableSeq,
                                 char *buffer, int64_t insertRows,
                                 int64_t   startTime,
                                 uint64_t *pRemainderBufLen) {
    assert(buffer);
    char *pstr = buffer;

    SSuperTable *stbInfo = pThreadInfo->stbInfo;
    int          headLen =
        generateStbSQLHead(stbInfo, tableName, tableSeq, pThreadInfo->db_name,
                           pstr, *pRemainderBufLen);

    if (headLen <= 0) {
        return 0;
    }
    // generate data buffer
    verbosePrint("[%d] %s() LN%d i=%" PRIu64 " buffer:\n%s\n",
                 pThreadInfo->threadID, __func__, __LINE__, i, buffer);

    pstr += headLen;
    *pRemainderBufLen -= headLen;

    int64_t dataLen = 0;

    verbosePrint("[%d] %s() LN%d i=%" PRIu64
                 " batchPerTblTimes=%u batchPerTbl = %u\n",
                 pThreadInfo->threadID, __func__, __LINE__, i, batchPerTblTimes,
                 batchPerTbl);

    if (0 == strncasecmp(stbInfo->startTimestamp, "now", 3)) {
        startTime = taosGetTimestamp(pThreadInfo->time_precision);
    }

    int32_t k = generateStbDataTail(stbInfo, batchPerTbl, pstr,
                                    *pRemainderBufLen, insertRows, 0, startTime,
                                    &(pThreadInfo->samplePos), &dataLen);

    if (k == batchPerTbl) {
        pstr += dataLen;
        *pRemainderBufLen -= dataLen;
    } else {
        debugPrint(
            "%s() LN%d, generated data tail: %u, not equal batch per table: "
            "%u\n",
            __func__, __LINE__, k, batchPerTbl);
        pstr -= headLen;
        pstr[0] = '\0';
        k = 0;
    }

    return k;
}

int64_t generateInterlaceDataWithoutStb(char *tableName, uint32_t batch,
                                        uint64_t tableSeq, char *dbName,
                                        char *buffer, int64_t insertRows,
                                        int64_t   startTime,
                                        uint64_t *pRemainderBufLen) {
    assert(buffer);
    char *pstr = buffer;

    int headLen =
        generateSQLHeadWithoutStb(tableName, dbName, pstr, *pRemainderBufLen);

    if (headLen <= 0) {
        return 0;
    }

    pstr += headLen;
    *pRemainderBufLen -= headLen;

    int64_t dataLen = 0;

    int32_t k = generateDataTailWithoutStb(batch, pstr, *pRemainderBufLen,
                                           insertRows, 0, startTime, &dataLen);

    if (k == batch) {
        pstr += dataLen;
        *pRemainderBufLen -= dataLen;
    } else {
        debugPrint(
            "%s() LN%d, generated data tail: %d, not equal batch per table: "
            "%u\n",
            __func__, __LINE__, k, batch);
        pstr -= headLen;
        pstr[0] = '\0';
        k = 0;
    }

    return k;
}

void rand_string(char *str, int size) {
    str[0] = 0;
    if (size > 0) {
        //--size;
        int n;
        for (n = 0; n < size; n++) {
            int key = abs(rand_tinyint()) % (int)(sizeof(charset) - 1);
            str[n] = charset[key];
        }
        str[n] = 0;
    }
}

char *rand_double_str() {
    static int cursor;
    cursor++;
    if (cursor > (MAX_PREPARED_RAND - 1)) cursor = 0;
    return g_randdouble_buff + (cursor * DOUBLE_BUFF_LEN);
}

double rand_double() {
    static int cursor;
    cursor++;
    cursor = cursor % MAX_PREPARED_RAND;
    return g_randdouble[cursor];
}

int getRowDataFromSample(char *dataBuf, int64_t maxLen, int64_t timestamp,
                         SSuperTable *stbInfo, int64_t *sampleUsePos) {
    if ((*sampleUsePos) == MAX_SAMPLES) {
        *sampleUsePos = 0;
    }

    int dataLen = 0;
    if (stbInfo->useSampleTs) {
        dataLen += snprintf(
            dataBuf + dataLen, maxLen - dataLen, "(%s",
            stbInfo->sampleDataBuf + stbInfo->lenOfOneRow * (*sampleUsePos));
    } else {
        dataLen += snprintf(dataBuf + dataLen, maxLen - dataLen,
                            "(%" PRId64 ", ", timestamp);
        dataLen += snprintf(
            dataBuf + dataLen, maxLen - dataLen, "%s",
            stbInfo->sampleDataBuf + stbInfo->lenOfOneRow * (*sampleUsePos));
    }

    dataLen += snprintf(dataBuf + dataLen, maxLen - dataLen, ")");

    (*sampleUsePos)++;

    return dataLen;
}

int64_t generateStbRowData(SSuperTable *stbInfo, char *recBuf,
                           int64_t remainderBufLen, int64_t timestamp) {
    int64_t dataLen = 0;
    char *  pstr = recBuf;
    int64_t maxLen = MAX_DATA_SIZE;
    int     tmpLen;

    dataLen +=
        snprintf(pstr + dataLen, maxLen - dataLen, "(%" PRId64 "", timestamp);

    for (int i = 0; i < stbInfo->columnCount; i++) {
        tstrncpy(pstr + dataLen, ",", 2);
        dataLen += 1;

        if ((stbInfo->columns[i].data_type == TSDB_DATA_TYPE_BINARY) ||
            (stbInfo->columns[i].data_type == TSDB_DATA_TYPE_NCHAR)) {
            if (stbInfo->columns[i].dataLen > TSDB_MAX_BINARY_LEN) {
                errorPrint2("binary or nchar length overflow, max size:%u\n",
                            (uint32_t)TSDB_MAX_BINARY_LEN);
                return -1;
            }

            if ((stbInfo->columns[i].dataLen + 1) >
                /* need count 3 extra chars \', \', and , */
                (remainderBufLen - dataLen - 3)) {
                return 0;
            }
            char *buf = (char *)calloc(stbInfo->columns[i].dataLen + 1, 1);
            if (NULL == buf) {
                errorPrint2("calloc failed! size:%d\n",
                            stbInfo->columns[i].dataLen);
                return -1;
            }
            rand_string(buf, stbInfo->columns[i].dataLen);
            dataLen +=
                snprintf(pstr + dataLen, maxLen - dataLen, "\'%s\'", buf);
            tmfree(buf);

        } else {
            char *tmp = NULL;
            switch (stbInfo->columns[i].data_type) {
                case TSDB_DATA_TYPE_INT:
                    if ((g_args.demo_mode) && (i == 1)) {
                        tmp = demo_voltage_int_str();
                    } else {
                        tmp = rand_int_str();
                    }
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, INT_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_UINT:
                    tmp = rand_uint_str();
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, INT_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_BIGINT:
                    tmp = rand_bigint_str();
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, BIGINT_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_UBIGINT:
                    tmp = rand_ubigint_str();
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, BIGINT_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_FLOAT:
                    if (g_args.demo_mode) {
                        if (i == 0) {
                            tmp = demo_current_float_str();
                        } else {
                            tmp = demo_phase_float_str();
                        }
                    } else {
                        tmp = rand_float_str();
                    }
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, FLOAT_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_DOUBLE:
                    tmp = rand_double_str();
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, DOUBLE_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_SMALLINT:
                    tmp = rand_smallint_str();
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, SMALLINT_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_USMALLINT:
                    tmp = rand_usmallint_str();
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, SMALLINT_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_TINYINT:
                    tmp = rand_tinyint_str();
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, TINYINT_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_UTINYINT:
                    tmp = rand_utinyint_str();
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, TINYINT_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_BOOL:
                    tmp = rand_bool_str();
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, BOOL_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_TIMESTAMP:
                    tmp = rand_bigint_str();
                    tmpLen = strlen(tmp);
                    tstrncpy(pstr + dataLen, tmp,
                             min(tmpLen + 1, BIGINT_BUFF_LEN));
                    break;

                case TSDB_DATA_TYPE_NULL:
                    break;

                default:
                    errorPrint2("Not support data type: %s\n",
                                stbInfo->columns[i].dataType);
                    exit(EXIT_FAILURE);
            }
            if (tmp) {
                dataLen += tmpLen;
            }
        }

        if (dataLen > (remainderBufLen - (128))) return 0;
    }

    dataLen += snprintf(pstr + dataLen, 2, ")");

    verbosePrint("%s() LN%d, dataLen:%" PRId64 "\n", __func__, __LINE__,
                 dataLen);
    verbosePrint("%s() LN%d, recBuf:\n\t%s\n", __func__, __LINE__, recBuf);

    return strlen(recBuf);
}

int64_t generateData(char *recBuf, char *data_type, int64_t timestamp,
                     int lenOfBinary) {
    memset(recBuf, 0, MAX_DATA_SIZE);
    char *pstr = recBuf;
    pstr += sprintf(pstr, "(%" PRId64 "", timestamp);

    int columnCount = g_args.columnCount;

    bool  b;
    char *s;
    for (int i = 0; i < columnCount; i++) {
        switch (data_type[i]) {
            case TSDB_DATA_TYPE_TINYINT:
                pstr += sprintf(pstr, ",%d", rand_tinyint());
                break;

            case TSDB_DATA_TYPE_SMALLINT:
                pstr += sprintf(pstr, ",%d", rand_smallint());
                break;

            case TSDB_DATA_TYPE_INT:
                pstr += sprintf(pstr, ",%d", rand_int());
                break;

            case TSDB_DATA_TYPE_BIGINT:
                pstr += sprintf(pstr, ",%" PRId64 "", rand_bigint());
                break;

            case TSDB_DATA_TYPE_TIMESTAMP:
                pstr += sprintf(pstr, ",%" PRId64 "", rand_bigint());
                break;

            case TSDB_DATA_TYPE_FLOAT:
                pstr += sprintf(pstr, ",%10.4f", rand_float());
                break;

            case TSDB_DATA_TYPE_DOUBLE:
                pstr += sprintf(pstr, ",%20.8f", rand_double());
                break;

            case TSDB_DATA_TYPE_BOOL:
                b = rand_bool() & 1;
                pstr += sprintf(pstr, ",%s", b ? "true" : "false");
                break;

            case TSDB_DATA_TYPE_BINARY:
                s = malloc(lenOfBinary + 1);
                if (s == NULL) {
                    errorPrint2(
                        "%s() LN%d, memory allocation %d bytes failed\n",
                        __func__, __LINE__, lenOfBinary + 1);
                    exit(EXIT_FAILURE);
                }
                rand_string(s, lenOfBinary);
                pstr += sprintf(pstr, ",\"%s\"", s);
                free(s);
                break;

            case TSDB_DATA_TYPE_NCHAR:
                s = malloc(lenOfBinary + 1);
                if (s == NULL) {
                    errorPrint2(
                        "%s() LN%d, memory allocation %d bytes failed\n",
                        __func__, __LINE__, lenOfBinary + 1);
                    exit(EXIT_FAILURE);
                }
                rand_string(s, lenOfBinary);
                pstr += sprintf(pstr, ",\"%s\"", s);
                free(s);
                break;

            case TSDB_DATA_TYPE_UTINYINT:
                pstr += sprintf(pstr, ",%d", rand_utinyint());
                break;

            case TSDB_DATA_TYPE_USMALLINT:
                pstr += sprintf(pstr, ",%d", rand_usmallint());
                break;

            case TSDB_DATA_TYPE_UINT:
                pstr += sprintf(pstr, ",%d", rand_uint());
                break;

            case TSDB_DATA_TYPE_UBIGINT:
                pstr += sprintf(pstr, ",%" PRId64 "", rand_ubigint());
                break;

            case TSDB_DATA_TYPE_NULL:
                break;

            default:
                errorPrint2("%s() LN%d, Unknown data type %d\n", __func__,
                            __LINE__, data_type[i]);
                exit(EXIT_FAILURE);
        }

        if (strlen(recBuf) > MAX_DATA_SIZE) {
            ERROR_EXIT("column length too long, abort");
        }
    }

    pstr += sprintf(pstr, ")");

    verbosePrint("%s() LN%d, recBuf:\n\t%s\n", __func__, __LINE__, recBuf);

    return (int32_t)strlen(recBuf);
}

int32_t generateProgressiveDataWithoutStb(
    char *tableName, threadInfo *pThreadInfo, char *buffer, int64_t insertRows,
    uint64_t recordFrom, int64_t startTime, int64_t *pRemainderBufLen) {
    assert(buffer != NULL);
    char *pstr = buffer;

    memset(buffer, 0, *pRemainderBufLen);

    int64_t headLen = generateSQLHeadWithoutStb(tableName, pThreadInfo->db_name,
                                                buffer, *pRemainderBufLen);

    if (headLen <= 0) {
        return 0;
    }
    pstr += headLen;
    *pRemainderBufLen -= headLen;

    int64_t dataLen;

    return generateDataTailWithoutStb(g_args.reqPerReq, pstr, *pRemainderBufLen,
                                      insertRows, recordFrom, startTime,
                                      /*pSamplePos, */ &dataLen);
}

int32_t generateStbProgressiveData(SSuperTable *stbInfo, char *tableName,
                                   int64_t tableSeq, char *dbName, char *buffer,
                                   int64_t insertRows, uint64_t recordFrom,
                                   int64_t startTime, int64_t *pSamplePos,
                                   int64_t *pRemainderBufLen) {
    assert(buffer != NULL);
    char *pstr = buffer;

    memset(pstr, 0, *pRemainderBufLen);

    int64_t headLen = generateStbSQLHead(stbInfo, tableName, tableSeq, dbName,
                                         buffer, *pRemainderBufLen);

    if (headLen <= 0) {
        return 0;
    }
    pstr += headLen;
    *pRemainderBufLen -= headLen;

    int64_t dataLen;

    return generateStbDataTail(stbInfo, g_args.reqPerReq, pstr,
                               *pRemainderBufLen, insertRows, recordFrom,
                               startTime, pSamplePos, &dataLen);
}

int generateStbSQLHead(SSuperTable *stbInfo, char *tableName, int64_t tableSeq,
                       char *dbName, char *buffer, int remainderBufLen) {
    int len;

    char headBuf[HEAD_BUFF_LEN];

    if (AUTO_CREATE_SUBTBL == stbInfo->autoCreateTable) {
        char *tagsValBuf = NULL;
        if (0 == stbInfo->tagSource) {
            tagsValBuf = generateTagValuesForStb(stbInfo, tableSeq);
        } else {
            tagsValBuf = getTagValueFromTagSample(
                stbInfo, tableSeq % stbInfo->tagSampleCount);
        }
        if (NULL == tagsValBuf) {
            errorPrint2("%s() LN%d, tag buf failed to allocate  memory\n",
                        __func__, __LINE__);
            return -1;
        }

        len =
            snprintf(headBuf, HEAD_BUFF_LEN, "%s.%s using %s.%s TAGS%s values",
                     dbName, tableName, dbName, stbInfo->stbName, tagsValBuf);
        tmfree(tagsValBuf);
    } else if (TBL_ALREADY_EXISTS == stbInfo->childTblExists) {
        len =
            snprintf(headBuf, HEAD_BUFF_LEN, "%s.%s values", dbName, tableName);
    } else {
        len =
            snprintf(headBuf, HEAD_BUFF_LEN, "%s.%s values", dbName, tableName);
    }

    if (len > remainderBufLen) return -1;

    tstrncpy(buffer, headBuf, len + 1);

    return len;
}

int generateSQLHeadWithoutStb(char *tableName, char *dbName, char *buffer,
                              int remainderBufLen) {
    int len;

    char headBuf[HEAD_BUFF_LEN];

    len = snprintf(headBuf, HEAD_BUFF_LEN, "%s.%s values", dbName, tableName);

    if (len > remainderBufLen) return -1;

    tstrncpy(buffer, headBuf, len + 1);

    return len;
}

int32_t generateStbDataTail(SSuperTable *stbInfo, uint32_t batch, char *buffer,
                            int64_t remainderBufLen, int64_t insertRows,
                            uint64_t recordFrom, int64_t startTime,
                            int64_t *pSamplePos, int64_t *dataLen) {
    uint64_t len = 0;

    char *pstr = buffer;

    bool tsRand;
    if (0 == strncasecmp(stbInfo->dataSource, "rand", strlen("rand"))) {
        tsRand = true;
    } else {
        tsRand = false;
    }
    verbosePrint("%s() LN%d batch=%u buflen=%" PRId64 "\n", __func__, __LINE__,
                 batch, remainderBufLen);

    int32_t k;
    for (k = 0; k < batch;) {
        char *data = pstr;

        int64_t lenOfRow = 0;

        if (tsRand) {
            if (stbInfo->disorderRatio > 0) {
                lenOfRow = generateStbRowData(
                    stbInfo, data, remainderBufLen,
                    startTime + getTSRandTail(stbInfo->timeStampStep, k,
                                              stbInfo->disorderRatio,
                                              stbInfo->disorderRange));
            } else {
                lenOfRow =
                    generateStbRowData(stbInfo, data, remainderBufLen,
                                       startTime + stbInfo->timeStampStep * k);
            }
        } else {
            lenOfRow = getRowDataFromSample(
                data,
                (remainderBufLen < MAX_DATA_SIZE) ? remainderBufLen
                                                  : MAX_DATA_SIZE,
                startTime + stbInfo->timeStampStep * k, stbInfo, pSamplePos);
        }

        if (lenOfRow == 0) {
            data[0] = '\0';
            break;
        }
        if ((lenOfRow + 1) > remainderBufLen) {
            break;
        }

        pstr += lenOfRow;
        k++;
        len += lenOfRow;
        remainderBufLen -= lenOfRow;

        verbosePrint("%s() LN%d len=%" PRIu64 " k=%u \nbuffer=%s\n", __func__,
                     __LINE__, len, k, buffer);

        recordFrom++;

        if (recordFrom >= insertRows) {
            break;
        }
    }

    *dataLen = len;
    return k;
}

int64_t getTSRandTail(int64_t timeStampStep, int32_t seq, int disorderRatio,
                      int disorderRange) {
    int64_t randTail = timeStampStep * seq;
    if (disorderRatio > 0) {
        int rand_num = taosRandom() % 100;
        if (rand_num < disorderRatio) {
            randTail = (randTail + (taosRandom() % disorderRange + 1)) * (-1);
            debugPrint("rand data generated, back %" PRId64 "\n", randTail);
        }
    }

    return randTail;
}

int32_t generateDataTailWithoutStb(
    uint32_t batch, char *buffer, int64_t remainderBufLen, int64_t insertRows,
    uint64_t recordFrom, int64_t startTime,
    /* int64_t *pSamplePos, */ int64_t *dataLen) {
    uint64_t len = 0;
    char *   pstr = buffer;

    verbosePrint("%s() LN%d batch=%d\n", __func__, __LINE__, batch);

    int32_t k = 0;
    for (k = 0; k < batch;) {
        char *data = pstr;
        memset(data, 0, MAX_DATA_SIZE);

        int64_t retLen = 0;

        char *data_type = g_args.data_type;
        int   lenOfBinary = g_args.binwidth;

        if (g_args.disorderRatio) {
            retLen =
                generateData(data, data_type,
                             startTime + getTSRandTail(g_args.timestamp_step, k,
                                                       g_args.disorderRatio,
                                                       g_args.disorderRange),
                             lenOfBinary);
        } else {
            retLen = generateData(data, data_type,
                                  startTime + g_args.timestamp_step * k,
                                  lenOfBinary);
        }

        if (len > remainderBufLen) break;

        pstr += retLen;
        k++;
        len += retLen;
        remainderBufLen -= retLen;

        verbosePrint("%s() LN%d len=%" PRIu64 " k=%d \nbuffer=%s\n", __func__,
                     __LINE__, len, k, buffer);

        recordFrom++;

        if (recordFrom >= insertRows) {
            break;
        }
    }

    *dataLen = len;
    return k;
}

char *generateTagValuesForStb(SSuperTable *stbInfo, int64_t tableSeq) {
    char *dataBuf = (char *)calloc(TSDB_MAX_SQL_LEN + 1, 1);
    if (NULL == dataBuf) {
        printf("calloc failed! size:%d\n", TSDB_MAX_SQL_LEN + 1);
        return NULL;
    }

    int dataLen = 0;
    dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen, "(");
    for (int i = 0; i < stbInfo->tagCount; i++) {
        if ((0 == strncasecmp(stbInfo->tags[i].dataType, "binary",
                              strlen("binary"))) ||
            (0 == strncasecmp(stbInfo->tags[i].dataType, "nchar",
                              strlen("nchar")))) {
            if (stbInfo->tags[i].dataLen > TSDB_MAX_BINARY_LEN) {
                printf("binary or nchar length overflow, max size:%u\n",
                       (uint32_t)TSDB_MAX_BINARY_LEN);
                tmfree(dataBuf);
                return NULL;
            }

            int32_t tagBufLen = stbInfo->tags[i].dataLen + 1;
            char *  buf = generateBinaryNCharTagValues(tableSeq, tagBufLen);
            if (NULL == buf) {
                tmfree(dataBuf);
                return NULL;
            }
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "\'%s\',", buf);
            tmfree(buf);
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "int",
                                    strlen("int"))) {
            if ((g_args.demo_mode) && (i == 0)) {
                dataLen +=
                    snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                             "%" PRId64 ",", (tableSeq % 10) + 1);
            } else {
                dataLen +=
                    snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                             "%" PRId64 ",", tableSeq);
            }
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "bigint",
                                    strlen("bigint"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%" PRId64 ",", rand_bigint());
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "float",
                                    strlen("float"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%f,", rand_float());
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "double",
                                    strlen("double"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%f,", rand_double());
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "smallint",
                                    strlen("smallint"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%d,", rand_smallint());
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "tinyint",
                                    strlen("tinyint"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%d,", rand_tinyint());
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "bool",
                                    strlen("bool"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%d,", rand_bool());
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "timestamp",
                                    strlen("timestamp"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%" PRId64 ",", rand_ubigint());
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "utinyint",
                                    strlen("utinyint"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%d,", rand_utinyint());
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "usmallint",
                                    strlen("usmallint"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%d,", rand_usmallint());
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "uint",
                                    strlen("uint"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%d,", rand_uint());
        } else if (0 == strncasecmp(stbInfo->tags[i].dataType, "ubigint",
                                    strlen("ubigint"))) {
            dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen,
                                "%" PRId64 ",", rand_ubigint());
        } else {
            errorPrint2("No support data type: %s\n",
                        stbInfo->tags[i].dataType);
            tmfree(dataBuf);
            return NULL;
        }
    }

    dataLen -= 1;
    dataLen += snprintf(dataBuf + dataLen, TSDB_MAX_SQL_LEN - dataLen, ")");
    return dataBuf;
}

void freeDataResource() {
    tmfree(g_randbool_buff);
    tmfree(g_randint_buff);
    tmfree(g_rand_voltage_buff);
    tmfree(g_randbigint_buff);
    tmfree(g_randsmallint_buff);
    tmfree(g_randtinyint_buff);
    tmfree(g_randfloat_buff);
    tmfree(g_rand_current_buff);
    tmfree(g_rand_phase_buff);

    tmfree(g_sampleDataBuf);

#if STMT_BIND_PARAM_BATCH == 1
    for (int l = 0; l < g_args.columnCount; l++) {
        if (g_sampleBindBatchArray) {
            tmfree((char *)((uintptr_t) * (uintptr_t *)(g_sampleBindBatchArray +
                                                        sizeof(char *) * l)));
        }
    }
    tmfree(g_sampleBindBatchArray);
#endif
}