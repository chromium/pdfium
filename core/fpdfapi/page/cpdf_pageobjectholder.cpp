// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pageobjectholder.h"

#include <algorithm>
#include <utility>

#include "constants/transparency.h"
#include "core/fpdfapi/page/cpdf_allstates.h"
#include "core/fpdfapi/page/cpdf_contentparser.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/fx_extension.h"
#include "third_party/base/stl_util.h"

bool GraphicsData::operator<(const GraphicsData& other) const {
  if (!FXSYS_SafeEQ(fillAlpha, other.fillAlpha))
    return FXSYS_SafeLT(fillAlpha, other.fillAlpha);
  if (!FXSYS_SafeEQ(strokeAlpha, other.strokeAlpha))
    return FXSYS_SafeLT(strokeAlpha, other.strokeAlpha);
  return blendType < other.blendType;
}

bool FontData::operator<(const FontData& other) const {
  if (baseFont != other.baseFont)
    return baseFont < other.baseFont;
  return type < other.type;
}

CPDF_PageObjectHolder::CPDF_PageObjectHolder(CPDF_Document* pDoc,
                                             CPDF_Dictionary* pDict,
                                             CPDF_Dictionary* pPageResources,
                                             CPDF_Dictionary* pResources)
    : m_pPageResources(pPageResources),
      m_pResources(pResources),
      m_pDict(pDict),
      m_pDocument(pDoc) {
  ASSERT(m_pDict);
}

CPDF_PageObjectHolder::~CPDF_PageObjectHolder() = default;

bool CPDF_PageObjectHolder::IsPage() const {
  return false;
}

void CPDF_PageObjectHolder::StartParse(
    std::unique_ptr<CPDF_ContentParser> pParser) {
  ASSERT(m_ParseState == ParseState::kNotParsed);
  m_pParser = std::move(pParser);
  m_ParseState = ParseState::kParsing;
}

void CPDF_PageObjectHolder::ContinueParse(PauseIndicatorIface* pPause) {
  if (m_ParseState == ParseState::kParsed)
    return;

  ASSERT(m_ParseState == ParseState::kParsing);
  if (m_pParser->Continue(pPause))
    return;

  m_ParseState = ParseState::kParsed;
  m_pDocument->IncrementParsedPageCount();
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

std::set<int32_t> CPDF_PageObjectHolder::TakeDirtyStreams() {
  auto dirty_streams = std::move(m_DirtyStreams);
  m_DirtyStreams.clear();
  return dirty_streams;
}

void CPDF_PageObjectHolder::LoadTransInfo() {
  CPDF_Dictionary* pGroup = m_pDict->GetDictFor("Group");
  if (!pGroup)
    return;

  if (pGroup->GetStringFor(pdfium::transparency::kGroupSubType) !=
      pdfium::transparency::kTransparency) {
    return;
  }
  m_Transparency.SetGroup();
  if (pGroup->GetIntegerFor(pdfium::transparency::kI))
    m_Transparency.SetIsolated();
}

CPDF_PageObject* CPDF_PageObjectHolder::GetPageObjectByIndex(
    size_t index) const {
  return pdfium::IndexInBounds(m_PageObjectList, index)
             ? m_PageObjectList[index].get()
             : nullptr;
}

void CPDF_PageObjectHolder::AppendPageObject(
    std::unique_ptr<CPDF_PageObject> pPageObj) {
  m_PageObjectList.push_back(std::move(pPageObj));
}

bool CPDF_PageObjectHolder::RemovePageObject(CPDF_PageObject* pPageObj) {
  pdfium::FakeUniquePtr<CPDF_PageObject> p(pPageObj);

  auto it =
      std::find(std::begin(m_PageObjectList), std::end(m_PageObjectList), p);
  if (it == std::end(m_PageObjectList))
    return false;

  it->release();
  m_PageObjectList.erase(it);

  int32_t content_stream = pPageObj->GetContentStream();
  if (content_stream >= 0)
    m_DirtyStreams.insert(content_stream);

  return true;
}

bool CPDF_PageObjectHolder::ErasePageObjectAtIndex(size_t index) {
  if (index >= m_PageObjectList.size())
    return false;

  m_PageObjectList.erase(m_PageObjectList.begin() + index);
  return true;
}
