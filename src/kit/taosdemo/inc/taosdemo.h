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
   when in some thread query return error, thread don't exit, but return, otherwise coredump in other thread.
   */

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
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wordexp.h>
#include <regex.h>
#else
#include <regex.h>
#include <stdio.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include "cJSON.h"

#include "os.h"
#include "taos.h"
#include "taoserror.h"
#include "tutil.h"
#include <time.h>

#define REQ_EXTRA_BUF_LEN   1024
#define RESP_BUF_LEN        4096

extern char configDir[];

#define STR_INSERT_INTO     "INSERT INTO "

#define MAX_RECORDS_PER_REQ     32766

#define HEAD_BUFF_LEN       TSDB_MAX_COLUMNS*24  // 16*MAX_COLUMNS + (192+32)*2 + insert into ..

#define BUFFER_SIZE         TSDB_MAX_ALLOWED_SQL_LEN
#define COND_BUF_LEN        (BUFFER_SIZE - 30)
#define COL_BUFFER_LEN      ((TSDB_COL_NAME_LEN + 15) * TSDB_MAX_COLUMNS)

#define MAX_USERNAME_SIZE  64
#define MAX_HOSTNAME_SIZE  253      // https://man7.org/linux/man-pages/man7/hostname.7.html
#define MAX_TB_NAME_SIZE   64
#define MAX_DATA_SIZE      (16*TSDB_MAX_COLUMNS)+20     // max record len: 16*MAX_COLUMNS, timestamp string and ,('') need extra space
#define OPT_ABORT          1 /* –abort */
#define MAX_FILE_NAME_LEN  256              // max file name length on linux is 255.

#define DEFAULT_START_TIME 1500000000000

#define MAX_PREPARED_RAND  1000000
#define INT_BUFF_LEN            12
#define BIGINT_BUFF_LEN         21
#define SMALLINT_BUFF_LEN       7
#define TINYINT_BUFF_LEN        5
#define BOOL_BUFF_LEN           6
#define FLOAT_BUFF_LEN          22
#define DOUBLE_BUFF_LEN         42
#define TIMESTAMP_BUFF_LEN      21

#define MAX_SAMPLES             10000
#define MAX_NUM_COLUMNS        (TSDB_MAX_COLUMNS - 1)      // exclude first column timestamp

#define MAX_DB_COUNT            8
#define MAX_SUPER_TABLE_COUNT   200

#define MAX_QUERY_SQL_COUNT     100

#define MAX_DATABASE_COUNT      256
#define INPUT_BUF_LEN           256

#define TBNAME_PREFIX_LEN       (TSDB_TABLE_NAME_LEN - 20) // 20 characters reserved for seq
#define SMALL_BUFF_LEN          8
#define DATATYPE_BUFF_LEN       (SMALL_BUFF_LEN*3)
#define NOTE_BUFF_LEN           (SMALL_BUFF_LEN*16)

#define DEFAULT_NTHREADS        8
#define DEFAULT_TIMESTAMP_STEP  1
#define DEFAULT_INTERLACE_ROWS  0
#define DEFAULT_DATATYPE_NUM    1
#define DEFAULT_CHILDTABLES     10000

#define STMT_BIND_PARAM_BATCH   1

enum TEST_MODE {
    INSERT_TEST,            // 0
    QUERY_TEST,             // 1
    SUBSCRIBE_TEST,         // 2
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

enum enumSYNC_MODE {
    SYNC_MODE,
    ASYNC_MODE,
    MODE_BUT
};

enum enum_TAOS_INTERFACE {
    TAOSC_IFACE,
    REST_IFACE,
    STMT_IFACE,
    INTERFACE_BUT
};

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

// -----------------------------------------SHOW TABLES CONFIGURE -------------------------------------
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