// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_ext.h"
#include "plex.h"
static void ConstructElement(CFX_ByteString* pNewData)
{
    new (pNewData) CFX_ByteString();
}
static void DestructElement(CFX_ByteString* pOldData)
{
    pOldData->~CFX_ByteString();
}
CFX_MapPtrToPtr::CFX_MapPtrToPtr(int nBlockSize)
    : m_pHashTable(NULL)
    , m_nHashTableSize(17)
    , m_nCount(0)
    , m_pFreeList(NULL)
    , m_pBlocks(NULL)
    , m_nBlockSize(nBlockSize)
{
    ASSERT(m_nBlockSize > 0);
}
void CFX_MapPtrToPtr::RemoveAll()
{
    if (m_pHashTable) {
        FX_Free(m_pHashTable);
        m_pHashTable = NULL;
    }
    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}
CFX_MapPtrToPtr::~CFX_MapPtrToPtr()
{
    RemoveAll();
    ASSERT(m_nCount == 0);
}
FX_DWORD CFX_MapPtrToPtr::HashKey(void* key) const
{
    return ((FX_DWORD)(FX_UINTPTR)key) >> 4;
}
void CFX_MapPtrToPtr::GetNextAssoc(FX_POSITION& rNextPosition, void*& rKey, void*& rValue) const
{
    ASSERT(m_pHashTable != NULL);
    CAssoc* pAssocRet = (CAssoc*)rNextPosition;
    ASSERT(pAssocRet != NULL);
    if (pAssocRet == (CAssoc*) - 1) {
        for (FX_DWORD nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocRet = m_pHashTable[nBucket]) != NULL) {
                break;
            }
        ASSERT(pAssocRet != NULL);
    }
    CAssoc* pAssocNext;
    if ((pAssocNext = pAssocRet->pNext) == NULL) {
        for (FX_DWORD nBucket = (HashKey(pAssocRet->key) % m_nHashTableSize) + 1; nBucket < m_nHashTableSize; nBucket ++) {
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL) {
                break;
            }
        }
    }
    rNextPosition = (FX_POSITION) pAssocNext;
    rKey = pAssocRet->key;
    rValue = pAssocRet->value;
}
FX_BOOL CFX_MapPtrToPtr::Lookup(void* key, void*& rValue) const
{
    FX_DWORD nHash;
    CAssoc* pAssoc = GetAssocAt(key, nHash);
    if (pAssoc == NULL) {
        return FALSE;
    }
    rValue = pAssoc->value;
    return TRUE;
}
void* CFX_MapPtrToPtr::GetValueAt(void* key) const
{
    FX_DWORD nHash;
    CAssoc* pAssoc = GetAssocAt(key, nHash);
    if (pAssoc == NULL) {
        return NULL;
    }
    return pAssoc->value;
}
void*& CFX_MapPtrToPtr::operator[](void* key)
{
    FX_DWORD nHash;
    CAssoc* pAssoc;
    if ((pAssoc = GetAssocAt(key, nHash)) == NULL) {
        if (m_pHashTable == NULL) {
            InitHashTable(m_nHashTableSize);
        }
        pAssoc = NewAssoc();
        pAssoc->key = key;
        pAssoc->pNext = m_pHashTable[nHash];
        m_pHashTable[nHash] = pAssoc;
    }
    return pAssoc->value;
}
CFX_MapPtrToPtr::CAssoc*
CFX_MapPtrToPtr::GetAssocAt(void* key, FX_DWORD& nHash) const
{
    nHash = HashKey(key) % m_nHashTableSize;
    if (m_pHashTable == NULL) {
        return NULL;
    }
    CAssoc* pAssoc;
    for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext) {
        if (pAssoc->key == key) {
            return pAssoc;
        }
    }
    return NULL;
}
CFX_MapPtrToPtr::CAssoc*
CFX_MapPtrToPtr::NewAssoc()
{
    if (m_pFreeList == NULL) {
        CFX_Plex* newBlock = CFX_Plex::Create(m_pBlocks, m_nBlockSize, sizeof(CFX_MapPtrToPtr::CAssoc));
        CFX_MapPtrToPtr::CAssoc* pAssoc = (CFX_MapPtrToPtr::CAssoc*)newBlock->data();
        pAssoc += m_nBlockSize - 1;
        for (int i = m_nBlockSize - 1; i >= 0; i--, pAssoc--) {
            pAssoc->pNext = m_pFreeList;
            m_pFreeList = pAssoc;
        }
    }
    ASSERT(m_pFreeList != NULL);
    CFX_MapPtrToPtr::CAssoc* pAssoc = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;
    m_nCount++;
    ASSERT(m_nCount > 0);
    pAssoc->key = 0;
    pAssoc->value = 0;
    return pAssoc;
}
void CFX_MapPtrToPtr::InitHashTable(
    FX_DWORD nHashSize, FX_BOOL bAllocNow)
{
    ASSERT(m_nCount == 0);
    ASSERT(nHashSize > 0);
    if (m_pHashTable != NULL) {
        FX_Free(m_pHashTable);
        m_pHashTable = NULL;
    }
    if (bAllocNow) {
        m_pHashTable = FX_Alloc(CAssoc*, nHashSize);
    }
    m_nHashTableSize = nHashSize;
}
FX_BOOL CFX_MapPtrToPtr::RemoveKey(void* key)
{
    if (m_pHashTable == NULL) {
        return FALSE;
    }
    CAssoc** ppAssocPrev;
    ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];
    CAssoc* pAssoc;
    for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext) {
        if (pAssoc->key == key) {
            *ppAssocPrev = pAssoc->pNext;
            FreeAssoc(pAssoc);
            return TRUE;
        }
        ppAssocPrev = &pAssoc->pNext;
    }
    return FALSE;
}
void CFX_MapPtrToPtr::FreeAssoc(CFX_MapPtrToPtr::CAssoc* pAssoc)
{
    pAssoc->pNext = m_pFreeList;
    m_pFreeList = pAssoc;
    m_nCount--;
    ASSERT(m_nCount >= 0);
    if (m_nCount == 0) {
        RemoveAll();
    }
}
CFX_MapByteStringToPtr::CFX_MapByteStringToPtr(int nBlockSize)
    : m_pHashTable(NULL)
    , m_nHashTableSize(17)
    , m_nCount(0)
    , m_pFreeList(NULL)
    , m_pBlocks(NULL)
    , m_nBlockSize(nBlockSize)
{
    ASSERT(m_nBlockSize > 0);
}
void CFX_MapByteStringToPtr::RemoveAll()
{
    if (m_pHashTable != NULL) {
        for (FX_DWORD nHash = 0; nHash < m_nHashTableSize; nHash++) {
            CAssoc* pAssoc;
            for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
                    pAssoc = pAssoc->pNext) {
                DestructElement(&pAssoc->key);
            }
        }
        FX_Free(m_pHashTable);
        m_pHashTable = NULL;
    }
    m_nCount = 0;
    m_pFreeList = NULL;
    m_pBlocks->FreeDataChain();
    m_pBlocks = NULL;
}
CFX_MapByteStringToPtr::~CFX_MapByteStringToPtr()
{
    RemoveAll();
    ASSERT(m_nCount == 0);
}
void CFX_MapByteStringToPtr::GetNextAssoc(FX_POSITION& rNextPosition,
        CFX_ByteString& rKey, void*& rValue) const
{
    ASSERT(m_pHashTable != NULL);
    CAssoc* pAssocRet = (CAssoc*)rNextPosition;
    ASSERT(pAssocRet != NULL);
    if (pAssocRet == (CAssoc*) - 1) {
        for (FX_DWORD nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocRet = m_pHashTable[nBucket]) != NULL) {
                break;
            }
        ASSERT(pAssocRet != NULL);
    }
    CAssoc* pAssocNext;
    if ((pAssocNext = pAssocRet->pNext) == NULL) {
        for (FX_DWORD nBucket = pAssocRet->nHashValue + 1;
                nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL) {
                break;
            }
    }
    rNextPosition = (FX_POSITION) pAssocNext;
    rKey = pAssocRet->key;
    rValue = pAssocRet->value;
}
FX_LPVOID CFX_MapByteStringToPtr::GetNextValue(FX_POSITION& rNextPosition) const
{
    ASSERT(m_pHashTable != NULL);
    CAssoc* pAssocRet = (CAssoc*)rNextPosition;
    ASSERT(pAssocRet != NULL);
    if (pAssocRet == (CAssoc*) - 1) {
        for (FX_DWORD nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocRet = m_pHashTable[nBucket]) != NULL) {
                break;
            }
        ASSERT(pAssocRet != NULL);
    }
    CAssoc* pAssocNext;
    if ((pAssocNext = pAssocRet->pNext) == NULL) {
        for (FX_DWORD nBucket = pAssocRet->nHashValue + 1;
                nBucket < m_nHashTableSize; nBucket++)
            if ((pAssocNext = m_pHashTable[nBucket]) != NULL) {
                break;
            }
    }
    rNextPosition = (FX_POSITION) pAssocNext;
    return pAssocRet->value;
}
void*& CFX_MapByteStringToPtr::operator[](FX_BSTR key)
{
    FX_DWORD nHash;
    CAssoc* pAssoc;
    if ((pAssoc = GetAssocAt(key, nHash)) == NULL) {
        if (m_pHashTable == NULL) {
            InitHashTable(m_nHashTableSize);
        }
        pAssoc = NewAssoc();
        pAssoc->nHashValue = nHash;
        pAssoc->key = key;
        pAssoc->pNext = m_pHashTable[nHash];
        m_pHashTable[nHash] = pAssoc;
    }
    return pAssoc->value;
}
CFX_MapByteStringToPtr::CAssoc*
CFX_MapByteStringToPtr::NewAssoc()
{
    if (m_pFreeList == NULL) {
        CFX_Plex* newBlock = CFX_Plex::Create(m_pBlocks, m_nBlockSize, sizeof(CFX_MapByteStringToPtr::CAssoc));
        CFX_MapByteStringToPtr::CAssoc* pAssoc = (CFX_MapByteStringToPtr::CAssoc*)newBlock->data();
        pAssoc += m_nBlockSize - 1;
        for (int i = m_nBlockSize - 1; i >= 0; i--, pAssoc--) {
            pAssoc->pNext = m_pFreeList;
            m_pFreeList = pAssoc;
        }
    }
    ASSERT(m_pFreeList != NULL);
    CFX_MapByteStringToPtr::CAssoc* pAssoc = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;
    m_nCount++;
    ASSERT(m_nCount > 0);
    ConstructElement(&pAssoc->key);
    pAssoc->value = 0;
    return pAssoc;
}
void CFX_MapByteStringToPtr::FreeAssoc(CFX_MapByteStringToPtr::CAssoc* pAssoc)
{
    DestructElement(&pAssoc->key);
    pAssoc->pNext = m_pFreeList;
    m_pFreeList = pAssoc;
    m_nCount--;
    ASSERT(m_nCount >= 0);
    if (m_nCount == 0) {
        RemoveAll();
    }
}
CFX_MapByteStringToPtr::CAssoc*
CFX_MapByteStringToPtr::GetAssocAt(FX_BSTR key, FX_DWORD& nHash) const
{
    nHash = HashKey(key) % m_nHashTableSize;
    if (m_pHashTable == NULL) {
        return NULL;
    }
    CAssoc* pAssoc;
    for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext) {
        if (pAssoc->key == key) {
            return pAssoc;
        }
    }
    return NULL;
}
FX_BOOL CFX_MapByteStringToPtr::Lookup(FX_BSTR key, void*& rValue) const
{
    FX_DWORD nHash;
    CAssoc* pAssoc = GetAssocAt(key, nHash);
    if (pAssoc == NULL) {
        return FALSE;
    }
    rValue = pAssoc->value;
    return TRUE;
}
void CFX_MapByteStringToPtr::InitHashTable(
    FX_DWORD nHashSize, FX_BOOL bAllocNow)
{
    ASSERT(m_nCount == 0);
    ASSERT(nHashSize > 0);
    if (m_pHashTable != NULL) {
        FX_Free(m_pHashTable);
        m_pHashTable = NULL;
    }
    if (bAllocNow) {
        m_pHashTable = FX_Alloc(CAssoc*, nHashSize);
    }
    m_nHashTableSize = nHashSize;
}
inline FX_DWORD CFX_MapByteStringToPtr::HashKey(FX_BSTR key) const
{
    FX_DWORD nHash = 0;
    int len = key.GetLength();
    FX_LPCBYTE buf = key.GetPtr();
    for (int i = 0; i < len; i ++) {
        nHash = (nHash << 5) + nHash + buf[i];
    }
    return nHash;
}
FX_BOOL CFX_MapByteStringToPtr::RemoveKey(FX_BSTR key)
{
    if (m_pHashTable == NULL) {
        return FALSE;
    }
    CAssoc** ppAssocPrev;
    ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];
    CAssoc* pAssoc;
    for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext) {
        if (pAssoc->key == key) {
            *ppAssocPrev = pAssoc->pNext;
            FreeAssoc(pAssoc);
            return TRUE;
        }
        ppAssocPrev = &pAssoc->pNext;
    }
    return FALSE;
}
struct _CompactString {
    FX_BYTE		m_CompactLen;
    FX_BYTE		m_LenHigh;
    FX_BYTE		m_LenLow;
    FX_BYTE		m_Unused;
    FX_LPBYTE	m_pBuffer;
};
static void _CompactStringRelease(_CompactString* pCompact)
{
    if (pCompact->m_CompactLen == 0xff) {
        FX_Free(pCompact->m_pBuffer);
    }
}
static FX_BOOL _CompactStringSame(_CompactString* pCompact, FX_LPCBYTE pStr, int len)
{
    if (len < sizeof(_CompactString)) {
        if (pCompact->m_CompactLen != len) {
            return FALSE;
        }
        return FXSYS_memcmp32(&pCompact->m_LenHigh, pStr, len) == 0;
    }
    if (pCompact->m_CompactLen != 0xff || pCompact->m_LenHigh * 256 + pCompact->m_LenLow != len) {
        return FALSE;
    }
    return FXSYS_memcmp32(pCompact->m_pBuffer, pStr, len) == 0;
}
static void _CompactStringStore(_CompactString* pCompact, FX_LPCBYTE pStr, int len)
{
    if (len < (int)sizeof(_CompactString)) {
        pCompact->m_CompactLen = (FX_BYTE)len;
        FXSYS_memcpy32(&pCompact->m_LenHigh, pStr, len);
        return;
    }
    pCompact->m_CompactLen = 0xff;
    pCompact->m_LenHigh = len / 256;
    pCompact->m_LenLow = len % 256;
    pCompact->m_pBuffer = FX_Alloc(FX_BYTE, len);
    FXSYS_memcpy32(pCompact->m_pBuffer, pStr, len);
}
static CFX_ByteStringC _CompactStringGet(_CompactString* pCompact)
{
    if (pCompact->m_CompactLen == 0xff) {
        return CFX_ByteStringC(pCompact->m_pBuffer, pCompact->m_LenHigh * 256 + pCompact->m_LenLow);
    }
    if (pCompact->m_CompactLen == 0xfe) {
        return CFX_ByteStringC();
    }
    return CFX_ByteStringC(&pCompact->m_LenHigh, pCompact->m_CompactLen);
}
#define CMAP_ALLOC_STEP		8
#define CMAP_INDEX_SIZE		8
CFX_CMapByteStringToPtr::CFX_CMapByteStringToPtr()
    : m_Buffer(sizeof(_CompactString) + sizeof(void*), CMAP_ALLOC_STEP, CMAP_INDEX_SIZE)
{
}
CFX_CMapByteStringToPtr::~CFX_CMapByteStringToPtr()
{
    RemoveAll();
}
void CFX_CMapByteStringToPtr::RemoveAll()
{
    int size = m_Buffer.GetSize();
    for (int i = 0; i < size; i++) {
        _CompactStringRelease((_CompactString*)m_Buffer.GetAt(i));
    }
    m_Buffer.RemoveAll();
}
FX_POSITION CFX_CMapByteStringToPtr::GetStartPosition() const
{
    int size = m_Buffer.GetSize();
    for (int i = 0; i < size; i ++) {
        _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(i);
        if (pKey->m_CompactLen != 0xfe) {
            return (FX_POSITION)(FX_UINTPTR)(i + 1);
        }
    }
    return NULL;
}
void CFX_CMapByteStringToPtr::GetNextAssoc(FX_POSITION& rNextPosition, CFX_ByteString& rKey, void*& rValue) const
{
    if (rNextPosition == NULL) {
        return;
    }
    int index = (int)(FX_UINTPTR)rNextPosition - 1;
    _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(index);
    rKey = _CompactStringGet(pKey);
    rValue = *(void**)(pKey + 1);
    index ++;
    int size = m_Buffer.GetSize();
    while (index < size) {
        pKey = (_CompactString*)m_Buffer.GetAt(index);
        if (pKey->m_CompactLen != 0xfe) {
            rNextPosition = (FX_POSITION)(FX_UINTPTR)(index + 1);
            return;
        }
        index ++;
    }
    rNextPosition = NULL;
}
FX_LPVOID CFX_CMapByteStringToPtr::GetNextValue(FX_POSITION& rNextPosition) const
{
    if (rNextPosition == NULL) {
        return NULL;
    }
    int index = (int)(FX_UINTPTR)rNextPosition - 1;
    _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(index);
    FX_LPVOID rValue = *(void**)(pKey + 1);
    index ++;
    int size = m_Buffer.GetSize();
    while (index < size) {
        pKey = (_CompactString*)m_Buffer.GetAt(index);
        if (pKey->m_CompactLen != 0xfe) {
            rNextPosition = (FX_POSITION)(FX_UINTPTR)(index + 1);
            return rValue;
        }
        index ++;
    }
    rNextPosition = NULL;
    return rValue;
}
FX_BOOL _CMapLookupCallback(void* param, void* pData)
{
    return !_CompactStringSame((_CompactString*)pData, ((CFX_ByteStringC*)param)->GetPtr(), ((CFX_ByteStringC*)param)->GetLength());
}
FX_BOOL CFX_CMapByteStringToPtr::Lookup(FX_BSTR key, void*& rValue) const
{
    void* p = m_Buffer.Iterate(_CMapLookupCallback, (void*)&key);
    if (!p) {
        return FALSE;
    }
    rValue = *(void**)((_CompactString*)p + 1);
    return TRUE;
}
void CFX_CMapByteStringToPtr::SetAt(FX_BSTR key, void* value)
{
    ASSERT(value != NULL);
    int index, key_len = key.GetLength();
    int size = m_Buffer.GetSize();
    for (index = 0; index < size; index ++) {
        _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(index);
        if (!_CompactStringSame(pKey, key.GetPtr(), key_len)) {
            continue;
        }
        *(void**)(pKey + 1) = value;
        return;
    }
    for (index = 0; index < size; index ++) {
        _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(index);
        if (pKey->m_CompactLen) {
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
void CFX_CMapByteStringToPtr::AddValue(FX_BSTR key, void* value)
{
    ASSERT(value != NULL);
    _CompactString* pKey = (_CompactString*)m_Buffer.Add();
    _CompactStringStore(pKey, key.GetPtr(), key.GetLength());
    *(void**)(pKey + 1) = value;
}
void CFX_CMapByteStringToPtr::RemoveKey(FX_BSTR key)
{
    int key_len = key.GetLength();
    int size = m_Buffer.GetSize();
    for (int index = 0; index < size; index++) {
        _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(index);
        if (!_CompactStringSame(pKey, key.GetPtr(), key_len)) {
            continue;
        }
        _CompactStringRelease(pKey);
        pKey->m_CompactLen = 0xfe;
        return;
    }
}
int CFX_CMapByteStringToPtr::GetCount() const
{
    int count = 0;
    int size = m_Buffer.GetSize();
    for (int i = 0; i < size; i ++) {
        _CompactString* pKey = (_CompactString*)m_Buffer.GetAt(i);
        if (pKey->m_CompactLen != 0xfe) {
            count ++;
        }
    }
    return count;
}
extern "C" {
    static int _CompareDWord(const void* p1, const void* p2)
    {
        return (*(FX_DWORD*)p1) - (*(FX_DWORD*)p2);
    }
};
struct _DWordPair {
    FX_DWORD key;
    FX_DWORD value;
};
FX_BOOL CFX_CMapDWordToDWord::Lookup(FX_DWORD key, FX_DWORD& value) const
{
    FX_LPVOID pResult = FXSYS_bsearch(&key, m_Buffer.GetBuffer(), m_Buffer.GetSize() / sizeof(_DWordPair),
                                      sizeof(_DWordPair), _CompareDWord);
    if (pResult == NULL) {
        return FALSE;
    }
    value = ((FX_DWORD*)pResult)[1];
    return TRUE;
}
FX_POSITION CFX_CMapDWordToDWord::GetStartPosition() const
{
    FX_DWORD count = m_Buffer.GetSize() / sizeof(_DWordPair);
    if (count == 0) {
        return NULL;
    }
    return (FX_POSITION)1;
}
void CFX_CMapDWordToDWord::GetNextAssoc(FX_POSITION& pos, FX_DWORD& key, FX_DWORD& value) const
{
    if (pos == 0) {
        return;
    }
    FX_DWORD index = ((FX_DWORD)(FX_UINTPTR)pos) - 1;
    FX_DWORD count = m_Buffer.GetSize() / sizeof(_DWordPair);
    _DWordPair* buf = (_DWordPair*)m_Buffer.GetBuffer();
    key = buf[index].key;
    value = buf[index].value;
    if (index == count - 1) {
        pos = 0;
    } else {
        pos = (FX_POSITION)((FX_UINTPTR)pos + 1);
    }
}
void CFX_CMapDWordToDWord::SetAt(FX_DWORD key, FX_DWORD value)
{
    FX_DWORD count = m_Buffer.GetSize() / sizeof(_DWordPair);
    _DWordPair* buf = (_DWordPair*)m_Buffer.GetBuffer();
    _DWordPair pair = {key, value};
    if (count == 0 || key > buf[count - 1].key) {
        m_Buffer.AppendBlock(&pair, sizeof(_DWordPair));
        return;
    }
    int low = 0, high = count - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (buf[mid].key < key) {
            low = mid + 1;
        } else if (buf[mid].key > key) {
            high = mid - 1;
        } else {
            buf[mid].value = value;
            return;
        }
    }
    m_Buffer.InsertBlock(low * sizeof(_DWordPair), &pair, sizeof(_DWordPair));
}
void CFX_CMapDWordToDWord::EstimateSize(FX_DWORD size, FX_DWORD grow_by)
{
    m_Buffer.EstimateSize(size, grow_by);
}
