// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSMEDIARULE_H_
#define XFA_FDE_CSS_CFDE_CSSMEDIARULE_H_

#include <memory>
#include <vector>

#include "xfa/fde/css/cfde_cssrule.h"

class CFDE_CSSMediaRule : public CFDE_CSSRule {
 public:
  explicit CFDE_CSSMediaRule(uint32_t dwMediaList);
  ~CFDE_CSSMediaRule() override;

  uint32_t GetMediaList() const;
  int32_t CountRules() const;
  CFDE_CSSRule* GetRule(int32_t index);

  std::vector<std::unique_ptr<CFDE_CSSRule>>& GetArray() { return m_RuleArray; }

 protected:
  uint32_t m_dwMediaList;
  std::vector<std::unique_ptr<CFDE_CSSRule>> m_RuleArray;
};

#endif  // XFA_FDE_CSS_CFDE_CSSMEDIARULE_H_
