// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_scrollbartp.h"

#include "xfa/fgas/graphics/cfgas_gecolor.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fwl/cfwl_scrollbar.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/ifwl_themeprovider.h"

CFWL_ScrollBarTP::CFWL_ScrollBarTP() : m_pThemeData(new SBThemeData) {
  SetThemeData();
}

CFWL_ScrollBarTP::~CFWL_ScrollBarTP() = default;

void CFWL_ScrollBarTP::DrawBackground(const CFWL_ThemeBackground& pParams) {
  CFWL_Widget* pWidget = pParams.GetWidget();
  FWLTHEME_STATE eState = FWLTHEME_STATE_Normal;
  if (pParams.m_dwStates & CFWL_PartState_Hovered)
    eState = FWLTHEME_STATE_Hover;
  else if (pParams.m_dwStates & CFWL_PartState_Pressed)
    eState = FWLTHEME_STATE_Pressed;
  else if (pParams.m_dwStates & CFWL_PartState_Disabled)
    eState = FWLTHEME_STATE_Disable;

  CFGAS_GEGraphics* pGraphics = pParams.GetGraphics();
  bool bVert = !!pWidget->GetStyleExts();
  switch (pParams.m_iPart) {
    case CFWL_ThemePart::Part::kForeArrow: {
      DrawMaxMinBtn(pGraphics, pParams.m_PartRect,
                    bVert ? FWLTHEME_DIRECTION_Up : FWLTHEME_DIRECTION_Left,
                    eState, pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kBackArrow: {
      DrawMaxMinBtn(pGraphics, pParams.m_PartRect,
                    bVert ? FWLTHEME_DIRECTION_Down : FWLTHEME_DIRECTION_Right,
                    eState, pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kThumb: {
      DrawThumbBtn(pGraphics, pParams.m_PartRect, bVert, eState,
                   pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kLowerTrack: {
      DrawTrack(pGraphics, pParams.m_PartRect, bVert, eState, true,
                pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kUpperTrack: {
      DrawTrack(pGraphics, pParams.m_PartRect, bVert, eState, false,
                pParams.m_matrix);
      break;
    }
    default:
      break;
  }
}

void CFWL_ScrollBarTP::DrawThumbBtn(CFGAS_GEGraphics* pGraphics,
                                    const CFX_RectF& input_rect,
                                    bool bVert,
                                    FWLTHEME_STATE eState,
                                    const CFX_Matrix& matrix) {
  if (eState < FWLTHEME_STATE_Normal || eState > FWLTHEME_STATE_Disable)
    return;

  CFX_RectF rect = input_rect;
  if (bVert)
    rect.Deflate(1, 0);
  else
    rect.Deflate(0, 1);

  if (rect.IsEmpty(0.1f))
    return;

  FillSolidRect(pGraphics, m_pThemeData->clrBtnBK[eState - 1][1], rect, matrix);

  pGraphics->SaveGraphState();

  CFGAS_GEPath path;
  path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
  pGraphics->SetStrokeColor(
      CFGAS_GEColor(m_pThemeData->clrBtnBorder[eState - 1]));
  pGraphics->StrokePath(path, matrix);
  pGraphics->RestoreGraphState();
}

void CFWL_ScrollBarTP::DrawTrack(CFGAS_GEGraphics* pGraphics,
                                 const CFX_RectF& rect,
                                 bool bVert,
                                 FWLTHEME_STATE eState,
                                 bool bLowerTrack,
                                 const CFX_Matrix& matrix) {
  if (eState < FWLTHEME_STATE_Normal || eState > FWLTHEME_STATE_Disable)
    return;

  pGraphics->SaveGraphState();
  CFGAS_GEPath path;
  float fRight = rect.right();
  float fBottom = rect.bottom();
  if (bVert) {
    path.AddRectangle(rect.left, rect.top, 1, rect.height);
    path.AddRectangle(fRight - 1, rect.top, 1, rect.height);
  } else {
    path.AddRectangle(rect.left, rect.top, rect.width, 1);
    path.AddRectangle(rect.left, fBottom - 1, rect.width, 1);
  }
  pGraphics->SetFillColor(CFGAS_GEColor(ArgbEncode(255, 238, 237, 229)));
  pGraphics->FillPath(path, CFX_FillRenderOptions::FillType::kWinding, matrix);
  path.Clear();
  path.AddRectangle(rect.left + 1, rect.top, rect.width - 2, rect.height);
  pGraphics->RestoreGraphState();
  FillSolidRect(pGraphics, m_pThemeData->clrTrackBKEnd, rect, matrix);
}

void CFWL_ScrollBarTP::DrawMaxMinBtn(CFGAS_GEGraphics* pGraphics,
                                     const CFX_RectF& rect,
                                     FWLTHEME_DIRECTION eDict,
                                     FWLTHEME_STATE eState,
                                     const CFX_Matrix& matrix) {
  DrawTrack(pGraphics, rect,
            eDict == FWLTHEME_DIRECTION_Up || eDict == FWLTHEME_DIRECTION_Down,
            eState, true, matrix);
  CFX_RectF rtArrowBtn = rect;
  rtArrowBtn.Deflate(1, 1, 1, 1);
  DrawArrowBtn(pGraphics, rtArrowBtn, eDict, eState, matrix);
}

void CFWL_ScrollBarTP::SetThemeData() {
  m_pThemeData->clrTrackBKStart = ArgbEncode(0xff, 243, 241, 236);
  m_pThemeData->clrTrackBKEnd = ArgbEncode(0xff, 254, 254, 251);
  m_pThemeData->clrBtnBK[0][0] = ArgbEncode(0xff, 197, 213, 252);
  m_pThemeData->clrBtnBK[0][1] = ArgbEncode(0xff, 182, 205, 251);
  m_pThemeData->clrBtnBK[1][0] = ArgbEncode(0xff, 216, 232, 255);
  m_pThemeData->clrBtnBK[1][1] = ArgbEncode(0xff, 204, 225, 255);
  m_pThemeData->clrBtnBK[2][0] = ArgbEncode(0xff, 167, 190, 245);
  m_pThemeData->clrBtnBK[2][1] = ArgbEncode(0xff, 146, 179, 249);
  m_pThemeData->clrBtnBK[3][0] = ArgbEncode(0xff, 164, 180, 139);
  m_pThemeData->clrBtnBK[3][1] = ArgbEncode(0xff, 141, 157, 115);
  m_pThemeData->clrBtnBorder[0] = ArgbEncode(0xff, 148, 176, 221);
  m_pThemeData->clrBtnBorder[1] = ArgbEncode(0xff, 218, 230, 254);
  m_pThemeData->clrBtnBorder[2] = ArgbEncode(0xff, 124, 159, 211);
  m_pThemeData->clrBtnBorder[3] = ArgbEncode(0xff, 236, 233, 216);
}
