// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_csstextbuf.h"

#include "core/fxcrt/fx_memory.h"

CFDE_CSSTextBuf::CFDE_CSSTextBuf()
    : m_pBuffer(nullptr), m_iBufLen(0), m_iDatLen(0) {}

CFDE_CSSTextBuf::~CFDE_CSSTextBuf() {
  FX_Free(m_pBuffer);
  m_pBuffer = nullptr;
  m_iDatLen = m_iBufLen;
}

void CFDE_CSSTextBuf::InitWithSize(int32_t iAllocSize) {
  ExpandBuf(iAllocSize);
}

void CFDE_CSSTextBuf::AppendChar(wchar_t wch) {
  if (m_iDatLen >= m_iBufLen)
    ExpandBuf(m_iBufLen * 2);

  m_pBuffer[m_iDatLen++] = wch;
}

int32_t CFDE_CSSTextBuf::TrimEnd() {
  while (m_iDatLen > 0 && m_pBuffer[m_iDatLen - 1] <= ' ')
    --m_iDatLen;
  AppendChar(0);
  return --m_iDatLen;
}

void CFDE_CSSTextBuf::ExpandBuf(int32_t iDesiredSize) {
  ASSERT(iDesiredSize > 0);
  if (m_pBuffer && m_iBufLen == iDesiredSize)
    return;

  if (m_pBuffer)
    m_pBuffer = FX_Realloc(wchar_t, m_pBuffer, iDesiredSize);
  else
    m_pBuffer = FX_Alloc(wchar_t, iDesiredSize);

  m_iBufLen = iDesiredSize;
}
