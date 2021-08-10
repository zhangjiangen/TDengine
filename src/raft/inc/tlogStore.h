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

#ifndef _TD_LOG_SOTRE_H_
#define _TD_LOG_SOTRE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SLogStoreVTable {
  // TODO: use function pointer to implement interfaces
} SLogStoreVTable;

typedef struct SLogStore {
  SLogStoreVTable vtable;
  void *          impl;
} SLogStore;

// TODO: add more interfaces for SLogStore
int tLogStoreInit(SLogStore *pStore, void *impl, const SLogStoreVTable *pVTable);

#ifdef __cplusplus
}
#endif

#endif /*_TD_LOG_SOTRE_H_*/