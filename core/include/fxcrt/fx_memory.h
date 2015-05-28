// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_MEMORY_H_
#define CORE_INCLUDE_FXCRT_FX_MEMORY_H_

#include "fx_system.h"

#ifdef __cplusplus
extern "C" {
#endif
// For external C libraries to malloc through PDFium. These may return NULL.
void*	FXMEM_DefaultAlloc(size_t byte_size, int flags);
void*	FXMEM_DefaultRealloc(void* pointer, size_t new_size, int flags);
void	FXMEM_DefaultFree(void* pointer, int flags);
#ifdef __cplusplus
}  // extern "C"

#include <stdlib.h>
#include <limits>
#include <new>

NEVER_INLINE void FX_OutOfMemoryTerminate();

inline void* FX_SafeRealloc(void* ptr, size_t num_members, size_t member_size) {
    if (num_members < std::numeric_limits<size_t>::max() / member_size) {
        return realloc(ptr, num_members * member_size);
    }
    return nullptr;
}

inline void* FX_AllocOrDie(size_t num_members, size_t member_size) {
    // TODO(tsepez): See if we can avoid the implicit memset(0).
    if (void* result = calloc(num_members, member_size)) {
        return result;
    }
    FX_OutOfMemoryTerminate();  // Never returns.
    return nullptr;  // Suppress compiler warning.
}

inline void* FX_AllocOrDie2D(size_t w, size_t h, size_t member_size) {
    if (w < std::numeric_limits<size_t>::max() / h) {
        return FX_AllocOrDie(w * h, member_size);
    }
    FX_OutOfMemoryTerminate();  // Never returns.
    return nullptr;  // Suppress compiler warning.
}

inline void* FX_ReallocOrDie(void* ptr, size_t num_members, size_t member_size) {
    if (void* result = FX_SafeRealloc(ptr, num_members, member_size)) {
        return result;
    }
    FX_OutOfMemoryTerminate();  // Never returns.
    return nullptr;  // Suppress compiler warning.
}

// Never returns NULL.
#define FX_Alloc(type, size) (type*)FX_AllocOrDie(size, sizeof(type))
#define FX_Alloc2D(type, w, h) (type*)FX_AllocOrDie2D(w, h, sizeof(type))
#define FX_Realloc(type, ptr, size) \
    (type*)FX_ReallocOrDie(ptr, size, sizeof(type))

// May return NULL.
#define FX_TryAlloc(type, size) (type*)calloc(size, sizeof(type))
#define FX_TryRealloc(type, ptr, size) \
    (type*)FX_SafeRealloc(ptr, size, sizeof(type))

#define FX_Free(ptr) free(ptr)

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
#endif  // __cplusplus

#endif  // CORE_INCLUDE_FXCRT_FX_MEMORY_H_
