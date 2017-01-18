// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSSTYLESHEET_H_
#define XFA_FDE_CSS_FDE_CSSSTYLESHEET_H_

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_ext.h"
#include "xfa/fde/css/cfde_cssrule.h"
#include "xfa/fde/css/fde_cssdeclaration.h"

class CFDE_CSSSyntaxParser;

class CFDE_CSSSelector {
 public:
  static std::unique_ptr<CFDE_CSSSelector> FromString(const FX_WCHAR* psz,
                                                      int32_t iLen);

  CFDE_CSSSelector(FDE_CSSSelectorType eType,
                   const FX_WCHAR* psz,
                   int32_t iLen,
                   bool bIgnoreCase);
  ~CFDE_CSSSelector();

  FDE_CSSSelectorType GetType() const;
  uint32_t GetNameHash() const;
  CFDE_CSSSelector* GetNextSelector() const;
  std::unique_ptr<CFDE_CSSSelector> ReleaseNextSelector();

  void SetNext(std::unique_ptr<CFDE_CSSSelector> pNext) {
    m_pNext = std::move(pNext);
  }

 protected:
  void SetType(FDE_CSSSelectorType eType) { m_eType = eType; }

  FDE_CSSSelectorType m_eType;
  uint32_t m_dwHash;
  std::unique_ptr<CFDE_CSSSelector> m_pNext;
};

class CFDE_CSSStyleRule : public CFDE_CSSRule {
 public:
  CFDE_CSSStyleRule();
  ~CFDE_CSSStyleRule() override;

  size_t CountSelectorLists() const;
  CFDE_CSSSelector* GetSelectorList(int32_t index) const;
  CFDE_CSSDeclaration* GetDeclaration();

  void SetSelector(std::vector<std::unique_ptr<CFDE_CSSSelector>>* list);

 private:
  CFDE_CSSDeclaration m_Declaration;
  std::vector<std::unique_ptr<CFDE_CSSSelector>> m_ppSelector;
};

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

class CFDE_CSSFontFaceRule : public CFDE_CSSRule {
 public:
  CFDE_CSSFontFaceRule();
  ~CFDE_CSSFontFaceRule() override;

  CFDE_CSSDeclaration* GetDeclaration() { return &m_Declaration; }

 private:
  CFDE_CSSDeclaration m_Declaration;
};

class CFDE_CSSStyleSheet : public IFX_Retainable {
 public:
  CFDE_CSSStyleSheet();
  ~CFDE_CSSStyleSheet() override;

  // IFX_Retainable:
  uint32_t Retain() override;
  uint32_t Release() override;

  bool LoadFromBuffer(const FX_WCHAR* pBuffer, int32_t iBufSize);

  bool GetUrl(CFX_WideString& szUrl);
  uint32_t GetMediaList() const;
  uint16_t GetCodePage() const;
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

  uint16_t m_wCodePage;
  uint16_t m_wRefCount;
  uint32_t m_dwMediaList;
  std::vector<std::unique_ptr<CFDE_CSSRule>> m_RuleArray;
  CFX_WideString m_szUrl;
  std::unordered_map<uint32_t, FX_WCHAR*> m_StringCache;
};

#endif  // XFA_FDE_CSS_FDE_CSSSTYLESHEET_H_
