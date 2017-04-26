// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSSTYLESHEET_H_
#define XFA_FDE_CSS_CFDE_CSSSTYLESHEET_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/css/cfde_csssyntaxparser.h"

class CFDE_CSSStyleRule;

class CFDE_CSSStyleSheet {
 public:
  CFDE_CSSStyleSheet();
  ~CFDE_CSSStyleSheet();

  bool LoadBuffer(const wchar_t* pBuffer, int32_t iBufSize);

  int32_t CountRules() const;
  CFDE_CSSStyleRule* GetRule(int32_t index) const;

 private:
  void Reset();
  FDE_CSSSyntaxStatus LoadStyleRule(
      CFDE_CSSSyntaxParser* pSyntax,
      std::vector<std::unique_ptr<CFDE_CSSStyleRule>>* ruleArray);
  void SkipRuleSet(CFDE_CSSSyntaxParser* pSyntax);

  std::vector<std::unique_ptr<CFDE_CSSStyleRule>> m_RuleArray;
  std::map<uint32_t, wchar_t*> m_StringCache;
};

#endif  // XFA_FDE_CSS_CFDE_CSSSTYLESHEET_H_
