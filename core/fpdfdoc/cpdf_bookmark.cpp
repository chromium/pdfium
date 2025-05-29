// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_bookmark.h"

#include <algorithm>
#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxge/dib/fx_dib.h"

CPDF_Bookmark::CPDF_Bookmark() = default;

CPDF_Bookmark::CPDF_Bookmark(const CPDF_Bookmark& that) = default;

CPDF_Bookmark::CPDF_Bookmark(RetainPtr<const CPDF_Dictionary> dict)
    : dict_(std::move(dict)) {}

CPDF_Bookmark::~CPDF_Bookmark() = default;

WideString CPDF_Bookmark::GetTitle() const {
  if (!dict_) {
    return WideString();
  }
  RetainPtr<const CPDF_String> pString =
      ToString(dict_->GetDirectObjectFor("Title"));
  if (!pString) {
    return WideString();
  }
  WideString title = pString->GetUnicodeText();
  WideString result;
  result.Reserve(title.GetLength());
  for (const wchar_t wc : title) {
    result += std::max(wc, static_cast<wchar_t>(0x20));
  }
  return result;
}

CPDF_Dest CPDF_Bookmark::GetDest(CPDF_Document* document) const {
  if (!dict_) {
    return CPDF_Dest(nullptr);
  }
  return CPDF_Dest::Create(document, dict_->GetDirectObjectFor("Dest"));
}

CPDF_Action CPDF_Bookmark::GetAction() const {
  return CPDF_Action(dict_ ? dict_->GetDictFor("A") : nullptr);
}

int CPDF_Bookmark::GetCount() const {
  return dict_->GetIntegerFor("Count");
}
