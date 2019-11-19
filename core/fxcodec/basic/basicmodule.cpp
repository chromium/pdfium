// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/basic/basicmodule.h"

#include <algorithm>
#include <utility>

#include "core/fxcodec/scanlinedecoder.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/ptr_util.h"

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
  bool v_Rewind() override;
  uint8_t* v_GetNextLine() override;
  uint32_t GetSrcOffset() override { return m_SrcOffset; }

 private:
  bool CheckDestSize();
  void GetNextOperator();
  void UpdateOperator(uint8_t used_bytes);

  std::unique_ptr<uint8_t, FxFreeDeleter> m_pScanline;
  pdfium::span<const uint8_t> m_SrcBuf;
  size_t m_dwLineBytes = 0;
  size_t m_SrcOffset = 0;
  bool m_bEOD = false;
  uint8_t m_Operator = 0;
};

RLScanlineDecoder::RLScanlineDecoder() = default;

RLScanlineDecoder::~RLScanlineDecoder() = default;

bool RLScanlineDecoder::CheckDestSize() {
  size_t i = 0;
  uint32_t old_size = 0;
  uint32_t dest_size = 0;
  while (i < m_SrcBuf.size()) {
    if (m_SrcBuf[i] < 128) {
      old_size = dest_size;
      dest_size += m_SrcBuf[i] + 1;
      if (dest_size < old_size) {
        return false;
      }
      i += m_SrcBuf[i] + 2;
    } else if (m_SrcBuf[i] > 128) {
      old_size = dest_size;
      dest_size += 257 - m_SrcBuf[i];
      if (dest_size < old_size) {
        return false;
      }
      i += 2;
    } else {
      break;
    }
  }
  if (((uint32_t)m_OrigWidth * m_nComps * m_bpc * m_OrigHeight + 7) / 8 >
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
  m_SrcBuf = src_buf;
  m_OutputWidth = m_OrigWidth = width;
  m_OutputHeight = m_OrigHeight = height;
  m_nComps = nComps;
  m_bpc = bpc;
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
  m_Pitch = pitch.ValueOrDie();
  // Overflow should already have been checked before this is called.
  m_dwLineBytes = (static_cast<uint32_t>(width) * nComps * bpc + 7) / 8;
  m_pScanline.reset(FX_Alloc(uint8_t, m_Pitch));
  return CheckDestSize();
}

bool RLScanlineDecoder::v_Rewind() {
  memset(m_pScanline.get(), 0, m_Pitch);
  m_SrcOffset = 0;
  m_bEOD = false;
  m_Operator = 0;
  return true;
}

uint8_t* RLScanlineDecoder::v_GetNextLine() {
  if (m_SrcOffset == 0) {
    GetNextOperator();
  } else if (m_bEOD) {
    return nullptr;
  }
  memset(m_pScanline.get(), 0, m_Pitch);
  uint32_t col_pos = 0;
  bool eol = false;
  while (m_SrcOffset < m_SrcBuf.size() && !eol) {
    if (m_Operator < 128) {
      uint32_t copy_len = m_Operator + 1;
      if (col_pos + copy_len >= m_dwLineBytes) {
        copy_len = m_dwLineBytes - col_pos;
        eol = true;
      }
      if (copy_len >= m_SrcBuf.size() - m_SrcOffset) {
        copy_len = m_SrcBuf.size() - m_SrcOffset;
        m_bEOD = true;
      }
      auto copy_span = m_SrcBuf.subspan(m_SrcOffset, copy_len);
      memcpy(m_pScanline.get() + col_pos, copy_span.data(), copy_span.size());
      col_pos += copy_len;
      UpdateOperator((uint8_t)copy_len);
    } else if (m_Operator > 128) {
      int fill = 0;
      if (m_SrcOffset - 1 < m_SrcBuf.size() - 1) {
        fill = m_SrcBuf[m_SrcOffset];
      }
      uint32_t duplicate_len = 257 - m_Operator;
      if (col_pos + duplicate_len >= m_dwLineBytes) {
        duplicate_len = m_dwLineBytes - col_pos;
        eol = true;
      }
      memset(m_pScanline.get() + col_pos, fill, duplicate_len);
      col_pos += duplicate_len;
      UpdateOperator((uint8_t)duplicate_len);
    } else {
      m_bEOD = true;
      break;
    }
  }
  return m_pScanline.get();
}

void RLScanlineDecoder::GetNextOperator() {
  if (m_SrcOffset >= m_SrcBuf.size()) {
    m_Operator = 128;
    return;
  }
  m_Operator = m_SrcBuf[m_SrcOffset];
  m_SrcOffset++;
}
void RLScanlineDecoder::UpdateOperator(uint8_t used_bytes) {
  if (used_bytes == 0) {
    return;
  }
  if (m_Operator < 128) {
    ASSERT((uint32_t)m_Operator + 1 >= used_bytes);
    if (used_bytes == m_Operator + 1) {
      m_SrcOffset += used_bytes;
      GetNextOperator();
      return;
    }
    m_Operator -= used_bytes;
    m_SrcOffset += used_bytes;
    if (m_SrcOffset >= m_SrcBuf.size()) {
      m_Operator = 128;
    }
    return;
  }
  uint8_t count = 257 - m_Operator;
  ASSERT((uint32_t)count >= used_bytes);
  if (used_bytes == count) {
    m_SrcOffset++;
    GetNextOperator();
    return;
  }
  count -= used_bytes;
  m_Operator = 257 - count;
}

}  // namespace

// static
std::unique_ptr<ScanlineDecoder> BasicModule::CreateRunLengthDecoder(
    pdfium::span<const uint8_t> src_buf,
    int width,
    int height,
    int nComps,
    int bpc) {
  auto pDecoder = pdfium::MakeUnique<RLScanlineDecoder>();
  if (!pDecoder->Create(src_buf, width, height, nComps, bpc))
    return nullptr;

  return std::move(pDecoder);
}

// static
bool BasicModule::RunLengthEncode(
    pdfium::span<const uint8_t> src_span,
    std::unique_ptr<uint8_t, FxFreeDeleter>* dest_buf,
    uint32_t* dest_size) {
  // Check inputs
  if (src_span.empty() || !dest_buf || !dest_size)
    return false;

  // Edge case
  if (src_span.size() == 1) {
    *dest_size = 3;
    dest_buf->reset(FX_Alloc(uint8_t, *dest_size));
    auto dest_buf_span = pdfium::make_span(dest_buf->get(), *dest_size);
    dest_buf_span[0] = 0;
    dest_buf_span[1] = src_span[0];
    dest_buf_span[2] = 128;
    return true;
  }

  // Worst case: 1 nonmatch, 2 match, 1 nonmatch, 2 match, etc. This becomes
  // 4 output chars for every 3 input, plus up to 4 more for the 1-2 chars
  // rounded off plus the terminating character.
  FX_SAFE_SIZE_T estimated_size = src_span.size();
  estimated_size += 2;
  estimated_size /= 3;
  estimated_size *= 4;
  estimated_size += 1;
  dest_buf->reset(FX_Alloc(uint8_t, estimated_size.ValueOrDie()));

  // Set up pointers.
  uint8_t* out = dest_buf->get();
  uint32_t run_start = 0;
  uint32_t run_end = 1;
  uint8_t x = src_span[run_start];
  uint8_t y = src_span[run_end];
  while (run_end < src_span.size()) {
    size_t max_len = std::min<size_t>(128, src_span.size() - run_start);
    while (x == y && (run_end - run_start < max_len - 1))
      y = src_span[++run_end];

    // Reached end with matched run. Update variables to expected values.
    if (x == y) {
      run_end++;
      if (run_end < src_span.size())
        y = src_span[run_end];
    }
    if (run_end - run_start > 1) {  // Matched run but not at end of input.
      out[0] = 257 - (run_end - run_start);
      out[1] = x;
      x = y;
      run_start = run_end;
      run_end++;
      if (run_end < src_span.size())
        y = src_span[run_end];
      out += 2;
      continue;
    }
    // Mismatched run
    while (x != y && run_end <= run_start + max_len) {
      out[run_end - run_start] = x;
      x = y;
      run_end++;
      if (run_end == src_span.size()) {
        if (run_end <= run_start + max_len) {
          out[run_end - run_start] = x;
          run_end++;
        }
        break;
      }
      y = src_span[run_end];
    }
    out[0] = run_end - run_start - 2;
    out += run_end - run_start;
    run_start = run_end - 1;
  }
  if (run_start < src_span.size()) {  // 1 leftover character
    out[0] = 0;
    out[1] = x;
    out += 2;
  }
  *out = 128;
  *dest_size = out + 1 - dest_buf->get();
  return true;
}

// static
bool BasicModule::A85Encode(pdfium::span<const uint8_t> src_span,
                            std::unique_ptr<uint8_t, FxFreeDeleter>* dest_buf,
                            uint32_t* dest_size) {
  // Check inputs.
  if (!dest_buf || !dest_size)
    return false;

  if (src_span.empty()) {
    *dest_size = 0;
    return false;
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
  dest_buf->reset(FX_Alloc(uint8_t, estimated_size.ValueOrDie()));

  // Set up pointers.
  uint8_t* out = dest_buf->get();
  uint32_t pos = 0;
  uint32_t line_length = 0;
  while (src_span.size() >= 4 && pos < src_span.size() - 3) {
    uint32_t val = ((uint32_t)(src_span[pos]) << 24) +
                   ((uint32_t)(src_span[pos + 1]) << 16) +
                   ((uint32_t)(src_span[pos + 2]) << 8) +
                   (uint32_t)(src_span[pos + 3]);
    pos += 4;
    if (val == 0) {  // All zero special case
      *out = 'z';
      out++;
      line_length++;
    } else {  // Compute base 85 characters and add 33.
      for (int i = 4; i >= 0; i--) {
        out[i] = (uint8_t)(val % 85) + 33;
        val = val / 85;
      }
      out += 5;
      line_length += 5;
    }
    if (line_length >= 75) {  // Add a return.
      *out++ = '\r';
      *out++ = '\n';
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
      if (i <= count)
        out[i] = (uint8_t)(val % 85) + 33;
      val = val / 85;
    }
    out += count + 1;
  }

  // Terminating characters.
  out[0] = '~';
  out[1] = '>';
  out += 2;
  *dest_size = out - dest_buf->get();
  return true;
}

}  // namespace fxcodec
