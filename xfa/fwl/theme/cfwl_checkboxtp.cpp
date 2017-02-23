// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_checkboxtp.h"

#include "core/fxge/cfx_pathdata.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fwl/cfwl_checkbox.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fxgraphics/cfx_color.h"
#include "xfa/fxgraphics/cfx_path.h"

namespace {

const int kSignPath = 100;

}  // namespace

#define CHECKBOX_COLOR_BOXLT1 (ArgbEncode(255, 172, 168, 153))
#define CHECKBOX_COLOR_BOXLT2 (ArgbEncode(255, 113, 111, 100))
#define CHECKBOX_COLOR_BOXRB1 (ArgbEncode(255, 241, 239, 226))
#define CHECKBOX_COLOR_BOXRB2 (ArgbEncode(255, 255, 255, 255))

CFWL_CheckBoxTP::CFWL_CheckBoxTP() : m_pThemeData(new CKBThemeData) {
  SetThemeData();
}

CFWL_CheckBoxTP::~CFWL_CheckBoxTP() {
  if (m_pCheckPath)
    m_pCheckPath->Clear();
}

void CFWL_CheckBoxTP::Initialize() {
  CFWL_WidgetTP::Initialize();
  InitTTO();
}

void CFWL_CheckBoxTP::Finalize() {
  FinalizeTTO();
  CFWL_WidgetTP::Finalize();
}

void CFWL_CheckBoxTP::DrawText(CFWL_ThemeText* pParams) {
  if (!m_pTextOut)
    return;

  m_pTextOut->SetTextColor(pParams->m_dwStates & CFWL_PartState_Disabled
                               ? FWLTHEME_CAPACITY_TextDisColor
                               : FWLTHEME_CAPACITY_TextColor);
  CFWL_WidgetTP::DrawText(pParams);
}

void CFWL_CheckBoxTP::DrawSignCheck(CFX_Graphics* pGraphics,
                                    const CFX_RectF* pRtSign,
                                    FX_ARGB argbFill,
                                    CFX_Matrix* pMatrix) {
  if (!m_pCheckPath)
    InitCheckPath(pRtSign->width);

  CFX_Matrix mt;
  mt.SetIdentity();
  mt.Translate(pRtSign->left, pRtSign->top);
  mt.Concat(*pMatrix);
  CFX_Color crFill(argbFill);
  pGraphics->SaveGraphState();
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(m_pCheckPath.get(), FXFILL_WINDING, &mt);
  pGraphics->RestoreGraphState();
}

void CFWL_CheckBoxTP::DrawSignCircle(CFX_Graphics* pGraphics,
                                     const CFX_RectF* pRtSign,
                                     FX_ARGB argbFill,
                                     CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.AddEllipse(*pRtSign);
  CFX_Color crFill(argbFill);
  pGraphics->SaveGraphState();
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  pGraphics->RestoreGraphState();
}

void CFWL_CheckBoxTP::DrawSignCross(CFX_Graphics* pGraphics,
                                    const CFX_RectF* pRtSign,
                                    FX_ARGB argbFill,
                                    CFX_Matrix* pMatrix) {
  CFX_Path path;
  FX_FLOAT fRight = pRtSign->right();
  FX_FLOAT fBottom = pRtSign->bottom();
  path.AddLine(pRtSign->TopLeft(), CFX_PointF(fRight, fBottom));
  path.AddLine(CFX_PointF(pRtSign->left, fBottom),
               CFX_PointF(fRight, pRtSign->top));

  CFX_Color crFill(argbFill);
  pGraphics->SaveGraphState();
  pGraphics->SetStrokeColor(&crFill);
  pGraphics->SetLineWidth(1.0f);
  pGraphics->StrokePath(&path, pMatrix);
  pGraphics->RestoreGraphState();
}

void CFWL_CheckBoxTP::DrawSignDiamond(CFX_Graphics* pGraphics,
                                      const CFX_RectF* pRtSign,
                                      FX_ARGB argbFill,
                                      CFX_Matrix* pMatrix) {
  CFX_Path path;
  FX_FLOAT fWidth = pRtSign->width;
  FX_FLOAT fHeight = pRtSign->height;
  FX_FLOAT fBottom = pRtSign->bottom();
  path.MoveTo(CFX_PointF(pRtSign->left + fWidth / 2, pRtSign->top));
  path.LineTo(CFX_PointF(pRtSign->left, pRtSign->top + fHeight / 2));
  path.LineTo(CFX_PointF(pRtSign->left + fWidth / 2, fBottom));
  path.LineTo(CFX_PointF(pRtSign->right(), pRtSign->top + fHeight / 2));
  path.LineTo(CFX_PointF(pRtSign->left + fWidth / 2, pRtSign->top));

  CFX_Color crFill(argbFill);
  pGraphics->SaveGraphState();
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  pGraphics->RestoreGraphState();
}

void CFWL_CheckBoxTP::DrawSignSquare(CFX_Graphics* pGraphics,
                                     const CFX_RectF* pRtSign,
                                     FX_ARGB argbFill,
                                     CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.AddRectangle(pRtSign->left, pRtSign->top, pRtSign->width,
                    pRtSign->height);
  CFX_Color crFill(argbFill);
  pGraphics->SaveGraphState();
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  pGraphics->RestoreGraphState();
}

void CFWL_CheckBoxTP::DrawSignStar(CFX_Graphics* pGraphics,
                                   const CFX_RectF* pRtSign,
                                   FX_ARGB argbFill,
                                   CFX_Matrix* pMatrix) {
  CFX_Path path;
  FX_FLOAT fBottom = pRtSign->bottom();
  FX_FLOAT fRadius =
      (pRtSign->top - fBottom) / (1 + static_cast<FX_FLOAT>(cos(FX_PI / 5.0f)));
  CFX_PointF ptCenter((pRtSign->left + pRtSign->right()) / 2.0f,
                      (pRtSign->top + fBottom) / 2.0f);

  CFX_PointF points[5];
  FX_FLOAT fAngel = FX_PI / 10.0f;
  for (int32_t i = 0; i < 5; i++) {
    points[i] =
        ptCenter + CFX_PointF(fRadius * static_cast<FX_FLOAT>(cos(fAngel)),
                              fRadius * static_cast<FX_FLOAT>(sin(fAngel)));
    fAngel += FX_PI * 2 / 5.0f;
  }

  path.MoveTo(points[0]);
  int32_t nNext = 0;
  for (int32_t j = 0; j < 5; j++) {
    nNext += 2;
    if (nNext >= 5)
      nNext -= 5;

    path.LineTo(points[nNext]);
  }

  CFX_Color crFill(argbFill);
  pGraphics->SaveGraphState();
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  pGraphics->RestoreGraphState();
}

void CFWL_CheckBoxTP::SetThemeData() {
  uint32_t* pData = (uint32_t*)&m_pThemeData->clrBoxBk;

  *pData++ = 0;
  *pData++ = 0;
  *pData++ = ArgbEncode(255, 220, 220, 215),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 240, 207),
  *pData++ = ArgbEncode(255, 248, 179, 48),
  *pData++ = ArgbEncode(255, 176, 176, 167),
  *pData++ = ArgbEncode(255, 241, 239, 239),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 220, 220, 215),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 240, 207),
  *pData++ = ArgbEncode(255, 248, 179, 48),
  *pData++ = ArgbEncode(255, 176, 176, 167),
  *pData++ = ArgbEncode(255, 241, 239, 239),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 220, 220, 215),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 240, 207),
  *pData++ = ArgbEncode(255, 248, 179, 48),
  *pData++ = ArgbEncode(255, 176, 176, 167),
  *pData++ = ArgbEncode(255, 241, 239, 239),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 255, 255);
  m_pThemeData->clrSignBorderNormal = ArgbEncode(255, 28, 81, 128);
  m_pThemeData->clrSignBorderDisable = ArgbEncode(255, 202, 200, 187);
  m_pThemeData->clrSignCheck = ArgbEncode(255, 28, 81, 128);
  m_pThemeData->clrSignNeutral = ArgbEncode(255, 28, 134, 26);
  m_pThemeData->clrSignNeutralNormal = ArgbEncode(255, 114, 192, 113);
  m_pThemeData->clrSignNeutralHover = ArgbEncode(255, 33, 161, 33);
  m_pThemeData->clrSignNeutralPressed = ArgbEncode(255, 28, 134, 26);
}

void CFWL_CheckBoxTP::InitCheckPath(FX_FLOAT fCheckLen) {
  if (!m_pCheckPath) {
    m_pCheckPath = pdfium::MakeUnique<CFX_Path>();

    FX_FLOAT fWidth = kSignPath;
    FX_FLOAT fHeight = -kSignPath;
    FX_FLOAT fBottom = kSignPath;
    CFX_PointF pt1(fWidth / 15.0f, fBottom + fHeight * 2 / 5.0f);
    CFX_PointF pt2(fWidth / 4.5f, fBottom + fHeight / 16.0f);
    CFX_PointF pt3(fWidth / 3.0f, fBottom);
    CFX_PointF pt4(fWidth * 14 / 15.0f, fBottom + fHeight * 15 / 16.0f);
    CFX_PointF pt5(fWidth / 3.6f, fBottom + fHeight / 3.5f);
    CFX_PointF pt12(fWidth / 7.0f, fBottom + fHeight * 2 / 7.0f);
    CFX_PointF pt21(fWidth / 5.0f, fBottom + fHeight / 5.0f);
    CFX_PointF pt23(fWidth / 4.4f, fBottom + fHeight * 0 / 16.0f);
    CFX_PointF pt32(fWidth / 4.0f, fBottom);
    CFX_PointF pt34(fWidth * (1 / 7.0f + 7 / 15.0f),
                    fBottom + fHeight * 4 / 5.0f);
    CFX_PointF pt43(fWidth * (1 / 7.0f + 7 / 15.0f),
                    fBottom + fHeight * 4 / 5.0f);
    CFX_PointF pt45(fWidth * 7 / 15.0f, fBottom + fHeight * 8 / 7.0f);
    CFX_PointF pt54(fWidth / 3.4f, fBottom + fHeight / 3.5f);
    CFX_PointF pt51(fWidth / 3.6f, fBottom + fHeight / 4.0f);
    CFX_PointF pt15(fWidth / 3.5f, fBottom + fHeight * 3.5f / 5.0f);
    m_pCheckPath->MoveTo(pt1);

    CFX_PointF p1 = CFX_PointF(pt12.x - pt1.x, pt12.y - pt1.y) * FX_BEZIER;
    CFX_PointF p2 = CFX_PointF(pt21.x - pt2.x, pt21.y - pt2.y) * FX_BEZIER;
    m_pCheckPath->BezierTo(pt1 + p1, pt2 + p2, pt2);

    p1 = CFX_PointF(pt23.x - pt2.x, pt23.y - pt2.y) * FX_BEZIER;
    p2 = CFX_PointF(pt32.x - pt3.x, pt32.y - pt3.y) * FX_BEZIER;
    m_pCheckPath->BezierTo(pt2 + p1, pt3 + p2, pt3);

    p1 = CFX_PointF(pt34.x - pt3.x, pt34.y - pt3.y) * FX_BEZIER;
    p2 = CFX_PointF(pt43.x - pt4.x, pt43.y - pt4.y) * FX_BEZIER;
    m_pCheckPath->BezierTo(pt3 + p1, pt4 + p2, pt4);

    p1 = CFX_PointF(pt45.x - pt4.x, pt45.y - pt4.y) * FX_BEZIER;
    p2 = CFX_PointF(pt54.x - pt5.x, pt54.y - pt5.y) * FX_BEZIER;
    m_pCheckPath->BezierTo(pt4 + p1, pt5 + p2, pt5);

    p1 = CFX_PointF(pt51.x - pt5.x, pt51.y - pt5.y) * FX_BEZIER;
    p2 = CFX_PointF(pt15.x - pt1.x, pt15.y - pt1.y) * FX_BEZIER;
    m_pCheckPath->BezierTo(pt5 + p1, pt1 + p2, pt1);

    FX_FLOAT fScale = fCheckLen / kSignPath;
    CFX_Matrix mt(1, 0, 0, 1, 0, 0);
    mt.Scale(fScale, fScale);

    m_pCheckPath->TransformBy(mt);
  }
}

void CFWL_CheckBoxTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (pParams->m_iPart != CFWL_Part::CheckBox)
    return;

  if ((pParams->m_dwStates & CFWL_PartState_Checked) ||
      (pParams->m_dwStates & CFWL_PartState_Neutral)) {
    DrawCheckSign(pParams->m_pWidget, pParams->m_pGraphics, pParams->m_rtPart,
                  pParams->m_dwStates, &pParams->m_matrix);
  }
}

void CFWL_CheckBoxTP::DrawCheckSign(CFWL_Widget* pWidget,
                                    CFX_Graphics* pGraphics,
                                    const CFX_RectF& pRtBox,
                                    int32_t iState,
                                    CFX_Matrix* pMatrix) {
  CFX_RectF rtSign(pRtBox);
  uint32_t dwColor = iState & CFWL_PartState_Neutral ? 0xFFA9A9A9 : 0xFF000000;

  uint32_t dwStyle = pWidget->GetStylesEx();
  rtSign.Deflate(rtSign.width / 4, rtSign.height / 4);
  switch (dwStyle & FWL_STYLEEXT_CKB_SignShapeMask) {
    case FWL_STYLEEXT_CKB_SignShapeCheck:
      DrawSignCheck(pGraphics, &rtSign, dwColor, pMatrix);
      break;
    case FWL_STYLEEXT_CKB_SignShapeCircle:
      DrawSignCircle(pGraphics, &rtSign, dwColor, pMatrix);
      break;
    case FWL_STYLEEXT_CKB_SignShapeCross:
      DrawSignCross(pGraphics, &rtSign, dwColor, pMatrix);
      break;
    case FWL_STYLEEXT_CKB_SignShapeDiamond:
      DrawSignDiamond(pGraphics, &rtSign, dwColor, pMatrix);
      break;
    case FWL_STYLEEXT_CKB_SignShapeSquare:
      DrawSignSquare(pGraphics, &rtSign, dwColor, pMatrix);
      break;
    case FWL_STYLEEXT_CKB_SignShapeStar:
      DrawSignStar(pGraphics, &rtSign, dwColor, pMatrix);
      break;
    default:
      break;
  }
}
