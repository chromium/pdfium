// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_cssstylesheet.h"

#include <memory>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/css/fde_cssdatatable.h"
#include "xfa/fde/css/fde_csssyntax.h"
#include "xfa/fgas/crt/fgas_codepage.h"

namespace {

bool IsCSSChar(FX_WCHAR wch) {
  return (wch >= 'a' && wch <= 'z') || (wch >= 'A' && wch <= 'Z');
}

int32_t GetCSSPseudoLen(const FX_WCHAR* psz, const FX_WCHAR* pEnd) {
  ASSERT(*psz == ':');
  const FX_WCHAR* pStart = psz;
  while (psz < pEnd) {
    FX_WCHAR wch = *psz;
    if (IsCSSChar(wch) || wch == ':')
      ++psz;
    else
      break;
  }
  return psz - pStart;
}

int32_t GetCSSNameLen(const FX_WCHAR* psz, const FX_WCHAR* pEnd) {
  const FX_WCHAR* pStart = psz;
  while (psz < pEnd) {
    FX_WCHAR wch = *psz;
    if (IsCSSChar(wch) || (wch >= '0' && wch <= '9') || wch == '_' ||
        wch == '-') {
      ++psz;
    } else {
      break;
    }
  }
  return psz - pStart;
}

}  // namespace

CFDE_CSSStyleSheet::CFDE_CSSStyleSheet()
    : m_wCodePage(FX_CODEPAGE_UTF8),
      m_wRefCount(1),
      m_dwMediaList(FDE_CSSMEDIATYPE_ALL) {
  ASSERT(m_dwMediaList > 0);
}

CFDE_CSSStyleSheet::~CFDE_CSSStyleSheet() {
  Reset();
}

void CFDE_CSSStyleSheet::Reset() {
  m_RuleArray.clear();
  m_Selectors.clear();
  m_StringCache.clear();
}

uint32_t CFDE_CSSStyleSheet::Retain() {
  return ++m_wRefCount;
}

uint32_t CFDE_CSSStyleSheet::Release() {
  uint32_t dwRefCount = --m_wRefCount;
  if (dwRefCount == 0)
    delete this;
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
  return pdfium::CollectionSize<int32_t>(m_RuleArray);
}

CFDE_CSSRule* CFDE_CSSStyleSheet::GetRule(int32_t index) {
  return m_RuleArray[index].get();
}

bool CFDE_CSSStyleSheet::LoadFromBuffer(const FX_WCHAR* pBuffer,
                                        int32_t iBufSize) {
  ASSERT(pBuffer && iBufSize > 0);

  m_wCodePage = FX_CODEPAGE_UTF8;
  m_szUrl = CFX_WideString();

  auto pSyntax = pdfium::MakeUnique<CFDE_CSSSyntaxParser>();
  return pSyntax->Init(pBuffer, iBufSize) && LoadFromSyntax(pSyntax.get());
}

bool CFDE_CSSStyleSheet::LoadFromSyntax(CFDE_CSSSyntaxParser* pSyntax) {
  Reset();
  FDE_CSSSyntaxStatus eStatus;
  do {
    switch (eStatus = pSyntax->DoSyntaxParse()) {
      case FDE_CSSSyntaxStatus::StyleRule:
        eStatus = LoadStyleRule(pSyntax, &m_RuleArray);
        break;
      case FDE_CSSSyntaxStatus::MediaRule:
        eStatus = LoadMediaRule(pSyntax);
        break;
      case FDE_CSSSyntaxStatus::FontFaceRule:
        eStatus = LoadFontFaceRule(pSyntax, &m_RuleArray);
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
  m_Selectors.clear();
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
              LoadStyleRule(pSyntax, &pMediaRule->GetArray());
          if (eStatus < FDE_CSSSyntaxStatus::None) {
            return eStatus;
          }
        } else {
          SkipRuleSet(pSyntax);
        }
        break;
      case FDE_CSSSyntaxStatus::DeclOpen:
        if ((dwMediaList & m_dwMediaList) > 0 && !pMediaRule) {
          m_RuleArray.push_back(
              pdfium::MakeUnique<CFDE_CSSMediaRule>(dwMediaList));
          pMediaRule =
              static_cast<CFDE_CSSMediaRule*>(m_RuleArray.back().get());
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
    std::vector<std::unique_ptr<CFDE_CSSRule>>* ruleArray) {
  m_Selectors.clear();

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
        auto pSelector = CFDE_CSSSelector::FromString(pszValue, iValueLen);
        if (pSelector)
          m_Selectors.push_back(std::move(pSelector));
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
        if (!pStyleRule && !m_Selectors.empty()) {
          auto rule = pdfium::MakeUnique<CFDE_CSSStyleRule>();
          pStyleRule = rule.get();
          pStyleRule->SetSelector(m_Selectors);
          ruleArray->push_back(std::move(rule));
        } else {
          SkipRuleSet(pSyntax);
          return FDE_CSSSyntaxStatus::None;
        }
        break;
      case FDE_CSSSyntaxStatus::DeclClose:
        if (pStyleRule && pStyleRule->GetDeclImp().empty()) {
          ruleArray->pop_back();
          pStyleRule = nullptr;
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
    std::vector<std::unique_ptr<CFDE_CSSRule>>* ruleArray) {
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
          auto rule = pdfium::MakeUnique<CFDE_CSSFontFaceRule>();
          pFontFaceRule = rule.get();
          ruleArray->push_back(std::move(rule));
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
    : CFDE_CSSRule(FDE_CSSRuleType::Style),
      m_iSelectors(0) {}

CFDE_CSSStyleRule::~CFDE_CSSStyleRule() {}

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
    const std::vector<std::unique_ptr<CFDE_CSSSelector>>& list) {
  ASSERT(m_ppSelector.empty());

  for (const auto& item : list)
    m_ppSelector.push_back(item.get());
}

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

CFDE_CSSSelector::CFDE_CSSSelector(FDE_CSSSelectorType eType,
                                   const FX_WCHAR* psz,
                                   int32_t iLen,
                                   bool bIgnoreCase)
    : m_eType(eType),
      m_dwHash(FX_HashCode_GetW(CFX_WideStringC(psz, iLen), bIgnoreCase)) {}

CFDE_CSSSelector::~CFDE_CSSSelector() {}

FDE_CSSSelectorType CFDE_CSSSelector::GetType() const {
  return m_eType;
}

uint32_t CFDE_CSSSelector::GetNameHash() const {
  return m_dwHash;
}

CFDE_CSSSelector* CFDE_CSSSelector::GetNextSelector() const {
  return m_pNext.get();
}

std::unique_ptr<CFDE_CSSSelector> CFDE_CSSSelector::ReleaseNextSelector() {
  return std::move(m_pNext);
}

std::unique_ptr<CFDE_CSSSelector> CFDE_CSSSelector::FromString(
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

  std::unique_ptr<CFDE_CSSSelector> pFirst = nullptr;
  CFDE_CSSSelector* pLast = nullptr;
  std::unique_ptr<CFDE_CSSSelector> pPseudoFirst = nullptr;
  CFDE_CSSSelector* pPseudoLast = nullptr;

  for (psz = pStart; psz < pEnd;) {
    FX_WCHAR wch = *psz;
    if (wch == '.' || wch == '#') {
      if (psz == pStart || psz[-1] == ' ') {
        auto p = pdfium::MakeUnique<CFDE_CSSSelector>(
            FDE_CSSSelectorType::Element, L"*", 1, true);

        if (pFirst) {
          pFirst->SetType(FDE_CSSSelectorType::Descendant);
          p->SetNext(std::move(pFirst));
        }
        pFirst = std::move(p);
        pLast = pFirst.get();
      }
      ASSERT(pLast);

      int32_t iNameLen = GetCSSNameLen(++psz, pEnd);
      if (iNameLen == 0)
        return nullptr;

      FDE_CSSSelectorType eType =
          wch == '.' ? FDE_CSSSelectorType::Class : FDE_CSSSelectorType::ID;
      auto p =
          pdfium::MakeUnique<CFDE_CSSSelector>(eType, psz, iNameLen, false);

      p->SetNext(pLast->ReleaseNextSelector());
      pLast->SetNext(std::move(p));
      pLast = pLast->GetNextSelector();
      psz += iNameLen;
    } else if (IsCSSChar(wch) || wch == '*') {
      int32_t iNameLen = wch == '*' ? 1 : GetCSSNameLen(psz, pEnd);
      if (iNameLen == 0)
        return nullptr;

      auto p = pdfium::MakeUnique<CFDE_CSSSelector>(
          FDE_CSSSelectorType::Element, psz, iNameLen, true);
      if (pFirst) {
        pFirst->SetType(FDE_CSSSelectorType::Descendant);
        p->SetNext(std::move(pFirst));
      }
      pFirst = std::move(p);
      pLast = pFirst.get();
      psz += iNameLen;
    } else if (wch == ':') {
      int32_t iNameLen = GetCSSPseudoLen(psz, pEnd);
      if (iNameLen == 0)
        return nullptr;

      auto p = pdfium::MakeUnique<CFDE_CSSSelector>(FDE_CSSSelectorType::Pseudo,
                                                    psz, iNameLen, true);
      CFDE_CSSSelector* ptr = p.get();
      if (pPseudoFirst)
        pPseudoLast->SetNext(std::move(p));
      else
        pPseudoFirst = std::move(p);
      pPseudoLast = ptr;
      psz += iNameLen;
    } else if (wch == ' ') {
      psz++;
    } else {
      return nullptr;
    }
  }
  if (!pPseudoFirst)
    return pFirst;

  pPseudoLast->SetNext(std::move(pFirst));
  return pPseudoFirst;
}

CFDE_CSSFontFaceRule::CFDE_CSSFontFaceRule()
    : CFDE_CSSRule(FDE_CSSRuleType::FontFace) {}

CFDE_CSSFontFaceRule::~CFDE_CSSFontFaceRule() {}
