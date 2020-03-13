// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSTEXTBUF_H_
#define CORE_FXCRT_CSS_CFX_CSSTEXTBUF_H_

#include <vector>

#include "core/fxcrt/fx_string.h"

class CFX_CSSTextBuf {
 public:
  CFX_CSSTextBuf();
  ~CFX_CSSTextBuf();

  void Clear() { m_Buffer.clear(); }
  bool IsEmpty() const { return m_Buffer.empty(); }
  void AppendCharIfNotLeadingBlank(wchar_t wch);
  WideStringView GetTrailingBlankTrimmedString() const;

 protected:
  std::vector<wchar_t> m_Buffer;
};

#endif  // CORE_FXCRT_CSS_CFX_CSSTEXTBUF_H_
