// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSEXTTEXTBUF_H_
#define CORE_FXCRT_CSS_CFX_CSSEXTTEXTBUF_H_

#include "core/fxcrt/fx_string.h"

class CFX_CSSExtTextBuf {
 public:
  explicit CFX_CSSExtTextBuf(WideStringView str);
  ~CFX_CSSExtTextBuf();

  bool IsEOF() const { return m_iPos >= m_Buffer.GetLength(); }
  void MoveNext() { m_iPos++; }
  wchar_t GetChar() const { return m_Buffer[m_iPos]; }
  wchar_t GetNextChar() const {
    return m_iPos + 1 < m_Buffer.GetLength() ? m_Buffer[m_iPos + 1] : 0;
  }

 protected:
  const WideStringView m_Buffer;
  size_t m_iPos = 0;
};

#endif  // CORE_FXCRT_CSS_CFX_CSSEXTTEXTBUF_H_
