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
  CFDE_CSSValueListParser(const FX_WCHAR* psz,
                          int32_t iLen,
                          FX_WCHAR separator);

  bool NextValue(FDE_CSSPrimitiveType& eType,
                 const FX_WCHAR*& pStart,
                 int32_t& iLength);

  FX_WCHAR m_Separator;

 private:
  int32_t SkipTo(FX_WCHAR wch, bool breakOnSpace, bool matchBrackets);

  const FX_WCHAR* m_pCur;
  const FX_WCHAR* m_pEnd;
};

#endif  // XFA_FDE_CSS_CFDE_CSSVALUELISTPARSER_H_
