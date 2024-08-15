// Copyright 2014 The PDFium Authors
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

namespace pdfium {

CFWL_ScrollBarTP::CFWL_ScrollBarTP()
    : m_pThemeData(std::make_unique<SBThemeData>()) {
  SetThemeData();
}

CFWL_ScrollBarTP::~CFWL_ScrollBarTP() = default;

void CFWL_ScrollBarTP::DrawBackground(const CFWL_ThemeBackground& pParams) {
  CFWL_Widget* pWidget = pParams.GetWidget();
  CFGAS_GEGraphics* pGraphics = pParams.GetGraphics();
  bool bVert = !!pWidget->GetStyleExts();
  switch (pParams.GetPart()) {
    case CFWL_ThemePart::Part::kForeArrow: {
      DrawMaxMinBtn(pGraphics, pParams.m_PartRect,
                    bVert ? FWLTHEME_DIRECTION::kUp : FWLTHEME_DIRECTION::kLeft,
                    pParams.GetThemeState(), pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kBackArrow: {
      DrawMaxMinBtn(
          pGraphics, pParams.m_PartRect,
          bVert ? FWLTHEME_DIRECTION::kDown : FWLTHEME_DIRECTION::kRight,
          pParams.GetThemeState(), pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kThumb: {
      DrawThumbBtn(pGraphics, pParams.m_PartRect, bVert,
                   pParams.GetThemeState(), pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kLowerTrack: {
      DrawTrack(pGraphics, pParams.m_PartRect, bVert, pParams.GetThemeState(),
                true, pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kUpperTrack: {
      DrawTrack(pGraphics, pParams.m_PartRect, bVert, pParams.GetThemeState(),
                false, pParams.m_matrix);
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
  if (eState < FWLTHEME_STATE::kNormal || eState > FWLTHEME_STATE::kDisable)
    return;

  CFX_RectF rect = input_rect;
  if (bVert)
    rect.Deflate(1, 0);
  else
    rect.Deflate(0, 1);

  if (rect.IsEmpty(0.1f))
    return;

  FillSolidRect(pGraphics,
                m_pThemeData->clrBtnBK[static_cast<size_t>(eState) - 1], rect,
                matrix);

  CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
  CFGAS_GEPath path;
  path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
  pGraphics->SetStrokeColor(CFGAS_GEColor(
      m_pThemeData->clrBtnBorder[static_cast<size_t>(eState) - 1]));
  pGraphics->StrokePath(path, matrix);
}

void CFWL_ScrollBarTP::DrawTrack(CFGAS_GEGraphics* pGraphics,
                                 const CFX_RectF& rect,
                                 bool bVert,
                                 FWLTHEME_STATE eState,
                                 bool bLowerTrack,
                                 const CFX_Matrix& matrix) {
  if (eState < FWLTHEME_STATE::kNormal || eState > FWLTHEME_STATE::kDisable)
    return;

  {
    CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
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
    pGraphics->FillPath(path, CFX_FillRenderOptions::FillType::kWinding,
                        matrix);
    path.Clear();
    path.AddRectangle(rect.left + 1, rect.top, rect.width - 2, rect.height);
  }
  FillSolidRect(pGraphics, m_pThemeData->clrTrackBKEnd, rect, matrix);
}

void CFWL_ScrollBarTP::DrawMaxMinBtn(CFGAS_GEGraphics* pGraphics,
                                     const CFX_RectF& rect,
                                     FWLTHEME_DIRECTION eDict,
                                     FWLTHEME_STATE eState,
                                     const CFX_Matrix& matrix) {
  DrawTrack(
      pGraphics, rect,
      eDict == FWLTHEME_DIRECTION::kUp || eDict == FWLTHEME_DIRECTION::kDown,
      eState, true, matrix);
  CFX_RectF rtArrowBtn = rect;
  rtArrowBtn.Deflate(1, 1, 1, 1);
  DrawArrowBtn(pGraphics, rtArrowBtn, eDict, eState, matrix);
}

void CFWL_ScrollBarTP::SetThemeData() {
  m_pThemeData->clrTrackBKStart = ArgbEncode(0xff, 243, 241, 236);
  m_pThemeData->clrTrackBKEnd = ArgbEncode(0xff, 254, 254, 251);
  m_pThemeData->clrBtnBK[0] = ArgbEncode(0xff, 182, 205, 251);
  m_pThemeData->clrBtnBK[1] = ArgbEncode(0xff, 204, 225, 255);
  m_pThemeData->clrBtnBK[2] = ArgbEncode(0xff, 146, 179, 249);
  m_pThemeData->clrBtnBK[3] = ArgbEncode(0xff, 141, 157, 115);
  m_pThemeData->clrBtnBorder[0] = ArgbEncode(0xff, 148, 176, 221);
  m_pThemeData->clrBtnBorder[1] = ArgbEncode(0xff, 218, 230, 254);
  m_pThemeData->clrBtnBorder[2] = ArgbEncode(0xff, 124, 159, 211);
  m_pThemeData->clrBtnBorder[3] = ArgbEncode(0xff, 236, 233, 216);
}

}  // namespace pdfium
