// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSVALUELIST_H_
#define XFA_FDE_CSS_CFDE_CSSVALUELIST_H_

#include <vector>

#include "xfa/fde/css/cfde_cssvalue.h"

class CFDE_CSSValueList : public CFDE_CSSValue {
 public:
  explicit CFDE_CSSValueList(std::vector<CFX_RetainPtr<CFDE_CSSValue>>& list);
  ~CFDE_CSSValueList() override;

  int32_t CountValues() const;
  CFX_RetainPtr<CFDE_CSSValue> GetValue(int32_t index) const;

 protected:
  std::vector<CFX_RetainPtr<CFDE_CSSValue>> m_ppList;
};

#endif  // XFA_FDE_CSS_CFDE_CSSVALUELIST_H_
