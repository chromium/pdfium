// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pattern.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/check.h"

CPDF_Pattern::CPDF_Pattern(CPDF_Document* doc,
                           RetainPtr<CPDF_Object> pObj,
                           const CFX_Matrix& parentMatrix)
    : document_(doc),
      pattern_obj_(std::move(pObj)),
      parent_matrix_(parentMatrix) {
  DCHECK(document_);
  DCHECK(pattern_obj_);
}

CPDF_Pattern::~CPDF_Pattern() = default;

CPDF_TilingPattern* CPDF_Pattern::AsTilingPattern() {
  return nullptr;
}

CPDF_ShadingPattern* CPDF_Pattern::AsShadingPattern() {
  return nullptr;
}

void CPDF_Pattern::SetPatternToFormMatrix() {
  RetainPtr<const CPDF_Dictionary> dict = pattern_obj()->GetDict();
  pattern_to_form_ = dict->GetMatrixFor("Matrix") * parent_matrix_;
}
