// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <stdlib.h>  // For abort().
#include "../../include/fxcrt/fx_memory.h"

void*	FXMEM_DefaultAlloc(size_t byte_size, int flags)
{
    return (void*)malloc(byte_size);
}
void*	FXMEM_DefaultRealloc(void* pointer, size_t new_size, int flags)
{
    return realloc(pointer, new_size);
}
void	FXMEM_DefaultFree(void* pointer, int flags)
{
    free(pointer);
}

NEVER_INLINE void FX_OutOfMemoryTerminate() {
    // Termimate cleanly if we can, else crash at a specific address (0xbd).
    abort();
    reinterpret_cast<void(*)()>(0xbd)();
}

CFX_GrowOnlyPool::CFX_GrowOnlyPool(size_t trunk_size)
{
    m_TrunkSize = trunk_size;
    m_pFirstTrunk = NULL;
}
CFX_GrowOnlyPool::~CFX_GrowOnlyPool()
{
    FreeAll();
}
struct _FX_GrowOnlyTrunk {
    size_t	m_Size;
    size_t	m_Allocated;
    _FX_GrowOnlyTrunk*	m_pNext;
};
void CFX_GrowOnlyPool::FreeAll()
{
    _FX_GrowOnlyTrunk* pTrunk = (_FX_GrowOnlyTrunk*)m_pFirstTrunk;
    while (pTrunk) {
        _FX_GrowOnlyTrunk* pNext = pTrunk->m_pNext;
        FX_Free(pTrunk);
        pTrunk = pNext;
    }
    m_pFirstTrunk = NULL;
}
void* CFX_GrowOnlyPool::Alloc(size_t size)
{
    size = (size + 3) / 4 * 4;
    _FX_GrowOnlyTrunk* pTrunk = (_FX_GrowOnlyTrunk*)m_pFirstTrunk;
    while (pTrunk) {
        if (pTrunk->m_Size - pTrunk->m_Allocated >= size) {
            void* p = (FX_LPBYTE)(pTrunk + 1) + pTrunk->m_Allocated;
            pTrunk->m_Allocated += size;
            return p;
        }
        pTrunk = pTrunk->m_pNext;
    }
    size_t alloc_size = size > m_TrunkSize ? size : m_TrunkSize;
    pTrunk = (_FX_GrowOnlyTrunk*)FX_Alloc(FX_BYTE, sizeof(_FX_GrowOnlyTrunk) + alloc_size);
    pTrunk->m_Size = alloc_size;
    pTrunk->m_Allocated = size;
    pTrunk->m_pNext = (_FX_GrowOnlyTrunk*)m_pFirstTrunk;
    m_pFirstTrunk = pTrunk;
    return pTrunk + 1;
}
