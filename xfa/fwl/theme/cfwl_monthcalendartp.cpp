// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_monthcalendartp.h"

#include "xfa/fde/cfde_textout.h"
#include "xfa/fgas/graphics/cfgas_gecolor.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fwl/cfwl_monthcalendar.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themetext.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace pdfium {

namespace {

constexpr FX_ARGB kCaptionColor = ArgbEncode(0xff, 0, 153, 255);
constexpr FX_ARGB kSeparatorColor = ArgbEncode(0xff, 141, 161, 239);
constexpr FX_ARGB kDatesHoverBackgroundColor = ArgbEncode(0xff, 193, 211, 251);
constexpr FX_ARGB kDatesSelectedBackgroundColor =
    ArgbEncode(0xff, 173, 188, 239);
constexpr FX_ARGB kDatesCircleColor = ArgbEncode(0xff, 103, 144, 209);
constexpr FX_ARGB kBackgroundColor = ArgbEncode(0xff, 255, 255, 255);

}  // namespace

CFWL_MonthCalendarTP::CFWL_MonthCalendarTP() = default;

CFWL_MonthCalendarTP::~CFWL_MonthCalendarTP() = default;

void CFWL_MonthCalendarTP::DrawBackground(const CFWL_ThemeBackground& pParams) {
  switch (pParams.GetPart()) {
    case CFWL_ThemePart::Part::kBorder: {
      DrawBorder(pParams.GetGraphics(), pParams.part_rect_, pParams.matrix_);
      break;
    }
    case CFWL_ThemePart::Part::kBackground: {
      DrawTotalBK(pParams, pParams.matrix_);
      break;
    }
    case CFWL_ThemePart::Part::kHeader: {
      DrawHeadBk(pParams, pParams.matrix_);
      break;
    }
    case CFWL_ThemePart::Part::kLBtn: {
      DrawArrowBtn(pParams.GetGraphics(), pParams.part_rect_,
                   FWLTHEME_DIRECTION::kLeft, pParams.GetThemeState(),
                   pParams.matrix_);
      break;
    }
    case CFWL_ThemePart::Part::kRBtn: {
      DrawArrowBtn(pParams.GetGraphics(), pParams.part_rect_,
                   FWLTHEME_DIRECTION::kRight, pParams.GetThemeState(),
                   pParams.matrix_);
      break;
    }
    case CFWL_ThemePart::Part::kHSeparator: {
      DrawHSeparator(pParams, pParams.matrix_);
      break;
    }
    case CFWL_ThemePart::Part::kDatesIn: {
      DrawDatesInBK(pParams, pParams.matrix_);
      break;
    }
    case CFWL_ThemePart::Part::kTodayCircle: {
      DrawTodayCircle(pParams, pParams.matrix_);
      break;
    }
    case CFWL_ThemePart::Part::kDateInCircle: {
      DrawDatesInCircle(pParams, pParams.matrix_);
      break;
    }
    case CFWL_ThemePart::Part::kWeekNumSep: {
      DrawWeekNumSep(pParams, pParams.matrix_);
      break;
    }
    default:
      break;
  }
}

void CFWL_MonthCalendarTP::DrawText(const CFWL_ThemeText& pParams) {
  EnsureTTOInitialized(pParams.GetWidget()->GetThemeProvider());
  if (pParams.GetPart() == CFWL_ThemePart::Part::kDatesIn &&
      !(pParams.states_ & CFWL_PartState::kFlagged) &&
      (pParams.states_ & Mask<CFWL_PartState>{CFWL_PartState::kHovered,
                                              CFWL_PartState::kSelected})) {
    text_out_->SetTextColor(0xFFFFFFFF);
  } else if (pParams.GetPart() == CFWL_ThemePart::Part::kCaption) {
    text_out_->SetTextColor(kCaptionColor);
  } else {
    text_out_->SetTextColor(0xFF000000);
  }
  CFWL_WidgetTP::DrawText(pParams);
}

void CFWL_MonthCalendarTP::DrawTotalBK(const CFWL_ThemeBackground& pParams,
                                       const CFX_Matrix& matrix) {
  CFGAS_GEPath path;
  CFX_RectF rtTotal(pParams.part_rect_);
  path.AddRectangle(rtTotal.left, rtTotal.top, rtTotal.width, rtTotal.height);

  CFGAS_GEGraphics::StateRestorer restorer(pParams.GetGraphics());
  pParams.GetGraphics()->SetFillColor(CFGAS_GEColor(kBackgroundColor));
  pParams.GetGraphics()->FillPath(
      path, CFX_FillRenderOptions::FillType::kWinding, matrix);
}

void CFWL_MonthCalendarTP::DrawHeadBk(const CFWL_ThemeBackground& pParams,
                                      const CFX_Matrix& matrix) {
  CFGAS_GEPath path;
  CFX_RectF rtHead = pParams.part_rect_;
  path.AddRectangle(rtHead.left, rtHead.top, rtHead.width, rtHead.height);

  CFGAS_GEGraphics::StateRestorer restorer(pParams.GetGraphics());
  pParams.GetGraphics()->SetFillColor(CFGAS_GEColor(kBackgroundColor));
  pParams.GetGraphics()->FillPath(
      path, CFX_FillRenderOptions::FillType::kWinding, matrix);
}

void CFWL_MonthCalendarTP::DrawLButton(const CFWL_ThemeBackground& pParams,
                                       const CFX_Matrix& matrix) {
  CFGAS_GEPath path;
  CFX_RectF rtLBtn = pParams.part_rect_;
  path.AddRectangle(rtLBtn.left, rtLBtn.top, rtLBtn.width, rtLBtn.height);

  CFGAS_GEGraphics::StateRestorer restorer(pParams.GetGraphics());
  pParams.GetGraphics()->SetStrokeColor(
      CFGAS_GEColor(ArgbEncode(0xff, 205, 219, 243)));
  pParams.GetGraphics()->StrokePath(path, matrix);
  if (pParams.states_ & CFWL_PartState::kPressed) {
    pParams.GetGraphics()->SetFillColor(
        CFGAS_GEColor(ArgbEncode(0xff, 174, 198, 242)));
    pParams.GetGraphics()->FillPath(
        path, CFX_FillRenderOptions::FillType::kWinding, matrix);
  } else {
    pParams.GetGraphics()->SetFillColor(
        CFGAS_GEColor(ArgbEncode(0xff, 227, 235, 249)));
    pParams.GetGraphics()->FillPath(
        path, CFX_FillRenderOptions::FillType::kWinding, matrix);
  }

  path.Clear();
  path.MoveTo(CFX_PointF(rtLBtn.left + rtLBtn.Width() / 3 * 2,
                         rtLBtn.top + rtLBtn.height / 4));
  path.LineTo(CFX_PointF(rtLBtn.left + rtLBtn.Width() / 3,
                         rtLBtn.top + rtLBtn.height / 2));
  path.LineTo(CFX_PointF(rtLBtn.left + rtLBtn.Width() / 3 * 2,
                         rtLBtn.bottom() - rtLBtn.height / 4));

  pParams.GetGraphics()->SetStrokeColor(
      CFGAS_GEColor(ArgbEncode(0xff, 50, 104, 205)));
  pParams.GetGraphics()->StrokePath(path, matrix);
}

void CFWL_MonthCalendarTP::DrawRButton(const CFWL_ThemeBackground& pParams,
                                       const CFX_Matrix& matrix) {
  CFGAS_GEPath path;
  CFX_RectF rtRBtn = pParams.part_rect_;
  path.AddRectangle(rtRBtn.left, rtRBtn.top, rtRBtn.width, rtRBtn.height);

  CFGAS_GEGraphics::StateRestorer restorer(pParams.GetGraphics());
  pParams.GetGraphics()->SetStrokeColor(
      CFGAS_GEColor(ArgbEncode(0xff, 205, 219, 243)));
  pParams.GetGraphics()->StrokePath(path, matrix);
  if (pParams.states_ & CFWL_PartState::kPressed) {
    pParams.GetGraphics()->SetFillColor(
        CFGAS_GEColor(ArgbEncode(0xff, 174, 198, 242)));
    pParams.GetGraphics()->FillPath(
        path, CFX_FillRenderOptions::FillType::kWinding, matrix);
  } else {
    pParams.GetGraphics()->SetFillColor(
        CFGAS_GEColor(ArgbEncode(0xff, 227, 235, 249)));
    pParams.GetGraphics()->FillPath(
        path, CFX_FillRenderOptions::FillType::kWinding, matrix);
  }

  path.Clear();
  path.MoveTo(CFX_PointF(rtRBtn.left + rtRBtn.Width() / 3,
                         rtRBtn.top + rtRBtn.height / 4));
  path.LineTo(CFX_PointF(rtRBtn.left + rtRBtn.Width() / 3 * 2,
                         rtRBtn.top + rtRBtn.height / 2));
  path.LineTo(CFX_PointF(rtRBtn.left + rtRBtn.Width() / 3,
                         rtRBtn.bottom() - rtRBtn.height / 4));

  pParams.GetGraphics()->SetStrokeColor(
      CFGAS_GEColor(ArgbEncode(0xff, 50, 104, 205)));
  pParams.GetGraphics()->StrokePath(path, matrix);
}

void CFWL_MonthCalendarTP::DrawHSeparator(const CFWL_ThemeBackground& pParams,
                                          const CFX_Matrix& matrix) {
  CFGAS_GEPath path;
  CFX_RectF rtHSep = pParams.part_rect_;
  path.MoveTo(CFX_PointF(rtHSep.left, rtHSep.top + rtHSep.height / 2));
  path.LineTo(CFX_PointF(rtHSep.right(), rtHSep.top + rtHSep.height / 2));

  CFGAS_GEGraphics::StateRestorer restorer(pParams.GetGraphics());
  pParams.GetGraphics()->SetStrokeColor(CFGAS_GEColor(kSeparatorColor));
  pParams.GetGraphics()->StrokePath(path, matrix);
}

void CFWL_MonthCalendarTP::DrawWeekNumSep(const CFWL_ThemeBackground& pParams,
                                          const CFX_Matrix& matrix) {
  CFGAS_GEPath path;
  CFX_RectF rtWeekSep = pParams.part_rect_;
  path.MoveTo(rtWeekSep.TopLeft());
  path.LineTo(rtWeekSep.BottomLeft());

  CFGAS_GEGraphics::StateRestorer restorer(pParams.GetGraphics());
  pParams.GetGraphics()->SetStrokeColor(CFGAS_GEColor(kSeparatorColor));
  pParams.GetGraphics()->StrokePath(path, matrix);
}

void CFWL_MonthCalendarTP::DrawDatesInBK(const CFWL_ThemeBackground& pParams,
                                         const CFX_Matrix& matrix) {
  CFGAS_GEGraphics::StateRestorer restorer(pParams.GetGraphics());
  if (pParams.states_ & CFWL_PartState::kSelected) {
    CFGAS_GEPath path;
    CFX_RectF rtSelDay = pParams.part_rect_;
    path.AddRectangle(rtSelDay.left, rtSelDay.top, rtSelDay.width,
                      rtSelDay.height);
    pParams.GetGraphics()->SetFillColor(
        CFGAS_GEColor(kDatesSelectedBackgroundColor));
    pParams.GetGraphics()->FillPath(
        path, CFX_FillRenderOptions::FillType::kWinding, matrix);
  } else if (pParams.states_ & CFWL_PartState::kHovered) {
    CFGAS_GEPath path;
    CFX_RectF rtSelDay = pParams.part_rect_;
    path.AddRectangle(rtSelDay.left, rtSelDay.top, rtSelDay.width,
                      rtSelDay.height);
    pParams.GetGraphics()->SetFillColor(
        CFGAS_GEColor(kDatesHoverBackgroundColor));
    pParams.GetGraphics()->FillPath(
        path, CFX_FillRenderOptions::FillType::kWinding, matrix);
  }
}

void CFWL_MonthCalendarTP::DrawDatesInCircle(
    const CFWL_ThemeBackground& pParams,
    const CFX_Matrix& matrix) {
  CFGAS_GEPath path;
  CFX_RectF rtSelDay = pParams.part_rect_;
  path.AddRectangle(rtSelDay.left, rtSelDay.top, rtSelDay.width,
                    rtSelDay.height);

  CFGAS_GEGraphics::StateRestorer restorer(pParams.GetGraphics());
  pParams.GetGraphics()->SetStrokeColor(CFGAS_GEColor(kDatesCircleColor));
  pParams.GetGraphics()->StrokePath(path, matrix);
}

void CFWL_MonthCalendarTP::DrawTodayCircle(const CFWL_ThemeBackground& pParams,
                                           const CFX_Matrix& matrix) {
  CFGAS_GEPath path;
  CFX_RectF rtTodayCircle = pParams.part_rect_;
  path.AddRectangle(rtTodayCircle.left, rtTodayCircle.top, rtTodayCircle.width,
                    rtTodayCircle.height);

  CFGAS_GEGraphics::StateRestorer restorer(pParams.GetGraphics());
  pParams.GetGraphics()->SetStrokeColor(CFGAS_GEColor(kDatesCircleColor));
  pParams.GetGraphics()->StrokePath(path, matrix);
}

}  // namespace pdfium
