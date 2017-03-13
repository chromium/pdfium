// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSVALUELISTPARSER_H_
#define XFA_FDE_CSS_CFDE_CSSVALUELISTPARSER_H_

#include "core/fxcrt/fx_system.h"
#include "xfa/fde/css/fde_css.h"

class CFDE_CSSValueListParser {
 public:
  CFDE_CSSValueListParser(const wchar_t* psz, int32_t iLen, wchar_t separator);

  bool NextValue(FDE_CSSPrimitiveType& eType,
                 const wchar_t*& pStart,
                 int32_t& iLength);

  wchar_t m_Separator;

 private:
  int32_t SkipTo(wchar_t wch, bool breakOnSpace, bool matchBrackets);

  const wchar_t* m_pCur;
  const wchar_t* m_pEnd;
};

#endif  // XFA_FDE_CSS_CFDE_CSSVALUELISTPARSER_H_
