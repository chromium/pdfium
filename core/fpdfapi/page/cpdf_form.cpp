// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_form.h"

#include <algorithm>

#include "core/fpdfapi/page/cpdf_contentparser.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "third_party/base/ptr_util.h"

// static
CPDF_Dictionary* CPDF_Form::ChooseResourcesDict(
    CPDF_Dictionary* pResources,
    CPDF_Dictionary* pParentResources,
    CPDF_Dictionary* pPageResources) {
  if (pResources)
    return pResources;
  return pParentResources ? pParentResources : pPageResources;
}

CPDF_Form::CPDF_Form(CPDF_Document* pDoc,
                     CPDF_Dictionary* pPageResources,
                     CPDF_Stream* pFormStream)
    : CPDF_Form(pDoc, pPageResources, pFormStream, nullptr) {}

CPDF_Form::CPDF_Form(CPDF_Document* pDoc,
                     CPDF_Dictionary* pPageResources,
                     CPDF_Stream* pFormStream,
                     CPDF_Dictionary* pParentResources)
    : CPDF_PageObjectHolder(
          pDoc,
          pFormStream->GetDict(),
          pPageResources,
          ChooseResourcesDict(pFormStream->GetDict()->GetDictFor("Resources"),
                              pParentResources,
                              pPageResources)),
      m_pFormStream(pFormStream) {
  LoadTransInfo();
}

CPDF_Form::~CPDF_Form() = default;

void CPDF_Form::ParseContent() {
  ParseContentInternal(nullptr, nullptr, nullptr, nullptr);
}

void CPDF_Form::ParseContent(const CPDF_AllStates* pGraphicStates,
                             const CFX_Matrix* pParentMatrix,
                             std::set<const uint8_t*>* pParsedSet) {
  ParseContentInternal(pGraphicStates, pParentMatrix, nullptr, pParsedSet);
}

void CPDF_Form::ParseContentForType3Char(CPDF_Type3Char* pType3Char) {
  ParseContentInternal(nullptr, nullptr, pType3Char, nullptr);
}

void CPDF_Form::ParseContentInternal(const CPDF_AllStates* pGraphicStates,
                                     const CFX_Matrix* pParentMatrix,
                                     CPDF_Type3Char* pType3Char,
                                     std::set<const uint8_t*>* pParsedSet) {
  if (GetParseState() == ParseState::kParsed)
    return;

  if (GetParseState() == ParseState::kNotParsed) {
    if (!pParsedSet) {
      if (!m_ParsedSet)
        m_ParsedSet = pdfium::MakeUnique<std::set<const uint8_t*>>();
      pParsedSet = m_ParsedSet.get();
    }
    StartParse(pdfium::MakeUnique<CPDF_ContentParser>(
        this, pGraphicStates, pParentMatrix, pType3Char, pParsedSet));
  }

  ASSERT(GetParseState() == ParseState::kParsing);
  ContinueParse(nullptr);
}

CFX_FloatRect CPDF_Form::CalcBoundingBox() const {
  if (GetPageObjectCount() == 0)
    return CFX_FloatRect();

  float left = 1000000.0f;
  float right = -1000000.0f;
  float bottom = 1000000.0f;
  float top = -1000000.0f;
  for (const auto& pObj : *this) {
    const auto& rect = pObj->GetRect();
    left = std::min(left, rect.left);
    right = std::max(right, rect.right);
    bottom = std::min(bottom, rect.bottom);
    top = std::max(top, rect.top);
  }
  return CFX_FloatRect(left, bottom, right, top);
}

const CPDF_Stream* CPDF_Form::GetStream() const {
  return m_pFormStream.Get();
}

bool CPDF_Form::HasPageObjects() const {
  return GetPageObjectCount() != 0;
}

const CPDF_ImageObject* CPDF_Form::GetSoleImageOfForm() const {
  return GetPageObjectCount() == 1 ? (*begin())->AsImage() : nullptr;
}
