// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_cssstyleselector.h"

#include <algorithm>
#include <map>
#include <memory>

#include "xfa/fde/css/fde_csscache.h"
#include "xfa/fde/css/fde_cssdeclaration.h"
#include "xfa/fde/css/fde_cssstylesheet.h"
#include "xfa/fde/css/fde_csssyntax.h"

#define FDE_CSSUNIVERSALHASH ('*')

void CFDE_CSSRuleCollection::Clear() {
  m_IDRules.clear();
  m_TagRules.clear();
  m_ClassRules.clear();
  m_pUniversalRules = nullptr;
  m_iSelectors = 0;
}

CFDE_CSSRuleCollection::CFDE_CSSRuleCollection()
    : m_pUniversalRules(nullptr), m_pPseudoRules(nullptr), m_iSelectors(0) {}

CFDE_CSSRuleCollection::~CFDE_CSSRuleCollection() {
  Clear();
}

void CFDE_CSSRuleCollection::AddRulesFrom(
    const CFX_ArrayTemplate<IFDE_CSSStyleSheet*>& sheets,
    uint32_t dwMediaList,
    CFGAS_FontMgr* pFontMgr) {
  int32_t iSheets = sheets.GetSize();
  for (int32_t i = 0; i < iSheets; ++i) {
    IFDE_CSSStyleSheet* pSheet = sheets.GetAt(i);
    if (uint32_t dwMatchMedia = pSheet->GetMediaList() & dwMediaList) {
      int32_t iRules = pSheet->CountRules();
      for (int32_t j = 0; j < iRules; j++) {
        AddRulesFrom(pSheet, pSheet->GetRule(j), dwMatchMedia, pFontMgr);
      }
    }
  }
}

void CFDE_CSSRuleCollection::AddRulesFrom(IFDE_CSSStyleSheet* pStyleSheet,
                                          IFDE_CSSRule* pRule,
                                          uint32_t dwMediaList,
                                          CFGAS_FontMgr* pFontMgr) {
  switch (pRule->GetType()) {
    case FDE_CSSRuleType::Style: {
      IFDE_CSSStyleRule* pStyleRule = static_cast<IFDE_CSSStyleRule*>(pRule);
      CFDE_CSSDeclaration* pDeclaration = pStyleRule->GetDeclaration();
      int32_t iSelectors = pStyleRule->CountSelectorLists();
      for (int32_t i = 0; i < iSelectors; ++i) {
        CFDE_CSSSelector* pSelector = pStyleRule->GetSelectorList(i);
        if (pSelector->GetType() == FDE_CSSSelectorType::Pseudo) {
          FDE_CSSRuleData* pData = NewRuleData(pSelector, pDeclaration);
          AddRuleTo(&m_pPseudoRules, pData);
          continue;
        }
        if (pSelector->GetNameHash() != FDE_CSSUNIVERSALHASH) {
          AddRuleTo(&m_TagRules, pSelector->GetNameHash(), pSelector,
                    pDeclaration);
          continue;
        }
        CFDE_CSSSelector* pNext = pSelector->GetNextSelector();
        if (!pNext) {
          FDE_CSSRuleData* pData = NewRuleData(pSelector, pDeclaration);
          AddRuleTo(&m_pUniversalRules, pData);
          continue;
        }
        switch (pNext->GetType()) {
          case FDE_CSSSelectorType::ID:
            AddRuleTo(&m_IDRules, pNext->GetNameHash(), pSelector,
                      pDeclaration);
            break;
          case FDE_CSSSelectorType::Class:
            AddRuleTo(&m_ClassRules, pNext->GetNameHash(), pSelector,
                      pDeclaration);
            break;
          case FDE_CSSSelectorType::Descendant:
          case FDE_CSSSelectorType::Element:
            AddRuleTo(&m_pUniversalRules, NewRuleData(pSelector, pDeclaration));
            break;
          default:
            ASSERT(false);
            break;
        }
      }
    } break;
    case FDE_CSSRuleType::Media: {
      IFDE_CSSMediaRule* pMediaRule = static_cast<IFDE_CSSMediaRule*>(pRule);
      if (pMediaRule->GetMediaList() & dwMediaList) {
        int32_t iRules = pMediaRule->CountRules();
        for (int32_t i = 0; i < iRules; ++i) {
          AddRulesFrom(pStyleSheet, pMediaRule->GetRule(i), dwMediaList,
                       pFontMgr);
        }
      }
    } break;
    default:
      break;
  }
}

void CFDE_CSSRuleCollection::AddRuleTo(
    std::map<uint32_t, FDE_CSSRuleData*>* pMap,
    uint32_t dwKey,
    CFDE_CSSSelector* pSel,
    CFDE_CSSDeclaration* pDecl) {
  FDE_CSSRuleData* pData = NewRuleData(pSel, pDecl);
  FDE_CSSRuleData* pList = (*pMap)[dwKey];
  if (!pList) {
    (*pMap)[dwKey] = pData;
  } else if (AddRuleTo(&pList, pData)) {
    (*pMap)[dwKey] = pList;
  }
}

bool CFDE_CSSRuleCollection::AddRuleTo(FDE_CSSRuleData** pList,
                                       FDE_CSSRuleData* pData) {
  if (*pList) {
    pData->pNext = (*pList)->pNext;
    (*pList)->pNext = pData;
    return false;
  }
  *pList = pData;
  return true;
}

FDE_CSSRuleData* CFDE_CSSRuleCollection::NewRuleData(
    CFDE_CSSSelector* pSel,
    CFDE_CSSDeclaration* pDecl) {
  return new FDE_CSSRuleData(pSel, pDecl, ++m_iSelectors);
}
