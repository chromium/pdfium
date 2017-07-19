// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_appstream.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cba_fontmap.h"
#include "fpdfsdk/fxedit/fxet_edit.h"
#include "fpdfsdk/pdfwindow/cpwl_edit.h"
#include "fpdfsdk/pdfwindow/cpwl_icon.h"
#include "fpdfsdk/pdfwindow/cpwl_utils.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

namespace {

CFX_ByteString GetAP_Check(const CFX_FloatRect& crBBox) {
  const float fWidth = crBBox.right - crBBox.left;
  const float fHeight = crBBox.top - crBBox.bottom;

  CFX_PointF pts[8][3] = {{CFX_PointF(0.28f, 0.52f), CFX_PointF(0.27f, 0.48f),
                           CFX_PointF(0.29f, 0.40f)},
                          {CFX_PointF(0.30f, 0.33f), CFX_PointF(0.31f, 0.29f),
                           CFX_PointF(0.31f, 0.28f)},
                          {CFX_PointF(0.39f, 0.28f), CFX_PointF(0.49f, 0.29f),
                           CFX_PointF(0.77f, 0.67f)},
                          {CFX_PointF(0.76f, 0.68f), CFX_PointF(0.78f, 0.69f),
                           CFX_PointF(0.76f, 0.75f)},
                          {CFX_PointF(0.76f, 0.75f), CFX_PointF(0.73f, 0.80f),
                           CFX_PointF(0.68f, 0.75f)},
                          {CFX_PointF(0.68f, 0.74f), CFX_PointF(0.68f, 0.74f),
                           CFX_PointF(0.44f, 0.47f)},
                          {CFX_PointF(0.43f, 0.47f), CFX_PointF(0.40f, 0.47f),
                           CFX_PointF(0.41f, 0.58f)},
                          {CFX_PointF(0.40f, 0.60f), CFX_PointF(0.28f, 0.66f),
                           CFX_PointF(0.30f, 0.56f)}};

  for (size_t i = 0; i < FX_ArraySize(pts); ++i) {
    for (size_t j = 0; j < FX_ArraySize(pts[0]); ++j) {
      pts[i][j].x = pts[i][j].x * fWidth + crBBox.left;
      pts[i][j].y *= pts[i][j].y * fHeight + crBBox.bottom;
    }
  }

  std::ostringstream csAP;
  csAP << pts[0][0].x << " " << pts[0][0].y << " m\n";

  for (size_t i = 0; i < FX_ArraySize(pts); ++i) {
    size_t nNext = i < FX_ArraySize(pts) - 1 ? i + 1 : 0;

    float px1 = pts[i][1].x - pts[i][0].x;
    float py1 = pts[i][1].y - pts[i][0].y;
    float px2 = pts[i][2].x - pts[nNext][0].x;
    float py2 = pts[i][2].y - pts[nNext][0].y;

    csAP << pts[i][0].x + px1 * FX_BEZIER << " "
         << pts[i][0].y + py1 * FX_BEZIER << " "
         << pts[nNext][0].x + px2 * FX_BEZIER << " "
         << pts[nNext][0].y + py2 * FX_BEZIER << " " << pts[nNext][0].x << " "
         << pts[nNext][0].y << " c\n";
  }

  return CFX_ByteString(csAP);
}

CFX_ByteString GetAP_Circle(const CFX_FloatRect& crBBox) {
  std::ostringstream csAP;

  float fWidth = crBBox.right - crBBox.left;
  float fHeight = crBBox.top - crBBox.bottom;

  CFX_PointF pt1(crBBox.left, crBBox.bottom + fHeight / 2);
  CFX_PointF pt2(crBBox.left + fWidth / 2, crBBox.top);
  CFX_PointF pt3(crBBox.right, crBBox.bottom + fHeight / 2);
  CFX_PointF pt4(crBBox.left + fWidth / 2, crBBox.bottom);

  csAP << pt1.x << " " << pt1.y << " m\n";

  float px = pt2.x - pt1.x;
  float py = pt2.y - pt1.y;

  csAP << pt1.x << " " << pt1.y + py * FX_BEZIER << " "
       << pt2.x - px * FX_BEZIER << " " << pt2.y << " " << pt2.x << " " << pt2.y
       << " c\n";

  px = pt3.x - pt2.x;
  py = pt2.y - pt3.y;

  csAP << pt2.x + px * FX_BEZIER << " " << pt2.y << " " << pt3.x << " "
       << pt3.y + py * FX_BEZIER << " " << pt3.x << " " << pt3.y << " c\n";

  px = pt3.x - pt4.x;
  py = pt3.y - pt4.y;

  csAP << pt3.x << " " << pt3.y - py * FX_BEZIER << " "
       << pt4.x + px * FX_BEZIER << " " << pt4.y << " " << pt4.x << " " << pt4.y
       << " c\n";

  px = pt4.x - pt1.x;
  py = pt1.y - pt4.y;

  csAP << pt4.x - px * FX_BEZIER << " " << pt4.y << " " << pt1.x << " "
       << pt1.y - py * FX_BEZIER << " " << pt1.x << " " << pt1.y << " c\n";

  return CFX_ByteString(csAP);
}

CFX_ByteString GetAP_Cross(const CFX_FloatRect& crBBox) {
  std::ostringstream csAP;

  csAP << crBBox.left << " " << crBBox.top << " m\n";
  csAP << crBBox.right << " " << crBBox.bottom << " l\n";
  csAP << crBBox.left << " " << crBBox.bottom << " m\n";
  csAP << crBBox.right << " " << crBBox.top << " l\n";

  return CFX_ByteString(csAP);
}

CFX_ByteString GetAP_Diamond(const CFX_FloatRect& crBBox) {
  std::ostringstream csAP;

  float fWidth = crBBox.right - crBBox.left;
  float fHeight = crBBox.top - crBBox.bottom;

  CFX_PointF pt1(crBBox.left, crBBox.bottom + fHeight / 2);
  CFX_PointF pt2(crBBox.left + fWidth / 2, crBBox.top);
  CFX_PointF pt3(crBBox.right, crBBox.bottom + fHeight / 2);
  CFX_PointF pt4(crBBox.left + fWidth / 2, crBBox.bottom);

  csAP << pt1.x << " " << pt1.y << " m\n";
  csAP << pt2.x << " " << pt2.y << " l\n";
  csAP << pt3.x << " " << pt3.y << " l\n";
  csAP << pt4.x << " " << pt4.y << " l\n";
  csAP << pt1.x << " " << pt1.y << " l\n";

  return CFX_ByteString(csAP);
}

CFX_ByteString GetAP_Square(const CFX_FloatRect& crBBox) {
  std::ostringstream csAP;

  csAP << crBBox.left << " " << crBBox.top << " m\n";
  csAP << crBBox.right << " " << crBBox.top << " l\n";
  csAP << crBBox.right << " " << crBBox.bottom << " l\n";
  csAP << crBBox.left << " " << crBBox.bottom << " l\n";
  csAP << crBBox.left << " " << crBBox.top << " l\n";

  return CFX_ByteString(csAP);
}

CFX_ByteString GetAP_Star(const CFX_FloatRect& crBBox) {
  std::ostringstream csAP;

  float fRadius = (crBBox.top - crBBox.bottom) / (1 + (float)cos(FX_PI / 5.0f));
  CFX_PointF ptCenter = CFX_PointF((crBBox.left + crBBox.right) / 2.0f,
                                   (crBBox.top + crBBox.bottom) / 2.0f);

  float px[5];
  float py[5];
  float fAngel = FX_PI / 10.0f;
  for (int32_t i = 0; i < 5; i++) {
    px[i] = ptCenter.x + fRadius * (float)cos(fAngel);
    py[i] = ptCenter.y + fRadius * (float)sin(fAngel);
    fAngel += FX_PI * 2 / 5.0f;
  }

  csAP << px[0] << " " << py[0] << " m\n";

  int32_t nNext = 0;
  for (int32_t j = 0; j < 5; j++) {
    nNext += 2;
    if (nNext >= 5)
      nNext -= 5;
    csAP << px[nNext] << " " << py[nNext] << " l\n";
  }

  return CFX_ByteString(csAP);
}

CFX_ByteString GetAP_HalfCircle(const CFX_FloatRect& crBBox, float fRotate) {
  std::ostringstream csAP;

  float fWidth = crBBox.right - crBBox.left;
  float fHeight = crBBox.top - crBBox.bottom;

  CFX_PointF pt1(-fWidth / 2, 0);
  CFX_PointF pt2(0, fHeight / 2);
  CFX_PointF pt3(fWidth / 2, 0);

  float px;
  float py;

  csAP << cos(fRotate) << " " << sin(fRotate) << " " << -sin(fRotate) << " "
       << cos(fRotate) << " " << crBBox.left + fWidth / 2 << " "
       << crBBox.bottom + fHeight / 2 << " cm\n";

  csAP << pt1.x << " " << pt1.y << " m\n";

  px = pt2.x - pt1.x;
  py = pt2.y - pt1.y;

  csAP << pt1.x << " " << pt1.y + py * FX_BEZIER << " "
       << pt2.x - px * FX_BEZIER << " " << pt2.y << " " << pt2.x << " " << pt2.y
       << " c\n";

  px = pt3.x - pt2.x;
  py = pt2.y - pt3.y;

  csAP << pt2.x + px * FX_BEZIER << " " << pt2.y << " " << pt3.x << " "
       << pt3.y + py * FX_BEZIER << " " << pt3.x << " " << pt3.y << " c\n";

  return CFX_ByteString(csAP);
}

CFX_ByteString GetAppStream_Check(const CFX_FloatRect& rcBBox,
                                  const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n"
      << CPWL_Utils::GetColorAppStream(crText, true) << GetAP_Check(rcBBox)
      << "f\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetAppStream_Circle(const CFX_FloatRect& rcBBox,
                                   const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n"
      << CPWL_Utils::GetColorAppStream(crText, true) << GetAP_Circle(rcBBox)
      << "f\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetAppStream_Cross(const CFX_FloatRect& rcBBox,
                                  const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n"
      << CPWL_Utils::GetColorAppStream(crText, false) << GetAP_Cross(rcBBox)
      << "S\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetAppStream_Diamond(const CFX_FloatRect& rcBBox,
                                    const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n1 w\n"
      << CPWL_Utils::GetColorAppStream(crText, true) << GetAP_Diamond(rcBBox)
      << "f\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetAppStream_Square(const CFX_FloatRect& rcBBox,
                                   const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n"
      << CPWL_Utils::GetColorAppStream(crText, true) << GetAP_Square(rcBBox)
      << "f\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetAppStream_Star(const CFX_FloatRect& rcBBox,
                                 const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n"
      << CPWL_Utils::GetColorAppStream(crText, true) << GetAP_Star(rcBBox)
      << "f\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetCircleFillAppStream(const CFX_FloatRect& rect,
                                      const CFX_Color& color) {
  std::ostringstream sAppStream;
  CFX_ByteString sColor = CPWL_Utils::GetColorAppStream(color, true);
  if (sColor.GetLength() > 0)
    sAppStream << "q\n" << sColor << GetAP_Circle(rect) << "f\nQ\n";
  return CFX_ByteString(sAppStream);
}

CFX_ByteString GetCircleBorderAppStream(const CFX_FloatRect& rect,
                                        float fWidth,
                                        const CFX_Color& color,
                                        const CFX_Color& crLeftTop,
                                        const CFX_Color& crRightBottom,
                                        BorderStyle nStyle,
                                        const CPWL_Dash& dash) {
  std::ostringstream sAppStream;
  CFX_ByteString sColor;

  if (fWidth > 0.0f) {
    sAppStream << "q\n";

    float fHalfWidth = fWidth / 2.0f;
    CFX_FloatRect rect_by_2 = rect.GetDeflated(fHalfWidth, fHalfWidth);

    float div = fHalfWidth * 0.75f;
    CFX_FloatRect rect_by_75 = rect.GetDeflated(div, div);
    switch (nStyle) {
      default:
      case BorderStyle::SOLID:
      case BorderStyle::UNDERLINE: {
        sColor = CPWL_Utils::GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fWidth << " w\n"
                     << sColor << GetAP_Circle(rect_by_2) << " S\nQ\n";
        }
      } break;
      case BorderStyle::DASH: {
        sColor = CPWL_Utils::GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fWidth << " w\n"
                     << "[" << dash.nDash << " " << dash.nGap << "] "
                     << dash.nPhase << " d\n"
                     << sColor << GetAP_Circle(rect_by_2) << " S\nQ\n";
        }
      } break;
      case BorderStyle::BEVELED: {
        sColor = CPWL_Utils::GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_Circle(rect) << " S\nQ\n";
        }

        sColor = CPWL_Utils::GetColorAppStream(crLeftTop, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_HalfCircle(rect_by_75, FX_PI / 4.0f)
                     << " S\nQ\n";
        }

        sColor = CPWL_Utils::GetColorAppStream(crRightBottom, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_HalfCircle(rect_by_75, FX_PI * 5 / 4.0f)
                     << " S\nQ\n";
        }
      } break;
      case BorderStyle::INSET: {
        sColor = CPWL_Utils::GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_Circle(rect) << " S\nQ\n";
        }

        sColor = CPWL_Utils::GetColorAppStream(crLeftTop, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_HalfCircle(rect_by_75, FX_PI / 4.0f)
                     << " S\nQ\n";
        }

        sColor = CPWL_Utils::GetColorAppStream(crRightBottom, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_HalfCircle(rect_by_75, FX_PI * 5 / 4.0f)
                     << " S\nQ\n";
        }
      } break;
    }

    sAppStream << "Q\n";
  }
  return CFX_ByteString(sAppStream);
}

CFX_ByteString GetCheckBoxAppStream(const CFX_FloatRect& rcBBox,
                                    int32_t nStyle,
                                    const CFX_Color& crText) {
  CFX_FloatRect rcCenter = rcBBox.GetCenterSquare();
  switch (nStyle) {
    default:
    case PCS_CHECK:
      return GetAppStream_Check(rcCenter, crText);
    case PCS_CIRCLE:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Circle(rcCenter, crText);
    case PCS_CROSS:
      return GetAppStream_Cross(rcCenter, crText);
    case PCS_DIAMOND:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Diamond(rcCenter, crText);
    case PCS_SQUARE:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Square(rcCenter, crText);
    case PCS_STAR:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Star(rcCenter, crText);
  }
}

CFX_ByteString GetRadioButtonAppStream(const CFX_FloatRect& rcBBox,
                                       int32_t nStyle,
                                       const CFX_Color& crText) {
  CFX_FloatRect rcCenter = rcBBox.GetCenterSquare();
  switch (nStyle) {
    default:
    case PCS_CHECK:
      return GetAppStream_Check(rcCenter, crText);
    case PCS_CIRCLE:
      rcCenter.Scale(1.0f / 2.0f);
      return GetAppStream_Circle(rcCenter, crText);
    case PCS_CROSS:
      return GetAppStream_Cross(rcCenter, crText);
    case PCS_DIAMOND:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Diamond(rcCenter, crText);
    case PCS_SQUARE:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Square(rcCenter, crText);
    case PCS_STAR:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Star(rcCenter, crText);
  }
}

CFX_ByteString GetPushButtonAppStream(const CFX_FloatRect& rcBBox,
                                      IPVT_FontMap* pFontMap,
                                      CPDF_Stream* pIconStream,
                                      CPDF_IconFit& IconFit,
                                      const CFX_WideString& sLabel,
                                      const CFX_Color& crText,
                                      float fFontSize,
                                      int32_t nLayOut) {
  const float fAutoFontScale = 1.0f / 3.0f;

  auto pEdit = pdfium::MakeUnique<CFX_Edit>();
  pEdit->SetFontMap(pFontMap);
  pEdit->SetAlignmentH(1, true);
  pEdit->SetAlignmentV(1, true);
  pEdit->SetMultiLine(false, true);
  pEdit->SetAutoReturn(false, true);
  if (IsFloatZero(fFontSize))
    pEdit->SetAutoFontSize(true, true);
  else
    pEdit->SetFontSize(fFontSize);

  pEdit->Initialize();
  pEdit->SetText(sLabel);

  CFX_FloatRect rcLabelContent = pEdit->GetContentRect();
  CPWL_Icon Icon;
  PWL_CREATEPARAM cp;
  cp.dwFlags = PWS_VISIBLE;
  Icon.Create(cp);
  Icon.SetIconFit(&IconFit);
  Icon.SetPDFStream(pIconStream);

  CFX_FloatRect rcLabel;
  CFX_FloatRect rcIcon;
  float fWidth = 0.0f;
  float fHeight = 0.0f;

  switch (nLayOut) {
    case PPBL_LABEL:
      rcLabel = rcBBox;
      break;
    case PPBL_ICON:
      rcIcon = rcBBox;
      break;
    case PPBL_ICONTOPLABELBOTTOM:
      if (pIconStream) {
        if (IsFloatZero(fFontSize)) {
          fHeight = rcBBox.top - rcBBox.bottom;
          rcLabel = CFX_FloatRect(rcBBox.left, rcBBox.bottom, rcBBox.right,
                                  rcBBox.bottom + fHeight * fAutoFontScale);
          rcIcon =
              CFX_FloatRect(rcBBox.left, rcLabel.top, rcBBox.right, rcBBox.top);
        } else {
          fHeight = rcLabelContent.Height();

          if (rcBBox.bottom + fHeight > rcBBox.top) {
            rcLabel = rcBBox;
          } else {
            rcLabel = CFX_FloatRect(rcBBox.left, rcBBox.bottom, rcBBox.right,
                                    rcBBox.bottom + fHeight);
            rcIcon = CFX_FloatRect(rcBBox.left, rcLabel.top, rcBBox.right,
                                   rcBBox.top);
          }
        }
      } else {
        rcLabel = rcBBox;
      }
      break;
    case PPBL_LABELTOPICONBOTTOM:
      if (pIconStream) {
        if (IsFloatZero(fFontSize)) {
          fHeight = rcBBox.top - rcBBox.bottom;
          rcLabel =
              CFX_FloatRect(rcBBox.left, rcBBox.top - fHeight * fAutoFontScale,
                            rcBBox.right, rcBBox.top);
          rcIcon = CFX_FloatRect(rcBBox.left, rcBBox.bottom, rcBBox.right,
                                 rcLabel.bottom);
        } else {
          fHeight = rcLabelContent.Height();

          if (rcBBox.bottom + fHeight > rcBBox.top) {
            rcLabel = rcBBox;
          } else {
            rcLabel = CFX_FloatRect(rcBBox.left, rcBBox.top - fHeight,
                                    rcBBox.right, rcBBox.top);
            rcIcon = CFX_FloatRect(rcBBox.left, rcBBox.bottom, rcBBox.right,
                                   rcLabel.bottom);
          }
        }
      } else {
        rcLabel = rcBBox;
      }
      break;
    case PPBL_ICONLEFTLABELRIGHT:
      if (pIconStream) {
        if (IsFloatZero(fFontSize)) {
          fWidth = rcBBox.right - rcBBox.left;
          if (rcLabelContent.Width() < fWidth * fAutoFontScale) {
            rcLabel = CFX_FloatRect(rcBBox.right - fWidth * fAutoFontScale,
                                    rcBBox.bottom, rcBBox.right, rcBBox.top);
            rcIcon = CFX_FloatRect(rcBBox.left, rcBBox.bottom, rcLabel.left,
                                   rcBBox.top);
          } else {
            if (rcLabelContent.Width() < fWidth) {
              rcLabel = CFX_FloatRect(rcBBox.right - rcLabelContent.Width(),
                                      rcBBox.bottom, rcBBox.right, rcBBox.top);
              rcIcon = CFX_FloatRect(rcBBox.left, rcBBox.bottom, rcLabel.left,
                                     rcBBox.top);
            } else {
              rcLabel = rcBBox;
            }
          }
        } else {
          fWidth = rcLabelContent.Width();
          if (rcBBox.left + fWidth > rcBBox.right) {
            rcLabel = rcBBox;
          } else {
            rcLabel = CFX_FloatRect(rcBBox.right - fWidth, rcBBox.bottom,
                                    rcBBox.right, rcBBox.top);
            rcIcon = CFX_FloatRect(rcBBox.left, rcBBox.bottom, rcLabel.left,
                                   rcBBox.top);
          }
        }
      } else {
        rcLabel = rcBBox;
      }
      break;
    case PPBL_LABELLEFTICONRIGHT:
      if (pIconStream) {
        if (IsFloatZero(fFontSize)) {
          fWidth = rcBBox.right - rcBBox.left;
          if (rcLabelContent.Width() < fWidth * fAutoFontScale) {
            rcLabel = CFX_FloatRect(rcBBox.left, rcBBox.bottom,
                                    rcBBox.left + fWidth * fAutoFontScale,
                                    rcBBox.top);
            rcIcon = CFX_FloatRect(rcLabel.right, rcBBox.bottom, rcBBox.right,
                                   rcBBox.top);
          } else {
            if (rcLabelContent.Width() < fWidth) {
              rcLabel = CFX_FloatRect(rcBBox.left, rcBBox.bottom,
                                      rcBBox.left + rcLabelContent.Width(),
                                      rcBBox.top);
              rcIcon = CFX_FloatRect(rcLabel.right, rcBBox.bottom, rcBBox.right,
                                     rcBBox.top);
            } else {
              rcLabel = rcBBox;
            }
          }
        } else {
          fWidth = rcLabelContent.Width();
          if (rcBBox.left + fWidth > rcBBox.right) {
            rcLabel = rcBBox;
          } else {
            rcLabel = CFX_FloatRect(rcBBox.left, rcBBox.bottom,
                                    rcBBox.left + fWidth, rcBBox.top);
            rcIcon = CFX_FloatRect(rcLabel.right, rcBBox.bottom, rcBBox.right,
                                   rcBBox.top);
          }
        }
      } else {
        rcLabel = rcBBox;
      }
      break;
    case PPBL_LABELOVERICON:
      rcLabel = rcBBox;
      rcIcon = rcBBox;
      break;
  }

  std::ostringstream sTemp;

  if (!rcIcon.IsEmpty()) {
    Icon.Move(rcIcon, false, false);
    sTemp << Icon.GetImageAppStream();
  }

  Icon.Destroy();

  if (!rcLabel.IsEmpty()) {
    pEdit->SetPlateRect(rcLabel);
    CFX_ByteString sEdit =
        CPWL_Utils::GetEditAppStream(pEdit.get(), CFX_PointF(0.0f, 0.0f));
    if (sEdit.GetLength() > 0) {
      sTemp << "BT\n"
            << CPWL_Utils::GetColorAppStream(crText) << sEdit << "ET\n";
    }
  }

  if (sTemp.tellp() <= 0)
    return CFX_ByteString();

  std::ostringstream sAppStream;
  sAppStream << "q\n"
             << rcBBox.left << " " << rcBBox.bottom << " "
             << rcBBox.right - rcBBox.left << " " << rcBBox.top - rcBBox.bottom
             << " re W n\n";
  sAppStream << sTemp.str().c_str() << "Q\n";
  return CFX_ByteString(sAppStream);
}

CFX_ByteString GetDropButtonAppStream(const CFX_FloatRect& rcBBox) {
  if (rcBBox.IsEmpty())
    return CFX_ByteString();

  std::ostringstream sAppStream;
  sAppStream << "q\n"
             << CPWL_Utils::GetColorAppStream(
                    CFX_Color(COLORTYPE_RGB, 220.0f / 255.0f, 220.0f / 255.0f,
                              220.0f / 255.0f),
                    true)
             << rcBBox.left << " " << rcBBox.bottom << " "
             << rcBBox.right - rcBBox.left << " " << rcBBox.top - rcBBox.bottom
             << " re f\n"
             << "Q\n";

  sAppStream << "q\n"
             << CPWL_Utils::GetBorderAppStream(
                    rcBBox, 2, CFX_Color(COLORTYPE_GRAY, 0),
                    CFX_Color(COLORTYPE_GRAY, 1),
                    CFX_Color(COLORTYPE_GRAY, 0.5), BorderStyle::BEVELED,
                    CPWL_Dash(3, 0, 0))
             << "Q\n";

  CFX_PointF ptCenter = CFX_PointF((rcBBox.left + rcBBox.right) / 2,
                                   (rcBBox.top + rcBBox.bottom) / 2);
  if (IsFloatBigger(rcBBox.right - rcBBox.left, 6) &&
      IsFloatBigger(rcBBox.top - rcBBox.bottom, 6)) {
    sAppStream << "q\n"
               << " 0 g\n"
               << ptCenter.x - 3 << " " << ptCenter.y + 1.5f << " m\n"
               << ptCenter.x + 3 << " " << ptCenter.y + 1.5f << " l\n"
               << ptCenter.x << " " << ptCenter.y - 1.5f << " l\n"
               << ptCenter.x - 3 << " " << ptCenter.y + 1.5f << " l f\n"
               << "Q\n";
  }

  return CFX_ByteString(sAppStream);
}

}  // namespace

CPWL_AppStream::CPWL_AppStream(CPDFSDK_Widget* widget, CPDF_Dictionary* dict)
    : widget_(widget), dict_(dict) {}

CPWL_AppStream::~CPWL_AppStream() {}

void CPWL_AppStream::SetAsPushButton() {
  CPDF_FormControl* pControl = widget_->GetFormControl();
  CFX_FloatRect rcWindow = widget_->GetRotatedRect();
  int32_t nLayout = 0;
  switch (pControl->GetTextPosition()) {
    case TEXTPOS_ICON:
      nLayout = PPBL_ICON;
      break;
    case TEXTPOS_BELOW:
      nLayout = PPBL_ICONTOPLABELBOTTOM;
      break;
    case TEXTPOS_ABOVE:
      nLayout = PPBL_LABELTOPICONBOTTOM;
      break;
    case TEXTPOS_RIGHT:
      nLayout = PPBL_ICONLEFTLABELRIGHT;
      break;
    case TEXTPOS_LEFT:
      nLayout = PPBL_LABELLEFTICONRIGHT;
      break;
    case TEXTPOS_OVERLAID:
      nLayout = PPBL_LABELOVERICON;
      break;
    default:
      nLayout = PPBL_LABEL;
      break;
  }

  CFX_Color crBackground;
  CFX_Color crBorder;
  int iColorType;
  float fc[4];
  pControl->GetOriginalBackgroundColor(iColorType, fc);
  if (iColorType > 0)
    crBackground = CFX_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

  pControl->GetOriginalBorderColor(iColorType, fc);
  if (iColorType > 0)
    crBorder = CFX_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

  float fBorderWidth = static_cast<float>(widget_->GetBorderWidth());
  CPWL_Dash dsBorder(3, 0, 0);
  CFX_Color crLeftTop;
  CFX_Color crRightBottom;

  BorderStyle nBorderStyle = widget_->GetBorderStyle();
  switch (nBorderStyle) {
    case BorderStyle::DASH:
      dsBorder = CPWL_Dash(3, 3, 0);
      break;
    case BorderStyle::BEVELED:
      fBorderWidth *= 2;
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 1);
      crRightBottom = crBackground / 2.0f;
      break;
    case BorderStyle::INSET:
      fBorderWidth *= 2;
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 0.5);
      crRightBottom = CFX_Color(COLORTYPE_GRAY, 0.75);
      break;
    default:
      break;
  }

  CFX_FloatRect rcClient = rcWindow.GetDeflated(fBorderWidth, fBorderWidth);
  CFX_Color crText(COLORTYPE_GRAY, 0);
  CFX_ByteString csNameTag;
  CPDF_DefaultAppearance da = pControl->GetDefaultAppearance();
  if (da.HasColor()) {
    da.GetColor(iColorType, fc);
    crText = CFX_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);
  }
  float fFontSize = 12.0f;
  if (da.HasFont())
    csNameTag = da.GetFont(&fFontSize);

  CFX_WideString csWCaption;
  CFX_WideString csNormalCaption;
  CFX_WideString csRolloverCaption;
  CFX_WideString csDownCaption;
  if (pControl->HasMKEntry("CA"))
    csNormalCaption = pControl->GetNormalCaption();

  if (pControl->HasMKEntry("RC"))
    csRolloverCaption = pControl->GetRolloverCaption();

  if (pControl->HasMKEntry("AC"))
    csDownCaption = pControl->GetDownCaption();

  CPDF_Stream* pNormalIcon = nullptr;
  CPDF_Stream* pRolloverIcon = nullptr;
  CPDF_Stream* pDownIcon = nullptr;
  if (pControl->HasMKEntry("I"))
    pNormalIcon = pControl->GetNormalIcon();

  if (pControl->HasMKEntry("RI"))
    pRolloverIcon = pControl->GetRolloverIcon();

  if (pControl->HasMKEntry("IX"))
    pDownIcon = pControl->GetDownIcon();

  if (pNormalIcon) {
    if (CPDF_Dictionary* pImageDict = pNormalIcon->GetDict()) {
      if (pImageDict->GetStringFor("Name").IsEmpty())
        pImageDict->SetNewFor<CPDF_String>("Name", "ImgA", false);
    }
  }

  if (pRolloverIcon) {
    if (CPDF_Dictionary* pImageDict = pRolloverIcon->GetDict()) {
      if (pImageDict->GetStringFor("Name").IsEmpty())
        pImageDict->SetNewFor<CPDF_String>("Name", "ImgB", false);
    }
  }

  if (pDownIcon) {
    if (CPDF_Dictionary* pImageDict = pDownIcon->GetDict()) {
      if (pImageDict->GetStringFor("Name").IsEmpty())
        pImageDict->SetNewFor<CPDF_String>("Name", "ImgC", false);
    }
  }

  CPDF_IconFit iconFit = pControl->GetIconFit();

  CBA_FontMap font_map(
      widget_.Get(),
      widget_->GetInterForm()->GetFormFillEnv()->GetSysHandler());
  font_map.SetAPType("N");

  CFX_ByteString csAP =
      CPWL_Utils::GetRectFillAppStream(rcWindow, crBackground) +
      CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder,
                                     crLeftTop, crRightBottom, nBorderStyle,
                                     dsBorder) +
      GetPushButtonAppStream(iconFit.GetFittingBounds() ? rcWindow : rcClient,
                             &font_map, pNormalIcon, iconFit, csNormalCaption,
                             crText, fFontSize, nLayout);

  Write("N", csAP, "");
  if (pNormalIcon)
    AddImage("N", pNormalIcon);

  CPDF_FormControl::HighlightingMode eHLM = pControl->GetHighlightingMode();
  if (eHLM == CPDF_FormControl::Push || eHLM == CPDF_FormControl::Toggle) {
    if (csRolloverCaption.IsEmpty() && !pRolloverIcon) {
      csRolloverCaption = csNormalCaption;
      pRolloverIcon = pNormalIcon;
    }

    font_map.SetAPType("R");

    csAP =
        CPWL_Utils::GetRectFillAppStream(rcWindow, crBackground) +
        CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder,
                                       crLeftTop, crRightBottom, nBorderStyle,
                                       dsBorder) +
        GetPushButtonAppStream(iconFit.GetFittingBounds() ? rcWindow : rcClient,
                               &font_map, pRolloverIcon, iconFit,
                               csRolloverCaption, crText, fFontSize, nLayout);

    Write("R", csAP, "");
    if (pRolloverIcon)
      AddImage("R", pRolloverIcon);

    if (csDownCaption.IsEmpty() && !pDownIcon) {
      csDownCaption = csNormalCaption;
      pDownIcon = pNormalIcon;
    }

    switch (nBorderStyle) {
      case BorderStyle::BEVELED: {
        CFX_Color crTemp = crLeftTop;
        crLeftTop = crRightBottom;
        crRightBottom = crTemp;
        break;
      }
      case BorderStyle::INSET: {
        crLeftTop = CFX_Color(COLORTYPE_GRAY, 0);
        crRightBottom = CFX_Color(COLORTYPE_GRAY, 1);
        break;
      }
      default:
        break;
    }

    font_map.SetAPType("D");

    csAP = CPWL_Utils::GetRectFillAppStream(rcWindow, crBackground - 0.25f) +
           CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder,
                                          crLeftTop, crRightBottom,
                                          nBorderStyle, dsBorder) +
           GetPushButtonAppStream(
               iconFit.GetFittingBounds() ? rcWindow : rcClient, &font_map,
               pDownIcon, iconFit, csDownCaption, crText, fFontSize, nLayout);

    Write("D", csAP, "");
    if (pDownIcon)
      AddImage("D", pDownIcon);
  } else {
    Remove("D");
    Remove("R");
  }
}

void CPWL_AppStream::SetAsCheckBox() {
  CPDF_FormControl* pControl = widget_->GetFormControl();
  CFX_Color crBackground, crBorder, crText;
  int iColorType;
  float fc[4];

  pControl->GetOriginalBackgroundColor(iColorType, fc);
  if (iColorType > 0)
    crBackground = CFX_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

  pControl->GetOriginalBorderColor(iColorType, fc);
  if (iColorType > 0)
    crBorder = CFX_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

  float fBorderWidth = static_cast<float>(widget_->GetBorderWidth());
  CPWL_Dash dsBorder(3, 0, 0);
  CFX_Color crLeftTop, crRightBottom;

  BorderStyle nBorderStyle = widget_->GetBorderStyle();
  switch (nBorderStyle) {
    case BorderStyle::DASH:
      dsBorder = CPWL_Dash(3, 3, 0);
      break;
    case BorderStyle::BEVELED:
      fBorderWidth *= 2;
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 1);
      crRightBottom = crBackground / 2.0f;
      break;
    case BorderStyle::INSET:
      fBorderWidth *= 2;
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 0.5);
      crRightBottom = CFX_Color(COLORTYPE_GRAY, 0.75);
      break;
    default:
      break;
  }

  CFX_FloatRect rcWindow = widget_->GetRotatedRect();
  CFX_FloatRect rcClient = rcWindow.GetDeflated(fBorderWidth, fBorderWidth);
  CPDF_DefaultAppearance da = pControl->GetDefaultAppearance();
  if (da.HasColor()) {
    da.GetColor(iColorType, fc);
    crText = CFX_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);
  }

  int32_t nStyle = 0;
  CFX_WideString csWCaption = pControl->GetNormalCaption();
  if (csWCaption.GetLength() > 0) {
    switch (csWCaption[0]) {
      case L'l':
        nStyle = PCS_CIRCLE;
        break;
      case L'8':
        nStyle = PCS_CROSS;
        break;
      case L'u':
        nStyle = PCS_DIAMOND;
        break;
      case L'n':
        nStyle = PCS_SQUARE;
        break;
      case L'H':
        nStyle = PCS_STAR;
        break;
      default:  // L'4'
        nStyle = PCS_CHECK;
        break;
    }
  } else {
    nStyle = PCS_CHECK;
  }

  CFX_ByteString csAP_N_ON =
      CPWL_Utils::GetRectFillAppStream(rcWindow, crBackground) +
      CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder,
                                     crLeftTop, crRightBottom, nBorderStyle,
                                     dsBorder);

  CFX_ByteString csAP_N_OFF = csAP_N_ON;

  switch (nBorderStyle) {
    case BorderStyle::BEVELED: {
      CFX_Color crTemp = crLeftTop;
      crLeftTop = crRightBottom;
      crRightBottom = crTemp;
      break;
    }
    case BorderStyle::INSET: {
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 0);
      crRightBottom = CFX_Color(COLORTYPE_GRAY, 1);
      break;
    }
    default:
      break;
  }

  CFX_ByteString csAP_D_ON =
      CPWL_Utils::GetRectFillAppStream(rcWindow, crBackground - 0.25f) +
      CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder,
                                     crLeftTop, crRightBottom, nBorderStyle,
                                     dsBorder);

  CFX_ByteString csAP_D_OFF = csAP_D_ON;

  csAP_N_ON += GetCheckBoxAppStream(rcClient, nStyle, crText);
  csAP_D_ON += GetCheckBoxAppStream(rcClient, nStyle, crText);

  Write("N", csAP_N_ON, pControl->GetCheckedAPState());
  Write("N", csAP_N_OFF, "Off");

  Write("D", csAP_D_ON, pControl->GetCheckedAPState());
  Write("D", csAP_D_OFF, "Off");

  CFX_ByteString csAS = widget_->GetAppState();
  if (csAS.IsEmpty())
    widget_->SetAppState("Off");
}

void CPWL_AppStream::SetAsRadioButton() {
  CPDF_FormControl* pControl = widget_->GetFormControl();
  CFX_Color crBackground;
  CFX_Color crBorder;
  CFX_Color crText;
  int iColorType;
  float fc[4];

  pControl->GetOriginalBackgroundColor(iColorType, fc);
  if (iColorType > 0)
    crBackground = CFX_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

  pControl->GetOriginalBorderColor(iColorType, fc);
  if (iColorType > 0)
    crBorder = CFX_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

  float fBorderWidth = static_cast<float>(widget_->GetBorderWidth());
  CPWL_Dash dsBorder(3, 0, 0);
  CFX_Color crLeftTop;
  CFX_Color crRightBottom;
  BorderStyle nBorderStyle = widget_->GetBorderStyle();
  switch (nBorderStyle) {
    case BorderStyle::DASH:
      dsBorder = CPWL_Dash(3, 3, 0);
      break;
    case BorderStyle::BEVELED:
      fBorderWidth *= 2;
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 1);
      crRightBottom = crBackground / 2.0f;
      break;
    case BorderStyle::INSET:
      fBorderWidth *= 2;
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 0.5);
      crRightBottom = CFX_Color(COLORTYPE_GRAY, 0.75);
      break;
    default:
      break;
  }

  CFX_FloatRect rcWindow = widget_->GetRotatedRect();
  CFX_FloatRect rcClient = rcWindow.GetDeflated(fBorderWidth, fBorderWidth);
  CPDF_DefaultAppearance da = pControl->GetDefaultAppearance();
  if (da.HasColor()) {
    da.GetColor(iColorType, fc);
    crText = CFX_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);
  }

  int32_t nStyle = 0;
  CFX_WideString csWCaption = pControl->GetNormalCaption();
  if (csWCaption.GetLength() > 0) {
    switch (csWCaption[0]) {
      default:  // L'l':
        nStyle = PCS_CIRCLE;
        break;
      case L'8':
        nStyle = PCS_CROSS;
        break;
      case L'u':
        nStyle = PCS_DIAMOND;
        break;
      case L'n':
        nStyle = PCS_SQUARE;
        break;
      case L'H':
        nStyle = PCS_STAR;
        break;
      case L'4':
        nStyle = PCS_CHECK;
        break;
    }
  } else {
    nStyle = PCS_CIRCLE;
  }

  CFX_ByteString csAP_N_ON;
  CFX_FloatRect rcCenter = rcWindow.GetCenterSquare().GetDeflated(1.0f, 1.0f);
  if (nStyle == PCS_CIRCLE) {
    if (nBorderStyle == BorderStyle::BEVELED) {
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 1);
      crRightBottom = crBackground - 0.25f;
    } else if (nBorderStyle == BorderStyle::INSET) {
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 0.5f);
      crRightBottom = CFX_Color(COLORTYPE_GRAY, 0.75f);
    }

    csAP_N_ON =
        GetCircleFillAppStream(rcCenter, crBackground) +
        GetCircleBorderAppStream(rcCenter, fBorderWidth, crBorder, crLeftTop,
                                 crRightBottom, nBorderStyle, dsBorder);
  } else {
    csAP_N_ON = CPWL_Utils::GetRectFillAppStream(rcWindow, crBackground) +
                CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder,
                                               crLeftTop, crRightBottom,
                                               nBorderStyle, dsBorder);
  }

  CFX_ByteString csAP_N_OFF = csAP_N_ON;

  switch (nBorderStyle) {
    case BorderStyle::BEVELED: {
      CFX_Color crTemp = crLeftTop;
      crLeftTop = crRightBottom;
      crRightBottom = crTemp;
      break;
    }
    case BorderStyle::INSET: {
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 0);
      crRightBottom = CFX_Color(COLORTYPE_GRAY, 1);
      break;
    }
    default:
      break;
  }

  CFX_ByteString csAP_D_ON;

  if (nStyle == PCS_CIRCLE) {
    CFX_Color crBK = crBackground - 0.25f;
    if (nBorderStyle == BorderStyle::BEVELED) {
      crLeftTop = crBackground - 0.25f;
      crRightBottom = CFX_Color(COLORTYPE_GRAY, 1);
      crBK = crBackground;
    } else if (nBorderStyle == BorderStyle::INSET) {
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 0);
      crRightBottom = CFX_Color(COLORTYPE_GRAY, 1);
    }

    csAP_D_ON =
        GetCircleFillAppStream(rcCenter, crBK) +
        GetCircleBorderAppStream(rcCenter, fBorderWidth, crBorder, crLeftTop,
                                 crRightBottom, nBorderStyle, dsBorder);
  } else {
    csAP_D_ON =
        CPWL_Utils::GetRectFillAppStream(rcWindow, crBackground - 0.25f) +
        CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder,
                                       crLeftTop, crRightBottom, nBorderStyle,
                                       dsBorder);
  }

  CFX_ByteString csAP_D_OFF = csAP_D_ON;

  csAP_N_ON += GetRadioButtonAppStream(rcClient, nStyle, crText);
  csAP_D_ON += GetRadioButtonAppStream(rcClient, nStyle, crText);

  Write("N", csAP_N_ON, pControl->GetCheckedAPState());
  Write("N", csAP_N_OFF, "Off");

  Write("D", csAP_D_ON, pControl->GetCheckedAPState());
  Write("D", csAP_D_OFF, "Off");

  CFX_ByteString csAS = widget_->GetAppState();
  if (csAS.IsEmpty())
    widget_->SetAppState("Off");
}

void CPWL_AppStream::SetAsComboBox(const CFX_WideString* sValue) {
  CPDF_FormControl* pControl = widget_->GetFormControl();
  CPDF_FormField* pField = pControl->GetField();
  std::ostringstream sBody;

  CFX_FloatRect rcClient = widget_->GetClientRect();
  CFX_FloatRect rcButton = rcClient;
  rcButton.left = rcButton.right - 13;
  rcButton.Normalize();

  auto pEdit = pdfium::MakeUnique<CFX_Edit>();
  pEdit->EnableRefresh(false);

  CBA_FontMap font_map(
      widget_.Get(),
      widget_->GetInterForm()->GetFormFillEnv()->GetSysHandler());
  pEdit->SetFontMap(&font_map);

  CFX_FloatRect rcEdit = rcClient;
  rcEdit.right = rcButton.left;
  rcEdit.Normalize();

  pEdit->SetPlateRect(rcEdit);
  pEdit->SetAlignmentV(1, true);

  float fFontSize = widget_->GetFontSize();
  if (IsFloatZero(fFontSize))
    pEdit->SetAutoFontSize(true, true);
  else
    pEdit->SetFontSize(fFontSize);

  pEdit->Initialize();

  if (sValue) {
    pEdit->SetText(*sValue);
  } else {
    int32_t nCurSel = pField->GetSelectedIndex(0);
    if (nCurSel < 0)
      pEdit->SetText(pField->GetValue());
    else
      pEdit->SetText(pField->GetOptionLabel(nCurSel));
  }

  CFX_FloatRect rcContent = pEdit->GetContentRect();
  CFX_ByteString sEdit =
      CPWL_Utils::GetEditAppStream(pEdit.get(), CFX_PointF());
  if (sEdit.GetLength() > 0) {
    sBody << "/Tx BMC\n"
          << "q\n";
    if (rcContent.Width() > rcEdit.Width() ||
        rcContent.Height() > rcEdit.Height()) {
      sBody << rcEdit.left << " " << rcEdit.bottom << " " << rcEdit.Width()
            << " " << rcEdit.Height() << " re\nW\nn\n";
    }

    CFX_Color crText = widget_->GetTextPWLColor();
    sBody << "BT\n"
          << CPWL_Utils::GetColorAppStream(crText) << sEdit << "ET\n"
          << "Q\nEMC\n";
  }

  sBody << GetDropButtonAppStream(rcButton);
  Write("N",
        GetBackgroundAppStream() + GetBorderAppStream() + CFX_ByteString(sBody),
        "");
}

void CPWL_AppStream::SetAsListBox() {
  CPDF_FormControl* pControl = widget_->GetFormControl();
  CPDF_FormField* pField = pControl->GetField();
  CFX_FloatRect rcClient = widget_->GetClientRect();
  std::ostringstream sBody;

  auto pEdit = pdfium::MakeUnique<CFX_Edit>();
  pEdit->EnableRefresh(false);

  CBA_FontMap font_map(
      widget_.Get(),
      widget_->GetInterForm()->GetFormFillEnv()->GetSysHandler());
  pEdit->SetFontMap(&font_map);
  pEdit->SetPlateRect(CFX_FloatRect(rcClient.left, 0.0f, rcClient.right, 0.0f));

  float fFontSize = widget_->GetFontSize();
  pEdit->SetFontSize(IsFloatZero(fFontSize) ? 12.0f : fFontSize);
  pEdit->Initialize();

  std::ostringstream sList;
  float fy = rcClient.top;

  int32_t nTop = pField->GetTopVisibleIndex();
  int32_t nCount = pField->CountOptions();
  int32_t nSelCount = pField->CountSelectedItems();

  for (int32_t i = nTop; i < nCount; ++i) {
    bool bSelected = false;
    for (int32_t j = 0; j < nSelCount; ++j) {
      if (pField->GetSelectedIndex(j) == i) {
        bSelected = true;
        break;
      }
    }

    pEdit->SetText(pField->GetOptionLabel(i));

    CFX_FloatRect rcContent = pEdit->GetContentRect();
    float fItemHeight = rcContent.Height();

    if (bSelected) {
      CFX_FloatRect rcItem =
          CFX_FloatRect(rcClient.left, fy - fItemHeight, rcClient.right, fy);
      sList << "q\n"
            << CPWL_Utils::GetColorAppStream(
                   CFX_Color(COLORTYPE_RGB, 0, 51.0f / 255.0f, 113.0f / 255.0f),
                   true)
            << rcItem.left << " " << rcItem.bottom << " " << rcItem.Width()
            << " " << rcItem.Height() << " re f\n"
            << "Q\n";

      sList << "BT\n"
            << CPWL_Utils::GetColorAppStream(CFX_Color(COLORTYPE_GRAY, 1), true)
            << CPWL_Utils::GetEditAppStream(pEdit.get(), CFX_PointF(0.0f, fy))
            << "ET\n";
    } else {
      CFX_Color crText = widget_->GetTextPWLColor();
      sList << "BT\n"
            << CPWL_Utils::GetColorAppStream(crText, true)
            << CPWL_Utils::GetEditAppStream(pEdit.get(), CFX_PointF(0.0f, fy))
            << "ET\n";
    }

    fy -= fItemHeight;
  }

  if (sList.tellp() > 0) {
    sBody << "/Tx BMC\n"
          << "q\n"
          << rcClient.left << " " << rcClient.bottom << " " << rcClient.Width()
          << " " << rcClient.Height() << " re\nW\nn\n";
    sBody << sList.str() << "Q\nEMC\n";
  }
  Write("N",
        GetBackgroundAppStream() + GetBorderAppStream() + CFX_ByteString(sBody),
        "");
}

void CPWL_AppStream::SetAsTextField(const CFX_WideString* sValue) {
  CPDF_FormControl* pControl = widget_->GetFormControl();
  CPDF_FormField* pField = pControl->GetField();
  std::ostringstream sBody;
  std::ostringstream sLines;

  auto pEdit = pdfium::MakeUnique<CFX_Edit>();
  pEdit->EnableRefresh(false);

  CBA_FontMap font_map(
      widget_.Get(),
      widget_->GetInterForm()->GetFormFillEnv()->GetSysHandler());
  pEdit->SetFontMap(&font_map);

  CFX_FloatRect rcClient = widget_->GetClientRect();
  pEdit->SetPlateRect(rcClient);
  pEdit->SetAlignmentH(pControl->GetControlAlignment(), true);

  uint32_t dwFieldFlags = pField->GetFieldFlags();
  bool bMultiLine = (dwFieldFlags >> 12) & 1;
  if (bMultiLine) {
    pEdit->SetMultiLine(true, true);
    pEdit->SetAutoReturn(true, true);
  } else {
    pEdit->SetAlignmentV(1, true);
  }

  uint16_t subWord = 0;
  if ((dwFieldFlags >> 13) & 1) {
    subWord = '*';
    pEdit->SetPasswordChar(subWord, true);
  }

  int nMaxLen = pField->GetMaxLen();
  bool bCharArray = (dwFieldFlags >> 24) & 1;
  float fFontSize = widget_->GetFontSize();

#ifdef PDF_ENABLE_XFA
  CFX_WideString sValueTmp;
  if (!sValue && widget_->GetMixXFAWidget()) {
    sValueTmp = widget_->GetValue(true);
    sValue = &sValueTmp;
  }
#endif  // PDF_ENABLE_XFA

  if (nMaxLen > 0) {
    if (bCharArray) {
      pEdit->SetCharArray(nMaxLen);

      if (IsFloatZero(fFontSize)) {
        fFontSize = CPWL_Edit::GetCharArrayAutoFontSize(font_map.GetPDFFont(0),
                                                        rcClient, nMaxLen);
      }
    } else {
      if (sValue)
        nMaxLen = sValue->GetLength();
      pEdit->SetLimitChar(nMaxLen);
    }
  }

  if (IsFloatZero(fFontSize))
    pEdit->SetAutoFontSize(true, true);
  else
    pEdit->SetFontSize(fFontSize);

  pEdit->Initialize();
  pEdit->SetText(sValue ? *sValue : pField->GetValue());

  CFX_FloatRect rcContent = pEdit->GetContentRect();
  CFX_ByteString sEdit = CPWL_Utils::GetEditAppStream(
      pEdit.get(), CFX_PointF(), nullptr, !bCharArray, subWord);

  if (sEdit.GetLength() > 0) {
    sBody << "/Tx BMC\n"
          << "q\n";
    if (rcContent.Width() > rcClient.Width() ||
        rcContent.Height() > rcClient.Height()) {
      sBody << rcClient.left << " " << rcClient.bottom << " "
            << rcClient.Width() << " " << rcClient.Height() << " re\nW\nn\n";
    }
    CFX_Color crText = widget_->GetTextPWLColor();
    sBody << "BT\n"
          << CPWL_Utils::GetColorAppStream(crText) << sEdit << "ET\n"
          << "Q\nEMC\n";
  }

  if (bCharArray) {
    switch (widget_->GetBorderStyle()) {
      case BorderStyle::SOLID: {
        CFX_ByteString sColor =
            CPWL_Utils::GetColorAppStream(widget_->GetBorderPWLColor(), false);
        if (sColor.GetLength() > 0) {
          sLines << "q\n"
                 << widget_->GetBorderWidth() << " w\n"
                 << CPWL_Utils::GetColorAppStream(widget_->GetBorderPWLColor(),
                                                  false)
                 << " 2 J 0 j\n";

          for (int32_t i = 1; i < nMaxLen; ++i) {
            sLines << rcClient.left +
                          ((rcClient.right - rcClient.left) / nMaxLen) * i
                   << " " << rcClient.bottom << " m\n"
                   << rcClient.left +
                          ((rcClient.right - rcClient.left) / nMaxLen) * i
                   << " " << rcClient.top << " l S\n";
          }

          sLines << "Q\n";
        }
        break;
      }
      case BorderStyle::DASH: {
        CFX_ByteString sColor =
            CPWL_Utils::GetColorAppStream(widget_->GetBorderPWLColor(), false);
        if (sColor.GetLength() > 0) {
          CPWL_Dash dsBorder = CPWL_Dash(3, 3, 0);

          sLines << "q\n"
                 << widget_->GetBorderWidth() << " w\n"
                 << CPWL_Utils::GetColorAppStream(widget_->GetBorderPWLColor(),
                                                  false)
                 << "[" << dsBorder.nDash << " " << dsBorder.nGap << "] "
                 << dsBorder.nPhase << " d\n";

          for (int32_t i = 1; i < nMaxLen; ++i) {
            sLines << rcClient.left +
                          ((rcClient.right - rcClient.left) / nMaxLen) * i
                   << " " << rcClient.bottom << " m\n"
                   << rcClient.left +
                          ((rcClient.right - rcClient.left) / nMaxLen) * i
                   << " " << rcClient.top << " l S\n";
          }

          sLines << "Q\n";
        }
        break;
      }
      default:
        break;
    }
  }

  Write("N",
        GetBackgroundAppStream() + GetBorderAppStream() +
            CFX_ByteString(sLines) + CFX_ByteString(sBody),
        "");
}

void CPWL_AppStream::AddImage(const CFX_ByteString& sAPType,
                              CPDF_Stream* pImage) {
  CPDF_Stream* pStream = dict_->GetStreamFor(sAPType);
  CPDF_Dictionary* pStreamDict = pStream->GetDict();
  CFX_ByteString sImageAlias = "IMG";

  if (CPDF_Dictionary* pImageDict = pImage->GetDict()) {
    sImageAlias = pImageDict->GetStringFor("Name");
    if (sImageAlias.IsEmpty())
      sImageAlias = "IMG";
  }

  CPDF_Dictionary* pStreamResList = pStreamDict->GetDictFor("Resources");
  if (!pStreamResList)
    pStreamResList = pStreamDict->SetNewFor<CPDF_Dictionary>("Resources");

  CPDF_Dictionary* pXObject =
      pStreamResList->SetNewFor<CPDF_Dictionary>("XObject");
  pXObject->SetNewFor<CPDF_Reference>(sImageAlias,
                                      widget_->GetPageView()->GetPDFDocument(),
                                      pImage->GetObjNum());
}

void CPWL_AppStream::Write(const CFX_ByteString& sAPType,
                           const CFX_ByteString& sContents,
                           const CFX_ByteString& sAPState) {
  CPDF_Stream* pStream = nullptr;
  CPDF_Dictionary* pParentDict = nullptr;
  if (sAPState.IsEmpty()) {
    pParentDict = dict_.Get();
    pStream = dict_->GetStreamFor(sAPType);
  } else {
    CPDF_Dictionary* pAPTypeDict = dict_->GetDictFor(sAPType);
    if (!pAPTypeDict)
      pAPTypeDict = dict_->SetNewFor<CPDF_Dictionary>(sAPType);

    pParentDict = pAPTypeDict;
    pStream = pAPTypeDict->GetStreamFor(sAPState);
  }

  if (!pStream) {
    CPDF_Document* doc = widget_->GetPageView()->GetPDFDocument();
    pStream = doc->NewIndirect<CPDF_Stream>();
    pParentDict->SetNewFor<CPDF_Reference>(sAPType, doc, pStream->GetObjNum());
  }

  CPDF_Dictionary* pStreamDict = pStream->GetDict();
  if (!pStreamDict) {
    auto pNewDict = pdfium::MakeUnique<CPDF_Dictionary>(
        widget_->GetPDFAnnot()->GetDocument()->GetByteStringPool());
    pStreamDict = pNewDict.get();
    pStreamDict->SetNewFor<CPDF_Name>("Type", "XObject");
    pStreamDict->SetNewFor<CPDF_Name>("Subtype", "Form");
    pStreamDict->SetNewFor<CPDF_Number>("FormType", 1);
    pStream->InitStream(nullptr, 0, std::move(pNewDict));
  }
  pStreamDict->SetMatrixFor("Matrix", widget_->GetMatrix());
  pStreamDict->SetRectFor("BBox", widget_->GetRotatedRect());
  pStream->SetData((uint8_t*)(sContents.c_str()), sContents.GetLength());
}

void CPWL_AppStream::Remove(const CFX_ByteString& sAPType) {
  dict_->RemoveFor(sAPType);
}

CFX_ByteString CPWL_AppStream::GetBackgroundAppStream() const {
  CFX_Color crBackground = widget_->GetFillPWLColor();
  if (crBackground.nColorType != COLORTYPE_TRANSPARENT)
    return CPWL_Utils::GetRectFillAppStream(widget_->GetRotatedRect(),
                                            crBackground);

  return CFX_ByteString();
}

CFX_ByteString CPWL_AppStream::GetBorderAppStream() const {
  CFX_FloatRect rcWindow = widget_->GetRotatedRect();
  CFX_Color crBorder = widget_->GetBorderPWLColor();
  CFX_Color crBackground = widget_->GetFillPWLColor();
  CFX_Color crLeftTop;
  CFX_Color crRightBottom;

  float fBorderWidth = static_cast<float>(widget_->GetBorderWidth());
  CPWL_Dash dsBorder(3, 0, 0);

  BorderStyle nBorderStyle = widget_->GetBorderStyle();
  switch (nBorderStyle) {
    case BorderStyle::DASH:
      dsBorder = CPWL_Dash(3, 3, 0);
      break;
    case BorderStyle::BEVELED:
      fBorderWidth *= 2;
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 1);
      crRightBottom = crBackground / 2.0f;
      break;
    case BorderStyle::INSET:
      fBorderWidth *= 2;
      crLeftTop = CFX_Color(COLORTYPE_GRAY, 0.5);
      crRightBottom = CFX_Color(COLORTYPE_GRAY, 0.75);
      break;
    default:
      break;
  }

  return CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder,
                                        crLeftTop, crRightBottom, nBorderStyle,
                                        dsBorder);
}
