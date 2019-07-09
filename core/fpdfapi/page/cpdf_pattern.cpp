// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pattern.h"

#include "core/fpdfapi/parser/cpdf_dictionary.h"

CPDF_Pattern::CPDF_Pattern(CPDF_Document* pDoc,
                           CPDF_Object* pObj,
                           const CFX_Matrix& parentMatrix)
    : m_pDocument(pDoc), m_pPatternObj(pObj), m_ParentMatrix(parentMatrix) {
  ASSERT(m_pDocument);
  ASSERT(m_pPatternObj);
}

CPDF_Pattern::~CPDF_Pattern() = default;

CPDF_TilingPattern* CPDF_Pattern::AsTilingPattern() {
  return nullptr;
}

CPDF_ShadingPattern* CPDF_Pattern::AsShadingPattern() {
  return nullptr;
}

void CPDF_Pattern::SetPatternToFormMatrix() {
  const CPDF_Dictionary* pDict = pattern_obj()->GetDict();
  m_Pattern2Form = pDict->GetMatrixFor("Matrix") * m_ParentMatrix;
}
