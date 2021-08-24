/*
 * Copyright (c) 2019 TAOS Data, Inc. <cli@taosdata.com>
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

#define _DEFAULT_SOURCE
#include "os.h"
#include "tfile.h"
#include "cache.h"
#include "hash.h"
#include "mnodeSdb.h"
#include "mnodeInt.h"
#include "twal.h"
#include "tglobal.h"
#include "taoserror.h"
#include "mnodeSdbTable.h"
#include "mnodeWalIndex.h"

struct walIndexFileInfo;

typedef struct walIndexItem {
  struct walIndexItem* prev;
  struct walIndexItem* next;

  struct walIndexFileInfo *pFileInfo;

  walIndex index;
} walIndexItem;

typedef struct walIndexFileInfo {
  char name[TSDB_FILENAME_LEN];

  uint64_t version;
  int64_t offset;
  int64_t total;

  struct walIndexFileInfo* next;

  walIndexItem* tail[SDB_TABLE_MAX];
  walIndexItem* head[SDB_TABLE_MAX];

  int64_t tableSize[SDB_TABLE_MAX];
} walIndexFileInfo;

static SHashObj* tsIndexItemTable[SDB_TABLE_MAX];
static walIndexFileInfo* tsWalFileInfo;
static walIndexFileInfo* tsCurrentWalFileInfo;

// index data type

typedef enum walIndexContentType {
  WAL_INDEX_HEADER        = 0,
  WAL_INDEX_TABLE_HEADER  = 1,
  WAL_INDEX_ITEM          = 2,
} walIndexContentType;

#pragma  pack(1)
typedef struct walIndexHeaderMeta {
  walIndexContentType type;
  int64_t totalSize;
  uint64_t version;
  int64_t offset;
  int32_t walNameSize;
} walIndexHeaderMeta;

#pragma  pack(1)
typedef struct walIndexHeader {
  walIndexHeaderMeta meta;

  char walName[];
} walIndexHeader;

#pragma  pack(1)
typedef struct walIndexTableHeader {
  walIndexContentType type;
  ESdbTable tableId;
  int64_t tableSize;
} walIndexTableHeader;

static int32_t buildIndex(void *wparam, void *hparam, int32_t qtype, void *tparam, void* head) {
  //SSdbRow *pRow = wparam;
  SWalHead *pHead = hparam;
  SWalHeadInfo* pHeadInfo = (SWalHeadInfo*)head;
  int32_t tableId = pHead->msgType / 10;
  int32_t action = pHead->msgType % 10;
  walIndexFileInfo *pFileInfo = tsCurrentWalFileInfo;
  ESdbKey keyType = sdbGetKeyTypeByTableId(tableId);

  SHashObj* pTable = tsIndexItemTable[tableId];

  int32_t keySize;
  void* key = sdbTableGetKeyAndSize(keyType, pHead->cont, &keySize, false);

  walIndexItem* pItem = calloc(1, sizeof(walIndexItem) + keySize);
  *pItem = (walIndexItem) {
    .index = (walIndex) {
      .keyLen = keySize,
      .offset = pHeadInfo->offset,
      .size   = pHeadInfo->len,
    },
    .next = NULL,
    .prev = NULL,
    .pFileInfo = pFileInfo,
  };
  memcpy(pItem->index.key, key, keySize);  

  // update or delete action will delete old index item
  if (action == SDB_ACTION_UPDATE || action == SDB_ACTION_DELETE) {
    walIndexItem** ppItem = (walIndexItem**)taosHashGet(pTable, key, keySize);
    assert(ppItem != NULL);

    walIndexFileInfo *pOldFileInfo = (*ppItem)->pFileInfo;
    walIndexItem* prev = (*ppItem)->prev;
    walIndexItem* next = (*ppItem)->next;

    if (pOldFileInfo->tail[tableId] == *ppItem) {
      pOldFileInfo->tail[tableId] = prev;
    }
    if (pOldFileInfo->head[tableId] == *ppItem) {
      pOldFileInfo->head[tableId] = next;
    }
    if (prev) prev->next = next;
    if (next) next->prev = prev;    

    pOldFileInfo->total -= sizeof(walIndex) + (*ppItem)->index.keyLen;
    pOldFileInfo->tableSize[tableId] -= sizeof(walIndex) + (*ppItem)->index.keyLen;
    assert(pOldFileInfo->total >= 0);
    free(*ppItem);
  }

  // insert or update action will save index item
  if (action == SDB_ACTION_INSERT || action == SDB_ACTION_UPDATE) {
    if (pFileInfo->tail[tableId] == NULL) {      
      pFileInfo->tail[tableId] = pItem;      
    } else {
      pItem->prev = tsWalFileInfo[0].tail[tableId];
      pFileInfo->tail[tableId]->next = pItem;
      pFileInfo->tail[tableId] = pItem;
    }
    if (pFileInfo->head[tableId] == NULL) {
      pFileInfo->head[tableId] = pItem;   
    }

    taosHashPut(pTable, key, keySize, &pItem, sizeof(walIndexItem**));
    pFileInfo->total += sizeof(walIndex) + keySize;
    pFileInfo->tableSize[tableId] += sizeof(walIndex) + keySize;
  }

  if (pHeadInfo->offset + pHeadInfo->len > pFileInfo->offset) {
    pFileInfo->offset = pHeadInfo->offset + pHeadInfo->len;
  }

  if (pHead->version > pFileInfo->version) {
    pFileInfo->version = pHead->version;
  }

  return 0;
}

static walIndexFileInfo* createWalIndexFileInfo(const char* walName) {
  walIndexFileInfo* pIndexFileInfo = calloc(1, sizeof(walIndexFileInfo));
  strcpy(pIndexFileInfo->name, walName);

  return pIndexFileInfo;
}

static void beginBuildWalIndex(const char* walName) {
  walIndexFileInfo* pIndexFileInfo = createWalIndexFileInfo(walName);
  if (!pIndexFileInfo) {
    return;
  }

  if (tsWalFileInfo == NULL) {
    tsWalFileInfo = pIndexFileInfo;
  } else {
    assert(tsCurrentWalFileInfo != NULL);
    tsCurrentWalFileInfo->next = pIndexFileInfo;
  }

  tsCurrentWalFileInfo = pIndexFileInfo;

  sdbInfo("file:%s, open for build index", walName);
}

void mnodeSdbBuildWalIndex(void* handle) {
  tsWalFileInfo = NULL;

  int i = 0;
  for (i = 0; i < SDB_TABLE_MAX; ++i) {
    ESdbKey keyType = sdbGetKeyTypeByTableId(i);
    _hash_fn_t hashFp = taosGetDefaultHashFunction(TSDB_DATA_TYPE_INT);
    if (keyType == SDB_KEY_STRING || keyType == SDB_KEY_VAR_STRING) {
      hashFp = taosGetDefaultHashFunction(TSDB_DATA_TYPE_BINARY);
    }
    tsIndexItemTable[i] = taosHashInit(1024, hashFp, true, HASH_ENTRY_LOCK);
  }

  char* save = NULL;
  char    name[TSDB_FILENAME_LEN] = {0};
  sprintf(name, "%s/wal/index", tsMnodeDir);
  sdbInfo("begin save index to file:%s success", name);
  remove(name);
  int64_t tfd = tfOpenM(name, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
  if (!tfValid(tfd)) {
    sdbError("file:%s, failed to open for build index since %s", name, strerror(errno));
    goto _err;
  }

  int32_t code = walRestore(handle, NULL, beginBuildWalIndex, buildIndex);
  if (code != 0) {
    sdbError("walRestore fail, buildIndex return");
    goto _err;
  }

  walIndexFileInfo* pFileInfo = tsWalFileInfo;
  while (pFileInfo) {
    assert(sizeof(walIndexHeaderMeta) == sizeof(walIndexContentType) + sizeof(int64_t)*2 + sizeof(uint64_t) + sizeof(int32_t) );
    int64_t headerSize = sizeof(walIndexHeaderMeta) + strlen(pFileInfo->name);

    int64_t indexTotal = pFileInfo->total;
    int64_t tableHeaderTotal = SDB_TABLE_MAX*(sizeof(walIndexTableHeader));
    int64_t total = headerSize + indexTotal + tableHeaderTotal;

    char *buffer = calloc(1, total);
    if (buffer == NULL) {
      goto _err;
    }
    save = buffer;

    // save header
    walIndexHeaderMeta *pIndexHeader = (walIndexHeaderMeta*)buffer;
    *pIndexHeader = (walIndexHeaderMeta) {
      .type         = WAL_INDEX_HEADER,
      .totalSize    = indexTotal + tableHeaderTotal,
      .version      = pFileInfo->version,
      .offset       = pFileInfo->offset,
      .walNameSize  = strlen(pFileInfo->name),
    };
    buffer += sizeof(walIndexHeaderMeta);
    memcpy(buffer, pFileInfo->name, strlen(pFileInfo->name)); buffer += strlen(pFileInfo->name);

    // build table info
    for (i = 0; i < SDB_TABLE_MAX; ++i) {
      // build table header
      walIndexTableHeader *pTableHeader = (walIndexTableHeader*)buffer;
      *pTableHeader = (walIndexTableHeader) {
        .type = WAL_INDEX_TABLE_HEADER,
        .tableId = i,
        .tableSize = pFileInfo->tableSize[i],
      };
      buffer += sizeof(walIndexTableHeader);

      char* pTableBegin = buffer;

      // build index item array
      walIndexItem* pItem = pFileInfo->head[i];
      while (pItem) {
        memcpy(buffer, &pItem->index, sizeof(walIndex) + pItem->index.keyLen);
        buffer += sizeof(walIndex) + pItem->index.keyLen;
        pItem = pItem->next;
      }

      assert((buffer - pTableBegin) == pFileInfo->tableSize[i]);  
    }
    assert(buffer - save == total);

    tfWrite(tfd, save, total);
    tfFsync(tfd);
    tfree(save);

    pFileInfo = pFileInfo->next;
  }

  tfClose(tfd);

_err:
  tfree(save);

  pFileInfo = tsWalFileInfo;
  while (pFileInfo) {    
    for (i = 0; i < SDB_TABLE_MAX; ++i) {
      walIndexItem* pItem = pFileInfo->head[i];
      while (pItem) {
        walIndexItem* pNext = pItem->next;
        free(pItem);
        pItem = pNext;
      }              
    }

    walIndexFileInfo* pNextFileInfo = pFileInfo->next;
    free(pFileInfo);
    pFileInfo = pNextFileInfo;         
  }

  for (i = 0; i < SDB_TABLE_MAX; ++i) {      
    taosHashCleanup(tsIndexItemTable[i]);
  }

  tsWalFileInfo = NULL;

  sdbInfo("save index to file:%s success", name);
}

int32_t sdbRestoreFromIndex(twalh handle, FWalIndexReader fpReader, FWalWrite writeFp) {
  char    name[TSDB_FILENAME_LEN] = {0};
  sprintf(name, "%s/wal/index", tsMnodeDir);
  int code = TSDB_CODE_SUCCESS;

  int64_t tfd = tfOpen(name, O_RDONLY);
  if (!tfValid(tfd)) {
    sdbError("file:%s, failed to open index since %s", name, strerror(errno));
    goto _err;
  }

  int64_t size = tfLseek(tfd, 0, SEEK_END);
  if (size < 0) {
    code = TAOS_SYSTEM_ERROR(errno);
    goto _err;
  }
  char *buffer = calloc(1, size);
  if (buffer == NULL) {
    code = TSDB_CODE_COM_OUT_OF_MEMORY;
    goto _err;
  }
  tfLseek(tfd, 0, SEEK_SET);
  if (tfRead(tfd, buffer, size) != size) {
    sdbError("file:%s, failed to read since %s", name, strerror(errno));
    code = TAOS_SYSTEM_ERROR(errno);
    goto _err;
  }

  char* save = buffer;
  
  while (size > 0) {
    // read header
    walIndexHeader *pHeader = (walIndexHeader*)buffer;
    assert(pHeader->meta.type == WAL_INDEX_HEADER);
    buffer += sizeof(walIndexHeaderMeta) + pHeader->meta.walNameSize;
    memset(name, 0, sizeof(name));
    memcpy(name, pHeader->walName, pHeader->meta.walNameSize);
    size -= sizeof(walIndexHeaderMeta) + pHeader->meta.walNameSize;

    int64_t total = pHeader->meta.totalSize;
    uint64_t walVersion = pHeader->meta.version;

    int64_t fd = tfOpen(name, O_RDONLY);
    if (!tfValid(fd)) {
      sdbError("file:%s, failed to open for build index since %s", name, strerror(errno));
      code = TAOS_SYSTEM_ERROR(errno);
      goto _err;
    }

    // read table header
    while (total > 0) {
      walIndexTableHeader *pTableHeader = (walIndexTableHeader*)buffer;
      assert(pTableHeader->type == WAL_INDEX_TABLE_HEADER);

      ESdbTable tableId = pTableHeader->tableId;
      int64_t tableSize = pTableHeader->tableSize;
      buffer += sizeof(walIndexTableHeader);

      total -= sizeof(walIndexTableHeader);

      int64_t readBytes = 0;
      while (readBytes < tableSize) {
        walIndex* pIndex = (walIndex*)buffer;
        buffer += sizeof(walIndex) + pIndex->keyLen;
        readBytes += sizeof(walIndex) + pIndex->keyLen;

        fpReader(fd, name, tableId, walVersion, pIndex);
      }
      assert(readBytes == tableSize);

      total -= readBytes;
    }
    assert(total == 0);

    code = walRestoreFrom(handle, NULL, name, pHeader->meta.offset, writeFp);
    if (code != TSDB_CODE_SUCCESS) {
      break;
    }

    size -= pHeader->meta.totalSize;
  }

_err:
  tfClose(tfd);
  tfree(save);
  return code;
}
