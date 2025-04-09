// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_bitstream.h"

#include <limits>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"

CFX_BitStream::CFX_BitStream(pdfium::span<const uint8_t> pData)
    : bit_size_(pData.size() * 8), data_(pData) {
  CHECK_LE(data_.size(), std::numeric_limits<size_t>::max() / 8);
}

CFX_BitStream::~CFX_BitStream() = default;

void CFX_BitStream::ByteAlign() {
  bit_pos_ = FxAlignToBoundary<8>(bit_pos_);
}

uint32_t CFX_BitStream::GetBits(uint32_t nBits) {
  DCHECK(nBits > 0);
  DCHECK(nBits <= 32);
  if (nBits > bit_size_ || bit_pos_ > bit_size_ - nBits) {
    return 0;
  }

  const uint32_t bit_pos = bit_pos_ % 8;
  size_t byte_pos = bit_pos_ / 8;
  uint8_t current_byte = data_[byte_pos];

  if (nBits == 1) {
    uint32_t bit = (current_byte & (1 << (7 - bit_pos))) ? 1 : 0;
    bit_pos_++;
    return bit;
  }

  uint32_t bit_left = nBits;
  uint32_t result = 0;
  if (bit_pos) {
    uint32_t bits_readable = 8 - bit_pos;
    if (bits_readable >= bit_left) {
      result = (current_byte & (0xff >> bit_pos)) >> (bits_readable - bit_left);
      bit_pos_ += bit_left;
      return result;
    }
    bit_left -= bits_readable;
    result = (current_byte & ((1 << bits_readable) - 1)) << bit_left;
    ++byte_pos;
  }
  while (bit_left >= 8) {
    bit_left -= 8;
    result |= data_[byte_pos++] << bit_left;
  }
  if (bit_left) {
    result |= data_[byte_pos] >> (8 - bit_left);
  }
  bit_pos_ += nBits;
  return result;
}
