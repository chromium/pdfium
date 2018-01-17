// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_page.h"

#include <set>
#include <utility>

#include "core/fpdfapi/cpdf_pagerendercontext.h"
#include "core/fpdfapi/page/cpdf_contentparser.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CPDF_Page::CPDF_Page(CPDF_Document* pDocument,
                     CPDF_Dictionary* pPageDict,
                     bool bPageCache)
    : CPDF_PageObjectHolder(pDocument, pPageDict),
      m_PageWidth(100),
      m_PageHeight(100),
      m_pView(nullptr) {
  if (bPageCache)
    m_pPageRender = pdfium::MakeUnique<CPDF_PageRenderCache>(this);
  if (!pPageDict)
    return;

  CPDF_Object* pPageAttr = GetPageAttr("Resources");
  m_pResources = pPageAttr ? pPageAttr->GetDict() : nullptr;
  m_pPageResources = m_pResources;

  CFX_FloatRect mediabox = GetBox("MediaBox");
  if (mediabox.IsEmpty())
    mediabox = CFX_FloatRect(0, 0, 612, 792);

  m_BBox = GetBox("CropBox");
  if (m_BBox.IsEmpty())
    m_BBox = mediabox;
  else
    m_BBox.Intersect(mediabox);

  m_PageWidth = m_BBox.Width();
  m_PageHeight = m_BBox.Height();

  int rotate = GetPageRotation();
  if (rotate % 2)
    std::swap(m_PageWidth, m_PageHeight);

  switch (rotate) {
    case 0:
      m_PageMatrix = CFX_Matrix(1.0f, 0, 0, 1.0f, -m_BBox.left, -m_BBox.bottom);
      break;
    case 1:
      m_PageMatrix =
          CFX_Matrix(0, -1.0f, 1.0f, 0, -m_BBox.bottom, m_BBox.right);
      break;
    case 2:
      m_PageMatrix = CFX_Matrix(-1.0f, 0, 0, -1.0f, m_BBox.right, m_BBox.top);
      break;
    case 3:
      m_PageMatrix = CFX_Matrix(0, 1.0f, -1.0f, 0, m_BBox.top, -m_BBox.left);
      break;
  }

  m_iTransparency = PDFTRANS_ISOLATED;
  LoadTransInfo();
}

CPDF_Page::~CPDF_Page() {}

bool CPDF_Page::IsPage() const {
  return true;
}

void CPDF_Page::StartParse() {
  if (m_ParseState == CONTENT_PARSED || m_ParseState == CONTENT_PARSING)
    return;

  m_pParser = pdfium::MakeUnique<CPDF_ContentParser>(this);
  m_ParseState = CONTENT_PARSING;
}

void CPDF_Page::ParseContent() {
  StartParse();
  ContinueParse(nullptr);
}

void CPDF_Page::SetRenderContext(
    std::unique_ptr<CPDF_PageRenderContext> pContext) {
  m_pRenderContext = std::move(pContext);
}

CPDF_Object* CPDF_Page::GetPageAttr(const ByteString& name) const {
  CPDF_Dictionary* pPageDict = m_pFormDict.Get();
  std::set<CPDF_Dictionary*> visited;
  while (1) {
    visited.insert(pPageDict);
    if (CPDF_Object* pObj = pPageDict->GetDirectObjectFor(name))
      return pObj;

    pPageDict = pPageDict->GetDictFor("Parent");
    if (!pPageDict || pdfium::ContainsKey(visited, pPageDict))
      break;
  }
  return nullptr;
}

CFX_FloatRect CPDF_Page::GetBox(const ByteString& name) const {
  CFX_FloatRect box;
  CPDF_Array* pBox = ToArray(GetPageAttr(name));
  if (pBox) {
    box = pBox->GetRect();
    box.Normalize();
  }
  return box;
}

CFX_Matrix CPDF_Page::GetDisplayMatrix(int xPos,
                                       int yPos,
                                       int xSize,
                                       int ySize,
                                       int iRotate) const {
  if (m_PageWidth == 0 || m_PageHeight == 0)
    return CFX_Matrix();

  float x0 = 0;
  float y0 = 0;
  float x1 = 0;
  float y1 = 0;
  float x2 = 0;
  float y2 = 0;
  iRotate %= 4;
  switch (iRotate) {
    case 0:
      x0 = xPos;
      y0 = yPos + ySize;
      x1 = xPos;
      y1 = yPos;
      x2 = xPos + xSize;
      y2 = yPos + ySize;
      break;
    case 1:
      x0 = xPos;
      y0 = yPos;
      x1 = xPos + xSize;
      y1 = yPos;
      x2 = xPos;
      y2 = yPos + ySize;
      break;
    case 2:
      x0 = xPos + xSize;
      y0 = yPos;
      x1 = xPos + xSize;
      y1 = yPos + ySize;
      x2 = xPos;
      y2 = yPos;
      break;
    case 3:
      x0 = xPos + xSize;
      y0 = yPos + ySize;
      x1 = xPos;
      y1 = yPos + ySize;
      x2 = xPos + xSize;
      y2 = yPos;
      break;
  }
  CFX_Matrix matrix = m_PageMatrix;
  matrix.Concat(CFX_Matrix((x2 - x0) / m_PageWidth, (y2 - y0) / m_PageWidth,
                           (x1 - x0) / m_PageHeight, (y1 - y0) / m_PageHeight,
                           x0, y0));
  return matrix;
}

int CPDF_Page::GetPageRotation() const {
  CPDF_Object* pRotate = GetPageAttr("Rotate");
  int rotate = pRotate ? (pRotate->GetInteger() / 90) % 4 : 0;
  return (rotate < 0) ? (rotate + 4) : rotate;
}

bool GraphicsData::operator<(const GraphicsData& other) const {
  if (fillAlpha != other.fillAlpha)
    return fillAlpha < other.fillAlpha;
  if (strokeAlpha != other.strokeAlpha)
    return strokeAlpha < other.strokeAlpha;
  return blendType < other.blendType;
}

bool FontData::operator<(const FontData& other) const {
  if (baseFont != other.baseFont)
    return baseFont < other.baseFont;
  return type < other.type;
}
