// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_csstextbuf.h"

#include "third_party/base/stl_util.h"

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

bool CFDE_CSSTextBuf::AttachBuffer(const wchar_t* pBuffer, int32_t iBufLen) {
  Reset();
  m_pBuffer = const_cast<wchar_t*>(pBuffer);
  m_iDatLen = m_iBufLen = iBufLen;
  return m_bExtBuf = true;
}

bool CFDE_CSSTextBuf::EstimateSize(int32_t iAllocSize) {
  ASSERT(iAllocSize > 0);
  Clear();
  m_bExtBuf = false;
  return ExpandBuf(iAllocSize);
}

bool CFDE_CSSTextBuf::ExpandBuf(int32_t iDesiredSize) {
  if (m_bExtBuf)
    return false;
  if (!m_pBuffer)
    m_pBuffer = FX_Alloc(wchar_t, iDesiredSize);
  else if (m_iBufLen != iDesiredSize)
    m_pBuffer = FX_Realloc(wchar_t, m_pBuffer, iDesiredSize);
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

  iLength = pdfium::clamp(iLength, 0, m_iDatLen - iStart);
  memmove(m_pBuffer, m_pBuffer + iStart, iLength * sizeof(wchar_t));
  m_iDatLen = iLength;
}
