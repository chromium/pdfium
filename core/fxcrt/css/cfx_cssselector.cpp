// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssselector.h"

#include <utility>

#include "core/fxcrt/fx_extension.h"

namespace {

int32_t GetCSSNameLen(const wchar_t* psz, const wchar_t* pEnd) {
  const wchar_t* pStart = psz;
  while (psz < pEnd) {
    if (!isascii(*psz) || (!isalnum(*psz) && *psz != '_' && *psz != '-')) {
      break;
    }
    ++psz;
  }
  return psz - pStart;
}

}  // namespace

CFX_CSSSelector::CFX_CSSSelector(const wchar_t* psz, int32_t iLen)
    : name_hash_(
          FX_HashCode_GetW(WideStringView(psz, iLen), /*bIgnoreCase=*/true)) {}

CFX_CSSSelector::~CFX_CSSSelector() = default;

// static.
std::unique_ptr<CFX_CSSSelector> CFX_CSSSelector::FromString(
    WideStringView str) {
  ASSERT(!str.IsEmpty());

  const wchar_t* psz = str.unterminated_c_str();
  const wchar_t* pStart = psz;
  const wchar_t* pEnd = psz + str.GetLength();
  for (; psz < pEnd; ++psz) {
    switch (*psz) {
      case '>':
      case '[':
      case '+':
        return nullptr;
    }
  }

  std::unique_ptr<CFX_CSSSelector> head;
  for (psz = pStart; psz < pEnd;) {
    wchar_t wch = *psz;
    if ((isascii(wch) && isalpha(wch)) || wch == '*') {
      int32_t iNameLen = wch == '*' ? 1 : GetCSSNameLen(psz, pEnd);
      auto new_head = std::make_unique<CFX_CSSSelector>(psz, iNameLen);
      if (head) {
        head->set_descendent_type();
        new_head->set_next(std::move(head));
      }
      head = std::move(new_head);
      psz += iNameLen;
    } else if (wch == ' ') {
      psz++;
    } else {
      return nullptr;
    }
  }
  return head;
}
