// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_MEMORY
#define _FX_MEMORY

#include "core/include/fxcrt/fx_memory.h"  // For FX_Alloc().

class IFX_MEMAllocator;
class CFX_Target;
enum FX_ALLOCTYPE {
  FX_ALLOCTYPE_Default = 0,
  FX_ALLOCTYPE_Static,
  FX_ALLOCTYPE_Fixed,
  FX_ALLOCTYPE_Dynamic,
};

class IFX_MEMAllocator {
 public:
  virtual ~IFX_MEMAllocator() {}
  virtual void Release() = 0;
  virtual void* Alloc(size_t size) = 0;
  virtual void Free(void* pBlock) = 0;
  virtual size_t GetBlockSize() const = 0;
  virtual size_t GetDefChunkSize() const = 0;
  virtual size_t SetDefChunkSize(size_t size) = 0;
  virtual size_t GetCurrentDataSize() const = 0;
};

IFX_MEMAllocator* FX_CreateAllocator(FX_ALLOCTYPE eType,
                                     size_t chunkSize,
                                     size_t blockSize);
class CFX_Target {
 public:
  virtual ~CFX_Target() {}
  void* operator new(size_t size) { return FX_Alloc(uint8_t, size); }
  void operator delete(void* p) { FX_Free(p); }
  void* operator new(size_t size, IFX_MEMAllocator* pAllocator) {
    return pAllocator->Alloc(size);
  }
  void operator delete(void* p, IFX_MEMAllocator* pAllocator) {
    pAllocator->Free(p);
  }
  void* operator new(size_t size, void* place) { return place; }
  void operator delete(void* p, void* place) {}
};
#define FXTARGET_NewWith(__allocator__) new (__allocator__)
#define FXTARGET_DeleteWith(__class__, __allocator__, pointer) \
  {                                                            \
    (pointer)->~__class__();                                   \
    (pointer)->operator delete((pointer), (__allocator__));    \
  }
#endif
