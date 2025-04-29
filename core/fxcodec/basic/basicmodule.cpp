// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/basic/basicmodule.h"

#include <stdint.h>

#include <algorithm>
#include <utility>

#include "core/fxcodec/scanlinedecoder.h"
#include "core/fxcrt/byteorder.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/stl_util.h"

namespace fxcodec {

namespace {

class RLScanlineDecoder final : public ScanlineDecoder {
 public:
  RLScanlineDecoder();
  ~RLScanlineDecoder() override;

  bool Create(pdfium::span<const uint8_t> src_buf,
              int width,
              int height,
              int nComps,
              int bpc);

  // ScanlineDecoder:
  [[nodiscard]] bool Rewind() override;
  pdfium::span<uint8_t> GetNextLine() override;
  uint32_t GetSrcOffset() override;

 private:
  bool CheckDestSize();
  void GetNextOperator();
  void UpdateOperator(uint8_t used_bytes);

  DataVector<uint8_t> scanline_;
  pdfium::raw_span<const uint8_t> src_buf_;
  size_t line_bytes_ = 0;
  size_t src_offset_ = 0;
  bool eod_ = false;
  uint8_t operator_ = 0;
};

RLScanlineDecoder::RLScanlineDecoder() = default;

RLScanlineDecoder::~RLScanlineDecoder() {
  // Span in superclass can't outlive our buffer.
  last_scanline_ = pdfium::span<uint8_t>();
}

bool RLScanlineDecoder::CheckDestSize() {
  size_t i = 0;
  uint32_t old_size = 0;
  uint32_t dest_size = 0;
  while (i < src_buf_.size()) {
    if (src_buf_[i] < 128) {
      old_size = dest_size;
      dest_size += src_buf_[i] + 1;
      if (dest_size < old_size) {
        return false;
      }
      i += src_buf_[i] + 2;
    } else if (src_buf_[i] > 128) {
      old_size = dest_size;
      dest_size += 257 - src_buf_[i];
      if (dest_size < old_size) {
        return false;
      }
      i += 2;
    } else {
      break;
    }
  }
  if (((uint32_t)orig_width_ * comps_ * bpc_ * orig_height_ + 7) / 8 >
      dest_size) {
    return false;
  }
  return true;
}

bool RLScanlineDecoder::Create(pdfium::span<const uint8_t> src_buf,
                               int width,
                               int height,
                               int nComps,
                               int bpc) {
  src_buf_ = src_buf;
  output_width_ = orig_width_ = width;
  output_height_ = orig_height_ = height;
  comps_ = nComps;
  bpc_ = bpc;
  // Aligning the pitch to 4 bytes requires an integer overflow check.
  FX_SAFE_UINT32 pitch = width;
  pitch *= nComps;
  pitch *= bpc;
  pitch += 31;
  pitch /= 32;
  pitch *= 4;
  if (!pitch.IsValid()) {
    return false;
  }
  pitch_ = pitch.ValueOrDie();
  // Overflow should already have been checked before this is called.
  line_bytes_ = (static_cast<uint32_t>(width) * nComps * bpc + 7) / 8;
  scanline_.resize(pitch_);
  return CheckDestSize();
}

bool RLScanlineDecoder::Rewind() {
  std::ranges::fill(scanline_, 0);
  src_offset_ = 0;
  eod_ = false;
  operator_ = 0;
  return true;
}

pdfium::span<uint8_t> RLScanlineDecoder::GetNextLine() {
  if (src_offset_ == 0) {
    GetNextOperator();
  } else if (eod_) {
    return pdfium::span<uint8_t>();
  }
  std::ranges::fill(scanline_, 0);
  uint32_t col_pos = 0;
  bool eol = false;
  auto scan_span = pdfium::span(scanline_);
  while (src_offset_ < src_buf_.size() && !eol) {
    if (operator_ < 128) {
      uint32_t copy_len = operator_ + 1;
      if (col_pos + copy_len >= line_bytes_) {
        copy_len = pdfium::checked_cast<uint32_t>(line_bytes_ - col_pos);
        eol = true;
      }
      if (copy_len >= src_buf_.size() - src_offset_) {
        copy_len =
            pdfium::checked_cast<uint32_t>(src_buf_.size() - src_offset_);
        eod_ = true;
      }
      fxcrt::Copy(src_buf_.subspan(src_offset_, copy_len),
                  scan_span.subspan(col_pos));
      col_pos += copy_len;
      UpdateOperator((uint8_t)copy_len);
    } else if (operator_ > 128) {
      int fill = 0;
      if (src_offset_ < src_buf_.size()) {
        fill = src_buf_[src_offset_];
      }
      uint32_t duplicate_len = 257 - operator_;
      if (col_pos + duplicate_len >= line_bytes_) {
        duplicate_len = pdfium::checked_cast<uint32_t>(line_bytes_ - col_pos);
        eol = true;
      }
      std::ranges::fill(scan_span.subspan(col_pos, duplicate_len), fill);
      col_pos += duplicate_len;
      UpdateOperator((uint8_t)duplicate_len);
    } else {
      eod_ = true;
      break;
    }
  }
  return scanline_;
}

uint32_t RLScanlineDecoder::GetSrcOffset() {
  return pdfium::checked_cast<uint32_t>(src_offset_);
}

void RLScanlineDecoder::GetNextOperator() {
  if (src_offset_ >= src_buf_.size()) {
    operator_ = 128;
    return;
  }
  operator_ = src_buf_[src_offset_];
  src_offset_++;
}
void RLScanlineDecoder::UpdateOperator(uint8_t used_bytes) {
  if (used_bytes == 0) {
    return;
  }
  if (operator_ < 128) {
    DCHECK((uint32_t)operator_ + 1 >= used_bytes);
    if (used_bytes == operator_ + 1) {
      src_offset_ += used_bytes;
      GetNextOperator();
      return;
    }
    operator_ -= used_bytes;
    src_offset_ += used_bytes;
    if (src_offset_ >= src_buf_.size()) {
      operator_ = 128;
    }
    return;
  }
  uint8_t count = 257 - operator_;
  DCHECK((uint32_t)count >= used_bytes);
  if (used_bytes == count) {
    src_offset_++;
    GetNextOperator();
    return;
  }
  count -= used_bytes;
  operator_ = 257 - count;
}

}  // namespace

// static
std::unique_ptr<ScanlineDecoder> BasicModule::CreateRunLengthDecoder(
    pdfium::span<const uint8_t> src_buf,
    int width,
    int height,
    int nComps,
    int bpc) {
  auto pDecoder = std::make_unique<RLScanlineDecoder>();
  if (!pDecoder->Create(src_buf, width, height, nComps, bpc)) {
    return nullptr;
  }

  return pDecoder;
}

// static
DataVector<uint8_t> BasicModule::RunLengthEncode(
    pdfium::span<const uint8_t> src_span) {
  if (src_span.empty()) {
    return {};
  }

  // Handle edge case.
  if (src_span.size() == 1) {
    return {0, src_span[0], 128};
  }

  // Worst case: 1 nonmatch, 2 match, 1 nonmatch, 2 match, etc. This becomes
  // 4 output chars for every 3 input, plus up to 4 more for the 1-2 chars
  // rounded off plus the terminating character.
  FX_SAFE_SIZE_T estimated_size = src_span.size();
  estimated_size += 2;
  estimated_size /= 3;
  estimated_size *= 4;
  estimated_size += 1;
  DataVector<uint8_t> result(estimated_size.ValueOrDie());

  // Set up span and counts.
  auto result_span = pdfium::span(result);
  uint32_t run_start = 0;
  uint32_t run_end = 1;
  uint8_t x = src_span[run_start];
  uint8_t y = src_span[run_end];
  while (run_end < src_span.size()) {
    size_t max_len = std::min<size_t>(128, src_span.size() - run_start);
    while (x == y && (run_end - run_start < max_len - 1)) {
      y = src_span[++run_end];
    }

    // Reached end with matched run. Update variables to expected values.
    if (x == y) {
      run_end++;
      if (run_end < src_span.size()) {
        y = src_span[run_end];
      }
    }
    if (run_end - run_start > 1) {  // Matched run but not at end of input.
      result_span[0] = 257 - (run_end - run_start);
      result_span[1] = x;
      x = y;
      run_start = run_end;
      run_end++;
      if (run_end < src_span.size()) {
        y = src_span[run_end];
      }
      result_span = result_span.subspan<2u>();
      continue;
    }
    // Mismatched run
    while (x != y && run_end <= run_start + max_len) {
      result_span[run_end - run_start] = x;
      x = y;
      run_end++;
      if (run_end == src_span.size()) {
        if (run_end <= run_start + max_len) {
          result_span[run_end - run_start] = x;
          run_end++;
        }
        break;
      }
      y = src_span[run_end];
    }
    result_span[0] = run_end - run_start - 2;
    result_span = result_span.subspan(run_end - run_start);
    run_start = run_end - 1;
  }
  if (run_start < src_span.size()) {  // 1 leftover character
    result_span[0] = 0;
    result_span[1] = x;
    result_span = result_span.subspan<2u>();
  }
  result_span[0] = 128;
  size_t new_size = 1 + result.size() - result_span.size();
  CHECK_LE(new_size, result.size());
  result.resize(new_size);
  return result;
}

// static
DataVector<uint8_t> BasicModule::A85Encode(
    pdfium::span<const uint8_t> src_span) {
  DataVector<uint8_t> result;
  if (src_span.empty()) {
    return result;
  }

  // Worst case: 5 output for each 4 input (plus up to 4 from leftover), plus
  // 2 character new lines each 75 output chars plus 2 termination chars. May
  // have fewer if there are special "z" chars.
  FX_SAFE_SIZE_T estimated_size = src_span.size();
  estimated_size /= 4;
  estimated_size *= 5;
  estimated_size += 4;
  estimated_size += src_span.size() / 30;
  estimated_size += 2;
  result.resize(estimated_size.ValueOrDie());

  // Set up span and counts.
  auto result_span = pdfium::span(result);
  uint32_t pos = 0;
  uint32_t line_length = 0;
  while (src_span.size() >= 4 && pos < src_span.size() - 3) {
    auto val_span = src_span.subspan(pos, 4u);
    uint32_t val = fxcrt::GetUInt32MSBFirst(val_span.first<4u>());
    pos += 4;
    if (val == 0) {  // All zero special case
      result_span[0] = 'z';
      result_span = result_span.subspan<1u>();
      line_length++;
    } else {  // Compute base 85 characters and add 33.
      for (int i = 4; i >= 0; i--) {
        result_span[i] = (val % 85) + 33;
        val /= 85;
      }
      result_span = result_span.subspan<5u>();
      line_length += 5;
    }
    if (line_length >= 75) {  // Add a return.
      result_span[0] = '\r';
      result_span[1] = '\n';
      result_span = result_span.subspan<2u>();
      line_length = 0;
    }
  }
  if (pos < src_span.size()) {  // Leftover bytes
    uint32_t val = 0;
    int count = 0;
    while (pos < src_span.size()) {
      val += (uint32_t)(src_span[pos]) << (8 * (3 - count));
      count++;
      pos++;
    }
    for (int i = 4; i >= 0; i--) {
      if (i <= count) {
        result_span[i] = (val % 85) + 33;
      }
      val /= 85;
    }
    result_span = result_span.subspan(static_cast<size_t>(count + 1));
  }

  // Terminating characters.
  result_span[0] = '~';
  result_span[1] = '>';
  size_t new_size = 2 + result.size() - result_span.size();
  CHECK_LE(new_size, result.size());
  result.resize(new_size);
  return result;
}

}  // namespace fxcodec
