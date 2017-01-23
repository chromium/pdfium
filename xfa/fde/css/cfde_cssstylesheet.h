// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSSTYLESHEET_H_
#define XFA_FDE_CSS_CFDE_CSSSTYLESHEET_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_string.h"
#include "xfa/fde/css/cfde_csssyntaxparser.h"

class CFDE_CSSStyleRule;

class CFDE_CSSStyleSheet : public IFX_Retainable {
 public:
  CFDE_CSSStyleSheet();
  ~CFDE_CSSStyleSheet() override;

  // IFX_Retainable:
  uint32_t Retain() override;
  uint32_t Release() override;

  bool LoadBuffer(const FX_WCHAR* pBuffer, int32_t iBufSize);

  int32_t CountRules() const;
  CFDE_CSSStyleRule* GetRule(int32_t index);

 private:
  void Reset();
  FDE_CSSSyntaxStatus LoadStyleRule(
      CFDE_CSSSyntaxParser* pSyntax,
      std::vector<std::unique_ptr<CFDE_CSSStyleRule>>* ruleArray);
  void SkipRuleSet(CFDE_CSSSyntaxParser* pSyntax);

  uint16_t m_wRefCount;
  std::vector<std::unique_ptr<CFDE_CSSStyleRule>> m_RuleArray;
  std::unordered_map<uint32_t, FX_WCHAR*> m_StringCache;
};

#endif  // XFA_FDE_CSS_CFDE_CSSSTYLESHEET_H_
