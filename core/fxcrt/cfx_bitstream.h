// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_BITSTREAM_H_
#define CORE_FXCRT_CFX_BITSTREAM_H_

#include <stddef.h>
#include <stdint.h>

#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/span.h"

class CFX_BitStream {
 public:
  explicit CFX_BitStream(pdfium::span<const uint8_t> pData);
  ~CFX_BitStream();

  void ByteAlign();

  bool IsEOF() const { return bit_pos_ >= bit_size_; }
  size_t GetPos() const { return bit_pos_; }
  uint32_t GetBits(uint32_t nBits);

  void SkipBits(size_t nBits) { bit_pos_ += nBits; }
  void Rewind() { bit_pos_ = 0; }

  size_t BitsRemaining() const {
    return bit_size_ >= bit_pos_ ? bit_size_ - bit_pos_ : 0;
  }

 private:
  size_t bit_pos_ = 0;
  const size_t bit_size_;
  pdfium::raw_span<const uint8_t> const data_;
};

#endif  // CORE_FXCRT_CFX_BITSTREAM_H_
