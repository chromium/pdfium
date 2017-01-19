// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_csstextbuf.h"

#include <algorithm>

CFDE_CSSTextBuf::CFDE_CSSTextBuf()
    : m_bExtBuf(false),
      m_pBuffer(nullptr),
      m_iBufLen(0),
      m_iDatLen(0),
      m_iDatPos(0) {}

CFDE_CSSTextBuf::~CFDE_CSSTextBuf() {
  Reset();
}

void CFDE_CSSTextBuf::Reset() {
  if (!m_bExtBuf) {
    FX_Free(m_pBuffer);
    m_pBuffer = nullptr;
  }
  m_iDatPos = m_iDatLen = m_iBufLen;
}

bool CFDE_CSSTextBuf::AttachBuffer(const FX_WCHAR* pBuffer, int32_t iBufLen) {
  Reset();
  m_pBuffer = const_cast<FX_WCHAR*>(pBuffer);
  m_iDatLen = m_iBufLen = iBufLen;
  return m_bExtBuf = true;
}

bool CFDE_CSSTextBuf::EstimateSize(int32_t iAllocSize) {
  ASSERT(iAllocSize > 0);
  Clear();
  m_bExtBuf = false;
  return ExpandBuf(iAllocSize);
}

int32_t CFDE_CSSTextBuf::LoadFromStream(
    const CFX_RetainPtr<IFGAS_Stream>& pTxtStream,
    int32_t iStreamOffset,
    int32_t iMaxChars,
    bool& bEOS) {
  ASSERT(iStreamOffset >= 0 && iMaxChars > 0);
  Clear();
  m_bExtBuf = false;
  if (!ExpandBuf(iMaxChars))
    return 0;

  if (pTxtStream->GetPosition() != iStreamOffset)
    pTxtStream->Seek(FX_STREAMSEEK_Begin, iStreamOffset);

  m_iDatLen = pTxtStream->ReadString(m_pBuffer, iMaxChars, bEOS);
  return m_iDatLen;
}

bool CFDE_CSSTextBuf::ExpandBuf(int32_t iDesiredSize) {
  if (m_bExtBuf)
    return false;
  if (!m_pBuffer)
    m_pBuffer = FX_Alloc(FX_WCHAR, iDesiredSize);
  else if (m_iBufLen != iDesiredSize)
    m_pBuffer = FX_Realloc(FX_WCHAR, m_pBuffer, iDesiredSize);
  else
    return true;

  if (!m_pBuffer) {
    m_iBufLen = 0;
    return false;
  }
  m_iBufLen = iDesiredSize;
  return true;
}

void CFDE_CSSTextBuf::Subtract(int32_t iStart, int32_t iLength) {
  ASSERT(iStart >= 0 && iLength >= 0);

  iLength = std::max(std::min(iLength, m_iDatLen - iStart), 0);
  FXSYS_memmove(m_pBuffer, m_pBuffer + iStart, iLength * sizeof(FX_WCHAR));
  m_iDatLen = iLength;
}
