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
#include "mnodeSdbTable.h"
#include "mnodeWalIndex.h"

struct mnodeSdbHashTable;
typedef struct mnodeSdbHashTable mnodeSdbHashTable;

struct mnodeSdbCacheTable;
typedef struct mnodeSdbCacheTable mnodeSdbCacheTable;

static int   calcHashPower(mnodeSdbTableOption options);

// hash table functions
static mnodeSdbHashTable* hashTableInit(mnodeSdbTable* table, mnodeSdbTableOption options);
static int  hashTableGet(mnodeSdbTable *pTable, const void *key, size_t keyLen, void** pRet);
static void hashTablePut(mnodeSdbTable *pTable, SSdbRow* pRow);
static void hashTableRemove(mnodeSdbTable *pTable, const void *key, size_t keyLen);
static void hashTableClear(mnodeSdbTable *pTable);
static void* hashTableIterate(mnodeSdbTable *pTable, void *p);
static void hashTableCancelIterate(mnodeSdbTable *pTable, void *p);
static int  hashTableIterValue(mnodeSdbTable *pTable,void *p, void** pRet);
static void hashTableFreeValue(mnodeSdbTable *pTable, void *p);

// lru cache functions
static mnodeSdbCacheTable* cacheInit(mnodeSdbTable* table, mnodeSdbTableOption options);
static void sdbCacheSyncWal(mnodeSdbTable *pTable, bool restore, SWalHead*, SSdbRow*, SWalHeadInfo*);
static int  sdbCacheGet(mnodeSdbTable *pTable, const void *key, size_t keyLen, void** pRet);
static void sdbCacheUnlockData(void* p);
static void sdbCachePut(mnodeSdbTable *pTable, SSdbRow* pRow);
static void sdbCacheRemove(mnodeSdbTable *pTable, const void *key, size_t keyLen);
static void sdbCacheClear(mnodeSdbTable *pTable);
static void* sdbCacheIterate(mnodeSdbTable *pTable, void *p);
static int  sdbCacheIterValue(mnodeSdbTable *pTable,void *p, void** pRet);
static void sdbCacheCancelIterate(mnodeSdbTable *pTable, void *p);
static void sdbCacheFreeValue(mnodeSdbTable *pTable, void *p);
static void sdbCacheReadIndex(mnodeSdbTable *pTable, const char* walName, void*);

static int loadCacheDataFromWal(void*, const void* key, uint8_t nkey, char** value, size_t *len, uint64_t *pExpire);

static int delCacheData(void*, const void* key, uint8_t nkey);

typedef int (*sdb_table_get_func_t)(mnodeSdbTable *pTable, const void *key, size_t keyLen, void** pRet);
typedef void (*sdb_table_unlock_func_t)(void*);
typedef void (*sdb_table_put_func_t)(mnodeSdbTable *pTable, SSdbRow* pRow);
typedef void (*sdb_table_del_func_t)(mnodeSdbTable *pTable, const void *key, size_t keyLen);
typedef void (*sdb_table_clear_func_t)(mnodeSdbTable *pTable);
typedef void* (*sdb_table_iter_func_t)(mnodeSdbTable *pTable, void *p);
typedef int  (*sdb_table_iter_val_func_t)(mnodeSdbTable *pTable,void *p, void**);
typedef void (*sdb_table_cancel_iter_func_t)(mnodeSdbTable *pTable, void *p);
typedef void (*sdb_table_sync_wal_func_t)(mnodeSdbTable *pTable, bool, SWalHead*, SSdbRow*,SWalHeadInfo*);
typedef void (*sdb_table_free_val_func_t)(mnodeSdbTable *pTable, void *p);
typedef void (*sdb_read_index_func_t)(mnodeSdbTable *pTable, const char* walName, void*);

struct mnodeSdbHashTable {
  SHashObj*    pTable;
  pthread_mutex_t mutex;
};

typedef struct walRecord {
  int8_t idx;

  walIndex index;
} walRecord;

struct mnodeSdbCacheTable {
  cacheTable* pTable;
  SHashObj*    pWalTable;
  pthread_mutex_t mutex;
};

typedef struct walFileInfo {
  int16_t idx;
  char name[200];
  int64_t tfd;
} walFileInfo;

struct mnodeSdbTable {
  void* iHandle;

  cache_t* pCache;    /* only used in lru cache */
  int16_t walSize;
  walFileInfo *walInfo;

  mnodeSdbTableOption options;

  sdb_table_get_func_t getFp;
  sdb_table_put_func_t putFp;
  sdb_table_unlock_func_t unlockFp;
  sdb_table_del_func_t delFp;
  sdb_table_clear_func_t clearFp;
  sdb_table_iter_func_t iterFp;
  sdb_table_iter_val_func_t iterValFp;
  sdb_table_cancel_iter_func_t cancelIterFp;
  sdb_table_sync_wal_func_t syncFp;
  sdb_read_index_func_t readIndexFp;
  sdb_table_free_val_func_t freeValFp;
};

mnodeSdbTable* mnodeSdbTableInit(mnodeSdbTableOption options) {
  mnodeSdbTable* pTable = calloc(1, sizeof(mnodeSdbTable));

  if (options.tableType == SDB_TABLE_HASH_TABLE) {
    pTable->iHandle = hashTableInit(pTable, options);
  } else {
    pTable->iHandle = cacheInit(pTable, options);
  }
  pTable->options = options;

  return pTable;
}

static void mnodeSdbTableDestroy(mnodeSdbTable* pTable) {
  free(pTable);
}

int mnodeSdbTableGet(mnodeSdbTable *pTable, const void *key, size_t keyLen, void** pRet) {
  return pTable->getFp(pTable, key, keyLen, pRet);
}

void mnodeSdbUnlockData(mnodeSdbTable *pTable, void* p) {
  if (pTable->unlockFp) pTable->unlockFp(p);
}

void mnodeSdbTablePut(mnodeSdbTable *pTable, SSdbRow* pRow) {
  pTable->putFp(pTable, pRow);
}

void mnodeSdbTableSyncWal(mnodeSdbTable *pTable, bool putToCache, void *wparam, void *hparam, void* tparam) {
  SWalHead *pHead = wparam;
  SSdbRow *pRow = hparam;
  if (pTable->syncFp) {
    pTable->syncFp(pTable, putToCache, pHead, pRow, tparam);
  }
}

void mnodeSdbTableReadIndex(mnodeSdbTable *pTable, const char* walName, void* p) {
  if (pTable->readIndexFp) {
    pTable->readIndexFp(pTable, walName, p);
  }
}

void mnodeSdbTableRemove(mnodeSdbTable *pTable, const SSdbRow* pRow) {
  int32_t keySize;
  void* key = sdbTableGetKeyAndSize(pTable->options.keyType, pRow->pObj, &keySize, true);
  pTable->delFp(pTable, key, keySize);
}

void mnodeSdbTableClear(mnodeSdbTable *pTable) {
  pTable->clearFp(pTable);
}

void mnodeSdbTableFreeValue(mnodeSdbTable *pTable, void *p) {
  pTable->freeValFp(pTable, p);
}

void *mnodeSdbTableIterate(mnodeSdbTable *pTable, void *p) {
  return pTable->iterFp(pTable, p);
}

int  mnodeSdbTableIterValue(mnodeSdbTable *pTable,void *p, void** pRet) {
  return pTable->iterValFp(pTable, p, pRet);
}

void mnodeSdbTableCancelIterate(mnodeSdbTable *pTable, void *p) {
  pTable->cancelIterFp(pTable, p);
}

// lru cache functions
static mnodeSdbCacheTable* cacheInit(mnodeSdbTable* pTable, mnodeSdbTableOption options) {
  if (pTable->pCache == NULL) {
    cacheOption opt = (cacheOption) {
      .factor = 1.2,
      .hotPercent = 30,
      .warmPercent = 30,
      .limit = (size_t)(1024 * 1024) * options.cacheLimitMBSize,
    };
    pTable->pCache = cacheCreate(&opt);
    assert(pTable->pCache);
  }

  mnodeSdbCacheTable* pCache = calloc(1, sizeof(mnodeSdbCacheTable));
  pthread_mutex_init(&pCache->mutex, NULL);
  
  int hashPower = calcHashPower(options);
  cacheTableOption tableOpt = (cacheTableOption) {
    .initHashPower = hashPower,
    .userData = pTable,
    .loadFp = loadCacheDataFromWal,
    .delFp  = delCacheData,
    .freeFp = options.freeFp,
    .keyType = options.keyType,
  };

  _hash_fn_t hashFp = taosGetDefaultHashFunction(TSDB_DATA_TYPE_INT);
  if (options.keyType == SDB_KEY_STRING || options.keyType == SDB_KEY_VAR_STRING) {
    hashFp = taosGetDefaultHashFunction(TSDB_DATA_TYPE_BINARY);
  }

  pCache->pWalTable = taosHashInit(options.hashSessions, hashFp, true, HASH_ENTRY_LOCK);
  pCache->pTable = cacheCreateTable(pTable->pCache, &tableOpt);
  
  pTable->walSize = 1;
  pTable->walInfo = calloc(pTable->walSize, sizeof(walFileInfo));

  pTable->getFp  = sdbCacheGet;
  pTable->putFp  = sdbCachePut;
  pTable->unlockFp = sdbCacheUnlockData;
  pTable->syncFp = sdbCacheSyncWal;
  pTable->readIndexFp = sdbCacheReadIndex;
  pTable->delFp  = sdbCacheRemove;
  pTable->clearFp = sdbCacheClear;
  pTable->iterFp = sdbCacheIterate;
  pTable->iterValFp = sdbCacheIterValue;
  pTable->cancelIterFp = sdbCacheCancelIterate;
  pTable->freeValFp = sdbCacheFreeValue;

  return pCache;
}

static int sdbCacheGet(mnodeSdbTable *pTable, const void *key, size_t keyLen, void** pRet) {
  assert(pRet != NULL);
  int nBytes;
  mnodeSdbCacheTable* pCache = pTable->iHandle;

  *pRet = cacheGet(pCache->pTable, key, keyLen, &nBytes);
  return *pRet != NULL ? 0 : -1;
}

static void sdbCacheUnlockData(void* p) {
  cacheItemUnlock(p);
}

static void sdbCachePut(mnodeSdbTable *pTable, SSdbRow* pRow) {
  // put data in cache in sdbCacheSyncWal
}

static void sdbCacheFreeValue(mnodeSdbTable *pTable, void *pObj) {
  mnodeSdbCacheTable* pCache = pTable->iHandle;
  cacheFreeItem(pCache->pTable, cacheItemByData(pObj));
}

static void sdbCacheRemove(mnodeSdbTable *pTable, const void *key, size_t keyLen) {
  mnodeSdbCacheTable* pCache = pTable->iHandle;

  cacheRemove(pCache->pTable, key, keyLen);
}

static void sdbCacheClear(mnodeSdbTable *pTable) {
  mnodeSdbCacheTable* pCache = pTable->iHandle;
  pthread_mutex_destroy(&pCache->mutex);
  cacheDestroyTable(pCache->pTable);

  // free pWalTable
  void *pIter = taosHashIterate(pCache->pWalTable, NULL);
  while (pIter) {
    walRecord **ppRecord = (walRecord**)pIter;
    pIter = taosHashIterate(pCache->pWalTable, pIter);
    if (ppRecord)
      free(*ppRecord);
  }
  taosHashCancelIterate(pCache->pWalTable, pIter);

  taosHashCleanup(pCache->pWalTable);

  free(pCache);

  if (pTable->pCache) {
    cacheDestroy(pTable->pCache);
    pTable->pCache = NULL;
  }

  free(pTable->walInfo);

  mnodeSdbTableDestroy(pTable);  
}

static void* sdbCacheIterate(mnodeSdbTable *pTable, void *p) {
  mnodeSdbCacheTable* pCache = pTable->iHandle;
  return taosHashIterate(pCache->pWalTable, p);
}

static int  sdbCacheIterValue(mnodeSdbTable *pTable,void *pIter, void** pRet) {
  int nBytes;
  mnodeSdbCacheTable* pCache = pTable->iHandle;
  walRecord** ppRecord = (walRecord**)pIter;
  *pRet = NULL;
  if (ppRecord == NULL) {
    return -1;
  }

  *pRet = cacheGet(pCache->pTable, (*ppRecord)->index.data, (*ppRecord)->index.keyLen, &nBytes);
  return *pRet != NULL ? 0 : -1;
}

static void sdbCacheCancelIterate(mnodeSdbTable *pTable, void* pIter) {
  mnodeSdbCacheTable* pCache = pTable->iHandle;
  taosHashCancelIterate(pCache->pWalTable, pIter);
}

static int16_t initCacheWalInfo(mnodeSdbTable *pTable, const char* walName) {
  int i = 0;

  for (i = 0; i < pTable->walSize; i++) {
    if (strcmp(pTable->walInfo[i].name, walName) == 0) {
      return i;
    }
    if (strlen(pTable->walInfo[i].name) == 0) {
      strcpy(pTable->walInfo[i].name, walName);
      pTable->walInfo[i].idx = i;
      pTable->walInfo[i].tfd = tfOpen(walName, O_RDONLY);
      if (!tfValid(pTable->walInfo[i].tfd)) {
        sdbError("open wal file %s for read error since %s", walName, strerror(errno));
        return -1;
      }

      return i;
    }
  }

  int16_t oldWal = pTable->walSize;
  pTable->walSize *= 2;
  void* p = realloc(pTable->walInfo, pTable->walSize * sizeof(walFileInfo));
  if (!p) {
    sdbError("realloc for walFileInfo fail");
    return -1;
  }
  pTable->walInfo = p;
  strcpy(pTable->walInfo[oldWal].name, walName);
  pTable->walInfo[oldWal].idx = oldWal;
  pTable->walInfo[oldWal].tfd = tfOpen(walName, O_RDONLY);

  return oldWal;
}

SWalHead* readWal(mnodeSdbTable *pTable, int idx, int64_t offset, int32_t size) {
  walFileInfo* pFileInfo = &(pTable->walInfo[idx]);
  int64_t tfd = pFileInfo->tfd;

  if (!tfValid(tfd)) {
    return NULL;
  }

  if (tfLseek(tfd, offset, SEEK_SET) < 0) {
    sdbError("seek wal offset %s:%" PRId64 " %s", pFileInfo->name, offset, strerror(errno));
    return NULL;
  }

  void* p = calloc(1, size);
  if (p == NULL) {
    return NULL;
  }
  SWalHead *pHead = p;
  int32_t ret = (int32_t)tfRead(tfd, pHead, size);

  if (ret < size) {
    sdbError("read wal record fail %s", strerror(errno));
    free(p);
    return NULL;
  }

  return pHead;
}

static int FORCE_INLINE sdbCacheLock(mnodeSdbCacheTable* pCache) {
  return pthread_mutex_lock(&pCache->mutex);
}

static int FORCE_INLINE sdbCacheUnlock(mnodeSdbCacheTable* pCache) {
  return pthread_mutex_unlock(&pCache->mutex);
}

static void sdbCacheReadIndex(mnodeSdbTable *pTable, const char* walName, void* p) {
  mnodeSdbCacheTable* pCache = pTable->iHandle;
  walIndex *pIndex = (walIndex*)p;
    
  int16_t idx = initCacheWalInfo(pTable, walName);

  walRecord *pWal = calloc(1, sizeof(walRecord) + pIndex->keyLen);
  pWal->idx = idx;
  memcpy(&(pWal->index), pIndex, sizeof(walIndex) + pIndex->keyLen);

  sdbCacheLock(pCache);
  taosHashPut(pCache->pWalTable, pIndex->data, pIndex->keyLen, &pWal, sizeof(walRecord**));
  sdbCacheUnlock(pCache);
}

static void sdbCacheSyncWal(mnodeSdbTable *pTable, bool putToCache, SWalHead* pHead, SSdbRow* pRow, SWalHeadInfo* pHeadInfo) {
  mnodeSdbCacheTable* pCache = pTable->iHandle;
  int64_t off = pHeadInfo->offset;
  int32_t keySize;
  void* key = sdbTableGetKeyAndSize(pTable->options.keyType, pRow->pObj, &keySize, true);
  
  //sdbInfo("sdbCacheSyncWal: %" PRIu64 ", %d", pHeadInfo->offset, pHeadInfo->len);

  sdbCacheLock(pCache);

  int16_t idx = initCacheWalInfo(pTable, pHeadInfo->name);
  assert(idx != -1);

  walRecord** ppRecord = (walRecord**)taosHashGet(pCache->pWalTable, key, keySize);
  if (ppRecord) {
    (*ppRecord)->index.offset = off;
    (*ppRecord)->index.size = sizeof(SWalHead) + pHead->len;
    (*ppRecord)->idx = idx;
  } else {
    walRecord *pWal = calloc(1, sizeof(walRecord) + sizeof(SWalHead) + pHead->len);
    if (pWal == NULL) {
      sdbCacheUnlock(pCache);
      return;
    }

    *pWal = (walRecord) {
        .index  = (walIndex) {
          .offset = off,
          .size   = sizeof(SWalHead) + pHead->len,
          .keyLen = keySize,      
        },
        .idx    = idx,
    };
    memcpy(pWal->index.data, key, keySize);
    taosHashPut(pCache->pWalTable, key, keySize, &pWal, sizeof(walRecord**));
  }
  
  sdbCacheUnlock(pCache);

  // in restore state,do not evict item,it will make starup slow
  if (putToCache) {
    int ret = cachePut(pCache->pTable, key, keySize, pRow->pObj, pTable->options.cacheDataLen, putToCache, pTable->options.expireTime);
    if (ret != CACHE_OK && pTable->options.freeFp) {
      pTable->options.freeFp(pRow->pObj);
    }
  } else {
    if (pTable->options.freeFp) pTable->options.freeFp(pRow->pObj);    
  }
  free(pRow->pObj);
}

static int loadCacheDataFromWal(void* userData, const void* key, uint8_t nkey, char** value, size_t *len, uint64_t *pExpire) {
  mnodeSdbTable* pTable = (mnodeSdbTable*)userData;
  mnodeSdbCacheTable* pCache = (mnodeSdbCacheTable*)pTable->iHandle;
  sdbCacheLock(pCache);
  
  walRecord** ppRecord = (walRecord**)taosHashGet(pCache->pWalTable, key, nkey);  
  if (ppRecord == NULL) {
    sdbCacheUnlock(pCache);
    sdbError("key %s has no wal index", (char*)key);
    return -1;
  }
  
  SWalHead* pHead = readWal(pTable, (*ppRecord)->idx, (*ppRecord)->index.offset, (*ppRecord)->index.size);
  if (pHead == NULL) {
    sdbError("read wal record for key %s fail", (char*)key);
    sdbCacheUnlock(pCache);
    return -1;
  }  

  char* p = calloc(1, pTable->options.cacheDataLen);
  if (p == NULL) {
    sdbError("calloc wal record for key %s fail", (char*)key);
    sdbCacheUnlock(pCache);
    return -1;
  }

  if (pTable->options.afterLoadFp) {
    pTable->options.afterLoadFp(pTable->options.userData, (*ppRecord)->index.data, (*ppRecord)->index.keyLen, pHead, p);
  }
  
  *value = p;
  *len = pTable->options.cacheDataLen;
  sdbCacheUnlock(pCache);
  free(pHead);

  return 0;
}

static int delCacheData(void* userData, const void* key, uint8_t nkey) {
  mnodeSdbTable* pTable = (mnodeSdbTable*)userData;
  mnodeSdbCacheTable* pCache = (mnodeSdbCacheTable*)pTable->iHandle;

  sdbCacheLock(pCache);
  walRecord** ppRecord = (walRecord**)taosHashGet(pCache->pWalTable, key, nkey);
  if (ppRecord) {
    free(*ppRecord);
    taosHashRemove(pCache->pWalTable, key, nkey);
  }
  sdbCacheUnlock(pCache);
  return 0;
}

// hash table functions
static mnodeSdbHashTable* hashTableInit(mnodeSdbTable* table, mnodeSdbTableOption options) {
  mnodeSdbHashTable* pTable = calloc(1, sizeof(mnodeSdbHashTable));

  _hash_fn_t hashFp = taosGetDefaultHashFunction(TSDB_DATA_TYPE_INT);
  if (options.keyType == SDB_KEY_STRING || options.keyType == SDB_KEY_VAR_STRING) {
    hashFp = taosGetDefaultHashFunction(TSDB_DATA_TYPE_BINARY);
  }
  pthread_mutex_init(&pTable->mutex, NULL);
  pTable->pTable = taosHashInit(options.hashSessions, hashFp, true, HASH_ENTRY_LOCK);

  table->getFp = hashTableGet;
  table->putFp = hashTablePut;
  table->freeValFp = hashTableFreeValue;
  table->delFp = hashTableRemove;
  table->clearFp = hashTableClear;
  table->iterFp = hashTableIterate;
  table->iterValFp = hashTableIterValue;
  table->cancelIterFp = hashTableCancelIterate;
  table->syncFp = NULL;

  return pTable;
}

static int  hashTableGet(mnodeSdbTable *pTable, const void *key, size_t keyLen, void** pRet) {
  assert(pRet != NULL);

  mnodeSdbHashTable* pHash = (mnodeSdbHashTable*)pTable->iHandle;
  pthread_mutex_lock(&pHash->mutex);
  void *p = taosHashGet(pHash->pTable, key, keyLen);
  *pRet = (p != NULL) ? *(void**)p : NULL;
  pthread_mutex_unlock(&pHash->mutex);

  return *pRet != NULL ? 0 : -1;
}

static void hashTablePut(mnodeSdbTable *pTable, SSdbRow* pRow) {
  mnodeSdbHashTable* pHash = (mnodeSdbHashTable*)pTable->iHandle;
  int32_t keySize;
  void* key = sdbTableGetKeyAndSize(pTable->options.keyType, pRow->pObj, &keySize, true);
  pthread_mutex_lock(&pHash->mutex);
  // hash table data is pRow->pObj pointer
  taosHashPut(pHash->pTable, key, keySize, &pRow->pObj, sizeof(int64_t));
  pthread_mutex_unlock(&pHash->mutex);
}

static void hashTableFreeValue(mnodeSdbTable *pTable, void *p) {
  tfree(p);
}

static void hashTableRemove(mnodeSdbTable *pTable, const void *key, size_t keyLen) {
  mnodeSdbHashTable* pHash = (mnodeSdbHashTable*)pTable->iHandle;
  pthread_mutex_lock(&pHash->mutex);
  taosHashRemove(pHash->pTable, key, keyLen);
  pthread_mutex_unlock(&pHash->mutex); 
}

static void hashTableClear(mnodeSdbTable *pTable) {
  mnodeSdbHashTable* pHash = (mnodeSdbHashTable*)pTable->iHandle;
  taosHashCleanup(pHash->pTable);
  pthread_mutex_destroy(&pHash->mutex);

  mnodeSdbTableDestroy(pTable);
  free(pHash);
}

static void* hashTableIterate(mnodeSdbTable *pTable, void *p) {
  mnodeSdbHashTable* pHash = (mnodeSdbHashTable*)pTable->iHandle;
  return taosHashIterate(pHash->pTable, p);
}

static int  hashTableIterValue(mnodeSdbTable *pTable,void *p, void** pRet) {
  *pRet = (p != NULL) ? *(void**)p : NULL;
  return p != NULL ? 0 : -1;
}

static void hashTableCancelIterate(mnodeSdbTable *pTable, void *pIter) {
  mnodeSdbHashTable* pHash = (mnodeSdbHashTable*)pTable->iHandle;
  taosHashCancelIterate(pHash->pTable, pIter);
}

void *sdbTableGetKeyAndSize(int keyType, const void* pObj, int32_t* pSize, bool isObj) {
  void*  key = isObj ? sdbGetObjKey(keyType, pObj) : (void*)pObj;
  *pSize = sizeof(int32_t);
  if (keyType == SDB_KEY_STRING || keyType == SDB_KEY_VAR_STRING) {
    *pSize =  (int32_t)strlen((char *)key);
  }

  return key;
}

static int calcHashPower(mnodeSdbTableOption options) {
  return 10;
}
