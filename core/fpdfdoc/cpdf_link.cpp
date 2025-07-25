// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_link.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"

CPDF_Link::CPDF_Link() = default;

CPDF_Link::CPDF_Link(RetainPtr<CPDF_Dictionary> dict)
    : dict_(std::move(dict)) {}

CPDF_Link::CPDF_Link(const CPDF_Link& that) = default;

CPDF_Link::~CPDF_Link() = default;

CFX_FloatRect CPDF_Link::GetRect() {
  return dict_->GetRectFor("Rect");
}

CPDF_Dest CPDF_Link::GetDest(CPDF_Document* doc) {
  return CPDF_Dest::Create(doc, dict_->GetDirectObjectFor("Dest"));
}

CPDF_Action CPDF_Link::GetAction() {
  return CPDF_Action(dict_->GetDictFor("A"));
}
