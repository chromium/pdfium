// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxcrt/fx_basic.h"
#include "plex.h"

namespace {

const uint8_t kFreeLength = 0xfe;
const uint8_t kHasAllocatedBufferLength = 0xff;

}  // namespace

CFX_MapPtrToPtr::CFX_MapPtrToPtr(int nBlockSize)
    : m_pHashTable(NULL),
      m_nHashTableSize(17),
      m_nCount(0),
      m_pFreeList(NULL),
      m_pBlocks(NULL),
      m_nBlockSize(nBlockSize) {
  ASSERT(m_nBlockSize > 0);
}
void CFX_MapPtrToPtr::RemoveAll() {
  FX_Free(m_pHashTable);
  m_pHashTable = NULL;
  m_nCount = 0;
  m_pFreeList = NULL;
  m_pBlocks->FreeDataChain();
  m_pBlocks = NULL;
}
CFX_MapPtrToPtr::~CFX_MapPtrToPtr() {
  RemoveAll();
  ASSERT(m_nCount == 0);
}
FX_DWORD CFX_MapPtrToPtr::HashKey(void* key) const {
  return ((FX_DWORD)(uintptr_t)key) >> 4;
}
void CFX_MapPtrToPtr::GetNextAssoc(FX_POSITION& rNextPosition,
                                   void*& rKey,
                                   void*& rValue) const {
  ASSERT(m_pHashTable);
  CAssoc* pAssocRet = (CAssoc*)rNextPosition;
  ASSERT(pAssocRet);
  if (pAssocRet == (CAssoc*)-1) {
    for (FX_DWORD nBucket = 0; nBucket < m_nHashTableSize; nBucket++) {
      if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
        break;
    }
    ASSERT(pAssocRet);
  }
  CAssoc* pAssocNext;
  if ((pAssocNext = pAssocRet->pNext) == NULL) {
    for (FX_DWORD nBucket = (HashKey(pAssocRet->key) % m_nHashTableSize) + 1;
         nBucket < m_nHashTableSize; nBucket++) {
      if ((pAssocNext = m_pHashTable[nBucket]) != NULL) {
        break;
      }
    }
  }
  rNextPosition = (FX_POSITION)pAssocNext;
  rKey = pAssocRet->key;
  rValue = pAssocRet->value;
}
FX_BOOL CFX_MapPtrToPtr::Lookup(void* key, void*& rValue) const {
  FX_DWORD nHash;
  CAssoc* pAssoc = GetAssocAt(key, nHash);
  if (!pAssoc) {
    return FALSE;
  }
  rValue = pAssoc->value;
  return TRUE;
}
void* CFX_MapPtrToPtr::GetValueAt(void* key) const {
  FX_DWORD nHash;
  CAssoc* pAssoc = GetAssocAt(key, nHash);
  if (!pAssoc) {
    return NULL;
  }
  return pAssoc->value;
}
void*& CFX_MapPtrToPtr::operator[](void* key) {
  FX_DWORD nHash;
  CAssoc* pAssoc;
  if ((pAssoc = GetAssocAt(key, nHash)) == NULL) {
    if (!m_pHashTable) {
      InitHashTable(m_nHashTableSize);
    }
    pAssoc = NewAssoc();
    pAssoc->key = key;
    pAssoc->pNext = m_pHashTable[nHash];
    m_pHashTable[nHash] = pAssoc;
  }
  return pAssoc->value;
}
CFX_MapPtrToPtr::CAssoc* CFX_MapPtrToPtr::GetAssocAt(void* key,
                                                     FX_DWORD& nHash) const {
  nHash = HashKey(key) % m_nHashTableSize;
  if (!m_pHashTable) {
    return NULL;
  }
  CAssoc* pAssoc;
  for (pAssoc = m_pHashTable[nHash]; pAssoc; pAssoc = pAssoc->pNext) {
    if (pAssoc->key == key)
      return pAssoc;
  }
  return NULL;
}
CFX_MapPtrToPtr::CAssoc* CFX_MapPtrToPtr::NewAssoc() {
  if (!m_pFreeList) {
    CFX_Plex* newBlock = CFX_Plex::Create(m_pBlocks, m_nBlockSize,
                                          sizeof(CFX_MapPtrToPtr::CAssoc));
    CFX_MapPtrToPtr::CAssoc* pAssoc =
        (CFX_MapPtrToPtr::CAssoc*)newBlock->data();
    pAssoc += m_nBlockSize - 1;
    for (int i = m_nBlockSize - 1; i >= 0; i--, pAssoc--) {
      pAssoc->pNext = m_pFreeList;
      m_pFreeList = pAssoc;
    }
  }
  CFX_MapPtrToPtr::CAssoc* pAssoc = m_pFreeList;
  m_pFreeList = m_pFreeList->pNext;
  m_nCount++;
  ASSERT(m_nCount > 0);
  pAssoc->key = 0;
  pAssoc->value = 0;
  return pAssoc;
}
void CFX_MapPtrToPtr::InitHashTable(FX_DWORD nHashSize, FX_BOOL bAllocNow) {
  ASSERT(m_nCount == 0);
  ASSERT(nHashSize > 0);
  FX_Free(m_pHashTable);
  m_pHashTable = NULL;
  if (bAllocNow) {
    m_pHashTable = FX_Alloc(CAssoc*, nHashSize);
  }
  m_nHashTableSize = nHashSize;
}
FX_BOOL CFX_MapPtrToPtr::RemoveKey(void* key) {
  if (!m_pHashTable) {
    return FALSE;
  }
  CAssoc** ppAssocPrev;
  ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];
  CAssoc* pAssoc;
  for (pAssoc = *ppAssocPrev; pAssoc; pAssoc = pAssoc->pNext) {
    if (pAssoc->key == key) {
      *ppAssocPrev = pAssoc->pNext;
      FreeAssoc(pAssoc);
      return TRUE;
    }
    ppAssocPrev = &pAssoc->pNext;
  }
  return FALSE;
}
void CFX_MapPtrToPtr::FreeAssoc(CFX_MapPtrToPtr::CAssoc* pAssoc) {
  pAssoc->pNext = m_pFreeList;
  m_pFreeList = pAssoc;
  m_nCount--;
  ASSERT(m_nCount >= 0);
  if (m_nCount == 0) {
    RemoveAll();
  }
}
struct _CompactString {
  uint8_t m_CompactLen;
  uint8_t m_LenHigh;
  uint8_t m_LenLow;
  uint8_t m_Unused;
  uint8_t* m_pBuffer;
};
static void _CompactStringRelease(_CompactString* pCompact) {
  if (pCompact->m_CompactLen == kHasAllocatedBufferLength) {
    FX_Free(pCompact->m_pBuffer);
  }
}
static FX_BOOL _CompactStringSame(_CompactString* pCompact,
                                  const uint8_t* pStr,
                                  int len) {
  if (len < sizeof(_CompactString)) {
    if (pCompact->m_CompactLen != len) {
      return FALSE;
    }
    return FXSYS_memcmp(&pCompact->m_LenHigh, pStr, len) == 0;
  }
  if (pCompact->m_CompactLen != kHasAllocatedBufferLength ||
      pCompact->m_LenHigh * 256 + pCompact->m_LenLow != len) {
    return FALSE;
  }
  return FXSYS_memcmp(pCompact->m_pBuffer, pStr, len) == 0;
}
static void _CompactStringStore(_CompactString* pCompact,
                                const uint8_t* pStr,
                                int len) {
  if (len < (int)sizeof(_CompactString)) {
    pCompact->m_CompactLen = (uint8_t)len;
    FXSYS_memcpy(&pCompact->m_LenHigh, pStr, len);
    return;
  }
  pCompact->m_CompactLen = kHasAllocatedBufferLength;
  pCompact->m_LenHigh = len / 256;
  pCompact->m_LenLow = len % 256;
  pCompact->m_pBuffer = FX_Alloc(uint8_t, len);
  FXSYS_memcpy(pCompact->m_pBuffer, pStr, len);
}
static CFX_ByteStringC _CompactStringGet(_CompactString* pCompact) {
  if (pCompact->m_CompactLen == kHasAllocatedBufferLength) {
    return CFX_ByteStringC(pCompact->m_pBuffer,
                           pCompact->m_LenHigh * 256 + pCompact->m_LenLow);
  }
  if (pCompact->m_CompactLen == kFreeLength) {
    return CFX_ByteStringC();
  }
  return CFX_ByteStringC(&pCompact->m_LenHigh, pCompact->m_CompactLen);
}
#define CMAP_ALLOC_STEP 8
#define CMAP_INDEX_SIZE 8
CFX_CMapByteStringToPtr::CFX_CMapByteStringToPtr()
    : m_Buffer(sizeof(_CompactString) + sizeof(void*),
               CMAP_ALLOC_STEP,
               CMAP_INDEX_SIZE) {}
CFX_CMapByteStringToPtr::~CFX_CMapByteStringToPtr() {
  RemoveAll();
}
void CFX_CMapByteStringToPtr::RemoveAll() {
  int size = m_Buffer.GetSize();
  for (int i = 0; i < size; i++) {
    _CompactStringRelease((_CompactString*)m_Buffer.GetAt(i));
  }
  m_Buffer.RemoveAll();
}
FX_POSITION CFX_CMapByteStringToPtr::GetStartPosition() const {
  int size = m_Buffer.GetSize();
  for (int i = 0; i < size; i++) {
    _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(i);
    if (pKey->m_CompactLen != kFreeLength) {
      return (FX_POSITION)(uintptr_t)(i + 1);
    }
  }
  return NULL;
}
void CFX_CMapByteStringToPtr::GetNextAssoc(FX_POSITION& rNextPosition,
                                           CFX_ByteString& rKey,
                                           void*& rValue) const {
  if (!rNextPosition) {
    return;
  }
  int index = (int)(uintptr_t)rNextPosition - 1;
  _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(index);
  rKey = _CompactStringGet(pKey);
  rValue = *(void**)(pKey + 1);
  index++;
  int size = m_Buffer.GetSize();
  while (index < size) {
    pKey = (_CompactString*)m_Buffer.GetAt(index);
    if (pKey->m_CompactLen != kFreeLength) {
      rNextPosition = (FX_POSITION)(uintptr_t)(index + 1);
      return;
    }
    index++;
  }
  rNextPosition = NULL;
}
void* CFX_CMapByteStringToPtr::GetNextValue(FX_POSITION& rNextPosition) const {
  if (!rNextPosition) {
    return NULL;
  }
  int index = (int)(uintptr_t)rNextPosition - 1;
  _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(index);
  void* rValue = *(void**)(pKey + 1);
  index++;
  int size = m_Buffer.GetSize();
  while (index < size) {
    pKey = (_CompactString*)m_Buffer.GetAt(index);
    if (pKey->m_CompactLen != kFreeLength) {
      rNextPosition = (FX_POSITION)(uintptr_t)(index + 1);
      return rValue;
    }
    index++;
  }
  rNextPosition = NULL;
  return rValue;
}
FX_BOOL _CMapLookupCallback(void* param, void* pData) {
  return !_CompactStringSame((_CompactString*)pData,
                             ((CFX_ByteStringC*)param)->GetPtr(),
                             ((CFX_ByteStringC*)param)->GetLength());
}
FX_BOOL CFX_CMapByteStringToPtr::Lookup(const CFX_ByteStringC& key,
                                        void*& rValue) const {
  void* p = m_Buffer.Iterate(_CMapLookupCallback, (void*)&key);
  if (!p) {
    return FALSE;
  }
  rValue = *(void**)((_CompactString*)p + 1);
  return TRUE;
}
void CFX_CMapByteStringToPtr::SetAt(const CFX_ByteStringC& key, void* value) {
  ASSERT(value);
  int key_len = key.GetLength();
  int size = m_Buffer.GetSize();
  for (int index = 0; index < size; index++) {
    _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(index);
    if (!_CompactStringSame(pKey, key.GetPtr(), key_len)) {
      continue;
    }
    *(void**)(pKey + 1) = value;
    return;
  }
  for (int index = 0; index < size; index++) {
    _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(index);
    if (pKey->m_CompactLen != kFreeLength) {
      continue;
    }
    _CompactStringStore(pKey, key.GetPtr(), key_len);
    *(void**)(pKey + 1) = value;
    return;
  }
  _CompactString* pKey = (_CompactString*)m_Buffer.Add();
  _CompactStringStore(pKey, key.GetPtr(), key_len);
  *(void**)(pKey + 1) = value;
}
void CFX_CMapByteStringToPtr::AddValue(const CFX_ByteStringC& key,
                                       void* value) {
  ASSERT(value);
  _CompactString* pKey = (_CompactString*)m_Buffer.Add();
  _CompactStringStore(pKey, key.GetPtr(), key.GetLength());
  *(void**)(pKey + 1) = value;
}
void CFX_CMapByteStringToPtr::RemoveKey(const CFX_ByteStringC& key) {
  int key_len = key.GetLength();
  int size = m_Buffer.GetSize();
  for (int index = 0; index < size; index++) {
    _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(index);
    if (!_CompactStringSame(pKey, key.GetPtr(), key_len)) {
      continue;
    }
    _CompactStringRelease(pKey);
    pKey->m_CompactLen = kFreeLength;
    return;
  }
}
int CFX_CMapByteStringToPtr::GetCount() const {
  int count = 0;
  int size = m_Buffer.GetSize();
  for (int i = 0; i < size; i++) {
    _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(i);
    if (pKey->m_CompactLen != kFreeLength) {
      count++;
    }
  }
  return count;
}
