// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_BitStream.h"

#include <algorithm>

#include "core/include/fpdfapi/fpdf_objects.h"

CJBig2_BitStream::CJBig2_BitStream(CPDF_StreamAcc* pSrcStream)
    : m_pBuf(pSrcStream->GetData()),
      m_dwLength(pSrcStream->GetSize()),
      m_dwByteIdx(0),
      m_dwBitIdx(0),
      m_dwObjNum(pSrcStream->GetStream() ? pSrcStream->GetStream()->GetObjNum()
                                         : 0) {
  if (m_dwLength > 256 * 1024 * 1024) {
    m_dwLength = 0;
    m_pBuf = nullptr;
  }
}

CJBig2_BitStream::~CJBig2_BitStream() {
}

int32_t CJBig2_BitStream::readNBits(FX_DWORD dwBits, FX_DWORD* dwResult) {
  FX_DWORD dwBitPos = getBitPos();
  if (dwBitPos > LengthInBits())
    return -1;

  *dwResult = 0;
  if (dwBitPos + dwBits <= LengthInBits())
    dwBitPos = dwBits;
  else
    dwBitPos = LengthInBits() - dwBitPos;

  for (; dwBitPos > 0; --dwBitPos) {
    *dwResult =
        (*dwResult << 1) | ((m_pBuf[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01);
    AdvanceBit();
  }
  return 0;
}

int32_t CJBig2_BitStream::readNBits(FX_DWORD dwBits, int32_t* nResult) {
  FX_DWORD dwBitPos = getBitPos();
  if (dwBitPos > LengthInBits())
    return -1;

  *nResult = 0;
  if (dwBitPos + dwBits <= LengthInBits())
    dwBitPos = dwBits;
  else
    dwBitPos = LengthInBits() - dwBitPos;

  for (; dwBitPos > 0; --dwBitPos) {
    *nResult =
        (*nResult << 1) | ((m_pBuf[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01);
    AdvanceBit();
  }
  return 0;
}

int32_t CJBig2_BitStream::read1Bit(FX_DWORD* dwResult) {
  if (!IsInBound())
    return -1;

  *dwResult = (m_pBuf[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01;
  AdvanceBit();
  return 0;
}

int32_t CJBig2_BitStream::read1Bit(FX_BOOL* bResult) {
  if (!IsInBound())
    return -1;

  *bResult = (m_pBuf[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01;
  AdvanceBit();
  return 0;
}

int32_t CJBig2_BitStream::read1Byte(uint8_t* cResult) {
  if (!IsInBound())
    return -1;

  *cResult = m_pBuf[m_dwByteIdx];
  ++m_dwByteIdx;
  return 0;
}

int32_t CJBig2_BitStream::readInteger(FX_DWORD* dwResult) {
  if (m_dwByteIdx + 3 >= m_dwLength)
    return -1;

  *dwResult = (m_pBuf[m_dwByteIdx] << 24) | (m_pBuf[m_dwByteIdx + 1] << 16) |
              (m_pBuf[m_dwByteIdx + 2] << 8) | m_pBuf[m_dwByteIdx + 3];
  m_dwByteIdx += 4;
  return 0;
}

int32_t CJBig2_BitStream::readShortInteger(FX_WORD* dwResult) {
  if (m_dwByteIdx + 1 >= m_dwLength)
    return -1;

  *dwResult = (m_pBuf[m_dwByteIdx] << 8) | m_pBuf[m_dwByteIdx + 1];
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
  return IsInBound() ? m_pBuf[m_dwByteIdx] : 0;
}

void CJBig2_BitStream::incByteIdx() {
  if (IsInBound())
    ++m_dwByteIdx;
}

uint8_t CJBig2_BitStream::getCurByte_arith() const {
  return IsInBound() ? m_pBuf[m_dwByteIdx] : 0xFF;
}

uint8_t CJBig2_BitStream::getNextByte_arith() const {
  return m_dwByteIdx + 1 < m_dwLength ? m_pBuf[m_dwByteIdx + 1] : 0xFF;
}

FX_DWORD CJBig2_BitStream::getOffset() const {
  return m_dwByteIdx;
}

void CJBig2_BitStream::setOffset(FX_DWORD dwOffset) {
  m_dwByteIdx = std::min(dwOffset, m_dwLength);
}

FX_DWORD CJBig2_BitStream::getBitPos() const {
  return (m_dwByteIdx << 3) + m_dwBitIdx;
}

void CJBig2_BitStream::setBitPos(FX_DWORD dwBitPos) {
  m_dwByteIdx = dwBitPos >> 3;
  m_dwBitIdx = dwBitPos & 7;
}

const uint8_t* CJBig2_BitStream::getBuf() const {
  return m_pBuf;
}

const uint8_t* CJBig2_BitStream::getPointer() const {
  return m_pBuf + m_dwByteIdx;
}

void CJBig2_BitStream::offset(FX_DWORD dwOffset) {
  m_dwByteIdx += dwOffset;
}

FX_DWORD CJBig2_BitStream::getByteLeft() const {
  return m_dwLength - m_dwByteIdx;
}

void CJBig2_BitStream::AdvanceBit() {
  if (m_dwBitIdx == 7) {
    ++m_dwByteIdx;
    m_dwBitIdx = 0;
  } else {
    ++m_dwBitIdx;
  }
}

bool CJBig2_BitStream::IsInBound() const {
  return m_dwByteIdx < m_dwLength;
}

FX_DWORD CJBig2_BitStream::LengthInBits() const {
  return m_dwLength << 3;
}

FX_DWORD CJBig2_BitStream::getObjNum() const {
  return m_dwObjNum;
}
