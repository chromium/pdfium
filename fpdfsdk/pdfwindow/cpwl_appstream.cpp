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
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfdoc/cpvt_word.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cba_fontmap.h"
#include "fpdfsdk/fxedit/fxet_edit.h"
#include "fpdfsdk/pdfwindow/cpwl_edit.h"
#include "fpdfsdk/pdfwindow/cpwl_icon.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

namespace {

// Checkbox & radiobutton styles.
enum class CheckStyle { kCheck = 0, kCircle, kCross, kDiamond, kSquare, kStar };

// Pushbutton layout styles.
enum class ButtonStyle {
  kLabel = 0,
  kIcon,
  kIconTopLabelBottom,
  kIconBottomLabelTop,
  kIconLeftLabelRight,
  kIconRightLabelLeft,
  kLabelOverIcon
};

CFX_ByteString GetColorAppStream(const CFX_Color& color,
                                 const bool& bFillOrStroke) {
  std::ostringstream sColorStream;

  switch (color.nColorType) {
    case COLORTYPE_RGB:
      sColorStream << color.fColor1 << " " << color.fColor2 << " "
                   << color.fColor3 << " " << (bFillOrStroke ? "rg" : "RG")
                   << "\n";
      break;
    case COLORTYPE_GRAY:
      sColorStream << color.fColor1 << " " << (bFillOrStroke ? "g" : "G")
                   << "\n";
      break;
    case COLORTYPE_CMYK:
      sColorStream << color.fColor1 << " " << color.fColor2 << " "
                   << color.fColor3 << " " << color.fColor4 << " "
                   << (bFillOrStroke ? "k" : "K") << "\n";
      break;
  }

  return CFX_ByteString(sColorStream);
}

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
      << GetColorAppStream(crText, true) << GetAP_Check(rcBBox) << "f\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetAppStream_Circle(const CFX_FloatRect& rcBBox,
                                   const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n"
      << GetColorAppStream(crText, true) << GetAP_Circle(rcBBox) << "f\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetAppStream_Cross(const CFX_FloatRect& rcBBox,
                                  const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n"
      << GetColorAppStream(crText, false) << GetAP_Cross(rcBBox) << "S\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetAppStream_Diamond(const CFX_FloatRect& rcBBox,
                                    const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n1 w\n"
      << GetColorAppStream(crText, true) << GetAP_Diamond(rcBBox) << "f\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetAppStream_Square(const CFX_FloatRect& rcBBox,
                                   const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n"
      << GetColorAppStream(crText, true) << GetAP_Square(rcBBox) << "f\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetAppStream_Star(const CFX_FloatRect& rcBBox,
                                 const CFX_Color& crText) {
  std::ostringstream sAP;
  sAP << "q\n"
      << GetColorAppStream(crText, true) << GetAP_Star(rcBBox) << "f\nQ\n";
  return CFX_ByteString(sAP);
}

CFX_ByteString GetCircleFillAppStream(const CFX_FloatRect& rect,
                                      const CFX_Color& color) {
  std::ostringstream sAppStream;
  CFX_ByteString sColor = GetColorAppStream(color, true);
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
        sColor = GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fWidth << " w\n"
                     << sColor << GetAP_Circle(rect_by_2) << " S\nQ\n";
        }
      } break;
      case BorderStyle::DASH: {
        sColor = GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fWidth << " w\n"
                     << "[" << dash.nDash << " " << dash.nGap << "] "
                     << dash.nPhase << " d\n"
                     << sColor << GetAP_Circle(rect_by_2) << " S\nQ\n";
        }
      } break;
      case BorderStyle::BEVELED: {
        sColor = GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_Circle(rect) << " S\nQ\n";
        }

        sColor = GetColorAppStream(crLeftTop, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_HalfCircle(rect_by_75, FX_PI / 4.0f)
                     << " S\nQ\n";
        }

        sColor = GetColorAppStream(crRightBottom, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_HalfCircle(rect_by_75, FX_PI * 5 / 4.0f)
                     << " S\nQ\n";
        }
      } break;
      case BorderStyle::INSET: {
        sColor = GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_Circle(rect) << " S\nQ\n";
        }

        sColor = GetColorAppStream(crLeftTop, false);
        if (sColor.GetLength() > 0) {
          sAppStream << "q\n"
                     << fHalfWidth << " w\n"
                     << sColor << GetAP_HalfCircle(rect_by_75, FX_PI / 4.0f)
                     << " S\nQ\n";
        }

        sColor = GetColorAppStream(crRightBottom, false);
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
                                    CheckStyle nStyle,
                                    const CFX_Color& crText) {
  CFX_FloatRect rcCenter = rcBBox.GetCenterSquare();
  switch (nStyle) {
    default:
    case CheckStyle::kCheck:
      return GetAppStream_Check(rcCenter, crText);
    case CheckStyle::kCircle:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Circle(rcCenter, crText);
    case CheckStyle::kCross:
      return GetAppStream_Cross(rcCenter, crText);
    case CheckStyle::kDiamond:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Diamond(rcCenter, crText);
    case CheckStyle::kSquare:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Square(rcCenter, crText);
    case CheckStyle::kStar:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Star(rcCenter, crText);
  }
}

CFX_ByteString GetRadioButtonAppStream(const CFX_FloatRect& rcBBox,
                                       CheckStyle nStyle,
                                       const CFX_Color& crText) {
  CFX_FloatRect rcCenter = rcBBox.GetCenterSquare();
  switch (nStyle) {
    default:
    case CheckStyle::kCheck:
      return GetAppStream_Check(rcCenter, crText);
    case CheckStyle::kCircle:
      rcCenter.Scale(1.0f / 2.0f);
      return GetAppStream_Circle(rcCenter, crText);
    case CheckStyle::kCross:
      return GetAppStream_Cross(rcCenter, crText);
    case CheckStyle::kDiamond:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Diamond(rcCenter, crText);
    case CheckStyle::kSquare:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Square(rcCenter, crText);
    case CheckStyle::kStar:
      rcCenter.Scale(2.0f / 3.0f);
      return GetAppStream_Star(rcCenter, crText);
  }
}

CFX_ByteString GetFontSetString(IPVT_FontMap* pFontMap,
                                int32_t nFontIndex,
                                float fFontSize) {
  if (!pFontMap)
    return CFX_ByteString();

  CFX_ByteString sFontAlias = pFontMap->GetPDFFontAlias(nFontIndex);
  if (sFontAlias.GetLength() <= 0 || fFontSize <= 0)
    return CFX_ByteString();

  std::ostringstream sRet;
  sRet << "/" << sFontAlias << " " << fFontSize << " Tf\n";
  return CFX_ByteString(sRet);
}

CFX_ByteString GetWordRenderString(const CFX_ByteString& strWords) {
  if (strWords.GetLength() > 0)
    return PDF_EncodeString(strWords, false) + " Tj\n";
  return CFX_ByteString();
}

CFX_ByteString GetEditAppStream(CFX_Edit* pEdit,
                                const CFX_PointF& ptOffset,
                                bool bContinuous,
                                uint16_t SubWord) {
  CFX_Edit_Iterator* pIterator = pEdit->GetIterator();
  pIterator->SetAt(0);

  std::ostringstream sEditStream;
  std::ostringstream sWords;
  int32_t nCurFontIndex = -1;
  CFX_PointF ptOld;
  CFX_PointF ptNew;
  CPVT_WordPlace oldplace;

  while (pIterator->NextWord()) {
    CPVT_WordPlace place = pIterator->GetAt();
    if (bContinuous) {
      if (place.LineCmp(oldplace) != 0) {
        if (sWords.tellp() > 0) {
          sEditStream << GetWordRenderString(CFX_ByteString(sWords));
          sWords.str("");
        }

        CPVT_Word word;
        if (pIterator->GetWord(word)) {
          ptNew = CFX_PointF(word.ptWord.x + ptOffset.x,
                             word.ptWord.y + ptOffset.y);
        } else {
          CPVT_Line line;
          pIterator->GetLine(line);
          ptNew = CFX_PointF(line.ptLine.x + ptOffset.x,
                             line.ptLine.y + ptOffset.y);
        }

        if (ptNew.x != ptOld.x || ptNew.y != ptOld.y) {
          sEditStream << ptNew.x - ptOld.x << " " << ptNew.y - ptOld.y
                      << " Td\n";

          ptOld = ptNew;
        }
      }

      CPVT_Word word;
      if (pIterator->GetWord(word)) {
        if (word.nFontIndex != nCurFontIndex) {
          if (sWords.tellp() > 0) {
            sEditStream << GetWordRenderString(CFX_ByteString(sWords));
            sWords.str("");
          }
          sEditStream << GetFontSetString(pEdit->GetFontMap(), word.nFontIndex,
                                          word.fFontSize);
          nCurFontIndex = word.nFontIndex;
        }

        sWords << GetPDFWordString(pEdit->GetFontMap(), nCurFontIndex,
                                   word.Word, SubWord);
      }

      oldplace = place;
    } else {
      CPVT_Word word;
      if (pIterator->GetWord(word)) {
        ptNew =
            CFX_PointF(word.ptWord.x + ptOffset.x, word.ptWord.y + ptOffset.y);

        if (ptNew.x != ptOld.x || ptNew.y != ptOld.y) {
          sEditStream << ptNew.x - ptOld.x << " " << ptNew.y - ptOld.y
                      << " Td\n";
          ptOld = ptNew;
        }

        if (word.nFontIndex != nCurFontIndex) {
          sEditStream << GetFontSetString(pEdit->GetFontMap(), word.nFontIndex,
                                          word.fFontSize);
          nCurFontIndex = word.nFontIndex;
        }

        sEditStream << GetWordRenderString(GetPDFWordString(
            pEdit->GetFontMap(), nCurFontIndex, word.Word, SubWord));
      }
    }
  }

  if (sWords.tellp() > 0) {
    sEditStream << GetWordRenderString(CFX_ByteString(sWords));
    sWords.str("");
  }

  std::ostringstream sAppStream;
  if (sEditStream.tellp() > 0) {
    int32_t nHorzScale = pEdit->GetHorzScale();
    if (nHorzScale != 100) {
      sAppStream << nHorzScale << " Tz\n";
    }

    float fCharSpace = pEdit->GetCharSpace();
    if (!IsFloatZero(fCharSpace)) {
      sAppStream << fCharSpace << " Tc\n";
    }

    sAppStream << sEditStream.str();
  }

  return CFX_ByteString(sAppStream);
}

CFX_ByteString GenerateIconAppStream(CPDF_IconFit& fit,
                                     CPDF_Stream* pIconStream,
                                     const CFX_FloatRect& rcIcon) {
  if (rcIcon.IsEmpty() || !pIconStream)
    return CFX_ByteString();

  CPWL_Icon icon;
  PWL_CREATEPARAM cp;
  cp.dwFlags = PWS_VISIBLE;
  icon.Create(cp);
  icon.SetIconFit(&fit);
  icon.SetPDFStream(pIconStream);
  icon.Move(rcIcon, false, false);

  CFX_ByteString sAlias = icon.GetImageAlias();
  if (sAlias.GetLength() <= 0)
    return CFX_ByteString();

  CFX_FloatRect rcPlate = icon.GetClientRect();
  CFX_Matrix mt = icon.GetImageMatrix().GetInverse();

  float fHScale;
  float fVScale;
  std::tie(fHScale, fVScale) = icon.GetScale();

  float fx;
  float fy;
  std::tie(fx, fy) = icon.GetImageOffset();

  std::ostringstream str;
  str << "q\n";
  str << rcPlate.left << " " << rcPlate.bottom << " "
      << rcPlate.right - rcPlate.left << " " << rcPlate.top - rcPlate.bottom
      << " re W n\n";

  str << fHScale << " 0 0 " << fVScale << " " << rcPlate.left + fx << " "
      << rcPlate.bottom + fy << " cm\n";
  str << mt.a << " " << mt.b << " " << mt.c << " " << mt.d << " " << mt.e << " "
      << mt.f << " cm\n";

  str << "0 g 0 G 1 w /" << sAlias << " Do\n"
      << "Q\n";
  icon.Destroy();

  return CFX_ByteString(str);
}

CFX_ByteString GetPushButtonAppStream(const CFX_FloatRect& rcBBox,
                                      IPVT_FontMap* pFontMap,
                                      CPDF_Stream* pIconStream,
                                      CPDF_IconFit& IconFit,
                                      const CFX_WideString& sLabel,
                                      const CFX_Color& crText,
                                      float fFontSize,
                                      ButtonStyle nLayOut) {
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
  CFX_FloatRect rcLabel;
  CFX_FloatRect rcIcon;
  float fWidth = 0.0f;
  float fHeight = 0.0f;

  switch (nLayOut) {
    case ButtonStyle::kLabel:
      rcLabel = rcBBox;
      break;
    case ButtonStyle::kIcon:
      rcIcon = rcBBox;
      break;
    case ButtonStyle::kIconTopLabelBottom:
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
    case ButtonStyle::kIconBottomLabelTop:
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
    case ButtonStyle::kIconLeftLabelRight:
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
    case ButtonStyle::kIconRightLabelLeft:
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
    case ButtonStyle::kLabelOverIcon:
      rcLabel = rcBBox;
      rcIcon = rcBBox;
      break;
  }

  std::ostringstream sTemp;
  sTemp << GenerateIconAppStream(IconFit, pIconStream, rcIcon);

  if (!rcLabel.IsEmpty()) {
    pEdit->SetPlateRect(rcLabel);
    CFX_ByteString sEdit =
        GetEditAppStream(pEdit.get(), CFX_PointF(0.0f, 0.0f), true, 0);
    if (sEdit.GetLength() > 0)
      sTemp << "BT\n" << GetColorAppStream(crText, true) << sEdit << "ET\n";
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

CFX_ByteString GetBorderAppStreamInternal(const CFX_FloatRect& rect,
                                          float fWidth,
                                          const CFX_Color& color,
                                          const CFX_Color& crLeftTop,
                                          const CFX_Color& crRightBottom,
                                          BorderStyle nStyle,
                                          const CPWL_Dash& dash) {
  std::ostringstream sAppStream;
  CFX_ByteString sColor;

  float fLeft = rect.left;
  float fRight = rect.right;
  float fTop = rect.top;
  float fBottom = rect.bottom;

  if (fWidth > 0.0f) {
    float fHalfWidth = fWidth / 2.0f;

    sAppStream << "q\n";

    switch (nStyle) {
      default:
      case BorderStyle::SOLID:
        sColor = GetColorAppStream(color, true);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fLeft << " " << fBottom << " " << fRight - fLeft << " "
                     << fTop - fBottom << " re\n";
          sAppStream << fLeft + fWidth << " " << fBottom + fWidth << " "
                     << fRight - fLeft - fWidth * 2 << " "
                     << fTop - fBottom - fWidth * 2 << " re\n";
          sAppStream << "f*\n";
        }
        break;
      case BorderStyle::DASH:
        sColor = GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fWidth << " w"
                     << " [" << dash.nDash << " " << dash.nGap << "] "
                     << dash.nPhase << " d\n";
          sAppStream << fLeft + fWidth / 2 << " " << fBottom + fWidth / 2
                     << " m\n";
          sAppStream << fLeft + fWidth / 2 << " " << fTop - fWidth / 2
                     << " l\n";
          sAppStream << fRight - fWidth / 2 << " " << fTop - fWidth / 2
                     << " l\n";
          sAppStream << fRight - fWidth / 2 << " " << fBottom + fWidth / 2
                     << " l\n";
          sAppStream << fLeft + fWidth / 2 << " " << fBottom + fWidth / 2
                     << " l S\n";
        }
        break;
      case BorderStyle::BEVELED:
      case BorderStyle::INSET:
        sColor = GetColorAppStream(crLeftTop, true);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fLeft + fHalfWidth << " " << fBottom + fHalfWidth
                     << " m\n";
          sAppStream << fLeft + fHalfWidth << " " << fTop - fHalfWidth
                     << " l\n";
          sAppStream << fRight - fHalfWidth << " " << fTop - fHalfWidth
                     << " l\n";
          sAppStream << fRight - fHalfWidth * 2 << " " << fTop - fHalfWidth * 2
                     << " l\n";
          sAppStream << fLeft + fHalfWidth * 2 << " " << fTop - fHalfWidth * 2
                     << " l\n";
          sAppStream << fLeft + fHalfWidth * 2 << " "
                     << fBottom + fHalfWidth * 2 << " l f\n";
        }

        sColor = GetColorAppStream(crRightBottom, true);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fRight - fHalfWidth << " " << fTop - fHalfWidth
                     << " m\n";
          sAppStream << fRight - fHalfWidth << " " << fBottom + fHalfWidth
                     << " l\n";
          sAppStream << fLeft + fHalfWidth << " " << fBottom + fHalfWidth
                     << " l\n";
          sAppStream << fLeft + fHalfWidth * 2 << " "
                     << fBottom + fHalfWidth * 2 << " l\n";
          sAppStream << fRight - fHalfWidth * 2 << " "
                     << fBottom + fHalfWidth * 2 << " l\n";
          sAppStream << fRight - fHalfWidth * 2 << " " << fTop - fHalfWidth * 2
                     << " l f\n";
        }

        sColor = GetColorAppStream(color, true);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fLeft << " " << fBottom << " " << fRight - fLeft << " "
                     << fTop - fBottom << " re\n";
          sAppStream << fLeft + fHalfWidth << " " << fBottom + fHalfWidth << " "
                     << fRight - fLeft - fHalfWidth * 2 << " "
                     << fTop - fBottom - fHalfWidth * 2 << " re f*\n";
        }
        break;
      case BorderStyle::UNDERLINE:
        sColor = GetColorAppStream(color, false);
        if (sColor.GetLength() > 0) {
          sAppStream << sColor;
          sAppStream << fWidth << " w\n";
          sAppStream << fLeft << " " << fBottom + fWidth / 2 << " m\n";
          sAppStream << fRight << " " << fBottom + fWidth / 2 << " l S\n";
        }
        break;
    }

    sAppStream << "Q\n";
  }

  return CFX_ByteString(sAppStream);
}

CFX_ByteString GetDropButtonAppStream(const CFX_FloatRect& rcBBox) {
  if (rcBBox.IsEmpty())
    return CFX_ByteString();

  std::ostringstream sAppStream;
  sAppStream << "q\n"
             << GetColorAppStream(CFX_Color(COLORTYPE_RGB, 220.0f / 255.0f,
                                            220.0f / 255.0f, 220.0f / 255.0f),
                                  true)
             << rcBBox.left << " " << rcBBox.bottom << " "
             << rcBBox.right - rcBBox.left << " " << rcBBox.top - rcBBox.bottom
             << " re f\n"
             << "Q\n";

  sAppStream << "q\n"
             << GetBorderAppStreamInternal(
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

CFX_ByteString GetRectFillAppStream(const CFX_FloatRect& rect,
                                    const CFX_Color& color) {
  std::ostringstream sAppStream;
  CFX_ByteString sColor = GetColorAppStream(color, true);
  if (sColor.GetLength() > 0) {
    sAppStream << "q\n" << sColor;
    sAppStream << rect.left << " " << rect.bottom << " "
               << rect.right - rect.left << " " << rect.top - rect.bottom
               << " re f\nQ\n";
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
  ButtonStyle nLayout = ButtonStyle::kLabel;
  switch (pControl->GetTextPosition()) {
    case TEXTPOS_ICON:
      nLayout = ButtonStyle::kIcon;
      break;
    case TEXTPOS_BELOW:
      nLayout = ButtonStyle::kIconTopLabelBottom;
      break;
    case TEXTPOS_ABOVE:
      nLayout = ButtonStyle::kIconBottomLabelTop;
      break;
    case TEXTPOS_RIGHT:
      nLayout = ButtonStyle::kIconLeftLabelRight;
      break;
    case TEXTPOS_LEFT:
      nLayout = ButtonStyle::kIconRightLabelLeft;
      break;
    case TEXTPOS_OVERLAID:
      nLayout = ButtonStyle::kLabelOverIcon;
      break;
    default:
      nLayout = ButtonStyle::kLabel;
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
      GetRectFillAppStream(rcWindow, crBackground) +
      GetBorderAppStreamInternal(rcWindow, fBorderWidth, crBorder, crLeftTop,
                                 crRightBottom, nBorderStyle, dsBorder) +
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
        GetRectFillAppStream(rcWindow, crBackground) +
        GetBorderAppStreamInternal(rcWindow, fBorderWidth, crBorder, crLeftTop,
                                   crRightBottom, nBorderStyle, dsBorder) +
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

    csAP =
        GetRectFillAppStream(rcWindow, crBackground - 0.25f) +
        GetBorderAppStreamInternal(rcWindow, fBorderWidth, crBorder, crLeftTop,
                                   crRightBottom, nBorderStyle, dsBorder) +
        GetPushButtonAppStream(iconFit.GetFittingBounds() ? rcWindow : rcClient,
                               &font_map, pDownIcon, iconFit, csDownCaption,
                               crText, fFontSize, nLayout);

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

  CheckStyle nStyle = CheckStyle::kCheck;
  CFX_WideString csWCaption = pControl->GetNormalCaption();
  if (csWCaption.GetLength() > 0) {
    switch (csWCaption[0]) {
      case L'l':
        nStyle = CheckStyle::kCircle;
        break;
      case L'8':
        nStyle = CheckStyle::kCross;
        break;
      case L'u':
        nStyle = CheckStyle::kDiamond;
        break;
      case L'n':
        nStyle = CheckStyle::kSquare;
        break;
      case L'H':
        nStyle = CheckStyle::kStar;
        break;
      case L'4':
      default:
        nStyle = CheckStyle::kCheck;
    }
  }

  CFX_ByteString csAP_N_ON =
      GetRectFillAppStream(rcWindow, crBackground) +
      GetBorderAppStreamInternal(rcWindow, fBorderWidth, crBorder, crLeftTop,
                                 crRightBottom, nBorderStyle, dsBorder);

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
      GetRectFillAppStream(rcWindow, crBackground - 0.25f) +
      GetBorderAppStreamInternal(rcWindow, fBorderWidth, crBorder, crLeftTop,
                                 crRightBottom, nBorderStyle, dsBorder);

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

  CheckStyle nStyle = CheckStyle::kCircle;
  CFX_WideString csWCaption = pControl->GetNormalCaption();
  if (csWCaption.GetLength() > 0) {
    switch (csWCaption[0]) {
      case L'8':
        nStyle = CheckStyle::kCross;
        break;
      case L'u':
        nStyle = CheckStyle::kDiamond;
        break;
      case L'n':
        nStyle = CheckStyle::kSquare;
        break;
      case L'H':
        nStyle = CheckStyle::kStar;
        break;
      case L'4':
        nStyle = CheckStyle::kCheck;
        break;
      case L'l':
      default:
        nStyle = CheckStyle::kCircle;
    }
  }

  CFX_ByteString csAP_N_ON;
  CFX_FloatRect rcCenter = rcWindow.GetCenterSquare().GetDeflated(1.0f, 1.0f);
  if (nStyle == CheckStyle::kCircle) {
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
    csAP_N_ON =
        GetRectFillAppStream(rcWindow, crBackground) +
        GetBorderAppStreamInternal(rcWindow, fBorderWidth, crBorder, crLeftTop,
                                   crRightBottom, nBorderStyle, dsBorder);
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

  if (nStyle == CheckStyle::kCircle) {
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
        GetRectFillAppStream(rcWindow, crBackground - 0.25f) +
        GetBorderAppStreamInternal(rcWindow, fBorderWidth, crBorder, crLeftTop,
                                   crRightBottom, nBorderStyle, dsBorder);
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
  CFX_ByteString sEdit = GetEditAppStream(pEdit.get(), CFX_PointF(), true, 0);
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
          << GetColorAppStream(crText, true) << sEdit << "ET\n"
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
            << GetColorAppStream(
                   CFX_Color(COLORTYPE_RGB, 0, 51.0f / 255.0f, 113.0f / 255.0f),
                   true)
            << rcItem.left << " " << rcItem.bottom << " " << rcItem.Width()
            << " " << rcItem.Height() << " re f\n"
            << "Q\n";

      sList << "BT\n"
            << GetColorAppStream(CFX_Color(COLORTYPE_GRAY, 1), true)
            << GetEditAppStream(pEdit.get(), CFX_PointF(0.0f, fy), true, 0)
            << "ET\n";
    } else {
      CFX_Color crText = widget_->GetTextPWLColor();
      sList << "BT\n"
            << GetColorAppStream(crText, true)
            << GetEditAppStream(pEdit.get(), CFX_PointF(0.0f, fy), true, 0)
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
  CFX_ByteString sEdit =
      GetEditAppStream(pEdit.get(), CFX_PointF(), !bCharArray, subWord);

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
          << GetColorAppStream(crText, true) << sEdit << "ET\n"
          << "Q\nEMC\n";
  }

  if (bCharArray) {
    switch (widget_->GetBorderStyle()) {
      case BorderStyle::SOLID: {
        CFX_ByteString sColor =
            GetColorAppStream(widget_->GetBorderPWLColor(), false);
        if (sColor.GetLength() > 0) {
          sLines << "q\n"
                 << widget_->GetBorderWidth() << " w\n"
                 << GetColorAppStream(widget_->GetBorderPWLColor(), false)
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
            GetColorAppStream(widget_->GetBorderPWLColor(), false);
        if (sColor.GetLength() > 0) {
          CPWL_Dash dsBorder = CPWL_Dash(3, 3, 0);

          sLines << "q\n"
                 << widget_->GetBorderWidth() << " w\n"
                 << GetColorAppStream(widget_->GetBorderPWLColor(), false)
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
    return GetRectFillAppStream(widget_->GetRotatedRect(), crBackground);

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

  return GetBorderAppStreamInternal(rcWindow, fBorderWidth, crBorder, crLeftTop,
                                    crRightBottom, nBorderStyle, dsBorder);
}
