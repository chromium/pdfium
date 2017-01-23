// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSSTYLERULE_H_
#define XFA_FDE_CSS_CFDE_CSSSTYLERULE_H_

#include <memory>
#include <vector>

#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/cfde_cssselector.h"

class CFDE_CSSStyleRule {
 public:
  CFDE_CSSStyleRule();
  ~CFDE_CSSStyleRule();

  size_t CountSelectorLists() const;
  CFDE_CSSSelector* GetSelectorList(int32_t index) const;
  CFDE_CSSDeclaration* GetDeclaration();

  void SetSelector(std::vector<std::unique_ptr<CFDE_CSSSelector>>* list);

 private:
  CFDE_CSSDeclaration m_Declaration;
  std::vector<std::unique_ptr<CFDE_CSSSelector>> m_ppSelector;
};

#endif  // XFA_FDE_CSS_CFDE_CSSSTYLERULE_H_
