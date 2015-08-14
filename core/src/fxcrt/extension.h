// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCRT_EXTENSION_H_
#define CORE_SRC_FXCRT_EXTENSION_H_

#include "../../include/fxcrt/fx_basic.h"
#include "../../include/fxcrt/fx_safe_types.h"

class IFXCRT_FileAccess {
 public:
  virtual ~IFXCRT_FileAccess() {}
  virtual FX_BOOL Open(const CFX_ByteStringC& fileName, FX_DWORD dwMode) = 0;
  virtual FX_BOOL Open(const CFX_WideStringC& fileName, FX_DWORD dwMode) = 0;
  virtual void Close() = 0;
  virtual void Release() = 0;
  virtual FX_FILESIZE GetSize() const = 0;
  virtual FX_FILESIZE GetPosition() const = 0;
  virtual FX_FILESIZE SetPosition(FX_FILESIZE pos) = 0;
  virtual size_t Read(void* pBuffer, size_t szBuffer) = 0;
  virtual size_t Write(const void* pBuffer, size_t szBuffer) = 0;
  virtual size_t ReadPos(void* pBuffer, size_t szBuffer, FX_FILESIZE pos) = 0;
  virtual size_t WritePos(const void* pBuffer,
                          size_t szBuffer,
                          FX_FILESIZE pos) = 0;
  virtual FX_BOOL Flush() = 0;
  virtual FX_BOOL Truncate(FX_FILESIZE szFile) = 0;
};
IFXCRT_FileAccess* FXCRT_FileAccess_Create();

class CFX_CRTFileStream final : public IFX_FileStream {
 public:
  CFX_CRTFileStream(IFXCRT_FileAccess* pFA) : m_pFile(pFA), m_dwCount(1) {}
  ~CFX_CRTFileStream() override {
    if (m_pFile) {
      m_pFile->Release();
    }
  }
  virtual IFX_FileStream* Retain() override {
    m_dwCount++;
    return this;
  }
  virtual void Release() override {
    FX_DWORD nCount = --m_dwCount;
    if (!nCount) {
      delete this;
    }
  }
  virtual FX_FILESIZE GetSize() override { return m_pFile->GetSize(); }
  virtual FX_BOOL IsEOF() override { return GetPosition() >= GetSize(); }
  virtual FX_FILESIZE GetPosition() override { return m_pFile->GetPosition(); }
  virtual FX_BOOL ReadBlock(void* buffer,
                            FX_FILESIZE offset,
                            size_t size) override {
    return (FX_BOOL)m_pFile->ReadPos(buffer, size, offset);
  }
  virtual size_t ReadBlock(void* buffer, size_t size) override {
    return m_pFile->Read(buffer, size);
  }
  virtual FX_BOOL WriteBlock(const void* buffer,
                             FX_FILESIZE offset,
                             size_t size) override {
    return (FX_BOOL)m_pFile->WritePos(buffer, size, offset);
  }
  virtual FX_BOOL Flush() override { return m_pFile->Flush(); }

 protected:
  IFXCRT_FileAccess* m_pFile;
  FX_DWORD m_dwCount;
};

#define FX_MEMSTREAM_BlockSize (64 * 1024)
#define FX_MEMSTREAM_Consecutive 0x01
#define FX_MEMSTREAM_TakeOver 0x02
class CFX_MemoryStream final : public IFX_MemoryStream {
 public:
  CFX_MemoryStream(FX_BOOL bConsecutive)
      : m_dwCount(1),
        m_nTotalSize(0),
        m_nCurSize(0),
        m_nCurPos(0),
        m_nGrowSize(FX_MEMSTREAM_BlockSize) {
    m_dwFlags =
        FX_MEMSTREAM_TakeOver | (bConsecutive ? FX_MEMSTREAM_Consecutive : 0);
  }
  CFX_MemoryStream(uint8_t* pBuffer, size_t nSize, FX_BOOL bTakeOver)
      : m_dwCount(1),
        m_nTotalSize(nSize),
        m_nCurSize(nSize),
        m_nCurPos(0),
        m_nGrowSize(FX_MEMSTREAM_BlockSize) {
    m_Blocks.Add(pBuffer);
    m_dwFlags =
        FX_MEMSTREAM_Consecutive | (bTakeOver ? FX_MEMSTREAM_TakeOver : 0);
  }
  ~CFX_MemoryStream() override {
    if (m_dwFlags & FX_MEMSTREAM_TakeOver) {
      for (int32_t i = 0; i < m_Blocks.GetSize(); i++) {
        FX_Free((uint8_t*)m_Blocks[i]);
      }
    }
    m_Blocks.RemoveAll();
  }
  virtual IFX_FileStream* Retain() override {
    m_dwCount++;
    return this;
  }
  virtual void Release() override {
    FX_DWORD nCount = --m_dwCount;
    if (nCount) {
      return;
    }
    delete this;
  }
  virtual FX_FILESIZE GetSize() override { return (FX_FILESIZE)m_nCurSize; }
  virtual FX_BOOL IsEOF() override { return m_nCurPos >= (size_t)GetSize(); }
  virtual FX_FILESIZE GetPosition() override { return (FX_FILESIZE)m_nCurPos; }
  virtual FX_BOOL ReadBlock(void* buffer,
                            FX_FILESIZE offset,
                            size_t size) override {
    if (!buffer || !size) {
      return FALSE;
    }

    FX_SAFE_SIZE_T newPos = size;
    newPos += offset;
    if (!newPos.IsValid() || newPos.ValueOrDefault(0) == 0 ||
        newPos.ValueOrDie() > m_nCurSize) {
      return FALSE;
    }

    m_nCurPos = newPos.ValueOrDie();
    if (m_dwFlags & FX_MEMSTREAM_Consecutive) {
      FXSYS_memcpy(buffer, (uint8_t*)m_Blocks[0] + (size_t)offset, size);
      return TRUE;
    }
    size_t nStartBlock = (size_t)offset / m_nGrowSize;
    offset -= (FX_FILESIZE)(nStartBlock * m_nGrowSize);
    while (size) {
      size_t nRead = m_nGrowSize - (size_t)offset;
      if (nRead > size) {
        nRead = size;
      }
      FXSYS_memcpy(
          buffer, (uint8_t*)m_Blocks[(int)nStartBlock] + (size_t)offset, nRead);
      buffer = ((uint8_t*)buffer) + nRead;
      size -= nRead;
      nStartBlock++;
      offset = 0;
    }
    return TRUE;
  }
  virtual size_t ReadBlock(void* buffer, size_t size) override {
    if (m_nCurPos >= m_nCurSize) {
      return 0;
    }
    size_t nRead = FX_MIN(size, m_nCurSize - m_nCurPos);
    if (!ReadBlock(buffer, (int32_t)m_nCurPos, nRead)) {
      return 0;
    }
    return nRead;
  }
  virtual FX_BOOL WriteBlock(const void* buffer,
                             FX_FILESIZE offset,
                             size_t size) override {
    if (!buffer || !size) {
      return FALSE;
    }
    if (m_dwFlags & FX_MEMSTREAM_Consecutive) {
      FX_SAFE_SIZE_T newPos = size;
      newPos += offset;
      if (!newPos.IsValid())
        return FALSE;

      m_nCurPos = newPos.ValueOrDie();
      if (m_nCurPos > m_nTotalSize) {
        m_nTotalSize =
            (m_nCurPos + m_nGrowSize - 1) / m_nGrowSize * m_nGrowSize;
        if (m_Blocks.GetSize() < 1) {
          void* block = FX_Alloc(uint8_t, m_nTotalSize);
          m_Blocks.Add(block);
        } else {
          m_Blocks[0] = FX_Realloc(uint8_t, m_Blocks[0], m_nTotalSize);
        }
        if (!m_Blocks[0]) {
          m_Blocks.RemoveAll();
          return FALSE;
        }
      }
      FXSYS_memcpy((uint8_t*)m_Blocks[0] + (size_t)offset, buffer, size);
      if (m_nCurSize < m_nCurPos) {
        m_nCurSize = m_nCurPos;
      }
      return TRUE;
    }

    FX_SAFE_SIZE_T newPos = size;
    newPos += offset;
    if (!newPos.IsValid()) {
      return FALSE;
    }

    if (!ExpandBlocks(newPos.ValueOrDie())) {
      return FALSE;
    }
    m_nCurPos = newPos.ValueOrDie();
    size_t nStartBlock = (size_t)offset / m_nGrowSize;
    offset -= (FX_FILESIZE)(nStartBlock * m_nGrowSize);
    while (size) {
      size_t nWrite = m_nGrowSize - (size_t)offset;
      if (nWrite > size) {
        nWrite = size;
      }
      FXSYS_memcpy((uint8_t*)m_Blocks[(int)nStartBlock] + (size_t)offset,
                   buffer, nWrite);
      buffer = ((uint8_t*)buffer) + nWrite;
      size -= nWrite;
      nStartBlock++;
      offset = 0;
    }
    return TRUE;
  }
  virtual FX_BOOL Flush() override { return TRUE; }
  virtual FX_BOOL IsConsecutive() const override {
    return m_dwFlags & FX_MEMSTREAM_Consecutive;
  }
  virtual void EstimateSize(size_t nInitSize, size_t nGrowSize) override {
    if (m_dwFlags & FX_MEMSTREAM_Consecutive) {
      if (m_Blocks.GetSize() < 1) {
        uint8_t* pBlock = FX_Alloc(uint8_t, FX_MAX(nInitSize, 4096));
        m_Blocks.Add(pBlock);
      }
      m_nGrowSize = FX_MAX(nGrowSize, 4096);
    } else if (m_Blocks.GetSize() < 1) {
      m_nGrowSize = FX_MAX(nGrowSize, 4096);
    }
  }
  virtual uint8_t* GetBuffer() const override {
    return m_Blocks.GetSize() ? (uint8_t*)m_Blocks[0] : NULL;
  }
  virtual void AttachBuffer(uint8_t* pBuffer,
                            size_t nSize,
                            FX_BOOL bTakeOver = FALSE) override {
    if (!(m_dwFlags & FX_MEMSTREAM_Consecutive)) {
      return;
    }
    m_Blocks.RemoveAll();
    m_Blocks.Add(pBuffer);
    m_nTotalSize = m_nCurSize = nSize;
    m_nCurPos = 0;
    m_dwFlags =
        FX_MEMSTREAM_Consecutive | (bTakeOver ? FX_MEMSTREAM_TakeOver : 0);
  }
  virtual void DetachBuffer() override {
    if (!(m_dwFlags & FX_MEMSTREAM_Consecutive)) {
      return;
    }
    m_Blocks.RemoveAll();
    m_nTotalSize = m_nCurSize = m_nCurPos = 0;
    m_dwFlags = FX_MEMSTREAM_TakeOver;
  }

 protected:
  CFX_PtrArray m_Blocks;
  FX_DWORD m_dwCount;
  size_t m_nTotalSize;
  size_t m_nCurSize;
  size_t m_nCurPos;
  size_t m_nGrowSize;
  FX_DWORD m_dwFlags;
  FX_BOOL ExpandBlocks(size_t size) {
    if (m_nCurSize < size) {
      m_nCurSize = size;
    }
    if (size <= m_nTotalSize) {
      return TRUE;
    }
    int32_t iCount = m_Blocks.GetSize();
    size = (size - m_nTotalSize + m_nGrowSize - 1) / m_nGrowSize;
    m_Blocks.SetSize(m_Blocks.GetSize() + (int32_t)size);
    while (size--) {
      uint8_t* pBlock = FX_Alloc(uint8_t, m_nGrowSize);
      m_Blocks.SetAt(iCount++, pBlock);
      m_nTotalSize += m_nGrowSize;
    }
    return TRUE;
  }
};

#ifdef __cplusplus
extern "C" {
#endif
#define MT_N 848
#define MT_M 456
#define MT_Matrix_A 0x9908b0df
#define MT_Upper_Mask 0x80000000
#define MT_Lower_Mask 0x7fffffff
typedef struct _FX_MTRANDOMCONTEXT {
  _FX_MTRANDOMCONTEXT() {
    mti = MT_N + 1;
    bHaveSeed = FALSE;
  }
  FX_DWORD mti;
  FX_BOOL bHaveSeed;
  FX_DWORD mt[MT_N];
} FX_MTRANDOMCONTEXT, *FX_LPMTRANDOMCONTEXT;
typedef FX_MTRANDOMCONTEXT const* FX_LPCMTRANDOMCONTEXT;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
FX_BOOL FX_GenerateCryptoRandom(FX_DWORD* pBuffer, int32_t iCount);
#endif
#ifdef __cplusplus
}
#endif

#endif  // CORE_SRC_FXCRT_EXTENSION_H_
