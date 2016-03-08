// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/src/fpdfapi/fpdf_page/pageint.h"

#include <algorithm>

#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_page.h"
#include "third_party/base/stl_util.h"

CPDF_PageObject::CPDF_PageObject() {}

CPDF_PageObject::~CPDF_PageObject() {}

void CPDF_PageObject::CopyData(const CPDF_PageObject* pSrc) {
  CopyStates(*pSrc);
  m_Left = pSrc->m_Left;
  m_Right = pSrc->m_Right;
  m_Top = pSrc->m_Top;
  m_Bottom = pSrc->m_Bottom;
}

void CPDF_PageObject::TransformClipPath(CFX_Matrix& matrix) {
  if (m_ClipPath.IsNull()) {
    return;
  }
  m_ClipPath.GetModify();
  m_ClipPath.Transform(matrix);
}

void CPDF_PageObject::TransformGeneralState(CFX_Matrix& matrix) {
  if (m_GeneralState.IsNull()) {
    return;
  }
  CPDF_GeneralStateData* pGS = m_GeneralState.GetModify();
  pGS->m_Matrix.Concat(matrix);
}

FX_RECT CPDF_PageObject::GetBBox(const CFX_Matrix* pMatrix) const {
  CFX_FloatRect rect(m_Left, m_Bottom, m_Right, m_Top);
  if (pMatrix) {
    pMatrix->TransformRect(rect);
  }
  return rect.GetOutterRect();
}

CPDF_TextObject::CPDF_TextObject()
    : m_PosX(0),
      m_PosY(0),
      m_nChars(0),
      m_pCharCodes(nullptr),
      m_pCharPos(nullptr) {}

CPDF_TextObject::~CPDF_TextObject() {
  if (m_nChars > 1) {
    FX_Free(m_pCharCodes);
  }
  FX_Free(m_pCharPos);
}

void CPDF_TextObject::GetItemInfo(int index, CPDF_TextObjectItem* pInfo) const {
  pInfo->m_CharCode =
      m_nChars == 1 ? (FX_DWORD)(uintptr_t)m_pCharCodes : m_pCharCodes[index];
  pInfo->m_OriginX = index ? m_pCharPos[index - 1] : 0;
  pInfo->m_OriginY = 0;
  if (pInfo->m_CharCode == -1) {
    return;
  }
  CPDF_Font* pFont = m_TextState.GetFont();
  if (!pFont->IsCIDFont()) {
    return;
  }
  if (!pFont->AsCIDFont()->IsVertWriting()) {
    return;
  }
  FX_WORD CID = pFont->AsCIDFont()->CIDFromCharCode(pInfo->m_CharCode);
  pInfo->m_OriginY = pInfo->m_OriginX;
  pInfo->m_OriginX = 0;
  short vx, vy;
  pFont->AsCIDFont()->GetVertOrigin(CID, vx, vy);
  FX_FLOAT fontsize = m_TextState.GetFontSize();
  pInfo->m_OriginX -= fontsize * vx / 1000;
  pInfo->m_OriginY -= fontsize * vy / 1000;
}

int CPDF_TextObject::CountChars() const {
  if (m_nChars == 1) {
    return 1;
  }
  int count = 0;
  for (int i = 0; i < m_nChars; ++i)
    if (m_pCharCodes[i] != (FX_DWORD)-1) {
      ++count;
    }
  return count;
}

void CPDF_TextObject::GetCharInfo(int index,
                                  FX_DWORD& charcode,
                                  FX_FLOAT& kerning) const {
  if (m_nChars == 1) {
    charcode = (FX_DWORD)(uintptr_t)m_pCharCodes;
    kerning = 0;
    return;
  }
  int count = 0;
  for (int i = 0; i < m_nChars; ++i) {
    if (m_pCharCodes[i] != (FX_DWORD)-1) {
      if (count == index) {
        charcode = m_pCharCodes[i];
        if (i == m_nChars - 1 || m_pCharCodes[i + 1] != (FX_DWORD)-1) {
          kerning = 0;
        } else {
          kerning = m_pCharPos[i];
        }
        return;
      }
      ++count;
    }
  }
}

void CPDF_TextObject::GetCharInfo(int index, CPDF_TextObjectItem* pInfo) const {
  if (m_nChars == 1) {
    GetItemInfo(0, pInfo);
    return;
  }
  int count = 0;
  for (int i = 0; i < m_nChars; ++i) {
    FX_DWORD charcode = m_pCharCodes[i];
    if (charcode == (FX_DWORD)-1) {
      continue;
    }
    if (count == index) {
      GetItemInfo(i, pInfo);
      break;
    }
    ++count;
  }
}

CPDF_TextObject* CPDF_TextObject::Clone() const {
  CPDF_TextObject* obj = new CPDF_TextObject;
  obj->CopyData(this);

  obj->m_nChars = m_nChars;
  if (m_nChars > 1) {
    obj->m_pCharCodes = FX_Alloc(FX_DWORD, m_nChars);
    FXSYS_memcpy(obj->m_pCharCodes, m_pCharCodes, m_nChars * sizeof(FX_DWORD));
    obj->m_pCharPos = FX_Alloc(FX_FLOAT, m_nChars - 1);
    FXSYS_memcpy(obj->m_pCharPos, m_pCharPos,
                 (m_nChars - 1) * sizeof(FX_FLOAT));
  } else {
    obj->m_pCharCodes = m_pCharCodes;
  }
  obj->m_PosX = m_PosX;
  obj->m_PosY = m_PosY;
  return obj;
}

void CPDF_TextObject::GetTextMatrix(CFX_Matrix* pMatrix) const {
  FX_FLOAT* pTextMatrix = m_TextState.GetMatrix();
  pMatrix->Set(pTextMatrix[0], pTextMatrix[2], pTextMatrix[1], pTextMatrix[3],
               m_PosX, m_PosY);
}

void CPDF_TextObject::SetSegments(const CFX_ByteString* pStrs,
                                  FX_FLOAT* pKerning,
                                  int nsegs) {
  if (m_nChars > 1) {
    FX_Free(m_pCharCodes);
    m_pCharCodes = nullptr;
  }
  FX_Free(m_pCharPos);
  m_pCharPos = nullptr;
  CPDF_Font* pFont = m_TextState.GetFont();
  m_nChars = 0;
  for (int i = 0; i < nsegs; ++i) {
    m_nChars += pFont->CountChar(pStrs[i], pStrs[i].GetLength());
  }
  m_nChars += nsegs - 1;
  if (m_nChars > 1) {
    m_pCharCodes = FX_Alloc(FX_DWORD, m_nChars);
    m_pCharPos = FX_Alloc(FX_FLOAT, m_nChars - 1);
    int index = 0;
    for (int i = 0; i < nsegs; ++i) {
      const FX_CHAR* segment = pStrs[i];
      int offset = 0, len = pStrs[i].GetLength();
      while (offset < len) {
        m_pCharCodes[index++] = pFont->GetNextChar(segment, len, offset);
      }
      if (i != nsegs - 1) {
        m_pCharPos[index - 1] = pKerning[i];
        m_pCharCodes[index++] = (FX_DWORD)-1;
      }
    }
  } else {
    int offset = 0;
    m_pCharCodes = (FX_DWORD*)(uintptr_t)pFont->GetNextChar(
        pStrs[0], pStrs[0].GetLength(), offset);
  }
}

void CPDF_TextObject::SetText(const CFX_ByteString& str) {
  SetSegments(&str, nullptr, 1);
  RecalcPositionData();
}

FX_FLOAT CPDF_TextObject::GetCharWidth(FX_DWORD charcode) const {
  FX_FLOAT fontsize = m_TextState.GetFontSize() / 1000;
  CPDF_Font* pFont = m_TextState.GetFont();
  FX_BOOL bVertWriting = FALSE;
  CPDF_CIDFont* pCIDFont = pFont->AsCIDFont();
  if (pCIDFont) {
    bVertWriting = pCIDFont->IsVertWriting();
  }
  if (!bVertWriting)
    return pFont->GetCharWidthF(charcode, 0) * fontsize;

  FX_WORD CID = pCIDFont->CIDFromCharCode(charcode);
  return pCIDFont->GetVertWidth(CID) * fontsize;
}

void CPDF_TextObject::CalcPositionData(FX_FLOAT* pTextAdvanceX,
                                       FX_FLOAT* pTextAdvanceY,
                                       FX_FLOAT horz_scale,
                                       int level) {
  FX_FLOAT curpos = 0;
  FX_FLOAT min_x = 10000 * 1.0f;
  FX_FLOAT max_x = -10000 * 1.0f;
  FX_FLOAT min_y = 10000 * 1.0f;
  FX_FLOAT max_y = -10000 * 1.0f;
  CPDF_Font* pFont = m_TextState.GetFont();
  FX_BOOL bVertWriting = FALSE;
  CPDF_CIDFont* pCIDFont = pFont->AsCIDFont();
  if (pCIDFont) {
    bVertWriting = pCIDFont->IsVertWriting();
  }
  FX_FLOAT fontsize = m_TextState.GetFontSize();
  for (int i = 0; i < m_nChars; ++i) {
    FX_DWORD charcode =
        m_nChars == 1 ? (FX_DWORD)(uintptr_t)m_pCharCodes : m_pCharCodes[i];
    if (i > 0) {
      if (charcode == (FX_DWORD)-1) {
        curpos -= (m_pCharPos[i - 1] * fontsize) / 1000;
        continue;
      }
      m_pCharPos[i - 1] = curpos;
    }
    FX_RECT char_rect;
    pFont->GetCharBBox(charcode, char_rect, level);
    FX_FLOAT charwidth;
    if (!bVertWriting) {
      if (min_y > char_rect.top) {
        min_y = (FX_FLOAT)char_rect.top;
      }
      if (max_y < char_rect.top) {
        max_y = (FX_FLOAT)char_rect.top;
      }
      if (min_y > char_rect.bottom) {
        min_y = (FX_FLOAT)char_rect.bottom;
      }
      if (max_y < char_rect.bottom) {
        max_y = (FX_FLOAT)char_rect.bottom;
      }
      FX_FLOAT char_left = curpos + char_rect.left * fontsize / 1000;
      FX_FLOAT char_right = curpos + char_rect.right * fontsize / 1000;
      if (min_x > char_left) {
        min_x = char_left;
      }
      if (max_x < char_left) {
        max_x = char_left;
      }
      if (min_x > char_right) {
        min_x = char_right;
      }
      if (max_x < char_right) {
        max_x = char_right;
      }
      charwidth = pFont->GetCharWidthF(charcode, level) * fontsize / 1000;
    } else {
      FX_WORD CID = pCIDFont->CIDFromCharCode(charcode);
      short vx;
      short vy;
      pCIDFont->GetVertOrigin(CID, vx, vy);
      char_rect.left -= vx;
      char_rect.right -= vx;
      char_rect.top -= vy;
      char_rect.bottom -= vy;
      if (min_x > char_rect.left) {
        min_x = (FX_FLOAT)char_rect.left;
      }
      if (max_x < char_rect.left) {
        max_x = (FX_FLOAT)char_rect.left;
      }
      if (min_x > char_rect.right) {
        min_x = (FX_FLOAT)char_rect.right;
      }
      if (max_x < char_rect.right) {
        max_x = (FX_FLOAT)char_rect.right;
      }
      FX_FLOAT char_top = curpos + char_rect.top * fontsize / 1000;
      FX_FLOAT char_bottom = curpos + char_rect.bottom * fontsize / 1000;
      if (min_y > char_top) {
        min_y = char_top;
      }
      if (max_y < char_top) {
        max_y = char_top;
      }
      if (min_y > char_bottom) {
        min_y = char_bottom;
      }
      if (max_y < char_bottom) {
        max_y = char_bottom;
      }
      charwidth = pCIDFont->GetVertWidth(CID) * fontsize / 1000;
    }
    curpos += charwidth;
    if (charcode == ' ' && (!pCIDFont || pCIDFont->GetCharSize(32) == 1)) {
      curpos += m_TextState.GetObject()->m_WordSpace;
    }
    curpos += m_TextState.GetObject()->m_CharSpace;
  }
  if (bVertWriting) {
    if (pTextAdvanceX) {
      *pTextAdvanceX = 0;
    }
    if (pTextAdvanceY) {
      *pTextAdvanceY = curpos;
    }
    min_x = min_x * fontsize / 1000;
    max_x = max_x * fontsize / 1000;
  } else {
    if (pTextAdvanceX) {
      *pTextAdvanceX = curpos * horz_scale;
    }
    if (pTextAdvanceY) {
      *pTextAdvanceY = 0;
    }
    min_y = min_y * fontsize / 1000;
    max_y = max_y * fontsize / 1000;
  }
  CFX_Matrix matrix;
  GetTextMatrix(&matrix);
  m_Left = min_x;
  m_Right = max_x;
  m_Bottom = min_y;
  m_Top = max_y;
  matrix.TransformRect(m_Left, m_Right, m_Top, m_Bottom);
  int textmode = m_TextState.GetObject()->m_TextMode;
  if (textmode == 1 || textmode == 2 || textmode == 5 || textmode == 6) {
    FX_FLOAT half_width = m_GraphState.GetObject()->m_LineWidth / 2;
    m_Left -= half_width;
    m_Right += half_width;
    m_Top += half_width;
    m_Bottom -= half_width;
  }
}

void CPDF_TextObject::Transform(const CFX_Matrix& matrix) {
  m_TextState.GetModify();
  CFX_Matrix text_matrix;
  GetTextMatrix(&text_matrix);
  text_matrix.Concat(matrix);
  FX_FLOAT* pTextMatrix = m_TextState.GetMatrix();
  pTextMatrix[0] = text_matrix.GetA();
  pTextMatrix[1] = text_matrix.GetC();
  pTextMatrix[2] = text_matrix.GetB();
  pTextMatrix[3] = text_matrix.GetD();
  m_PosX = text_matrix.GetE();
  m_PosY = text_matrix.GetF();
  CalcPositionData(nullptr, nullptr, 0);
}

void CPDF_TextObject::SetPosition(FX_FLOAT x, FX_FLOAT y) {
  FX_FLOAT dx = x - m_PosX;
  FX_FLOAT dy = y - m_PosY;
  m_PosX = x;
  m_PosY = y;
  m_Left += dx;
  m_Right += dx;
  m_Top += dy;
  m_Bottom += dy;
}

CPDF_ShadingObject::CPDF_ShadingObject() : m_pShading(nullptr) {}

CPDF_ShadingObject::~CPDF_ShadingObject() {}

CPDF_ShadingObject* CPDF_ShadingObject::Clone() const {
  CPDF_ShadingObject* obj = new CPDF_ShadingObject;
  obj->CopyData(this);

  obj->m_pShading = m_pShading;
  if (obj->m_pShading && obj->m_pShading->m_pDocument) {
    CPDF_DocPageData* pDocPageData =
        obj->m_pShading->m_pDocument->GetPageData();
    obj->m_pShading = (CPDF_ShadingPattern*)pDocPageData->GetPattern(
        obj->m_pShading->m_pShadingObj, m_pShading->m_bShadingObj,
        &obj->m_pShading->m_ParentMatrix);
  }
  obj->m_Matrix = m_Matrix;
  return obj;
}

void CPDF_ShadingObject::Transform(const CFX_Matrix& matrix) {
  if (!m_ClipPath.IsNull()) {
    m_ClipPath.GetModify();
    m_ClipPath.Transform(matrix);
  }
  m_Matrix.Concat(matrix);
  if (!m_ClipPath.IsNull()) {
    CalcBoundingBox();
  } else {
    matrix.TransformRect(m_Left, m_Right, m_Top, m_Bottom);
  }
}

void CPDF_ShadingObject::CalcBoundingBox() {
  if (m_ClipPath.IsNull()) {
    return;
  }
  CFX_FloatRect rect = m_ClipPath.GetClipBox();
  m_Left = rect.left;
  m_Bottom = rect.bottom;
  m_Right = rect.right;
  m_Top = rect.top;
}

CPDF_FormObject::CPDF_FormObject() : m_pForm(nullptr) {}

CPDF_FormObject::~CPDF_FormObject() {
  delete m_pForm;
}

void CPDF_FormObject::Transform(const CFX_Matrix& matrix) {
  m_FormMatrix.Concat(matrix);
  CalcBoundingBox();
}

CPDF_FormObject* CPDF_FormObject::Clone() const {
  CPDF_FormObject* obj = new CPDF_FormObject;
  obj->CopyData(this);

  obj->m_pForm = m_pForm->Clone();
  obj->m_FormMatrix = m_FormMatrix;
  return obj;
}

void CPDF_FormObject::CalcBoundingBox() {
  CFX_FloatRect form_rect = m_pForm->CalcBoundingBox();
  form_rect.Transform(&m_FormMatrix);
  m_Left = form_rect.left;
  m_Bottom = form_rect.bottom;
  m_Right = form_rect.right;
  m_Top = form_rect.top;
}

CPDF_PageObject* CPDF_PageObjectList::GetPageObjectByIndex(int index) {
  if (index < 0 || index >= pdfium::CollectionSize<int>(*this))
    return nullptr;
  return (*this)[index].get();
}

CPDF_PageObjectHolder::CPDF_PageObjectHolder()
    : m_pFormDict(nullptr),
      m_pFormStream(nullptr),
      m_pDocument(nullptr),
      m_pPageResources(nullptr),
      m_pResources(nullptr),
      m_Transparency(0),
      m_bBackgroundAlphaNeeded(FALSE),
      m_bHasImageMask(FALSE),
      m_ParseState(CONTENT_NOT_PARSED) {}

void CPDF_PageObjectHolder::ContinueParse(IFX_Pause* pPause) {
  if (!m_pParser) {
    return;
  }
  m_pParser->Continue(pPause);
  if (m_pParser->GetStatus() == CPDF_ContentParser::Done) {
    m_ParseState = CONTENT_PARSED;
    m_pParser.reset();
  }
}

void CPDF_PageObjectHolder::Transform(const CFX_Matrix& matrix) {
  for (auto& pObj : m_PageObjectList)
    pObj->Transform(matrix);
}

CFX_FloatRect CPDF_PageObjectHolder::CalcBoundingBox() const {
  if (m_PageObjectList.empty())
    return CFX_FloatRect(0, 0, 0, 0);

  FX_FLOAT left = 1000000.0f;
  FX_FLOAT right = -1000000.0f;
  FX_FLOAT bottom = 1000000.0f;
  FX_FLOAT top = -1000000.0f;
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
  CPDF_Dictionary* pGroup = m_pFormDict->GetDictBy("Group");
  if (!pGroup) {
    return;
  }
  if (pGroup->GetStringBy("S") != "Transparency") {
    return;
  }
  m_Transparency |= PDFTRANS_GROUP;
  if (pGroup->GetIntegerBy("I")) {
    m_Transparency |= PDFTRANS_ISOLATED;
  }
  if (pGroup->GetIntegerBy("K")) {
    m_Transparency |= PDFTRANS_KNOCKOUT;
  }
}

CPDF_Page::CPDF_Page() : m_pPageRender(nullptr) {}

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

CPDF_Page::~CPDF_Page() {
  if (m_pPageRender) {
    IPDF_RenderModule* pModule = CPDF_ModuleMgr::Get()->GetRenderModule();
    pModule->DestroyPageCache(m_pPageRender);
  }
}

CPDF_Object* FPDFAPI_GetPageAttr(CPDF_Dictionary* pPageDict,
                                 const CFX_ByteStringC& name) {
  int level = 0;
  while (1) {
    CPDF_Object* pObj = pPageDict->GetElementValue(name);
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

CPDF_Object* CPDF_Page::GetPageAttr(const CFX_ByteStringC& name) const {
  return FPDFAPI_GetPageAttr(m_pFormDict, name);
}

CPDF_Form::CPDF_Form(CPDF_Document* pDoc,
                     CPDF_Dictionary* pPageResources,
                     CPDF_Stream* pFormStream,
                     CPDF_Dictionary* pParentResources) {
  m_pDocument = pDoc;
  m_pFormStream = pFormStream;
  m_pFormDict = pFormStream ? pFormStream->GetDict() : NULL;
  m_pResources = m_pFormDict->GetDictBy("Resources");
  m_pPageResources = pPageResources;
  if (!m_pResources) {
    m_pResources = pParentResources;
  }
  if (!m_pResources) {
    m_pResources = pPageResources;
  }
  m_Transparency = 0;
  LoadTransInfo();
}

CPDF_Form::~CPDF_Form() {}

void CPDF_Form::StartParse(CPDF_AllStates* pGraphicStates,
                           CFX_Matrix* pParentMatrix,
                           CPDF_Type3Char* pType3Char,
                           CPDF_ParseOptions* pOptions,
                           int level) {
  if (m_ParseState == CONTENT_PARSED || m_ParseState == CONTENT_PARSING) {
    return;
  }
  m_pParser.reset(new CPDF_ContentParser);
  m_pParser->Start(this, pGraphicStates, pParentMatrix, pType3Char, pOptions,
                   level);
  m_ParseState = CONTENT_PARSING;
}

void CPDF_Form::ParseContent(CPDF_AllStates* pGraphicStates,
                             CFX_Matrix* pParentMatrix,
                             CPDF_Type3Char* pType3Char,
                             CPDF_ParseOptions* pOptions,
                             int level) {
  StartParse(pGraphicStates, pParentMatrix, pType3Char, pOptions, level);
  ContinueParse(NULL);
}

CPDF_Form* CPDF_Form::Clone() const {
  CPDF_Form* pCloneForm =
      new CPDF_Form(m_pDocument, m_pPageResources, m_pFormStream, m_pResources);
  for (const auto& pObj : m_PageObjectList)
    pCloneForm->m_PageObjectList.emplace_back(pObj->Clone());

  return pCloneForm;
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
  int x0, y0, x1, y1, x2, y2;
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

CPDF_ParseOptions::CPDF_ParseOptions() {
  m_bTextOnly = FALSE;
  m_bMarkedContent = TRUE;
  m_bSeparateForm = TRUE;
  m_bDecodeInlineImage = FALSE;
}
