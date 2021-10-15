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
#include "taosdemo.h"
#include "demoInput.h"
#include "demoInsert.h"
#include "demoQuery.h"
#include "demoSubscribe.h"
/* ************ Global variables ************  */

SArguments g_args = {
    NULL,           // metaFile
    0,              // test_mode
    "localhost",    // host
    6030,           // port
    INTERFACE_BUT,  // iface
    "root",         // user
#ifdef _TD_POWER_
    "powerdb",  // password
#elif (_TD_TQ_ == true)
    "tqueue",  // password
#elif (_TD_PRO_ == true)
    "prodb",  // password
#else
    "taosdata",  // password
#endif
    "test",          // database
    1,               // replica
    "d",             // tb_prefix
    NULL,            // sqlFile
    true,            // use_metric
    true,            // drop_database
    false,           // aggr_func
    false,           // debug_print
    false,           // verbose_print
    false,           // performance statistic print
    false,           // answer_yes;
    "./output.txt",  // output_file
    0,               // mode : sync or async
    {TSDB_DATA_TYPE_FLOAT, TSDB_DATA_TYPE_INT, TSDB_DATA_TYPE_FLOAT},
    {
        "FLOAT",  // dataType
        "INT",    // dataType
        "FLOAT",  // dataType. demo mode has 3 columns
    },
    64,  // binwidth
    4,   // columnCount, timestamp + float + int + float
    20 + FLOAT_BUFF_LEN + INT_BUFF_LEN + FLOAT_BUFF_LEN,  // lenOfOneRow
    DEFAULT_NTHREADS,                                     // nthreads
    0,                                                    // insert_interval
    DEFAULT_TIMESTAMP_STEP,                               // timestamp_step
    1,                                                    // query_times
    DEFAULT_INTERLACE_ROWS,                               // interlaceRows;
    30000,                                                // reqPerReq
    (1024 * 1024),                                        // max_sql_len
    DEFAULT_CHILDTABLES,                                  // ntables
    10000,                                                // insertRows
    0,                                                    // abort
    0,                                                    // disorderRatio
    1000,                                                 // disorderRange
    1,                                                    // method_of_delete
    0,                                                    // totalInsertRows;
    0,                                                    // totalAffectedRows;
    true,                                                 // demo_mode;
};
SDbs g_Dbs;

int64_t        g_totalChildTables = DEFAULT_CHILDTABLES;
int64_t        g_actualChildTables = 0;
SQueryMetaInfo g_queryInfo;
FILE *         g_fpOfInsertResult = NULL;

/* Used by main to communicate with parse_opt. */
static char *g_dupstr = NULL;

///////////////////////////////////////////////////

static void parse_args(int argc, char *argv[], SArguments *arguments) {
    for (int i = 1; i < argc; i++) {
        if ((0 == strncmp(argv[i], "-f", strlen("-f"))) ||
            (0 == strncmp(argv[i], "--file", strlen("--file")))) {
            arguments->demo_mode = false;

            if (2 == strlen(argv[i])) {
                if (i + 1 == argc) {
                    errorPrintReqArg(argv[0], "f");
                    exit(EXIT_FAILURE);
                }
                arguments->metaFile = argv[++i];
            } else if (0 == strncmp(argv[i], "-f", strlen("-f"))) {
                arguments->metaFile = (char *)(argv[i] + strlen("-f"));
            } else if (strlen("--file") == strlen(argv[i])) {
                if (i + 1 == argc) {
                    errorPrintReqArg3(argv[0], "--file");
                    exit(EXIT_FAILURE);
                }
                arguments->metaFile = argv[++i];
            } else if (0 == strncmp(argv[i], "--file=", strlen("--file="))) {
                arguments->metaFile = (char *)(argv[i] + strlen("--file="));
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-c", strlen("-c"))) ||
                   (0 ==
                    strncmp(argv[i], "--config-dir", strlen("--config-dir")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "c");
                    exit(EXIT_FAILURE);
                }
                tstrncpy(configDir, argv[++i], TSDB_FILENAME_LEN);
            } else if (0 == strncmp(argv[i], "-c", strlen("-c"))) {
                tstrncpy(configDir, (char *)(argv[i] + strlen("-c")),
                         TSDB_FILENAME_LEN);
            } else if (strlen("--config-dir") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--config-dir");
                    exit(EXIT_FAILURE);
                }
                tstrncpy(configDir, argv[++i], TSDB_FILENAME_LEN);
            } else if (0 == strncmp(argv[i],
                                    "--config-dir=", strlen("--config-dir="))) {
                tstrncpy(configDir, (char *)(argv[i] + strlen("--config-dir=")),
                         TSDB_FILENAME_LEN);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-h", strlen("-h"))) ||
                   (0 == strncmp(argv[i], "--host", strlen("--host")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "h");
                    exit(EXIT_FAILURE);
                }
                arguments->host = argv[++i];
            } else if (0 == strncmp(argv[i], "-h", strlen("-h"))) {
                arguments->host = (char *)(argv[i] + strlen("-h"));
            } else if (strlen("--host") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--host");
                    exit(EXIT_FAILURE);
                }
                arguments->host = argv[++i];
            } else if (0 == strncmp(argv[i], "--host=", strlen("--host="))) {
                arguments->host = (char *)(argv[i] + strlen("--host="));
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "-PP") == 0) {
            arguments->performance_print = true;
        } else if ((0 == strncmp(argv[i], "-P", strlen("-P"))) ||
                   (0 == strncmp(argv[i], "--port", strlen("--port")))) {
            uint64_t port;
            char     strPort[BIGINT_BUFF_LEN];

            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "P");
                    exit(EXIT_FAILURE);
                } else if (isStringNumber(argv[i + 1])) {
                    tstrncpy(strPort, argv[++i], BIGINT_BUFF_LEN);
                } else {
                    errorPrintReqArg2(argv[0], "P");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "--port=", strlen("--port="))) {
                if (isStringNumber((char *)(argv[i] + strlen("--port=")))) {
                    tstrncpy(strPort, (char *)(argv[i] + strlen("--port=")),
                             BIGINT_BUFF_LEN);
                } else {
                    errorPrintReqArg2(argv[0], "--port");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-P", strlen("-P"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-P")))) {
                    tstrncpy(strPort, (char *)(argv[i] + strlen("-P")),
                             BIGINT_BUFF_LEN);
                } else {
                    errorPrintReqArg2(argv[0], "--port");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--port") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--port");
                    exit(EXIT_FAILURE);
                } else if (isStringNumber(argv[i + 1])) {
                    tstrncpy(strPort, argv[++i], BIGINT_BUFF_LEN);
                } else {
                    errorPrintReqArg2(argv[0], "--port");
                    exit(EXIT_FAILURE);
                }
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }

            port = atoi(strPort);
            if (port > 65535) {
                errorWrongValue("taosdump", "-P or --port", strPort);
                exit(EXIT_FAILURE);
            }
            arguments->port = (uint16_t)port;

        } else if ((0 == strncmp(argv[i], "-I", strlen("-I"))) ||
                   (0 ==
                    strncmp(argv[i], "--interface", strlen("--interface")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "I");
                    exit(EXIT_FAILURE);
                }
                if (0 == strcasecmp(argv[i + 1], "taosc")) {
                    arguments->iface = TAOSC_IFACE;
                } else if (0 == strcasecmp(argv[i + 1], "rest")) {
                    arguments->iface = REST_IFACE;
                } else if (0 == strcasecmp(argv[i + 1], "stmt")) {
                    arguments->iface = STMT_IFACE;
                } else {
                    errorWrongValue(argv[0], "-I", argv[i + 1]);
                    exit(EXIT_FAILURE);
                }
                i++;
            } else if (0 == strncmp(argv[i],
                                    "--interface=", strlen("--interface="))) {
                if (0 == strcasecmp((char *)(argv[i] + strlen("--interface=")),
                                    "taosc")) {
                    arguments->iface = TAOSC_IFACE;
                } else if (0 == strcasecmp(
                                    (char *)(argv[i] + strlen("--interface=")),
                                    "rest")) {
                    arguments->iface = REST_IFACE;
                } else if (0 == strcasecmp(
                                    (char *)(argv[i] + strlen("--interface=")),
                                    "stmt")) {
                    arguments->iface = STMT_IFACE;
                } else {
                    errorPrintReqArg3(argv[0], "--interface");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-I", strlen("-I"))) {
                if (0 ==
                    strcasecmp((char *)(argv[i] + strlen("-I")), "taosc")) {
                    arguments->iface = TAOSC_IFACE;
                } else if (0 == strcasecmp((char *)(argv[i] + strlen("-I")),
                                           "rest")) {
                    arguments->iface = REST_IFACE;
                } else if (0 == strcasecmp((char *)(argv[i] + strlen("-I")),
                                           "stmt")) {
                    arguments->iface = STMT_IFACE;
                } else {
                    errorWrongValue(argv[0], "-I",
                                    (char *)(argv[i] + strlen("-I")));
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--interface") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--interface");
                    exit(EXIT_FAILURE);
                }
                if (0 == strcasecmp(argv[i + 1], "taosc")) {
                    arguments->iface = TAOSC_IFACE;
                } else if (0 == strcasecmp(argv[i + 1], "rest")) {
                    arguments->iface = REST_IFACE;
                } else if (0 == strcasecmp(argv[i + 1], "stmt")) {
                    arguments->iface = STMT_IFACE;
                } else {
                    errorWrongValue(argv[0], "--interface", argv[i + 1]);
                    exit(EXIT_FAILURE);
                }
                i++;
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-u", strlen("-u"))) ||
                   (0 == strncmp(argv[i], "--user", strlen("--user")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "u");
                    exit(EXIT_FAILURE);
                }
                arguments->user = argv[++i];
            } else if (0 == strncmp(argv[i], "-u", strlen("-u"))) {
                arguments->user = (char *)(argv[i++] + strlen("-u"));
            } else if (0 == strncmp(argv[i], "--user=", strlen("--user="))) {
                arguments->user = (char *)(argv[i++] + strlen("--user="));
            } else if (strlen("--user") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--user");
                    exit(EXIT_FAILURE);
                }
                arguments->user = argv[++i];
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-p", strlen("-p"))) ||
                   (0 == strcmp(argv[i], "--password"))) {
            if ((strlen(argv[i]) == 2) ||
                (0 == strcmp(argv[i], "--password"))) {
                printf("Enter password: ");
                taosSetConsoleEcho(false);
                if (scanf("%s", arguments->password) > 1) {
                    fprintf(stderr, "password read error!\n");
                }
                taosSetConsoleEcho(true);
            } else {
                tstrncpy(arguments->password, (char *)(argv[i] + 2),
                         SHELL_MAX_PASSWORD_LEN);
            }
        } else if ((0 == strncmp(argv[i], "-o", strlen("-o"))) ||
                   (0 == strncmp(argv[i], "--output", strlen("--output")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--output");
                    exit(EXIT_FAILURE);
                }
                arguments->output_file = argv[++i];
            } else if (0 ==
                       strncmp(argv[i], "--output=", strlen("--output="))) {
                arguments->output_file =
                    (char *)(argv[i++] + strlen("--output="));
            } else if (0 == strncmp(argv[i], "-o", strlen("-o"))) {
                arguments->output_file = (char *)(argv[i++] + strlen("-o"));
            } else if (strlen("--output") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--output");
                    exit(EXIT_FAILURE);
                }
                arguments->output_file = argv[++i];
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-s", strlen("-s"))) ||
                   (0 ==
                    strncmp(argv[i], "--sql-file", strlen("--sql-file")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "s");
                    exit(EXIT_FAILURE);
                }
                arguments->sqlFile = argv[++i];
            } else if (0 ==
                       strncmp(argv[i], "--sql-file=", strlen("--sql-file="))) {
                arguments->sqlFile =
                    (char *)(argv[i++] + strlen("--sql-file="));
            } else if (0 == strncmp(argv[i], "-s", strlen("-s"))) {
                arguments->sqlFile = (char *)(argv[i++] + strlen("-s"));
            } else if (strlen("--sql-file") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--sql-file");
                    exit(EXIT_FAILURE);
                }
                arguments->sqlFile = argv[++i];
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-q", strlen("-q"))) ||
                   (0 ==
                    strncmp(argv[i], "--query-mode", strlen("--query-mode")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "q");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "q");
                    exit(EXIT_FAILURE);
                }
                arguments->async_mode = atoi(argv[++i]);
            } else if (0 == strncmp(argv[i],
                                    "--query-mode=", strlen("--query-mode="))) {
                if (isStringNumber(
                        (char *)(argv[i] + strlen("--query-mode=")))) {
                    arguments->async_mode =
                        atoi((char *)(argv[i] + strlen("--query-mode=")));
                } else {
                    errorPrintReqArg2(argv[0], "--query-mode");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-q", strlen("-q"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-q")))) {
                    arguments->async_mode =
                        atoi((char *)(argv[i] + strlen("-q")));
                } else {
                    errorPrintReqArg2(argv[0], "-q");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--query-mode") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--query-mode");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--query-mode");
                    exit(EXIT_FAILURE);
                }
                arguments->async_mode = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-T", strlen("-T"))) ||
                   (0 == strncmp(argv[i], "--threads", strlen("--threads")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "T");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "T");
                    exit(EXIT_FAILURE);
                }
                arguments->nthreads = atoi(argv[++i]);
            } else if (0 ==
                       strncmp(argv[i], "--threads=", strlen("--threads="))) {
                if (isStringNumber((char *)(argv[i] + strlen("--threads=")))) {
                    arguments->nthreads =
                        atoi((char *)(argv[i] + strlen("--threads=")));
                } else {
                    errorPrintReqArg2(argv[0], "--threads");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-T", strlen("-T"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-T")))) {
                    arguments->nthreads =
                        atoi((char *)(argv[i] + strlen("-T")));
                } else {
                    errorPrintReqArg2(argv[0], "-T");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--threads") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--threads");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--threads");
                    exit(EXIT_FAILURE);
                }
                arguments->nthreads = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-i", strlen("-i"))) ||
                   (0 == strncmp(argv[i], "--insert-interval",
                                 strlen("--insert-interval")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "i");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "i");
                    exit(EXIT_FAILURE);
                }
                arguments->insert_interval = atoi(argv[++i]);
            } else if (0 == strncmp(argv[i], "--insert-interval=",
                                    strlen("--insert-interval="))) {
                if (isStringNumber(
                        (char *)(argv[i] + strlen("--insert-interval=")))) {
                    arguments->insert_interval =
                        atoi((char *)(argv[i] + strlen("--insert-interval=")));
                } else {
                    errorPrintReqArg3(argv[0], "--insert-innterval");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-i", strlen("-i"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-i")))) {
                    arguments->insert_interval =
                        atoi((char *)(argv[i] + strlen("-i")));
                } else {
                    errorPrintReqArg3(argv[0], "-i");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--insert-interval") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--insert-interval");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--insert-interval");
                    exit(EXIT_FAILURE);
                }
                arguments->insert_interval = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-S", strlen("-S"))) ||
                   (0 ==
                    strncmp(argv[i], "--time-step", strlen("--time-step")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "S");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "S");
                    exit(EXIT_FAILURE);
                }
                arguments->async_mode = atoi(argv[++i]);
            } else if (0 == strncmp(argv[i],
                                    "--time-step=", strlen("--time-step="))) {
                if (isStringNumber(
                        (char *)(argv[i] + strlen("--time-step=")))) {
                    arguments->async_mode =
                        atoi((char *)(argv[i] + strlen("--time-step=")));
                } else {
                    errorPrintReqArg2(argv[0], "--time-step");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-S", strlen("-S"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-S")))) {
                    arguments->async_mode =
                        atoi((char *)(argv[i] + strlen("-S")));
                } else {
                    errorPrintReqArg2(argv[0], "-S");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--time-step") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--time-step");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--time-step");
                    exit(EXIT_FAILURE);
                }
                arguments->async_mode = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(argv[i], "-qt") == 0) {
            if ((argc == i + 1) || (!isStringNumber(argv[i + 1]))) {
                printHelp();
                errorPrint("%s", "\n\t-qt need a number following!\n");
                exit(EXIT_FAILURE);
            }
            arguments->query_times = atoi(argv[++i]);
        } else if ((0 == strncmp(argv[i], "-B", strlen("-B"))) ||
                   (0 == strncmp(argv[i], "--interlace-rows",
                                 strlen("--interlace-rows")))) {
            if (strlen("-B") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "B");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "B");
                    exit(EXIT_FAILURE);
                }
                arguments->interlaceRows = atoi(argv[++i]);
            } else if (0 == strncmp(argv[i], "--interlace-rows=",
                                    strlen("--interlace-rows="))) {
                if (isStringNumber(
                        (char *)(argv[i] + strlen("--interlace-rows=")))) {
                    arguments->interlaceRows =
                        atoi((char *)(argv[i] + strlen("--interlace-rows=")));
                } else {
                    errorPrintReqArg2(argv[0], "--interlace-rows");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-B", strlen("-B"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-B")))) {
                    arguments->interlaceRows =
                        atoi((char *)(argv[i] + strlen("-B")));
                } else {
                    errorPrintReqArg2(argv[0], "-B");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--interlace-rows") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--interlace-rows");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--interlace-rows");
                    exit(EXIT_FAILURE);
                }
                arguments->interlaceRows = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-r", strlen("-r"))) ||
                   (0 == strncmp(argv[i], "--rec-per-req", 13))) {
            if (strlen("-r") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "r");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "r");
                    exit(EXIT_FAILURE);
                }
                arguments->reqPerReq = atoi(argv[++i]);
            } else if (0 == strncmp(argv[i], "--rec-per-req=",
                                    strlen("--rec-per-req="))) {
                if (isStringNumber(
                        (char *)(argv[i] + strlen("--rec-per-req=")))) {
                    arguments->reqPerReq =
                        atoi((char *)(argv[i] + strlen("--rec-per-req=")));
                } else {
                    errorPrintReqArg2(argv[0], "--rec-per-req");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-r", strlen("-r"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-r")))) {
                    arguments->reqPerReq =
                        atoi((char *)(argv[i] + strlen("-r")));
                } else {
                    errorPrintReqArg2(argv[0], "-r");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--rec-per-req") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--rec-per-req");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--rec-per-req");
                    exit(EXIT_FAILURE);
                }
                arguments->reqPerReq = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-t", strlen("-t"))) ||
                   (0 == strncmp(argv[i], "--tables", strlen("--tables")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "t");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "t");
                    exit(EXIT_FAILURE);
                }
                arguments->ntables = atoi(argv[++i]);
            } else if (0 ==
                       strncmp(argv[i], "--tables=", strlen("--tables="))) {
                if (isStringNumber((char *)(argv[i] + strlen("--tables=")))) {
                    arguments->ntables =
                        atoi((char *)(argv[i] + strlen("--tables=")));
                } else {
                    errorPrintReqArg2(argv[0], "--tables");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-t", strlen("-t"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-t")))) {
                    arguments->ntables = atoi((char *)(argv[i] + strlen("-t")));
                } else {
                    errorPrintReqArg2(argv[0], "-t");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--tables") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--tables");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--tables");
                    exit(EXIT_FAILURE);
                }
                arguments->ntables = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }

            g_totalChildTables = arguments->ntables;
        } else if ((0 == strncmp(argv[i], "-n", strlen("-n"))) ||
                   (0 == strncmp(argv[i], "--records", strlen("--records")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "n");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "n");
                    exit(EXIT_FAILURE);
                }
                arguments->insertRows = atoi(argv[++i]);
            } else if (0 ==
                       strncmp(argv[i], "--records=", strlen("--records="))) {
                if (isStringNumber((char *)(argv[i] + strlen("--records=")))) {
                    arguments->insertRows =
                        atoi((char *)(argv[i] + strlen("--records=")));
                } else {
                    errorPrintReqArg2(argv[0], "--records");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-n", strlen("-n"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-n")))) {
                    arguments->insertRows =
                        atoi((char *)(argv[i] + strlen("-n")));
                } else {
                    errorPrintReqArg2(argv[0], "-n");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--records") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--records");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--records");
                    exit(EXIT_FAILURE);
                }
                arguments->insertRows = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-d", strlen("-d"))) ||
                   (0 ==
                    strncmp(argv[i], "--database", strlen("--database")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "d");
                    exit(EXIT_FAILURE);
                }
                arguments->database = argv[++i];
            } else if (0 ==
                       strncmp(argv[i], "--database=", strlen("--database="))) {
                arguments->output_file =
                    (char *)(argv[i] + strlen("--database="));
            } else if (0 == strncmp(argv[i], "-d", strlen("-d"))) {
                arguments->output_file = (char *)(argv[i] + strlen("-d"));
            } else if (strlen("--database") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--database");
                    exit(EXIT_FAILURE);
                }
                arguments->database = argv[++i];
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-l", strlen("-l"))) ||
                   (0 == strncmp(argv[i], "--columns", strlen("--columns")))) {
            arguments->demo_mode = false;
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "l");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "l");
                    exit(EXIT_FAILURE);
                }
                arguments->columnCount = atoi(argv[++i]);
            } else if (0 ==
                       strncmp(argv[i], "--columns=", strlen("--columns="))) {
                if (isStringNumber((char *)(argv[i] + strlen("--columns=")))) {
                    arguments->columnCount =
                        atoi((char *)(argv[i] + strlen("--columns=")));
                } else {
                    errorPrintReqArg2(argv[0], "--columns");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-l", strlen("-l"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-l")))) {
                    arguments->columnCount =
                        atoi((char *)(argv[i] + strlen("-l")));
                } else {
                    errorPrintReqArg2(argv[0], "-l");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--columns") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--columns");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--columns");
                    exit(EXIT_FAILURE);
                }
                arguments->columnCount = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }

            if (arguments->columnCount > MAX_NUM_COLUMNS) {
                printf("WARNING: max acceptable columns count is %d\n",
                       MAX_NUM_COLUMNS);
                prompt();
                arguments->columnCount = MAX_NUM_COLUMNS;
            }

            for (int col = DEFAULT_DATATYPE_NUM; col < arguments->columnCount;
                 col++) {
                arguments->dataType[col] = "INT";
                arguments->data_type[col] = TSDB_DATA_TYPE_INT;
            }
            for (int col = arguments->columnCount; col < MAX_NUM_COLUMNS;
                 col++) {
                arguments->dataType[col] = NULL;
                arguments->data_type[col] = TSDB_DATA_TYPE_NULL;
            }
        } else if ((0 == strncmp(argv[i], "-b", strlen("-b"))) ||
                   (0 ==
                    strncmp(argv[i], "--data-type", strlen("--data-type")))) {
            arguments->demo_mode = false;

            char *dataType;
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "b");
                    exit(EXIT_FAILURE);
                }
                dataType = argv[++i];
            } else if (0 == strncmp(argv[i],
                                    "--data-type=", strlen("--data-type="))) {
                dataType = (char *)(argv[i] + strlen("--data-type="));
            } else if (0 == strncmp(argv[i], "-b", strlen("-b"))) {
                dataType = (char *)(argv[i] + strlen("-b"));
            } else if (strlen("--data-type") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--data-type");
                    exit(EXIT_FAILURE);
                }
                dataType = argv[++i];
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }

            if (strstr(dataType, ",") == NULL) {
                // only one col
                if (strcasecmp(dataType, "INT") &&
                    strcasecmp(dataType, "FLOAT") &&
                    strcasecmp(dataType, "TINYINT") &&
                    strcasecmp(dataType, "BOOL") &&
                    strcasecmp(dataType, "SMALLINT") &&
                    strcasecmp(dataType, "BIGINT") &&
                    strcasecmp(dataType, "DOUBLE") &&
                    strcasecmp(dataType, "TIMESTAMP") &&
                    !regexMatch(dataType,
                                "^(NCHAR|BINARY)(\\([1-9][0-9]*\\))?$",
                                REG_ICASE | REG_EXTENDED) &&
                    strcasecmp(dataType, "UTINYINT") &&
                    strcasecmp(dataType, "USMALLINT") &&
                    strcasecmp(dataType, "UINT") &&
                    strcasecmp(dataType, "UBIGINT")) {
                    printHelp();
                    errorPrint("%s", "-b: Invalid data_type!\n");
                    exit(EXIT_FAILURE);
                }
                arguments->dataType[0] = dataType;
                if (0 == strcasecmp(dataType, "INT")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_INT;
                } else if (0 == strcasecmp(dataType, "TINYINT")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_TINYINT;
                } else if (0 == strcasecmp(dataType, "SMALLINT")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_SMALLINT;
                } else if (0 == strcasecmp(dataType, "BIGINT")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_BIGINT;
                } else if (0 == strcasecmp(dataType, "FLOAT")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_FLOAT;
                } else if (0 == strcasecmp(dataType, "DOUBLE")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_DOUBLE;
                } else if (1 == regexMatch(dataType,
                                           "^BINARY(\\([1-9][0-9]*\\))?$",
                                           REG_ICASE | REG_EXTENDED)) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_BINARY;
                } else if (1 == regexMatch(dataType,
                                           "^NCHAR(\\([1-9][0-9]*\\))?$",
                                           REG_ICASE | REG_EXTENDED)) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_NCHAR;
                } else if (0 == strcasecmp(dataType, "BOOL")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_BOOL;
                } else if (0 == strcasecmp(dataType, "TIMESTAMP")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_TIMESTAMP;
                } else if (0 == strcasecmp(dataType, "UTINYINT")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_UTINYINT;
                } else if (0 == strcasecmp(dataType, "USMALLINT")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_USMALLINT;
                } else if (0 == strcasecmp(dataType, "UINT")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_UINT;
                } else if (0 == strcasecmp(dataType, "UBIGINT")) {
                    arguments->data_type[0] = TSDB_DATA_TYPE_UBIGINT;
                } else {
                    arguments->data_type[0] = TSDB_DATA_TYPE_NULL;
                }
                arguments->dataType[1] = NULL;
                arguments->data_type[1] = TSDB_DATA_TYPE_NULL;
            } else {
                // more than one col
                int index = 0;
                g_dupstr = strdup(dataType);
                char *running = g_dupstr;
                char *token = strsep(&running, ",");
                while (token != NULL) {
                    if (strcasecmp(token, "INT") &&
                        strcasecmp(token, "FLOAT") &&
                        strcasecmp(token, "TINYINT") &&
                        strcasecmp(token, "BOOL") &&
                        strcasecmp(token, "SMALLINT") &&
                        strcasecmp(token, "BIGINT") &&
                        strcasecmp(token, "DOUBLE") &&
                        strcasecmp(token, "TIMESTAMP") &&
                        !regexMatch(token,
                                    "^(NCHAR|BINARY)(\\([1-9][0-9]*\\))?$",
                                    REG_ICASE | REG_EXTENDED) &&
                        strcasecmp(token, "UTINYINT") &&
                        strcasecmp(token, "USMALLINT") &&
                        strcasecmp(token, "UINT") &&
                        strcasecmp(token, "UBIGINT")) {
                        printHelp();
                        free(g_dupstr);
                        errorPrint("%s", "-b: Invalid data_type!\n");
                        exit(EXIT_FAILURE);
                    }

                    if (0 == strcasecmp(token, "INT")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_INT;
                    } else if (0 == strcasecmp(token, "FLOAT")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_FLOAT;
                    } else if (0 == strcasecmp(token, "SMALLINT")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_SMALLINT;
                    } else if (0 == strcasecmp(token, "BIGINT")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_BIGINT;
                    } else if (0 == strcasecmp(token, "DOUBLE")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_DOUBLE;
                    } else if (0 == strcasecmp(token, "TINYINT")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_TINYINT;
                    } else if (1 == regexMatch(token,
                                               "^BINARY(\\([1-9][0-9]*\\))?$",
                                               REG_ICASE | REG_EXTENDED)) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_BINARY;
                    } else if (1 == regexMatch(token,
                                               "^NCHAR(\\([1-9][0-9]*\\))?$",
                                               REG_ICASE | REG_EXTENDED)) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_NCHAR;
                    } else if (0 == strcasecmp(token, "BOOL")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_BOOL;
                    } else if (0 == strcasecmp(token, "TIMESTAMP")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_TIMESTAMP;
                    } else if (0 == strcasecmp(token, "UTINYINT")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_UTINYINT;
                    } else if (0 == strcasecmp(token, "USMALLINT")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_USMALLINT;
                    } else if (0 == strcasecmp(token, "UINT")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_UINT;
                    } else if (0 == strcasecmp(token, "UBIGINT")) {
                        arguments->data_type[index] = TSDB_DATA_TYPE_UBIGINT;
                    } else {
                        arguments->data_type[index] = TSDB_DATA_TYPE_NULL;
                    }
                    arguments->dataType[index] = token;
                    index++;
                    token = strsep(&running, ",");
                    if (index >= MAX_NUM_COLUMNS) break;
                }
                arguments->dataType[index] = NULL;
                arguments->data_type[index] = TSDB_DATA_TYPE_NULL;
            }
        } else if ((0 == strncmp(argv[i], "-w", strlen("-w"))) ||
                   (0 ==
                    strncmp(argv[i], "--binwidth", strlen("--binwidth")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "w");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "w");
                    exit(EXIT_FAILURE);
                }
                arguments->binwidth = atoi(argv[++i]);
            } else if (0 ==
                       strncmp(argv[i], "--binwidth=", strlen("--binwidth="))) {
                if (isStringNumber((char *)(argv[i] + strlen("--binwidth=")))) {
                    arguments->binwidth =
                        atoi((char *)(argv[i] + strlen("--binwidth=")));
                } else {
                    errorPrintReqArg2(argv[0], "--binwidth");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-w", strlen("-w"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-w")))) {
                    arguments->binwidth =
                        atoi((char *)(argv[i] + strlen("-w")));
                } else {
                    errorPrintReqArg2(argv[0], "-w");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--binwidth") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--binwidth");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--binwidth");
                    exit(EXIT_FAILURE);
                }
                arguments->binwidth = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-m", strlen("-m"))) ||
                   (0 == strncmp(argv[i], "--table-prefix",
                                 strlen("--table-prefix")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "m");
                    exit(EXIT_FAILURE);
                }
                arguments->tb_prefix = argv[++i];
            } else if (0 == strncmp(argv[i], "--table-prefix=",
                                    strlen("--table-prefix="))) {
                arguments->tb_prefix =
                    (char *)(argv[i] + strlen("--table-prefix="));
            } else if (0 == strncmp(argv[i], "-m", strlen("-m"))) {
                arguments->tb_prefix = (char *)(argv[i] + strlen("-m"));
            } else if (strlen("--table-prefix") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--table-prefix");
                    exit(EXIT_FAILURE);
                }
                arguments->tb_prefix = argv[++i];
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((strcmp(argv[i], "-N") == 0) ||
                   (0 == strcmp(argv[i], "--normal-table"))) {
            arguments->demo_mode = false;
            arguments->use_metric = false;
        } else if ((strcmp(argv[i], "-M") == 0) ||
                   (0 == strcmp(argv[i], "--random"))) {
            arguments->demo_mode = false;
        } else if ((strcmp(argv[i], "-x") == 0) ||
                   (0 == strcmp(argv[i], "--aggr-func"))) {
            arguments->aggr_func = true;
        } else if ((strcmp(argv[i], "-y") == 0) ||
                   (0 == strcmp(argv[i], "--answer-yes"))) {
            arguments->answer_yes = true;
        } else if ((strcmp(argv[i], "-g") == 0) ||
                   (0 == strcmp(argv[i], "--debug"))) {
            arguments->debug_print = true;
        } else if (strcmp(argv[i], "-gg") == 0) {
            arguments->verbose_print = true;
        } else if ((0 == strncmp(argv[i], "-R", strlen("-R"))) ||
                   (0 == strncmp(argv[i], "--disorder-range",
                                 strlen("--disorder-range")))) {
            if (strlen("-R") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "R");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "R");
                    exit(EXIT_FAILURE);
                }
                arguments->disorderRange = atoi(argv[++i]);
            } else if (0 == strncmp(argv[i], "--disorder-range=",
                                    strlen("--disorder-range="))) {
                if (isStringNumber(
                        (char *)(argv[i] + strlen("--disorder-range=")))) {
                    arguments->disorderRange =
                        atoi((char *)(argv[i] + strlen("--disorder-range=")));
                } else {
                    errorPrintReqArg2(argv[0], "--disorder-range");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-R", strlen("-R"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-R")))) {
                    arguments->disorderRange =
                        atoi((char *)(argv[i] + strlen("-R")));
                } else {
                    errorPrintReqArg2(argv[0], "-R");
                    exit(EXIT_FAILURE);
                }

                if (arguments->disorderRange < 0) {
                    errorPrint("Invalid disorder range %d, will be set to %d\n",
                               arguments->disorderRange, 1000);
                    arguments->disorderRange = 1000;
                }
            } else if (strlen("--disorder-range") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--disorder-range");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--disorder-range");
                    exit(EXIT_FAILURE);
                }
                arguments->disorderRange = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }
        } else if ((0 == strncmp(argv[i], "-O", strlen("-O"))) ||
                   (0 ==
                    strncmp(argv[i], "--disorder", strlen("--disorder")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "O");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "O");
                    exit(EXIT_FAILURE);
                }
                arguments->disorderRatio = atoi(argv[++i]);
            } else if (0 ==
                       strncmp(argv[i], "--disorder=", strlen("--disorder="))) {
                if (isStringNumber((char *)(argv[i] + strlen("--disorder=")))) {
                    arguments->disorderRatio =
                        atoi((char *)(argv[i] + strlen("--disorder=")));
                } else {
                    errorPrintReqArg2(argv[0], "--disorder");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-O", strlen("-O"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-O")))) {
                    arguments->disorderRatio =
                        atoi((char *)(argv[i] + strlen("-O")));
                } else {
                    errorPrintReqArg2(argv[0], "-O");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--disorder") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--disorder");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--disorder");
                    exit(EXIT_FAILURE);
                }
                arguments->disorderRatio = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }

            if (arguments->disorderRatio > 50) {
                errorPrint("Invalid disorder ratio %d, will be set to %d\n",
                           arguments->disorderRatio, 50);
                arguments->disorderRatio = 50;
            }

            if (arguments->disorderRatio < 0) {
                errorPrint("Invalid disorder ratio %d, will be set to %d\n",
                           arguments->disorderRatio, 0);
                arguments->disorderRatio = 0;
            }
        } else if ((0 == strncmp(argv[i], "-a", strlen("-a"))) ||
                   (0 == strncmp(argv[i], "--replica", strlen("--replica")))) {
            if (2 == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg(argv[0], "a");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "a");
                    exit(EXIT_FAILURE);
                }
                arguments->replica = atoi(argv[++i]);
            } else if (0 ==
                       strncmp(argv[i], "--replica=", strlen("--replica="))) {
                if (isStringNumber((char *)(argv[i] + strlen("--replica=")))) {
                    arguments->replica =
                        atoi((char *)(argv[i] + strlen("--replica=")));
                } else {
                    errorPrintReqArg2(argv[0], "--replica");
                    exit(EXIT_FAILURE);
                }
            } else if (0 == strncmp(argv[i], "-a", strlen("-a"))) {
                if (isStringNumber((char *)(argv[i] + strlen("-a")))) {
                    arguments->replica = atoi((char *)(argv[i] + strlen("-a")));
                } else {
                    errorPrintReqArg2(argv[0], "-a");
                    exit(EXIT_FAILURE);
                }
            } else if (strlen("--replica") == strlen(argv[i])) {
                if (argc == i + 1) {
                    errorPrintReqArg3(argv[0], "--replica");
                    exit(EXIT_FAILURE);
                } else if (!isStringNumber(argv[i + 1])) {
                    errorPrintReqArg2(argv[0], "--replica");
                    exit(EXIT_FAILURE);
                }
                arguments->replica = atoi(argv[++i]);
            } else {
                errorUnrecognized(argv[0], argv[i]);
                exit(EXIT_FAILURE);
            }

            if (arguments->replica > 3 || arguments->replica < 1) {
                errorPrint("Invalid replica value %d, will be set to %d\n",
                           arguments->replica, 1);
                arguments->replica = 1;
            }
        } else if (strcmp(argv[i], "-D") == 0) {
            arguments->method_of_delete = atoi(argv[++i]);
            if (arguments->method_of_delete > 3) {
                errorPrint("%s",
                           "\n\t-D need a value (0~3) number following!\n");
                exit(EXIT_FAILURE);
            }
        } else if ((strcmp(argv[i], "--version") == 0) ||
                   (strcmp(argv[i], "-V") == 0)) {
            printVersion();
            exit(0);
        } else if ((strcmp(argv[i], "--help") == 0) ||
                   (strcmp(argv[i], "-?") == 0)) {
            printHelp();
            exit(0);
        } else if (strcmp(argv[i], "--usage") == 0) {
            printf(
                "    Usage: taosdemo [-f JSONFILE] [-u USER] [-p PASSWORD] [-c CONFIG_DIR]\n\
                    [-h HOST] [-P PORT] [-I INTERFACE] [-d DATABASE] [-a REPLICA]\n\
                    [-m TABLEPREFIX] [-s SQLFILE] [-N] [-o OUTPUTFILE] [-q QUERYMODE]\n\
                    [-b DATATYPES] [-w WIDTH_OF_BINARY] [-l COLUMNS] [-T THREADNUMBER]\n\
                    [-i SLEEPTIME] [-S TIME_STEP] [-B INTERLACE_ROWS] [-t TABLES]\n\
                    [-n RECORDS] [-M] [-x] [-y] [-O ORDERMODE] [-R RANGE] [-a REPLIcA][-g]\n\
                    [--help] [--usage] [--version]\n");
            exit(0);
        } else {
            // to simulate argp_option output
            if (strlen(argv[i]) > 2) {
                if (0 == strncmp(argv[i], "--", 2)) {
                    fprintf(stderr, "%s: unrecognized options '%s'\n", argv[0],
                            argv[i]);
                } else if (0 == strncmp(argv[i], "-", 1)) {
                    char tmp[2] = {0};
                    tstrncpy(tmp, argv[i] + 1, 2);
                    fprintf(stderr, "%s: invalid options -- '%s'\n", argv[0],
                            tmp);
                } else {
                    fprintf(stderr, "%s: Too many arguments\n", argv[0]);
                }
            } else {
                fprintf(stderr, "%s invalid options -- '%s'\n", argv[0],
                        (char *)((char *)argv[i]) + 1);
            }
            fprintf(stderr,
                    "Try `taosdemo --help' or `taosdemo --usage' for more "
                    "information.\n");
            exit(EXIT_FAILURE);
        }
    }

    int columnCount;
    for (columnCount = 0; columnCount < MAX_NUM_COLUMNS; columnCount++) {
        if (g_args.dataType[columnCount] == NULL) {
            break;
        }
    }

    if (0 == columnCount) {
        ERROR_EXIT("data type error!");
    }
    g_args.columnCount = columnCount;

    g_args.lenOfOneRow = 20;  // timestamp
    for (int c = 0; c < g_args.columnCount; c++) {
        switch (g_args.data_type[c]) {
            case TSDB_DATA_TYPE_BINARY:
                g_args.lenOfOneRow += g_args.binwidth + 3;
                break;

            case TSDB_DATA_TYPE_NCHAR:
                g_args.lenOfOneRow += g_args.binwidth + 3;
                break;

            case TSDB_DATA_TYPE_INT:
            case TSDB_DATA_TYPE_UINT:
                g_args.lenOfOneRow += INT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_BIGINT:
            case TSDB_DATA_TYPE_UBIGINT:
                g_args.lenOfOneRow += BIGINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_SMALLINT:
            case TSDB_DATA_TYPE_USMALLINT:
                g_args.lenOfOneRow += SMALLINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_TINYINT:
            case TSDB_DATA_TYPE_UTINYINT:
                g_args.lenOfOneRow += TINYINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_BOOL:
                g_args.lenOfOneRow += BOOL_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_FLOAT:
                g_args.lenOfOneRow += FLOAT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_DOUBLE:
                g_args.lenOfOneRow += DOUBLE_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_TIMESTAMP:
                g_args.lenOfOneRow += TIMESTAMP_BUFF_LEN;
                break;

            default:
                errorPrint2("get error data type : %s\n", g_args.dataType[c]);
                exit(EXIT_FAILURE);
        }
    }

    if (((arguments->debug_print) && (NULL != arguments->metaFile)) ||
        arguments->verbose_print) {
        printf(
            "##################################################################"
            "#\n");
        printf("# meta file:                         %s\n",
               arguments->metaFile);
        printf("# Server IP:                         %s:%hu\n",
               arguments->host == NULL ? "localhost" : arguments->host,
               arguments->port);
        printf("# User:                              %s\n", arguments->user);
        printf("# Password:                          %s\n",
               arguments->password);
        printf("# Use metric:                        %s\n",
               arguments->use_metric ? "true" : "false");
        if (*(arguments->dataType)) {
            printf("# Specified data type:               ");
            for (int c = 0; c < MAX_NUM_COLUMNS; c++)
                if (arguments->dataType[c])
                    printf("%s,", arguments->dataType[c]);
                else
                    break;
            printf("\n");
        }
        printf("# Insertion interval:                %" PRIu64 "\n",
               arguments->insert_interval);
        printf("# Number of records per req:         %u\n",
               arguments->reqPerReq);
        printf("# Max SQL length:                    %" PRIu64 "\n",
               arguments->max_sql_len);
        printf("# Length of Binary:                  %d\n",
               arguments->binwidth);
        printf("# Number of Threads:                 %d\n",
               arguments->nthreads);
        printf("# Number of Tables:                  %" PRId64 "\n",
               arguments->ntables);
        printf("# Number of Data per Table:          %" PRId64 "\n",
               arguments->insertRows);
        printf("# Database name:                     %s\n",
               arguments->database);
        printf("# Table prefix:                      %s\n",
               arguments->tb_prefix);
        if (arguments->disorderRatio) {
            printf("# Data order:                        %d\n",
                   arguments->disorderRatio);
            printf("# Data out of order rate:            %d\n",
                   arguments->disorderRange);
        }
        printf("# Delete method:                     %d\n",
               arguments->method_of_delete);
        printf("# Answer yes when prompt:            %d\n",
               arguments->answer_yes);
        printf("# Print debug info:                  %d\n",
               arguments->debug_print);
        printf("# Print verbose info:                %d\n",
               arguments->verbose_print);
        printf(
            "##################################################################"
            "#\n");

        prompt();
    }
}

#if 0
static const char charNum[] = "0123456789";

static void nonrand_string(char *, int) __attribute__ ((unused));   // reserve for debugging purpose
static void nonrand_string(char *str, int size)
{
    str[0] = 0;
    if (size > 0) {
        int n;
        for (n = 0; n < size; n++) {
            str[n] = charNum[n % 10];
        }
        str[n] = 0;
    }
}
#endif

void testMetaFile() {
    if (INSERT_TEST == g_args.test_mode) {
        if (g_Dbs.cfgDir[0]) taos_options(TSDB_OPTION_CONFIGDIR, g_Dbs.cfgDir);

        insertTestProcess();

    } else if (QUERY_TEST == g_args.test_mode) {
        if (g_queryInfo.cfgDir[0])
            taos_options(TSDB_OPTION_CONFIGDIR, g_queryInfo.cfgDir);

        queryTestProcess();

    } else if (SUBSCRIBE_TEST == g_args.test_mode) {
        if (g_queryInfo.cfgDir[0])
            taos_options(TSDB_OPTION_CONFIGDIR, g_queryInfo.cfgDir);

        subscribeTestProcess();

    } else {
        ;
    }
}

void testCmdLine() {
    if (strlen(configDir)) {
        wordexp_t full_path;
        if (wordexp(configDir, &full_path, 0) != 0) {
            errorPrint("Invalid path %s\n", configDir);
            return;
        }
        taos_options(TSDB_OPTION_CONFIGDIR, full_path.we_wordv[0]);
        wordfree(&full_path);
    }

    g_args.test_mode = INSERT_TEST;
    insertTestProcess();

    if (g_Dbs.aggr_func) {
        queryAggrFunc();
    }
}

void querySqlFile(TAOS *taos, char *sqlFile) {
    FILE *fp = fopen(sqlFile, "r");
    if (fp == NULL) {
        printf("failed to open file %s, reason:%s\n", sqlFile, strerror(errno));
        return;
    }

    int    read_len = 0;
    char * cmd = calloc(1, TSDB_MAX_BYTES_PER_ROW);
    size_t cmd_len = 0;
    char * line = NULL;
    size_t line_len = 0;

    double t = taosGetTimestampMs();

    while ((read_len = tgetline(&line, &line_len, fp)) != -1) {
        if (read_len >= TSDB_MAX_BYTES_PER_ROW) continue;
        line[--read_len] = '\0';

        if (read_len == 0 || isCommentLine(line)) {  // line starts with #
            continue;
        }

        if (line[read_len - 1] == '\\') {
            line[read_len - 1] = ' ';
            memcpy(cmd + cmd_len, line, read_len);
            cmd_len += read_len;
            continue;
        }

        memcpy(cmd + cmd_len, line, read_len);
        if (0 != queryDbExec(taos, cmd, NO_INSERT_TYPE, false)) {
            errorPrint2("%s() LN%d, queryDbExec %s failed!\n", __func__,
                        __LINE__, cmd);
            tmfree(cmd);
            tmfree(line);
            tmfclose(fp);
            return;
        }
        memset(cmd, 0, TSDB_MAX_BYTES_PER_ROW);
        cmd_len = 0;
    }

    t = taosGetTimestampMs() - t;
    printf("run %s took %.6f second(s)\n\n", sqlFile, t);

    tmfree(cmd);
    tmfree(line);
    tmfclose(fp);
    return;
}

void setParaFromArg() {
    char type[20];
    char length[20];
    if (g_args.host) {
        tstrncpy(g_Dbs.host, g_args.host, MAX_HOSTNAME_SIZE);
    } else {
        tstrncpy(g_Dbs.host, "127.0.0.1", MAX_HOSTNAME_SIZE);
    }

    if (g_args.user) {
        tstrncpy(g_Dbs.user, g_args.user, MAX_USERNAME_SIZE);
    }

    tstrncpy(g_Dbs.password, g_args.password, SHELL_MAX_PASSWORD_LEN);

    if (g_args.port) {
        g_Dbs.port = g_args.port;
    }

    g_Dbs.threadCount = g_args.nthreads;
    g_Dbs.threadCountForCreateTbl = g_args.nthreads;

    g_Dbs.dbCount = 1;
    g_Dbs.db[0].drop = true;

    tstrncpy(g_Dbs.db[0].dbName, g_args.database, TSDB_DB_NAME_LEN);
    g_Dbs.db[0].dbCfg.replica = g_args.replica;
    tstrncpy(g_Dbs.db[0].dbCfg.precision, "ms", SMALL_BUFF_LEN);

    tstrncpy(g_Dbs.resultFile, g_args.output_file, MAX_FILE_NAME_LEN);

    g_Dbs.use_metric = g_args.use_metric;

    g_Dbs.aggr_func = g_args.aggr_func;

    char   dataString[TSDB_MAX_BYTES_PER_ROW];
    char * data_type = g_args.data_type;
    char **dataType = g_args.dataType;

    memset(dataString, 0, TSDB_MAX_BYTES_PER_ROW);

    if ((data_type[0] == TSDB_DATA_TYPE_BINARY) ||
        (data_type[0] == TSDB_DATA_TYPE_BOOL) ||
        (data_type[0] == TSDB_DATA_TYPE_NCHAR)) {
        g_Dbs.aggr_func = false;
    }

    if (g_args.use_metric) {
        g_Dbs.db[0].superTblCount = 1;
        tstrncpy(g_Dbs.db[0].superTbls[0].stbName, "meters",
                 TSDB_TABLE_NAME_LEN);
        g_Dbs.db[0].superTbls[0].childTblCount = g_args.ntables;
        g_Dbs.threadCount = g_args.nthreads;
        g_Dbs.threadCountForCreateTbl = g_args.nthreads;
        g_Dbs.asyncMode = g_args.async_mode;

        g_Dbs.db[0].superTbls[0].autoCreateTable = PRE_CREATE_SUBTBL;
        g_Dbs.db[0].superTbls[0].childTblExists = TBL_NO_EXISTS;
        g_Dbs.db[0].superTbls[0].disorderRange = g_args.disorderRange;
        g_Dbs.db[0].superTbls[0].disorderRatio = g_args.disorderRatio;
        tstrncpy(g_Dbs.db[0].superTbls[0].childTblPrefix, g_args.tb_prefix,
                 TBNAME_PREFIX_LEN);
        tstrncpy(g_Dbs.db[0].superTbls[0].dataSource, "rand", SMALL_BUFF_LEN);

        if (g_args.iface == INTERFACE_BUT) {
            g_Dbs.db[0].superTbls[0].iface = TAOSC_IFACE;
        } else {
            g_Dbs.db[0].superTbls[0].iface = g_args.iface;
        }
        tstrncpy(g_Dbs.db[0].superTbls[0].startTimestamp,
                 "2017-07-14 10:40:00.000", MAX_TB_NAME_SIZE);
        g_Dbs.db[0].superTbls[0].timeStampStep = g_args.timestamp_step;

        g_Dbs.db[0].superTbls[0].insertRows = g_args.insertRows;
        g_Dbs.db[0].superTbls[0].maxSqlLen = g_args.max_sql_len;

        g_Dbs.db[0].superTbls[0].columnCount = 0;
        for (int i = 0; i < MAX_NUM_COLUMNS; i++) {
            if (data_type[i] == TSDB_DATA_TYPE_NULL) {
                break;
            }

            g_Dbs.db[0].superTbls[0].columns[i].data_type = data_type[i];
            tstrncpy(g_Dbs.db[0].superTbls[0].columns[i].dataType, dataType[i],
                     min(DATATYPE_BUFF_LEN, strlen(dataType[i]) + 1));
            if (1 == regexMatch(dataType[i],
                                "^(NCHAR|BINARY)(\\([1-9][0-9]*\\))$",
                                REG_ICASE | REG_EXTENDED)) {
                sscanf(dataType[i], "%[^(](%[^)]", type, length);
                g_Dbs.db[0].superTbls[0].columns[i].dataLen = atoi(length);
                tstrncpy(g_Dbs.db[0].superTbls[0].columns[i].dataType, type,
                         min(DATATYPE_BUFF_LEN, strlen(type) + 1));
            } else {
                g_Dbs.db[0].superTbls[0].columns[i].dataLen = g_args.binwidth;
                tstrncpy(g_Dbs.db[0].superTbls[0].columns[i].dataType,
                         dataType[i],
                         min(DATATYPE_BUFF_LEN, strlen(dataType[i]) + 1));
            }
            g_Dbs.db[0].superTbls[0].columnCount++;
        }

        if (g_Dbs.db[0].superTbls[0].columnCount > g_args.columnCount) {
            g_Dbs.db[0].superTbls[0].columnCount = g_args.columnCount;
        } else {
            for (int i = g_Dbs.db[0].superTbls[0].columnCount;
                 i < g_args.columnCount; i++) {
                g_Dbs.db[0].superTbls[0].columns[i].data_type =
                    TSDB_DATA_TYPE_INT;
                tstrncpy(g_Dbs.db[0].superTbls[0].columns[i].dataType, "INT",
                         min(DATATYPE_BUFF_LEN, strlen("INT") + 1));
                g_Dbs.db[0].superTbls[0].columns[i].dataLen = 0;
                g_Dbs.db[0].superTbls[0].columnCount++;
            }
        }

        tstrncpy(g_Dbs.db[0].superTbls[0].tags[0].dataType, "INT",
                 min(DATATYPE_BUFF_LEN, strlen("INT") + 1));
        g_Dbs.db[0].superTbls[0].tags[0].dataLen = 0;

        tstrncpy(g_Dbs.db[0].superTbls[0].tags[1].dataType, "BINARY",
                 min(DATATYPE_BUFF_LEN, strlen("BINARY") + 1));
        g_Dbs.db[0].superTbls[0].tags[1].dataLen = g_args.binwidth;
        g_Dbs.db[0].superTbls[0].tagCount = 2;
    } else {
        g_Dbs.threadCountForCreateTbl = g_args.nthreads;
        g_Dbs.db[0].superTbls[0].tagCount = 0;
    }
}

int main(int argc, char *argv[]) {
    parse_args(argc, argv, &g_args);

    debugPrint("meta file: %s\n", g_args.metaFile);

    if (g_args.metaFile) {
        g_totalChildTables = 0;
        initOfInsertMeta();
        initOfQueryMeta();

        if (false == getInfoFromJsonFile(g_args.metaFile)) {
            printf("Failed to read %s\n", g_args.metaFile);
            return 1;
        }

        testMetaFile();
    } else {
        memset(&g_Dbs, 0, sizeof(SDbs));
        setParaFromArg();

        if (NULL != g_args.sqlFile) {
            TAOS *qtaos = taos_connect(g_Dbs.host, g_Dbs.user, g_Dbs.password,
                                       g_Dbs.db[0].dbName, g_Dbs.port);
            querySqlFile(qtaos, g_args.sqlFile);
            taos_close(qtaos);

        } else {
            testCmdLine();
        }

        if (g_dupstr) free(g_dupstr);
    }

    return 0;
}