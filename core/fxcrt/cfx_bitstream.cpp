// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_bitstream.h"

#include <limits>

#include "core/fxcrt/fx_system.h"

CFX_BitStream::CFX_BitStream(const uint8_t* pData, uint32_t dwSize)
    : m_BitPos(0), m_BitSize(dwSize * 8), m_pData(pData) {
  ASSERT(dwSize <= std::numeric_limits<uint32_t>::max() / 8);
}

CFX_BitStream::~CFX_BitStream() {}

void CFX_BitStream::ByteAlign() {
  m_BitPos = (m_BitPos + 7) & ~7;
}

uint32_t CFX_BitStream::GetBits(uint32_t nBits) {
  if (nBits > m_BitSize || m_BitPos + nBits > m_BitSize)
    return 0;

  const uint8_t* data = m_pData.Get();

  if (nBits == 1) {
    int bit = (data[m_BitPos / 8] & (1 << (7 - m_BitPos % 8))) ? 1 : 0;
    m_BitPos++;
    return bit;
  }

  uint32_t byte_pos = m_BitPos / 8;
  uint32_t bit_pos = m_BitPos % 8;
  uint32_t bit_left = nBits;
  uint32_t result = 0;
  if (bit_pos) {
    if (8 - bit_pos >= bit_left) {
      result = (data[byte_pos] & (0xff >> bit_pos)) >> (8 - bit_pos - bit_left);
      m_BitPos += bit_left;
      return result;
    }
    bit_left -= 8 - bit_pos;
    result = (data[byte_pos++] & ((1 << (8 - bit_pos)) - 1)) << bit_left;
  }
  while (bit_left >= 8) {
    bit_left -= 8;
    result |= data[byte_pos++] << bit_left;
  }
  if (bit_left)
    result |= data[byte_pos] >> (8 - bit_left);
  m_BitPos += nBits;
  return result;
}
