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

class CFDE_CSSRule;

class CFDE_CSSStyleSheet : public IFX_Retainable {
 public:
  CFDE_CSSStyleSheet();
  ~CFDE_CSSStyleSheet() override;

  // IFX_Retainable:
  uint32_t Retain() override;
  uint32_t Release() override;

  bool LoadFromBuffer(const FX_WCHAR* pBuffer, int32_t iBufSize);

  uint32_t GetMediaList() const;
  int32_t CountRules() const;
  CFDE_CSSRule* GetRule(int32_t index);

 private:
  void Reset();
  bool LoadFromSyntax(CFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSyntaxStatus LoadStyleRule(
      CFDE_CSSSyntaxParser* pSyntax,
      std::vector<std::unique_ptr<CFDE_CSSRule>>* ruleArray);
  FDE_CSSSyntaxStatus LoadImportRule(CFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSyntaxStatus LoadPageRule(CFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSyntaxStatus LoadMediaRule(CFDE_CSSSyntaxParser* pSyntax);
  FDE_CSSSyntaxStatus LoadFontFaceRule(
      CFDE_CSSSyntaxParser* pSyntax,
      std::vector<std::unique_ptr<CFDE_CSSRule>>* ruleArray);
  FDE_CSSSyntaxStatus SkipRuleSet(CFDE_CSSSyntaxParser* pSyntax);

  uint16_t m_wRefCount;
  uint32_t m_dwMediaList;
  std::vector<std::unique_ptr<CFDE_CSSRule>> m_RuleArray;
  std::unordered_map<uint32_t, FX_WCHAR*> m_StringCache;
};

#endif  // XFA_FDE_CSS_CFDE_CSSSTYLESHEET_H_
