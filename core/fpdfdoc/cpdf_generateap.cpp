// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_generateap.h"

#include <algorithm>
#include <sstream>
#include <utility>

#include "constants/annotation_common.h"
#include "constants/appearance.h"
#include "constants/font_encodings.h"
#include "constants/form_fields.h"
#include "core/fpdfapi/edit/cpdf_contentstream_write_utils.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_boolean.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpdf_color_utils.h"
#include "core/fpdfdoc/cpdf_defaultappearance.h"
#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fpdfdoc/cpvt_fontmap.h"
#include "core/fpdfdoc/cpvt_variabletext.h"
#include "core/fpdfdoc/cpvt_word.h"
#include "core/fxcrt/fx_string_wrappers.h"
#include "core/fxcrt/notreached.h"
#include "core/fxge/cfx_renderdevice.h"

namespace {

struct CPVT_Dash {
  CPVT_Dash(int32_t dash, int32_t gap, int32_t phase)
      : dash(dash), gap(gap), phase(phase) {}

  int32_t dash;
  int32_t gap;
  int32_t phase;
};

enum class PaintOperation { kStroke, kFill };

ByteString GetPDFWordString(IPVT_FontMap* pFontMap,
                            int32_t font_index,
                            uint16_t word,
                            uint16_t sub_word) {
  if (sub_word > 0) {
    return ByteString::Format("%c", sub_word);
  }

  if (!pFontMap)
    return ByteString();

  RetainPtr<CPDF_Font> pPDFFont = pFontMap->GetPDFFont(font_index);
  if (!pPDFFont)
    return ByteString();

  if (pPDFFont->GetBaseFontName() == "Symbol" ||
      pPDFFont->GetBaseFontName() == "ZapfDingbats") {
    return ByteString::Format("%c", word);
  }

  ByteString word_string;
  uint32_t char_code = pPDFFont->CharCodeFromUnicode(word);
  if (char_code != CPDF_Font::kInvalidCharCode) {
    pPDFFont->AppendChar(&word_string, char_code);
  }
  return word_string;
}

ByteString GetWordRenderString(ByteStringView words) {
  if (words.IsEmpty()) {
    return ByteString();
  }
  return PDF_EncodeString(words) + " Tj\n";
}

ByteString GetFontSetString(IPVT_FontMap* pFontMap,
                            int32_t font_index,
                            float font_size) {
  fxcrt::ostringstream font_stream;
  if (pFontMap) {
    ByteString font_alias = pFontMap->GetPDFFontAlias(font_index);
    if (font_alias.GetLength() > 0 && font_size > 0) {
      font_stream << "/" << font_alias << " ";
      WriteFloat(font_stream, font_size) << " Tf\n";
    }
  }
  return ByteString(font_stream);
}

ByteString GenerateEditAP(IPVT_FontMap* pFontMap,
                          CPVT_VariableText::Iterator* pIterator,
                          const CFX_PointF& offset,
                          bool continuous,
                          uint16_t sub_word) {
  fxcrt::ostringstream edit_stream;
  fxcrt::ostringstream line_stream;
  CFX_PointF old_point;
  CFX_PointF new_point;
  int32_t current_font_index = -1;
  CPVT_WordPlace oldplace;
  ByteString words;
  pIterator->SetAt(0);
  while (pIterator->NextWord()) {
    CPVT_WordPlace place = pIterator->GetWordPlace();
    if (continuous) {
      if (place.LineCmp(oldplace) != 0) {
        if (!words.IsEmpty()) {
          line_stream << GetWordRenderString(words.AsStringView());
          edit_stream << line_stream.str();
          line_stream.str("");
          words.clear();
        }
        CPVT_Word word;
        if (pIterator->GetWord(word)) {
          new_point =
              CFX_PointF(word.ptWord.x + offset.x, word.ptWord.y + offset.y);
        } else {
          CPVT_Line line;
          pIterator->GetLine(line);
          new_point =
              CFX_PointF(line.ptLine.x + offset.x, line.ptLine.y + offset.y);
        }
        if (new_point != old_point) {
          WritePoint(line_stream, new_point - old_point) << " Td\n";
          old_point = new_point;
        }
      }
      CPVT_Word word;
      if (pIterator->GetWord(word)) {
        if (word.nFontIndex != current_font_index) {
          if (!words.IsEmpty()) {
            line_stream << GetWordRenderString(words.AsStringView());
            words.clear();
          }
          line_stream << GetFontSetString(pFontMap, word.nFontIndex,
                                          word.fFontSize);
          current_font_index = word.nFontIndex;
        }
        words +=
            GetPDFWordString(pFontMap, current_font_index, word.Word, sub_word);
      }
      oldplace = place;
    } else {
      CPVT_Word word;
      if (pIterator->GetWord(word)) {
        new_point =
            CFX_PointF(word.ptWord.x + offset.x, word.ptWord.y + offset.y);
        if (new_point != old_point) {
          WritePoint(edit_stream, new_point - old_point) << " Td\n";
          old_point = new_point;
        }
        if (word.nFontIndex != current_font_index) {
          edit_stream << GetFontSetString(pFontMap, word.nFontIndex,
                                          word.fFontSize);
          current_font_index = word.nFontIndex;
        }
        edit_stream << GetWordRenderString(
            GetPDFWordString(pFontMap, current_font_index, word.Word, sub_word)
                .AsStringView());
      }
    }
  }
  if (!words.IsEmpty()) {
    line_stream << GetWordRenderString(words.AsStringView());
    edit_stream << line_stream.str();
  }
  return ByteString(edit_stream);
}

ByteString GenerateColorAP(const CFX_Color& color, PaintOperation nOperation) {
  fxcrt::ostringstream color_stream;
  switch (color.nColorType) {
    case CFX_Color::Type::kRGB:
      WriteFloat(color_stream, color.fColor1) << " ";
      WriteFloat(color_stream, color.fColor2) << " ";
      WriteFloat(color_stream, color.fColor3) << " ";
      color_stream << (nOperation == PaintOperation::kStroke ? "RG" : "rg")
                   << "\n";
      return ByteString(color_stream);
    case CFX_Color::Type::kGray:
      WriteFloat(color_stream, color.fColor1) << " ";
      color_stream << (nOperation == PaintOperation::kStroke ? "G" : "g")
                   << "\n";
      return ByteString(color_stream);
    case CFX_Color::Type::kCMYK:
      WriteFloat(color_stream, color.fColor1) << " ";
      WriteFloat(color_stream, color.fColor2) << " ";
      WriteFloat(color_stream, color.fColor3) << " ";
      WriteFloat(color_stream, color.fColor4) << " ";
      color_stream << (nOperation == PaintOperation::kStroke ? "K" : "k")
                   << "\n";
      return ByteString(color_stream);
    case CFX_Color::Type::kTransparent:
      return ByteString();
  }
  NOTREACHED();
}

ByteString GenerateBorderAP(const CFX_FloatRect& rect,
                            float width,
                            const CFX_Color& border_color,
                            const CFX_Color& left_top_color,
                            const CFX_Color& right_bottom_color,
                            BorderStyle style,
                            const CPVT_Dash& dash) {
  if (width <= 0) {
    return ByteString();
  }

  fxcrt::ostringstream app_stream;
  const float left = rect.left;
  const float bottom = rect.bottom;
  const float right = rect.right;
  const float top = rect.top;
  const float half_width = width / 2.0f;
  switch (style) {
    case BorderStyle::kSolid: {
      ByteString color_string =
          GenerateColorAP(border_color, PaintOperation::kFill);
      if (color_string.GetLength() > 0) {
        app_stream << color_string;
        WriteRect(app_stream, rect) << " re\n";
        CFX_FloatRect inner_rect = rect;
        inner_rect.Deflate(width, width);
        WriteRect(app_stream, inner_rect) << " re f*\n";
      }
      return ByteString(app_stream);
    }
    case BorderStyle::kDash: {
      ByteString color_string =
          GenerateColorAP(border_color, PaintOperation::kStroke);
      if (color_string.GetLength() > 0) {
        app_stream << color_string;
        WriteFloat(app_stream, width) << " w [" << dash.dash << " " << dash.gap
                                      << "] " << dash.phase << " d\n";
        WritePoint(app_stream, {left + half_width, bottom + half_width})
            << " m\n";
        WritePoint(app_stream, {left + half_width, top - half_width}) << " l\n";
        WritePoint(app_stream, {right - half_width, top - half_width})
            << " l\n";
        WritePoint(app_stream, {right - half_width, bottom + half_width})
            << " l\n";
        WritePoint(app_stream, {left + half_width, bottom + half_width})
            << " l S\n";
      }
      return ByteString(app_stream);
    }
    case BorderStyle::kBeveled:
    case BorderStyle::kInset: {
      ByteString color_string =
          GenerateColorAP(left_top_color, PaintOperation::kFill);
      if (color_string.GetLength() > 0) {
        app_stream << color_string;
        WritePoint(app_stream, {left + half_width, bottom + half_width})
            << " m\n";
        WritePoint(app_stream, {left + half_width, top - half_width}) << " l\n";
        WritePoint(app_stream, {right - half_width, top - half_width})
            << " l\n";
        WritePoint(app_stream, {right - width, top - width}) << " l\n";
        WritePoint(app_stream, {left + width, top - width}) << " l\n";
        WritePoint(app_stream, {left + width, bottom + width}) << " l f\n";
      }
      color_string = GenerateColorAP(right_bottom_color, PaintOperation::kFill);
      if (color_string.GetLength() > 0) {
        app_stream << color_string;
        WritePoint(app_stream, {right - half_width, top - half_width})
            << " m\n";
        WritePoint(app_stream, {right - half_width, bottom + half_width})
            << " l\n";
        WritePoint(app_stream, {left + half_width, bottom + half_width})
            << " l\n";
        WritePoint(app_stream, {left + width, bottom + width}) << " l\n";
        WritePoint(app_stream, {right - width, bottom + width}) << " l\n";
        WritePoint(app_stream, {right - width, top - width}) << " l f\n";
      }
      color_string = GenerateColorAP(border_color, PaintOperation::kFill);
      if (color_string.GetLength() > 0) {
        app_stream << color_string;
        WriteRect(app_stream, rect) << " re\n";
        CFX_FloatRect inner_rect = rect;
        inner_rect.Deflate(half_width, half_width);
        WriteRect(app_stream, inner_rect) << " re f*\n";
      }
      return ByteString(app_stream);
    }
    case BorderStyle::kUnderline: {
      ByteString color_string =
          GenerateColorAP(border_color, PaintOperation::kStroke);
      if (color_string.GetLength() > 0) {
        app_stream << color_string;
        WriteFloat(app_stream, width) << " w\n";
        WritePoint(app_stream, {left, bottom + half_width}) << " m\n";
        WritePoint(app_stream, {right, bottom + half_width}) << " l S\n";
      }
      return ByteString(app_stream);
    }
  }
  NOTREACHED();
}

ByteString GetColorStringWithDefault(const CPDF_Array* pColor,
                                     const CFX_Color& default_color,
                                     PaintOperation nOperation) {
  if (pColor) {
    CFX_Color color = fpdfdoc::CFXColorFromArray(*pColor);
    return GenerateColorAP(color, nOperation);
  }

  return GenerateColorAP(default_color, nOperation);
}

float GetBorderWidth(const CPDF_Dictionary* pDict) {
  RetainPtr<const CPDF_Dictionary> pBorderStyleDict = pDict->GetDictFor("BS");
  if (pBorderStyleDict && pBorderStyleDict->KeyExist("W"))
    return pBorderStyleDict->GetFloatFor("W");

  auto pBorderArray = pDict->GetArrayFor(pdfium::annotation::kBorder);
  if (pBorderArray && pBorderArray->size() > 2)
    return pBorderArray->GetFloatAt(2);

  return 1;
}

RetainPtr<const CPDF_Array> GetDashArray(const CPDF_Dictionary* pDict) {
  RetainPtr<const CPDF_Dictionary> pBorderStyleDict = pDict->GetDictFor("BS");
  if (pBorderStyleDict && pBorderStyleDict->GetByteStringFor("S") == "D")
    return pBorderStyleDict->GetArrayFor("D");

  RetainPtr<const CPDF_Array> pBorderArray =
      pDict->GetArrayFor(pdfium::annotation::kBorder);
  if (pBorderArray && pBorderArray->size() == 4)
    return pBorderArray->GetArrayAt(3);

  return nullptr;
}

ByteString GetDashPatternString(const CPDF_Dictionary* pDict) {
  RetainPtr<const CPDF_Array> pDashArray = GetDashArray(pDict);
  if (!pDashArray || pDashArray->IsEmpty())
    return ByteString();

  // Support maximum of ten elements in the dash array.
  size_t pDashArrayCount = std::min<size_t>(pDashArray->size(), 10);
  fxcrt::ostringstream dash_stream;

  dash_stream << "[";
  for (size_t i = 0; i < pDashArrayCount; ++i)
    WriteFloat(dash_stream, pDashArray->GetFloatAt(i)) << " ";
  dash_stream << "] 0 d\n";

  return ByteString(dash_stream);
}

ByteString GetPopupContentsString(CPDF_Document* pDoc,
                                  const CPDF_Dictionary& pAnnotDict,
                                  RetainPtr<CPDF_Font> pDefFont,
                                  const ByteString& sFontName) {
  WideString value(pAnnotDict.GetUnicodeTextFor(pdfium::form_fields::kT));
  value += L'\n';
  value += pAnnotDict.GetUnicodeTextFor(pdfium::annotation::kContents);

  CPVT_FontMap map(pDoc, nullptr, std::move(pDefFont), sFontName);
  CPVT_VariableText::Provider prd(&map);
  CPVT_VariableText vt(&prd);
  vt.SetPlateRect(pAnnotDict.GetRectFor(pdfium::annotation::kRect));
  vt.SetFontSize(12);
  vt.SetAutoReturn(true);
  vt.SetMultiLine(true);
  vt.Initialize();
  vt.SetText(value);
  vt.RearrangeAll();

  CFX_PointF offset(3.0f, -3.0f);
  ByteString content = GenerateEditAP(&map, vt.GetIterator(), offset, false, 0);

  if (content.IsEmpty()) {
    return ByteString();
  }

  ByteString color = GenerateColorAP(CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0),
                                     PaintOperation::kFill);

  return ByteString{"BT\n", color.AsStringView(), content.AsStringView(),
                    "ET\n", "Q\n"};
}

RetainPtr<CPDF_Dictionary> GenerateFallbackFontDict(CPDF_Document* doc) {
  auto font_dict = doc->NewIndirect<CPDF_Dictionary>();
  font_dict->SetNewFor<CPDF_Name>("Type", "Font");
  font_dict->SetNewFor<CPDF_Name>("Subtype", "Type1");
  font_dict->SetNewFor<CPDF_Name>("BaseFont", CFX_Font::kDefaultAnsiFontName);
  font_dict->SetNewFor<CPDF_Name>("Encoding",
                                  pdfium::font_encodings::kWinAnsiEncoding);
  return font_dict;
}

RetainPtr<CPDF_Dictionary> GenerateResourceFontDict(
    CPDF_Document* doc,
    const ByteString& font_name,
    uint32_t font_dict_obj_num) {
  auto resource_font_dict = doc->New<CPDF_Dictionary>();
  resource_font_dict->SetNewFor<CPDF_Reference>(font_name, doc,
                                                font_dict_obj_num);
  return resource_font_dict;
}

ByteString GetPaintOperatorString(bool is_stroke_rect, bool is_fill_rect) {
  if (is_stroke_rect) {
    return is_fill_rect ? "b" : "s";
  }
  return is_fill_rect ? "f" : "n";
}

ByteString GenerateTextSymbolAP(const CFX_FloatRect& rect) {
  fxcrt::ostringstream app_stream;
  app_stream << GenerateColorAP(CFX_Color(CFX_Color::Type::kRGB, 1, 1, 0),
                                PaintOperation::kFill);
  app_stream << GenerateColorAP(CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0),
                                PaintOperation::kStroke);

  static constexpr int kBorderWidth = 1;
  app_stream << kBorderWidth << " w\n";

  static constexpr float kHalfWidth = kBorderWidth / 2.0f;
  static constexpr int kTipDelta = 4;

  CFX_FloatRect outerRect1 = rect;
  outerRect1.Deflate(kHalfWidth, kHalfWidth);
  outerRect1.bottom += kTipDelta;

  CFX_FloatRect outerRect2 = outerRect1;
  outerRect2.left += kTipDelta;
  outerRect2.right = outerRect2.left + kTipDelta;
  outerRect2.top = outerRect2.bottom - kTipDelta;
  float outerRect2Middle = (outerRect2.left + outerRect2.right) / 2;

  // Draw outer boxes.
  WritePoint(app_stream, {outerRect1.left, outerRect1.bottom}) << " m\n";
  WritePoint(app_stream, {outerRect1.left, outerRect1.top}) << " l\n";
  WritePoint(app_stream, {outerRect1.right, outerRect1.top}) << " l\n";
  WritePoint(app_stream, {outerRect1.right, outerRect1.bottom}) << " l\n";
  WritePoint(app_stream, {outerRect2.right, outerRect2.bottom}) << " l\n";
  WritePoint(app_stream, {outerRect2Middle, outerRect2.top}) << " l\n";
  WritePoint(app_stream, {outerRect2.left, outerRect2.bottom}) << " l\n";
  WritePoint(app_stream, {outerRect1.left, outerRect1.bottom}) << " l\n";

  // Draw inner lines.
  CFX_FloatRect lineRect = outerRect1;
  const float delta_x = 2;
  const float delta_y = (lineRect.top - lineRect.bottom) / 4;

  lineRect.left += delta_x;
  lineRect.right -= delta_x;
  for (int i = 0; i < 3; ++i) {
    lineRect.top -= delta_y;
    WritePoint(app_stream, {lineRect.left, lineRect.top}) << " m\n";
    WritePoint(app_stream, {lineRect.right, lineRect.top}) << " l\n";
  }
  app_stream << "B*\n";

  return ByteString(app_stream);
}

RetainPtr<CPDF_Dictionary> GenerateExtGStateDict(
    const CPDF_Dictionary& pAnnotDict,
    const ByteString& sExtGSDictName,
    const ByteString& blend_mode) {
  auto pGSDict =
      pdfium::MakeRetain<CPDF_Dictionary>(pAnnotDict.GetByteStringPool());
  pGSDict->SetNewFor<CPDF_Name>("Type", "ExtGState");

  float opacity = pAnnotDict.KeyExist("CA") ? pAnnotDict.GetFloatFor("CA") : 1;
  pGSDict->SetNewFor<CPDF_Number>("CA", opacity);
  pGSDict->SetNewFor<CPDF_Number>("ca", opacity);
  pGSDict->SetNewFor<CPDF_Boolean>("AIS", false);
  pGSDict->SetNewFor<CPDF_Name>("BM", blend_mode);

  auto pExtGStateDict =
      pdfium::MakeRetain<CPDF_Dictionary>(pAnnotDict.GetByteStringPool());
  pExtGStateDict->SetFor(sExtGSDictName, pGSDict);
  return pExtGStateDict;
}

RetainPtr<CPDF_Dictionary> GenerateResourceDict(
    CPDF_Document* pDoc,
    RetainPtr<CPDF_Dictionary> pExtGStateDict,
    RetainPtr<CPDF_Dictionary> pResourceFontDict) {
  auto pResourceDict = pDoc->New<CPDF_Dictionary>();
  if (pExtGStateDict)
    pResourceDict->SetFor("ExtGState", pExtGStateDict);
  if (pResourceFontDict)
    pResourceDict->SetFor("Font", pResourceFontDict);
  return pResourceDict;
}

void GenerateAndSetAPDict(CPDF_Document* doc,
                          CPDF_Dictionary* annot_dict,
                          fxcrt::ostringstream* app_stream,
                          RetainPtr<CPDF_Dictionary> resource_dict,
                          bool is_text_markup_annotation) {
  auto stream_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  stream_dict->SetNewFor<CPDF_Number>("FormType", 1);
  stream_dict->SetNewFor<CPDF_Name>("Type", "XObject");
  stream_dict->SetNewFor<CPDF_Name>("Subtype", "Form");
  stream_dict->SetMatrixFor("Matrix", CFX_Matrix());

  CFX_FloatRect rect = is_text_markup_annotation
                           ? CPDF_Annot::BoundingRectFromQuadPoints(annot_dict)
                           : annot_dict->GetRectFor(pdfium::annotation::kRect);
  stream_dict->SetRectFor("BBox", rect);
  stream_dict->SetFor("Resources", std::move(resource_dict));

  auto normal_stream = doc->NewIndirect<CPDF_Stream>(std::move(stream_dict));
  normal_stream->SetDataFromStringstream(app_stream);

  RetainPtr<CPDF_Dictionary> ap_dict =
      annot_dict->GetOrCreateDictFor(pdfium::annotation::kAP);
  ap_dict->SetNewFor<CPDF_Reference>("N", doc, normal_stream->GetObjNum());
}

bool GenerateCircleAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict) {
  fxcrt::ostringstream app_stream;
  ByteString sExtGSDictName = "GS";
  app_stream << "/" << sExtGSDictName << " gs ";

  RetainPtr<const CPDF_Array> pInteriorColor = pAnnotDict->GetArrayFor("IC");
  app_stream << GetColorStringWithDefault(
      pInteriorColor.Get(), CFX_Color(CFX_Color::Type::kTransparent),
      PaintOperation::kFill);

  app_stream << GetColorStringWithDefault(
      pAnnotDict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  float border_width = GetBorderWidth(pAnnotDict);
  bool is_stroke_rect = border_width > 0;

  if (is_stroke_rect) {
    app_stream << border_width << " w ";
    app_stream << GetDashPatternString(pAnnotDict);
  }

  CFX_FloatRect rect = pAnnotDict->GetRectFor(pdfium::annotation::kRect);
  rect.Normalize();

  if (is_stroke_rect) {
    // Deflating rect because stroking a path entails painting all points
    // whose perpendicular distance from the path in user space is less than
    // or equal to half the line width.
    rect.Deflate(border_width / 2, border_width / 2);
  }

  const float middle_x = (rect.left + rect.right) / 2;
  const float middle_y = (rect.top + rect.bottom) / 2;

  // `kL` is precalculated approximate value of 4 * tan((3.14 / 2) / 4) / 3,
  // where `kL` * radius is a good approximation of control points for
  // arc with 90 degrees.
  static constexpr float kL = 0.5523f;
  const float delta_x = kL * rect.Width() / 2.0;
  const float delta_y = kL * rect.Height() / 2.0;

  // Starting point
  app_stream << middle_x << " " << rect.top << " m\n";
  // First Bezier Curve
  app_stream << middle_x + delta_x << " " << rect.top << " " << rect.right
             << " " << middle_y + delta_y << " " << rect.right << " "
             << middle_y << " c\n";
  // Second Bezier Curve
  app_stream << rect.right << " " << middle_y - delta_y << " "
             << middle_x + delta_x << " " << rect.bottom << " " << middle_x
             << " " << rect.bottom << " c\n";
  // Third Bezier Curve
  app_stream << middle_x - delta_x << " " << rect.bottom << " " << rect.left
             << " " << middle_y - delta_y << " " << rect.left << " " << middle_y
             << " c\n";
  // Fourth Bezier Curve
  app_stream << rect.left << " " << middle_y + delta_y << " "
             << middle_x - delta_x << " " << rect.top << " " << middle_x << " "
             << rect.top << " c\n";

  bool is_fill_rect = pInteriorColor && !pInteriorColor->IsEmpty();
  app_stream << GetPaintOperatorString(is_stroke_rect, is_fill_rect) << "\n";

  auto pExtGStateDict =
      GenerateExtGStateDict(*pAnnotDict, sExtGSDictName, "Normal");
  auto pResourceDict =
      GenerateResourceDict(pDoc, std::move(pExtGStateDict), nullptr);
  GenerateAndSetAPDict(pDoc, pAnnotDict, &app_stream, std::move(pResourceDict),
                       false /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateHighlightAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict) {
  fxcrt::ostringstream app_stream;
  ByteString sExtGSDictName = "GS";
  app_stream << "/" << sExtGSDictName << " gs ";

  app_stream << GetColorStringWithDefault(
      pAnnotDict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 1, 1, 0), PaintOperation::kFill);

  RetainPtr<const CPDF_Array> pArray = pAnnotDict->GetArrayFor("QuadPoints");
  if (pArray) {
    size_t nQuadPointCount = CPDF_Annot::QuadPointCount(pArray.Get());
    for (size_t i = 0; i < nQuadPointCount; ++i) {
      CFX_FloatRect rect = CPDF_Annot::RectFromQuadPoints(pAnnotDict, i);
      rect.Normalize();

      app_stream << rect.left << " " << rect.top << " m " << rect.right << " "
                 << rect.top << " l " << rect.right << " " << rect.bottom
                 << " l " << rect.left << " " << rect.bottom << " l h f\n";
    }
  }

  auto pExtGStateDict =
      GenerateExtGStateDict(*pAnnotDict, sExtGSDictName, "Multiply");
  auto pResourceDict =
      GenerateResourceDict(pDoc, std::move(pExtGStateDict), nullptr);
  GenerateAndSetAPDict(pDoc, pAnnotDict, &app_stream, std::move(pResourceDict),
                       true /*IsTextMarkupAnnotation*/);

  return true;
}

bool GenerateInkAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict) {
  RetainPtr<const CPDF_Array> pInkList = pAnnotDict->GetArrayFor("InkList");
  if (!pInkList || pInkList->IsEmpty())
    return false;

  float border_width = GetBorderWidth(pAnnotDict);
  const bool is_stroke = border_width > 0;
  if (!is_stroke) {
    return false;
  }

  ByteString sExtGSDictName = "GS";
  fxcrt::ostringstream app_stream;
  app_stream << "/" << sExtGSDictName << " gs ";
  app_stream << GetColorStringWithDefault(
      pAnnotDict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  app_stream << border_width << " w ";
  app_stream << GetDashPatternString(pAnnotDict);

  // Set inflated rect as a new rect because paths near the border with large
  // width should not be clipped to the original rect.
  CFX_FloatRect rect = pAnnotDict->GetRectFor(pdfium::annotation::kRect);
  rect.Inflate(border_width / 2, border_width / 2);
  pAnnotDict->SetRectFor(pdfium::annotation::kRect, rect);

  for (size_t i = 0; i < pInkList->size(); i++) {
    RetainPtr<const CPDF_Array> pInkCoordList = pInkList->GetArrayAt(i);
    if (!pInkCoordList || pInkCoordList->size() < 2)
      continue;

    app_stream << pInkCoordList->GetFloatAt(0) << " "
               << pInkCoordList->GetFloatAt(1) << " m ";

    for (size_t j = 0; j < pInkCoordList->size() - 1; j += 2) {
      app_stream << pInkCoordList->GetFloatAt(j) << " "
                 << pInkCoordList->GetFloatAt(j + 1) << " l ";
    }

    app_stream << "S\n";
  }

  auto pExtGStateDict =
      GenerateExtGStateDict(*pAnnotDict, sExtGSDictName, "Normal");
  auto pResourceDict =
      GenerateResourceDict(pDoc, std::move(pExtGStateDict), nullptr);
  GenerateAndSetAPDict(pDoc, pAnnotDict, &app_stream, std::move(pResourceDict),
                       false /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateTextAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict) {
  fxcrt::ostringstream app_stream;
  ByteString sExtGSDictName = "GS";
  app_stream << "/" << sExtGSDictName << " gs ";

  CFX_FloatRect rect = pAnnotDict->GetRectFor(pdfium::annotation::kRect);
  const float note_length = 20;
  CFX_FloatRect note_rect(rect.left, rect.bottom, rect.left + note_length,
                          rect.bottom + note_length);
  pAnnotDict->SetRectFor(pdfium::annotation::kRect, note_rect);

  app_stream << GenerateTextSymbolAP(note_rect);

  auto pExtGStateDict =
      GenerateExtGStateDict(*pAnnotDict, sExtGSDictName, "Normal");
  auto pResourceDict =
      GenerateResourceDict(pDoc, std::move(pExtGStateDict), nullptr);
  GenerateAndSetAPDict(pDoc, pAnnotDict, &app_stream, std::move(pResourceDict),
                       false /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateUnderlineAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict) {
  fxcrt::ostringstream app_stream;
  ByteString sExtGSDictName = "GS";
  app_stream << "/" << sExtGSDictName << " gs ";

  app_stream << GetColorStringWithDefault(
      pAnnotDict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  RetainPtr<const CPDF_Array> pArray = pAnnotDict->GetArrayFor("QuadPoints");
  if (pArray) {
    static constexpr int kLineWidth = 1;
    app_stream << kLineWidth << " w ";
    size_t nQuadPointCount = CPDF_Annot::QuadPointCount(pArray.Get());
    for (size_t i = 0; i < nQuadPointCount; ++i) {
      CFX_FloatRect rect = CPDF_Annot::RectFromQuadPoints(pAnnotDict, i);
      rect.Normalize();
      app_stream << rect.left << " " << rect.bottom + kLineWidth << " m "
                 << rect.right << " " << rect.bottom + kLineWidth << " l S\n";
    }
  }

  auto pExtGStateDict =
      GenerateExtGStateDict(*pAnnotDict, sExtGSDictName, "Normal");
  auto pResourceDict =
      GenerateResourceDict(pDoc, std::move(pExtGStateDict), nullptr);
  GenerateAndSetAPDict(pDoc, pAnnotDict, &app_stream, std::move(pResourceDict),
                       true /*IsTextMarkupAnnotation*/);
  return true;
}

bool GeneratePopupAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict) {
  fxcrt::ostringstream app_stream;
  ByteString sExtGSDictName = "GS";
  app_stream << "/" << sExtGSDictName << " gs\n";

  app_stream << GenerateColorAP(CFX_Color(CFX_Color::Type::kRGB, 1, 1, 0),
                                PaintOperation::kFill);
  app_stream << GenerateColorAP(CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0),
                                PaintOperation::kStroke);

  const float border_width = 1;
  app_stream << border_width << " w\n";

  CFX_FloatRect rect = pAnnotDict->GetRectFor(pdfium::annotation::kRect);
  rect.Normalize();
  rect.Deflate(border_width / 2, border_width / 2);

  app_stream << rect.left << " " << rect.bottom << " " << rect.Width() << " "
             << rect.Height() << " re b\n";

  RetainPtr<CPDF_Dictionary> font_dict = GenerateFallbackFontDict(pDoc);
  auto* pData = CPDF_DocPageData::FromDocument(pDoc);
  RetainPtr<CPDF_Font> pDefFont = pData->GetFont(font_dict);
  if (!pDefFont)
    return false;

  const ByteString font_name = "FONT";
  RetainPtr<CPDF_Dictionary> resource_font_dict =
      GenerateResourceFontDict(pDoc, font_name, font_dict->GetObjNum());
  RetainPtr<CPDF_Dictionary> pExtGStateDict =
      GenerateExtGStateDict(*pAnnotDict, sExtGSDictName, "Normal");
  RetainPtr<CPDF_Dictionary> pResourceDict = GenerateResourceDict(
      pDoc, std::move(pExtGStateDict), std::move(resource_font_dict));

  app_stream << GetPopupContentsString(pDoc, *pAnnotDict, std::move(pDefFont),
                                       font_name);
  GenerateAndSetAPDict(pDoc, pAnnotDict, &app_stream, std::move(pResourceDict),
                       false /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateSquareAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict) {
  const ByteString sExtGSDictName = "GS";
  fxcrt::ostringstream app_stream;
  app_stream << "/" << sExtGSDictName << " gs ";

  RetainPtr<const CPDF_Array> pInteriorColor = pAnnotDict->GetArrayFor("IC");
  app_stream << GetColorStringWithDefault(
      pInteriorColor.Get(), CFX_Color(CFX_Color::Type::kTransparent),
      PaintOperation::kFill);

  app_stream << GetColorStringWithDefault(
      pAnnotDict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  float border_width = GetBorderWidth(pAnnotDict);
  const bool is_stroke_rect = border_width > 0;
  if (is_stroke_rect) {
    app_stream << border_width << " w ";
    app_stream << GetDashPatternString(pAnnotDict);
  }

  CFX_FloatRect rect = pAnnotDict->GetRectFor(pdfium::annotation::kRect);
  rect.Normalize();

  if (is_stroke_rect) {
    // Deflating rect because stroking a path entails painting all points
    // whose perpendicular distance from the path in user space is less than
    // or equal to half the line width.
    rect.Deflate(border_width / 2, border_width / 2);
  }

  const bool is_fill_rect = pInteriorColor && (pInteriorColor->size() > 0);
  app_stream << rect.left << " " << rect.bottom << " " << rect.Width() << " "
             << rect.Height() << " re "
             << GetPaintOperatorString(is_stroke_rect, is_fill_rect) << "\n";

  auto pExtGStateDict =
      GenerateExtGStateDict(*pAnnotDict, sExtGSDictName, "Normal");
  auto pResourceDict =
      GenerateResourceDict(pDoc, std::move(pExtGStateDict), nullptr);
  GenerateAndSetAPDict(pDoc, pAnnotDict, &app_stream, std::move(pResourceDict),
                       false /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateSquigglyAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict) {
  fxcrt::ostringstream app_stream;
  ByteString sExtGSDictName = "GS";
  app_stream << "/" << sExtGSDictName << " gs ";

  app_stream << GetColorStringWithDefault(
      pAnnotDict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  RetainPtr<const CPDF_Array> pArray = pAnnotDict->GetArrayFor("QuadPoints");
  if (pArray) {
    static constexpr int kLineWidth = 1;
    static constexpr int kDelta = 2;
    app_stream << kLineWidth << " w ";
    size_t nQuadPointCount = CPDF_Annot::QuadPointCount(pArray.Get());
    for (size_t i = 0; i < nQuadPointCount; ++i) {
      CFX_FloatRect rect = CPDF_Annot::RectFromQuadPoints(pAnnotDict, i);
      rect.Normalize();

      const float top = rect.bottom + kDelta;
      const float bottom = rect.bottom;
      app_stream << rect.left << " " << top << " m ";

      float x = rect.left + kDelta;
      bool isUpwards = false;
      while (x < rect.right) {
        app_stream << x << " " << (isUpwards ? top : bottom) << " l ";
        x += kDelta;
        isUpwards = !isUpwards;
      }

      float remainder = rect.right - (x - kDelta);
      if (isUpwards)
        app_stream << rect.right << " " << bottom + remainder << " l ";
      else
        app_stream << rect.right << " " << top - remainder << " l ";

      app_stream << "S\n";
    }
  }

  auto pExtGStateDict =
      GenerateExtGStateDict(*pAnnotDict, sExtGSDictName, "Normal");
  auto pResourceDict =
      GenerateResourceDict(pDoc, std::move(pExtGStateDict), nullptr);
  GenerateAndSetAPDict(pDoc, pAnnotDict, &app_stream, std::move(pResourceDict),
                       true /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateStrikeOutAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict) {
  fxcrt::ostringstream app_stream;
  ByteString sExtGSDictName = "GS";
  app_stream << "/" << sExtGSDictName << " gs ";

  app_stream << GetColorStringWithDefault(
      pAnnotDict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  RetainPtr<const CPDF_Array> pArray = pAnnotDict->GetArrayFor("QuadPoints");
  if (pArray) {
    size_t nQuadPointCount = CPDF_Annot::QuadPointCount(pArray.Get());
    for (size_t i = 0; i < nQuadPointCount; ++i) {
      CFX_FloatRect rect = CPDF_Annot::RectFromQuadPoints(pAnnotDict, i);
      rect.Normalize();

      float y = (rect.top + rect.bottom) / 2;
      static constexpr int kLineWidth = 1;
      app_stream << kLineWidth << " w " << rect.left << " " << y << " m "
                 << rect.right << " " << y << " l S\n";
    }
  }

  auto pExtGStateDict =
      GenerateExtGStateDict(*pAnnotDict, sExtGSDictName, "Normal");
  auto pResourceDict =
      GenerateResourceDict(pDoc, std::move(pExtGStateDict), nullptr);
  GenerateAndSetAPDict(pDoc, pAnnotDict, &app_stream, std::move(pResourceDict),
                       true /*IsTextMarkupAnnotation*/);
  return true;
}

}  // namespace

// static
void CPDF_GenerateAP::GenerateFormAP(CPDF_Document* pDoc,
                                     CPDF_Dictionary* pAnnotDict,
                                     FormType type) {
  RetainPtr<CPDF_Dictionary> pRootDict = pDoc->GetMutableRoot();
  if (!pRootDict)
    return;

  RetainPtr<CPDF_Dictionary> pFormDict =
      pRootDict->GetMutableDictFor("AcroForm");
  if (!pFormDict)
    return;

  ByteString DA;
  RetainPtr<const CPDF_Object> pDAObj =
      CPDF_FormField::GetFieldAttrForDict(pAnnotDict, "DA");
  if (pDAObj)
    DA = pDAObj->GetString();
  if (DA.IsEmpty())
    DA = pFormDict->GetByteStringFor("DA");
  if (DA.IsEmpty())
    return;

  CPDF_DefaultAppearance appearance(DA);

  float font_size = 0;
  std::optional<ByteString> font = appearance.GetFont(&font_size);
  if (!font.has_value())
    return;

  ByteString font_name = font.value();

  CFX_Color text_color = fpdfdoc::CFXColorFromString(DA);
  RetainPtr<CPDF_Dictionary> pDRDict = pFormDict->GetMutableDictFor("DR");
  if (!pDRDict)
    return;

  RetainPtr<CPDF_Dictionary> pDRFontDict = pDRDict->GetMutableDictFor("Font");
  if (!ValidateFontResourceDict(pDRFontDict.Get()))
    return;

  RetainPtr<CPDF_Dictionary> pFontDict =
      pDRFontDict->GetMutableDictFor(font_name);
  if (!pFontDict) {
    pFontDict = GenerateFallbackFontDict(pDoc);
    pDRFontDict->SetNewFor<CPDF_Reference>(font_name, pDoc,
                                           pFontDict->GetObjNum());
  }
  auto* pData = CPDF_DocPageData::FromDocument(pDoc);
  RetainPtr<CPDF_Font> pDefFont = pData->GetFont(pFontDict);
  if (!pDefFont)
    return;

  CFX_FloatRect annot_rect = pAnnotDict->GetRectFor(pdfium::annotation::kRect);
  RetainPtr<const CPDF_Dictionary> pMKDict = pAnnotDict->GetDictFor("MK");
  const int32_t rotate =
      pMKDict ? pMKDict->GetIntegerFor(pdfium::appearance::kR) : 0;

  CFX_FloatRect bbox_rect;
  CFX_Matrix matrix;
  switch (rotate % 360) {
    case 0:
      bbox_rect = CFX_FloatRect(0, 0, annot_rect.right - annot_rect.left,
                                annot_rect.top - annot_rect.bottom);
      break;
    case 90:
      matrix = CFX_Matrix(0, 1, -1, 0, annot_rect.right - annot_rect.left, 0);
      bbox_rect = CFX_FloatRect(0, 0, annot_rect.top - annot_rect.bottom,
                                annot_rect.right - annot_rect.left);
      break;
    case 180:
      matrix = CFX_Matrix(-1, 0, 0, -1, annot_rect.right - annot_rect.left,
                          annot_rect.top - annot_rect.bottom);
      bbox_rect = CFX_FloatRect(0, 0, annot_rect.right - annot_rect.left,
                                annot_rect.top - annot_rect.bottom);
      break;
    case 270:
      matrix = CFX_Matrix(0, -1, 1, 0, 0, annot_rect.top - annot_rect.bottom);
      bbox_rect = CFX_FloatRect(0, 0, annot_rect.top - annot_rect.bottom,
                                annot_rect.right - annot_rect.left);
      break;
  }

  BorderStyle nBorderStyle = BorderStyle::kSolid;
  float border_width = 1;
  CPVT_Dash dsBorder(3, 0, 0);
  CFX_Color left_top_color;
  CFX_Color right_bottom_color;
  if (RetainPtr<const CPDF_Dictionary> pBSDict = pAnnotDict->GetDictFor("BS")) {
    if (pBSDict->KeyExist("W"))
      border_width = pBSDict->GetFloatFor("W");

    if (RetainPtr<const CPDF_Array> pArray = pBSDict->GetArrayFor("D")) {
      dsBorder = CPVT_Dash(pArray->GetIntegerAt(0), pArray->GetIntegerAt(1),
                           pArray->GetIntegerAt(2));
    }
    if (pBSDict->GetByteStringFor("S").GetLength()) {
      switch (pBSDict->GetByteStringFor("S")[0]) {
        case 'S':
          nBorderStyle = BorderStyle::kSolid;
          break;
        case 'D':
          nBorderStyle = BorderStyle::kDash;
          break;
        case 'B':
          nBorderStyle = BorderStyle::kBeveled;
          border_width *= 2;
          left_top_color = CFX_Color(CFX_Color::Type::kGray, 1);
          right_bottom_color = CFX_Color(CFX_Color::Type::kGray, 0.5);
          break;
        case 'I':
          nBorderStyle = BorderStyle::kInset;
          border_width *= 2;
          left_top_color = CFX_Color(CFX_Color::Type::kGray, 0.5);
          right_bottom_color = CFX_Color(CFX_Color::Type::kGray, 0.75);
          break;
        case 'U':
          nBorderStyle = BorderStyle::kUnderline;
          break;
      }
    }
  }
  CFX_Color border_color;
  CFX_Color background_color;
  if (pMKDict) {
    RetainPtr<const CPDF_Array> pArray =
        pMKDict->GetArrayFor(pdfium::appearance::kBC);
    if (pArray)
      border_color = fpdfdoc::CFXColorFromArray(*pArray);
    pArray = pMKDict->GetArrayFor(pdfium::appearance::kBG);
    if (pArray)
      background_color = fpdfdoc::CFXColorFromArray(*pArray);
  }
  fxcrt::ostringstream app_stream;
  ByteString background =
      GenerateColorAP(background_color, PaintOperation::kFill);
  if (background.GetLength() > 0) {
    app_stream << "q\n" << background;
    WriteRect(app_stream, bbox_rect) << " re f\nQ\n";
  }
  ByteString border_stream =
      GenerateBorderAP(bbox_rect, border_width, border_color, left_top_color,
                       right_bottom_color, nBorderStyle, dsBorder);
  if (border_stream.GetLength() > 0) {
    app_stream << "q\n" << border_stream << "Q\n";
  }

  CFX_FloatRect body_rect = CFX_FloatRect(
      bbox_rect.left + border_width, bbox_rect.bottom + border_width,
      bbox_rect.right - border_width, bbox_rect.top - border_width);
  body_rect.Normalize();

  RetainPtr<CPDF_Dictionary> pAPDict =
      pAnnotDict->GetOrCreateDictFor(pdfium::annotation::kAP);
  RetainPtr<CPDF_Stream> pNormalStream = pAPDict->GetMutableStreamFor("N");
  RetainPtr<CPDF_Dictionary> pStreamDict;
  if (pNormalStream) {
    pStreamDict = pNormalStream->GetMutableDict();
    RetainPtr<CPDF_Dictionary> pStreamResList =
        pStreamDict->GetMutableDictFor("Resources");
    if (pStreamResList) {
      RetainPtr<CPDF_Dictionary> pStreamResFontList =
          pStreamResList->GetMutableDictFor("Font");
      if (pStreamResFontList) {
        if (!ValidateFontResourceDict(pStreamResFontList.Get()))
          return;
      } else {
        pStreamResFontList = pStreamResList->SetNewFor<CPDF_Dictionary>("Font");
      }
      if (!pStreamResFontList->KeyExist(font_name)) {
        pStreamResFontList->SetNewFor<CPDF_Reference>(font_name, pDoc,
                                                      pFontDict->GetObjNum());
      }
    } else {
      pStreamDict->SetFor("Resources", pFormDict->GetDictFor("DR")->Clone());
    }
    pStreamDict->SetMatrixFor("Matrix", matrix);
    pStreamDict->SetRectFor("BBox", bbox_rect);
  } else {
    pNormalStream =
        pDoc->NewIndirect<CPDF_Stream>(pdfium::MakeRetain<CPDF_Dictionary>());
    pAPDict->SetNewFor<CPDF_Reference>("N", pDoc, pNormalStream->GetObjNum());
  }
  CPVT_FontMap map(
      pDoc, pStreamDict ? pStreamDict->GetMutableDictFor("Resources") : nullptr,
      std::move(pDefFont), font_name);
  CPVT_VariableText::Provider prd(&map);

  switch (type) {
    case CPDF_GenerateAP::kTextField: {
      RetainPtr<const CPDF_Object> pV = CPDF_FormField::GetFieldAttrForDict(
          pAnnotDict, pdfium::form_fields::kV);
      WideString value = pV ? pV->GetUnicodeText() : WideString();
      RetainPtr<const CPDF_Object> pQ =
          CPDF_FormField::GetFieldAttrForDict(pAnnotDict, "Q");
      const int32_t align = pQ ? pQ->GetInteger() : 0;
      RetainPtr<const CPDF_Object> pFf = CPDF_FormField::GetFieldAttrForDict(
          pAnnotDict, pdfium::form_fields::kFf);
      const uint32_t flags = pFf ? pFf->GetInteger() : 0;
      RetainPtr<const CPDF_Object> pMaxLen =
          CPDF_FormField::GetFieldAttrForDict(pAnnotDict, "MaxLen");
      const uint32_t max_len = pMaxLen ? pMaxLen->GetInteger() : 0;
      CPVT_VariableText vt(&prd);
      vt.SetPlateRect(body_rect);
      vt.SetAlignment(align);
      if (FXSYS_IsFloatZero(font_size)) {
        vt.SetAutoFontSize(true);
      } else {
        vt.SetFontSize(font_size);
      }

      bool is_multi_line = (flags >> 12) & 1;
      if (is_multi_line) {
        vt.SetMultiLine(true);
        vt.SetAutoReturn(true);
      }
      uint16_t sub_word = 0;
      if ((flags >> 13) & 1) {
        sub_word = '*';
        vt.SetPasswordChar(sub_word);
      }
      bool is_char_array = (flags >> 24) & 1;
      if (is_char_array) {
        vt.SetCharArray(max_len);
      } else {
        vt.SetLimitChar(max_len);
      }

      vt.Initialize();
      vt.SetText(value);
      vt.RearrangeAll();
      CFX_FloatRect content_rect = vt.GetContentRect();
      CFX_PointF offset;
      if (!is_multi_line) {
        offset = CFX_PointF(
            0.0f, (content_rect.Height() - body_rect.Height()) / 2.0f);
      }
      ByteString body = GenerateEditAP(&map, vt.GetIterator(), offset,
                                       !is_char_array, sub_word);
      if (body.GetLength() > 0) {
        app_stream << "/Tx BMC\n" << "q\n";
        if (content_rect.Width() > body_rect.Width() ||
            content_rect.Height() > body_rect.Height()) {
          WriteRect(app_stream, body_rect) << " re\nW\nn\n";
        }
        app_stream << "BT\n"
                   << GenerateColorAP(text_color, PaintOperation::kFill) << body
                   << "ET\n"
                   << "Q\nEMC\n";
      }
      break;
    }
    case CPDF_GenerateAP::kComboBox: {
      RetainPtr<const CPDF_Object> pV = CPDF_FormField::GetFieldAttrForDict(
          pAnnotDict, pdfium::form_fields::kV);
      WideString value = pV ? pV->GetUnicodeText() : WideString();
      CPVT_VariableText vt(&prd);
      CFX_FloatRect button_rect = body_rect;
      button_rect.left = button_rect.right - 13;
      button_rect.Normalize();
      CFX_FloatRect edit_rect = body_rect;
      edit_rect.right = button_rect.left;
      edit_rect.Normalize();
      vt.SetPlateRect(edit_rect);
      if (FXSYS_IsFloatZero(font_size)) {
        vt.SetAutoFontSize(true);
      } else {
        vt.SetFontSize(font_size);
      }

      vt.Initialize();
      vt.SetText(value);
      vt.RearrangeAll();
      CFX_FloatRect content_rect = vt.GetContentRect();
      CFX_PointF offset =
          CFX_PointF(0.0f, (content_rect.Height() - edit_rect.Height()) / 2.0f);
      ByteString edit = GenerateEditAP(&map, vt.GetIterator(), offset, true, 0);
      if (edit.GetLength() > 0) {
        app_stream << "/Tx BMC\nq\n";
        WriteRect(app_stream, edit_rect) << " re\nW\nn\n";
        app_stream << "BT\n"
                   << GenerateColorAP(text_color, PaintOperation::kFill) << edit
                   << "ET\n"
                   << "Q\nEMC\n";
      }
      ByteString button =
          GenerateColorAP(CFX_Color(CFX_Color::Type::kRGB, 220.0f / 255.0f,
                                    220.0f / 255.0f, 220.0f / 255.0f),
                          PaintOperation::kFill);
      if (button.GetLength() > 0 && !button_rect.IsEmpty()) {
        app_stream << "q\n" << button;
        WriteRect(app_stream, button_rect) << " re f\n";
        app_stream << "Q\n";
        ByteString button_border = GenerateBorderAP(
            button_rect, 2, CFX_Color(CFX_Color::Type::kGray, 0),
            CFX_Color(CFX_Color::Type::kGray, 1),
            CFX_Color(CFX_Color::Type::kGray, 0.5), BorderStyle::kBeveled,
            CPVT_Dash(3, 0, 0));
        if (button_border.GetLength() > 0) {
          app_stream << "q\n" << button_border << "Q\n";
        }

        CFX_PointF center((button_rect.left + button_rect.right) / 2,
                          (button_rect.top + button_rect.bottom) / 2);
        if (FXSYS_IsFloatBigger(button_rect.Width(), 6) &&
            FXSYS_IsFloatBigger(button_rect.Height(), 6)) {
          app_stream << "q\n0 g\n";
          WritePoint(app_stream, {center.x - 3, center.y + 1.5f}) << " m\n";
          WritePoint(app_stream, {center.x + 3, center.y + 1.5f}) << " l\n";
          WritePoint(app_stream, {center.x, center.y - 1.5f}) << " l\n";
          WritePoint(app_stream, {center.x - 3, center.y + 1.5f}) << " l f\n";
          app_stream << button << "Q\n";
        }
      }
      break;
    }
    case CPDF_GenerateAP::kListBox: {
      RetainPtr<const CPDF_Array> pOpts =
          ToArray(CPDF_FormField::GetFieldAttrForDict(pAnnotDict, "Opt"));
      RetainPtr<const CPDF_Array> pSels =
          ToArray(CPDF_FormField::GetFieldAttrForDict(pAnnotDict, "I"));
      RetainPtr<const CPDF_Object> pTi =
          CPDF_FormField::GetFieldAttrForDict(pAnnotDict, "TI");
      const int32_t top = pTi ? pTi->GetInteger() : 0;
      fxcrt::ostringstream body_stream;
      if (pOpts) {
        float fy = body_rect.top;
        for (size_t i = top, sz = pOpts->size(); i < sz; i++) {
          if (FXSYS_IsFloatSmaller(fy, body_rect.bottom)) {
            break;
          }

          if (RetainPtr<const CPDF_Object> pOpt = pOpts->GetDirectObjectAt(i)) {
            WideString item;
            if (pOpt->IsString()) {
              item = pOpt->GetUnicodeText();
            } else if (const CPDF_Array* pArray = pOpt->AsArray()) {
              RetainPtr<const CPDF_Object> pDirectObj =
                  pArray->GetDirectObjectAt(1);
              if (pDirectObj)
                item = pDirectObj->GetUnicodeText();
            }
            bool is_selected = false;
            if (pSels) {
              for (size_t s = 0, ssz = pSels->size(); s < ssz; s++) {
                int value = pSels->GetIntegerAt(s);
                if (value >= 0 && i == static_cast<size_t>(value)) {
                  is_selected = true;
                  break;
                }
              }
            }
            CPVT_VariableText vt(&prd);
            vt.SetPlateRect(
                CFX_FloatRect(body_rect.left, 0.0f, body_rect.right, 0.0f));
            vt.SetFontSize(FXSYS_IsFloatZero(font_size) ? 12.0f : font_size);
            vt.Initialize();
            vt.SetText(item);
            vt.RearrangeAll();

            const float item_height = vt.GetContentRect().Height();
            if (is_selected) {
              CFX_FloatRect item_rect = CFX_FloatRect(
                  body_rect.left, fy - item_height, body_rect.right, fy);
              body_stream << "q\n"
                          << GenerateColorAP(
                                 CFX_Color(CFX_Color::Type::kRGB, 0,
                                           51.0f / 255.0f, 113.0f / 255.0f),
                                 PaintOperation::kFill);
              WriteRect(body_stream, item_rect) << " re f\nQ\n";
              body_stream << "BT\n"
                          << GenerateColorAP(
                                 CFX_Color(CFX_Color::Type::kGray, 1),
                                 PaintOperation::kFill)
                          << GenerateEditAP(&map, vt.GetIterator(),
                                            CFX_PointF(0.0f, fy), true, 0)
                          << "ET\n";
            } else {
              body_stream << "BT\n"
                          << GenerateColorAP(text_color, PaintOperation::kFill)
                          << GenerateEditAP(&map, vt.GetIterator(),
                                            CFX_PointF(0.0f, fy), true, 0)
                          << "ET\n";
            }
            fy -= item_height;
          }
        }
      }
      if (body_stream.tellp() > 0) {
        app_stream << "/Tx BMC\nq\n";
        WriteRect(app_stream, body_rect) << " re\nW\nn\n"
                                         << body_stream.str() << "Q\nEMC\n";
      }
      break;
    }
  }

  if (!pNormalStream)
    return;

  pNormalStream->SetDataFromStringstreamAndRemoveFilter(&app_stream);
  pStreamDict = pNormalStream->GetMutableDict();
  pStreamDict->SetMatrixFor("Matrix", matrix);
  pStreamDict->SetRectFor("BBox", bbox_rect);
  RetainPtr<CPDF_Dictionary> pStreamResList =
      pStreamDict->GetMutableDictFor("Resources");
  if (!pStreamResList) {
    pStreamDict->SetFor("Resources", pFormDict->GetDictFor("DR")->Clone());
    return;
  }

  RetainPtr<CPDF_Dictionary> pStreamResFontList =
      pStreamResList->GetMutableDictFor("Font");
  if (pStreamResFontList) {
    if (!ValidateFontResourceDict(pStreamResFontList.Get()))
      return;
  } else {
    pStreamResFontList = pStreamResList->SetNewFor<CPDF_Dictionary>("Font");
  }

  if (!pStreamResFontList->KeyExist(font_name)) {
    pStreamResFontList->SetNewFor<CPDF_Reference>(font_name, pDoc,
                                                  pFontDict->GetObjNum());
  }
}

// static
void CPDF_GenerateAP::GenerateEmptyAP(CPDF_Document* pDoc,
                                      CPDF_Dictionary* pAnnotDict) {
  auto pExtGStateDict = GenerateExtGStateDict(*pAnnotDict, "GS", "Normal");
  auto pResourceDict =
      GenerateResourceDict(pDoc, std::move(pExtGStateDict), nullptr);

  fxcrt::ostringstream stream;
  GenerateAndSetAPDict(pDoc, pAnnotDict, &stream, std::move(pResourceDict),
                       false);
}

// static
bool CPDF_GenerateAP::GenerateAnnotAP(CPDF_Document* pDoc,
                                      CPDF_Dictionary* pAnnotDict,
                                      CPDF_Annot::Subtype subtype) {
  switch (subtype) {
    case CPDF_Annot::Subtype::CIRCLE:
      return GenerateCircleAP(pDoc, pAnnotDict);
    case CPDF_Annot::Subtype::HIGHLIGHT:
      return GenerateHighlightAP(pDoc, pAnnotDict);
    case CPDF_Annot::Subtype::INK:
      return GenerateInkAP(pDoc, pAnnotDict);
    case CPDF_Annot::Subtype::POPUP:
      return GeneratePopupAP(pDoc, pAnnotDict);
    case CPDF_Annot::Subtype::SQUARE:
      return GenerateSquareAP(pDoc, pAnnotDict);
    case CPDF_Annot::Subtype::SQUIGGLY:
      return GenerateSquigglyAP(pDoc, pAnnotDict);
    case CPDF_Annot::Subtype::STRIKEOUT:
      return GenerateStrikeOutAP(pDoc, pAnnotDict);
    case CPDF_Annot::Subtype::TEXT:
      return GenerateTextAP(pDoc, pAnnotDict);
    case CPDF_Annot::Subtype::UNDERLINE:
      return GenerateUnderlineAP(pDoc, pAnnotDict);
    default:
      return false;
  }
}
