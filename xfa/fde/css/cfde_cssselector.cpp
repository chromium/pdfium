// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssselector.h"

#include <utility>

#include "third_party/base/ptr_util.h"

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
