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

// static.
std::unique_ptr<CFDE_CSSSelector> CFDE_CSSSelector::FromString(
    const CFX_WideStringC& str) {
  ASSERT(!str.IsEmpty());

  const FX_WCHAR* psz = str.c_str();
  const FX_WCHAR* pStart = psz;
  const FX_WCHAR* pEnd = psz + str.GetLength();
  for (; psz < pEnd; ++psz) {
    switch (*psz) {
      case '>':
      case '[':
      case '+':
        return nullptr;
    }
  }

  std::unique_ptr<CFDE_CSSSelector> pFirst = nullptr;
  for (psz = pStart; psz < pEnd;) {
    FX_WCHAR wch = *psz;
    if (IsCSSChar(wch) || wch == '*') {
      int32_t iNameLen = wch == '*' ? 1 : GetCSSNameLen(psz, pEnd);
      auto p = pdfium::MakeUnique<CFDE_CSSSelector>(
          FDE_CSSSelectorType::Element, psz, iNameLen, true);
      if (pFirst) {
        pFirst->SetType(FDE_CSSSelectorType::Descendant);
        p->SetNext(std::move(pFirst));
      }
      pFirst = std::move(p);
      psz += iNameLen;
    } else if (wch == ' ') {
      psz++;
    } else {
      return nullptr;
    }
  }
  return pFirst;
}
