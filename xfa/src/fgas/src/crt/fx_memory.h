// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_MEMORY_IMP
#define _FX_MEMORY_IMP
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
typedef struct _FX_STATICSTORECHUNK {
  _FX_STATICSTORECHUNK* pNextChunk;
  size_t iChunkSize;
  size_t iFreeSize;
} FX_STATICSTORECHUNK, *FX_LPSTATICSTORECHUNK;
typedef FX_STATICSTORECHUNK const* FX_LPCSTATICSTORECHUNK;
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
  FX_LPSTATICSTORECHUNK m_pChunk;
  FX_LPSTATICSTORECHUNK m_pLastChunk;
  FX_LPSTATICSTORECHUNK AllocChunk(size_t size);
  FX_LPSTATICSTORECHUNK FindChunk(size_t size);
};
#if _FX_OS_ != _FX_ANDROID_
#pragma pack(push, 1)
#endif
typedef struct _FX_FIXEDSTORECHUNK {
  uint8_t* FirstFlag() const {
    return (uint8_t*)this + sizeof(_FX_FIXEDSTORECHUNK);
  }
  uint8_t* FirstBlock() const { return FirstFlag() + iChunkSize; }
  _FX_FIXEDSTORECHUNK* pNextChunk;
  size_t iChunkSize;
  size_t iFreeNum;
} FX_FIXEDSTORECHUNK, *FX_LPFIXEDSTORECHUNK;
typedef FX_FIXEDSTORECHUNK const* FX_LPCFIXEDSTORECHUNK;
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
  size_t m_iBlockSize;
  size_t m_iDefChunkSize;
  FX_LPFIXEDSTORECHUNK m_pChunk;
  FX_LPFIXEDSTORECHUNK AllocChunk();
};
#if _FX_OS_ != _FX_ANDROID_
#pragma pack(push, 1)
#endif
typedef struct _FX_DYNAMICSTOREBLOCK {
  _FX_DYNAMICSTOREBLOCK* NextBlock() const {
    return (_FX_DYNAMICSTOREBLOCK*)(Data() + iBlockSize);
  }
  uint8_t* Data() const {
    return (uint8_t*)this + sizeof(_FX_DYNAMICSTOREBLOCK);
  }
  size_t iBlockSize;
  FX_BOOL bUsed;
} FX_DYNAMICSTOREBLOCK, *FX_LPDYNAMICSTOREBLOCK;
typedef FX_DYNAMICSTOREBLOCK const* FX_LPCDYNAMICSTOREBLOCK;
typedef struct _FX_DYNAMICSTORECHUNK {
  FX_LPDYNAMICSTOREBLOCK FirstBlock() const {
    return (FX_LPDYNAMICSTOREBLOCK)((uint8_t*)this +
                                    sizeof(_FX_DYNAMICSTORECHUNK));
  }
  _FX_DYNAMICSTORECHUNK* pNextChunk;
  size_t iChunkSize;
  size_t iFreeSize;
} FX_DYNAMICSTORECHUNK, *FX_LPDYNAMICSTORECHUNK;
typedef FX_DYNAMICSTORECHUNK const* FX_LPCDYNAMICSTORECHUNK;
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
  size_t m_iDefChunkSize;
  FX_LPDYNAMICSTORECHUNK m_pChunk;
  FX_LPDYNAMICSTORECHUNK AllocChunk(size_t size);
};
#endif
