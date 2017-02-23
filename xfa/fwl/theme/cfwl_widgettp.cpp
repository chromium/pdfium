// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_widgettp.h"

#include <algorithm>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fwl/ifwl_themeprovider.h"
#include "xfa/fxgraphics/cfx_color.h"
#include "xfa/fxgraphics/cfx_path.h"
#include "xfa/fxgraphics/cfx_shading.h"

CFWL_WidgetTP::CFWL_WidgetTP()
    : m_dwRefCount(1), m_pFDEFont(nullptr), m_pColorData(nullptr) {}

CFWL_WidgetTP::~CFWL_WidgetTP() {}

void CFWL_WidgetTP::Initialize() {}

void CFWL_WidgetTP::Finalize() {
  if (m_pTextOut)
    FinalizeTTO();
}

void CFWL_WidgetTP::DrawBackground(CFWL_ThemeBackground* pParams) {}

void CFWL_WidgetTP::DrawText(CFWL_ThemeText* pParams) {
  if (!m_pTextOut)
    InitTTO();

  int32_t iLen = pParams->m_wsText.GetLength();
  if (iLen <= 0)
    return;

  CFX_Graphics* pGraphics = pParams->m_pGraphics;
  m_pTextOut->SetRenderDevice(pGraphics->GetRenderDevice());
  m_pTextOut->SetStyles(pParams->m_dwTTOStyles);
  m_pTextOut->SetAlignment(pParams->m_iTTOAlign);

  CFX_Matrix* pMatrix = &pParams->m_matrix;
  pMatrix->Concat(*pGraphics->GetMatrix());
  m_pTextOut->SetMatrix(*pMatrix);
  m_pTextOut->DrawLogicText(pParams->m_wsText.c_str(), iLen, pParams->m_rtPart);
}

void CFWL_WidgetTP::InitializeArrowColorData() {
  if (m_pColorData)
    return;

  m_pColorData = pdfium::MakeUnique<CColorData>();
  m_pColorData->clrBorder[0] = ArgbEncode(255, 202, 216, 249);
  m_pColorData->clrBorder[1] = ArgbEncode(255, 171, 190, 233);
  m_pColorData->clrBorder[2] = ArgbEncode(255, 135, 147, 219);
  m_pColorData->clrBorder[3] = ArgbEncode(255, 172, 168, 153);
  m_pColorData->clrStart[0] = ArgbEncode(255, 225, 234, 254);
  m_pColorData->clrStart[1] = ArgbEncode(255, 253, 255, 255);
  m_pColorData->clrStart[2] = ArgbEncode(255, 110, 142, 241);
  m_pColorData->clrStart[3] = ArgbEncode(255, 254, 254, 251);
  m_pColorData->clrEnd[0] = ArgbEncode(255, 175, 204, 251);
  m_pColorData->clrEnd[1] = ArgbEncode(255, 185, 218, 251);
  m_pColorData->clrEnd[2] = ArgbEncode(255, 210, 222, 235);
  m_pColorData->clrEnd[3] = ArgbEncode(255, 243, 241, 236);
  m_pColorData->clrSign[0] = ArgbEncode(255, 77, 97, 133);
  m_pColorData->clrSign[1] = ArgbEncode(255, 77, 97, 133);
  m_pColorData->clrSign[2] = ArgbEncode(255, 77, 97, 133);
  m_pColorData->clrSign[3] = ArgbEncode(255, 128, 128, 128);
}


void CFWL_WidgetTP::InitTTO() {
  if (m_pTextOut)
    return;

  m_pFDEFont = CFWL_FontManager::GetInstance()->FindFont(L"Helvetica", 0, 0);
  m_pTextOut = pdfium::MakeUnique<CFDE_TextOut>();
  m_pTextOut->SetFont(m_pFDEFont);
  m_pTextOut->SetFontSize(FWLTHEME_CAPACITY_FontSize);
  m_pTextOut->SetTextColor(FWLTHEME_CAPACITY_TextColor);
  m_pTextOut->SetEllipsisString(L"...");
}

void CFWL_WidgetTP::FinalizeTTO() {
  m_pTextOut.reset();
}

void CFWL_WidgetTP::DrawBorder(CFX_Graphics* pGraphics,
                               const CFX_RectF* pRect,
                               CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return;
  if (!pRect)
    return;
  CFX_Path path;
  path.AddRectangle(pRect->left, pRect->top, pRect->width, pRect->height);
  path.AddRectangle(pRect->left + 1, pRect->top + 1, pRect->width - 2,
                    pRect->height - 2);
  pGraphics->SaveGraphState();
  CFX_Color crFill(ArgbEncode(255, 0, 0, 0));
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_ALTERNATE, pMatrix);
  pGraphics->RestoreGraphState();
}

void CFWL_WidgetTP::FillBackground(CFX_Graphics* pGraphics,
                                   const CFX_RectF* pRect,
                                   CFX_Matrix* pMatrix) {
  FillSoildRect(pGraphics, FWLTHEME_COLOR_Background, pRect, pMatrix);
}

void CFWL_WidgetTP::FillSoildRect(CFX_Graphics* pGraphics,
                                  FX_ARGB fillColor,
                                  const CFX_RectF* pRect,
                                  CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return;
  if (!pRect)
    return;
  pGraphics->SaveGraphState();
  CFX_Color crFill(fillColor);
  pGraphics->SetFillColor(&crFill);
  CFX_Path path;
  path.AddRectangle(pRect->left, pRect->top, pRect->width, pRect->height);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  pGraphics->RestoreGraphState();
}

void CFWL_WidgetTP::DrawAxialShading(CFX_Graphics* pGraphics,
                                     FX_FLOAT fx1,
                                     FX_FLOAT fy1,
                                     FX_FLOAT fx2,
                                     FX_FLOAT fy2,
                                     FX_ARGB beginColor,
                                     FX_ARGB endColor,
                                     CFX_Path* path,
                                     int32_t fillMode,
                                     CFX_Matrix* pMatrix) {
  if (!pGraphics || !path)
    return;

  CFX_PointF begPoint(fx1, fy1);
  CFX_PointF endPoint(fx2, fy2);
  CFX_Shading shading(begPoint, endPoint, false, false, beginColor, endColor);
  pGraphics->SaveGraphState();
  CFX_Color color1(&shading);
  pGraphics->SetFillColor(&color1);
  pGraphics->FillPath(path, fillMode, pMatrix);
  pGraphics->RestoreGraphState();
}

void CFWL_WidgetTP::DrawFocus(CFX_Graphics* pGraphics,
                              const CFX_RectF* pRect,
                              CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return;
  if (!pRect)
    return;
  pGraphics->SaveGraphState();
  CFX_Color cr(0xFF000000);
  pGraphics->SetStrokeColor(&cr);
  FX_FLOAT DashPattern[2] = {1, 1};
  pGraphics->SetLineDash(0.0f, DashPattern, 2);
  CFX_Path path;
  path.AddRectangle(pRect->left, pRect->top, pRect->width, pRect->height);
  pGraphics->StrokePath(&path, pMatrix);
  pGraphics->RestoreGraphState();
}

void CFWL_WidgetTP::DrawArrow(CFX_Graphics* pGraphics,
                              const CFX_RectF* pRect,
                              FWLTHEME_DIRECTION eDict,
                              FX_ARGB argSign,
                              CFX_Matrix* pMatrix) {
  bool bVert =
      (eDict == FWLTHEME_DIRECTION_Up || eDict == FWLTHEME_DIRECTION_Down);
  FX_FLOAT fLeft =
      (FX_FLOAT)(((pRect->width - (bVert ? 9 : 6)) / 2 + pRect->left) + 0.5);
  FX_FLOAT fTop =
      (FX_FLOAT)(((pRect->height - (bVert ? 6 : 9)) / 2 + pRect->top) + 0.5);
  CFX_Path path;
  switch (eDict) {
    case FWLTHEME_DIRECTION_Down: {
      path.MoveTo(CFX_PointF(fLeft, fTop + 1));
      path.LineTo(CFX_PointF(fLeft + 4, fTop + 5));
      path.LineTo(CFX_PointF(fLeft + 8, fTop + 1));
      path.LineTo(CFX_PointF(fLeft + 7, fTop));
      path.LineTo(CFX_PointF(fLeft + 4, fTop + 3));
      path.LineTo(CFX_PointF(fLeft + 1, fTop));
      break;
    }
    case FWLTHEME_DIRECTION_Up: {
      path.MoveTo(CFX_PointF(fLeft, fTop + 4));
      path.LineTo(CFX_PointF(fLeft + 4, fTop));
      path.LineTo(CFX_PointF(fLeft + 8, fTop + 4));
      path.LineTo(CFX_PointF(fLeft + 7, fTop + 5));
      path.LineTo(CFX_PointF(fLeft + 4, fTop + 2));
      path.LineTo(CFX_PointF(fLeft + 1, fTop + 5));
      break;
    }
    case FWLTHEME_DIRECTION_Right: {
      path.MoveTo(CFX_PointF(fLeft + 1, fTop));
      path.LineTo(CFX_PointF(fLeft + 5, fTop + 4));
      path.LineTo(CFX_PointF(fLeft + 1, fTop + 8));
      path.LineTo(CFX_PointF(fLeft, fTop + 7));
      path.LineTo(CFX_PointF(fLeft + 3, fTop + 4));
      path.LineTo(CFX_PointF(fLeft, fTop + 1));
      break;
    }
    case FWLTHEME_DIRECTION_Left: {
      path.MoveTo(CFX_PointF(fLeft, fTop + 4));
      path.LineTo(CFX_PointF(fLeft + 4, fTop));
      path.LineTo(CFX_PointF(fLeft + 5, fTop + 1));
      path.LineTo(CFX_PointF(fLeft + 2, fTop + 4));
      path.LineTo(CFX_PointF(fLeft + 5, fTop + 7));
      path.LineTo(CFX_PointF(fLeft + 4, fTop + 8));
      break;
    }
  }
  CFX_Color cr(argSign);
  pGraphics->SetFillColor(&cr);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
}

void CFWL_WidgetTP::DrawBtn(CFX_Graphics* pGraphics,
                            const CFX_RectF* pRect,
                            FWLTHEME_STATE eState,
                            CFX_Matrix* pMatrix) {
  CFX_Path path;
  InitializeArrowColorData();

  FX_FLOAT fRight = pRect->right();
  FX_FLOAT fBottom = pRect->bottom();
  path.AddRectangle(pRect->left, pRect->top, pRect->width, pRect->height);
  DrawAxialShading(pGraphics, pRect->left, pRect->top, fRight, fBottom,
                   m_pColorData->clrStart[eState - 1],
                   m_pColorData->clrEnd[eState - 1], &path, FXFILL_WINDING,
                   pMatrix);

  CFX_Color rcStroke;
  rcStroke.Set(m_pColorData->clrBorder[eState - 1]);
  pGraphics->SetStrokeColor(&rcStroke);
  pGraphics->StrokePath(&path, pMatrix);
}

void CFWL_WidgetTP::DrawArrowBtn(CFX_Graphics* pGraphics,
                                 const CFX_RectF* pRect,
                                 FWLTHEME_DIRECTION eDict,
                                 FWLTHEME_STATE eState,
                                 CFX_Matrix* pMatrix) {
  DrawBtn(pGraphics, pRect, eState, pMatrix);

  InitializeArrowColorData();
  DrawArrow(pGraphics, pRect, eDict, m_pColorData->clrSign[eState - 1],
            pMatrix);
}

CFWL_FontData::CFWL_FontData() : m_dwStyles(0), m_dwCodePage(0) {}

CFWL_FontData::~CFWL_FontData() {}

bool CFWL_FontData::Equal(const CFX_WideStringC& wsFontFamily,
                          uint32_t dwFontStyles,
                          uint16_t wCodePage) {
  return m_wsFamily == wsFontFamily && m_dwStyles == dwFontStyles &&
         m_dwCodePage == wCodePage;
}

bool CFWL_FontData::LoadFont(const CFX_WideStringC& wsFontFamily,
                             uint32_t dwFontStyles,
                             uint16_t dwCodePage) {
  m_wsFamily = wsFontFamily;
  m_dwStyles = dwFontStyles;
  m_dwCodePage = dwCodePage;
  if (!m_pFontMgr) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    m_pFontMgr = CFGAS_FontMgr::Create(FX_GetDefFontEnumerator());
#else
    m_pFontSource = pdfium::MakeUnique<CFX_FontSourceEnum_File>();
    m_pFontMgr = CFGAS_FontMgr::Create(m_pFontSource.get());
#endif
  }
  m_pFont = CFGAS_GEFont::LoadFont(wsFontFamily.c_str(), dwFontStyles,
                                   dwCodePage, m_pFontMgr.get());
  return !!m_pFont;
}

CFWL_FontManager* CFWL_FontManager::s_FontManager = nullptr;
CFWL_FontManager* CFWL_FontManager::GetInstance() {
  if (!s_FontManager)
    s_FontManager = new CFWL_FontManager;
  return s_FontManager;
}

void CFWL_FontManager::DestroyInstance() {
  delete s_FontManager;
  s_FontManager = nullptr;
}

CFWL_FontManager::CFWL_FontManager() {}

CFWL_FontManager::~CFWL_FontManager() {}

CFX_RetainPtr<CFGAS_GEFont> CFWL_FontManager::FindFont(
    const CFX_WideStringC& wsFontFamily,
    uint32_t dwFontStyles,
    uint16_t wCodePage) {
  for (const auto& pData : m_FontsArray) {
    if (pData->Equal(wsFontFamily, dwFontStyles, wCodePage))
      return pData->GetFont();
  }
  auto pFontData = pdfium::MakeUnique<CFWL_FontData>();
  if (!pFontData->LoadFont(wsFontFamily, dwFontStyles, wCodePage))
    return nullptr;

  m_FontsArray.push_back(std::move(pFontData));
  return m_FontsArray.back()->GetFont();
}

void FWLTHEME_Release() {
  CFWL_FontManager::DestroyInstance();
}
