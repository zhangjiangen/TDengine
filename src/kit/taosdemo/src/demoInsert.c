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

#include "demo.h"
#include "demoData.h"

static int calcRowLen(SSuperTable *superTbls) {
    int colIndex;
    int lenOfOneRow = 0;

    for (colIndex = 0; colIndex < superTbls->columnCount; colIndex++) {
        char *dataType = superTbls->columns[colIndex].dataType;

        switch (superTbls->columns[colIndex].data_type) {
            case TSDB_DATA_TYPE_BINARY:
                lenOfOneRow += superTbls->columns[colIndex].dataLen + 3;
                break;

            case TSDB_DATA_TYPE_NCHAR:
                lenOfOneRow += superTbls->columns[colIndex].dataLen + 3;
                break;

            case TSDB_DATA_TYPE_INT:
            case TSDB_DATA_TYPE_UINT:
                lenOfOneRow += INT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_BIGINT:
            case TSDB_DATA_TYPE_UBIGINT:
                lenOfOneRow += BIGINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_SMALLINT:
            case TSDB_DATA_TYPE_USMALLINT:
                lenOfOneRow += SMALLINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_TINYINT:
            case TSDB_DATA_TYPE_UTINYINT:
                lenOfOneRow += TINYINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_BOOL:
                lenOfOneRow += BOOL_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_FLOAT:
                lenOfOneRow += FLOAT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_DOUBLE:
                lenOfOneRow += DOUBLE_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_TIMESTAMP:
                lenOfOneRow += TIMESTAMP_BUFF_LEN;
                break;

            default:
                errorPrint("get error data type : %s\n", dataType);
                return -1;
        }
        lenOfOneRow += SML_LINE_SQL_SYNTAX_OFFSET;
    }

    superTbls->lenOfOneRow = lenOfOneRow + TIMESTAMP_BUFF_LEN;  // timestamp

    int tagIndex;
    int lenOfTagOfOneRow = 0;
    for (tagIndex = 0; tagIndex < superTbls->tagCount; tagIndex++) {
        char *dataType = superTbls->tags[tagIndex].dataType;
        switch (superTbls->tags[tagIndex].data_type) {
            case TSDB_DATA_TYPE_BINARY:
                lenOfTagOfOneRow += superTbls->tags[tagIndex].dataLen + 3;
                break;
            case TSDB_DATA_TYPE_NCHAR:
                lenOfTagOfOneRow += superTbls->tags[tagIndex].dataLen + 3;
                break;
            case TSDB_DATA_TYPE_INT:
            case TSDB_DATA_TYPE_UINT:
                lenOfTagOfOneRow +=
                    superTbls->tags[tagIndex].dataLen + INT_BUFF_LEN;
                break;
            case TSDB_DATA_TYPE_BIGINT:
            case TSDB_DATA_TYPE_UBIGINT:
                lenOfTagOfOneRow +=
                    superTbls->tags[tagIndex].dataLen + BIGINT_BUFF_LEN;
                break;
            case TSDB_DATA_TYPE_SMALLINT:
            case TSDB_DATA_TYPE_USMALLINT:
                lenOfTagOfOneRow +=
                    superTbls->tags[tagIndex].dataLen + SMALLINT_BUFF_LEN;
                break;
            case TSDB_DATA_TYPE_TINYINT:
            case TSDB_DATA_TYPE_UTINYINT:
                lenOfTagOfOneRow +=
                    superTbls->tags[tagIndex].dataLen + TINYINT_BUFF_LEN;
                break;
            case TSDB_DATA_TYPE_BOOL:
                lenOfTagOfOneRow +=
                    superTbls->tags[tagIndex].dataLen + BOOL_BUFF_LEN;
                break;
            case TSDB_DATA_TYPE_FLOAT:
                lenOfTagOfOneRow +=
                    superTbls->tags[tagIndex].dataLen + FLOAT_BUFF_LEN;
                break;
            case TSDB_DATA_TYPE_DOUBLE:
                lenOfTagOfOneRow +=
                    superTbls->tags[tagIndex].dataLen + DOUBLE_BUFF_LEN;
                break;
            default:
                errorPrint("get error tag type : %s\n", dataType);
                return -1;
        }
        lenOfOneRow += SML_LINE_SQL_SYNTAX_OFFSET;
    }

    lenOfTagOfOneRow +=
        2 * TSDB_TABLE_NAME_LEN * 2 + SML_LINE_SQL_SYNTAX_OFFSET;
    superTbls->lenOfOneRow += lenOfTagOfOneRow;

    superTbls->lenOfTagOfOneRow = lenOfTagOfOneRow;

    return 0;
}

static int getSuperTableFromServer(SDataBase *dbInfo) {
    TAOS *taos = NULL;
    taos = taos_connect(g_Dbs.host, g_Dbs.user, g_Dbs.password, dbInfo->dbName,
                        g_Dbs.port);
    if (taos == NULL) {
        errorPrint("Failed to connect to TDengine, reason:%s\n",
                   taos_errstr(NULL));
        return -1;
    }

    char        command[SQL_BUFF_LEN] = "\0";
    TAOS_RES *  res;
    TAOS_RES *  res2;
    TAOS_ROW    row = NULL;
    TAOS_ROW    row2 = NULL;
    TAOS_FIELD *fields;
    int         count = 0;
    int32_t     code;

    res = taos_query(taos, "show stables");
    code = taos_errno(res);
    if (code != 0) {
        printf("failed to show stables, reason: %s\n", taos_errstr(res));
        taos_free_result(res);
        return -1;
    }
    int stbIndex = 0;
    while ((row = taos_fetch_row(res)) != NULL) {
        snprintf(command, SQL_BUFF_LEN, "describe %s.%s", dbInfo->dbName,
                 (char *)row[TSDB_SHOW_STABLES_NAME_INDEX]);
        res2 = taos_query(taos, command);
        code = taos_errno(res2);
        if (code != 0) {
            printf("failed to run command %s, reason: %s\n", command,
                   taos_errstr(res2));
            taos_free_result(res2);
            return -1;
        }
        SSuperTable *superTbls = &(dbInfo->superTbls[stbIndex]);

        int tagIndex = 0;
        int columnIndex = 0;
        fields = taos_fetch_fields(res2);
        while ((row2 = taos_fetch_row(res2)) != NULL) {
            if (0 == count) {
                count++;
                continue;
            }

            if (strcmp((char *)row2[TSDB_DESCRIBE_METRIC_NOTE_INDEX], "TAG") ==
                0) {
                tstrncpy(superTbls->tags[tagIndex].field,
                         (char *)row2[TSDB_DESCRIBE_METRIC_FIELD_INDEX],
                         fields[TSDB_DESCRIBE_METRIC_FIELD_INDEX].bytes);
                if (0 ==
                    strncasecmp((char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                                "INT", strlen("INT"))) {
                    superTbls->tags[tagIndex].data_type = TSDB_DATA_TYPE_INT;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "TINYINT", strlen("TINYINT"))) {
                    superTbls->tags[tagIndex].data_type =
                        TSDB_DATA_TYPE_TINYINT;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "SMALLINT", strlen("SMALLINT"))) {
                    superTbls->tags[tagIndex].data_type =
                        TSDB_DATA_TYPE_SMALLINT;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "BIGINT", strlen("BIGINT"))) {
                    superTbls->tags[tagIndex].data_type = TSDB_DATA_TYPE_BIGINT;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "FLOAT", strlen("FLOAT"))) {
                    superTbls->tags[tagIndex].data_type = TSDB_DATA_TYPE_FLOAT;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "DOUBLE", strlen("DOUBLE"))) {
                    superTbls->tags[tagIndex].data_type = TSDB_DATA_TYPE_DOUBLE;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "BINARY", strlen("BINARY"))) {
                    superTbls->tags[tagIndex].data_type = TSDB_DATA_TYPE_BINARY;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "NCHAR", strlen("NCHAR"))) {
                    superTbls->tags[tagIndex].data_type = TSDB_DATA_TYPE_NCHAR;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "BOOL", strlen("BOOL"))) {
                    superTbls->tags[tagIndex].data_type = TSDB_DATA_TYPE_BOOL;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "TIMESTAMP", strlen("TIMESTAMP"))) {
                    superTbls->tags[tagIndex].data_type =
                        TSDB_DATA_TYPE_TIMESTAMP;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "TINYINT UNSIGNED",
                               strlen("TINYINT UNSIGNED"))) {
                    superTbls->tags[tagIndex].data_type =
                        TSDB_DATA_TYPE_UTINYINT;
                    tstrncpy(
                        superTbls->tags[tagIndex].dataType, "UTINYINT",
                        min(DATATYPE_BUFF_LEN,
                            fields[TSDB_DESCRIBE_METRIC_TYPE_INDEX].bytes) +
                            1);
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "SMALLINT UNSIGNED",
                               strlen("SMALLINT UNSIGNED"))) {
                    superTbls->tags[tagIndex].data_type =
                        TSDB_DATA_TYPE_USMALLINT;
                    tstrncpy(
                        superTbls->tags[tagIndex].dataType, "USMALLINT",
                        min(DATATYPE_BUFF_LEN,
                            fields[TSDB_DESCRIBE_METRIC_TYPE_INDEX].bytes) +
                            1);
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "INT UNSIGNED", strlen("INT UNSIGNED"))) {
                    superTbls->tags[tagIndex].data_type = TSDB_DATA_TYPE_UINT;
                    tstrncpy(
                        superTbls->tags[tagIndex].dataType, "UINT",
                        min(DATATYPE_BUFF_LEN,
                            fields[TSDB_DESCRIBE_METRIC_TYPE_INDEX].bytes) +
                            1);
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "BIGINT UNSIGNED", strlen("BIGINT UNSIGNED"))) {
                    superTbls->tags[tagIndex].data_type =
                        TSDB_DATA_TYPE_UBIGINT;
                    tstrncpy(
                        superTbls->tags[tagIndex].dataType, "UBIGINT",
                        min(DATATYPE_BUFF_LEN,
                            fields[TSDB_DESCRIBE_METRIC_TYPE_INDEX].bytes) +
                            1);
                } else {
                    superTbls->tags[tagIndex].data_type = TSDB_DATA_TYPE_NULL;
                }
                superTbls->tags[tagIndex].dataLen =
                    *((int *)row2[TSDB_DESCRIBE_METRIC_LENGTH_INDEX]);
                tstrncpy(superTbls->tags[tagIndex].note,
                         (char *)row2[TSDB_DESCRIBE_METRIC_NOTE_INDEX],
                         min(NOTE_BUFF_LEN,
                             fields[TSDB_DESCRIBE_METRIC_NOTE_INDEX].bytes) +
                             1);
                if (strstr((char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                           "UNSIGNED") == NULL) {
                    tstrncpy(
                        superTbls->tags[tagIndex].dataType,
                        (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                        min(DATATYPE_BUFF_LEN,
                            fields[TSDB_DESCRIBE_METRIC_TYPE_INDEX].bytes) +
                            1);
                }
                tagIndex++;
            } else {
                tstrncpy(superTbls->columns[columnIndex].field,
                         (char *)row2[TSDB_DESCRIBE_METRIC_FIELD_INDEX],
                         fields[TSDB_DESCRIBE_METRIC_FIELD_INDEX].bytes);

                if (0 == strncasecmp(
                             (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                             "INT", strlen("INT")) &&
                    strstr((char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                           "UNSIGNED") == NULL) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_INT;
                } else if (0 == strncasecmp(
                                    (char *)
                                        row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                                    "TINYINT", strlen("TINYINT")) &&
                           strstr((char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                                  "UNSIGNED") == NULL) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_TINYINT;
                } else if (0 == strncasecmp(
                                    (char *)
                                        row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                                    "SMALLINT", strlen("SMALLINT")) &&
                           strstr((char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                                  "UNSIGNED") == NULL) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_SMALLINT;
                } else if (0 == strncasecmp(
                                    (char *)
                                        row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                                    "BIGINT", strlen("BIGINT")) &&
                           strstr((char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                                  "UNSIGNED") == NULL) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_BIGINT;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "FLOAT", strlen("FLOAT"))) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_FLOAT;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "DOUBLE", strlen("DOUBLE"))) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_DOUBLE;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "BINARY", strlen("BINARY"))) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_BINARY;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "NCHAR", strlen("NCHAR"))) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_NCHAR;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "BOOL", strlen("BOOL"))) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_BOOL;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "TIMESTAMP", strlen("TIMESTAMP"))) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_TIMESTAMP;
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "TINYINT UNSIGNED",
                               strlen("TINYINT UNSIGNED"))) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_UTINYINT;
                    tstrncpy(
                        superTbls->columns[columnIndex].dataType, "UTINYINT",
                        min(DATATYPE_BUFF_LEN,
                            fields[TSDB_DESCRIBE_METRIC_TYPE_INDEX].bytes) +
                            1);
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "SMALLINT UNSIGNED",
                               strlen("SMALLINT UNSIGNED"))) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_USMALLINT;
                    tstrncpy(
                        superTbls->columns[columnIndex].dataType, "USMALLINT",
                        min(DATATYPE_BUFF_LEN,
                            fields[TSDB_DESCRIBE_METRIC_TYPE_INDEX].bytes) +
                            1);
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "INT UNSIGNED", strlen("INT UNSIGNED"))) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_UINT;
                    tstrncpy(
                        superTbls->columns[columnIndex].dataType, "UINT",
                        min(DATATYPE_BUFF_LEN,
                            fields[TSDB_DESCRIBE_METRIC_TYPE_INDEX].bytes) +
                            1);
                } else if (0 ==
                           strncasecmp(
                               (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                               "BIGINT UNSIGNED", strlen("BIGINT UNSIGNED"))) {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_UBIGINT;
                    tstrncpy(
                        superTbls->columns[columnIndex].dataType, "UBIGINT",
                        min(DATATYPE_BUFF_LEN,
                            fields[TSDB_DESCRIBE_METRIC_TYPE_INDEX].bytes) +
                            1);
                } else {
                    superTbls->columns[columnIndex].data_type =
                        TSDB_DATA_TYPE_NULL;
                }
                superTbls->columns[columnIndex].dataLen =
                    *((int *)row2[TSDB_DESCRIBE_METRIC_LENGTH_INDEX]);
                tstrncpy(superTbls->columns[columnIndex].note,
                         (char *)row2[TSDB_DESCRIBE_METRIC_NOTE_INDEX],
                         min(NOTE_BUFF_LEN,
                             fields[TSDB_DESCRIBE_METRIC_NOTE_INDEX].bytes) +
                             1);

                if (strstr((char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                           "UNSIGNED") == NULL) {
                    tstrncpy(
                        superTbls->columns[columnIndex].dataType,
                        (char *)row2[TSDB_DESCRIBE_METRIC_TYPE_INDEX],
                        min(DATATYPE_BUFF_LEN,
                            fields[TSDB_DESCRIBE_METRIC_TYPE_INDEX].bytes) +
                            1);
                }

                columnIndex++;
            }
            count++;
        }

        superTbls->columnCount = columnIndex;
        superTbls->tagCount = tagIndex;
        taos_free_result(res);

        calcRowLen(superTbls);
        stbIndex++;
    }
    return 0;
}

static int createSuperTable(TAOS *taos, char *dbName, SSuperTable *superTbl,
                            char *command) {
    char cols[COL_BUFFER_LEN] = "\0";
    int  len = 0;

    int lenOfOneRow = 0;

    if (superTbl->columnCount == 0) {
        errorPrint("super table column count is %d\n", superTbl->columnCount);
        return -1;
    }

    for (int colIndex = 0; colIndex < superTbl->columnCount; colIndex++) {
        switch (superTbl->columns[colIndex].data_type) {
            case TSDB_DATA_TYPE_BINARY:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s(%d)",
                                colIndex, "BINARY",
                                superTbl->columns[colIndex].dataLen);
                lenOfOneRow += superTbl->columns[colIndex].dataLen + 3;
                break;

            case TSDB_DATA_TYPE_NCHAR:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s(%d)",
                                colIndex, "NCHAR",
                                superTbl->columns[colIndex].dataLen);
                lenOfOneRow += superTbl->columns[colIndex].dataLen + 3;
                break;

            case TSDB_DATA_TYPE_INT:
                if ((g_args.demo_mode) && (colIndex == 1)) {
                    len += snprintf(cols + len, COL_BUFFER_LEN - len,
                                    ", VOLTAGE INT");
                } else {
                    len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                    colIndex, "INT");
                }
                lenOfOneRow += INT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_BIGINT:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                colIndex, "BIGINT");
                lenOfOneRow += BIGINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_SMALLINT:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                colIndex, "SMALLINT");
                lenOfOneRow += SMALLINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_TINYINT:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                colIndex, "TINYINT");
                lenOfOneRow += TINYINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_BOOL:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                colIndex, "BOOL");
                lenOfOneRow += BOOL_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_FLOAT:
                if (g_args.demo_mode) {
                    if (colIndex == 0) {
                        len += snprintf(cols + len, COL_BUFFER_LEN - len,
                                        ", CURRENT FLOAT");
                    } else if (colIndex == 2) {
                        len += snprintf(cols + len, COL_BUFFER_LEN - len,
                                        ", PHASE FLOAT");
                    }
                } else {
                    len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                    colIndex, "FLOAT");
                }

                lenOfOneRow += FLOAT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_DOUBLE:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                colIndex, "DOUBLE");
                lenOfOneRow += DOUBLE_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_TIMESTAMP:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                colIndex, "TIMESTAMP");
                lenOfOneRow += TIMESTAMP_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_UTINYINT:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                colIndex, "TINYINT UNSIGNED");
                lenOfOneRow += TINYINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_USMALLINT:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                colIndex, "SMALLINT UNSIGNED");
                lenOfOneRow += SMALLINT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_UINT:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                colIndex, "INT UNSIGNED");
                lenOfOneRow += INT_BUFF_LEN;
                break;

            case TSDB_DATA_TYPE_UBIGINT:
                len += snprintf(cols + len, COL_BUFFER_LEN - len, ",C%d %s",
                                colIndex, "BIGINT UNSIGNED");
                lenOfOneRow += BIGINT_BUFF_LEN;
                break;

            default:
                taos_close(taos);
                errorPrint("config error data type : %s\n",
                           superTbl->columns[colIndex].dataType);
                return -1;
        }
    }

    superTbl->lenOfOneRow = lenOfOneRow + TIMESTAMP_BUFF_LEN;  // timestamp

    // save for creating child table
    superTbl->colsOfCreateChildTable =
        (char *)calloc(1, len + TIMESTAMP_BUFF_LEN);
    if (NULL == superTbl->colsOfCreateChildTable) {
        taos_close(taos);
        errorPrint("%s", "failed to allocate memory\n");
        return -1;
    }

    snprintf(superTbl->colsOfCreateChildTable, len + TIMESTAMP_BUFF_LEN,
             "(ts timestamp%s)", cols);
    verbosePrint("%s() LN%d: %s\n", __func__, __LINE__,
                 superTbl->colsOfCreateChildTable);

    if (superTbl->tagCount == 0) {
        errorPrint("super table tag count is %d\n", superTbl->tagCount);
        return -1;
    }

    char tags[TSDB_MAX_TAGS_LEN] = "\0";
    int  tagIndex;
    len = 0;

    int lenOfTagOfOneRow = 0;
    len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "(");
    for (tagIndex = 0; tagIndex < superTbl->tagCount; tagIndex++) {
        char *dataType = superTbl->tags[tagIndex].dataType;

        if (strcasecmp(dataType, "BINARY") == 0) {
            if ((g_args.demo_mode) && (tagIndex == 1)) {
                len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len,
                                "location BINARY(%d),",
                                superTbl->tags[tagIndex].dataLen);
            } else {
                len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len,
                                "T%d %s(%d),", tagIndex, "BINARY",
                                superTbl->tags[tagIndex].dataLen);
            }
            lenOfTagOfOneRow += superTbl->tags[tagIndex].dataLen + 3;
        } else if (strcasecmp(dataType, "NCHAR") == 0) {
            len +=
                snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s(%d),",
                         tagIndex, "NCHAR", superTbl->tags[tagIndex].dataLen);
            lenOfTagOfOneRow += superTbl->tags[tagIndex].dataLen + 3;
        } else if (strcasecmp(dataType, "INT") == 0) {
            if ((g_args.demo_mode) && (tagIndex == 0)) {
                len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len,
                                "groupId INT, ");
            } else {
                len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                                tagIndex, "INT");
            }
            lenOfTagOfOneRow += superTbl->tags[tagIndex].dataLen + INT_BUFF_LEN;
        } else if (strcasecmp(dataType, "BIGINT") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "BIGINT");
            lenOfTagOfOneRow +=
                superTbl->tags[tagIndex].dataLen + BIGINT_BUFF_LEN;
        } else if (strcasecmp(dataType, "SMALLINT") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "SMALLINT");
            lenOfTagOfOneRow +=
                superTbl->tags[tagIndex].dataLen + SMALLINT_BUFF_LEN;
        } else if (strcasecmp(dataType, "TINYINT") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "TINYINT");
            lenOfTagOfOneRow +=
                superTbl->tags[tagIndex].dataLen + TINYINT_BUFF_LEN;
        } else if (strcasecmp(dataType, "BOOL") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "BOOL");
            lenOfTagOfOneRow +=
                superTbl->tags[tagIndex].dataLen + BOOL_BUFF_LEN;
        } else if (strcasecmp(dataType, "FLOAT") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "FLOAT");
            lenOfTagOfOneRow +=
                superTbl->tags[tagIndex].dataLen + FLOAT_BUFF_LEN;
        } else if (strcasecmp(dataType, "DOUBLE") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "DOUBLE");
            lenOfTagOfOneRow +=
                superTbl->tags[tagIndex].dataLen + DOUBLE_BUFF_LEN;
        } else if (strcasecmp(dataType, "UTINYINT") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "TINYINT UNSIGNED");
            lenOfTagOfOneRow +=
                superTbl->tags[tagIndex].dataLen + TINYINT_BUFF_LEN;
        } else if (strcasecmp(dataType, "USMALLINT") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "SMALLINT UNSIGNED");
            lenOfTagOfOneRow +=
                superTbl->tags[tagIndex].dataLen + SMALLINT_BUFF_LEN;
        } else if (strcasecmp(dataType, "UINT") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "INT UNSIGNED");
            lenOfTagOfOneRow += superTbl->tags[tagIndex].dataLen + INT_BUFF_LEN;
        } else if (strcasecmp(dataType, "UBIGINT") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "BIGINT UNSIGNED");
            lenOfTagOfOneRow +=
                superTbl->tags[tagIndex].dataLen + BIGINT_BUFF_LEN;
        } else if (strcasecmp(dataType, "TIMESTAMP") == 0) {
            len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, "T%d %s,",
                            tagIndex, "TIMESTAMP");
            lenOfTagOfOneRow +=
                superTbl->tags[tagIndex].dataLen + TIMESTAMP_BUFF_LEN;
        } else {
            taos_close(taos);
            errorPrint("config error tag type : %s\n", dataType);
            return -1;
        }
    }

    len -= 1;
    len += snprintf(tags + len, TSDB_MAX_TAGS_LEN - len, ")");

    superTbl->lenOfTagOfOneRow = lenOfTagOfOneRow;

    snprintf(command, BUFFER_SIZE,
             superTbl->escapeChar
                 ? "CREATE TABLE IF NOT EXISTS %s.`%s` (ts TIMESTAMP%s) TAGS %s"
                 : "CREATE TABLE IF NOT EXISTS %s.%s (ts TIMESTAMP%s) TAGS %s",
             dbName, superTbl->stbName, cols, tags);
    if (0 != queryDbExec(taos, command, NO_INSERT_TYPE, false)) {
        errorPrint("create supertable %s failed!\n\n", superTbl->stbName);
        return -1;
    }

    debugPrint("create supertable %s success!\n\n", superTbl->stbName);
    return 0;
}

int createDatabases(char *command, SDataBase *dbInfo) {
    TAOS *taos = NULL;
    taos =
        taos_connect(g_Dbs.host, g_Dbs.user, g_Dbs.password, NULL, g_Dbs.port);
    if (taos == NULL) {
        errorPrint("Failed to connect to TDengine, reason:%s\n",
                   taos_errstr(NULL));
        return -1;
    }
    if (dbInfo->drop) {
        sprintf(command, "drop database if exists %s;", dbInfo->dbName);
        if (0 != queryDbExec(taos, command, NO_INSERT_TYPE, false)) {
            taos_close(taos);
            return -1;
        }

        int dataLen = 0;
        dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                            "CREATE DATABASE IF NOT EXISTS %s", dbInfo->dbName);

        if (dbInfo->dbCfg.blocks > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " BLOCKS %d", dbInfo->dbCfg.blocks);
        }
        if (dbInfo->dbCfg.cache > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " CACHE %d", dbInfo->dbCfg.cache);
        }
        if (dbInfo->dbCfg.days > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " DAYS %d", dbInfo->dbCfg.days);
        }
        if (dbInfo->dbCfg.keep > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " KEEP %d", dbInfo->dbCfg.keep);
        }
        if (dbInfo->dbCfg.quorum > 1) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " QUORUM %d", dbInfo->dbCfg.quorum);
        }
        if (dbInfo->dbCfg.replica > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " REPLICA %d", dbInfo->dbCfg.replica);
        }
        if (dbInfo->dbCfg.update > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " UPDATE %d", dbInfo->dbCfg.update);
        }
        if (dbInfo->dbCfg.minRows > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " MINROWS %d", dbInfo->dbCfg.minRows);
        }
        if (dbInfo->dbCfg.maxRows > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " MAXROWS %d", dbInfo->dbCfg.maxRows);
        }
        if (dbInfo->dbCfg.comp > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " COMP %d", dbInfo->dbCfg.comp);
        }
        if (dbInfo->dbCfg.walLevel > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " wal %d", dbInfo->dbCfg.walLevel);
        }
        if (dbInfo->dbCfg.cacheLast > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " CACHELAST %d", dbInfo->dbCfg.cacheLast);
        }
        if (dbInfo->dbCfg.fsync > 0) {
            dataLen += snprintf(command + dataLen, BUFFER_SIZE - dataLen,
                                " FSYNC %d", dbInfo->dbCfg.fsync);
        }
        dataLen += snprintf(
            command + dataLen, BUFFER_SIZE - dataLen, " precision \'%s\';",
            dbInfo->dbCfg.precision == TSDB_TIME_PRECISION_MILLI   ? "ms"
            : dbInfo->dbCfg.precision == TSDB_TIME_PRECISION_MICRO ? "us"
                                                                   : "ns");

        if (0 != queryDbExec(taos, command, NO_INSERT_TYPE, false)) {
            taos_close(taos);
            errorPrint("\ncreate database %s failed!\n\n", dbInfo->dbName);
            return -1;
        }
        printf("\ncreate database %s success!\n\n", dbInfo->dbName);
    }
    return 0;
}

int createStables(SDataBase *dbInfo) {
    TAOS *taos = NULL;
    int   ret = 0;
    taos =
        taos_connect(g_Dbs.host, g_Dbs.user, g_Dbs.password, NULL, g_Dbs.port);
    if (taos == NULL) {
        errorPrint("Failed to connect to TDengine, reason:%s\n",
                   taos_errstr(NULL));
        return -1;
    }

    debugPrint("%s() LN%d supertbl count:%" PRIu64 "\n", __func__, __LINE__,
               dbInfo->superTblCount);

    for (uint64_t j = 0; j < dbInfo->superTblCount; j++) {
        char *cmd = calloc(1, BUFFER_SIZE);
        if (NULL == cmd) {
            errorPrint("%s", "failed to allocate memory\n");
            return -1;
        }
        ret =
            createSuperTable(taos, dbInfo->dbName, &dbInfo->superTbls[j], cmd);
        tmfree(cmd);

        if (0 != ret) {
            errorPrint("create super table %s failed!\n\n",
                       dbInfo->superTbls[j].stbName);
            return -1;
        }
    }
    taos_close(taos);
    return 0;
}

static void *createTable(void *sarg) {
    threadInfo * pThreadInfo = (threadInfo *)sarg;
    SSuperTable *stbInfo = pThreadInfo->stbInfo;

    setThreadName("createTable");

    uint64_t lastPrintTime = taosGetTimestampMs();

    int buff_len = BUFFER_SIZE;

    pThreadInfo->buffer = calloc(1, buff_len);
    if (NULL == pThreadInfo->buffer) {
        errorPrint("%s", "failed to allocate memory\n");
        return NULL;
    }

    int len = 0;
    int batchNum = 0;

    verbosePrint("%s() LN%d: Creating table from %" PRIu64 " to %" PRIu64 "\n",
                 __func__, __LINE__, pThreadInfo->start_table_from,
                 pThreadInfo->end_table_to);

    for (uint64_t i = pThreadInfo->start_table_from;
         i <= pThreadInfo->end_table_to; i++) {
        if (0 == g_Dbs.use_metric) {
            snprintf(pThreadInfo->buffer, buff_len,
                     g_args.escapeChar
                         ? "CREATE TABLE IF NOT EXISTS %s.`%s%" PRIu64 "` %s;"
                         : "CREATE TABLE IF NOT EXISTS %s.%s%" PRIu64 " %s;",
                     pThreadInfo->db_name, g_args.tb_prefix, i,
                     pThreadInfo->cols);
            batchNum++;
        } else {
            if (stbInfo == NULL) {
                free(pThreadInfo->buffer);
                errorPrint(
                    "%s() LN%d, use metric, but super table info is NULL\n",
                    __func__, __LINE__);
                exit(EXIT_FAILURE);
            } else {
                if (0 == len) {
                    batchNum = 0;
                    memset(pThreadInfo->buffer, 0, buff_len);
                    len += snprintf(pThreadInfo->buffer + len, buff_len - len,
                                    "CREATE TABLE ");
                }

                char *tagsValBuf =
                    (char *)calloc(TSDB_MAX_ALLOWED_SQL_LEN + 1, 1);
                if (NULL == tagsValBuf) {
                    errorPrint("%s", "failed to allocate memory\n");
                    return NULL;
                }

                if (0 == stbInfo->tagSource) {
                    if (generateTagValuesForStb(stbInfo, i, tagsValBuf)) {
                        tmfree(tagsValBuf);
                        tmfree(pThreadInfo->buffer);
                        exit(EXIT_FAILURE);
                    }
                } else {
                    snprintf(tagsValBuf, TSDB_MAX_ALLOWED_SQL_LEN, "(%s)",
                             stbInfo->tagDataBuf +
                                 stbInfo->lenOfTagOfOneRow *
                                     (i % stbInfo->tagSampleCount));
                }
                len += snprintf(
                    pThreadInfo->buffer + len, buff_len - len,
                    stbInfo->escapeChar ? "if not exists %s.`%s%" PRIu64
                                          "` using %s.`%s` tags %s "
                                        : "if not exists %s.%s%" PRIu64
                                          " using %s.%s tags %s ",
                    pThreadInfo->db_name, stbInfo->childTblPrefix, i,
                    pThreadInfo->db_name, stbInfo->stbName, tagsValBuf);
                tmfree(tagsValBuf);
                batchNum++;
                if ((batchNum < stbInfo->batchCreateTableNum) &&
                    ((buff_len - len) >=
                     (stbInfo->lenOfTagOfOneRow + EXTRA_SQL_LEN))) {
                    continue;
                }
            }
        }

        len = 0;

        if (0 != queryDbExec(pThreadInfo->taos, pThreadInfo->buffer,
                             NO_INSERT_TYPE, false)) {
            errorPrint("queryDbExec() failed. buffer:\n%s\n",
                       pThreadInfo->buffer);
            free(pThreadInfo->buffer);
            return NULL;
        }
        pThreadInfo->tables_created += batchNum;
        uint64_t currentPrintTime = taosGetTimestampMs();
        if (currentPrintTime - lastPrintTime > PRINT_STAT_INTERVAL) {
            printf("thread[%d] already create %" PRIu64 " - %" PRIu64
                   " tables\n",
                   pThreadInfo->threadID, pThreadInfo->start_table_from, i);
            lastPrintTime = currentPrintTime;
        }
    }

    if (0 != len) {
        if (0 != queryDbExec(pThreadInfo->taos, pThreadInfo->buffer,
                             NO_INSERT_TYPE, false)) {
            errorPrint("queryDbExec() failed. buffer:\n%s\n",
                       pThreadInfo->buffer);
        }
        pThreadInfo->tables_created += batchNum;
    }
    free(pThreadInfo->buffer);
    return NULL;
}

int startMultiThreadCreateChildTable(char *cols, int threads,
                                     uint64_t tableFrom, int64_t ntables,
                                     char *db_name, SSuperTable *stbInfo) {
    pthread_t *pids = calloc(1, threads * sizeof(pthread_t));
    if (NULL == pids) {
        errorPrint("%s", "failed to allocate memory\n");
        return -1;
    }
    threadInfo *infos = calloc(1, threads * sizeof(threadInfo));
    if (NULL == infos) {
        errorPrint("%s", "failed to allocate memory\n");
        tmfree(pids);
        return -1;
    }

    if (threads < 1) {
        threads = 1;
    }

    int64_t a = ntables / threads;
    if (a < 1) {
        threads = (int)ntables;
        a = 1;
    }

    int64_t b = 0;
    b = ntables % threads;

    for (int64_t i = 0; i < threads; i++) {
        threadInfo *pThreadInfo = infos + i;
        pThreadInfo->threadID = (int)i;
        tstrncpy(pThreadInfo->db_name, db_name, TSDB_DB_NAME_LEN);
        pThreadInfo->stbInfo = stbInfo;
        verbosePrint("%s() %d db_name: %s\n", __func__, __LINE__, db_name);
        pThreadInfo->taos = taos_connect(g_Dbs.host, g_Dbs.user, g_Dbs.password,
                                         db_name, g_Dbs.port);
        if (pThreadInfo->taos == NULL) {
            errorPrint("failed to connect to TDengine, reason:%s\n",
                       taos_errstr(NULL));
            free(pids);
            free(infos);
            return -1;
        }

        pThreadInfo->start_table_from = tableFrom;
        pThreadInfo->ntables = i < b ? a + 1 : a;
        pThreadInfo->end_table_to = i < b ? tableFrom + a : tableFrom + a - 1;
        tableFrom = pThreadInfo->end_table_to + 1;
        pThreadInfo->use_metric = true;
        pThreadInfo->cols = cols;
        pThreadInfo->minDelay = UINT64_MAX;
        pThreadInfo->tables_created = 0;
        pthread_create(pids + i, NULL, createTable, pThreadInfo);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(pids[i], NULL);
    }

    for (int i = 0; i < threads; i++) {
        threadInfo *pThreadInfo = infos + i;
        taos_close(pThreadInfo->taos);

        g_actualChildTables += pThreadInfo->tables_created;
    }

    free(pids);
    free(infos);

    return 0;
}

int createChildTables(SDataBase *dbInfo) {
    int32_t code = 0;
    fprintf(stderr, "creating %" PRId64 " table(s) with %d thread(s)\n\n",
            g_totalChildTables, g_Dbs.threadCountForCreateTbl);
    if (g_fpOfInsertResult) {
        fprintf(g_fpOfInsertResult,
                "creating %" PRId64 " table(s) with %d thread(s)\n\n",
                g_totalChildTables, g_Dbs.threadCountForCreateTbl);
    }
    double start = (double)taosGetTimestampMs();
    char   tblColsBuf[TSDB_MAX_BYTES_PER_ROW];
    int    len;

    if (g_Dbs.use_metric) {
        if (dbInfo->superTblCount > 0) {
            // with super table
            for (int j = 0; j < dbInfo->superTblCount; j++) {
                if ((AUTO_CREATE_SUBTBL ==
                     dbInfo->superTbls[j].autoCreateTable) ||
                    (TBL_ALREADY_EXISTS ==
                     dbInfo->superTbls[j].childTblExists)) {
                    continue;
                }
                verbosePrint("%s() LN%d: %s\n", __func__, __LINE__,
                             dbInfo->superTbls[j].colsOfCreateChildTable);
                uint64_t startFrom = 0;

                verbosePrint("%s() LN%d: create %" PRId64
                             " child tables from %" PRIu64 "\n",
                             __func__, __LINE__, g_totalChildTables, startFrom);

                code = startMultiThreadCreateChildTable(
                    dbInfo->superTbls[j].colsOfCreateChildTable,
                    g_Dbs.threadCountForCreateTbl, startFrom,
                    dbInfo->superTbls[j].childTblCount, dbInfo->dbName,
                    &(dbInfo->superTbls[j]));
                if (code) {
                    errorPrint(
                        "startMultiThreadCreateChildTable() "
                        "failed stable %d\n",
                        j);
                    return code;
                }
            }
        }
    } else {
        // normal table
        len = snprintf(tblColsBuf, TSDB_MAX_BYTES_PER_ROW, "(TS TIMESTAMP");
        for (int j = 0; j < g_args.columnCount; j++) {
            if ((strncasecmp(g_args.dataType[j], "BINARY", strlen("BINARY")) ==
                 0) ||
                (strncasecmp(g_args.dataType[j], "NCHAR", strlen("NCHAR")) ==
                 0)) {
                snprintf(tblColsBuf + len, TSDB_MAX_BYTES_PER_ROW - len,
                         ",C%d %s(%d)", j, g_args.dataType[j], g_args.binwidth);
            } else {
                snprintf(tblColsBuf + len, TSDB_MAX_BYTES_PER_ROW - len,
                         ",C%d %s", j, g_args.dataType[j]);
            }
            len = (int)strlen(tblColsBuf);
        }

        snprintf(tblColsBuf + len, TSDB_MAX_BYTES_PER_ROW - len, ")");

        verbosePrint(
            "%s() LN%d: dbName: %s num of tb: %" PRId64 " schema: %s\n",
            __func__, __LINE__, dbInfo->dbName, g_args.ntables, tblColsBuf);
        code = startMultiThreadCreateChildTable(
            tblColsBuf, g_Dbs.threadCountForCreateTbl, 0, g_args.ntables,
            dbInfo->dbName, NULL);
        if (code) {
            errorPrint("%s",
                       " startMultiThreadCreateChildTable() "
                       "failed\n");
            return code;
        }
    }

    double end = (double)taosGetTimestampMs();
    fprintf(stderr,
            "\nSpent %.4f seconds to create %" PRId64
            " table(s) with %d thread(s), actual %" PRId64
            " table(s) created\n\n",
            (end - start) / 1000.0, g_totalChildTables,
            g_Dbs.threadCountForCreateTbl, g_actualChildTables);
    if (g_fpOfInsertResult) {
        fprintf(g_fpOfInsertResult,
                "\nSpent %.4f seconds to create %" PRId64
                " table(s) with %d thread(s), actual %" PRId64
                " table(s) created\n\n",
                (end - start) / 1000.0, g_totalChildTables,
                g_Dbs.threadCountForCreateTbl, g_actualChildTables);
    }
    return code;
}

void postFreeResource() {
    tmfclose(g_fpOfInsertResult);
    tmfree(g_dupstr);
    for (int i = 0; i < g_Dbs.dbCount; i++) {
        for (uint64_t j = 0; j < g_Dbs.db[i].superTblCount; j++) {
            tmfree(g_Dbs.db[i].superTbls[j].stmtBuffer);
            tmfree(g_Dbs.db[i].superTbls[j].colsOfCreateChildTable);
            tmfree(g_Dbs.db[i].superTbls[j].sampleDataBuf);
            for (int c = 0; c < g_Dbs.db[i].superTbls[j].columnCount; c++) {
                if (g_Dbs.db[i].superTbls[j].sampleBindBatchArray) {
                    tmfree((char *)((uintptr_t) *
                                    (uintptr_t *)(g_Dbs.db[i]
                                                      .superTbls[j]
                                                      .sampleBindBatchArray +
                                                  sizeof(char *) * c)));
                }
            }
            tmfree(g_Dbs.db[i].superTbls[j].sampleBindBatchArray);
            tmfree(g_Dbs.db[i].superTbls[j].bind_ts);
            tmfree(g_Dbs.db[i].superTbls[j].bind_ts_array);
            tmfree(g_Dbs.db[i].superTbls[j].bindParams);
            tmfree(g_Dbs.db[i].superTbls[j].is_null);

            if (0 != g_Dbs.db[i].superTbls[j].tagDataBuf) {
                tmfree(g_Dbs.db[i].superTbls[j].tagDataBuf);
                g_Dbs.db[i].superTbls[j].tagDataBuf = NULL;
            }
            if (0 != g_Dbs.db[i].superTbls[j].childTblName) {
                tmfree(g_Dbs.db[i].superTbls[j].childTblName);
                g_Dbs.db[i].superTbls[j].childTblName = NULL;
            }
        }

        for (uint64_t j = 0; j < g_Dbs.db[i].normalTblCount; j++) {
            tmfree(g_Dbs.db[i].normalTbls[j].tbName);
            tmfree(g_Dbs.db[i].normalTbls[j].smlHead);
            if (g_Dbs.db[i].iface == SML_IFACE &&
                g_Dbs.db[i].lineProtocol == TSDB_SML_JSON_PROTOCOL) {
                cJSON_Delete(g_Dbs.db[i].normalTbls[j].smlJsonTags);
            }
        }

        tmfree(g_Dbs.db[i].superTbls);
        tmfree(g_Dbs.db[i].normalTbls);
    }
    tmfree(g_Dbs.db);
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

    for (int l = 0; l < g_args.columnCount; l++) {
        if (g_sampleBindBatchArray) {
            tmfree((char *)((uintptr_t) * (uintptr_t *)(g_sampleBindBatchArray +
                                                        sizeof(char *) * l)));
        }
    }
    tmfree(g_sampleBindBatchArray);
}

static int32_t execInsert(threadInfo *pThreadInfo, uint32_t k) {
    int32_t   affectedRows;
    SDataBase dbInfo = g_Dbs.db[pThreadInfo->dbSeq];
    TAOS_RES *res;
    int32_t   code;
    uint16_t  iface = dbInfo.iface;

    debugPrint("[%d] %s() LN%d %s\n", pThreadInfo->threadID, __func__, __LINE__,
               (iface == TAOSC_IFACE)  ? "taosc"
               : (iface == REST_IFACE) ? "rest"
                                       : "stmt");

    switch (iface) {
        case TAOSC_IFACE:
            verbosePrint("[%d] %s() LN%d %s\n", pThreadInfo->threadID, __func__,
                         __LINE__, pThreadInfo->buffer);

            affectedRows = queryDbExec(pThreadInfo->taos, pThreadInfo->buffer,
                                       INSERT_TYPE, false);
            break;

        case REST_IFACE:
            verbosePrint("[%d] %s() LN%d %s\n", pThreadInfo->threadID, __func__,
                         __LINE__, pThreadInfo->buffer);

            if (0 != postProceSql(g_Dbs.host, g_Dbs.port, pThreadInfo->buffer,
                                  pThreadInfo)) {
                errorPrint("========restful return fail, threadID[%d]\n",
                           pThreadInfo->threadID);
                return -1;
            } else {
                affectedRows = k;
            }
            break;

        case STMT_IFACE:
            debugPrint("%s() LN%d, stmt=%p", __func__, __LINE__,
                       pThreadInfo->stmt);
            if (0 != taos_stmt_execute(pThreadInfo->stmt)) {
                errorPrint(
                    "%s() LN%d, failied to execute insert statement. reason: "
                    "%s\n",
                    __func__, __LINE__, taos_stmt_errstr(pThreadInfo->stmt));

                fprintf(stderr,
                        "\n\033[31m === Please reduce batch number if WAL size "
                        "exceeds limit. ===\033[0m\n\n");
                return -1;
            }
            affectedRows = k;
            break;
        case SML_IFACE:
            res = taos_schemaless_insert(
                pThreadInfo->taos, pThreadInfo->lines,
                dbInfo.lineProtocol == TSDB_SML_JSON_PROTOCOL ? 0 : k,
                dbInfo.lineProtocol, dbInfo.dbCfg.smlTsPrecision);
            code = taos_errno(res);
            affectedRows = taos_affected_rows(res);
            if (code != TSDB_CODE_SUCCESS) {
                errorPrint(
                    "%s() LN%d, failed to execute schemaless insert. reason: "
                    "%s\n",
                    __func__, __LINE__, taos_errstr(res));
                return -1;
            }
            break;
        default:
            errorPrint("Unknown insert mode: %d\n", dbInfo.iface);
            affectedRows = 0;
    }

    return affectedRows;
}

static void __attribute__((unused))
getTableName(char *pTblName, threadInfo *pThreadInfo, uint64_t tableSeq) {
    SSuperTable *stbInfo = pThreadInfo->stbInfo;
    if (stbInfo) {
        if (AUTO_CREATE_SUBTBL != stbInfo->autoCreateTable) {
            if (stbInfo->childTblLimit > 0) {
                snprintf(pTblName, TSDB_TABLE_NAME_LEN,
                         stbInfo->escapeChar ? "`%s`" : "%s",
                         stbInfo->childTblName +
                             (tableSeq - stbInfo->childTblOffset) *
                                 TSDB_TABLE_NAME_LEN);
            } else {
                verbosePrint("[%d] %s() LN%d: from=%" PRIu64 " count=%" PRId64
                             " seq=%" PRIu64 "\n",
                             pThreadInfo->threadID, __func__, __LINE__,
                             pThreadInfo->start_table_from,
                             pThreadInfo->ntables, tableSeq);
                snprintf(
                    pTblName, TSDB_TABLE_NAME_LEN,
                    stbInfo->escapeChar ? "`%s`" : "%s",
                    stbInfo->childTblName + tableSeq * TSDB_TABLE_NAME_LEN);
            }
        } else {
            snprintf(pTblName, TSDB_TABLE_NAME_LEN,
                     stbInfo->escapeChar ? "`%s%" PRIu64 "`" : "%s%" PRIu64 "",
                     stbInfo->childTblPrefix, tableSeq);
        }
    } else {
        snprintf(pTblName, TSDB_TABLE_NAME_LEN,
                 g_args.escapeChar ? "`%s%" PRIu64 "`" : "%s%" PRIu64 "",
                 g_args.tb_prefix, tableSeq);
    }
}

static int execStbBindParamBatch(threadInfo *pThreadInfo, SNormalTable *tbInfo,
                                 int64_t remainderRows, int64_t startTime) {
    TAOS_STMT *  stmt = pThreadInfo->stmt;
    SSuperTable *stbInfo = tbInfo->stbInfo;
    uint32_t     columnCount = stbInfo->columnCount;
    int64_t      samplePos = 0;

    uint32_t thisBatch = (uint32_t)(MAX_SAMPLES - samplePos);

    if (thisBatch > remainderRows) {
        thisBatch = remainderRows;
    }

    memset(stbInfo->bindParams, 0,
           (sizeof(TAOS_MULTI_BIND) * (columnCount + 1)));
    memset(stbInfo->is_null, 0, thisBatch);

    for (int c = 0; c < columnCount + 1; c++) {
        TAOS_MULTI_BIND *param =
            (TAOS_MULTI_BIND *)(stbInfo->bindParams +
                                sizeof(TAOS_MULTI_BIND) * c);

        char data_type;

        if (c == 0) {
            data_type = TSDB_DATA_TYPE_TIMESTAMP;
            param->buffer_length = sizeof(int64_t);
            param->buffer = stbInfo->bind_ts_array;

        } else {
            data_type = stbInfo->columns[c - 1].data_type;

            char *tmpP;

            switch (data_type) {
                case TSDB_DATA_TYPE_BINARY:
                    param->buffer_length = stbInfo->columns[c - 1].dataLen;

                    tmpP =
                        (char *)((uintptr_t) *
                                 (uintptr_t *)(stbInfo->sampleBindBatchArray +
                                               sizeof(char *) * (c - 1)));

                    verbosePrint("%s() LN%d, tmpP=%p pos=%" PRId64
                                 " width=%" PRIxPTR " position=%" PRId64 "\n",
                                 __func__, __LINE__, tmpP, samplePos,
                                 param->buffer_length,
                                 samplePos * param->buffer_length);

                    param->buffer =
                        (void *)(tmpP + samplePos * param->buffer_length);
                    break;

                case TSDB_DATA_TYPE_NCHAR:
                    param->buffer_length = stbInfo->columns[c - 1].dataLen;

                    tmpP =
                        (char *)((uintptr_t) *
                                 (uintptr_t *)(stbInfo->sampleBindBatchArray +
                                               sizeof(char *) * (c - 1)));

                    verbosePrint("%s() LN%d, tmpP=%p pos=%" PRId64
                                 " width=%" PRIxPTR " position=%" PRId64 "\n",
                                 __func__, __LINE__, tmpP, samplePos,
                                 param->buffer_length,
                                 samplePos * param->buffer_length);

                    param->buffer =
                        (void *)(tmpP + samplePos * param->buffer_length);
                    break;

                case TSDB_DATA_TYPE_INT:
                case TSDB_DATA_TYPE_UINT:
                    param->buffer_length = sizeof(int32_t);
                    param->buffer =
                        (void *)((uintptr_t) *
                                     (uintptr_t *)(stbInfo
                                                       ->sampleBindBatchArray +
                                                   sizeof(char *) * (c - 1)) +
                                 stbInfo->columns[c - 1].dataLen * samplePos);
                    break;

                case TSDB_DATA_TYPE_TINYINT:
                case TSDB_DATA_TYPE_UTINYINT:
                    param->buffer_length = sizeof(int8_t);
                    param->buffer =
                        (void *)((uintptr_t) *
                                     (uintptr_t *)(stbInfo
                                                       ->sampleBindBatchArray +
                                                   sizeof(char *) * (c - 1)) +
                                 stbInfo->columns[c - 1].dataLen * samplePos);
                    break;

                case TSDB_DATA_TYPE_SMALLINT:
                case TSDB_DATA_TYPE_USMALLINT:
                    param->buffer_length = sizeof(int16_t);
                    param->buffer =
                        (void *)((uintptr_t) *
                                     (uintptr_t *)(stbInfo
                                                       ->sampleBindBatchArray +
                                                   sizeof(char *) * (c - 1)) +
                                 stbInfo->columns[c - 1].dataLen * samplePos);
                    break;

                case TSDB_DATA_TYPE_BIGINT:
                case TSDB_DATA_TYPE_UBIGINT:
                    param->buffer_length = sizeof(int64_t);
                    param->buffer =
                        (void *)((uintptr_t) *
                                     (uintptr_t *)(stbInfo
                                                       ->sampleBindBatchArray +
                                                   sizeof(char *) * (c - 1)) +
                                 stbInfo->columns[c - 1].dataLen * samplePos);
                    break;

                case TSDB_DATA_TYPE_BOOL:
                    param->buffer_length = sizeof(int8_t);
                    param->buffer =
                        (void *)((uintptr_t) *
                                     (uintptr_t *)(stbInfo
                                                       ->sampleBindBatchArray +
                                                   sizeof(char *) * (c - 1)) +
                                 stbInfo->columns[c - 1].dataLen * samplePos);
                    break;

                case TSDB_DATA_TYPE_FLOAT:
                    param->buffer_length = sizeof(float);
                    param->buffer =
                        (void *)((uintptr_t) *
                                     (uintptr_t *)(stbInfo
                                                       ->sampleBindBatchArray +
                                                   sizeof(char *) * (c - 1)) +
                                 stbInfo->columns[c - 1].dataLen * samplePos);
                    break;

                case TSDB_DATA_TYPE_DOUBLE:
                    param->buffer_length = sizeof(double);
                    param->buffer =
                        (void *)((uintptr_t) *
                                     (uintptr_t *)(stbInfo
                                                       ->sampleBindBatchArray +
                                                   sizeof(char *) * (c - 1)) +
                                 stbInfo->columns[c - 1].dataLen * samplePos);
                    break;

                case TSDB_DATA_TYPE_TIMESTAMP:
                    param->buffer_length = sizeof(int64_t);
                    param->buffer =
                        (void *)((uintptr_t) *
                                     (uintptr_t *)(stbInfo
                                                       ->sampleBindBatchArray +
                                                   sizeof(char *) * (c - 1)) +
                                 stbInfo->columns[c - 1].dataLen * samplePos);
                    break;

                default:
                    errorPrint("wrong data type: %d\n", data_type);
                    return -1;
            }
        }

        param->buffer_type = data_type;
        param->length = calloc(1, sizeof(int32_t) * thisBatch);
        if (param->length == NULL) {
            errorPrint("%s", "failed to allocate memory\n");
            return -1;
        }

        for (int b = 0; b < thisBatch; b++) {
            if (param->buffer_type == TSDB_DATA_TYPE_NCHAR) {
                param->length[b] = (int32_t)strlen(
                    (char *)param->buffer + b * stbInfo->columns[c].dataLen);
            } else {
                param->length[b] = (int32_t)param->buffer_length;
            }
        }
        param->is_null = stbInfo->is_null;
        param->num = thisBatch;
    }

    uint32_t k;
    for (k = 0; k < thisBatch;) {
        /* columnCount + 1 (ts) */
        if (stbInfo->disorderRatio) {
            *(stbInfo->bind_ts_array + k) =
                startTime + getTSRandTail(stbInfo->timeStampStep, k,
                                          stbInfo->disorderRatio,
                                          stbInfo->disorderRange);
        } else {
            *(stbInfo->bind_ts_array + k) =
                startTime + stbInfo->timeStampStep * k;
        }

        debugPrint("%s() LN%d, k=%d ts=%" PRId64 "\n", __func__, __LINE__, k,
                   *(stbInfo->bind_ts_array + k));
        k++;

        samplePos++;
        if (samplePos == MAX_SAMPLES) {
            samplePos = 0;
        }
    }

    if (taos_stmt_bind_param_batch(stmt,
                                   (TAOS_MULTI_BIND *)stbInfo->bindParams)) {
        errorPrint("taos_stmt_bind_param_batch() failed! reason: %s\n",
                   taos_stmt_errstr(stmt));
        return -1;
    }

    for (int c = 0; c < stbInfo->columnCount + 1; c++) {
        TAOS_MULTI_BIND *param =
            (TAOS_MULTI_BIND *)(stbInfo->bindParams +
                                sizeof(TAOS_MULTI_BIND) * c);
        free(param->length);
    }

    // if msg > 3MB, break
    if (taos_stmt_add_batch(stmt)) {
        errorPrint("taos_stmt_add_batch() failed! reason: %s\n",
                   taos_stmt_errstr(stmt));
        return -1;
    }
    return k;
}

int32_t prepareStbStmt(threadInfo *pThreadInfo, SNormalTable *tbInfo,
                       int64_t remainderRows, int64_t startTime) {
    SSuperTable *stbInfo = tbInfo->stbInfo;

    int64_t tableSeq = tbInfo->tbSeq;
    char *  tableName = tbInfo->tbName;

    char *tagsArray = calloc(1, sizeof(TAOS_BIND) * stbInfo->tagCount);
    if (NULL == tagsArray) {
        errorPrint("%s", "failed to allocate memory\n");
        return -1;
    }
    char *tagsValBuf = (char *)calloc(TSDB_MAX_ALLOWED_SQL_LEN + 1, 1);
    if (NULL == tagsValBuf) {
        tmfree(tagsArray);
        errorPrint("%s", "failed to allocate memory\n");
        return -1;
    }

    pThreadInfo->stmt = taos_stmt_init(pThreadInfo->taos);

    if (NULL == pThreadInfo->stmt) {
        errorPrint("taos_stmt_init() failed! reason: %s\n",
                   taos_stmt_errstr(pThreadInfo->stmt));
        return -1;
    }
    TAOS_STMT *stmt = pThreadInfo->stmt;

    if (taos_stmt_prepare(stmt, stbInfo->stmtBuffer, 0)) {
        tmfree(tagsArray);
        tmfree(tagsValBuf);
        errorPrint("taos_stmt_prepare(%s) failed! reason: %s\n",
                   stbInfo->stmtBuffer, taos_stmt_errstr(stmt));
        return -1;
    }

    if (AUTO_CREATE_SUBTBL == stbInfo->autoCreateTable) {
        if (0 == stbInfo->tagSource) {
            if (generateTagValuesForStb(stbInfo, tableSeq, tagsValBuf)) {
                tmfree(tagsValBuf);
                tmfree(tagsArray);
                return -1;
            }
        } else {
            snprintf(
                tagsValBuf, TSDB_MAX_ALLOWED_SQL_LEN, "(%s)",
                stbInfo->tagDataBuf + stbInfo->lenOfTagOfOneRow *
                                          (tableSeq % stbInfo->tagSampleCount));
        }

        if (prepareStbStmtBindTag(tagsArray, stbInfo, tagsValBuf,
                                  pThreadInfo->time_precision)) {
            tmfree(tagsValBuf);
            tmfree(tagsArray);
            return -1;
        }

        if (taos_stmt_set_tbname_tags(stmt, tableName,
                                      (TAOS_BIND *)tagsArray)) {
            errorPrint("taos_stmt_set_tbname_tags() failed! reason: %s\n",
                       taos_stmt_errstr(stmt));
            return -1;
        }

    } else {
        if (taos_stmt_set_tbname(stmt, tableName)) {
            errorPrint("taos_stmt_set_tbname() failed! reason: %s\n",
                       taos_stmt_errstr(stmt));
            return -1;
        }
    }
    tmfree(tagsValBuf);
    tmfree(tagsArray);
    return execStbBindParamBatch(pThreadInfo, tbInfo, remainderRows, startTime);
}

void *syncWriteInterlace(void *sarg) {
    threadInfo *pThreadInfo = (threadInfo *)sarg;
    SDataBase   dbInfo = g_Dbs.db[pThreadInfo->dbSeq];
    pThreadInfo->totalInsertRows = 0;
    pThreadInfo->totalInsertRows = 0;
    int64_t  insertRows = dbInfo.insertRows;
    int64_t  interlaceRows = dbInfo.interlaceRows;
    int64_t  tableBatch = g_args.reqPerReq / dbInfo.interlaceRows;
    uint64_t lastPrintTime = taosGetTimestampMs();
    uint64_t startTs;
    uint64_t endTs;
    if (tableBatch == 0) {
        tableBatch = 1;
    }
    int      percentComplete = 0;
    uint64_t tableSeq = pThreadInfo->start_table_from;
    while (pThreadInfo->totalInsertRows < pThreadInfo->ntables * insertRows) {
        // startTs = taosGetTimestampUs();
        int32_t generated;
        for (int i = 0; i < tableBatch; i++) {
            SNormalTable *tbInfo = &(dbInfo.normalTbls[tableSeq]);
            if (tbInfo->rowsNeedInsert == 0) {
                continue;
            }
            int tmp;
            int batch;
            if (tbInfo->rowsNeedInsert > interlaceRows) {
                batch = interlaceRows;
                tbInfo->rowsNeedInsert -= interlaceRows;
            } else {
                batch = tbInfo->rowsNeedInsert;
                tbInfo->rowsNeedInsert = 0;
            }
            if (dbInfo.iface == STMT_IFACE) {
                tmp = prepareStbStmt(pThreadInfo, tbInfo, batch,
                                     tbInfo->currentTime);
            } else if (dbInfo.iface == SML_IFACE) {
                tmp = generateSmlMutablePart(pThreadInfo, tbInfo, batch,
                                             tbInfo->currentTime,
                                             dbInfo.lineProtocol);
            } else {
                tmp = generateStbProgressiveData(pThreadInfo, tbInfo, batch,
                                                 tbInfo->currentTime);
            }
            if (tmp < 0) {
                return NULL;
            }
            generated += tmp;
            tableSeq++;
            if (tableSeq > pThreadInfo->end_table_to) {
                tableSeq = pThreadInfo->start_table_from;
            }
        }
        startTs = taosGetTimestampUs();
        int64_t affectedRows = execInsert(pThreadInfo, generated);
        endTs = taosGetTimestampUs();
        uint64_t delay = endTs - startTs;
        performancePrint("%s() LN%d, insert execution time is %10.2f ms\n",
                         __func__, __LINE__, delay / 1000.0);
        verbosePrint("[%d] %s() LN%d affectedRows=%" PRId64 "\n",
                     pThreadInfo->threadID, __func__, __LINE__, affectedRows);
        if (delay > pThreadInfo->maxDelay) pThreadInfo->maxDelay = delay;
        if (delay < pThreadInfo->minDelay) pThreadInfo->minDelay = delay;
        pThreadInfo->cntDelay++;
        pThreadInfo->totalDelay += delay;
        if (affectedRows < 0) {
            errorPrint("affected rows: %" PRIu64 "\n", affectedRows);
            goto free_of_interlace;
        }
        pThreadInfo->totalAffectedRows += affectedRows;
        int currentPercent = (int)(pThreadInfo->totalAffectedRows * 100 /
                                   insertRows * pThreadInfo->ntables);
        if (currentPercent > percentComplete) {
            printf("[%d]:%d%%\n", pThreadInfo->threadID, currentPercent);
            percentComplete = currentPercent;
        }
        int64_t currentPrintTime = taosGetTimestampMs();
        if (currentPrintTime - lastPrintTime > 30 * 1000) {
            printf("thread[%d] has currently inserted rows: %" PRId64
                   ", affected rows: %" PRId64 "\n",
                   pThreadInfo->threadID, pThreadInfo->totalInsertRows,
                   pThreadInfo->totalAffectedRows);
            lastPrintTime = currentPrintTime;
        }
    }
    if (percentComplete < 100) {
        printf("[%d]:%d%%\n", pThreadInfo->threadID, percentComplete);
    }
free_of_interlace:
    printStatPerThread(pThreadInfo);
    return NULL;
}

void *syncWriteProgressive(void *sarg) {
    threadInfo *pThreadInfo = (threadInfo *)sarg;
    SDataBase   dbInfo = g_Dbs.db[pThreadInfo->dbSeq];
    uint64_t    lastPrintTime = taosGetTimestampMs();
    uint64_t    startTs = taosGetTimestampMs();

    uint64_t endTs;

    pThreadInfo->totalInsertRows = 0;
    pThreadInfo->totalAffectedRows = 0;

    int     percentComplete = 0;
    int64_t totalRows = 0;

    for (uint64_t tableSeq = pThreadInfo->start_table_from;
         tableSeq <= pThreadInfo->end_table_to; tableSeq++) {
        SNormalTable *tbInfo = &(dbInfo.normalTbls[tableSeq]);
        int64_t       start_time = tbInfo->stbInfo->startTime;
        int64_t timeStampStep = tbInfo->stbInfo ? tbInfo->stbInfo->timeStampStep
                                                : g_args.timestamp_step;
        uint64_t insertRows = tbInfo->stbInfo->insertRows;
        totalRows += insertRows;
        for (uint64_t i = 0; i < insertRows;) {
            // measure prepare + insert
            startTs = taosGetTimestampUs();

            int32_t generated;
            if (dbInfo.iface == STMT_IFACE) {
                generated = prepareStbStmt(pThreadInfo, tbInfo,
                                           (insertRows - i) < g_args.reqPerReq
                                               ? (insertRows - i)
                                               : g_args.reqPerReq,
                                           start_time);
            } else if (dbInfo.iface == SML_IFACE) {
                generated = generateSmlMutablePart(
                    pThreadInfo, tbInfo,
                    (insertRows - i) < g_args.reqPerReq ? (insertRows - i)
                                                        : g_args.reqPerReq,
                    start_time, dbInfo.lineProtocol);
            } else {
                generated = generateStbProgressiveData(
                    pThreadInfo, tbInfo,
                    (insertRows - i) < g_args.reqPerReq ? (insertRows - i)
                                                        : g_args.reqPerReq,
                    start_time);
            }

            verbosePrint("[%d] %s() LN%d generated=%d\n", pThreadInfo->threadID,
                         __func__, __LINE__, generated);

            if (generated > 0) {
                i += generated;
            } else {
                goto free_of_progressive;
            }

            start_time += generated * timeStampStep;
            pThreadInfo->totalInsertRows += generated;

            // only measure insert
            // startTs = taosGetTimestampUs();

            int32_t affectedRows = execInsert(pThreadInfo, generated);

            endTs = taosGetTimestampUs();

            if (dbInfo.iface == SML_IFACE) {
                tmfree(pThreadInfo->lines[0]);
            } else if (dbInfo.iface == STMT_IFACE) {
                taos_stmt_close(pThreadInfo->stmt);
            }

            uint64_t delay = endTs - startTs;
            performancePrint("%s() LN%d, insert execution time is %10.f ms\n",
                             __func__, __LINE__, delay / 1000.0);
            verbosePrint("[%d] %s() LN%d affectedRows=%d\n",
                         pThreadInfo->threadID, __func__, __LINE__,
                         affectedRows);

            if (delay > pThreadInfo->maxDelay) pThreadInfo->maxDelay = delay;
            if (delay < pThreadInfo->minDelay) pThreadInfo->minDelay = delay;
            pThreadInfo->cntDelay++;
            pThreadInfo->totalDelay += delay;

            if (affectedRows < 0) {
                errorPrint("affected rows: %d\n", affectedRows);
                goto free_of_progressive;
            }

            if (dbInfo.iface == SML_IFACE &&
                dbInfo.lineProtocol != TSDB_SML_JSON_PROTOCOL) {
                for (int index = 0; index < affectedRows; index++) {
                    tmfree(pThreadInfo->lines[index]);
                }
            }

            pThreadInfo->totalAffectedRows += affectedRows;

            int currentPercent =
                (int)(pThreadInfo->totalAffectedRows * 100 / totalRows);
            if (currentPercent > percentComplete) {
                printf("[%d]:%d%%\n", pThreadInfo->threadID, currentPercent);
                percentComplete = currentPercent;
            }
            int64_t currentPrintTime = taosGetTimestampMs();
            if (currentPrintTime - lastPrintTime > 30 * 1000) {
                printf("thread[%d] has currently inserted rows: %" PRId64
                       ", affected rows: %" PRId64 "\n",
                       pThreadInfo->threadID, pThreadInfo->totalInsertRows,
                       pThreadInfo->totalAffectedRows);
                lastPrintTime = currentPrintTime;
            }

            if (i >= insertRows) break;
        }  // insertRows

        if ((g_args.verbose_print) && (tableSeq == pThreadInfo->ntables - 1) &&
            (tbInfo->stbInfo) &&
            (0 == strncasecmp(tbInfo->stbInfo->dataSource, "sample",
                              strlen("sample")))) {
            verbosePrint("%s() LN%d samplePos=%" PRId64 "\n", __func__,
                         __LINE__, pThreadInfo->samplePos);
        }
    }  // tableSeq

    if (percentComplete < 100) {
        printf("[%d]:%d%%\n", pThreadInfo->threadID, percentComplete);
    }

free_of_progressive:
    printStatPerThread(pThreadInfo);
    return NULL;
}

int setUpStables(SDataBase *dbInfo) {
    dbInfo->normalTblCount = 0;
    if (dbInfo->iface == REST_IFACE) {
        if (convertHostToServAddr(g_Dbs.host, g_Dbs.port, &(g_Dbs.serv_addr)) !=
            0) {
            errorPrint("%s", "convert host to server address\n");
            return -1;
        }
    }

    for (uint64_t j = 0; j < dbInfo->superTblCount; j++) {
        SSuperTable *stbInfo = &dbInfo->superTbls[j];
        if (dbInfo->iface == SML_IFACE) {
            if (dbInfo->lineProtocol != TSDB_SML_LINE_PROTOCOL) {
                if (stbInfo->columnCount != 1) {
                    errorPrint(
                        "Schemaless telnet/json protocol can only "
                        "have 1 "
                        "column "
                        "instead of %d\n",
                        stbInfo->columnCount);
                    return -1;
                }
            }
        }
        if (calcRowLen(stbInfo)) {
            return -1;
        }

        if (prepareSampleForStb(stbInfo)) {
            return -1;
        }

        dbInfo->normalTblCount += stbInfo->childTblCount;

        if (dbInfo->iface == STMT_IFACE) {
            if (parseSamplefileToStmtBatch(stbInfo)) {
                return -1;
            }

            stbInfo->stmtBuffer = calloc(1, BUFFER_SIZE);
            if (stbInfo->stmtBuffer == NULL) {
                errorPrint("%s", "failed to allocate memory\n");
                return -1;
            }

            char *pstr = stbInfo->stmtBuffer;

            if (AUTO_CREATE_SUBTBL == stbInfo->autoCreateTable) {
                pstr += sprintf(pstr, "INSERT INTO ? USING %s TAGS(?",
                                stbInfo->stbName);
                for (int tag = 0; tag < (stbInfo->tagCount - 1); tag++) {
                    pstr += sprintf(pstr, ",?");
                }
                pstr += sprintf(pstr, ") VALUES(?");
            } else {
                pstr += sprintf(pstr, "INSERT INTO ? VALUES(?");
            }

            int columnCount = stbInfo->columnCount;

            for (int col = 0; col < columnCount; col++) {
                pstr += sprintf(pstr, ",?");
            }
            pstr += sprintf(pstr, ")");

            stbInfo->bind_ts_array =
                calloc(1, sizeof(int64_t) * g_args.reqPerReq);
            if (NULL == stbInfo->bind_ts_array) {
                errorPrint("%s", "failed to allocate memory\n");
                return -1;
            }

            stbInfo->bindParams =
                calloc(1, sizeof(TAOS_MULTI_BIND) * (stbInfo->columnCount + 1));
            if (NULL == stbInfo->bindParams) {
                errorPrint("%s", "failed to allocate memory\n");
                return -1;
            }

            stbInfo->is_null = calloc(1, g_args.reqPerReq);
            if (NULL == stbInfo->is_null) {
                errorPrint("%s", "failed to allocate memory\n");
                return -1;
            }

            stbInfo->bind_ts = calloc(1, sizeof(int64_t));
            if (NULL == stbInfo->bind_ts) {
                errorPrint("%s", "failed to allocate memory\n");
                return -1;
            }
        }
    }

    dbInfo->normalTbls = calloc(dbInfo->normalTblCount, sizeof(SNormalTable));
    if (NULL == dbInfo->normalTbls) {
        errorPrint("%s", "failed to allocate memory\n");
        return -1;
    }

    for (uint64_t j = 0; j < dbInfo->normalTblCount; j++) {
        uint64_t childTblOffset = 0;
        for (uint64_t k = 0; k < dbInfo->superTblCount; k++) {
            if ((j - childTblOffset) < dbInfo->superTbls[k].childTblCount) {
                SSuperTable *stbInfo = &(dbInfo->superTbls[k]);
                dbInfo->normalTbls[j].stbInfo = stbInfo;
                dbInfo->normalTbls[j].tbSeq = j - childTblOffset;
                dbInfo->normalTbls[j].tbName = calloc(1, TSDB_TABLE_NAME_LEN);
                if (dbInfo->normalTbls[j].tbName == NULL) {
                    errorPrint("%s", "failed to allocate memory\n");
                    return -1;
                }
                snprintf(dbInfo->normalTbls[j].tbName, TSDB_TABLE_NAME_LEN,
                         "%s%" PRIu64 "", stbInfo->childTblPrefix,
                         dbInfo->normalTbls[j].tbSeq);

                if (dbInfo->iface == SML_IFACE) {
                    if (dbInfo->lineProtocol == TSDB_SML_LINE_PROTOCOL ||
                        dbInfo->lineProtocol == TSDB_SML_TELNET_PROTOCOL) {
                        dbInfo->normalTbls[j].smlHead =
                            (char *)calloc(1, stbInfo->lenOfOneRow);
                        if (NULL == dbInfo->normalTbls[j].smlHead) {
                            errorPrint("%s", "failed to allocate memory\n");
                            return -1;
                        }
                        if (generateSmlConstPart(dbInfo->normalTbls[j].smlHead,
                                                 stbInfo, j - childTblOffset,
                                                 dbInfo->lineProtocol)) {
                            return -1;
                        }
                    } else {
                        dbInfo->normalTbls[j].smlJsonTags =
                            cJSON_CreateObject();
                        if (generateSmlJsonTags(
                                dbInfo->normalTbls[j].smlJsonTags, stbInfo,
                                j - childTblOffset)) {
                            return -1;
                        }
                    }
                }
                break;
            } else {
                childTblOffset += dbInfo->superTbls[k].childTblCount;
            }
        }
    }
    return 0;
}

int startMultiThreadInsertData(uint16_t iface, int dbSeq) {
    int64_t  a = g_Dbs.db[dbSeq].normalTblCount / g_Dbs.threadCount;
    uint64_t tableFrom = 0;
    if (a < 1) {
        g_Dbs.threadCount = (int)g_Dbs.db[dbSeq].normalTblCount;
        a = 1;
    }

    int64_t b = 0;
    if (g_Dbs.threadCount != 0) {
        b = g_Dbs.db[dbSeq].normalTblCount % g_Dbs.threadCount;
    }

    pthread_t *pids = calloc(1, g_Dbs.threadCount * sizeof(pthread_t));
    if (pids == NULL) {
        errorPrint("%s", "failed to allocate memory\n");
        return -1;
    }
    threadInfo *infos = calloc(1, g_Dbs.threadCount * sizeof(threadInfo));
    if (infos == NULL) {
        errorPrint("%s", "failed to allocate memory\n");
        tmfree(pids);
        return -1;
    }

    for (int i = 0; i < g_Dbs.threadCount; i++) {
        threadInfo *pThreadInfo = infos + i;
        pThreadInfo->dbSeq = dbSeq;
        pThreadInfo->threadID = i;
        pThreadInfo->start_table_from = tableFrom;
        pThreadInfo->ntables = i < b ? a + 1 : a;
        pThreadInfo->end_table_to = i < b ? tableFrom + a : tableFrom + a - 1;
        tableFrom = pThreadInfo->end_table_to + 1;
        pThreadInfo->minDelay = UINT64_MAX;

        if (iface != REST_IFACE) {
            // t_info->taos = taos;
            pThreadInfo->taos =
                taos_connect(g_Dbs.host, g_Dbs.user, g_Dbs.password,
                             g_Dbs.db[dbSeq].dbName, g_Dbs.port);
            if (NULL == pThreadInfo->taos) {
                free(infos);
                errorPrint(
                    "connect to server fail from insert sub "
                    "thread,reason:%s\n ",
                    taos_errstr(NULL));
                return -1;
            }
        }

        if (iface == REST_IFACE || iface == TAOSC_IFACE) {
            pThreadInfo->buffer = calloc(1, TSDB_MAX_ALLOWED_SQL_LEN);
            if (NULL == pThreadInfo->buffer) {
                errorPrint("%s", "failed to allocate memory\n");
                return -1;
            }
        }

        if (iface == SML_IFACE) {
            if (g_Dbs.db[dbSeq].lineProtocol != TSDB_SML_JSON_PROTOCOL) {
                pThreadInfo->lines = calloc(g_args.reqPerReq, sizeof(char *));
            } else {
                pThreadInfo->lines = calloc(1, sizeof(char *));
            }
            if (NULL == pThreadInfo->lines) {
                errorPrint("%s", "failed to allocate memory\n");
                return -1;
            }
        }

        if (iface == REST_IFACE) {
#ifdef WINDOWS
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            SOCKET sockfd;
#else
            int sockfd;
#endif
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
#ifdef WINDOWS
                errorPrint("Could not create socket : %d", WSAGetLastError());
#endif
                debugPrint("%s() LN%d, sockfd=%d\n", __func__, __LINE__,
                           sockfd);
                errorPrint("%s\n", "failed to create socket");
                return -1;
            }

            int retConn = connect(sockfd, (struct sockaddr *)&(g_Dbs.serv_addr),
                                  sizeof(struct sockaddr));
            debugPrint("%s() LN%d connect() return %d\n", __func__, __LINE__,
                       retConn);
            if (retConn < 0) {
                errorPrint("%s\n", "failed to connect");
                return -1;
            }
            pThreadInfo->sockfd = sockfd;
        }

        tsem_init(&(pThreadInfo->lock_sem), 0, 0);
        if (g_Dbs.db[dbSeq].interlaceRows > 0) {
            pthread_create(pids + i, NULL, syncWriteInterlace, pThreadInfo);
        } else {
            pthread_create(pids + i, NULL, syncWriteProgressive, pThreadInfo);
        }
    }

    int64_t start = taosGetTimestampUs();

    for (int i = 0; i < g_Dbs.threadCount; i++) {
        pthread_join(pids[i], NULL);
    }

    uint64_t totalDelay = 0;
    uint64_t maxDelay = 0;
    uint64_t minDelay = UINT64_MAX;
    uint64_t cntDelay = 0;
    double   avgDelay = 0;

    for (int i = 0; i < g_Dbs.threadCount; i++) {
        threadInfo *pThreadInfo = infos + i;
        tmfree(pThreadInfo->buffer);
        tmfree(pThreadInfo->lines);
        tsem_destroy(&(pThreadInfo->lock_sem));
        taos_close(pThreadInfo->taos);
        if (iface == REST_IFACE) {
#ifdef WINDOWS
            closesocket(pThreadInfo->sockfd);
            WSACleanup();
#else
            close(pThreadInfo->sockfd);
#endif
        }

        debugPrint("%s() LN%d, [%d] totalInsert=%" PRIu64
                   " totalAffected=%" PRIu64 "\n",
                   __func__, __LINE__, pThreadInfo->threadID,
                   pThreadInfo->totalInsertRows,
                   pThreadInfo->totalAffectedRows);

        g_Dbs.db[dbSeq].totalAffectedRows += pThreadInfo->totalAffectedRows;
        g_Dbs.db[dbSeq].totalInsertRows += pThreadInfo->totalInsertRows;

        totalDelay += pThreadInfo->totalDelay;
        cntDelay += pThreadInfo->cntDelay;
        if (pThreadInfo->maxDelay > maxDelay) maxDelay = pThreadInfo->maxDelay;
        if (pThreadInfo->minDelay < minDelay) minDelay = pThreadInfo->minDelay;
    }

    if (cntDelay == 0) cntDelay = 1;
    avgDelay = (double)totalDelay / cntDelay;

    int64_t end = taosGetTimestampUs();
    int64_t t = end - start;
    if (0 == t) t = 1;

    double tInMs = (double)t / 1000000.0;

    fprintf(stderr,
            "Spent %.4f seconds to insert rows: %" PRIu64
            ", affected rows: %" PRIu64
            " with %d thread(s) into %s. %.2f records/second\n\n",
            tInMs, g_Dbs.db[dbSeq].totalInsertRows,
            g_Dbs.db[dbSeq].totalAffectedRows, g_Dbs.threadCount,
            g_Dbs.db[dbSeq].dbName,
            (double)(g_Dbs.db[dbSeq].totalInsertRows / tInMs));

    if (g_fpOfInsertResult) {
        fprintf(g_fpOfInsertResult,
                "Spent %.4f seconds to insert rows: %" PRIu64
                ", affected rows: %" PRIu64
                " with %d thread(s) into %s. %.2f records/second\n\n",
                tInMs, g_Dbs.db[dbSeq].totalInsertRows,
                g_Dbs.db[dbSeq].totalAffectedRows, g_Dbs.threadCount,
                g_Dbs.db[dbSeq].dbName,
                (double)(g_Dbs.db[dbSeq].totalInsertRows / tInMs));
    }

    if (minDelay != UINT64_MAX) {
        fprintf(stderr,
                "insert delay, avg: %10.2fms, max: %10.2fms, min: %10.2fms\n\n",
                (double)avgDelay / 1000.0, (double)maxDelay / 1000.0,
                (double)minDelay / 1000.0);

        if (g_fpOfInsertResult) {
            fprintf(g_fpOfInsertResult,
                    "insert delay, avg:%10.2fms, max: %10.2fms, min: "
                    "%10.2fms\n\n",
                    (double)avgDelay / 1000.0, (double)maxDelay / 1000.0,
                    (double)minDelay / 1000.0);
        }
    }

    free(pids);
    free(infos);
    return 0;
}

int insertTestProcess() {
    int32_t code = -1;
    char *  cmdBuffer = calloc(1, BUFFER_SIZE);
    if (NULL == cmdBuffer) {
        errorPrint("%s", "failed to allocate memory\n");
        goto end_insert_process;
    }

    printfInsertMeta();

    debugPrint("%d result file: %s\n", __LINE__, g_Dbs.resultFile);
    g_fpOfInsertResult = fopen(g_Dbs.resultFile, "a");
    if (NULL == g_fpOfInsertResult) {
        errorPrint("failed to open %s for save result\n", g_Dbs.resultFile);
        goto end_insert_process;
    }

    if (g_fpOfInsertResult) {
        printfInsertMetaToFile(g_fpOfInsertResult);
    }

    prompt();

    if (init_rand_data()) {
        goto end_insert_process;
    }

    for (int i = 0; i < g_Dbs.dbCount; i++) {
        SDataBase *dbInfo = &(g_Dbs.db[i]);
        if (dbInfo->drop) {
            if (createDatabases(cmdBuffer, dbInfo)) {
                goto end_insert_process;
            }
            memset(cmdBuffer, 0, BUFFER_SIZE);
            if (SML_IFACE != dbInfo->iface) {
                if (createStables(dbInfo)) {
                    goto end_insert_process;
                }
            }
            if (prepareSampleData(dbInfo)) {
                goto end_insert_process;
            }
            if (SML_IFACE != dbInfo->iface) {
                if (createChildTables(dbInfo)) {
                    goto end_insert_process;
                }
            }
        } else {
            if (getSuperTableFromServer(dbInfo)) {
                goto end_insert_process;
            }
        }
        if (setUpStables(dbInfo)) {
            goto end_insert_process;
        }

        if (g_Dbs.use_metric) {
            if (g_Dbs.db[i].superTblCount > 0) {
                if (startMultiThreadInsertData(g_Dbs.db[i].iface, i)) {
                    goto end_insert_process;
                }
            }
        } else {
            if (SML_IFACE == g_Dbs.db[i].iface) {
                code = -1;
                errorPrint("%s\n", "Schemaless insertion must include stable");
                goto end_insert_process;
            } else {
                if (startMultiThreadInsertData(g_Dbs.db[i].iface, i)) {
                    goto end_insert_process;
                }
            }
        }
    }
    code = 0;
end_insert_process:
    tmfree(cmdBuffer);
    return code;
}