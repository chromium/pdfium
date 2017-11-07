// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pageobjectholder.h"

#include <algorithm>

#include "core/fpdfapi/page/cpdf_allstates.h"
#include "core/fpdfapi/page/cpdf_contentparser.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"

CPDF_PageObjectHolder::CPDF_PageObjectHolder(CPDF_Document* pDoc,
                                             CPDF_Dictionary* pFormDict)
    : m_pFormDict(pFormDict),
      m_pFormStream(nullptr),
      m_pDocument(pDoc),
      m_pPageResources(nullptr),
      m_pResources(nullptr),
      m_iTransparency(0),
      m_bBackgroundAlphaNeeded(false),
      m_ParseState(CONTENT_NOT_PARSED) {
  // TODO(thestig): Check if |m_pFormDict| is never a nullptr and simplify
  // callers that checks for that.
}

CPDF_PageObjectHolder::~CPDF_PageObjectHolder() {}

bool CPDF_PageObjectHolder::IsPage() const {
  return false;
}

void CPDF_PageObjectHolder::ContinueParse(IFX_PauseIndicator* pPause) {
  if (!m_pParser)
    return;

  if (m_pParser->Continue(pPause))
    return;

  m_ParseState = CONTENT_PARSED;
  if (m_pParser->GetCurStates())
    m_LastCTM = m_pParser->GetCurStates()->m_CTM;
  m_pParser.reset();
}

void CPDF_PageObjectHolder::AddImageMaskBoundingBox(const CFX_FloatRect& box) {
  m_MaskBoundingBoxes.push_back(box);
}

void CPDF_PageObjectHolder::Transform(const CFX_Matrix& matrix) {
  for (auto& pObj : m_PageObjectList)
    pObj->Transform(matrix);
}

CFX_FloatRect CPDF_PageObjectHolder::CalcBoundingBox() const {
  if (m_PageObjectList.empty())
    return CFX_FloatRect();

  float left = 1000000.0f;
  float right = -1000000.0f;
  float bottom = 1000000.0f;
  float top = -1000000.0f;
  for (const auto& pObj : m_PageObjectList) {
    left = std::min(left, pObj->m_Left);
    right = std::max(right, pObj->m_Right);
    bottom = std::min(bottom, pObj->m_Bottom);
    top = std::max(top, pObj->m_Top);
  }
  return CFX_FloatRect(left, bottom, right, top);
}

void CPDF_PageObjectHolder::LoadTransInfo() {
  if (!m_pFormDict) {
    return;
  }
  CPDF_Dictionary* pGroup = m_pFormDict->GetDictFor("Group");
  if (!pGroup) {
    return;
  }
  if (pGroup->GetStringFor("S") != "Transparency") {
    return;
  }
  m_iTransparency |= PDFTRANS_GROUP;
  if (pGroup->GetIntegerFor("I")) {
    m_iTransparency |= PDFTRANS_ISOLATED;
  }
  if (pGroup->GetIntegerFor("K")) {
    m_iTransparency |= PDFTRANS_KNOCKOUT;
  }
}
