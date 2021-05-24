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

#ifndef _TD_TSDB_RECOVER_H_
#define _TD_TSDB_RECOVER_H_

/**
 * check and recover data
 */
int tsdbRecoverDataMain(STsdbRepo *pRepo);

void tsdbClearBakFiles();

/**
 * Print buffer as binary.
 */
#define TSDB_PRINT_BUF_STACK_LEN_MAX 128
FORCE_INLINE void tsdbPrintBinary(int32_t vgId, const char* fileName, void* bufStart, int len, int offset) {
  char printBuf[2 * TSDB_PRINT_BUF_STACK_LEN_MAX + 1] = "\0";
  int  lenPrint = len;
  int  bufIndex = 0;
  int  printIndex = 0;
  do {
    if (lenPrint >= TSDB_PRINT_BUF_STACK_LEN_MAX) {
      for (int i = 0; i < TSDB_PRINT_BUF_STACK_LEN_MAX; ++i) {
        snprintf(printBuf + i * 2, 2 * TSDB_PRINT_BUF_STACK_LEN_MAX, "%x", *((char*)bufStart + bufIndex++));
      }
      tsdbDebug("vgId:%d file %s, offset %d, bin[%d][%d]: %s", vgId, fileName, offset, printIndex++,
                TSDB_PRINT_BUF_STACK_LEN_MAX, printBuf);
    } else {
      for (int i = 0; i < lenPrint; ++i) {
        snprintf(printBuf + 2 * i, 2 * TSDB_PRINT_BUF_STACK_LEN_MAX, "%x", *((char*)bufStart + bufIndex++));
      }
      tsdbDebug("vgId:%d file %s, offset %d, bin[%d][%d]: %s", vgId, fileName, offset, printIndex++, lenPrint,
                printBuf);
      break;
    }
    memset(printBuf, 0, 2 * TSDB_PRINT_BUF_STACK_LEN_MAX + 1);
  } while ((lenPrint -= TSDB_PRINT_BUF_STACK_LEN_MAX) > 0);
}

#endif /* _TD_TSDB_RECOVER_H_ */