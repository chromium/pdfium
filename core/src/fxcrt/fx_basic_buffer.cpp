// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "core/include/fxcrt/fx_basic.h"

FX_STRSIZE FX_ftoa(FX_FLOAT f, FX_CHAR* buf);
CFX_BinaryBuf::CFX_BinaryBuf()
    : m_AllocStep(0), m_pBuffer(NULL), m_DataSize(0), m_AllocSize(0) {}
CFX_BinaryBuf::CFX_BinaryBuf(FX_STRSIZE size)
    : m_AllocStep(0), m_DataSize(size), m_AllocSize(size) {
  m_pBuffer = FX_Alloc(uint8_t, size);
}
CFX_BinaryBuf::~CFX_BinaryBuf() {
  FX_Free(m_pBuffer);
}
void CFX_BinaryBuf::Delete(int start_index, int count) {
  if (!m_pBuffer || start_index < 0 || start_index + count > m_DataSize) {
    return;
  }
  FXSYS_memmove(m_pBuffer + start_index, m_pBuffer + start_index + count,
                m_DataSize - start_index - count);
  m_DataSize -= count;
}
void CFX_BinaryBuf::Clear() {
  m_DataSize = 0;
}
void CFX_BinaryBuf::DetachBuffer() {
  m_DataSize = 0;
  m_pBuffer = NULL;
  m_AllocSize = 0;
}
void CFX_BinaryBuf::AttachData(void* buffer, FX_STRSIZE size) {
  FX_Free(m_pBuffer);
  m_DataSize = size;
  m_pBuffer = (uint8_t*)buffer;
  m_AllocSize = size;
}
void CFX_BinaryBuf::TakeOver(CFX_BinaryBuf& other) {
  AttachData(other.GetBuffer(), other.GetSize());
  other.DetachBuffer();
}
void CFX_BinaryBuf::EstimateSize(FX_STRSIZE size, FX_STRSIZE step) {
  m_AllocStep = step;
  if (m_AllocSize >= size) {
    return;
  }
  ExpandBuf(size - m_DataSize);
}
void CFX_BinaryBuf::ExpandBuf(FX_STRSIZE add_size) {
  FX_STRSIZE new_size = add_size + m_DataSize;
  if (m_AllocSize >= new_size) {
    return;
  }
  int alloc_step;
  if (m_AllocStep == 0) {
    alloc_step = m_AllocSize / 4;
    if (alloc_step < 128) {
      alloc_step = 128;
    }
  } else {
    alloc_step = m_AllocStep;
  }
  new_size = (new_size + alloc_step - 1) / alloc_step * alloc_step;
  uint8_t* pNewBuffer = m_pBuffer;
  if (pNewBuffer) {
    pNewBuffer = FX_Realloc(uint8_t, m_pBuffer, new_size);
  } else {
    pNewBuffer = FX_Alloc(uint8_t, new_size);
  }
  m_pBuffer = pNewBuffer;
  m_AllocSize = new_size;
}
void CFX_BinaryBuf::CopyData(const void* pStr, FX_STRSIZE size) {
  if (size == 0) {
    m_DataSize = 0;
    return;
  }
  if (m_AllocSize < size) {
    ExpandBuf(size - m_DataSize);
  }
  if (!m_pBuffer) {
    return;
  }
  FXSYS_memcpy(m_pBuffer, pStr, size);
  m_DataSize = size;
}
void CFX_BinaryBuf::AppendBlock(const void* pBuf, FX_STRSIZE size) {
  ExpandBuf(size);
  if (pBuf && m_pBuffer) {
    FXSYS_memcpy(m_pBuffer + m_DataSize, pBuf, size);
  }
  m_DataSize += size;
}
void CFX_BinaryBuf::InsertBlock(FX_STRSIZE pos,
                                const void* pBuf,
                                FX_STRSIZE size) {
  ExpandBuf(size);
  if (!m_pBuffer) {
    return;
  }
  FXSYS_memmove(m_pBuffer + pos + size, m_pBuffer + pos, m_DataSize - pos);
  if (pBuf) {
    FXSYS_memcpy(m_pBuffer + pos, pBuf, size);
  }
  m_DataSize += size;
}
void CFX_BinaryBuf::AppendFill(uint8_t byte, FX_STRSIZE count) {
  ExpandBuf(count);
  if (!m_pBuffer) {
    return;
  }
  FXSYS_memset(m_pBuffer + m_DataSize, byte, count);
  m_DataSize += count;
}
CFX_ByteStringC CFX_BinaryBuf::GetByteString() const {
  return CFX_ByteStringC(m_pBuffer, m_DataSize);
}
CFX_ByteTextBuf& CFX_ByteTextBuf::operator<<(const CFX_ByteStringC& lpsz) {
  AppendBlock(lpsz.GetPtr(), lpsz.GetLength());
  return *this;
}
CFX_ByteTextBuf& CFX_ByteTextBuf::operator<<(int i) {
  char buf[32];
  FXSYS_itoa(i, buf, 10);
  AppendBlock(buf, FXSYS_strlen(buf));
  return *this;
}
CFX_ByteTextBuf& CFX_ByteTextBuf::operator<<(FX_DWORD i) {
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
  AppendBlock(buf.m_pBuffer, buf.m_DataSize);
  return *this;
}
void CFX_ByteTextBuf::operator=(const CFX_ByteStringC& str) {
  CopyData(str.GetPtr(), str.GetLength());
}
void CFX_WideTextBuf::AppendChar(FX_WCHAR ch) {
  if (m_AllocSize < m_DataSize + (FX_STRSIZE)sizeof(FX_WCHAR)) {
    ExpandBuf(sizeof(FX_WCHAR));
  }
  ASSERT(m_pBuffer);
  *(FX_WCHAR*)(m_pBuffer + m_DataSize) = ch;
  m_DataSize += sizeof(FX_WCHAR);
}
CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const CFX_WideStringC& str) {
  AppendBlock(str.GetPtr(), str.GetLength() * sizeof(FX_WCHAR));
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
  if (m_AllocSize < m_DataSize + (FX_STRSIZE)(len * sizeof(FX_WCHAR))) {
    ExpandBuf(len * sizeof(FX_WCHAR));
  }
  ASSERT(m_pBuffer);
  FX_WCHAR* str = (FX_WCHAR*)(m_pBuffer + m_DataSize);
  for (FX_STRSIZE j = 0; j < len; j++) {
    *str++ = buf[j];
  }
  m_DataSize += len * sizeof(FX_WCHAR);
  return *this;
}
CFX_WideTextBuf& CFX_WideTextBuf::operator<<(double f) {
  char buf[32];
  FX_STRSIZE len = FX_ftoa((FX_FLOAT)f, buf);
  if (m_AllocSize < m_DataSize + (FX_STRSIZE)(len * sizeof(FX_WCHAR))) {
    ExpandBuf(len * sizeof(FX_WCHAR));
  }
  ASSERT(m_pBuffer);
  FX_WCHAR* str = (FX_WCHAR*)(m_pBuffer + m_DataSize);
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
  AppendBlock(buf.m_pBuffer, buf.m_DataSize);
  return *this;
}
void CFX_WideTextBuf::operator=(const CFX_WideStringC& str) {
  CopyData(str.GetPtr(), str.GetLength() * sizeof(FX_WCHAR));
}
CFX_WideStringC CFX_WideTextBuf::GetWideString() const {
  return CFX_WideStringC((const FX_WCHAR*)m_pBuffer,
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
CFX_ArchiveSaver& CFX_ArchiveSaver::operator<<(FX_DWORD i) {
  if (m_pStream) {
    m_pStream->WriteBlock(&i, sizeof(FX_DWORD));
  } else {
    m_SavingBuf.AppendBlock(&i, sizeof(FX_DWORD));
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
    m_pStream->WriteBlock(bstr.GetPtr(), len);
  } else {
    m_SavingBuf.AppendBlock(&len, sizeof(int));
    m_SavingBuf.AppendBlock(bstr.GetPtr(), len);
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
  return operator<<(encoded);
}
void CFX_ArchiveSaver::Write(const void* pData, FX_STRSIZE dwSize) {
  if (m_pStream) {
    m_pStream->WriteBlock(pData, dwSize);
  } else {
    m_SavingBuf.AppendBlock(pData, dwSize);
  }
}
CFX_ArchiveLoader::CFX_ArchiveLoader(const uint8_t* pData, FX_DWORD dwSize) {
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
CFX_ArchiveLoader& CFX_ArchiveLoader::operator>>(FX_DWORD& i) {
  Read(&i, sizeof(FX_DWORD));
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
FX_BOOL CFX_ArchiveLoader::Read(void* pBuf, FX_DWORD dwSize) {
  if (m_LoadingPos + dwSize > m_LoadingSize) {
    return FALSE;
  }
  FXSYS_memcpy(pBuf, m_pLoadingBuf + m_LoadingPos, dwSize);
  m_LoadingPos += dwSize;
  return TRUE;
}
#endif  // PDF_ENABLE_XFA

void CFX_BitStream::Init(const uint8_t* pData, FX_DWORD dwSize) {
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
FX_DWORD CFX_BitStream::GetBits(FX_DWORD nBits) {
  if (nBits > m_BitSize || m_BitPos + nBits > m_BitSize) {
    return 0;
  }
  if (nBits == 1) {
    int bit = (m_pData[m_BitPos / 8] & (1 << (7 - m_BitPos % 8))) ? 1 : 0;
    m_BitPos++;
    return bit;
  }
  FX_DWORD byte_pos = m_BitPos / 8;
  FX_DWORD bit_pos = m_BitPos % 8, bit_left = nBits;
  FX_DWORD result = 0;
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
IFX_BufferArchive::IFX_BufferArchive(FX_STRSIZE size)
    : m_BufSize(size), m_pBuffer(NULL), m_Length(0) {}
void IFX_BufferArchive::Clear() {
  m_Length = 0;
  FX_Free(m_pBuffer);
  m_pBuffer = NULL;
}
FX_BOOL IFX_BufferArchive::Flush() {
  FX_BOOL bRet = DoWork(m_pBuffer, m_Length);
  m_Length = 0;
  return bRet;
}
int32_t IFX_BufferArchive::AppendBlock(const void* pBuf, size_t size) {
  if (!pBuf || size < 1) {
    return 0;
  }
  if (!m_pBuffer) {
    m_pBuffer = FX_Alloc(uint8_t, m_BufSize);
  }
  uint8_t* buffer = (uint8_t*)pBuf;
  FX_STRSIZE temp_size = (FX_STRSIZE)size;
  while (temp_size > 0) {
    FX_STRSIZE buf_size = std::min(m_BufSize - m_Length, (FX_STRSIZE)temp_size);
    FXSYS_memcpy(m_pBuffer + m_Length, buffer, buf_size);
    m_Length += buf_size;
    if (m_Length == m_BufSize) {
      if (!Flush()) {
        return -1;
      }
    }
    temp_size -= buf_size;
    buffer += buf_size;
  }
  return (int32_t)size;
}
int32_t IFX_BufferArchive::AppendByte(uint8_t byte) {
  return AppendBlock(&byte, 1);
}
int32_t IFX_BufferArchive::AppendDWord(FX_DWORD i) {
  char buf[32];
  FXSYS_itoa(i, buf, 10);
  return AppendBlock(buf, (size_t)FXSYS_strlen(buf));
}
int32_t IFX_BufferArchive::AppendString(const CFX_ByteStringC& lpsz) {
  return AppendBlock(lpsz.GetPtr(), lpsz.GetLength());
}
CFX_FileBufferArchive::CFX_FileBufferArchive(FX_STRSIZE size)
    : IFX_BufferArchive(size), m_pFile(NULL), m_bTakeover(FALSE) {}
CFX_FileBufferArchive::~CFX_FileBufferArchive() {
  Clear();
}
void CFX_FileBufferArchive::Clear() {
  if (m_pFile && m_bTakeover) {
    m_pFile->Release();
  }
  m_pFile = NULL;
  m_bTakeover = FALSE;
  IFX_BufferArchive::Clear();
}
FX_BOOL CFX_FileBufferArchive::AttachFile(IFX_StreamWrite* pFile,
                                          FX_BOOL bTakeover) {
  if (!pFile) {
    return FALSE;
  }
  if (m_pFile && m_bTakeover) {
    m_pFile->Release();
  }
  m_pFile = pFile;
  m_bTakeover = bTakeover;
  return TRUE;
}
FX_BOOL CFX_FileBufferArchive::DoWork(const void* pBuf, size_t size) {
  if (!m_pFile) {
    return FALSE;
  }
  if (!pBuf || size < 1) {
    return TRUE;
  }
  return m_pFile->WriteBlock(pBuf, size);
}
