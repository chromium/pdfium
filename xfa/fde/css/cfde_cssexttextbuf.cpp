// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssexttextbuf.h"

CFDE_CSSExtTextBuf::CFDE_CSSExtTextBuf()
    : m_pExtBuffer(nullptr), m_iDatLen(0), m_iDatPos(0) {}

CFDE_CSSExtTextBuf::~CFDE_CSSExtTextBuf() {}

void CFDE_CSSExtTextBuf::AttachBuffer(const wchar_t* pBuffer, int32_t iBufLen) {
  m_pExtBuffer = pBuffer;
  m_iDatLen = iBufLen;
}
