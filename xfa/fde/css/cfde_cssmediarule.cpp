// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssmediarule.h"

#include "third_party/base/stl_util.h"

CFDE_CSSMediaRule::CFDE_CSSMediaRule(uint32_t dwMediaList)
    : CFDE_CSSRule(FDE_CSSRuleType::Media), m_dwMediaList(dwMediaList) {}

CFDE_CSSMediaRule::~CFDE_CSSMediaRule() {}

uint32_t CFDE_CSSMediaRule::GetMediaList() const {
  return m_dwMediaList;
}

int32_t CFDE_CSSMediaRule::CountRules() const {
  return pdfium::CollectionSize<int32_t>(m_RuleArray);
}

CFDE_CSSRule* CFDE_CSSMediaRule::GetRule(int32_t index) {
  return m_RuleArray[index].get();
}
