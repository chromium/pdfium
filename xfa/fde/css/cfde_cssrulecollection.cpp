// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssrulecollection.h"

#include <algorithm>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fde/css/cfde_cssdeclaration.h"
#include "xfa/fde/css/cfde_cssselector.h"
#include "xfa/fde/css/cfde_cssstylerule.h"
#include "xfa/fde/css/cfde_cssstylesheet.h"
#include "xfa/fde/css/cfde_csssyntaxparser.h"

CFDE_CSSRuleCollection::CFDE_CSSRuleCollection() : m_iSelectors(0) {}

CFDE_CSSRuleCollection::~CFDE_CSSRuleCollection() {
  Clear();
}

void CFDE_CSSRuleCollection::Clear() {
  m_TagRules.clear();
  m_iSelectors = 0;
}

const std::vector<std::unique_ptr<CFDE_CSSRuleCollection::Data>>*
CFDE_CSSRuleCollection::GetTagRuleData(const CFX_WideString& tagname) const {
  auto it = m_TagRules.find(FX_HashCode_GetW(tagname.c_str(), true));
  return it != m_TagRules.end() ? &it->second : nullptr;
}

void CFDE_CSSRuleCollection::AddRulesFrom(const CFDE_CSSStyleSheet* sheet,
                                          CFGAS_FontMgr* pFontMgr) {
  int32_t iRules = sheet->CountRules();
  for (int32_t j = 0; j < iRules; j++)
    AddRulesFrom(sheet, sheet->GetRule(j), pFontMgr);
}

void CFDE_CSSRuleCollection::AddRulesFrom(const CFDE_CSSStyleSheet* pStyleSheet,
                                          CFDE_CSSStyleRule* pStyleRule,
                                          CFGAS_FontMgr* pFontMgr) {
  CFDE_CSSDeclaration* pDeclaration = pStyleRule->GetDeclaration();
  int32_t iSelectors = pStyleRule->CountSelectorLists();
  for (int32_t i = 0; i < iSelectors; ++i) {
    CFDE_CSSSelector* pSelector = pStyleRule->GetSelectorList(i);
    m_TagRules[pSelector->GetNameHash()].push_back(
        pdfium::MakeUnique<Data>(pSelector, pDeclaration));
    m_iSelectors++;
  }
}

CFDE_CSSRuleCollection::Data::Data(CFDE_CSSSelector* pSel,
                                   CFDE_CSSDeclaration* pDecl)
    : pSelector(pSel), pDeclaration(pDecl) {}
