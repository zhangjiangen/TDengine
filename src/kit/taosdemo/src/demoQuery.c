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

#include "demoQuery.h"
void *queryNtableAggrFunc(void *sarg);
void  selectAndGetResult(threadInfo *pThreadInfo, char *command);
void *queryStableAggrFunc(void *sarg);
void *superTableQuery(void *sarg);
void *specifiedTableQuery(void *sarg);
/* Globale Variables */
char *g_aggreFuncDemo[] = {"*",
                           "count(*)",
                           "avg(current)",
                           "sum(current)",
                           "max(current)",
                           "min(current)",
                           "first(current)",
                           "last(current)"};

char *g_aggreFunc[] = {"*",       "count(*)", "avg(C0)",   "sum(C0)",
                       "max(C0)", "min(C0)",  "first(C0)", "last(C0)"};

void initOfQueryMeta() {
    memset(&g_queryInfo, 0, sizeof(SQueryMetaInfo));

    // set default values
    tstrncpy(g_queryInfo.host, "127.0.0.1", MAX_HOSTNAME_SIZE);
    g_queryInfo.port = 6030;
    tstrncpy(g_queryInfo.user, TSDB_DEFAULT_USER, MAX_USERNAME_SIZE);
    tstrncpy(g_queryInfo.password, TSDB_DEFAULT_PASS, SHELL_MAX_PASSWORD_LEN);
}

int queryTestProcess() {
    setupForAnsiEscape();
    printfQueryMeta();
    resetAfterAnsiEscape();

    TAOS *taos = NULL;
    taos = taos_connect(g_queryInfo.host, g_queryInfo.user,
                        g_queryInfo.password, NULL, g_queryInfo.port);
    if (taos == NULL) {
        errorPrint("Failed to connect to TDengine, reason:%s\n",
                   taos_errstr(NULL));
        exit(EXIT_FAILURE);
    }

    if (0 != g_queryInfo.superQueryInfo.sqlCount) {
        getAllChildNameOfSuperTable(taos, g_queryInfo.dbName,
                                    g_queryInfo.superQueryInfo.stbName,
                                    &g_queryInfo.superQueryInfo.childTblName,
                                    &g_queryInfo.superQueryInfo.childTblCount);
    }

    prompt();

    if (g_args.debug_print || g_args.verbose_print) {
        printfQuerySystemInfo(taos);
    }

    if (0 == strncasecmp(g_queryInfo.queryMode, "rest", strlen("rest"))) {
        if (convertHostToServAddr(g_queryInfo.host, g_queryInfo.port,
                                  &g_queryInfo.serv_addr) != 0)
            ERROR_EXIT("convert host to server address");
    }

    pthread_t * pids = NULL;
    threadInfo *infos = NULL;
    //==== create sub threads for query from specify table
    int      nConcurrent = g_queryInfo.specifiedQueryInfo.concurrent;
    uint64_t nSqlCount = g_queryInfo.specifiedQueryInfo.sqlCount;

    uint64_t startTs = taosGetTimestampMs();

    if ((nSqlCount > 0) && (nConcurrent > 0)) {
        pids = calloc(1, nConcurrent * nSqlCount * sizeof(pthread_t));
        infos = calloc(1, nConcurrent * nSqlCount * sizeof(threadInfo));

        if ((NULL == pids) || (NULL == infos)) {
            taos_close(taos);
            ERROR_EXIT("memory allocation failed for create threads\n");
        }

        for (uint64_t i = 0; i < nSqlCount; i++) {
            for (int j = 0; j < nConcurrent; j++) {
                uint64_t    seq = i * nConcurrent + j;
                threadInfo *pThreadInfo = infos + seq;
                pThreadInfo->threadID = seq;
                pThreadInfo->querySeq = i;

                if (0 == strncasecmp(g_queryInfo.queryMode, "taosc", 5)) {
                    char sqlStr[TSDB_DB_NAME_LEN + 5];
                    sprintf(sqlStr, "USE %s", g_queryInfo.dbName);
                    if (0 != queryDbExec(taos, sqlStr, NO_INSERT_TYPE, false)) {
                        taos_close(taos);
                        free(infos);
                        free(pids);
                        errorPrint2("use database %s failed!\n\n",
                                    g_queryInfo.dbName);
                        return -1;
                    }
                }

                pThreadInfo->taos =
                    NULL;  // workaround to use separate taos connection;

                pthread_create(pids + seq, NULL, specifiedTableQuery,
                               pThreadInfo);
            }
        }
    } else {
        g_queryInfo.specifiedQueryInfo.concurrent = 0;
    }

    taos_close(taos);

    pthread_t * pidsOfSub = NULL;
    threadInfo *infosOfSub = NULL;
    //==== create sub threads for query from all sub table of the super table
    if ((g_queryInfo.superQueryInfo.sqlCount > 0) &&
        (g_queryInfo.superQueryInfo.threadCnt > 0)) {
        pidsOfSub =
            calloc(1, g_queryInfo.superQueryInfo.threadCnt * sizeof(pthread_t));
        infosOfSub = calloc(
            1, g_queryInfo.superQueryInfo.threadCnt * sizeof(threadInfo));

        if ((NULL == pidsOfSub) || (NULL == infosOfSub)) {
            free(infos);
            free(pids);

            ERROR_EXIT("memory allocation failed for create threads\n");
        }

        int64_t ntables = g_queryInfo.superQueryInfo.childTblCount;
        int     threads = g_queryInfo.superQueryInfo.threadCnt;

        int64_t a = ntables / threads;
        if (a < 1) {
            threads = ntables;
            a = 1;
        }

        int64_t b = 0;
        if (threads != 0) {
            b = ntables % threads;
        }

        uint64_t tableFrom = 0;
        for (int i = 0; i < threads; i++) {
            threadInfo *pThreadInfo = infosOfSub + i;
            pThreadInfo->threadID = i;

            pThreadInfo->start_table_from = tableFrom;
            pThreadInfo->ntables = i < b ? a + 1 : a;
            pThreadInfo->end_table_to =
                i < b ? tableFrom + a : tableFrom + a - 1;
            tableFrom = pThreadInfo->end_table_to + 1;
            pThreadInfo->taos =
                NULL;  // workaround to use separate taos connection;
            pthread_create(pidsOfSub + i, NULL, superTableQuery, pThreadInfo);
        }

        g_queryInfo.superQueryInfo.threadCnt = threads;
    } else {
        g_queryInfo.superQueryInfo.threadCnt = 0;
    }

    if ((nSqlCount > 0) && (nConcurrent > 0)) {
        for (int i = 0; i < nConcurrent; i++) {
            for (int j = 0; j < nSqlCount; j++) {
                pthread_join(pids[i * nSqlCount + j], NULL);
            }
        }
    }

    tmfree((char *)pids);
    tmfree((char *)infos);

    for (int i = 0; i < g_queryInfo.superQueryInfo.threadCnt; i++) {
        pthread_join(pidsOfSub[i], NULL);
    }

    tmfree((char *)pidsOfSub);
    tmfree((char *)infosOfSub);

    //  taos_close(taos);// workaround to use separate taos connection;
    uint64_t endTs = taosGetTimestampMs();

    uint64_t totalQueried = g_queryInfo.specifiedQueryInfo.totalQueried +
                            g_queryInfo.superQueryInfo.totalQueried;

    fprintf(stderr,
            "==== completed total queries: %" PRIu64
            ", the QPS of all threads: %10.3f====\n",
            totalQueried,
            (double)(totalQueried / ((endTs - startTs) / 1000.0)));
    return 0;
}

void queryAggrFunc() {
    // query data

    pthread_t   read_id;
    threadInfo *pThreadInfo = calloc(1, sizeof(threadInfo));
    assert(pThreadInfo);
    pThreadInfo->start_time = DEFAULT_START_TIME;  // 2017-07-14 10:40:00.000
    pThreadInfo->start_table_from = 0;

    if (g_args.use_metric) {
        pThreadInfo->ntables = g_Dbs.db[0].superTbls[0].childTblCount;
        pThreadInfo->end_table_to = g_Dbs.db[0].superTbls[0].childTblCount - 1;
        pThreadInfo->stbInfo = &g_Dbs.db[0].superTbls[0];
        tstrncpy(pThreadInfo->tb_prefix,
                 g_Dbs.db[0].superTbls[0].childTblPrefix, TBNAME_PREFIX_LEN);
    } else {
        pThreadInfo->ntables = g_args.ntables;
        pThreadInfo->end_table_to = g_args.ntables - 1;
        tstrncpy(pThreadInfo->tb_prefix, g_args.tb_prefix, TSDB_TABLE_NAME_LEN);
    }

    pThreadInfo->taos = taos_connect(g_Dbs.host, g_Dbs.user, g_Dbs.password,
                                     g_Dbs.db[0].dbName, g_Dbs.port);
    if (pThreadInfo->taos == NULL) {
        free(pThreadInfo);
        errorPrint2("Failed to connect to TDengine, reason:%s\n",
                    taos_errstr(NULL));
        exit(EXIT_FAILURE);
    }

    tstrncpy(pThreadInfo->filePath, g_Dbs.resultFile, MAX_FILE_NAME_LEN);

    if (!g_Dbs.use_metric) {
        pthread_create(&read_id, NULL, queryNtableAggrFunc, pThreadInfo);
    } else {
        pthread_create(&read_id, NULL, queryStableAggrFunc, pThreadInfo);
    }
    pthread_join(read_id, NULL);
    taos_close(pThreadInfo->taos);
    free(pThreadInfo);
}

void *superTableQuery(void *sarg) {
    char *sqlstr = calloc(1, BUFFER_SIZE);
    assert(sqlstr);

    threadInfo *pThreadInfo = (threadInfo *)sarg;

    setThreadName("superTableQuery");

    if (pThreadInfo->taos == NULL) {
        TAOS *taos = NULL;
        taos = taos_connect(g_queryInfo.host, g_queryInfo.user,
                            g_queryInfo.password, NULL, g_queryInfo.port);
        if (taos == NULL) {
            errorPrint("[%d] Failed to connect to TDengine, reason:%s\n",
                       pThreadInfo->threadID, taos_errstr(NULL));
            free(sqlstr);
            return NULL;
        } else {
            pThreadInfo->taos = taos;
        }
    }

    uint64_t st = 0;
    uint64_t et = (int64_t)g_queryInfo.superQueryInfo.queryInterval;

    uint64_t queryTimes = g_queryInfo.superQueryInfo.queryTimes;
    uint64_t totalQueried = 0;
    uint64_t startTs = taosGetTimestampMs();

    uint64_t lastPrintTime = taosGetTimestampMs();
    while (queryTimes--) {
        if (g_queryInfo.superQueryInfo.queryInterval &&
            (et - st) < (int64_t)g_queryInfo.superQueryInfo.queryInterval) {
            taosMsleep(g_queryInfo.superQueryInfo.queryInterval -
                       (et - st));  // ms
            // printf("========sleep duration:%"PRId64 "========inserted
            // rows:%d, table range:%d - %d\n", (1000 - (et - st)), i,
            // pThreadInfo->start_table_from, pThreadInfo->end_table_to);
        }

        st = taosGetTimestampMs();
        for (int i = pThreadInfo->start_table_from;
             i <= pThreadInfo->end_table_to; i++) {
            for (int j = 0; j < g_queryInfo.superQueryInfo.sqlCount; j++) {
                memset(sqlstr, 0, BUFFER_SIZE);
                replaceChildTblName(g_queryInfo.superQueryInfo.sql[j], sqlstr,
                                    i);
                if (g_queryInfo.superQueryInfo.result[j][0] != '\0') {
                    sprintf(pThreadInfo->filePath, "%s-%d",
                            g_queryInfo.superQueryInfo.result[j],
                            pThreadInfo->threadID);
                }
                selectAndGetResult(pThreadInfo, sqlstr);

                totalQueried++;
                g_queryInfo.superQueryInfo.totalQueried++;

                int64_t currentPrintTime = taosGetTimestampMs();
                int64_t endTs = taosGetTimestampMs();
                if (currentPrintTime - lastPrintTime > 30 * 1000) {
                    printf(
                        "thread[%d] has currently completed queries: %" PRIu64
                        ", QPS: %10.3f\n",
                        pThreadInfo->threadID, totalQueried,
                        (double)(totalQueried / ((endTs - startTs) / 1000.0)));
                    lastPrintTime = currentPrintTime;
                }
            }
        }
        et = taosGetTimestampMs();
        printf("####thread[%" PRId64
               "] complete all sqls to allocate all sub-tables[%" PRIu64
               " - %" PRIu64 "] once queries duration:%.4fs\n\n",
               taosGetSelfPthreadId(), pThreadInfo->start_table_from,
               pThreadInfo->end_table_to, (double)(et - st) / 1000.0);
    }

    free(sqlstr);
    return NULL;
}

void *specifiedTableQuery(void *sarg) {
    threadInfo *pThreadInfo = (threadInfo *)sarg;

    setThreadName("specTableQuery");

    if (pThreadInfo->taos == NULL) {
        TAOS *taos = NULL;
        taos = taos_connect(g_queryInfo.host, g_queryInfo.user,
                            g_queryInfo.password, NULL, g_queryInfo.port);
        if (taos == NULL) {
            errorPrint2("[%d] Failed to connect to TDengine, reason:%s\n",
                        pThreadInfo->threadID, taos_errstr(NULL));
            return NULL;
        } else {
            pThreadInfo->taos = taos;
        }
    }

    char sqlStr[TSDB_DB_NAME_LEN + 5];
    sprintf(sqlStr, "use %s", g_queryInfo.dbName);
    if (0 != queryDbExec(pThreadInfo->taos, sqlStr, NO_INSERT_TYPE, false)) {
        taos_close(pThreadInfo->taos);
        errorPrint("use database %s failed!\n\n", g_queryInfo.dbName);
        return NULL;
    }

    uint64_t st = 0;
    uint64_t et = 0;

    uint64_t queryTimes = g_queryInfo.specifiedQueryInfo.queryTimes;

    uint64_t totalQueried = 0;
    uint64_t lastPrintTime = taosGetTimestampMs();
    uint64_t startTs = taosGetTimestampMs();

    if (g_queryInfo.specifiedQueryInfo.result[pThreadInfo->querySeq][0] !=
        '\0') {
        sprintf(pThreadInfo->filePath, "%s-%d",
                g_queryInfo.specifiedQueryInfo.result[pThreadInfo->querySeq],
                pThreadInfo->threadID);
    }

    while (queryTimes--) {
        if (g_queryInfo.specifiedQueryInfo.queryInterval &&
            (et - st) < (int64_t)g_queryInfo.specifiedQueryInfo.queryInterval) {
            taosMsleep(g_queryInfo.specifiedQueryInfo.queryInterval -
                       (et - st));  // ms
        }

        st = taosGetTimestampMs();

        selectAndGetResult(
            pThreadInfo,
            g_queryInfo.specifiedQueryInfo.sql[pThreadInfo->querySeq]);

        et = taosGetTimestampMs();
        printf("=thread[%" PRId64 "] use %s complete one sql, Spent %10.3f s\n",
               taosGetSelfPthreadId(), g_queryInfo.queryMode,
               (et - st) / 1000.0);

        totalQueried++;
        g_queryInfo.specifiedQueryInfo.totalQueried++;

        uint64_t currentPrintTime = taosGetTimestampMs();
        uint64_t endTs = taosGetTimestampMs();
        if (currentPrintTime - lastPrintTime > 30 * 1000) {
            debugPrint("%s() LN%d, endTs=%" PRIu64 " ms, startTs=%" PRIu64
                       " ms\n",
                       __func__, __LINE__, endTs, startTs);
            printf("thread[%d] has currently completed queries: %" PRIu64
                   ", QPS: %10.6f\n",
                   pThreadInfo->threadID, totalQueried,
                   (double)(totalQueried / ((endTs - startTs) / 1000.0)));
            lastPrintTime = currentPrintTime;
        }
    }
    return NULL;
}

void *queryNtableAggrFunc(void *sarg) {
    threadInfo *pThreadInfo = (threadInfo *)sarg;
    TAOS *      taos = pThreadInfo->taos;
    setThreadName("queryNtableAggrFunc");
    char *command = calloc(1, BUFFER_SIZE);
    assert(command);

    uint64_t startTime = pThreadInfo->start_time;
    char *   tb_prefix = pThreadInfo->tb_prefix;
    FILE *   fp = fopen(pThreadInfo->filePath, "a");
    if (NULL == fp) {
        errorPrint2("fopen %s fail, reason:%s.\n", pThreadInfo->filePath,
                    strerror(errno));
        free(command);
        return NULL;
    }

    int64_t insertRows;
    /*  if (pThreadInfo->stbInfo) {
        insertRows = pThreadInfo->stbInfo->insertRows; //  nrecords_per_table;
        } else {
        */
    insertRows = g_args.insertRows;
    //  }

    int64_t ntables =
        pThreadInfo->ntables;  // pThreadInfo->end_table_to -
                               // pThreadInfo->start_table_from + 1;
    int64_t totalData = insertRows * ntables;
    bool    aggr_func = g_Dbs.aggr_func;

    char **aggreFunc;
    int    n;

    if (g_args.demo_mode) {
        aggreFunc = g_aggreFuncDemo;
        n = aggr_func ? (sizeof(g_aggreFuncDemo) / sizeof(g_aggreFuncDemo[0]))
                      : 2;
    } else {
        aggreFunc = g_aggreFunc;
        n = aggr_func ? (sizeof(g_aggreFunc) / sizeof(g_aggreFunc[0])) : 2;
    }

    if (!aggr_func) {
        printf(
            "\nThe first field is either Binary or Bool. Aggregation functions "
            "are not supported.\n");
    }
    printf("%" PRId64 " records:\n", totalData);
    fprintf(
        fp,
        "| QFunctions |    QRecords    |   QSpeed(R/s)   |  QLatency(ms) |\n");

    for (int j = 0; j < n; j++) {
        double   totalT = 0;
        uint64_t count = 0;
        for (int64_t i = 0; i < ntables; i++) {
            sprintf(command, "SELECT %s FROM %s%" PRId64 " WHERE ts>= %" PRIu64,
                    aggreFunc[j], tb_prefix, i, startTime);

            double t = taosGetTimestampUs();
            debugPrint("%s() LN%d, sql command: %s\n", __func__, __LINE__,
                       command);
            TAOS_RES *pSql = taos_query(taos, command);
            int32_t   code = taos_errno(pSql);

            if (code != 0) {
                errorPrint2("Failed to query:%s\n", taos_errstr(pSql));
                taos_free_result(pSql);
                taos_close(taos);
                fclose(fp);
                free(command);
                return NULL;
            }

            while (taos_fetch_row(pSql) != NULL) {
                count++;
            }

            t = taosGetTimestampUs() - t;
            totalT += t;

            taos_free_result(pSql);
        }

        fprintf(fp, "|%10s  |   %" PRId64 "   |  %12.2f   |   %10.2f  |\n",
                aggreFunc[j][0] == '*' ? "   *   " : aggreFunc[j], totalData,
                (double)(ntables * insertRows) / totalT, totalT / 1000000);
        printf("select %10s took %.6f second(s)\n", aggreFunc[j],
               totalT / 1000000);
    }
    fprintf(fp, "\n");
    fclose(fp);
    free(command);
    return NULL;
}

void *queryStableAggrFunc(void *sarg) {
    threadInfo *pThreadInfo = (threadInfo *)sarg;
    TAOS *      taos = pThreadInfo->taos;
    setThreadName("queryStableAggrFunc");
    char *command = calloc(1, BUFFER_SIZE);
    assert(command);

    FILE *fp = fopen(pThreadInfo->filePath, "a");
    if (NULL == fp) {
        printf("fopen %s fail, reason:%s.\n", pThreadInfo->filePath,
               strerror(errno));
        free(command);
        return NULL;
    }

    int64_t insertRows = pThreadInfo->stbInfo->insertRows;
    int64_t ntables =
        pThreadInfo->ntables;  // pThreadInfo->end_table_to -
                               // pThreadInfo->start_table_from + 1;
    int64_t totalData = insertRows * ntables;
    bool    aggr_func = g_Dbs.aggr_func;

    char **aggreFunc;
    int    n;

    if (g_args.demo_mode) {
        aggreFunc = g_aggreFuncDemo;
        n = aggr_func ? (sizeof(g_aggreFuncDemo) / sizeof(g_aggreFuncDemo[0]))
                      : 2;
    } else {
        aggreFunc = g_aggreFunc;
        n = aggr_func ? (sizeof(g_aggreFunc) / sizeof(g_aggreFunc[0])) : 2;
    }

    if (!aggr_func) {
        printf(
            "\nThe first field is either Binary or Bool. Aggregation functions "
            "are not supported.\n");
    }

    printf("%" PRId64 " records:\n", totalData);
    fprintf(fp, "Querying On %" PRId64 " records:\n", totalData);

    for (int j = 0; j < n; j++) {
        char condition[COND_BUF_LEN] = "\0";
        char tempS[64] = "\0";

        int64_t m = 10 < ntables ? 10 : ntables;

        for (int64_t i = 1; i <= m; i++) {
            if (i == 1) {
                if (g_args.demo_mode) {
                    sprintf(tempS, "groupid = %" PRId64 "", i);
                } else {
                    sprintf(tempS, "t0 = %" PRId64 "", i);
                }
            } else {
                if (g_args.demo_mode) {
                    sprintf(tempS, " or groupid = %" PRId64 " ", i);
                } else {
                    sprintf(tempS, " or t0 = %" PRId64 " ", i);
                }
            }
            strncat(condition, tempS, COND_BUF_LEN - 1);

            sprintf(command, "SELECT %s FROM meters WHERE %s", aggreFunc[j],
                    condition);

            printf("Where condition: %s\n", condition);

            debugPrint("%s() LN%d, sql command: %s\n", __func__, __LINE__,
                       command);
            fprintf(fp, "%s\n", command);

            double t = taosGetTimestampUs();

            TAOS_RES *pSql = taos_query(taos, command);
            int32_t   code = taos_errno(pSql);

            if (code != 0) {
                errorPrint2("Failed to query:%s\n", taos_errstr(pSql));
                taos_free_result(pSql);
                taos_close(taos);
                fclose(fp);
                free(command);
                return NULL;
            }
            int count = 0;
            while (taos_fetch_row(pSql) != NULL) {
                count++;
            }
            t = taosGetTimestampUs() - t;

            fprintf(fp, "| Speed: %12.2f(per s) | Latency: %.4f(ms) |\n",
                    ntables * insertRows / (t / 1000), t);
            printf("select %10s took %.6f second(s)\n\n", aggreFunc[j],
                   t / 1000000);

            taos_free_result(pSql);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    free(command);

    return NULL;
}

void selectAndGetResult(threadInfo *pThreadInfo, char *command) {
    if (0 == strncasecmp(g_queryInfo.queryMode, "taosc", strlen("taosc"))) {
        TAOS_RES *res = taos_query(pThreadInfo->taos, command);
        if (res == NULL || taos_errno(res) != 0) {
            errorPrint2("%s() LN%d, failed to execute sql:%s, reason:%s\n",
                        __func__, __LINE__, command, taos_errstr(res));
            taos_free_result(res);
            return;
        }

        fetchResult(res, pThreadInfo);
        taos_free_result(res);

    } else if (0 ==
               strncasecmp(g_queryInfo.queryMode, "rest", strlen("rest"))) {
        int retCode = postProceSql(g_queryInfo.host, &(g_queryInfo.serv_addr),
                                   g_queryInfo.port, command, pThreadInfo);
        if (0 != retCode) {
            printf("====restful return fail, threadID[%d]\n",
                   pThreadInfo->threadID);
        }

    } else {
        errorPrint2("%s() LN%d, unknown query mode: %s\n", __func__, __LINE__,
                    g_queryInfo.queryMode);
    }
}