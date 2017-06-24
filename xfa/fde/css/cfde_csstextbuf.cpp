// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_csstextbuf.h"

#include "third_party/base/stl_util.h"

CFDE_CSSTextBuf::CFDE_CSSTextBuf()
    : m_bExtBuf(false),
      m_pExtBuffer(nullptr),
      m_pBuffer(nullptr),
      m_iBufLen(0),
      m_iDatLen(0),
      m_iDatPos(0) {}

CFDE_CSSTextBuf::~CFDE_CSSTextBuf() {
  Reset();
}

void CFDE_CSSTextBuf::AttachBuffer(const wchar_t* pBuffer, int32_t iBufLen) {
  Reset();
  m_pExtBuffer = pBuffer;
  m_iDatLen = m_iBufLen = iBufLen;
  m_bExtBuf = true;
}

bool CFDE_CSSTextBuf::EstimateSize(int32_t iAllocSize) {
  ASSERT(iAllocSize > 0);
  Clear();
  m_bExtBuf = false;
  return ExpandBuf(iAllocSize);
}

bool CFDE_CSSTextBuf::AppendChar(wchar_t wch) {
  if (m_iDatLen >= m_iBufLen && !ExpandBuf(m_iBufLen * 2))
    return false;

  ASSERT(!m_bExtBuf);
  m_pBuffer[m_iDatLen++] = wch;
  return true;
}

void CFDE_CSSTextBuf::Reset() {
  if (!m_bExtBuf) {
    FX_Free(m_pBuffer);
    m_pBuffer = nullptr;
  }
  m_iDatPos = m_iDatLen = m_iBufLen;
}

int32_t CFDE_CSSTextBuf::TrimEnd() {
  ASSERT(!m_bExtBuf);
  while (m_iDatLen > 0 && m_pBuffer[m_iDatLen - 1] <= ' ')
    --m_iDatLen;
  AppendChar(0);
  return --m_iDatLen;
}

const wchar_t* CFDE_CSSTextBuf::GetBuffer() const {
  return m_bExtBuf ? m_pExtBuffer : m_pBuffer;
}

bool CFDE_CSSTextBuf::ExpandBuf(int32_t iDesiredSize) {
  if (m_bExtBuf)
    return false;

  if (m_pBuffer && m_iBufLen == iDesiredSize)
    return true;

  if (m_pBuffer)
    m_pBuffer = FX_Realloc(wchar_t, m_pBuffer, iDesiredSize);
  else
    m_pBuffer = FX_Alloc(wchar_t, iDesiredSize);

  m_iBufLen = iDesiredSize;
  return true;
}

void CFDE_CSSTextBuf::Subtract(int32_t iStart, int32_t iLength) {
  ASSERT(iStart >= 0 && iLength >= 0);
  ASSERT(!m_bExtBuf);

  iLength = pdfium::clamp(iLength, 0, m_iDatLen - iStart);
  memmove(m_pBuffer, m_pBuffer + iStart, iLength * sizeof(wchar_t));
  m_iDatLen = iLength;
}
