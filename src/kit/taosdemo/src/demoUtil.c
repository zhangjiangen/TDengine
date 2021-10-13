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

#include "demoUtil.h"

void prompt() {
    if (!g_args.answer_yes) {
        printf("         Press enter key to continue or Ctrl-C to stop\n\n");
        (void)getchar();
    }
}

int taosRandom() { return rand(); }

#ifdef WINDOWS
#define _CRT_RAND_S

#include <windows.h>
#include <winsock2.h>

typedef unsigned __int32 uint32_t;

#pragma comment(lib, "ws2_32.lib")
// Some old MinGW/CYGWIN distributions don't define this:
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif  // ENABLE_VIRTUAL_TERMINAL_PROCESSING

HANDLE g_stdoutHandle;
DWORD  g_consoleMode;

void setupForAnsiEscape(void) {
    DWORD mode = 0;
    g_stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (g_stdoutHandle == INVALID_HANDLE_VALUE) {
        exit(GetLastError());
    }

    if (!GetConsoleMode(g_stdoutHandle, &mode)) {
        exit(GetLastError());
    }

    g_consoleMode = mode;

    // Enable ANSI escape codes
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(g_stdoutHandle, mode)) {
        exit(GetLastError());
    }
}

void resetAfterAnsiEscape(void) {
    // Reset colors
    printf("\x1b[0m");

    // Reset console mode
    if (!SetConsoleMode(g_stdoutHandle, g_consoleMode)) {
        exit(GetLastError());
    }
}

int taosRandom() {
    int number;
    rand_s(&number);

    return number;
}
#else  // Not windows
void setupForAnsiEscape(void) {}

void resetAfterAnsiEscape(void) {
    // Reset colors
    printf("\x1b[0m");
}

#endif  // ifdef Windows

int queryDbExec(TAOS *taos, char *command, QUERY_TYPE type, bool quiet) {
    verbosePrint("%s() LN%d - command: %s\n", __func__, __LINE__, command);

    TAOS_RES *res = taos_query(taos, command);
    int32_t   code = taos_errno(res);

    if (code != 0) {
        if (!quiet) {
            errorPrint2("Failed to execute <%s>, reason: %s\n", command,
                        taos_errstr(res));
        }
        taos_free_result(res);
        // taos_close(taos);
        return -1;
    }

    if (INSERT_TYPE == type) {
        int affectedRows = taos_affected_rows(res);
        taos_free_result(res);
        return affectedRows;
    }

    taos_free_result(res);
    return 0;
}

void postFreeResource() {
    tmfclose(g_fpOfInsertResult);

    for (int i = 0; i < g_Dbs.dbCount; i++) {
        for (uint64_t j = 0; j < g_Dbs.db[i].superTblCount; j++) {
            if (0 != g_Dbs.db[i].superTbls[j].colsOfCreateChildTable) {
                tmfree(g_Dbs.db[i].superTbls[j].colsOfCreateChildTable);
                g_Dbs.db[i].superTbls[j].colsOfCreateChildTable = NULL;
            }
            if (0 != g_Dbs.db[i].superTbls[j].sampleDataBuf) {
                tmfree(g_Dbs.db[i].superTbls[j].sampleDataBuf);
                g_Dbs.db[i].superTbls[j].sampleDataBuf = NULL;
            }

#if STMT_BIND_PARAM_BATCH == 1
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
#endif
            if (0 != g_Dbs.db[i].superTbls[j].tagDataBuf) {
                tmfree(g_Dbs.db[i].superTbls[j].tagDataBuf);
                g_Dbs.db[i].superTbls[j].tagDataBuf = NULL;
            }
            if (0 != g_Dbs.db[i].superTbls[j].childTblName) {
                tmfree(g_Dbs.db[i].superTbls[j].childTblName);
                g_Dbs.db[i].superTbls[j].childTblName = NULL;
            }
        }
    }

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

/*
   Read 10000 lines at most. If more than 10000 lines, continue to read after
   using
   */
int readTagFromCsvFileToMem(SSuperTable *stbInfo) {
    size_t  n = 0;
    ssize_t readLen = 0;
    char *  line = NULL;

    FILE *fp = fopen(stbInfo->tagsFile, "r");
    if (fp == NULL) {
        printf("Failed to open tags file: %s, reason:%s\n", stbInfo->tagsFile,
               strerror(errno));
        return -1;
    }

    if (stbInfo->tagDataBuf) {
        free(stbInfo->tagDataBuf);
        stbInfo->tagDataBuf = NULL;
    }

    int   tagCount = 10000;
    int   count = 0;
    char *tagDataBuf = calloc(1, stbInfo->lenOfTagOfOneRow * tagCount);
    if (tagDataBuf == NULL) {
        printf("Failed to calloc, reason:%s\n", strerror(errno));
        fclose(fp);
        return -1;
    }

    while ((readLen = tgetline(&line, &n, fp)) != -1) {
        if (('\r' == line[readLen - 1]) || ('\n' == line[readLen - 1])) {
            line[--readLen] = 0;
        }

        if (readLen == 0) {
            continue;
        }

        memcpy(tagDataBuf + count * stbInfo->lenOfTagOfOneRow, line, readLen);
        count++;

        if (count >= tagCount - 1) {
            char *tmp = realloc(
                tagDataBuf, (size_t)tagCount * 1.5 * stbInfo->lenOfTagOfOneRow);
            if (tmp != NULL) {
                tagDataBuf = tmp;
                tagCount = (int)(tagCount * 1.5);
                memset(
                    tagDataBuf + count * stbInfo->lenOfTagOfOneRow, 0,
                    (size_t)((tagCount - count) * stbInfo->lenOfTagOfOneRow));
            } else {
                // exit, if allocate more memory failed
                printf("realloc fail for save tag val from %s\n",
                       stbInfo->tagsFile);
                tmfree(tagDataBuf);
                free(line);
                fclose(fp);
                return -1;
            }
        }
    }

    stbInfo->tagDataBuf = tagDataBuf;
    stbInfo->tagSampleCount = count;

    free(line);
    fclose(fp);
    return 0;
}

void ERROR_EXIT(const char *msg) {
    errorPrint("%s", msg);
    exit(-1);
}

int regexMatch(const char *s, const char *reg, int cflags) {
    regex_t regex;
    char    msgbuf[100] = {0};

    /* Compile regular expression */
    if (regcomp(&regex, reg, cflags) != 0) {
        ERROR_EXIT("Fail to compile regex\n");
    }

    /* Execute regular expression */
    int reti = regexec(&regex, s, 0, NULL, 0);
    if (!reti) {
        regfree(&regex);
        return 1;
    } else if (reti == REG_NOMATCH) {
        regfree(&regex);
        return 0;
    } else {
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        regfree(&regex);
        printf("Regex match failed: %s\n", msgbuf);
        exit(EXIT_FAILURE);
    }

    return 0;
}

int isCommentLine(char *line) {
    if (line == NULL) return 1;

    return regexMatch(line, "^\\s*#.*", REG_EXTENDED);
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

bool isStringNumber(char *input) {
    int len = strlen(input);
    if (0 == len) {
        return false;
    }

    for (int i = 0; i < len; i++) {
        if (!isdigit(input[i])) return false;
    }
    return true;
}

void tmfclose(FILE *fp) {
    if (NULL != fp) {
        fclose(fp);
    }
}

void tmfree(char *buf) {
    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }
}

char *formatTimestamp(char *buf, int64_t val, int precision) {
    time_t tt;
    if (precision == TSDB_TIME_PRECISION_MICRO) {
        tt = (time_t)(val / 1000000);
    }
    if (precision == TSDB_TIME_PRECISION_NANO) {
        tt = (time_t)(val / 1000000000);
    } else {
        tt = (time_t)(val / 1000);
    }

    /* comment out as it make testcases like select_with_tags.sim fail.
       but in windows, this may cause the call to localtime crash if tt < 0,
       need to find a better solution.
       if (tt < 0) {
       tt = 0;
       }
       */

#ifdef WINDOWS
    if (tt < 0) tt = 0;
#endif

    struct tm *ptm = localtime(&tt);
    size_t     pos = strftime(buf, 32, "%Y-%m-%d %H:%M:%S", ptm);

    if (precision == TSDB_TIME_PRECISION_MICRO) {
        sprintf(buf + pos, ".%06d", (int)(val % 1000000));
    } else if (precision == TSDB_TIME_PRECISION_NANO) {
        sprintf(buf + pos, ".%09d", (int)(val % 1000000000));
    } else {
        sprintf(buf + pos, ".%03d", (int)(val % 1000));
    }

    return buf;
}

int getDbFromServer(TAOS *taos, SDbInfo **dbInfos) {
    TAOS_RES *res;
    TAOS_ROW  row = NULL;
    int       count = 0;

    res = taos_query(taos, "show databases;");
    int32_t code = taos_errno(res);

    if (code != 0) {
        errorPrint2("failed to run <show databases>, reason: %s\n",
                    taos_errstr(res));
        return -1;
    }

    TAOS_FIELD *fields = taos_fetch_fields(res);

    while ((row = taos_fetch_row(res)) != NULL) {
        // sys database name : 'log'
        if (strncasecmp(row[TSDB_SHOW_DB_NAME_INDEX], "log",
                        fields[TSDB_SHOW_DB_NAME_INDEX].bytes) == 0) {
            continue;
        }

        dbInfos[count] = (SDbInfo *)calloc(1, sizeof(SDbInfo));
        if (dbInfos[count] == NULL) {
            errorPrint2("failed to allocate memory for some dbInfo[%d]\n",
                        count);
            return -1;
        }

        tstrncpy(dbInfos[count]->name, (char *)row[TSDB_SHOW_DB_NAME_INDEX],
                 fields[TSDB_SHOW_DB_NAME_INDEX].bytes);
        formatTimestamp(dbInfos[count]->create_time,
                        *(int64_t *)row[TSDB_SHOW_DB_CREATED_TIME_INDEX],
                        TSDB_TIME_PRECISION_MILLI);
        dbInfos[count]->ntables = *((int64_t *)row[TSDB_SHOW_DB_NTABLES_INDEX]);
        dbInfos[count]->vgroups = *((int32_t *)row[TSDB_SHOW_DB_VGROUPS_INDEX]);
        dbInfos[count]->replica = *((int16_t *)row[TSDB_SHOW_DB_REPLICA_INDEX]);
        dbInfos[count]->quorum = *((int16_t *)row[TSDB_SHOW_DB_QUORUM_INDEX]);
        dbInfos[count]->days = *((int16_t *)row[TSDB_SHOW_DB_DAYS_INDEX]);

        tstrncpy(dbInfos[count]->keeplist, (char *)row[TSDB_SHOW_DB_KEEP_INDEX],
                 fields[TSDB_SHOW_DB_KEEP_INDEX].bytes);
        dbInfos[count]->cache = *((int32_t *)row[TSDB_SHOW_DB_CACHE_INDEX]);
        dbInfos[count]->blocks = *((int32_t *)row[TSDB_SHOW_DB_BLOCKS_INDEX]);
        dbInfos[count]->minrows = *((int32_t *)row[TSDB_SHOW_DB_MINROWS_INDEX]);
        dbInfos[count]->maxrows = *((int32_t *)row[TSDB_SHOW_DB_MAXROWS_INDEX]);
        dbInfos[count]->wallevel =
            *((int8_t *)row[TSDB_SHOW_DB_WALLEVEL_INDEX]);
        dbInfos[count]->fsync = *((int32_t *)row[TSDB_SHOW_DB_FSYNC_INDEX]);
        dbInfos[count]->comp =
            (int8_t)(*((int8_t *)row[TSDB_SHOW_DB_COMP_INDEX]));
        dbInfos[count]->cachelast =
            (int8_t)(*((int8_t *)row[TSDB_SHOW_DB_CACHELAST_INDEX]));

        tstrncpy(dbInfos[count]->precision,
                 (char *)row[TSDB_SHOW_DB_PRECISION_INDEX],
                 fields[TSDB_SHOW_DB_PRECISION_INDEX].bytes);
        dbInfos[count]->update = *((int8_t *)row[TSDB_SHOW_DB_UPDATE_INDEX]);
        tstrncpy(dbInfos[count]->status, (char *)row[TSDB_SHOW_DB_STATUS_INDEX],
                 fields[TSDB_SHOW_DB_STATUS_INDEX].bytes);

        count++;
        if (count > MAX_DATABASE_COUNT) {
            errorPrint("%s() LN%d, The database count overflow than %d\n",
                       __func__, __LINE__, MAX_DATABASE_COUNT);
            break;
        }
    }

    return count;
}

int postProceSql(char *host, struct sockaddr_in *pServAddr, uint16_t port,
                 char *sqlstr, threadInfo *pThreadInfo) {
    char *req_fmt =
        "POST %s HTTP/1.1\r\nHost: %s:%d\r\nAccept: */*\r\nAuthorization: "
        "Basic %s\r\nContent-Length: %d\r\nContent-Type: "
        "application/x-www-form-urlencoded\r\n\r\n%s";

    char *url = "/rest/sql";

    int      bytes, sent, received, req_str_len, resp_len;
    char *   request_buf;
    char     response_buf[RESP_BUF_LEN];
    uint16_t rest_port = port + TSDB_PORT_HTTP;

    int req_buf_len = strlen(sqlstr) + REQ_EXTRA_BUF_LEN;

    request_buf = malloc(req_buf_len);
    if (NULL == request_buf) {
        errorPrint("%s", "cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    char userpass_buf[INPUT_BUF_LEN];
    int  mod_table[] = {0, 2, 1};

    static char base64[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

    snprintf(userpass_buf, INPUT_BUF_LEN, "%s:%s", g_Dbs.user, g_Dbs.password);
    size_t userpass_buf_len = strlen(userpass_buf);
    size_t encoded_len = 4 * ((userpass_buf_len + 2) / 3);

    char base64_buf[INPUT_BUF_LEN];
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
        debugPrint("%s() LN%d, sockfd=%d\n", __func__, __LINE__, sockfd);
        free(request_buf);
        ERROR_EXIT("opening socket");
    }

    int retConn =
        connect(sockfd, (struct sockaddr *)pServAddr, sizeof(struct sockaddr));
    debugPrint("%s() LN%d connect() return %d\n", __func__, __LINE__, retConn);
    if (retConn < 0) {
        free(request_buf);
        ERROR_EXIT("connecting");
    }

    memset(base64_buf, 0, INPUT_BUF_LEN);

    for (int n = 0, m = 0; n < userpass_buf_len;) {
        uint32_t oct_a =
            n < userpass_buf_len ? (unsigned char)userpass_buf[n++] : 0;
        uint32_t oct_b =
            n < userpass_buf_len ? (unsigned char)userpass_buf[n++] : 0;
        uint32_t oct_c =
            n < userpass_buf_len ? (unsigned char)userpass_buf[n++] : 0;
        uint32_t triple = (oct_a << 0x10) + (oct_b << 0x08) + oct_c;

        base64_buf[m++] = base64[(triple >> 3 * 6) & 0x3f];
        base64_buf[m++] = base64[(triple >> 2 * 6) & 0x3f];
        base64_buf[m++] = base64[(triple >> 1 * 6) & 0x3f];
        base64_buf[m++] = base64[(triple >> 0 * 6) & 0x3f];
    }

    for (int l = 0; l < mod_table[userpass_buf_len % 3]; l++)
        base64_buf[encoded_len - 1 - l] = '=';

    debugPrint("%s() LN%d: auth string base64 encoded: %s\n", __func__,
               __LINE__, base64_buf);
    char *auth = base64_buf;

    int r = snprintf(request_buf, req_buf_len, req_fmt, url, host, rest_port,
                     auth, strlen(sqlstr), sqlstr);
    if (r >= req_buf_len) {
        free(request_buf);
        ERROR_EXIT("too long request");
    }
    verbosePrint("%s() LN%d: Request:\n%s\n", __func__, __LINE__, request_buf);

    req_str_len = strlen(request_buf);
    sent = 0;
    do {
#ifdef WINDOWS
        bytes = send(sockfd, request_buf + sent, req_str_len - sent, 0);
#else
        bytes = write(sockfd, request_buf + sent, req_str_len - sent);
#endif
        if (bytes < 0) ERROR_EXIT("writing message to socket");
        if (bytes == 0) break;
        sent += bytes;
    } while (sent < req_str_len);

    memset(response_buf, 0, RESP_BUF_LEN);
    resp_len = sizeof(response_buf) - 1;
    received = 0;
    do {
#ifdef WINDOWS
        bytes = recv(sockfd, response_buf + received, resp_len - received, 0);
#else
        bytes = read(sockfd, response_buf + received, resp_len - received);
#endif
        if (bytes < 0) {
            free(request_buf);
            ERROR_EXIT("reading response from socket");
        }
        if (bytes == 0) break;
        received += bytes;
    } while (received < resp_len);

    if (received == resp_len) {
        free(request_buf);
        ERROR_EXIT("storing complete response from socket");
    }

    response_buf[RESP_BUF_LEN - 1] = '\0';
    printf("Response:\n%s\n", response_buf);

    if (strlen(pThreadInfo->filePath) > 0) {
        appendResultBufToFile(response_buf, pThreadInfo);
    }

    free(request_buf);
#ifdef WINDOWS
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif

    return 0;
}

int convertHostToServAddr(char *host, uint16_t port,
                          struct sockaddr_in *serv_addr) {
    uint16_t        rest_port = port + TSDB_PORT_HTTP;
    struct hostent *server = gethostbyname(host);
    if ((server == NULL) || (server->h_addr == NULL)) {
        errorPrint2("%s", "no such host");
        return -1;
    }

    debugPrint("h_name: %s\nh_addr=%p\nh_addretype: %s\nh_length: %d\n",
               server->h_name, server->h_addr,
               (server->h_addrtype == AF_INET) ? "ipv4" : "ipv6",
               server->h_length);

    memset(serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(rest_port);
#ifdef WINDOWS
    serv_addr->sin_addr.s_addr = inet_addr(host);
#else
    memcpy(&(serv_addr->sin_addr.s_addr), server->h_addr, server->h_length);
#endif
    return 0;
}

void replaceChildTblName(char *inSql, char *outSql, int tblIndex) {
    char sourceString[32] = "xxxx";
    char subTblName[TSDB_TABLE_NAME_LEN];
    sprintf(subTblName, "%s.%s", g_queryInfo.dbName,
            g_queryInfo.superQueryInfo.childTblName +
                tblIndex * TSDB_TABLE_NAME_LEN);

    // printf("inSql: %s\n", inSql);

    char *pos = strstr(inSql, sourceString);
    if (0 == pos) {
        return;
    }

    tstrncpy(outSql, inSql, pos - inSql + 1);
    // printf("1: %s\n", outSql);
    strncat(outSql, subTblName, BUFFER_SIZE - 1);
    // printf("2: %s\n", outSql);
    strncat(outSql, pos + strlen(sourceString), BUFFER_SIZE - 1);
    // printf("3: %s\n", outSql);
}

void fetchResult(TAOS_RES *res, threadInfo *pThreadInfo) {
    TAOS_ROW    row = NULL;
    int         num_rows = 0;
    int         num_fields = taos_field_count(res);
    TAOS_FIELD *fields = taos_fetch_fields(res);

    char *databuf = (char *)calloc(1, 100 * 1024 * 1024);
    if (databuf == NULL) {
        errorPrint2(
            "%s() LN%d, failed to malloc, warning: save result to file "
            "slowly!\n",
            __func__, __LINE__);
        return;
    }

    int64_t totalLen = 0;

    // fetch the records row by row
    while ((row = taos_fetch_row(res))) {
        if (totalLen >= (100 * 1024 * 1024 - HEAD_BUFF_LEN * 2)) {
            if (strlen(pThreadInfo->filePath) > 0)
                appendResultBufToFile(databuf, pThreadInfo);
            totalLen = 0;
            memset(databuf, 0, 100 * 1024 * 1024);
        }
        num_rows++;
        char temp[HEAD_BUFF_LEN] = {0};
        int  len = taos_print_row(temp, row, fields, num_fields);
        len += sprintf(temp + len, "\n");
        // printf("query result:%s\n", temp);
        memcpy(databuf + totalLen, temp, len);
        totalLen += len;
        verbosePrint("%s() LN%d, totalLen: %" PRId64 "\n", __func__, __LINE__,
                     totalLen);
    }

    verbosePrint("%s() LN%d, databuf=%s resultFile=%s\n", __func__, __LINE__,
                 databuf, pThreadInfo->filePath);
    if (strlen(pThreadInfo->filePath) > 0) {
        appendResultBufToFile(databuf, pThreadInfo);
    }
    free(databuf);
}

int getChildNameOfSuperTableWithLimitAndOffset(TAOS *taos, char *dbName,
                                               char *   stbName,
                                               char **  childTblNameOfSuperTbl,
                                               int64_t *childTblCountOfSuperTbl,
                                               int64_t limit, uint64_t offset) {
    char command[1024] = "\0";
    char limitBuf[100] = "\0";

    TAOS_RES *res;
    TAOS_ROW  row = NULL;

    char *childTblName = *childTblNameOfSuperTbl;

    snprintf(limitBuf, 100, " limit %" PRId64 " offset %" PRIu64 "", limit,
             offset);

    // get all child table name use cmd: select tbname from superTblName;
    snprintf(command, 1024, "select tbname from %s.%s %s", dbName, stbName,
             limitBuf);

    res = taos_query(taos, command);
    int32_t code = taos_errno(res);
    if (code != 0) {
        taos_free_result(res);
        taos_close(taos);
        errorPrint2("%s() LN%d, failed to run command %s\n", __func__, __LINE__,
                    command);
        exit(EXIT_FAILURE);
    }

    int64_t childTblCount = (limit < 0) ? 10000 : limit;
    int64_t count = 0;
    if (childTblName == NULL) {
        childTblName = (char *)calloc(1, childTblCount * TSDB_TABLE_NAME_LEN);
        if (NULL == childTblName) {
            taos_free_result(res);
            taos_close(taos);
            errorPrint2("%s() LN%d, failed to allocate memory!\n", __func__,
                        __LINE__);
            exit(EXIT_FAILURE);
        }
    }

    char *pTblName = childTblName;
    while ((row = taos_fetch_row(res)) != NULL) {
        int32_t *len = taos_fetch_lengths(res);

        if (0 == strlen((char *)row[0])) {
            errorPrint2("%s() LN%d, No.%" PRId64 " table return empty name\n",
                        __func__, __LINE__, count);
            exit(EXIT_FAILURE);
        }

        tstrncpy(pTblName, (char *)row[0], len[0] + 1);
        // printf("==== sub table name: %s\n", pTblName);
        count++;
        if (count >= childTblCount - 1) {
            char *tmp =
                realloc(childTblName,
                        (size_t)childTblCount * 1.5 * TSDB_TABLE_NAME_LEN + 1);
            if (tmp != NULL) {
                childTblName = tmp;
                childTblCount = (int)(childTblCount * 1.5);
                memset(childTblName + count * TSDB_TABLE_NAME_LEN, 0,
                       (size_t)((childTblCount - count) * TSDB_TABLE_NAME_LEN));
            } else {
                // exit, if allocate more memory failed
                tmfree(childTblName);
                taos_free_result(res);
                taos_close(taos);
                errorPrint2(
                    "%s() LN%d, realloc fail for save child table name of "
                    "%s.%s\n",
                    __func__, __LINE__, dbName, stbName);
                exit(EXIT_FAILURE);
            }
        }
        pTblName = childTblName + count * TSDB_TABLE_NAME_LEN;
    }

    *childTblCountOfSuperTbl = count;
    *childTblNameOfSuperTbl = childTblName;

    taos_free_result(res);
    return 0;
}

int getAllChildNameOfSuperTable(TAOS *taos, char *dbName, char *stbName,
                                char **  childTblNameOfSuperTbl,
                                int64_t *childTblCountOfSuperTbl) {
    return getChildNameOfSuperTableWithLimitAndOffset(
        taos, dbName, stbName, childTblNameOfSuperTbl, childTblCountOfSuperTbl,
        -1, 0);
}

static void getTableName(char *pTblName, threadInfo *pThreadInfo,
                         uint64_t tableSeq) {
    SSuperTable *stbInfo = pThreadInfo->stbInfo;
    if (stbInfo) {
        if (AUTO_CREATE_SUBTBL != stbInfo->autoCreateTable) {
            if (stbInfo->childTblLimit > 0) {
                snprintf(pTblName, TSDB_TABLE_NAME_LEN, "%s",
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
                    pTblName, TSDB_TABLE_NAME_LEN, "%s",
                    stbInfo->childTblName + tableSeq * TSDB_TABLE_NAME_LEN);
            }
        } else {
            snprintf(pTblName, TSDB_TABLE_NAME_LEN, "%s%" PRIu64 "",
                     stbInfo->childTblPrefix, tableSeq);
        }
    } else {
        snprintf(pTblName, TSDB_TABLE_NAME_LEN, "%s%" PRIu64 "",
                 g_args.tb_prefix, tableSeq);
    }
}
