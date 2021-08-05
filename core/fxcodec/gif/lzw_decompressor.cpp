// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/gif/lzw_decompressor.h"

#include <string.h>

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/numerics/safe_math.h"
#include "third_party/base/ptr_util.h"

namespace fxcodec {

std::unique_ptr<LZWDecompressor> LZWDecompressor::Create(uint8_t color_exp,
                                                         uint8_t code_exp) {
  // |color_exp| generates 2^(n + 1) codes, where as the code_exp reserves 2^n.
  // This is a quirk of the GIF spec.
  if (code_exp > GIF_MAX_LZW_EXP || code_exp < color_exp + 1)
    return nullptr;

  // Private ctor.
  return pdfium::WrapUnique(new LZWDecompressor(color_exp, code_exp));
}

LZWDecompressor::LZWDecompressor(uint8_t color_exp, uint8_t code_exp)
    : code_size_(code_exp),
      code_color_end_(static_cast<uint16_t>(1 << (color_exp + 1))),
      code_clear_(static_cast<uint16_t>(1 << code_exp)),
      code_end_(static_cast<uint16_t>((1 << code_exp) + 1)) {}

LZWDecompressor::~LZWDecompressor() = default;

LZWDecompressor::Status LZWDecompressor::Decode(const uint8_t* src_buf,
                                                uint32_t src_size,
                                                uint8_t* dest_buf,
                                                uint32_t* dest_size) {
  if (!src_buf || src_size == 0 || !dest_buf || !dest_size)
    return Status::kError;

  if (*dest_size == 0)
    return Status::kInsufficientDestSize;

  next_in_ = src_buf;
  avail_in_ = src_size;

  ClearTable();

  uint32_t i = 0;
  if (decompressed_next_ != 0) {
    uint32_t extracted_size = ExtractData(dest_buf, *dest_size);
    if (decompressed_next_ != 0)
      return Status::kInsufficientDestSize;

    dest_buf += extracted_size;
    i += extracted_size;
  }

  while (i <= *dest_size && (avail_in_ > 0 || bits_left_ >= code_size_cur_)) {
    if (code_size_cur_ > GIF_MAX_LZW_EXP)
      return Status::kError;

    if (avail_in_ > 0) {
      if (bits_left_ > 31)
        return Status::kError;

      FX_SAFE_UINT32 safe_code = *next_in_++;
      safe_code <<= bits_left_;
      safe_code |= code_store_;
      if (!safe_code.IsValid())
        return Status::kError;

      code_store_ = safe_code.ValueOrDie();
      --avail_in_;
      bits_left_ += 8;
    }

    while (bits_left_ >= code_size_cur_) {
      uint16_t code =
          static_cast<uint16_t>(code_store_) & ((1 << code_size_cur_) - 1);
      code_store_ >>= code_size_cur_;
      bits_left_ -= code_size_cur_;
      if (code == code_clear_) {
        ClearTable();
        continue;
      }
      if (code == code_end_) {
        *dest_size = i;
        return Status::kSuccess;
      }

      if (code_old_ != static_cast<uint16_t>(-1)) {
        if (code_next_ < GIF_MAX_LZW_CODE) {
          if (code == code_next_) {
            AddCode(code_old_, code_first_);
            if (!DecodeString(code))
              return Status::kError;
          } else if (code > code_next_) {
            return Status::kError;
          } else {
            if (!DecodeString(code))
              return Status::kError;

            uint8_t append_char = decompressed_[decompressed_next_ - 1];
            AddCode(code_old_, append_char);
          }
        }
      } else {
        if (!DecodeString(code))
          return Status::kError;
      }

      code_old_ = code;
      uint32_t extracted_size = ExtractData(dest_buf, *dest_size - i);
      if (decompressed_next_ != 0)
        return Status::kInsufficientDestSize;

      dest_buf += extracted_size;
      i += extracted_size;
    }
  }

  if (avail_in_ != 0)
    return Status::kError;

  *dest_size = i;
  return Status::kUnfinished;
}

void LZWDecompressor::ClearTable() {
  code_size_cur_ = code_size_ + 1;
  code_next_ = code_end_ + 1;
  code_old_ = static_cast<uint16_t>(-1);
  memset(code_table_, 0, sizeof(code_table_));
  for (uint16_t i = 0; i < code_clear_; i++)
    code_table_[i].suffix = static_cast<uint8_t>(i);
  decompressed_.resize(code_next_ - code_clear_ + 1);
  decompressed_next_ = 0;
}

void LZWDecompressor::AddCode(uint16_t prefix_code, uint8_t append_char) {
  if (code_next_ == GIF_MAX_LZW_CODE)
    return;

  code_table_[code_next_].prefix = prefix_code;
  code_table_[code_next_].suffix = append_char;
  if (++code_next_ < GIF_MAX_LZW_CODE) {
    if (code_next_ >> code_size_cur_)
      code_size_cur_++;
  }
}

bool LZWDecompressor::DecodeString(uint16_t code) {
  decompressed_.resize(code_next_ - code_clear_ + 1);
  decompressed_next_ = 0;

  while (code >= code_clear_ && code <= code_next_) {
    if (code == code_table_[code].prefix ||
        decompressed_next_ >= decompressed_.size())
      return false;

    decompressed_[decompressed_next_++] = code_table_[code].suffix;
    code = code_table_[code].prefix;
  }

  if (code >= code_color_end_)
    return false;

  decompressed_[decompressed_next_++] = static_cast<uint8_t>(code);
  code_first_ = static_cast<uint8_t>(code);
  return true;
}

uint32_t LZWDecompressor::ExtractData(uint8_t* dest_buf, uint32_t dest_size) {
  if (dest_size == 0)
    return 0;

  uint32_t copy_size = dest_size <= decompressed_next_
                           ? dest_size
                           : static_cast<uint32_t>(decompressed_next_);
  std::reverse_copy(decompressed_.data() + decompressed_next_ - copy_size,
                    decompressed_.data() + decompressed_next_, dest_buf);
  decompressed_next_ -= copy_size;
  return copy_size;
}

}  // namespace fxcodec
