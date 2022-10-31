// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_GIF_LZW_DECOMPRESSOR_H_
#define CORE_FXCODEC_GIF_LZW_DECOMPRESSOR_H_

#include <stdint.h>

#include <memory>

#include "core/fxcodec/gif/cfx_gif.h"
#include "core/fxcrt/data_vector.h"

namespace fxcodec {

class LZWDecompressor {
 public:
  enum class Status {
    kError,
    kSuccess,
    kUnfinished,
    kInsufficientDestSize,
  };

  struct CodeEntry {
    uint16_t prefix;
    uint8_t suffix;
  };

  // Returns nullptr on error
  static std::unique_ptr<LZWDecompressor> Create(uint8_t color_exp,
                                                 uint8_t code_exp);
  ~LZWDecompressor();

  void SetSource(const uint8_t* src_buf, uint32_t src_size);
  Status Decode(uint8_t* dest_buf, uint32_t* dest_size);

  // Used by unittests, should not be called in production code.
  uint32_t ExtractDataForTest(uint8_t* dest_buf, uint32_t dest_size) {
    return ExtractData(dest_buf, dest_size);
  }

  DataVector<uint8_t>* DecompressedForTest() { return &decompressed_; }
  size_t* DecompressedNextForTest() { return &decompressed_next_; }

 private:
  // Use Create() instead.
  LZWDecompressor(uint8_t color_exp, uint8_t code_exp);

  void ClearTable();
  void AddCode(uint16_t prefix_code, uint8_t append_char);
  bool DecodeString(uint16_t code);
  uint32_t ExtractData(uint8_t* dest_buf, uint32_t dest_size);

  const uint8_t code_size_;
  uint8_t code_size_cur_ = 0;
  const uint16_t code_color_end_;
  const uint16_t code_clear_;
  const uint16_t code_end_;
  uint16_t code_next_ = 0;
  uint8_t code_first_ = 0;
  DataVector<uint8_t> decompressed_;
  size_t decompressed_next_ = 0;
  uint16_t code_old_ = 0;
  const uint8_t* next_in_ = nullptr;
  uint32_t avail_in_ = 0;
  uint8_t bits_left_ = 0;
  uint32_t code_store_ = 0;
  CodeEntry code_table_[GIF_MAX_LZW_CODE];
};

}  // namespace fxcodec

using LZWDecompressor = fxcodec::LZWDecompressor;

#endif  // CORE_FXCODEC_GIF_LZW_DECOMPRESSOR_H_
