// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FGAS_SRC_CRT_FX_MEMORY_H_
#define XFA_SRC_FGAS_SRC_CRT_FX_MEMORY_H_

#include "xfa/src/fgas/include/fx_mem.h"

class CFX_DefStore;
class CFX_StaticStore;
class CFX_FixedStore;
class CFX_DynamicStore;
class CFX_DefStore : public IFX_MEMAllocator, public CFX_Target {
 public:
  CFX_DefStore() {}
  ~CFX_DefStore() {}
  virtual void Release() { delete this; }
  virtual void* Alloc(size_t size) { return FX_Alloc(uint8_t, size); }
  virtual void Free(void* pBlock) { FX_Free(pBlock); }
  virtual size_t GetBlockSize() const { return 0; }
  virtual size_t GetDefChunkSize() const { return 0; }
  virtual size_t SetDefChunkSize(size_t size) { return 0; }
  virtual size_t GetCurrentDataSize() const { return 0; }
};

#if _FX_OS_ != _FX_ANDROID_
#pragma pack(push, 1)
#endif
struct FX_STATICSTORECHUNK {
  FX_STATICSTORECHUNK* pNextChunk;
  size_t iChunkSize;
  size_t iFreeSize;
};
#if _FX_OS_ != _FX_ANDROID_
#pragma pack(pop)
#endif

class CFX_StaticStore : public IFX_MEMAllocator, public CFX_Target {
 public:
  CFX_StaticStore(size_t iDefChunkSize = 4096);
  ~CFX_StaticStore();
  virtual void Release() { delete this; }
  virtual void* Alloc(size_t size);
  virtual void Free(void* pBlock) {}
  virtual size_t GetBlockSize() const { return 0; }
  virtual size_t GetDefChunkSize() const { return m_iDefChunkSize; }
  virtual size_t SetDefChunkSize(size_t size);
  virtual size_t GetCurrentDataSize() const { return m_iAllocatedSize; }

 protected:
  size_t m_iAllocatedSize;
  size_t m_iDefChunkSize;
  FX_STATICSTORECHUNK* m_pChunk;
  FX_STATICSTORECHUNK* m_pLastChunk;
  FX_STATICSTORECHUNK* AllocChunk(size_t size);
  FX_STATICSTORECHUNK* FindChunk(size_t size);
};

#if _FX_OS_ != _FX_ANDROID_
#pragma pack(push, 1)
#endif
struct FX_FIXEDSTORECHUNK {
  uint8_t* FirstFlag() { return reinterpret_cast<uint8_t*>(this + 1); }
  uint8_t* FirstBlock() { return FirstFlag() + iChunkSize; }

  FX_FIXEDSTORECHUNK* pNextChunk;
  size_t iChunkSize;
  size_t iFreeNum;
};
#if _FX_OS_ != _FX_ANDROID_
#pragma pack(pop)
#endif

class CFX_FixedStore : public IFX_MEMAllocator, public CFX_Target {
 public:
  CFX_FixedStore(size_t iBlockSize, size_t iBlockNumsInChunk);
  virtual ~CFX_FixedStore();
  virtual void Release() { delete this; }
  virtual void* Alloc(size_t size);
  virtual void Free(void* pBlock);
  virtual size_t GetBlockSize() const { return m_iBlockSize; }
  virtual size_t GetDefChunkSize() const { return m_iDefChunkSize; }
  virtual size_t SetDefChunkSize(size_t iChunkSize);
  virtual size_t GetCurrentDataSize() const { return 0; }

 protected:
  FX_FIXEDSTORECHUNK* AllocChunk();

  size_t m_iBlockSize;
  size_t m_iDefChunkSize;
  FX_FIXEDSTORECHUNK* m_pChunk;
};

#if _FX_OS_ != _FX_ANDROID_
#pragma pack(push, 1)
#endif
struct FX_DYNAMICSTOREBLOCK {
  uint8_t* Data() { return reinterpret_cast<uint8_t*>(this + 1); }
  FX_DYNAMICSTOREBLOCK* NextBlock() {
    return reinterpret_cast<FX_DYNAMICSTOREBLOCK*>(Data() + iBlockSize);
  }
  size_t iBlockSize;
  FX_BOOL bUsed;
};

struct FX_DYNAMICSTORECHUNK {
  FX_DYNAMICSTOREBLOCK* FirstBlock() {
    return reinterpret_cast<FX_DYNAMICSTOREBLOCK*>(this + 1);
  }
  FX_DYNAMICSTORECHUNK* pNextChunk;
  size_t iChunkSize;
  size_t iFreeSize;
};
#if _FX_OS_ != _FX_ANDROID_
#pragma pack(pop)
#endif

class CFX_DynamicStore : public IFX_MEMAllocator, public CFX_Target {
 public:
  CFX_DynamicStore(size_t iDefChunkSize = 4096);
  virtual ~CFX_DynamicStore();
  virtual void Release() { delete this; }
  virtual void* Alloc(size_t size);
  virtual void Free(void* pBlock);
  virtual size_t GetBlockSize() const { return 0; }
  virtual size_t GetDefChunkSize() const { return m_iDefChunkSize; }
  virtual size_t SetDefChunkSize(size_t size);
  virtual size_t GetCurrentDataSize() const { return 0; }

 protected:
  FX_DYNAMICSTORECHUNK* AllocChunk(size_t size);

  size_t m_iDefChunkSize;
  FX_DYNAMICSTORECHUNK* m_pChunk;
};

#endif  // XFA_SRC_FGAS_SRC_CRT_FX_MEMORY_H_
