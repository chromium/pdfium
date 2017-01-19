// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSRULECOLLECTION_H_
#define XFA_FDE_CSS_CFDE_CSSRULECOLLECTION_H_

#include <map>

#include "core/fxcrt/fx_basic.h"

class CFDE_CSSDeclaration;
class CFDE_CSSRule;
class CFDE_CSSSelector;
class CFDE_CSSStyleSheet;
class CFGAS_FontMgr;

class CFDE_CSSRuleCollection {
 public:
  class Data {
   public:
    Data(CFDE_CSSSelector* pSel, CFDE_CSSDeclaration* pDecl, uint32_t dwPos);

    CFDE_CSSSelector* const pSelector;
    CFDE_CSSDeclaration* const pDeclaration;
    uint32_t dwPriority;
    Data* pNext;
  };

  CFDE_CSSRuleCollection();
  ~CFDE_CSSRuleCollection();

  void AddRulesFrom(const CFX_ArrayTemplate<CFDE_CSSStyleSheet*>& sheets,
                    uint32_t dwMediaList,
                    CFGAS_FontMgr* pFontMgr);
  void Clear();
  int32_t CountSelectors() const { return m_iSelectors; }

  Data* GetIDRuleData(uint32_t dwIDHash) {
    auto it = m_IDRules.find(dwIDHash);
    return it != m_IDRules.end() ? it->second : nullptr;
  }

  Data* GetTagRuleData(uint32_t dwTagHash) {
    auto it = m_TagRules.find(dwTagHash);
    return it != m_TagRules.end() ? it->second : nullptr;
  }

  Data* GetClassRuleData(uint32_t dwIDHash) {
    auto it = m_ClassRules.find(dwIDHash);
    return it != m_ClassRules.end() ? it->second : nullptr;
  }

  Data* GetUniversalRuleData() { return m_pUniversalRules; }
  Data* GetPseudoRuleData() { return m_pPseudoRules; }

 protected:
  void AddRulesFrom(CFDE_CSSStyleSheet* pStyleSheet,
                    CFDE_CSSRule* pRule,
                    uint32_t dwMediaList,
                    CFGAS_FontMgr* pFontMgr);
  void AddRuleTo(std::map<uint32_t, Data*>* pMap,
                 uint32_t dwKey,
                 CFDE_CSSSelector* pSel,
                 CFDE_CSSDeclaration* pDecl);
  bool AddRuleTo(Data** pList, Data* pData);
  Data* NewRuleData(CFDE_CSSSelector* pSel, CFDE_CSSDeclaration* pDecl);

  std::map<uint32_t, Data*> m_IDRules;
  std::map<uint32_t, Data*> m_TagRules;
  std::map<uint32_t, Data*> m_ClassRules;
  Data* m_pUniversalRules;
  Data* m_pPseudoRules;
  int32_t m_iSelectors;
};

#endif  // XFA_FDE_CSS_CFDE_CSSRULECOLLECTION_H_
