// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_css.h"

FDE_CSSVALUETYPE IFDE_CSSPrimitiveValue::GetType() const {
  return FDE_CSSVALUETYPE_Primitive;
}

FDE_CSSVALUETYPE IFDE_CSSValueList::GetType() const {
  return FDE_CSSVALUETYPE_List;
}

FDE_CSSRuleType IFDE_CSSStyleRule::GetType() const {
  return FDE_CSSRuleType::Style;
}

FDE_CSSRuleType IFDE_CSSMediaRule::GetType() const {
  return FDE_CSSRuleType::Media;
}

FDE_CSSRuleType IFDE_CSSFontFaceRule::GetType() const {
  return FDE_CSSRuleType::FontFace;
}
