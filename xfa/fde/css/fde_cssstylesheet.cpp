// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_cssstylesheet.h"

#include <memory>

#include "third_party/base/ptr_util.h"
#include "xfa/fde/css/fde_cssdatatable.h"
#include "xfa/fde/css/fde_csssyntax.h"
#include "xfa/fgas/crt/fgas_codepage.h"

IFDE_CSSStyleSheet* IFDE_CSSStyleSheet::LoadFromBuffer(
    const CFX_WideString& szUrl,
    const FX_WCHAR* pBuffer,
    int32_t iBufSize,
    uint16_t wCodePage,
    uint32_t dwMediaList) {
  CFDE_CSSStyleSheet* pStyleSheet = new CFDE_CSSStyleSheet(dwMediaList);
  if (!pStyleSheet->LoadFromBuffer(szUrl, pBuffer, iBufSize, wCodePage)) {
    pStyleSheet->Release();
    pStyleSheet = nullptr;
  }
  return pStyleSheet;
}

CFDE_CSSStyleSheet::CFDE_CSSStyleSheet(uint32_t dwMediaList)
    : m_wCodePage(FX_CODEPAGE_UTF8),
      m_wRefCount(1),
      m_dwMediaList(dwMediaList),
      m_RuleArray(100) {
  ASSERT(m_dwMediaList > 0);
}

CFDE_CSSStyleSheet::~CFDE_CSSStyleSheet() {
  Reset();
}

void CFDE_CSSStyleSheet::Reset() {
  for (int32_t i = m_RuleArray.GetSize() - 1; i >= 0; --i) {
    IFDE_CSSRule* pRule = m_RuleArray.GetAt(i);
    switch (pRule->GetType()) {
      case FDE_CSSRuleType::Style:
        static_cast<CFDE_CSSStyleRule*>(pRule)->~CFDE_CSSStyleRule();
        break;
      case FDE_CSSRuleType::Media:
        static_cast<CFDE_CSSMediaRule*>(pRule)->~CFDE_CSSMediaRule();
        break;
      case FDE_CSSRuleType::FontFace:
        static_cast<CFDE_CSSFontFaceRule*>(pRule)->~CFDE_CSSFontFaceRule();
        break;
      default:
        ASSERT(false);
        break;
    }
  }
  m_RuleArray.RemoveAll(false);
  m_Selectors.RemoveAll();
  m_StringCache.clear();
}

uint32_t CFDE_CSSStyleSheet::Retain() {
  return ++m_wRefCount;
}

uint32_t CFDE_CSSStyleSheet::Release() {
  uint32_t dwRefCount = --m_wRefCount;
  if (dwRefCount == 0) {
    delete this;
  }
  return dwRefCount;
}

bool CFDE_CSSStyleSheet::GetUrl(CFX_WideString& szUrl) {
  szUrl = m_szUrl;
  return szUrl.GetLength() > 0;
}

uint32_t CFDE_CSSStyleSheet::GetMediaList() const {
  return m_dwMediaList;
}

uint16_t CFDE_CSSStyleSheet::GetCodePage() const {
  return m_wCodePage;
}

int32_t CFDE_CSSStyleSheet::CountRules() const {
  return m_RuleArray.GetSize();
}

IFDE_CSSRule* CFDE_CSSStyleSheet::GetRule(int32_t index) {
  return m_RuleArray.GetAt(index);
}

bool CFDE_CSSStyleSheet::LoadFromBuffer(const CFX_WideString& szUrl,
                                        const FX_WCHAR* pBuffer,
                                        int32_t iBufSize,
                                        uint16_t wCodePage) {
  ASSERT(pBuffer && iBufSize > 0);
  std::unique_ptr<CFDE_CSSSyntaxParser> pSyntax(new CFDE_CSSSyntaxParser);
  bool bRet = pSyntax->Init(pBuffer, iBufSize) && LoadFromSyntax(pSyntax.get());
  m_wCodePage = wCodePage;
  m_szUrl = szUrl;
  return bRet;
}

bool CFDE_CSSStyleSheet::LoadFromSyntax(CFDE_CSSSyntaxParser* pSyntax) {
  Reset();
  FDE_CSSSyntaxStatus eStatus;
  do {
    switch (eStatus = pSyntax->DoSyntaxParse()) {
      case FDE_CSSSyntaxStatus::StyleRule:
        eStatus = LoadStyleRule(pSyntax, m_RuleArray);
        break;
      case FDE_CSSSyntaxStatus::MediaRule:
        eStatus = LoadMediaRule(pSyntax);
        break;
      case FDE_CSSSyntaxStatus::FontFaceRule:
        eStatus = LoadFontFaceRule(pSyntax, m_RuleArray);
        break;
      case FDE_CSSSyntaxStatus::ImportRule:
        eStatus = LoadImportRule(pSyntax);
        break;
      case FDE_CSSSyntaxStatus::PageRule:
        eStatus = LoadPageRule(pSyntax);
        break;
      default:
        break;
    }
  } while (eStatus >= FDE_CSSSyntaxStatus::None);
  m_Selectors.RemoveAll();
  m_StringCache.clear();
  return eStatus != FDE_CSSSyntaxStatus::Error;
}

FDE_CSSSyntaxStatus CFDE_CSSStyleSheet::LoadMediaRule(
    CFDE_CSSSyntaxParser* pSyntax) {
  uint32_t dwMediaList = 0;
  CFDE_CSSMediaRule* pMediaRule = nullptr;
  for (;;) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSyntaxStatus::MediaType: {
        int32_t iLen;
        const FX_WCHAR* psz = pSyntax->GetCurrentString(iLen);
        const FDE_CSSMEDIATYPETABLE* pMediaType =
            FDE_GetCSSMediaTypeByName(CFX_WideStringC(psz, iLen));
        if (pMediaType)
          dwMediaList |= pMediaType->wValue;
      } break;
      case FDE_CSSSyntaxStatus::StyleRule:
        if (pMediaRule) {
          FDE_CSSSyntaxStatus eStatus =
              LoadStyleRule(pSyntax, pMediaRule->GetArray());
          if (eStatus < FDE_CSSSyntaxStatus::None) {
            return eStatus;
          }
        } else {
          SkipRuleSet(pSyntax);
        }
        break;
      case FDE_CSSSyntaxStatus::DeclOpen:
        if ((dwMediaList & m_dwMediaList) > 0 && !pMediaRule) {
          pMediaRule = new CFDE_CSSMediaRule(dwMediaList);
          m_RuleArray.Add(pMediaRule);
        }
        break;
      case FDE_CSSSyntaxStatus::DeclClose:
        return FDE_CSSSyntaxStatus::None;
      case FDE_CSSSyntaxStatus::EOS:
        return FDE_CSSSyntaxStatus::EOS;
      case FDE_CSSSyntaxStatus::Error:
      default:
        return FDE_CSSSyntaxStatus::Error;
    }
  }
}

FDE_CSSSyntaxStatus CFDE_CSSStyleSheet::LoadStyleRule(
    CFDE_CSSSyntaxParser* pSyntax,
    CFX_MassArrayTemplate<IFDE_CSSRule*>& ruleArray) {
  m_Selectors.RemoveAt(0, m_Selectors.GetSize());
  CFDE_CSSStyleRule* pStyleRule = nullptr;
  const FX_WCHAR* pszValue = nullptr;
  int32_t iValueLen = 0;
  FDE_CSSPropertyArgs propertyArgs;
  propertyArgs.pStringCache = &m_StringCache;
  propertyArgs.pProperty = nullptr;
  CFX_WideString wsName;
  for (;;) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSyntaxStatus::Selector: {
        pszValue = pSyntax->GetCurrentString(iValueLen);
        CFDE_CSSSelector* pSelector =
            CFDE_CSSSelector::FromString(pszValue, iValueLen);
        if (pSelector)
          m_Selectors.Add(pSelector);
      } break;
      case FDE_CSSSyntaxStatus::PropertyName:
        pszValue = pSyntax->GetCurrentString(iValueLen);
        propertyArgs.pProperty =
            FDE_GetCSSPropertyByName(CFX_WideStringC(pszValue, iValueLen));
        if (!propertyArgs.pProperty)
          wsName = CFX_WideStringC(pszValue, iValueLen);
        break;
      case FDE_CSSSyntaxStatus::PropertyValue:
        if (propertyArgs.pProperty) {
          pszValue = pSyntax->GetCurrentString(iValueLen);
          if (iValueLen > 0) {
            pStyleRule->GetDeclImp().AddProperty(&propertyArgs, pszValue,
                                                 iValueLen);
          }
        } else if (iValueLen > 0) {
          pszValue = pSyntax->GetCurrentString(iValueLen);
          if (iValueLen > 0) {
            pStyleRule->GetDeclImp().AddProperty(&propertyArgs, wsName.c_str(),
                                                 wsName.GetLength(), pszValue,
                                                 iValueLen);
          }
        }
        break;
      case FDE_CSSSyntaxStatus::DeclOpen:
        if (!pStyleRule && m_Selectors.GetSize() > 0) {
          pStyleRule = new CFDE_CSSStyleRule;
          pStyleRule->SetSelector(m_Selectors);
          ruleArray.Add(pStyleRule);
        } else {
          SkipRuleSet(pSyntax);
          return FDE_CSSSyntaxStatus::None;
        }
        break;
      case FDE_CSSSyntaxStatus::DeclClose:
        if (pStyleRule && !pStyleRule->GetDeclImp().GetStartPosition()) {
          pStyleRule->~CFDE_CSSStyleRule();
          ruleArray.RemoveLast(1);
        }
        return FDE_CSSSyntaxStatus::None;
      case FDE_CSSSyntaxStatus::EOS:
        return FDE_CSSSyntaxStatus::EOS;
      case FDE_CSSSyntaxStatus::Error:
      default:
        return FDE_CSSSyntaxStatus::Error;
    }
  }
}

FDE_CSSSyntaxStatus CFDE_CSSStyleSheet::LoadFontFaceRule(
    CFDE_CSSSyntaxParser* pSyntax,
    CFX_MassArrayTemplate<IFDE_CSSRule*>& ruleArray) {
  CFDE_CSSFontFaceRule* pFontFaceRule = nullptr;
  const FX_WCHAR* pszValue = nullptr;
  int32_t iValueLen = 0;
  FDE_CSSPropertyArgs propertyArgs;
  propertyArgs.pStringCache = &m_StringCache;
  propertyArgs.pProperty = nullptr;
  for (;;) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSyntaxStatus::PropertyName:
        pszValue = pSyntax->GetCurrentString(iValueLen);
        propertyArgs.pProperty =
            FDE_GetCSSPropertyByName(CFX_WideStringC(pszValue, iValueLen));
        break;
      case FDE_CSSSyntaxStatus::PropertyValue:
        if (propertyArgs.pProperty) {
          pszValue = pSyntax->GetCurrentString(iValueLen);
          if (iValueLen > 0) {
            pFontFaceRule->GetDeclImp().AddProperty(&propertyArgs, pszValue,
                                                    iValueLen);
          }
        }
        break;
      case FDE_CSSSyntaxStatus::DeclOpen:
        if (!pFontFaceRule) {
          pFontFaceRule = new CFDE_CSSFontFaceRule;
          ruleArray.Add(pFontFaceRule);
        }
        break;
      case FDE_CSSSyntaxStatus::DeclClose:
        return FDE_CSSSyntaxStatus::None;
      case FDE_CSSSyntaxStatus::EOS:
        return FDE_CSSSyntaxStatus::EOS;
      case FDE_CSSSyntaxStatus::Error:
      default:
        return FDE_CSSSyntaxStatus::Error;
    }
  }
}

FDE_CSSSyntaxStatus CFDE_CSSStyleSheet::LoadImportRule(
    CFDE_CSSSyntaxParser* pSyntax) {
  for (;;) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSyntaxStatus::ImportClose:
        return FDE_CSSSyntaxStatus::None;
      case FDE_CSSSyntaxStatus::URI:
        break;
      case FDE_CSSSyntaxStatus::EOS:
        return FDE_CSSSyntaxStatus::EOS;
      case FDE_CSSSyntaxStatus::Error:
      default:
        return FDE_CSSSyntaxStatus::Error;
    }
  }
}

FDE_CSSSyntaxStatus CFDE_CSSStyleSheet::LoadPageRule(
    CFDE_CSSSyntaxParser* pSyntax) {
  return SkipRuleSet(pSyntax);
}

FDE_CSSSyntaxStatus CFDE_CSSStyleSheet::SkipRuleSet(
    CFDE_CSSSyntaxParser* pSyntax) {
  for (;;) {
    switch (pSyntax->DoSyntaxParse()) {
      case FDE_CSSSyntaxStatus::Selector:
      case FDE_CSSSyntaxStatus::DeclOpen:
      case FDE_CSSSyntaxStatus::PropertyName:
      case FDE_CSSSyntaxStatus::PropertyValue:
        break;
      case FDE_CSSSyntaxStatus::DeclClose:
        return FDE_CSSSyntaxStatus::None;
      case FDE_CSSSyntaxStatus::EOS:
        return FDE_CSSSyntaxStatus::EOS;
      case FDE_CSSSyntaxStatus::Error:
      default:
        return FDE_CSSSyntaxStatus::Error;
    }
  }
}

CFDE_CSSStyleRule::CFDE_CSSStyleRule()
    : m_ppSelector(nullptr), m_iSelectors(0) {}

int32_t CFDE_CSSStyleRule::CountSelectorLists() const {
  return m_iSelectors;
}

CFDE_CSSSelector* CFDE_CSSStyleRule::GetSelectorList(int32_t index) const {
  return m_ppSelector[index];
}

CFDE_CSSDeclaration* CFDE_CSSStyleRule::GetDeclaration() {
  return &m_Declaration;
}

void CFDE_CSSStyleRule::SetSelector(
    const CFX_ArrayTemplate<CFDE_CSSSelector*>& list) {
  ASSERT(!m_ppSelector);
  m_iSelectors = list.GetSize();
  m_ppSelector = static_cast<CFDE_CSSSelector**>(
      FX_Alloc(CFDE_CSSSelector*, m_iSelectors));
  for (int32_t i = 0; i < m_iSelectors; ++i) {
    m_ppSelector[i] = list.GetAt(i);
  }
}

CFDE_CSSMediaRule::CFDE_CSSMediaRule(uint32_t dwMediaList)
    : m_dwMediaList(dwMediaList), m_RuleArray(100) {}

CFDE_CSSMediaRule::~CFDE_CSSMediaRule() {
  for (int32_t i = m_RuleArray.GetSize() - 1; i >= 0; --i) {
    IFDE_CSSRule* pRule = m_RuleArray.GetAt(i);
    switch (pRule->GetType()) {
      case FDE_CSSRuleType::Style:
        static_cast<CFDE_CSSStyleRule*>(pRule)->~CFDE_CSSStyleRule();
        break;
      default:
        ASSERT(false);
        break;
    }
  }
}

uint32_t CFDE_CSSMediaRule::GetMediaList() const {
  return m_dwMediaList;
}

int32_t CFDE_CSSMediaRule::CountRules() const {
  return m_RuleArray.GetSize();
}

IFDE_CSSRule* CFDE_CSSMediaRule::GetRule(int32_t index) {
  return m_RuleArray.GetAt(index);
}

bool FDE_IsCSSChar(FX_WCHAR wch) {
  return (wch >= 'a' && wch <= 'z') || (wch >= 'A' && wch <= 'Z');
}

int32_t FDE_GetCSSPseudoLen(const FX_WCHAR* psz, const FX_WCHAR* pEnd) {
  ASSERT(*psz == ':');
  const FX_WCHAR* pStart = psz;
  while (psz < pEnd) {
    FX_WCHAR wch = *psz;
    if (FDE_IsCSSChar(wch) || wch == ':') {
      ++psz;
    } else {
      break;
    }
  }
  return psz - pStart;
}

int32_t FDE_GetCSSNameLen(const FX_WCHAR* psz, const FX_WCHAR* pEnd) {
  const FX_WCHAR* pStart = psz;
  while (psz < pEnd) {
    FX_WCHAR wch = *psz;
    if (FDE_IsCSSChar(wch) || (wch >= '0' && wch <= '9') || wch == '_' ||
        wch == '-') {
      ++psz;
    } else {
      break;
    }
  }
  return psz - pStart;
}

CFDE_CSSSelector::CFDE_CSSSelector(FDE_CSSSelectorType eType,
                                   const FX_WCHAR* psz,
                                   int32_t iLen,
                                   bool bIgnoreCase)
    : m_eType(eType),
      m_dwHash(FX_HashCode_GetW(CFX_WideStringC(psz, iLen), bIgnoreCase)),
      m_pNext(nullptr) {}

FDE_CSSSelectorType CFDE_CSSSelector::GetType() const {
  return m_eType;
}

uint32_t CFDE_CSSSelector::GetNameHash() const {
  return m_dwHash;
}

CFDE_CSSSelector* CFDE_CSSSelector::GetNextSelector() const {
  return m_pNext;
}

CFDE_CSSSelector* CFDE_CSSSelector::FromString(
    const FX_WCHAR* psz,
    int32_t iLen) {
  ASSERT(psz && iLen > 0);

  const FX_WCHAR* pStart = psz;
  const FX_WCHAR* pEnd = psz + iLen;
  for (; psz < pEnd; ++psz) {
    switch (*psz) {
      case '>':
      case '[':
      case '+':
        return nullptr;
    }
  }
  CFDE_CSSSelector* pFirst = nullptr;
  CFDE_CSSSelector* pLast = nullptr;
  CFDE_CSSSelector* pPseudoFirst = nullptr;
  CFDE_CSSSelector* pPseudoLast = nullptr;
  for (psz = pStart; psz < pEnd;) {
    FX_WCHAR wch = *psz;
    if (wch == '.' || wch == '#') {
      if (psz == pStart || psz[-1] == ' ') {
        CFDE_CSSSelector* p =
            new CFDE_CSSSelector(FDE_CSSSelectorType::Element, L"*", 1, true);
        if (!p)
          return nullptr;

        if (pFirst) {
          pFirst->SetType(FDE_CSSSelectorType::Descendant);
          p->SetNext(pFirst);
        }
        pFirst = pLast = p;
      }
      ASSERT(pLast);
      int32_t iNameLen = FDE_GetCSSNameLen(++psz, pEnd);
      if (iNameLen == 0) {
        return nullptr;
      }
      FDE_CSSSelectorType eType =
          wch == '.' ? FDE_CSSSelectorType::Class : FDE_CSSSelectorType::ID;
      CFDE_CSSSelector* p = new CFDE_CSSSelector(eType, psz, iNameLen, false);
      if (!p)
        return nullptr;

      p->SetNext(pLast->GetNextSelector());
      pLast->SetNext(p);
      pLast = p;
      psz += iNameLen;
    } else if (FDE_IsCSSChar(wch) || wch == '*') {
      int32_t iNameLen = wch == '*' ? 1 : FDE_GetCSSNameLen(psz, pEnd);
      if (iNameLen == 0) {
        return nullptr;
      }
      CFDE_CSSSelector* p = new CFDE_CSSSelector(FDE_CSSSelectorType::Element,
                                                 psz, iNameLen, true);
      if (!p)
        return nullptr;

      if (pFirst) {
        pFirst->SetType(FDE_CSSSelectorType::Descendant);
        p->SetNext(pFirst);
      }
      pFirst = p;
      pLast = p;
      psz += iNameLen;
    } else if (wch == ':') {
      int32_t iNameLen = FDE_GetCSSPseudoLen(psz, pEnd);
      if (iNameLen == 0) {
        return nullptr;
      }
      CFDE_CSSSelector* p = new CFDE_CSSSelector(FDE_CSSSelectorType::Pseudo,
                                                 psz, iNameLen, true);
      if (!p)
        return nullptr;

      if (pPseudoFirst)
        pPseudoLast->SetNext(p);
      else
        pPseudoFirst = p;
      pPseudoLast = p;
      psz += iNameLen;
    } else if (wch == ' ') {
      psz++;
    } else {
      return nullptr;
    }
  }
  if (!pPseudoFirst)
    return pFirst;

  pPseudoLast->SetNext(pFirst);
  return pPseudoFirst;
}

CFDE_CSSDeclaration* CFDE_CSSFontFaceRule::GetDeclaration() {
  return &m_Declaration;
}
