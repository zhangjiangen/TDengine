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

/*
   when in some thread query return error, thread don't exit, but return,
   otherwise coredump in other thread.
   */
#ifndef _TAOSDEMO_H_
#define _TAOSDEMO_H_

#include <stdint.h>
#include <taos.h>
#include <taoserror.h>
#define _GNU_SOURCE
#define CURL_STATICLIB

#ifdef LINUX
#include <argp.h>
#include <inttypes.h>
#ifndef _ALPINE
#include <error.h>
#endif
#include <pthread.h>
#include <regex.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wordexp.h>
#else
#include <regex.h>
#include <stdio.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include "cJSON.h"

#include <time.h>
#include "os.h"
#include "taos.h"
#include "taoserror.h"
#include "tutil.h"

#define REQ_EXTRA_BUF_LEN 1024
#define RESP_BUF_LEN 4096

extern char configDir[];

#define STR_INSERT_INTO "INSERT INTO "

#define MAX_RECORDS_PER_REQ 32766

#define HEAD_BUFF_LEN \
    TSDB_MAX_COLUMNS * 24  // 16*MAX_COLUMNS + (192+32)*2 + insert into ..

#define BUFFER_SIZE TSDB_MAX_ALLOWED_SQL_LEN
#define COND_BUF_LEN (BUFFER_SIZE - 30)
#define COL_BUFFER_LEN ((TSDB_COL_NAME_LEN + 15) * TSDB_MAX_COLUMNS)

#define MAX_USERNAME_SIZE 64
#define MAX_HOSTNAME_SIZE \
    253  // https://man7.org/linux/man-pages/man7/hostname.7.html
#define MAX_TB_NAME_SIZE 64
#define MAX_DATA_SIZE \
    (16 * TSDB_MAX_COLUMNS) + 20  // max record len: 16*MAX_COLUMNS, timestamp
                                  // string and ,('') need extra space
#define OPT_ABORT 1               /* –abort */
#define MAX_FILE_NAME_LEN 256     // max file name length on linux is 255.

#define DEFAULT_START_TIME 1500000000000

#define MAX_PREPARED_RAND 1000000
#define INT_BUFF_LEN 12
#define BIGINT_BUFF_LEN 21
#define TINYINT_BUFF_LEN 5
#define SMALLINT_BUFF_LEN 7
#define BOOL_BUFF_LEN 6
#define FLOAT_BUFF_LEN 22
#define DOUBLE_BUFF_LEN 42
#define TIMESTAMP_BUFF_LEN 21

#define MAX_SAMPLES 10000
#define MAX_NUM_COLUMNS \
    (TSDB_MAX_COLUMNS - 1)  // exclude first column timestamp

#define MAX_DB_COUNT 8
#define MAX_SUPER_TABLE_COUNT 200

#define MAX_QUERY_SQL_COUNT 100

#define MAX_DATABASE_COUNT 256
#define INPUT_BUF_LEN 256

#define TBNAME_PREFIX_LEN \
    (TSDB_TABLE_NAME_LEN - 20)  // 20 characters reserved for seq
#define SMALL_BUFF_LEN 8
#define DATATYPE_BUFF_LEN (SMALL_BUFF_LEN * 3)
#define NOTE_BUFF_LEN (SMALL_BUFF_LEN * 16)

#define DEFAULT_NTHREADS 8
#define DEFAULT_TIMESTAMP_STEP 1
#define DEFAULT_INTERLACE_ROWS 0
#define DEFAULT_DATATYPE_NUM 1
#define DEFAULT_CHILDTABLES 10000

#define STMT_BIND_PARAM_BATCH 1

#if _MSC_VER <= 1900
#define __func__ __FUNCTION__
#endif

#define debugPrint(fmt, ...)                            \
    do {                                                \
        if (g_args.debug_print || g_args.verbose_print) \
            fprintf(stderr, "DEBG: " fmt, __VA_ARGS__); \
    } while (0)

#define verbosePrint(fmt, ...)                                                \
    do {                                                                      \
        if (g_args.verbose_print) fprintf(stderr, "VERB: " fmt, __VA_ARGS__); \
    } while (0)

#define performancePrint(fmt, ...)                      \
    do {                                                \
        if (g_args.performance_print)                   \
            fprintf(stderr, "PERF: " fmt, __VA_ARGS__); \
    } while (0)

#define errorPrint(fmt, ...)                         \
    do {                                             \
        fprintf(stderr, " \033[31m");                \
        fprintf(stderr, "ERROR: " fmt, __VA_ARGS__); \
        fprintf(stderr, " \033[0m");                 \
    } while (0)

#define errorPrint2(fmt, ...)                                             \
    do {                                                                  \
        struct tm      Tm, *ptm;                                          \
        struct timeval timeSecs;                                          \
        time_t         curTime;                                           \
        gettimeofday(&timeSecs, NULL);                                    \
        curTime = timeSecs.tv_sec;                                        \
        ptm = localtime_r(&curTime, &Tm);                                 \
        fprintf(stderr, " \033[31m");                                     \
        fprintf(stderr, "%02d/%02d %02d:%02d:%02d.%06d %08" PRId64 " ",   \
                ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, \
                ptm->tm_sec, (int32_t)timeSecs.tv_usec,                   \
                taosGetSelfPthreadId());                                  \
        fprintf(stderr, " \033[0m");                                      \
        errorPrint(fmt, __VA_ARGS__);                                     \
    } while (0)

#define SHOW_PARSE_RESULT_START()                                           \
    do {                                                                    \
        if (g_args.metaFile)                                                \
            printf(                                                         \
                "\033[1m\033[40;32m================ %s parse result START " \
                "================\033[0m\n",                                \
                g_args.metaFile);                                           \
    } while (0)

#define SHOW_PARSE_RESULT_END()                                       \
    do {                                                              \
        if (g_args.metaFile)                                          \
            printf(                                                   \
                "\033[1m\033[40;32m================ %s parse result " \
                "END================\033[0m\n",                       \
                g_args.metaFile);                                     \
    } while (0)

#define SHOW_PARSE_RESULT_START_TO_FILE(fp)                               \
    do {                                                                  \
        if (g_args.metaFile)                                              \
            fprintf(fp,                                                   \
                    "\033[1m\033[40;32m================ %s parse result " \
                    "START ================\033[0m\n",                    \
                    g_args.metaFile);                                     \
    } while (0)

#define SHOW_PARSE_RESULT_END_TO_FILE(fp)                                 \
    do {                                                                  \
        if (g_args.metaFile)                                              \
            fprintf(fp,                                                   \
                    "\033[1m\033[40;32m================ %s parse result " \
                    "END================\033[0m\n",                       \
                    g_args.metaFile);                                     \
    } while (0)

// for strncpy buffer overflow
#define min(a, b) (((a) < (b)) ? (a) : (b))

#ifndef TAOSDEMO_COMMIT_SHA1
#define TAOSDEMO_COMMIT_SHA1 "unknown"
#endif

#ifndef TD_VERNUMBER
#define TD_VERNUMBER "unknown"
#endif

#ifndef TAOSDEMO_STATUS
#define TAOSDEMO_STATUS "unknown"
#endif

enum TEST_MODE {
    INSERT_TEST,     // 0
    QUERY_TEST,      // 1
    SUBSCRIBE_TEST,  // 2
    INVAID_TEST
};

typedef enum CREATE_SUB_TABLE_MOD_EN {
    PRE_CREATE_SUBTBL,
    AUTO_CREATE_SUBTBL,
    NO_CREATE_SUBTBL
} CREATE_SUB_TABLE_MOD_EN;

typedef enum TABLE_EXISTS_EN {
    TBL_NO_EXISTS,
    TBL_ALREADY_EXISTS,
    TBL_EXISTS_BUTT
} TABLE_EXISTS_EN;

enum enumSYNC_MODE { SYNC_MODE, ASYNC_MODE, MODE_BUT };

enum enum_TAOS_INTERFACE { TAOSC_IFACE, REST_IFACE, STMT_IFACE, INTERFACE_BUT };

typedef enum enumQUERY_CLASS {
    SPECIFIED_CLASS,
    STABLE_CLASS,
    CLASS_BUT
} QUERY_CLASS;

typedef enum enum_PROGRESSIVE_OR_INTERLACE {
    PROGRESSIVE_INSERT_MODE,
    INTERLACE_INSERT_MODE,
    INVALID_INSERT_MODE
} PROG_OR_INTERLACE_MODE;

typedef enum enumQUERY_TYPE {
    NO_INSERT_TYPE,
    INSERT_TYPE,
    QUERY_TYPE_BUT
} QUERY_TYPE;

enum _show_db_index {
    TSDB_SHOW_DB_NAME_INDEX,
    TSDB_SHOW_DB_CREATED_TIME_INDEX,
    TSDB_SHOW_DB_NTABLES_INDEX,
    TSDB_SHOW_DB_VGROUPS_INDEX,
    TSDB_SHOW_DB_REPLICA_INDEX,
    TSDB_SHOW_DB_QUORUM_INDEX,
    TSDB_SHOW_DB_DAYS_INDEX,
    TSDB_SHOW_DB_KEEP_INDEX,
    TSDB_SHOW_DB_CACHE_INDEX,
    TSDB_SHOW_DB_BLOCKS_INDEX,
    TSDB_SHOW_DB_MINROWS_INDEX,
    TSDB_SHOW_DB_MAXROWS_INDEX,
    TSDB_SHOW_DB_WALLEVEL_INDEX,
    TSDB_SHOW_DB_FSYNC_INDEX,
    TSDB_SHOW_DB_COMP_INDEX,
    TSDB_SHOW_DB_CACHELAST_INDEX,
    TSDB_SHOW_DB_PRECISION_INDEX,
    TSDB_SHOW_DB_UPDATE_INDEX,
    TSDB_SHOW_DB_STATUS_INDEX,
    TSDB_MAX_SHOW_DB
};

// -----------------------------------------SHOW TABLES CONFIGURE
// -------------------------------------
enum _show_stables_index {
    TSDB_SHOW_STABLES_NAME_INDEX,
    TSDB_SHOW_STABLES_CREATED_TIME_INDEX,
    TSDB_SHOW_STABLES_COLUMNS_INDEX,
    TSDB_SHOW_STABLES_METRIC_INDEX,
    TSDB_SHOW_STABLES_UID_INDEX,
    TSDB_SHOW_STABLES_TID_INDEX,
    TSDB_SHOW_STABLES_VGID_INDEX,
    TSDB_MAX_SHOW_STABLES
};

enum _describe_table_index {
    TSDB_DESCRIBE_METRIC_FIELD_INDEX,
    TSDB_DESCRIBE_METRIC_TYPE_INDEX,
    TSDB_DESCRIBE_METRIC_LENGTH_INDEX,
    TSDB_DESCRIBE_METRIC_NOTE_INDEX,
    TSDB_MAX_DESCRIBE_METRIC
};
typedef struct SArguments_S {
    char*    metaFile;
    uint32_t test_mode;
    char*    host;
    uint16_t port;
    uint16_t iface;
    char*    user;
    char     password[SHELL_MAX_PASSWORD_LEN];
    char*    database;
    int      replica;
    char*    tb_prefix;
    char*    sqlFile;
    bool     use_metric;
    bool     drop_database;
    bool     aggr_func;
    bool     answer_yes;
    bool     debug_print;
    bool     verbose_print;
    bool     performance_print;
    char*    output_file;
    bool     async_mode;
    char     data_type[MAX_NUM_COLUMNS + 1];
    char*    dataType[MAX_NUM_COLUMNS + 1];
    uint32_t binwidth;
    uint32_t columnCount;
    uint64_t lenOfOneRow;
    uint32_t nthreads;
    uint64_t insert_interval;
    uint64_t timestamp_step;
    int64_t  query_times;
    uint32_t interlaceRows;
    uint32_t reqPerReq;  // num_of_records_per_req
    uint64_t max_sql_len;
    int64_t  ntables;
    int64_t  insertRows;
    int      abort;
    uint32_t disorderRatio;  // 0: no disorder, >0: x%
    int      disorderRange;  // ms, us or ns. according to database precision
    uint32_t method_of_delete;
    uint64_t totalInsertRows;
    uint64_t totalAffectedRows;
    bool     demo_mode;  // use default column name and semi-random data
} SArguments;

typedef struct SColumn_S {
    char     field[TSDB_COL_NAME_LEN];
    char     data_type;
    char     dataType[DATATYPE_BUFF_LEN];
    uint32_t dataLen;
    char     note[NOTE_BUFF_LEN];
} StrColumn;

typedef struct SSuperTable_S {
    char     stbName[TSDB_TABLE_NAME_LEN];
    char     dataSource[SMALL_BUFF_LEN];  // rand_gen or sample
    char     childTblPrefix[TBNAME_PREFIX_LEN];
    uint16_t childTblExists;
    int64_t  childTblCount;
    uint64_t batchCreateTableNum;  // 0: no batch,  > 0: batch table number in
                                   // one sql
    uint8_t  autoCreateTable;  // 0: create sub table, 1: auto create sub table
    uint16_t iface;            // 0: taosc, 1: rest, 2: stmt
    int64_t  childTblLimit;
    uint64_t childTblOffset;

    //  int          multiThreadWriteOneTbl;  // 0: no, 1: yes
    uint32_t interlaceRows;  //
    int      disorderRatio;  // 0: no disorder, >0: x%
    int      disorderRange;  // ms, us or ns. according to database precision
    uint64_t maxSqlLen;      //

    uint64_t insertInterval;  // insert interval, will override global insert
                              // interval
    int64_t insertRows;
    int64_t timeStampStep;
    char    startTimestamp[MAX_TB_NAME_SIZE];
    char    sampleFormat[SMALL_BUFF_LEN];  // csv, json
    char    sampleFile[MAX_FILE_NAME_LEN];
    char    tagsFile[MAX_FILE_NAME_LEN];

    uint32_t  columnCount;
    StrColumn columns[TSDB_MAX_COLUMNS];
    uint32_t  tagCount;
    StrColumn tags[TSDB_MAX_TAGS];

    char*    childTblName;
    char*    colsOfCreateChildTable;
    uint64_t lenOfOneRow;
    uint64_t lenOfTagOfOneRow;

    char* sampleDataBuf;
    bool  useSampleTs;

    uint32_t tagSource;  // 0: rand, 1: tag sample
    char*    tagDataBuf;
    uint32_t tagSampleCount;
    uint32_t tagUsePos;

#if STMT_BIND_PARAM_BATCH == 1
    // bind param batch
    char* sampleBindBatchArray;
#endif
    // statistics
    uint64_t totalInsertRows;
    uint64_t totalAffectedRows;
} SSuperTable;

typedef struct normalTable_S {
    SSuperTable* stbInfo;
    uint64_t     tbSeq;
    char*        tbName;
} SNormalTable;

typedef struct {
    char    name[TSDB_DB_NAME_LEN];
    char    create_time[32];
    int64_t ntables;
    int32_t vgroups;
    int16_t replica;
    int16_t quorum;
    int16_t days;
    char    keeplist[64];
    int32_t cache;  // MB
    int32_t blocks;
    int32_t minrows;
    int32_t maxrows;
    int8_t  wallevel;
    int32_t fsync;
    int8_t  comp;
    int8_t  cachelast;
    char    precision[SMALL_BUFF_LEN];  // time resolution
    int8_t  update;
    char    status[16];
} SDbInfo;

typedef struct SDbCfg_S {
    //  int       maxtablesPerVnode;
    uint32_t minRows;  // 0 means default
    uint32_t maxRows;  // 0 means default
    int      comp;
    int      walLevel;
    int      cacheLast;
    int      fsync;
    int      replica;
    int      update;
    int      keep;
    int      days;
    int      cache;
    int      blocks;
    int      quorum;
    char     precision[SMALL_BUFF_LEN];
} SDbCfg;

typedef struct SDataBase_S {
    char        dbName[TSDB_DB_NAME_LEN];
    bool        drop;  // 0: use exists, 1: if exists, drop then new create
    SDbCfg      dbCfg;
    uint64_t    superTblCount;
    SSuperTable superTbls[MAX_SUPER_TABLE_COUNT];
} SDataBase;

typedef struct SDbs_S {
    char               cfgDir[MAX_FILE_NAME_LEN];
    char               host[MAX_HOSTNAME_SIZE];
    struct sockaddr_in serv_addr;

    uint16_t port;
    char     user[MAX_USERNAME_SIZE];
    char     password[SHELL_MAX_PASSWORD_LEN];
    char     resultFile[MAX_FILE_NAME_LEN];
    bool     use_metric;
    bool     aggr_func;
    bool     asyncMode;

    uint32_t  threadCount;
    uint32_t  threadCountForCreateTbl;
    uint32_t  dbCount;
    SDataBase db[MAX_DB_COUNT];

    // statistics
    uint64_t totalInsertRows;
    uint64_t totalAffectedRows;

} SDbs;

typedef struct SpecifiedQueryInfo_S {
    uint64_t  queryInterval;  // 0: unlimited  > 0   loop/s
    uint32_t  concurrent;
    int       sqlCount;
    uint32_t  asyncMode;          // 0: sync, 1: async
    uint64_t  subscribeInterval;  // ms
    uint64_t  queryTimes;
    bool      subscribeRestart;
    int       subscribeKeepProgress;
    char      sql[MAX_QUERY_SQL_COUNT][BUFFER_SIZE + 1];
    char      result[MAX_QUERY_SQL_COUNT][MAX_FILE_NAME_LEN];
    int       resubAfterConsume[MAX_QUERY_SQL_COUNT];
    int       endAfterConsume[MAX_QUERY_SQL_COUNT];
    TAOS_SUB* tsub[MAX_QUERY_SQL_COUNT];
    char      topic[MAX_QUERY_SQL_COUNT][32];
    int       consumed[MAX_QUERY_SQL_COUNT];
    TAOS_RES* res[MAX_QUERY_SQL_COUNT];
    uint64_t  totalQueried;
} SpecifiedQueryInfo;

typedef struct SuperQueryInfo_S {
    char     stbName[TSDB_TABLE_NAME_LEN];
    uint64_t queryInterval;  // 0: unlimited  > 0   loop/s
    uint32_t threadCnt;
    uint32_t asyncMode;          // 0: sync, 1: async
    uint64_t subscribeInterval;  // ms
    bool     subscribeRestart;
    int      subscribeKeepProgress;
    uint64_t queryTimes;
    int64_t  childTblCount;
    char childTblPrefix[TBNAME_PREFIX_LEN];  // 20 characters reserved for seq
    int  sqlCount;
    char sql[MAX_QUERY_SQL_COUNT][BUFFER_SIZE + 1];
    char result[MAX_QUERY_SQL_COUNT][MAX_FILE_NAME_LEN];
    int  resubAfterConsume;
    int  endAfterConsume;
    TAOS_SUB* tsub[MAX_QUERY_SQL_COUNT];

    char*    childTblName;
    uint64_t totalQueried;
} SuperQueryInfo;

typedef struct SQueryMetaInfo_S {
    char               cfgDir[MAX_FILE_NAME_LEN];
    char               host[MAX_HOSTNAME_SIZE];
    uint16_t           port;
    struct sockaddr_in serv_addr;
    char               user[MAX_USERNAME_SIZE];
    char               password[SHELL_MAX_PASSWORD_LEN];
    char               dbName[TSDB_DB_NAME_LEN];
    char               queryMode[SMALL_BUFF_LEN];  // taosc, rest

    SpecifiedQueryInfo specifiedQueryInfo;
    SuperQueryInfo     superQueryInfo;
    uint64_t           totalQueried;
} SQueryMetaInfo;

typedef struct SThreadInfo_S {
    TAOS*      taos;
    TAOS_STMT* stmt;
    int64_t*   bind_ts;

#if STMT_BIND_PARAM_BATCH == 1
    int64_t* bind_ts_array;
    char*    bindParams;
    char*    is_null;
#else
    char* sampleBindArray;
#endif

    int          threadID;
    char         db_name[TSDB_DB_NAME_LEN];
    int          dbNum;
    uint32_t     time_precision;
    char         filePath[4096];
    FILE*        fp;
    char         tb_prefix[TSDB_TABLE_NAME_LEN];
    uint64_t     start_table_from;
    uint64_t     end_table_to;
    int64_t      ntables;
    int64_t      tables_created;
    uint64_t     data_of_rate;
    int64_t      start_time;
    char*        cols;
    bool         use_metric;
    SSuperTable* stbInfo;
    char*        buffer;  // sql cmd buffer

    // for async insert
    tsem_t   lock_sem;
    int64_t  counter;
    uint64_t st;
    uint64_t et;
    uint64_t lastTs;

    // sample data
    int64_t samplePos;
    // statistics
    uint64_t totalInsertRows;
    uint64_t totalAffectedRows;

    // insert delay statistics
    uint64_t cntDelay;
    uint64_t totalDelay;
    uint64_t avgDelay;
    uint64_t maxDelay;
    uint64_t minDelay;

    // seq of query or subscribe
    uint64_t  querySeq;  // sequence number of sql command
    TAOS_SUB* tsub;

    SNormalTable* tableList;
} threadInfo;

extern SDbs           g_Dbs;
extern SArguments     g_args;
extern SQueryMetaInfo g_queryInfo;
extern FILE*          g_fpOfInsertResult;
extern int64_t        g_totalChildTables;
extern int64_t        g_actualChildTables;
#endif