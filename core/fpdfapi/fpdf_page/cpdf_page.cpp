// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"

#include "core/fpdfapi/fpdf_page/include/cpdf_pageobject.h"
#include "core/fpdfapi/fpdf_page/pageint.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_object.h"
#include "core/fpdfapi/include/cpdf_modulemgr.h"
#include "core/fpdfapi/ipdf_rendermodule.h"

CPDF_Object* FPDFAPI_GetPageAttr(CPDF_Dictionary* pPageDict,
                                 const CFX_ByteStringC& name) {
  int level = 0;
  while (1) {
    CPDF_Object* pObj = pPageDict->GetDirectObjectBy(name);
    if (pObj) {
      return pObj;
    }
    CPDF_Dictionary* pParent = pPageDict->GetDictBy("Parent");
    if (!pParent || pParent == pPageDict) {
      return NULL;
    }
    pPageDict = pParent;
    level++;
    if (level == 1000) {
      return NULL;
    }
  }
}

CPDF_Page::CPDF_Page() : m_pPageRender(nullptr) {}

CPDF_Page::~CPDF_Page() {
  if (m_pPageRender) {
    IPDF_RenderModule* pModule = CPDF_ModuleMgr::Get()->GetRenderModule();
    pModule->DestroyPageCache(m_pPageRender);
  }
}

void CPDF_Page::Load(CPDF_Document* pDocument,
                     CPDF_Dictionary* pPageDict,
                     FX_BOOL bPageCache) {
  m_pDocument = (CPDF_Document*)pDocument;
  m_pFormDict = pPageDict;
  if (bPageCache) {
    m_pPageRender =
        CPDF_ModuleMgr::Get()->GetRenderModule()->CreatePageCache(this);
  }
  if (!pPageDict) {
    m_PageWidth = m_PageHeight = 100 * 1.0f;
    m_pPageResources = m_pResources = NULL;
    return;
  }
  CPDF_Object* pageAttr = GetPageAttr("Resources");
  m_pResources = pageAttr ? pageAttr->GetDict() : NULL;
  m_pPageResources = m_pResources;
  CPDF_Object* pRotate = GetPageAttr("Rotate");
  int rotate = 0;
  if (pRotate) {
    rotate = pRotate->GetInteger() / 90 % 4;
  }
  if (rotate < 0) {
    rotate += 4;
  }
  CPDF_Array* pMediaBox = ToArray(GetPageAttr("MediaBox"));
  CFX_FloatRect mediabox;
  if (pMediaBox) {
    mediabox = pMediaBox->GetRect();
    mediabox.Normalize();
  }
  if (mediabox.IsEmpty()) {
    mediabox = CFX_FloatRect(0, 0, 612, 792);
  }

  CPDF_Array* pCropBox = ToArray(GetPageAttr("CropBox"));
  if (pCropBox) {
    m_BBox = pCropBox->GetRect();
    m_BBox.Normalize();
  }
  if (m_BBox.IsEmpty()) {
    m_BBox = mediabox;
  } else {
    m_BBox.Intersect(mediabox);
  }
  if (rotate % 2) {
    m_PageHeight = m_BBox.right - m_BBox.left;
    m_PageWidth = m_BBox.top - m_BBox.bottom;
  } else {
    m_PageWidth = m_BBox.right - m_BBox.left;
    m_PageHeight = m_BBox.top - m_BBox.bottom;
  }
  switch (rotate) {
    case 0:
      m_PageMatrix.Set(1.0f, 0, 0, 1.0f, -m_BBox.left, -m_BBox.bottom);
      break;
    case 1:
      m_PageMatrix.Set(0, -1.0f, 1.0f, 0, -m_BBox.bottom, m_BBox.right);
      break;
    case 2:
      m_PageMatrix.Set(-1.0f, 0, 0, -1.0f, m_BBox.right, m_BBox.top);
      break;
    case 3:
      m_PageMatrix.Set(0, 1.0f, -1.0f, 0, m_BBox.top, -m_BBox.left);
      break;
  }
  m_Transparency = PDFTRANS_ISOLATED;
  LoadTransInfo();
}

void CPDF_Page::StartParse(CPDF_ParseOptions* pOptions) {
  if (m_ParseState == CONTENT_PARSED || m_ParseState == CONTENT_PARSING) {
    return;
  }
  m_pParser.reset(new CPDF_ContentParser);
  m_pParser->Start(this, pOptions);
  m_ParseState = CONTENT_PARSING;
}

void CPDF_Page::ParseContent(CPDF_ParseOptions* pOptions) {
  StartParse(pOptions);
  ContinueParse(nullptr);
}

CPDF_Object* CPDF_Page::GetPageAttr(const CFX_ByteStringC& name) const {
  return FPDFAPI_GetPageAttr(m_pFormDict, name);
}

void CPDF_Page::GetDisplayMatrix(CFX_Matrix& matrix,
                                 int xPos,
                                 int yPos,
                                 int xSize,
                                 int ySize,
                                 int iRotate) const {
  if (m_PageWidth == 0 || m_PageHeight == 0) {
    return;
  }
  CFX_Matrix display_matrix;
  int x0 = 0;
  int y0 = 0;
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;
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
  display_matrix.Set(
      ((FX_FLOAT)(x2 - x0)) / m_PageWidth, ((FX_FLOAT)(y2 - y0)) / m_PageWidth,
      ((FX_FLOAT)(x1 - x0)) / m_PageHeight,
      ((FX_FLOAT)(y1 - y0)) / m_PageHeight, (FX_FLOAT)x0, (FX_FLOAT)y0);
  matrix = m_PageMatrix;
  matrix.Concat(display_matrix);
}
