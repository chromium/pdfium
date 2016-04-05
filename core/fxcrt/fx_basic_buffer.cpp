// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>
#include <limits>

#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_safe_types.h"
#include "third_party/base/numerics/safe_conversions.h"

CFX_BinaryBuf::CFX_BinaryBuf()
    : m_AllocStep(0), m_AllocSize(0), m_DataSize(0) {}

CFX_BinaryBuf::CFX_BinaryBuf(FX_STRSIZE size)
    : m_AllocStep(0), m_AllocSize(size), m_DataSize(size) {
  m_pBuffer.reset(FX_Alloc(uint8_t, size));
}

CFX_BinaryBuf::~CFX_BinaryBuf() {}

void CFX_BinaryBuf::Delete(int start_index, int count) {
  if (!m_pBuffer || start_index < 0 || count < 0 || count > m_DataSize ||
      start_index > m_DataSize - count) {
    return;
  }
  FXSYS_memmove(m_pBuffer.get() + start_index,
                m_pBuffer.get() + start_index + count,
                m_DataSize - start_index - count);
  m_DataSize -= count;
}

void CFX_BinaryBuf::Clear() {
  m_DataSize = 0;
}

uint8_t* CFX_BinaryBuf::DetachBuffer() {
  m_DataSize = 0;
  m_AllocSize = 0;
  return m_pBuffer.release();
}

void CFX_BinaryBuf::AttachData(uint8_t* buffer, FX_STRSIZE size) {
  m_pBuffer.reset(buffer);
  m_DataSize = size;
  m_AllocSize = size;
}

void CFX_BinaryBuf::EstimateSize(FX_STRSIZE size, FX_STRSIZE step) {
  m_AllocStep = step;
  if (m_AllocSize < size)
    ExpandBuf(size - m_DataSize);
}

void CFX_BinaryBuf::ExpandBuf(FX_STRSIZE add_size) {
  FX_SAFE_STRSIZE new_size = m_DataSize;
  new_size += add_size;
  if (m_AllocSize >= new_size.ValueOrDie())
    return;

  int alloc_step = std::max(128, m_AllocStep ? m_AllocStep : m_AllocSize / 4);
  new_size += alloc_step - 1;  // Quantize, don't combine these lines.
  new_size /= alloc_step;
  new_size *= alloc_step;
  m_AllocSize = new_size.ValueOrDie();
  m_pBuffer.reset(m_pBuffer
                      ? FX_Realloc(uint8_t, m_pBuffer.release(), m_AllocSize)
                      : FX_Alloc(uint8_t, m_AllocSize));
}

void CFX_BinaryBuf::AppendBlock(const void* pBuf, FX_STRSIZE size) {
  if (size <= 0)
    return;

  ExpandBuf(size);
  if (pBuf) {
    FXSYS_memcpy(m_pBuffer.get() + m_DataSize, pBuf, size);
  } else {
    FXSYS_memset(m_pBuffer.get() + m_DataSize, 0, size);
  }
  m_DataSize += size;
}

void CFX_BinaryBuf::InsertBlock(FX_STRSIZE pos,
                                const void* pBuf,
                                FX_STRSIZE size) {
  if (size <= 0)
    return;

  ExpandBuf(size);
  FXSYS_memmove(m_pBuffer.get() + pos + size, m_pBuffer.get() + pos,
                m_DataSize - pos);
  if (pBuf) {
    FXSYS_memcpy(m_pBuffer.get() + pos, pBuf, size);
  } else {
    FXSYS_memset(m_pBuffer.get() + pos, 0, size);
  }
  m_DataSize += size;
}

CFX_ByteStringC CFX_ByteTextBuf::GetByteString() const {
  return CFX_ByteStringC(m_pBuffer.get(), m_DataSize);
}

CFX_ByteTextBuf& CFX_ByteTextBuf::operator<<(const CFX_ByteStringC& lpsz) {
  AppendBlock(lpsz.raw_str(), lpsz.GetLength());
  return *this;
}

CFX_ByteTextBuf& CFX_ByteTextBuf::operator<<(int i) {
  char buf[32];
  FXSYS_itoa(i, buf, 10);
  AppendBlock(buf, FXSYS_strlen(buf));
  return *this;
}

CFX_ByteTextBuf& CFX_ByteTextBuf::operator<<(uint32_t i) {
  char buf[32];
  FXSYS_itoa(i, buf, 10);
  AppendBlock(buf, FXSYS_strlen(buf));
  return *this;
}

CFX_ByteTextBuf& CFX_ByteTextBuf::operator<<(double f) {
  char buf[32];
  FX_STRSIZE len = FX_ftoa((FX_FLOAT)f, buf);
  AppendBlock(buf, len);
  return *this;
}

CFX_ByteTextBuf& CFX_ByteTextBuf::operator<<(const CFX_ByteTextBuf& buf) {
  AppendBlock(buf.m_pBuffer.get(), buf.m_DataSize);
  return *this;
}

void CFX_WideTextBuf::AppendChar(FX_WCHAR ch) {
  ExpandBuf(sizeof(FX_WCHAR));
  *(FX_WCHAR*)(m_pBuffer.get() + m_DataSize) = ch;
  m_DataSize += sizeof(FX_WCHAR);
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const CFX_WideStringC& str) {
  AppendBlock(str.raw_str(), str.GetLength() * sizeof(FX_WCHAR));
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const CFX_WideString& str) {
  AppendBlock(str.c_str(), str.GetLength() * sizeof(FX_WCHAR));
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(int i) {
  char buf[32];
  FXSYS_itoa(i, buf, 10);
  FX_STRSIZE len = FXSYS_strlen(buf);
  ExpandBuf(len * sizeof(FX_WCHAR));
  FX_WCHAR* str = (FX_WCHAR*)(m_pBuffer.get() + m_DataSize);
  for (FX_STRSIZE j = 0; j < len; j++) {
    *str++ = buf[j];
  }
  m_DataSize += len * sizeof(FX_WCHAR);
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(double f) {
  char buf[32];
  FX_STRSIZE len = FX_ftoa((FX_FLOAT)f, buf);
  ExpandBuf(len * sizeof(FX_WCHAR));
  FX_WCHAR* str = (FX_WCHAR*)(m_pBuffer.get() + m_DataSize);
  for (FX_STRSIZE i = 0; i < len; i++) {
    *str++ = buf[i];
  }
  m_DataSize += len * sizeof(FX_WCHAR);
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const FX_WCHAR* lpsz) {
  AppendBlock(lpsz, FXSYS_wcslen(lpsz) * sizeof(FX_WCHAR));
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const CFX_WideTextBuf& buf) {
  AppendBlock(buf.m_pBuffer.get(), buf.m_DataSize);
  return *this;
}

CFX_WideStringC CFX_WideTextBuf::GetWideString() const {
  return CFX_WideStringC((const FX_WCHAR*)m_pBuffer.get(),
                         m_DataSize / sizeof(FX_WCHAR));
}

#ifdef PDF_ENABLE_XFA
CFX_ArchiveSaver& CFX_ArchiveSaver::operator<<(uint8_t i) {
  if (m_pStream) {
    m_pStream->WriteBlock(&i, 1);
  } else {
    m_SavingBuf.AppendByte(i);
  }
  return *this;
}
CFX_ArchiveSaver& CFX_ArchiveSaver::operator<<(int i) {
  if (m_pStream) {
    m_pStream->WriteBlock(&i, sizeof(int));
  } else {
    m_SavingBuf.AppendBlock(&i, sizeof(int));
  }
  return *this;
}
CFX_ArchiveSaver& CFX_ArchiveSaver::operator<<(uint32_t i) {
  if (m_pStream) {
    m_pStream->WriteBlock(&i, sizeof(uint32_t));
  } else {
    m_SavingBuf.AppendBlock(&i, sizeof(uint32_t));
  }
  return *this;
}
CFX_ArchiveSaver& CFX_ArchiveSaver::operator<<(FX_FLOAT f) {
  if (m_pStream) {
    m_pStream->WriteBlock(&f, sizeof(FX_FLOAT));
  } else {
    m_SavingBuf.AppendBlock(&f, sizeof(FX_FLOAT));
  }
  return *this;
}
CFX_ArchiveSaver& CFX_ArchiveSaver::operator<<(const CFX_ByteStringC& bstr) {
  int len = bstr.GetLength();
  if (m_pStream) {
    m_pStream->WriteBlock(&len, sizeof(int));
    m_pStream->WriteBlock(bstr.raw_str(), len);
  } else {
    m_SavingBuf.AppendBlock(&len, sizeof(int));
    m_SavingBuf.AppendBlock(bstr.raw_str(), len);
  }
  return *this;
}
CFX_ArchiveSaver& CFX_ArchiveSaver::operator<<(const FX_WCHAR* wstr) {
  FX_STRSIZE len = FXSYS_wcslen(wstr);
  if (m_pStream) {
    m_pStream->WriteBlock(&len, sizeof(int));
    m_pStream->WriteBlock(wstr, len);
  } else {
    m_SavingBuf.AppendBlock(&len, sizeof(int));
    m_SavingBuf.AppendBlock(wstr, len);
  }
  return *this;
}
CFX_ArchiveSaver& CFX_ArchiveSaver::operator<<(const CFX_WideString& wstr) {
  CFX_ByteString encoded = wstr.UTF16LE_Encode();
  return operator<<(encoded.AsByteStringC());
}
void CFX_ArchiveSaver::Write(const void* pData, FX_STRSIZE dwSize) {
  if (m_pStream) {
    m_pStream->WriteBlock(pData, dwSize);
  } else {
    m_SavingBuf.AppendBlock(pData, dwSize);
  }
}
CFX_ArchiveLoader::CFX_ArchiveLoader(const uint8_t* pData, uint32_t dwSize) {
  m_pLoadingBuf = pData;
  m_LoadingPos = 0;
  m_LoadingSize = dwSize;
}
FX_BOOL CFX_ArchiveLoader::IsEOF() {
  return m_LoadingPos >= m_LoadingSize;
}
CFX_ArchiveLoader& CFX_ArchiveLoader::operator>>(uint8_t& i) {
  if (m_LoadingPos >= m_LoadingSize) {
    return *this;
  }
  i = m_pLoadingBuf[m_LoadingPos++];
  return *this;
}
CFX_ArchiveLoader& CFX_ArchiveLoader::operator>>(int& i) {
  Read(&i, sizeof(int));
  return *this;
}
CFX_ArchiveLoader& CFX_ArchiveLoader::operator>>(uint32_t& i) {
  Read(&i, sizeof(uint32_t));
  return *this;
}
CFX_ArchiveLoader& CFX_ArchiveLoader::operator>>(FX_FLOAT& i) {
  Read(&i, sizeof(FX_FLOAT));
  return *this;
}
CFX_ArchiveLoader& CFX_ArchiveLoader::operator>>(CFX_ByteString& str) {
  if (m_LoadingPos + 4 > m_LoadingSize) {
    return *this;
  }
  int len;
  operator>>(len);
  str.Empty();
  if (len <= 0 || m_LoadingPos + len > m_LoadingSize) {
    return *this;
  }
  FX_CHAR* buffer = str.GetBuffer(len);
  FXSYS_memcpy(buffer, m_pLoadingBuf + m_LoadingPos, len);
  str.ReleaseBuffer(len);
  m_LoadingPos += len;
  return *this;
}
CFX_ArchiveLoader& CFX_ArchiveLoader::operator>>(CFX_WideString& str) {
  CFX_ByteString encoded;
  operator>>(encoded);
  str = CFX_WideString::FromUTF16LE(
      reinterpret_cast<const unsigned short*>(encoded.c_str()),
      encoded.GetLength() / sizeof(unsigned short));
  return *this;
}
FX_BOOL CFX_ArchiveLoader::Read(void* pBuf, uint32_t dwSize) {
  if (m_LoadingPos + dwSize > m_LoadingSize) {
    return FALSE;
  }
  FXSYS_memcpy(pBuf, m_pLoadingBuf + m_LoadingPos, dwSize);
  m_LoadingPos += dwSize;
  return TRUE;
}
#endif  // PDF_ENABLE_XFA

void CFX_BitStream::Init(const uint8_t* pData, uint32_t dwSize) {
  m_pData = pData;
  m_BitSize = dwSize * 8;
  m_BitPos = 0;
}
void CFX_BitStream::ByteAlign() {
  int mod = m_BitPos % 8;
  if (mod == 0) {
    return;
  }
  m_BitPos += 8 - mod;
}
uint32_t CFX_BitStream::GetBits(uint32_t nBits) {
  if (nBits > m_BitSize || m_BitPos + nBits > m_BitSize) {
    return 0;
  }
  if (nBits == 1) {
    int bit = (m_pData[m_BitPos / 8] & (1 << (7 - m_BitPos % 8))) ? 1 : 0;
    m_BitPos++;
    return bit;
  }
  uint32_t byte_pos = m_BitPos / 8;
  uint32_t bit_pos = m_BitPos % 8, bit_left = nBits;
  uint32_t result = 0;
  if (bit_pos) {
    if (8 - bit_pos >= bit_left) {
      result =
          (m_pData[byte_pos] & (0xff >> bit_pos)) >> (8 - bit_pos - bit_left);
      m_BitPos += bit_left;
      return result;
    }
    bit_left -= 8 - bit_pos;
    result = (m_pData[byte_pos++] & ((1 << (8 - bit_pos)) - 1)) << bit_left;
  }
  while (bit_left >= 8) {
    bit_left -= 8;
    result |= m_pData[byte_pos++] << bit_left;
  }
  if (bit_left) {
    result |= m_pData[byte_pos] >> (8 - bit_left);
  }
  m_BitPos += nBits;
  return result;
}

CFX_FileBufferArchive::CFX_FileBufferArchive()
    : m_Length(0), m_pFile(nullptr) {}

CFX_FileBufferArchive::~CFX_FileBufferArchive() {}

void CFX_FileBufferArchive::Clear() {
  m_Length = 0;
  m_pBuffer.reset();
  m_pFile = nullptr;
}

bool CFX_FileBufferArchive::Flush() {
  size_t nRemaining = m_Length;
  m_Length = 0;
  if (!m_pFile)
    return false;
  if (!m_pBuffer || !nRemaining)
    return true;
  return m_pFile->WriteBlock(m_pBuffer.get(), nRemaining) > 0;
}

int32_t CFX_FileBufferArchive::AppendBlock(const void* pBuf, size_t size) {
  if (!pBuf || size < 1) {
    return 0;
  }
  if (!m_pBuffer) {
    m_pBuffer.reset(FX_Alloc(uint8_t, kBufSize));
  }
  const uint8_t* buffer = reinterpret_cast<const uint8_t*>(pBuf);
  size_t temp_size = size;
  while (temp_size) {
    size_t buf_size = std::min(kBufSize - m_Length, temp_size);
    FXSYS_memcpy(m_pBuffer.get() + m_Length, buffer, buf_size);
    m_Length += buf_size;
    if (m_Length == kBufSize) {
      if (!Flush()) {
        return -1;
      }
    }
    temp_size -= buf_size;
    buffer += buf_size;
  }
  return pdfium::base::checked_cast<int32_t>(size);
}

int32_t CFX_FileBufferArchive::AppendByte(uint8_t byte) {
  return AppendBlock(&byte, 1);
}

int32_t CFX_FileBufferArchive::AppendDWord(uint32_t i) {
  char buf[32];
  FXSYS_itoa(i, buf, 10);
  return AppendBlock(buf, (size_t)FXSYS_strlen(buf));
}

int32_t CFX_FileBufferArchive::AppendString(const CFX_ByteStringC& lpsz) {
  return AppendBlock(lpsz.raw_str(), lpsz.GetLength());
}

void CFX_FileBufferArchive::AttachFile(IFX_StreamWrite* pFile) {
  FXSYS_assert(pFile);
  m_pFile = pFile;
}
