// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSRULECOLLECTION_H_
#define XFA_FDE_CSS_CFDE_CSSRULECOLLECTION_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_basic.h"

class CFDE_CSSDeclaration;
class CFDE_CSSSelector;
class CFDE_CSSStyleRule;
class CFDE_CSSStyleSheet;
class CFGAS_FontMgr;

class CFDE_CSSRuleCollection {
 public:
  class Data {
   public:
    Data(CFDE_CSSSelector* pSel, CFDE_CSSDeclaration* pDecl);

    CFDE_CSSSelector* const pSelector;
    CFDE_CSSDeclaration* const pDeclaration;
  };

  CFDE_CSSRuleCollection();
  ~CFDE_CSSRuleCollection();

  void AddRulesFrom(const CFDE_CSSStyleSheet* sheet, CFGAS_FontMgr* pFontMgr);
  void Clear();
  int32_t CountSelectors() const { return m_iSelectors; }

  const std::vector<std::unique_ptr<Data>>* GetTagRuleData(
      const CFX_WideString& tagname) const;

 private:
  void AddRulesFrom(const CFDE_CSSStyleSheet* pStyleSheet,
                    CFDE_CSSStyleRule* pRule,
                    CFGAS_FontMgr* pFontMgr);

  std::map<uint32_t, std::vector<std::unique_ptr<Data>>> m_TagRules;
  int32_t m_iSelectors;
};

#endif  // XFA_FDE_CSS_CFDE_CSSRULECOLLECTION_H_
