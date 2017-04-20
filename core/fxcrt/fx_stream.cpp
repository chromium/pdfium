// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_stream.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/ifxcrt_fileaccess.h"

#define FX_MEMSTREAM_BlockSize (64 * 1024)
#define FX_MEMSTREAM_Consecutive 0x01
#define FX_MEMSTREAM_TakeOver 0x02

namespace {

class CFX_CRTFileStream final : public IFX_SeekableStream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFX_SeekableStream:
  FX_FILESIZE GetSize() override { return m_pFile->GetSize(); }
  bool IsEOF() override { return GetPosition() >= GetSize(); }
  FX_FILESIZE GetPosition() override { return m_pFile->GetPosition(); }
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override {
    return m_pFile->ReadPos(buffer, size, offset) > 0;
  }
  size_t ReadBlock(void* buffer, size_t size) override {
    return m_pFile->Read(buffer, size);
  }
  bool WriteBlock(const void* buffer,
                  FX_FILESIZE offset,
                  size_t size) override {
    return !!m_pFile->WritePos(buffer, size, offset);
  }
  bool Flush() override { return m_pFile->Flush(); }

 private:
  explicit CFX_CRTFileStream(std::unique_ptr<IFXCRT_FileAccess> pFA)
      : m_pFile(std::move(pFA)) {}
  ~CFX_CRTFileStream() override {}

  std::unique_ptr<IFXCRT_FileAccess> m_pFile;
};

class CFX_MemoryStream final : public IFX_MemoryStream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFX_MemoryStream
  FX_FILESIZE GetSize() override {
    return static_cast<FX_FILESIZE>(m_nCurSize);
  }
  bool IsEOF() override { return m_nCurPos >= static_cast<size_t>(GetSize()); }
  FX_FILESIZE GetPosition() override {
    return static_cast<FX_FILESIZE>(m_nCurPos);
  }
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override;
  size_t ReadBlock(void* buffer, size_t size) override;
  bool WriteBlock(const void* buffer, FX_FILESIZE offset, size_t size) override;
  bool Flush() override { return true; }
  bool IsConsecutive() const override {
    return !!(m_dwFlags & FX_MEMSTREAM_Consecutive);
  }
  void EstimateSize(size_t nInitSize, size_t nGrowSize) override;
  uint8_t* GetBuffer() const override {
    return !m_Blocks.empty() ? m_Blocks.front() : nullptr;
  }
  void AttachBuffer(uint8_t* pBuffer,
                    size_t nSize,
                    bool bTakeOver = false) override;
  void DetachBuffer() override;

 private:
  explicit CFX_MemoryStream(bool bConsecutive);
  CFX_MemoryStream(uint8_t* pBuffer, size_t nSize, bool bTakeOver);
  ~CFX_MemoryStream() override;

  bool ExpandBlocks(size_t size);

  std::vector<uint8_t*> m_Blocks;
  size_t m_nTotalSize;
  size_t m_nCurSize;
  size_t m_nCurPos;
  size_t m_nGrowSize;
  uint32_t m_dwFlags;
};

CFX_MemoryStream::CFX_MemoryStream(bool bConsecutive)
    : m_nTotalSize(0),
      m_nCurSize(0),
      m_nCurPos(0),
      m_nGrowSize(FX_MEMSTREAM_BlockSize) {
  m_dwFlags =
      FX_MEMSTREAM_TakeOver | (bConsecutive ? FX_MEMSTREAM_Consecutive : 0);
}

CFX_MemoryStream::CFX_MemoryStream(uint8_t* pBuffer,
                                   size_t nSize,
                                   bool bTakeOver)
    : m_nTotalSize(nSize),
      m_nCurSize(nSize),
      m_nCurPos(0),
      m_nGrowSize(FX_MEMSTREAM_BlockSize) {
  m_Blocks.push_back(pBuffer);
  m_dwFlags =
      FX_MEMSTREAM_Consecutive | (bTakeOver ? FX_MEMSTREAM_TakeOver : 0);
}

CFX_MemoryStream::~CFX_MemoryStream() {
  if (m_dwFlags & FX_MEMSTREAM_TakeOver) {
    for (uint8_t* pBlock : m_Blocks)
      FX_Free(pBlock);
  }
}

bool CFX_MemoryStream::ReadBlock(void* buffer,
                                 FX_FILESIZE offset,
                                 size_t size) {
  if (!buffer || !size || offset < 0)
    return false;

  FX_SAFE_SIZE_T newPos = size;
  newPos += offset;
  if (!newPos.IsValid() || newPos.ValueOrDefault(0) == 0 ||
      newPos.ValueOrDie() > m_nCurSize) {
    return false;
  }

  m_nCurPos = newPos.ValueOrDie();
  if (m_dwFlags & FX_MEMSTREAM_Consecutive) {
    memcpy(buffer, m_Blocks[0] + static_cast<size_t>(offset), size);
    return true;
  }

  size_t nStartBlock = static_cast<size_t>(offset) / m_nGrowSize;
  offset -= static_cast<FX_FILESIZE>(nStartBlock * m_nGrowSize);
  while (size) {
    size_t nRead = std::min(size, m_nGrowSize - static_cast<size_t>(offset));
    memcpy(buffer, m_Blocks[nStartBlock] + offset, nRead);
    buffer = static_cast<uint8_t*>(buffer) + nRead;
    size -= nRead;
    ++nStartBlock;
    offset = 0;
  }
  return true;
}

size_t CFX_MemoryStream::ReadBlock(void* buffer, size_t size) {
  if (m_nCurPos >= m_nCurSize)
    return 0;

  size_t nRead = std::min(size, m_nCurSize - m_nCurPos);
  if (!ReadBlock(buffer, static_cast<int32_t>(m_nCurPos), nRead))
    return 0;

  return nRead;
}

bool CFX_MemoryStream::WriteBlock(const void* buffer,
                                  FX_FILESIZE offset,
                                  size_t size) {
  if (!buffer || !size)
    return false;

  if (m_dwFlags & FX_MEMSTREAM_Consecutive) {
    FX_SAFE_SIZE_T newPos = size;
    newPos += offset;
    if (!newPos.IsValid())
      return false;

    m_nCurPos = newPos.ValueOrDie();
    if (m_nCurPos > m_nTotalSize) {
      m_nTotalSize = (m_nCurPos + m_nGrowSize - 1) / m_nGrowSize * m_nGrowSize;
      if (m_Blocks.empty())
        m_Blocks.push_back(FX_Alloc(uint8_t, m_nTotalSize));
      else
        m_Blocks[0] = FX_Realloc(uint8_t, m_Blocks[0], m_nTotalSize);
    }

    memcpy(m_Blocks[0] + offset, buffer, size);
    m_nCurSize = std::max(m_nCurSize, m_nCurPos);

    return true;
  }

  FX_SAFE_SIZE_T newPos = size;
  newPos += offset;
  if (!newPos.IsValid())
    return false;
  if (!ExpandBlocks(newPos.ValueOrDie()))
    return false;

  m_nCurPos = newPos.ValueOrDie();
  size_t nStartBlock = static_cast<size_t>(offset) / m_nGrowSize;
  offset -= static_cast<FX_FILESIZE>(nStartBlock * m_nGrowSize);
  while (size) {
    size_t nWrite = std::min(size, m_nGrowSize - static_cast<size_t>(offset));
    memcpy(m_Blocks[nStartBlock] + offset, buffer, nWrite);
    buffer = static_cast<const uint8_t*>(buffer) + nWrite;
    size -= nWrite;
    ++nStartBlock;
    offset = 0;
  }
  return true;
}

void CFX_MemoryStream::EstimateSize(size_t nInitSize, size_t nGrowSize) {
  if (m_dwFlags & FX_MEMSTREAM_Consecutive) {
    if (m_Blocks.empty()) {
      m_Blocks.push_back(
          FX_Alloc(uint8_t, std::max(nInitSize, static_cast<size_t>(4096))));
    }
    m_nGrowSize = std::max(nGrowSize, static_cast<size_t>(4096));
  } else if (m_Blocks.empty()) {
    m_nGrowSize = std::max(nGrowSize, static_cast<size_t>(4096));
  }
}

void CFX_MemoryStream::AttachBuffer(uint8_t* pBuffer,
                                    size_t nSize,
                                    bool bTakeOver) {
  if (!(m_dwFlags & FX_MEMSTREAM_Consecutive))
    return;

  m_Blocks.clear();
  m_Blocks.push_back(pBuffer);
  m_nTotalSize = nSize;
  m_nCurSize = nSize;
  m_nCurPos = 0;
  m_dwFlags =
      FX_MEMSTREAM_Consecutive | (bTakeOver ? FX_MEMSTREAM_TakeOver : 0);
}

void CFX_MemoryStream::DetachBuffer() {
  if (!(m_dwFlags & FX_MEMSTREAM_Consecutive))
    return;

  m_Blocks.clear();
  m_nTotalSize = 0;
  m_nCurSize = 0;
  m_nCurPos = 0;
  m_dwFlags = FX_MEMSTREAM_TakeOver;
}

bool CFX_MemoryStream::ExpandBlocks(size_t size) {
  m_nCurSize = std::max(m_nCurSize, size);
  if (size <= m_nTotalSize)
    return true;

  size = (size - m_nTotalSize + m_nGrowSize - 1) / m_nGrowSize;
  size_t iCount = m_Blocks.size();
  m_Blocks.resize(iCount + size);
  while (size--) {
    m_Blocks[iCount++] = FX_Alloc(uint8_t, m_nGrowSize);
    m_nTotalSize += m_nGrowSize;
  }
  return true;
}

}  // namespace

// static
CFX_RetainPtr<IFX_SeekableStream> IFX_SeekableStream::CreateFromFilename(
    const char* filename,
    uint32_t dwModes) {
  std::unique_ptr<IFXCRT_FileAccess> pFA(IFXCRT_FileAccess::Create());
  if (!pFA->Open(filename, dwModes))
    return nullptr;
  return pdfium::MakeRetain<CFX_CRTFileStream>(std::move(pFA));
}

// static
CFX_RetainPtr<IFX_SeekableStream> IFX_SeekableStream::CreateFromFilename(
    const wchar_t* filename,
    uint32_t dwModes) {
  std::unique_ptr<IFXCRT_FileAccess> pFA(IFXCRT_FileAccess::Create());
  if (!pFA->Open(filename, dwModes))
    return nullptr;
  return pdfium::MakeRetain<CFX_CRTFileStream>(std::move(pFA));
}

// static
CFX_RetainPtr<IFX_SeekableReadStream>
IFX_SeekableReadStream::CreateFromFilename(const char* filename) {
  return IFX_SeekableStream::CreateFromFilename(filename, FX_FILEMODE_ReadOnly);
}

// static
CFX_RetainPtr<IFX_MemoryStream> IFX_MemoryStream::Create(uint8_t* pBuffer,
                                                         size_t dwSize,
                                                         bool bTakeOver) {
  return pdfium::MakeRetain<CFX_MemoryStream>(pBuffer, dwSize, bTakeOver);
}

// static
CFX_RetainPtr<IFX_MemoryStream> IFX_MemoryStream::Create(bool bConsecutive) {
  return pdfium::MakeRetain<CFX_MemoryStream>(bConsecutive);
}

bool IFX_SeekableWriteStream::WriteBlock(const void* pData, size_t size) {
  return WriteBlock(pData, GetSize(), size);
}

bool IFX_SeekableReadStream::IsEOF() {
  return false;
}

FX_FILESIZE IFX_SeekableReadStream::GetPosition() {
  return 0;
}

size_t IFX_SeekableReadStream::ReadBlock(void* buffer, size_t size) {
  return 0;
}

bool IFX_SeekableStream::WriteBlock(const void* buffer, size_t size) {
  return WriteBlock(buffer, GetSize(), size);
}
