// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_widgettp.h"

#include <algorithm>
#include <utility>

#include "xfa/fde/cfde_textout.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/graphics/cfgas_gecolor.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace pdfium {

CFWL_WidgetTP::CFWL_WidgetTP() = default;

CFWL_WidgetTP::~CFWL_WidgetTP() = default;

void CFWL_WidgetTP::Trace(cppgc::Visitor* visitor) const {}

void CFWL_WidgetTP::DrawBackground(const CFWL_ThemeBackground& pParams) {}

void CFWL_WidgetTP::DrawText(const CFWL_ThemeText& pParams) {
  EnsureTTOInitialized(pParams.GetWidget()->GetThemeProvider());
  if (pParams.text_.IsEmpty()) {
    return;
  }

  CFGAS_GEGraphics* pGraphics = pParams.GetGraphics();
  text_out_->SetStyles(pParams.tto_styles_);
  text_out_->SetAlignment(pParams.tto_align_);

  CFX_Matrix matrix = pParams.matrix_;
  matrix.Concat(*pGraphics->GetMatrix());
  text_out_->SetMatrix(matrix);
  text_out_->DrawLogicText(pGraphics->GetRenderDevice(), pParams.text_,
                           pParams.part_rect_);
}

void CFWL_WidgetTP::InitializeArrowColorData() {
  if (color_data_) {
    return;
  }

  color_data_ = std::make_unique<CColorData>();
  color_data_->clrBorder[0] = ArgbEncode(255, 202, 216, 249);
  color_data_->clrBorder[1] = ArgbEncode(255, 171, 190, 233);
  color_data_->clrBorder[2] = ArgbEncode(255, 135, 147, 219);
  color_data_->clrBorder[3] = ArgbEncode(255, 172, 168, 153);
  color_data_->clrStart[0] = ArgbEncode(255, 225, 234, 254);
  color_data_->clrStart[1] = ArgbEncode(255, 253, 255, 255);
  color_data_->clrStart[2] = ArgbEncode(255, 110, 142, 241);
  color_data_->clrStart[3] = ArgbEncode(255, 254, 254, 251);
  color_data_->clrEnd[0] = ArgbEncode(255, 175, 204, 251);
  color_data_->clrEnd[1] = ArgbEncode(255, 185, 218, 251);
  color_data_->clrEnd[2] = ArgbEncode(255, 210, 222, 235);
  color_data_->clrEnd[3] = ArgbEncode(255, 243, 241, 236);
  color_data_->clrSign[0] = ArgbEncode(255, 77, 97, 133);
  color_data_->clrSign[1] = ArgbEncode(255, 77, 97, 133);
  color_data_->clrSign[2] = ArgbEncode(255, 77, 97, 133);
  color_data_->clrSign[3] = ArgbEncode(255, 128, 128, 128);
}

void CFWL_WidgetTP::EnsureTTOInitialized(IFWL_ThemeProvider* pProvider) {
  if (text_out_) {
    return;
  }

  text_out_ = std::make_unique<CFDE_TextOut>();
  text_out_->SetFont(pProvider->GetFWLFont());
  text_out_->SetFontSize(FWLTHEME_CAPACITY_FontSize);
  text_out_->SetTextColor(FWLTHEME_CAPACITY_TextColor);
}

void CFWL_WidgetTP::DrawBorder(CFGAS_GEGraphics* pGraphics,
                               const CFX_RectF& rect,
                               const CFX_Matrix& matrix) {
  if (!pGraphics) {
    return;
  }

  CFGAS_GEPath path;
  path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
  path.AddRectangle(rect.left + 1, rect.top + 1, rect.width - 2,
                    rect.height - 2);

  CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
  pGraphics->SetFillColor(CFGAS_GEColor(ArgbEncode(255, 0, 0, 0)));
  pGraphics->FillPath(path, CFX_FillRenderOptions::FillType::kEvenOdd, matrix);
}

void CFWL_WidgetTP::FillBackground(CFGAS_GEGraphics* pGraphics,
                                   const CFX_RectF& rect,
                                   const CFX_Matrix& matrix) {
  FillSolidRect(pGraphics, FWLTHEME_COLOR_Background, rect, matrix);
}

void CFWL_WidgetTP::FillSolidRect(CFGAS_GEGraphics* pGraphics,
                                  FX_ARGB fillColor,
                                  const CFX_RectF& rect,
                                  const CFX_Matrix& matrix) {
  if (!pGraphics) {
    return;
  }

  CFGAS_GEPath path;
  path.AddRectangle(rect.left, rect.top, rect.width, rect.height);

  CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
  pGraphics->SetFillColor(CFGAS_GEColor(fillColor));
  pGraphics->FillPath(path, CFX_FillRenderOptions::FillType::kWinding, matrix);
}

void CFWL_WidgetTP::DrawFocus(CFGAS_GEGraphics* pGraphics,
                              const CFX_RectF& rect,
                              const CFX_Matrix& matrix) {
  if (!pGraphics) {
    return;
  }

  CFGAS_GEPath path;
  path.AddRectangle(rect.left, rect.top, rect.width, rect.height);

  CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
  pGraphics->SetStrokeColor(CFGAS_GEColor(0xFF000000));
  pGraphics->SetLineDash({1, 1});
  pGraphics->StrokePath(path, matrix);
}

void CFWL_WidgetTP::DrawArrow(CFGAS_GEGraphics* pGraphics,
                              const CFX_RectF& rect,
                              FWLTHEME_DIRECTION eDict,
                              FX_ARGB argSign,
                              const CFX_Matrix& matrix) {
  bool bVert =
      (eDict == FWLTHEME_DIRECTION::kUp || eDict == FWLTHEME_DIRECTION::kDown);
  float fLeft = ((rect.width - (bVert ? 9 : 6)) / 2 + rect.left) + 0.5f;
  float fTop = ((rect.height - (bVert ? 6 : 9)) / 2 + rect.top) + 0.5f;
  CFGAS_GEPath path;
  switch (eDict) {
    case FWLTHEME_DIRECTION::kDown:
      path.MoveTo(CFX_PointF(fLeft, fTop + 1));
      path.LineTo(CFX_PointF(fLeft + 4, fTop + 5));
      path.LineTo(CFX_PointF(fLeft + 8, fTop + 1));
      path.LineTo(CFX_PointF(fLeft + 7, fTop));
      path.LineTo(CFX_PointF(fLeft + 4, fTop + 3));
      path.LineTo(CFX_PointF(fLeft + 1, fTop));
      break;
    case FWLTHEME_DIRECTION::kUp:
      path.MoveTo(CFX_PointF(fLeft, fTop + 4));
      path.LineTo(CFX_PointF(fLeft + 4, fTop));
      path.LineTo(CFX_PointF(fLeft + 8, fTop + 4));
      path.LineTo(CFX_PointF(fLeft + 7, fTop + 5));
      path.LineTo(CFX_PointF(fLeft + 4, fTop + 2));
      path.LineTo(CFX_PointF(fLeft + 1, fTop + 5));
      break;
    case FWLTHEME_DIRECTION::kRight:
      path.MoveTo(CFX_PointF(fLeft + 1, fTop));
      path.LineTo(CFX_PointF(fLeft + 5, fTop + 4));
      path.LineTo(CFX_PointF(fLeft + 1, fTop + 8));
      path.LineTo(CFX_PointF(fLeft, fTop + 7));
      path.LineTo(CFX_PointF(fLeft + 3, fTop + 4));
      path.LineTo(CFX_PointF(fLeft, fTop + 1));
      break;
    case FWLTHEME_DIRECTION::kLeft:
      path.MoveTo(CFX_PointF(fLeft, fTop + 4));
      path.LineTo(CFX_PointF(fLeft + 4, fTop));
      path.LineTo(CFX_PointF(fLeft + 5, fTop + 1));
      path.LineTo(CFX_PointF(fLeft + 2, fTop + 4));
      path.LineTo(CFX_PointF(fLeft + 5, fTop + 7));
      path.LineTo(CFX_PointF(fLeft + 4, fTop + 8));
      break;
  }
  pGraphics->SetFillColor(CFGAS_GEColor(argSign));
  pGraphics->FillPath(path, CFX_FillRenderOptions::FillType::kWinding, matrix);
}

void CFWL_WidgetTP::DrawBtn(CFGAS_GEGraphics* pGraphics,
                            const CFX_RectF& rect,
                            FWLTHEME_STATE eState,
                            const CFX_Matrix& matrix) {
  InitializeArrowColorData();
  FillSolidRect(pGraphics, color_data_->clrEnd[static_cast<size_t>(eState) - 1],
                rect, matrix);

  CFGAS_GEPath path;
  path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
  pGraphics->SetStrokeColor(
      CFGAS_GEColor(color_data_->clrBorder[static_cast<size_t>(eState) - 1]));
  pGraphics->StrokePath(path, matrix);
}

void CFWL_WidgetTP::DrawArrowBtn(CFGAS_GEGraphics* pGraphics,
                                 const CFX_RectF& rect,
                                 FWLTHEME_DIRECTION eDict,
                                 FWLTHEME_STATE eState,
                                 const CFX_Matrix& matrix) {
  DrawBtn(pGraphics, rect, eState, matrix);
  InitializeArrowColorData();
  DrawArrow(pGraphics, rect, eDict,
            color_data_->clrSign[static_cast<size_t>(eState) - 1], matrix);
}

}  // namespace pdfium
