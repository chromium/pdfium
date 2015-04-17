// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_MEMORY_H_
#define _FX_MEMORY_H_

#include "fx_system.h"

#ifdef __cplusplus
#include <new>
extern "C" {
#endif
#define FX_Alloc(type, size)						(type*)calloc(size, sizeof(type))
#define FX_Realloc(type, ptr, size)					(type*)realloc(ptr, sizeof(type) * (size))
#define FX_AllocNL(type, size)						FX_Alloc(type, size)
#define FX_ReallocNL(type, ptr, size)				FX_Realloc(type, ptr, size)
#define FX_Free(ptr)								free(ptr)
void*	FXMEM_DefaultAlloc(size_t byte_size, int flags);
void*	FXMEM_DefaultRealloc(void* pointer, size_t new_size, int flags);
void	FXMEM_DefaultFree(void* pointer, int flags);
#ifdef __cplusplus
}

#define FX_NEW new
#define FX_NEW_VECTOR(Pointer, Class, Count) (Pointer = new Class[Count])
#define FX_DELETE_VECTOR(Pointer, Class, Count) delete[] Pointer

class CFX_DestructObject 
{
public:

    virtual ~CFX_DestructObject() {}
};
class CFX_GrowOnlyPool 
{
public:

    CFX_GrowOnlyPool(size_t trunk_size = 16384);

    ~CFX_GrowOnlyPool();

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
};
#endif
#endif
