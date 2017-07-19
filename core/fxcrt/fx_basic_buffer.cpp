// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_safe_types.h"
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
  memmove(m_pBuffer.get() + start_index, m_pBuffer.get() + start_index + count,
          m_DataSize - start_index - count);
  m_DataSize -= count;
}

void CFX_BinaryBuf::Clear() {
  m_DataSize = 0;
}

std::unique_ptr<uint8_t, FxFreeDeleter> CFX_BinaryBuf::DetachBuffer() {
  m_DataSize = 0;
  m_AllocSize = 0;
  return std::move(m_pBuffer);
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
    memcpy(m_pBuffer.get() + m_DataSize, pBuf, size);
  } else {
    memset(m_pBuffer.get() + m_DataSize, 0, size);
  }
  m_DataSize += size;
}

void CFX_BinaryBuf::InsertBlock(FX_STRSIZE pos,
                                const void* pBuf,
                                FX_STRSIZE size) {
  if (size <= 0)
    return;

  ExpandBuf(size);
  memmove(m_pBuffer.get() + pos + size, m_pBuffer.get() + pos,
          m_DataSize - pos);
  if (pBuf) {
    memcpy(m_pBuffer.get() + pos, pBuf, size);
  } else {
    memset(m_pBuffer.get() + pos, 0, size);
  }
  m_DataSize += size;
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
  FX_STRSIZE len = FX_ftoa((float)f, buf);
  AppendBlock(buf, len);
  return *this;
}

CFX_ByteTextBuf& CFX_ByteTextBuf::operator<<(const CFX_ByteTextBuf& buf) {
  AppendBlock(buf.m_pBuffer.get(), buf.m_DataSize);
  return *this;
}

void CFX_WideTextBuf::AppendChar(wchar_t ch) {
  ExpandBuf(sizeof(wchar_t));
  *(wchar_t*)(m_pBuffer.get() + m_DataSize) = ch;
  m_DataSize += sizeof(wchar_t);
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const CFX_WideStringC& str) {
  AppendBlock(str.unterminated_c_str(), str.GetLength() * sizeof(wchar_t));
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const CFX_WideString& str) {
  AppendBlock(str.c_str(), str.GetLength() * sizeof(wchar_t));
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(int i) {
  char buf[32];
  FXSYS_itoa(i, buf, 10);
  FX_STRSIZE len = FXSYS_strlen(buf);
  ExpandBuf(len * sizeof(wchar_t));
  wchar_t* str = (wchar_t*)(m_pBuffer.get() + m_DataSize);
  for (FX_STRSIZE j = 0; j < len; j++) {
    *str++ = buf[j];
  }
  m_DataSize += len * sizeof(wchar_t);
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(double f) {
  char buf[32];
  FX_STRSIZE len = FX_ftoa((float)f, buf);
  ExpandBuf(len * sizeof(wchar_t));
  wchar_t* str = (wchar_t*)(m_pBuffer.get() + m_DataSize);
  for (FX_STRSIZE i = 0; i < len; i++) {
    *str++ = buf[i];
  }
  m_DataSize += len * sizeof(wchar_t);
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const wchar_t* lpsz) {
  AppendBlock(lpsz, FXSYS_wcslen(lpsz) * sizeof(wchar_t));
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const CFX_WideTextBuf& buf) {
  AppendBlock(buf.m_pBuffer.get(), buf.m_DataSize);
  return *this;
}

void CFX_BitStream::Init(const uint8_t* pData, uint32_t dwSize) {
  m_pData = pData;
  m_BitSize = dwSize * 8;
  m_BitPos = 0;
}

void CFX_BitStream::ByteAlign() {
  m_BitPos = (m_BitPos + 7) & ~7;
}

uint32_t CFX_BitStream::GetBits(uint32_t nBits) {
  if (nBits > m_BitSize || m_BitPos + nBits > m_BitSize)
    return 0;

  if (nBits == 1) {
    int bit = (m_pData[m_BitPos / 8] & (1 << (7 - m_BitPos % 8))) ? 1 : 0;
    m_BitPos++;
    return bit;
  }

  uint32_t byte_pos = m_BitPos / 8;
  uint32_t bit_pos = m_BitPos % 8;
  uint32_t bit_left = nBits;
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
  if (bit_left)
    result |= m_pData[byte_pos] >> (8 - bit_left);
  m_BitPos += nBits;
  return result;
}
