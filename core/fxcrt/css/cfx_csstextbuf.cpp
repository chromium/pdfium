// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_csstextbuf.h"

CFX_CSSTextBuf::CFX_CSSTextBuf() {
  m_Buffer.reserve(32);
}

CFX_CSSTextBuf::~CFX_CSSTextBuf() = default;

void CFX_CSSTextBuf::AppendCharIfNotLeadingBlank(wchar_t wch) {
  if (m_Buffer.empty() && wch <= ' ')
    return;

  m_Buffer.push_back(wch);
}

WideStringView CFX_CSSTextBuf::GetTrailingBlankTrimmedString() const {
  WideStringView result(m_Buffer);
  while (!result.IsEmpty() && result.Back() <= ' ')
    result = result.First(result.GetLength() - 1);

  return result;
}
