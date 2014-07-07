// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_basic.h"
#include "mem_int.h"

void FXMEM_DestroyFoxitMgr(FXMEM_FoxitMgr* pFoxitMgr)
{
    if (pFoxitMgr == NULL) {
        return;
    }
    CFX_MemoryMgr* p = (CFX_MemoryMgr*)pFoxitMgr;
    if (p->m_pSystemMgr->CollectAll) {
        p->m_pSystemMgr->CollectAll(p->m_pSystemMgr);
    }
    if (p->m_bReleaseMgr) {
        p->m_pSystemMgr->Free(p->m_pSystemMgr, p, 0);
    }
    if (p->m_pExternalMemory) {
        free(p->m_pExternalMemory);
    }
}
#ifdef __cplusplus
extern "C" {
#endif
static void* _DefAllocDebug(IFX_Allocator* pAllocator, size_t num, size_t size, FX_LPCSTR filename, int line)
{
    if (size == 0 || num > SIZE_MAX/size)
        return NULL;

    size = size * num; 
    return ((FX_DefAllocator*)pAllocator)->m_pFoxitMgr->AllocDebug(size, 0, filename, line);
}
static void* _DefAlloc(IFX_Allocator* pAllocator, size_t num, size_t size)
{
    if (size == 0 || num > SIZE_MAX/size)
        return NULL;

    size = size * num;
    return ((FX_DefAllocator*)pAllocator)->m_pFoxitMgr->Alloc(size, 0);
}
static void* _DefReallocDebug(IFX_Allocator* pAllocator, void* p, size_t new_num, size_t size, FX_LPCSTR filename, int line)
{
    if (size == 0 || new_num > SIZE_MAX/size)
        return NULL;

    size = size * new_num;
    return ((FX_DefAllocator*)pAllocator)->m_pFoxitMgr->ReallocDebug(p, size, 0, filename, line);
}
static void* _DefRealloc(IFX_Allocator* pAllocator, void* p, size_t new_num, size_t size)
{
    if (size == 0 || new_num > SIZE_MAX/size)
        return NULL;

    size = size * new_num;
    return ((FX_DefAllocator*)pAllocator)->m_pFoxitMgr->Realloc(p, size, 0);
}
static void _DefFree(IFX_Allocator* pAllocator, void* p)
{
    ((FX_DefAllocator*)pAllocator)->m_pFoxitMgr->Free(p, 0);
}
#ifdef __cplusplus
}
#endif
void CFX_MemoryMgr::Init(FXMEM_SystemMgr* pSystemMgr)
{
    m_pSystemMgr = pSystemMgr;
    IFX_Allocator &ac = m_DefAllocator.m_Allocator;
    ac.m_Alloc = _DefAlloc;
    ac.m_AllocDebug = _DefAllocDebug;
    ac.m_Realloc = _DefRealloc;
    ac.m_ReallocDebug = _DefReallocDebug;
    ac.m_Free = _DefFree;
    m_DefAllocator.m_pFoxitMgr = this;
    m_pExternalMemory = NULL;
    m_bReleaseMgr = TRUE;
}
void CFX_MemoryMgr::PurgeMgr()
{
    if (m_pSystemMgr->Purge) {
        m_pSystemMgr->Purge(m_pSystemMgr);
    }
}
void* CFX_MemoryMgr::Alloc(size_t size, int flags)
{
    void* p = m_pSystemMgr->Alloc(m_pSystemMgr, size, flags);
    if (p == NULL) {
        return NULL;
    }
    return p;
}
void* CFX_MemoryMgr::AllocDebug(size_t size, int flags, FX_LPCSTR file, int line)
{
    void* p = m_pSystemMgr->AllocDebug(m_pSystemMgr, size, flags, file, line);
    if (p == NULL) {
        return NULL;
    }
    return p;
}
void* CFX_MemoryMgr::Realloc(void* p, size_t size, int flags)
{
    void* p1 = m_pSystemMgr->Realloc(m_pSystemMgr, p, size, flags);
    if (p1 == NULL) {
        return NULL;
    }
    return p1;
}
void* CFX_MemoryMgr::ReallocDebug(void* p, size_t size, int flags, FX_LPCSTR file, int line)
{
    void* p1 = m_pSystemMgr->ReallocDebug(m_pSystemMgr, p, size, flags, file, line);
    if (p1 == NULL) {
        return NULL;
    }
    return p1;
}
void CFX_MemoryMgr::Free(void* p, int flags)
{
    if (p == NULL) {
        return;
    }
    m_pSystemMgr->Free(m_pSystemMgr, p, flags);
}
CFX_MemoryMgr* g_pDefFoxitMgr = NULL;
void* FXMEM_DefaultAlloc(size_t size, int flags)
{
    return g_pDefFoxitMgr->Alloc(size, flags);
}
void* FXMEM_DefaultAlloc2(size_t size, size_t unit, int flags)
{
    return g_pDefFoxitMgr->Alloc(size * unit, flags);
}
void* FXMEM_DefaultRealloc(void* p, size_t size, int flags)
{
    if (p == NULL) {
        return FXMEM_DefaultAlloc(size, flags);
    }
    return g_pDefFoxitMgr->Realloc(p, size, flags);
}
void* FXMEM_DefaultRealloc2(void* p, size_t size, size_t unit, int flags)
{
    if (p == NULL) {
        return FXMEM_DefaultAlloc2(size, unit, flags);
    }
    return g_pDefFoxitMgr->Realloc(p, size * unit, flags);
}
void* FXMEM_DefaultAllocDebug(size_t size, int flags, FX_LPCSTR file, int line)
{
    return g_pDefFoxitMgr->AllocDebug(size, flags, file, line);
}
void* FXMEM_DefaultAllocDebug2(size_t size, size_t unit, int flags, FX_LPCSTR file, int line)
{
    return g_pDefFoxitMgr->AllocDebug(size * unit, flags, file, line);
}
void* FXMEM_DefaultReallocDebug(void* p, size_t size, int flags, FX_LPCSTR file, int line)
{
    if (p == NULL) {
        return FXMEM_DefaultAllocDebug(size, flags, file, line);
    }
    return g_pDefFoxitMgr->ReallocDebug(p, size, flags, file, line);
}
void* FXMEM_DefaultReallocDebug2(void* p, size_t size, size_t unit, int flags, FX_LPCSTR file, int line)
{
    if (p == NULL) {
        return FXMEM_DefaultAllocDebug2(size, unit, flags, file, line);
    }
    return g_pDefFoxitMgr->ReallocDebug(p, size * unit, flags, file, line);
}
void FXMEM_DefaultFree(void* p, int flags)
{
    g_pDefFoxitMgr->Free(p, flags);
}
IFX_Allocator* FXMEM_GetDefAllocator()
{
    return &g_pDefFoxitMgr->m_DefAllocator.m_Allocator;
}
void* CFX_Object::operator new(size_t size)
{
    return g_pDefFoxitMgr->Alloc(size, 0);
}
void* CFX_Object::operator new[](size_t size)
{
    return g_pDefFoxitMgr->Alloc(size, 0);
}
void* CFX_Object::operator new[](size_t size, FX_LPCSTR file, int line)
{
    return g_pDefFoxitMgr->AllocDebug(size, 0, file, line);
}
void* CFX_Object::operator new(size_t size, FX_LPCSTR file, int line)
{
    return g_pDefFoxitMgr->AllocDebug(size, 0, file, line);
}
void CFX_Object::operator delete(void* p)
{
    g_pDefFoxitMgr->Free(p, 0);
}
void CFX_Object::operator delete[](void* p)
{
    g_pDefFoxitMgr->Free(p, 0);
}
void CFX_Object::operator delete(void* p, FX_LPCSTR file, int line)
{
    g_pDefFoxitMgr->Free(p, 0);
}
void CFX_Object::operator delete[](void* p, FX_LPCSTR file, int line)
{
    g_pDefFoxitMgr->Free(p, 0);
}
void* CFX_AllocObject::operator new(size_t size, IFX_Allocator* pAllocator, FX_LPCSTR filename, int line)
{
    void* p = pAllocator ? pAllocator->m_AllocDebug(pAllocator, size, 1, filename, line) :
              g_pDefFoxitMgr->AllocDebug(size, 0, filename, line);
    ((CFX_AllocObject*)p)->m_pAllocator = pAllocator;
    return p;
}
void CFX_AllocObject::operator delete (void* p, IFX_Allocator* pAllocator, FX_LPCSTR filename, int line)
{
    if (pAllocator) {
        pAllocator->m_Free(pAllocator, p);
    } else {
        g_pDefFoxitMgr->Free(p, 0);
    }
}
void* CFX_AllocObject::operator new(size_t size, IFX_Allocator* pAllocator)
{
    void* p = pAllocator ? pAllocator->m_Alloc(pAllocator, size, 1) : g_pDefFoxitMgr->Alloc(size, 0);
    ((CFX_AllocObject*)p)->m_pAllocator = pAllocator;
    return p;
}
void CFX_AllocObject::operator delete(void* p)
{
    if (((CFX_AllocObject*)p)->m_pAllocator) {
        (((CFX_AllocObject*)p)->m_pAllocator)->m_Free(((CFX_AllocObject*)p)->m_pAllocator, p);
    } else {
        g_pDefFoxitMgr->Free(p, 0);
    }
}
void CFX_AllocObject::operator delete(void* p, IFX_Allocator* pAllocator)
{
    if (pAllocator) {
        pAllocator->m_Free(pAllocator, p);
    } else {
        g_pDefFoxitMgr->Free(p, 0);
    }
}
extern "C" {
    static void* _GOPAllocDebug(IFX_Allocator* pAllocator, size_t num, size_t size, FX_LPCSTR file, int line)
    {
        if (size == 0 || num > SIZE_MAX/size)
            return NULL;

        size = size * num;
        return ((CFX_GrowOnlyPool*)pAllocator)->Alloc(size);
    }
    static void* _GOPAlloc(IFX_Allocator* pAllocator, size_t num, size_t size)
    {
        if (size == 0 || num > SIZE_MAX/size)
            return NULL;

        size = size * num;
        return ((CFX_GrowOnlyPool*)pAllocator)->Alloc(size);
    }
    static void* _GOPReallocDebug(IFX_Allocator* pAllocator, void* p, size_t new_num, size_t size, FX_LPCSTR file, int line)
    {
        if (size == 0 || new_num > SIZE_MAX/size)
            return NULL;

        size = size * new_num;
        return ((CFX_GrowOnlyPool*)pAllocator)->Realloc(p, size);
    }
    static void* _GOPRealloc(IFX_Allocator* pAllocator, void* p, size_t new_num, size_t size)
    {
        if (size == 0 || new_num > SIZE_MAX/size)
            return NULL;

        size = size * new_num;
        return ((CFX_GrowOnlyPool*)pAllocator)->Realloc(p, size);
    }
    static void _GOPFree(IFX_Allocator* pAllocator, void* p)
    {
    }
};
CFX_GrowOnlyPool::CFX_GrowOnlyPool(IFX_Allocator* pAllocator, size_t trunk_size)
{
    m_TrunkSize = trunk_size;
    m_pFirstTrunk = NULL;
    m_pAllocator = pAllocator ? pAllocator : &g_pDefFoxitMgr->m_DefAllocator.m_Allocator;
    m_AllocDebug = _GOPAllocDebug;
    m_Alloc = _GOPAlloc;
    m_ReallocDebug = _GOPReallocDebug;
    m_Realloc = _GOPRealloc;
    m_Free = _GOPFree;
}
CFX_GrowOnlyPool::~CFX_GrowOnlyPool()
{
    FreeAll();
}
void CFX_GrowOnlyPool::SetAllocator(IFX_Allocator* pAllocator)
{
    ASSERT(m_pFirstTrunk == NULL);
    m_pAllocator = pAllocator ? pAllocator : &g_pDefFoxitMgr->m_DefAllocator.m_Allocator;
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
        m_pAllocator->m_Free(m_pAllocator, pTrunk);
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
    
    if (alloc_size > SIZE_MAX - sizeof(_FX_GrowOnlyTrunk) )
        return NULL;

    pTrunk = (_FX_GrowOnlyTrunk*)m_pAllocator->m_Alloc(m_pAllocator, sizeof(_FX_GrowOnlyTrunk) + alloc_size, 1);
    pTrunk->m_Size = alloc_size;
    pTrunk->m_Allocated = size;
    pTrunk->m_pNext = (_FX_GrowOnlyTrunk*)m_pFirstTrunk;
    m_pFirstTrunk = pTrunk;
    return pTrunk + 1;
}
