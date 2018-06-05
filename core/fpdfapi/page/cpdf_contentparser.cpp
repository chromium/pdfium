// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_contentparser.h"

#include "core/fpdfapi/font/cpdf_type3char.h"
#include "core/fpdfapi/page/cpdf_allstates.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_path.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/pauseindicator_iface.h"
#include "third_party/base/ptr_util.h"

#define PARSE_STEP_LIMIT 100

CPDF_ContentParser::CPDF_ContentParser(CPDF_Page* pPage)
    : m_CurrentStage(Stage::kGetContent), m_pObjectHolder(pPage) {
  if (!pPage || !pPage->GetDocument() || !pPage->GetFormDict()) {
    m_CurrentStage = Stage::kComplete;
    return;
  }

  CPDF_Object* pContent = pPage->GetFormDict()->GetDirectObjectFor("Contents");
  if (!pContent) {
    m_CurrentStage = Stage::kComplete;
    return;
  }

  CPDF_Stream* pStream = pContent->AsStream();
  if (pStream) {
    RetainPtr<CPDF_StreamAcc> pSingleStream =
        pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
    pSingleStream->LoadAllDataFiltered();
    m_StreamArray.push_back(pSingleStream);
    m_CurrentStage = Stage::kParse;
    return;
  }

  CPDF_Array* pArray = pContent->AsArray();
  if (!pArray) {
    m_CurrentStage = Stage::kComplete;
    return;
  }

  m_nStreams = pArray->GetCount();
  if (m_nStreams == 0) {
    m_CurrentStage = Stage::kComplete;
    return;
  }
  m_StreamArray.resize(m_nStreams);
}

CPDF_ContentParser::CPDF_ContentParser(CPDF_Form* pForm,
                                       CPDF_AllStates* pGraphicStates,
                                       const CFX_Matrix* pParentMatrix,
                                       CPDF_Type3Char* pType3Char,
                                       std::set<const uint8_t*>* parsedSet)
    : m_CurrentStage(Stage::kParse),
      m_pObjectHolder(pForm),
      m_pType3Char(pType3Char) {
  CFX_Matrix form_matrix = pForm->GetFormDict()->GetMatrixFor("Matrix");
  if (pGraphicStates)
    form_matrix.Concat(pGraphicStates->m_CTM);

  CPDF_Array* pBBox = pForm->GetFormDict()->GetArrayFor("BBox");
  CFX_FloatRect form_bbox;
  CPDF_Path ClipPath;
  if (pBBox) {
    form_bbox = pBBox->GetRect();
    ClipPath.Emplace();
    ClipPath.AppendRect(form_bbox.left, form_bbox.bottom, form_bbox.right,
                        form_bbox.top);
    ClipPath.Transform(&form_matrix);
    if (pParentMatrix)
      ClipPath.Transform(pParentMatrix);

    form_bbox = form_matrix.TransformRect(form_bbox);
    if (pParentMatrix)
      form_bbox = pParentMatrix->TransformRect(form_bbox);
  }

  CPDF_Dictionary* pResources = pForm->GetFormDict()->GetDictFor("Resources");
  m_pParser = pdfium::MakeUnique<CPDF_StreamContentParser>(
      pForm->GetDocument(), pForm->m_pPageResources.Get(),
      pForm->m_pResources.Get(), pParentMatrix, pForm, pResources, form_bbox,
      pGraphicStates, parsedSet);
  m_pParser->GetCurStates()->m_CTM = form_matrix;
  m_pParser->GetCurStates()->m_ParentMatrix = form_matrix;
  if (ClipPath.HasRef()) {
    m_pParser->GetCurStates()->m_ClipPath.AppendPath(ClipPath, FXFILL_WINDING,
                                                     true);
  }
  if (pForm->GetTransparency().IsGroup()) {
    CPDF_GeneralState* pState = &m_pParser->GetCurStates()->m_GeneralState;
    pState->SetBlendType(FXDIB_BLEND_NORMAL);
    pState->SetStrokeAlpha(1.0f);
    pState->SetFillAlpha(1.0f);
    pState->SetSoftMask(nullptr);
  }
  RetainPtr<CPDF_StreamAcc> pSingleStream =
      pdfium::MakeRetain<CPDF_StreamAcc>(pForm->m_pFormStream.Get());
  pSingleStream->LoadAllDataFiltered();
  m_StreamArray.push_back(pSingleStream);
}

CPDF_ContentParser::~CPDF_ContentParser() {}

// Returning |true| means that there is more content to be processed and
// Continue() should be called again. Returning |false| means that we've
// completed the parse and Continue() is complete.
bool CPDF_ContentParser::Continue(PauseIndicatorIface* pPause) {
  while (m_CurrentStage == Stage::kGetContent) {
    m_CurrentStage = GetContent();
    if (pPause && pPause->NeedToPauseNow())
      return true;
  }

  while (m_CurrentStage == Stage::kParse) {
    m_CurrentStage = Parse();
    if (pPause && pPause->NeedToPauseNow())
      return true;
  }

  if (m_CurrentStage == Stage::kCheckClip)
    m_CurrentStage = CheckClip();

  ASSERT(m_CurrentStage == Stage::kComplete);
  return false;
}

CPDF_ContentParser::Stage CPDF_ContentParser::GetContent() {
  CPDF_Array* pContent =
      m_pObjectHolder->GetFormDict()->GetArrayFor("Contents");
  CPDF_Stream* pStreamObj = ToStream(
      pContent ? pContent->GetDirectObjectAt(m_CurrentOffset) : nullptr);
  m_StreamArray[m_CurrentOffset] =
      pdfium::MakeRetain<CPDF_StreamAcc>(pStreamObj);
  m_StreamArray[m_CurrentOffset]->LoadAllDataFiltered();
  m_CurrentOffset++;

  if (m_CurrentOffset >= m_nStreams) {
    m_CurrentOffset = 0;
    return Stage::kParse;
  }

  return Stage::kGetContent;
}

CPDF_ContentParser::Stage CPDF_ContentParser::Parse() {
  if (!m_pParser) {
    m_parsedSet = pdfium::MakeUnique<std::set<const uint8_t*>>();
    m_pParser = pdfium::MakeUnique<CPDF_StreamContentParser>(
        m_pObjectHolder->GetDocument(), m_pObjectHolder->m_pPageResources.Get(),
        nullptr, nullptr, m_pObjectHolder.Get(),
        m_pObjectHolder->m_pResources.Get(), m_pObjectHolder->GetBBox(),
        nullptr, m_parsedSet.get());
    m_pParser->GetCurStates()->m_ColorState.SetDefault();
  }

  m_CurrentOffset += m_pParser->Parse(
      m_StreamArray[m_CurrentStream]->GetData() + m_CurrentOffset,
      m_StreamArray[m_CurrentStream]->GetSize() - m_CurrentOffset,
      PARSE_STEP_LIMIT, m_CurrentStream);

  if (m_CurrentOffset >= m_StreamArray[m_CurrentStream]->GetSize()) {
    m_CurrentOffset = 0;
    ++m_CurrentStream;
    if (m_CurrentStream >= m_nStreams)
      return Stage::kCheckClip;
  }

  return Stage::kParse;
}

CPDF_ContentParser::Stage CPDF_ContentParser::CheckClip() {
  if (m_pType3Char) {
    m_pType3Char->InitializeFromStreamData(m_pParser->IsColored(),
                                           m_pParser->GetType3Data());
  }

  for (auto& pObj : *m_pObjectHolder->GetPageObjectList()) {
    if (!pObj->m_ClipPath.HasRef())
      continue;
    if (pObj->m_ClipPath.GetPathCount() != 1)
      continue;
    if (pObj->m_ClipPath.GetTextCount() > 0)
      continue;

    CPDF_Path ClipPath = pObj->m_ClipPath.GetPath(0);
    if (!ClipPath.IsRect() || pObj->IsShading())
      continue;

    CFX_PointF point0 = ClipPath.GetPoint(0);
    CFX_PointF point2 = ClipPath.GetPoint(2);
    CFX_FloatRect old_rect(point0.x, point0.y, point2.x, point2.y);
    CFX_FloatRect obj_rect(pObj->m_Left, pObj->m_Bottom, pObj->m_Right,
                           pObj->m_Top);
    if (old_rect.Contains(obj_rect))
      pObj->m_ClipPath.SetNull();
  }
  return Stage::kComplete;
}
