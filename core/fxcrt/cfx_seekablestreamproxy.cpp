// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_seekablestreamproxy.h"

#include <stdint.h>

#include <algorithm>
#include <limits>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/span_util.h"

namespace {

// Returns {src bytes consumed, dst chars produced}.
// Invalid sequences are silently not output.
std::pair<size_t, size_t> UTF8Decode(pdfium::span<const uint8_t> pSrc,
                                     pdfium::span<wchar_t> pDst) {
  DCHECK(!pDst.empty());

  uint32_t dwCode = 0;
  int32_t iPending = 0;
  size_t iSrcNum = 0;
  size_t iDstNum = 0;
  for (size_t iIndex = 0; iIndex < pSrc.size() && iDstNum < pDst.size();
       ++iIndex) {
    ++iSrcNum;
    uint8_t byte = pSrc[iIndex];
    if (byte < 0x80) {
      iPending = 0;
      pDst[iDstNum++] = byte;
    } else if (byte < 0xc0) {
      if (iPending < 1) {
        continue;
      }

      dwCode = dwCode << 6;
      dwCode |= (byte & 0x3f);
      --iPending;
      if (iPending == 0) {
        pDst[iDstNum++] = dwCode;
      }
    } else if (byte < 0xe0) {
      iPending = 1;
      dwCode = (byte & 0x1f);
    } else if (byte < 0xf0) {
      iPending = 2;
      dwCode = (byte & 0x0f);
    } else if (byte < 0xf8) {
      iPending = 3;
      dwCode = (byte & 0x07);
    } else if (byte < 0xfc) {
      iPending = 4;
      dwCode = (byte & 0x03);
    } else if (byte < 0xfe) {
      iPending = 5;
      dwCode = (byte & 0x01);
    }
  }
  return {iSrcNum, iDstNum};
}

void UTF16ToWChar(pdfium::span<wchar_t> buffer) {
#if defined(WCHAR_T_IS_32_BIT)
  auto src = fxcrt::reinterpret_span<uint16_t>(buffer);
  // Perform self-intersecting copy in reverse order.
  for (size_t i = buffer.size(); i > 0; --i) {
    buffer[i - 1] = static_cast<wchar_t>(src[i - 1]);
  }
#endif  // defined(WCHAR_T_IS_32_BIT)
}

void SwapByteOrder(pdfium::span<uint16_t> str) {
  for (auto& wch : str) {
    wch = (wch >> 8) | (wch << 8);
  }
}

}  // namespace

#define BOM_UTF8_MASK 0x00FFFFFF
#define BOM_UTF8 0x00BFBBEF
#define BOM_UTF16_MASK 0x0000FFFF
#define BOM_UTF16_BE 0x0000FFFE
#define BOM_UTF16_LE 0x0000FEFF

CFX_SeekableStreamProxy::CFX_SeekableStreamProxy(
    const RetainPtr<IFX_SeekableReadStream>& stream)
    : stream_(stream) {
  DCHECK(stream_);

  Seek(From::Begin, 0);

  uint32_t bom = 0;
  ReadData(pdfium::byte_span_from_ref(bom).first<3>());

  bom &= BOM_UTF8_MASK;
  if (bom == BOM_UTF8) {
    bom_length_ = 3;
    code_page_ = FX_CodePage::kUTF8;
  } else {
    bom &= BOM_UTF16_MASK;
    if (bom == BOM_UTF16_BE) {
      bom_length_ = 2;
      code_page_ = FX_CodePage::kUTF16BE;
    } else if (bom == BOM_UTF16_LE) {
      bom_length_ = 2;
      code_page_ = FX_CodePage::kUTF16LE;
    } else {
      bom_length_ = 0;
      code_page_ = FX_GetACP();
    }
  }

  Seek(From::Begin, static_cast<FX_FILESIZE>(bom_length_));
}

CFX_SeekableStreamProxy::~CFX_SeekableStreamProxy() = default;

FX_FILESIZE CFX_SeekableStreamProxy::GetSize() const {
  return stream_->GetSize();
}

FX_FILESIZE CFX_SeekableStreamProxy::GetPosition() const {
  return position_;
}

bool CFX_SeekableStreamProxy::IsEOF() const {
  return position_ >= GetSize();
}

void CFX_SeekableStreamProxy::Seek(From eSeek, FX_FILESIZE iOffset) {
  switch (eSeek) {
    case From::Begin:
      position_ = iOffset;
      break;
    case From::Current: {
      FX_SAFE_FILESIZE new_pos = position_;
      new_pos += iOffset;
      position_ =
          new_pos.ValueOrDefault(std::numeric_limits<FX_FILESIZE>::max());
    } break;
  }
  position_ = std::clamp(position_, static_cast<FX_FILESIZE>(0), GetSize());
}

void CFX_SeekableStreamProxy::SetCodePage(FX_CodePage wCodePage) {
  if (bom_length_ > 0) {
    return;
  }
  code_page_ = wCodePage;
}

size_t CFX_SeekableStreamProxy::ReadData(pdfium::span<uint8_t> buffer) {
  DCHECK(!buffer.empty());
  const size_t remaining = static_cast<size_t>(GetSize() - position_);
  size_t read_size = std::min(buffer.size(), remaining);
  if (read_size == 0) {
    return 0;
  }
  if (!stream_->ReadBlockAtOffset(buffer.first(read_size), position_)) {
    return 0;
  }
  FX_SAFE_FILESIZE new_pos = position_;
  new_pos += read_size;
  position_ = new_pos.ValueOrDefault(position_);
  return new_pos.IsValid() ? read_size : 0;
}

size_t CFX_SeekableStreamProxy::ReadBlock(pdfium::span<wchar_t> buffer) {
  if (buffer.empty()) {
    return 0;
  }
  if (code_page_ == FX_CodePage::kUTF16LE ||
      code_page_ == FX_CodePage::kUTF16BE) {
    size_t bytes_to_read = buffer.size() * sizeof(uint16_t);
    size_t bytes_read =
        ReadData(pdfium::as_writable_bytes(buffer).first(bytes_to_read));
    size_t elements = bytes_read / sizeof(uint16_t);
    if (code_page_ == FX_CodePage::kUTF16BE) {
      SwapByteOrder(fxcrt::reinterpret_span<uint16_t>(buffer).first(elements));
    }
    UTF16ToWChar(buffer.first(elements));
    return elements;
  }
  FX_FILESIZE pos = GetPosition();
  size_t bytes_to_read =
      std::min(buffer.size(), static_cast<size_t>(GetSize() - pos));
  if (bytes_to_read == 0) {
    return 0;
  }
  DataVector<uint8_t> byte_buf(bytes_to_read);
  size_t bytes_read = ReadData(byte_buf);
  if (code_page_ != FX_CodePage::kUTF8) {
    return 0;
  }
  auto [src_bytes_consumed, dest_wchars_produced] =
      UTF8Decode(pdfium::span(byte_buf).first(bytes_read), buffer);
  Seek(From::Current, src_bytes_consumed - bytes_read);
  return dest_wchars_produced;
}
