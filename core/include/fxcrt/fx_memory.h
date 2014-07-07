// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_MEMORY_H_
#define _FX_MEMORY_H_
#ifndef _FX_SYSTEM_H_
#include "fx_system.h"
#endif
#define FXMEM_NONLEAVE			1
#define FXMEM_MOVABLE			2
#define FXMEM_DISCARDABLE		4
#ifdef __cplusplus
extern "C" {
#endif
typedef struct _FXMEM_SystemMgr {

    void* (*Alloc)(struct _FXMEM_SystemMgr* pMgr, size_t size, int flags);

    void* (*AllocDebug)(struct _FXMEM_SystemMgr* pMgr, size_t size, int flags, FX_LPCSTR file, int line);

    void* (*Realloc)(struct _FXMEM_SystemMgr* pMgr, void* pointer, size_t size, int flags);

    void* (*ReallocDebug)(struct _FXMEM_SystemMgr* pMgr, void* pointer, size_t size, int flags, FX_LPCSTR file, int line);

    void* (*Lock)(struct _FXMEM_SystemMgr* pMgr, void* handle);

    void  (*Unlock)(struct _FXMEM_SystemMgr* pMgr, void* handle);

    void  (*Free)(struct _FXMEM_SystemMgr* pMgr, void* pointer, int flags);

    void  (*Purge)(struct _FXMEM_SystemMgr* pMgr);

    void  (*CollectAll)(struct _FXMEM_SystemMgr* pMgr);


    void* user;
} FXMEM_SystemMgr;
FX_DEFINEHANDLE(FXMEM_FoxitMgr)
typedef struct _FXMEM_SystemMgr2 {

    FX_BOOL	(*More)(struct _FXMEM_SystemMgr2* pMgr, size_t alloc_size, void** new_memory, size_t* new_size);

    void	(*Free)(struct _FXMEM_SystemMgr2* pMgr, void* memory);
} FXMEM_SystemMgr2;
FXMEM_FoxitMgr* FXMEM_CreateMemoryMgr(size_t size, FX_BOOL extensible);
void	FXMEM_DestroyFoxitMgr(FXMEM_FoxitMgr* pFoxitMgr);
void*	FXMEM_DefaultAlloc(size_t byte_size, int flags);
void*	FXMEM_DefaultAlloc2(size_t units, size_t unit_size, int flags);
void*	FXMEM_DefaultRealloc(void* pointer, size_t new_size, int flags);
void*	FXMEM_DefaultRealloc2(void* pointer, size_t units, size_t unit_size, int flags);
void	FXMEM_DefaultFree(void* pointer, int flags);
#define FX_Alloc(type, size)			(type*)FXMEM_DefaultAlloc2(size, sizeof(type), 0)
#define FX_Realloc(type, ptr, size)		(type*)FXMEM_DefaultRealloc2(ptr, size, sizeof(type), 0)
#define FX_AllocNL(type, size)			(type*)FXMEM_DefaultAlloc2(size, sizeof(type), FXMEM_NONLEAVE)
#define FX_ReallocNL(type, ptr, size)	(type*)FXMEM_DefaultRealloc2(ptr, size, sizeof(type), FXMEM_NONLEAVE)
#define FX_Free(pointer) FXMEM_DefaultFree(pointer, 0)
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
#if defined(_DEBUG)
#define FX_NEW new(__FILE__, __LINE__)
#else

#define FX_NEW new
#endif
class CFX_Object
{
public:

    void*			operator new (size_t size, FX_LPCSTR file, int line);

    void			operator delete (void* p, FX_LPCSTR file, int line);

    void*			operator new (size_t size);

    void			operator delete (void* p);

    void*			operator new[] (size_t size, FX_LPCSTR file, int line);

    void			operator delete[] (void* p, FX_LPCSTR file, int line);

    void*			operator new[] (size_t size);

    void			operator delete[] (void* p);

    void*			operator new (size_t, void* buf)
    {
        return buf;
    }

    void			operator delete (void*, void*) {}
};
#define FX_NEW_VECTOR(Pointer, Class, Count) \
    { \
        Pointer = FX_Alloc(Class, Count); \
        if (Pointer) { \
            for (int i = 0; i < (Count); i ++) new (Pointer + i) Class; \
        } \
    }
#define FX_DELETE_VECTOR(Pointer, Class, Count) \
    { \
        for (int i = 0; i < (Count); i ++) Pointer[i].~Class(); \
        FX_Free(Pointer); \
    }
class CFX_DestructObject : public CFX_Object
{
public:

    virtual ~CFX_DestructObject() {}
};
#ifdef __cplusplus
extern "C" {
#endif
typedef struct _IFX_Allocator {

    void*	(*m_AllocDebug)(struct _IFX_Allocator* pAllocator, size_t num, size_t size, FX_LPCSTR file, int line);

    void*	(*m_Alloc)(struct _IFX_Allocator* pAllocator, size_t num, size_t size);

    void*	(*m_ReallocDebug)(struct _IFX_Allocator* pAllocator, void* p, size_t num, size_t size, FX_LPCSTR file, int line);

    void*	(*m_Realloc)(struct _IFX_Allocator* pAllocator, void* p, size_t num, size_t size);

    void	(*m_Free)(struct _IFX_Allocator* pAllocator, void* p);
} IFX_Allocator;
IFX_Allocator* FXMEM_GetDefAllocator();
#ifdef __cplusplus
}
#endif
#ifdef _DEBUG

#define FX_Allocator_Alloc(fxAllocator, type, size) \
    ((fxAllocator) ? (type*)(fxAllocator)->m_AllocDebug((fxAllocator), (size), sizeof(type), __FILE__, __LINE__) : (FX_Alloc(type, size)))

#define FX_Allocator_Realloc(fxAllocator, type, ptr, new_size) \
    ((fxAllocator) ? (type*)(fxAllocator)->m_ReallocDebug((fxAllocator), (ptr), (new_size) , sizeof(type), __FILE__, __LINE__) : (FX_Realloc(type, ptr, new_size)))
#else

#define FX_Allocator_Alloc(fxAllocator, type, size) \
    ((fxAllocator) ? (type*)(fxAllocator)->m_Alloc((fxAllocator), (size), sizeof(type)) : (FX_Alloc(type, size)))

#define FX_Allocator_Realloc(fxAllocator, type, ptr, new_size) \
    ((fxAllocator) ? (type*)(fxAllocator)->m_Realloc((fxAllocator), (ptr), (new_size), sizeof(type)) : (FX_Realloc(type, ptr, new_size)))
#endif
#define FX_Allocator_Free(fxAllocator, ptr) \
    ((fxAllocator) ? (fxAllocator)->m_Free((fxAllocator), (ptr)) : (FX_Free(ptr)))
inline void* operator new(size_t size, IFX_Allocator* fxAllocator)
{
    return (void*)FX_Allocator_Alloc(fxAllocator, FX_BYTE, size);
}
inline void operator delete(void* ptr, IFX_Allocator* fxAllocator)
{
}
#define FX_NewAtAllocator(fxAllocator) \
    ::new(fxAllocator)
#define FX_DeleteAtAllocator(pointer, fxAllocator, __class__) \
    (pointer)->~__class__(); \
    FX_Allocator_Free(fxAllocator, pointer)
class CFX_AllocObject
{
public:

    void*			operator new (size_t size, IFX_Allocator* pAllocator, FX_LPCSTR file, int line);
#ifndef _FX_NO_EXCEPTION_

    void			operator delete (void* p, IFX_Allocator* pAllocator, FX_LPCSTR file, int line);
#endif

    void*			operator new (size_t size, IFX_Allocator* pAllocator);

    void			operator delete (void* p);
#ifndef _FX_NO_EXCEPTION_

    void			operator delete (void* p, IFX_Allocator* pAllocator);
#endif

    void*			operator new (size_t, void* buf)
    {
        return buf;
    }
#ifndef _FX_NO_EXCEPTION_

    void			operator delete (void*, void*) {}
#endif

    IFX_Allocator*	GetAllocator() const
    {
        return m_pAllocator;
    }
private:

    void*			operator new[] (size_t size, IFX_Allocator* pAllocator, FX_LPCSTR file, int line)
    {
        return operator new(size, pAllocator, file, line);
    }
#ifndef _FX_NO_EXCEPTION_

    void			operator delete[] (void* p, IFX_Allocator* pAllocator, FX_LPCSTR file, int line) {}
#endif

    void*			operator new[] (size_t size, IFX_Allocator* pAllocator)
    {
        return operator new(size, pAllocator);
    }

    void			operator delete[] (void* p) {}
#ifndef _FX_NO_EXCEPTION_

    void			operator delete[] (void* p, IFX_Allocator* pAllocator) {}
#endif
protected:

    IFX_Allocator*	m_pAllocator;
};
#if defined(_DEBUG)
#define FX_NEWAT(pAllocator) new(pAllocator, __FILE__, __LINE__)
#else

#define FX_NEWAT(pAllocator) new(pAllocator)
#endif
class CFX_GrowOnlyPool : public IFX_Allocator, public CFX_Object
{
public:

    CFX_GrowOnlyPool(IFX_Allocator* pAllocator = NULL, size_t trunk_size = 16384);

    ~CFX_GrowOnlyPool();

    void	SetAllocator(IFX_Allocator* pAllocator);

    void	SetTrunkSize(size_t trunk_size)
    {
        m_TrunkSize = trunk_size;
    }

    void*	AllocDebug(size_t size, FX_LPCSTR file, int line)
    {
        return Alloc(size);
    }

    void*	Alloc(size_t size);

    void*	ReallocDebug(void* p, size_t new_size, FX_LPCSTR file, int line)
    {
        return NULL;
    }

    void*	Realloc(void* p, size_t new_size)
    {
        return NULL;
    }

    void	Free(void*) {}

    void	FreeAll();
private:

    size_t	m_TrunkSize;

    void*	m_pFirstTrunk;

    IFX_Allocator*	m_pAllocator;
};
#endif
#ifdef __cplusplus
extern "C" {
#endif
#define FX_FIXEDMEM_PAGESIZE		(4096 * 16)
#define FX_FIXEDMEM_MIDBLOCKSIZE	(4096)
typedef struct _FX_MEMCONFIG {

    size_t	nPageNum_Init8;

    size_t	nPageNum_Init16;

    size_t	nPageNum_Init32;

    size_t	nPageNum_More16;

    size_t	nPageNum_More32;

    size_t	nPageSize_Mid;

    size_t	nPageNum_InitMid;

    size_t	nPageNum_MoreMid;

    size_t	nPageSize_Large;

    size_t	nPageSize_Alone;
} FX_MEMCONFIG;
void	FXMEM_SetConfig(const FX_MEMCONFIG* memConfig);
#ifdef __cplusplus
}
#endif
#endif
