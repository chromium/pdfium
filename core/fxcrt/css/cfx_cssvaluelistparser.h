// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSVALUELISTPARSER_H_
#define CORE_FXCRT_CSS_CFX_CSSVALUELISTPARSER_H_

#include <stdint.h>

#include "core/fxcrt/css/cfx_cssvalue.h"

class CFX_CSSValueListParser {
 public:
  CFX_CSSValueListParser(const wchar_t* psz, size_t nLen, wchar_t separator);

  bool NextValue(CFX_CSSValue::PrimitiveType* eType,
                 const wchar_t** pStart,
                 size_t* nLength);
  void UseCommaSeparator() { m_Separator = ','; }

 private:
  size_t SkipToChar(wchar_t wch);
  size_t SkipToCharMatchingParens(wchar_t wch);

  wchar_t m_Separator;
  const wchar_t* m_pCur;
  const wchar_t* m_pEnd;
};

#endif  // CORE_FXCRT_CSS_CFX_CSSVALUELISTPARSER_H_
