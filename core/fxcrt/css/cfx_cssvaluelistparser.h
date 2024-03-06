// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CSS_CFX_CSSVALUELISTPARSER_H_
#define CORE_FXCRT_CSS_CFX_CSSVALUELISTPARSER_H_

#include <stdint.h>

#include <optional>

#include "core/fxcrt/css/cfx_cssvalue.h"
#include "core/fxcrt/widestring.h"

class CFX_CSSValueListParser {
 public:
  struct Result {
    CFX_CSSValue::PrimitiveType type = CFX_CSSValue::PrimitiveType::kUnknown;
    WideStringView string_view;
  };

  CFX_CSSValueListParser(WideStringView list, wchar_t separator);

  std::optional<Result> NextValue();
  void UseCommaSeparator() { m_Separator = ','; }

 private:
  size_t SkipToChar(wchar_t wch);
  size_t SkipToCharMatchingParens(wchar_t wch);

  wchar_t m_Separator;
  const wchar_t* m_pCur;
  const wchar_t* m_pEnd;
};

#endif  // CORE_FXCRT_CSS_CFX_CSSVALUELISTPARSER_H_
