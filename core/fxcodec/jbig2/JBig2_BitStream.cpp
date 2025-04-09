// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jbig2/JBig2_BitStream.h"

#include <algorithm>

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/numerics/safe_conversions.h"

namespace {

pdfium::span<const uint8_t> ValidatedSpan(pdfium::span<const uint8_t> sp) {
  if (sp.size() > 256 * 1024 * 1024) {
    return {};
  }
  return sp;
}

}  // namespace

CJBig2_BitStream::CJBig2_BitStream(pdfium::span<const uint8_t> pSrcStream,
                                   uint64_t key)
    : span_(ValidatedSpan(pSrcStream)), key_(key) {}

CJBig2_BitStream::~CJBig2_BitStream() = default;

int32_t CJBig2_BitStream::readNBits(uint32_t dwBits, uint32_t* dwResult) {
  if (!IsInBounds()) {
    return -1;
  }

  uint32_t dwBitPos = getBitPos();
  if (dwBitPos > LengthInBits()) {
    return -1;
  }

  *dwResult = 0;
  if (dwBitPos + dwBits <= LengthInBits()) {
    dwBitPos = dwBits;
  } else {
    dwBitPos = LengthInBits() - dwBitPos;
  }

  for (; dwBitPos > 0; --dwBitPos) {
    *dwResult =
        (*dwResult << 1) | ((span_[byte_idx_] >> (7 - bit_idx_)) & 0x01);
    AdvanceBit();
  }
  return 0;
}

int32_t CJBig2_BitStream::readNBits(uint32_t dwBits, int32_t* nResult) {
  if (!IsInBounds()) {
    return -1;
  }

  uint32_t dwBitPos = getBitPos();
  if (dwBitPos > LengthInBits()) {
    return -1;
  }

  *nResult = 0;
  if (dwBitPos + dwBits <= LengthInBits()) {
    dwBitPos = dwBits;
  } else {
    dwBitPos = LengthInBits() - dwBitPos;
  }

  for (; dwBitPos > 0; --dwBitPos) {
    *nResult = (*nResult << 1) | ((span_[byte_idx_] >> (7 - bit_idx_)) & 0x01);
    AdvanceBit();
  }
  return 0;
}

int32_t CJBig2_BitStream::read1Bit(uint32_t* dwResult) {
  if (!IsInBounds()) {
    return -1;
  }

  *dwResult = (span_[byte_idx_] >> (7 - bit_idx_)) & 0x01;
  AdvanceBit();
  return 0;
}

int32_t CJBig2_BitStream::read1Bit(bool* bResult) {
  if (!IsInBounds()) {
    return -1;
  }

  *bResult = (span_[byte_idx_] >> (7 - bit_idx_)) & 0x01;
  AdvanceBit();
  return 0;
}

int32_t CJBig2_BitStream::read1Byte(uint8_t* cResult) {
  if (!IsInBounds()) {
    return -1;
  }

  *cResult = span_[byte_idx_];
  ++byte_idx_;
  return 0;
}

int32_t CJBig2_BitStream::readInteger(uint32_t* dwResult) {
  if (byte_idx_ + 3 >= span_.size()) {
    return -1;
  }

  *dwResult = (span_[byte_idx_] << 24) | (span_[byte_idx_ + 1] << 16) |
              (span_[byte_idx_ + 2] << 8) | span_[byte_idx_ + 3];
  byte_idx_ += 4;
  return 0;
}

int32_t CJBig2_BitStream::readShortInteger(uint16_t* dwResult) {
  if (byte_idx_ + 1 >= span_.size()) {
    return -1;
  }

  *dwResult = (span_[byte_idx_] << 8) | span_[byte_idx_ + 1];
  byte_idx_ += 2;
  return 0;
}

void CJBig2_BitStream::alignByte() {
  if (bit_idx_ != 0) {
    addOffset(1);
    bit_idx_ = 0;
  }
}

uint8_t CJBig2_BitStream::getCurByte() const {
  return IsInBounds() ? span_[byte_idx_] : 0;
}

void CJBig2_BitStream::incByteIdx() {
  addOffset(1);
}

uint8_t CJBig2_BitStream::getCurByte_arith() const {
  return IsInBounds() ? span_[byte_idx_] : 0xFF;
}

uint8_t CJBig2_BitStream::getNextByte_arith() const {
  return byte_idx_ + 1 < span_.size() ? span_[byte_idx_ + 1] : 0xFF;
}

uint32_t CJBig2_BitStream::getOffset() const {
  return byte_idx_;
}

void CJBig2_BitStream::setOffset(uint32_t dwOffset) {
  byte_idx_ =
      std::min(dwOffset, pdfium::checked_cast<uint32_t>(getBufSpan().size()));
}

void CJBig2_BitStream::addOffset(uint32_t dwDelta) {
  FX_SAFE_UINT32 new_offset = byte_idx_;
  new_offset += dwDelta;
  if (new_offset.IsValid()) {
    setOffset(new_offset.ValueOrDie());
  }
}

uint32_t CJBig2_BitStream::getBitPos() const {
  return (byte_idx_ << 3) + bit_idx_;
}

void CJBig2_BitStream::setBitPos(uint32_t dwBitPos) {
  byte_idx_ = dwBitPos >> 3;
  bit_idx_ = dwBitPos & 7;
}

const uint8_t* CJBig2_BitStream::getPointer() const {
  return span_.subspan(byte_idx_).data();
}

uint32_t CJBig2_BitStream::getByteLeft() const {
  FX_SAFE_UINT32 result = getBufSpan().size();
  result -= byte_idx_;
  return result.ValueOrDie();
}

void CJBig2_BitStream::AdvanceBit() {
  if (bit_idx_ == 7) {
    ++byte_idx_;
    bit_idx_ = 0;
  } else {
    ++bit_idx_;
  }
}

bool CJBig2_BitStream::IsInBounds() const {
  return byte_idx_ < getBufSpan().size();
}

uint32_t CJBig2_BitStream::LengthInBits() const {
  FX_SAFE_UINT32 result = getBufSpan().size();
  result *= 8;
  return result.ValueOrDie();
}
