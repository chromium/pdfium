// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_tilingpattern.h"

#include <math.h>

#include <utility>

#include "core/fpdfapi/page/cpdf_allstates.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/check.h"

CPDF_TilingPattern::CPDF_TilingPattern(CPDF_Document* doc,
                                       RetainPtr<CPDF_Object> pPatternObj,
                                       const CFX_Matrix& parentMatrix)
    : CPDF_Pattern(doc, std::move(pPatternObj), parentMatrix) {
  DCHECK(document());
  colored_ = pattern_obj()->GetDict()->GetIntegerFor("PaintType") == 1;
  SetPatternToFormMatrix();
}

CPDF_TilingPattern::~CPDF_TilingPattern() = default;

CPDF_TilingPattern* CPDF_TilingPattern::AsTilingPattern() {
  return this;
}

std::unique_ptr<CPDF_Form> CPDF_TilingPattern::Load(CPDF_PageObject* pPageObj) {
  RetainPtr<const CPDF_Dictionary> dict = pattern_obj()->GetDict();
  colored_ = dict->GetIntegerFor("PaintType") == 1;
  xstep_ = fabsf(dict->GetFloatFor("XStep"));
  ystep_ = fabsf(dict->GetFloatFor("YStep"));

  RetainPtr<CPDF_Stream> pStream = ToStream(pattern_obj());
  if (!pStream) {
    return nullptr;
  }

  const CFX_Matrix& matrix = parent_matrix();
  auto form =
      std::make_unique<CPDF_Form>(document(), nullptr, std::move(pStream));

  CPDF_AllStates all_states;
  all_states.mutable_color_state().Emplace();
  all_states.mutable_graph_state().Emplace();
  all_states.mutable_text_state().Emplace();
  all_states.mutable_general_state() = pPageObj->general_state();
  form->ParseContent(&all_states, &matrix, nullptr);
  bbox_ = dict->GetRectFor("BBox");
  return form;
}
