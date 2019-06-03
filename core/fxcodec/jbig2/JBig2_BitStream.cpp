// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jbig2/JBig2_BitStream.h"

#include <algorithm>

namespace {

pdfium::span<const uint8_t> ValidatedSpan(pdfium::span<const uint8_t> sp) {
  if (sp.size() > 256 * 1024 * 1024)
    return {};
  return sp;
}

}  // namespace

CJBig2_BitStream::CJBig2_BitStream(pdfium::span<const uint8_t> pSrcStream,
                                   uint32_t dwObjNum)
    : m_Span(ValidatedSpan(pSrcStream)), m_dwObjNum(dwObjNum) {}

CJBig2_BitStream::~CJBig2_BitStream() = default;

int32_t CJBig2_BitStream::readNBits(uint32_t dwBits, uint32_t* dwResult) {
  if (!IsInBounds())
    return -1;

  uint32_t dwBitPos = getBitPos();
  if (dwBitPos > LengthInBits())
    return -1;

  *dwResult = 0;
  if (dwBitPos + dwBits <= LengthInBits())
    dwBitPos = dwBits;
  else
    dwBitPos = LengthInBits() - dwBitPos;

  for (; dwBitPos > 0; --dwBitPos) {
    *dwResult =
        (*dwResult << 1) | ((m_Span[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01);
    AdvanceBit();
  }
  return 0;
}

int32_t CJBig2_BitStream::readNBits(uint32_t dwBits, int32_t* nResult) {
  if (!IsInBounds())
    return -1;

  uint32_t dwBitPos = getBitPos();
  if (dwBitPos > LengthInBits())
    return -1;

  *nResult = 0;
  if (dwBitPos + dwBits <= LengthInBits())
    dwBitPos = dwBits;
  else
    dwBitPos = LengthInBits() - dwBitPos;

  for (; dwBitPos > 0; --dwBitPos) {
    *nResult =
        (*nResult << 1) | ((m_Span[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01);
    AdvanceBit();
  }
  return 0;
}

int32_t CJBig2_BitStream::read1Bit(uint32_t* dwResult) {
  if (!IsInBounds())
    return -1;

  *dwResult = (m_Span[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01;
  AdvanceBit();
  return 0;
}

int32_t CJBig2_BitStream::read1Bit(bool* bResult) {
  if (!IsInBounds())
    return -1;

  *bResult = (m_Span[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01;
  AdvanceBit();
  return 0;
}

int32_t CJBig2_BitStream::read1Byte(uint8_t* cResult) {
  if (!IsInBounds())
    return -1;

  *cResult = m_Span[m_dwByteIdx];
  ++m_dwByteIdx;
  return 0;
}

int32_t CJBig2_BitStream::readInteger(uint32_t* dwResult) {
  if (m_dwByteIdx + 3 >= m_Span.size())
    return -1;

  *dwResult = (m_Span[m_dwByteIdx] << 24) | (m_Span[m_dwByteIdx + 1] << 16) |
              (m_Span[m_dwByteIdx + 2] << 8) | m_Span[m_dwByteIdx + 3];
  m_dwByteIdx += 4;
  return 0;
}

int32_t CJBig2_BitStream::readShortInteger(uint16_t* dwResult) {
  if (m_dwByteIdx + 1 >= m_Span.size())
    return -1;

  *dwResult = (m_Span[m_dwByteIdx] << 8) | m_Span[m_dwByteIdx + 1];
  m_dwByteIdx += 2;
  return 0;
}

void CJBig2_BitStream::alignByte() {
  if (m_dwBitIdx != 0) {
    ++m_dwByteIdx;
    m_dwBitIdx = 0;
  }
}

uint8_t CJBig2_BitStream::getCurByte() const {
  return IsInBounds() ? m_Span[m_dwByteIdx] : 0;
}

void CJBig2_BitStream::incByteIdx() {
  if (IsInBounds())
    ++m_dwByteIdx;
}

uint8_t CJBig2_BitStream::getCurByte_arith() const {
  return IsInBounds() ? m_Span[m_dwByteIdx] : 0xFF;
}

uint8_t CJBig2_BitStream::getNextByte_arith() const {
  return m_dwByteIdx + 1 < m_Span.size() ? m_Span[m_dwByteIdx + 1] : 0xFF;
}

uint32_t CJBig2_BitStream::getOffset() const {
  return m_dwByteIdx;
}

void CJBig2_BitStream::setOffset(uint32_t dwOffset) {
  m_dwByteIdx = std::min<size_t>(dwOffset, m_Span.size());
}

uint32_t CJBig2_BitStream::getBitPos() const {
  return (m_dwByteIdx << 3) + m_dwBitIdx;
}

void CJBig2_BitStream::setBitPos(uint32_t dwBitPos) {
  m_dwByteIdx = dwBitPos >> 3;
  m_dwBitIdx = dwBitPos & 7;
}

const uint8_t* CJBig2_BitStream::getBuf() const {
  return m_Span.data();
}

const uint8_t* CJBig2_BitStream::getPointer() const {
  return getBuf() + m_dwByteIdx;
}

void CJBig2_BitStream::offset(uint32_t dwOffset) {
  m_dwByteIdx += dwOffset;
}

uint32_t CJBig2_BitStream::getByteLeft() const {
  return m_Span.size() - m_dwByteIdx;
}

void CJBig2_BitStream::AdvanceBit() {
  if (m_dwBitIdx == 7) {
    ++m_dwByteIdx;
    m_dwBitIdx = 0;
  } else {
    ++m_dwBitIdx;
  }
}

bool CJBig2_BitStream::IsInBounds() const {
  return m_dwByteIdx < m_Span.size();
}

uint32_t CJBig2_BitStream::LengthInBits() const {
  return m_Span.size() << 3;
}

uint32_t CJBig2_BitStream::getObjNum() const {
  return m_dwObjNum;
}
