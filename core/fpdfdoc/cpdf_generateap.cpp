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

constexpr char kGSDictName[] = "GS";

struct CPVT_Dash {
  CPVT_Dash(int32_t dash, int32_t gap, int32_t phase)
      : dash(dash), gap(gap), phase(phase) {}

  int32_t dash;
  int32_t gap;
  int32_t phase;
};

enum class PaintOperation { kStroke, kFill };

ByteString GetPDFWordString(IPVT_FontMap* font_map,
                            int32_t font_index,
                            uint16_t word,
                            uint16_t sub_word) {
  if (sub_word > 0) {
    return ByteString::Format("%c", sub_word);
  }

  if (!font_map) {
    return ByteString();
  }

  RetainPtr<CPDF_Font> pdf_font = font_map->GetPDFFont(font_index);
  if (!pdf_font) {
    return ByteString();
  }

  if (pdf_font->GetBaseFontName() == "Symbol" ||
      pdf_font->GetBaseFontName() == "ZapfDingbats") {
    return ByteString::Format("%c", word);
  }

  ByteString word_string;
  uint32_t char_code = pdf_font->CharCodeFromUnicode(word);
  if (char_code != CPDF_Font::kInvalidCharCode) {
    pdf_font->AppendChar(&word_string, char_code);
  }
  return word_string;
}

ByteString GetWordRenderString(ByteStringView words) {
  if (words.IsEmpty()) {
    return ByteString();
  }
  return PDF_EncodeString(words) + " Tj\n";
}

ByteString GetFontSetString(IPVT_FontMap* font_map,
                            int32_t font_index,
                            float font_size) {
  fxcrt::ostringstream font_stream;
  if (font_map) {
    ByteString font_alias = font_map->GetPDFFontAlias(font_index);
    if (font_alias.GetLength() > 0 && font_size > 0) {
      font_stream << "/" << font_alias << " ";
      WriteFloat(font_stream, font_size) << " Tf\n";
    }
  }
  return ByteString(font_stream);
}

struct BorderStyleInfo {
  float width = 1;
  BorderStyle style = BorderStyle::kSolid;
  CPVT_Dash dash_pattern{3, 0, 0};
};

BorderStyleInfo GetBorderStyleInfo(const CPDF_Dictionary* border_style_dict) {
  BorderStyleInfo border_style_info;
  if (!border_style_dict) {
    return border_style_info;
  }

  if (border_style_dict->KeyExist("W")) {
    border_style_info.width = border_style_dict->GetFloatFor("W");
  }

  const ByteString border_style_string =
      border_style_dict->GetByteStringFor("S");
  if (border_style_string.GetLength()) {
    switch (border_style_string[0]) {
      case 'S':
        border_style_info.style = BorderStyle::kSolid;
        break;
      case 'D':
        border_style_info.style = BorderStyle::kDash;
        break;
      case 'B':
        border_style_info.style = BorderStyle::kBeveled;
        border_style_info.width *= 2;
        break;
      case 'I':
        border_style_info.style = BorderStyle::kInset;
        border_style_info.width *= 2;
        break;
      case 'U':
        border_style_info.style = BorderStyle::kUnderline;
        break;
    }
  }

  RetainPtr<const CPDF_Array> dash_array = border_style_dict->GetArrayFor("D");
  if (dash_array) {
    border_style_info.dash_pattern =
        CPVT_Dash(dash_array->GetIntegerAt(0), dash_array->GetIntegerAt(1),
                  dash_array->GetIntegerAt(2));
  }

  return border_style_info;
}

ByteString GenerateEditAP(IPVT_FontMap* font_map,
                          CPVT_VariableText::Iterator* vt_iterator,
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
  vt_iterator->SetAt(0);
  while (vt_iterator->NextWord()) {
    CPVT_WordPlace place = vt_iterator->GetWordPlace();
    if (continuous) {
      if (place.LineCmp(oldplace) != 0) {
        if (!words.IsEmpty()) {
          line_stream << GetWordRenderString(words.AsStringView());
          edit_stream << line_stream.str();
          line_stream.str("");
          words.clear();
        }
        CPVT_Word word;
        if (vt_iterator->GetWord(word)) {
          new_point =
              CFX_PointF(word.ptWord.x + offset.x, word.ptWord.y + offset.y);
        } else {
          CPVT_Line line;
          vt_iterator->GetLine(line);
          new_point =
              CFX_PointF(line.ptLine.x + offset.x, line.ptLine.y + offset.y);
        }
        if (new_point != old_point) {
          WritePoint(line_stream, new_point - old_point) << " Td\n";
          old_point = new_point;
        }
      }
      CPVT_Word word;
      if (vt_iterator->GetWord(word)) {
        if (word.nFontIndex != current_font_index) {
          if (!words.IsEmpty()) {
            line_stream << GetWordRenderString(words.AsStringView());
            words.clear();
          }
          line_stream << GetFontSetString(font_map, word.nFontIndex,
                                          word.fFontSize);
          current_font_index = word.nFontIndex;
        }
        words +=
            GetPDFWordString(font_map, current_font_index, word.Word, sub_word);
      }
      oldplace = place;
    } else {
      CPVT_Word word;
      if (vt_iterator->GetWord(word)) {
        new_point =
            CFX_PointF(word.ptWord.x + offset.x, word.ptWord.y + offset.y);
        if (new_point != old_point) {
          WritePoint(edit_stream, new_point - old_point) << " Td\n";
          old_point = new_point;
        }
        if (word.nFontIndex != current_font_index) {
          edit_stream << GetFontSetString(font_map, word.nFontIndex,
                                          word.fFontSize);
          current_font_index = word.nFontIndex;
        }
        edit_stream << GetWordRenderString(
            GetPDFWordString(font_map, current_font_index, word.Word, sub_word)
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

ByteString GenerateColorAP(const CFX_Color& color, PaintOperation operation) {
  fxcrt::ostringstream color_stream;
  switch (color.nColorType) {
    case CFX_Color::Type::kRGB:
      WriteFloat(color_stream, color.fColor1) << " ";
      WriteFloat(color_stream, color.fColor2) << " ";
      WriteFloat(color_stream, color.fColor3) << " ";
      color_stream << (operation == PaintOperation::kStroke ? "RG" : "rg")
                   << "\n";
      return ByteString(color_stream);
    case CFX_Color::Type::kGray:
      WriteFloat(color_stream, color.fColor1) << " ";
      color_stream << (operation == PaintOperation::kStroke ? "G" : "g")
                   << "\n";
      return ByteString(color_stream);
    case CFX_Color::Type::kCMYK:
      WriteFloat(color_stream, color.fColor1) << " ";
      WriteFloat(color_stream, color.fColor2) << " ";
      WriteFloat(color_stream, color.fColor3) << " ";
      WriteFloat(color_stream, color.fColor4) << " ";
      color_stream << (operation == PaintOperation::kStroke ? "K" : "k")
                   << "\n";
      return ByteString(color_stream);
    case CFX_Color::Type::kTransparent:
      return ByteString();
  }
  NOTREACHED();
}

ByteString GenerateBorderAP(const CFX_FloatRect& rect,
                            const BorderStyleInfo& border_style_info,
                            const CFX_Color& border_color) {
  const float width = border_style_info.width;
  if (width <= 0) {
    return ByteString();
  }

  fxcrt::ostringstream app_stream;
  const float left = rect.left;
  const float bottom = rect.bottom;
  const float right = rect.right;
  const float top = rect.top;
  const float half_width = width / 2.0f;
  switch (border_style_info.style) {
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
        const auto& dash = border_style_info.dash_pattern;
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
      const float left_top_gray_value =
          border_style_info.style == BorderStyle::kBeveled ? 1.0f : 0.5f;
      app_stream << GenerateColorAP(
          CFX_Color(CFX_Color::Type::kGray, left_top_gray_value),
          PaintOperation::kFill);
      WritePoint(app_stream, {left + half_width, bottom + half_width})
          << " m\n";
      WritePoint(app_stream, {left + half_width, top - half_width}) << " l\n";
      WritePoint(app_stream, {right - half_width, top - half_width}) << " l\n";
      WritePoint(app_stream, {right - width, top - width}) << " l\n";
      WritePoint(app_stream, {left + width, top - width}) << " l\n";
      WritePoint(app_stream, {left + width, bottom + width}) << " l f\n";

      const float right_bottom_gray_value =
          border_style_info.style == BorderStyle::kBeveled ? 0.5f : 0.75f;
      app_stream << GenerateColorAP(
          CFX_Color(CFX_Color::Type::kGray, right_bottom_gray_value),
          PaintOperation::kFill);
      WritePoint(app_stream, {right - half_width, top - half_width}) << " m\n";
      WritePoint(app_stream, {right - half_width, bottom + half_width})
          << " l\n";
      WritePoint(app_stream, {left + half_width, bottom + half_width})
          << " l\n";
      WritePoint(app_stream, {left + width, bottom + width}) << " l\n";
      WritePoint(app_stream, {right - width, bottom + width}) << " l\n";
      WritePoint(app_stream, {right - width, top - width}) << " l f\n";

      ByteString color_string =
          GenerateColorAP(border_color, PaintOperation::kFill);
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

ByteString GetColorStringWithDefault(const CPDF_Array* color_array,
                                     const CFX_Color& default_color,
                                     PaintOperation operation) {
  if (color_array) {
    CFX_Color color = fpdfdoc::CFXColorFromArray(*color_array);
    return GenerateColorAP(color, operation);
  }

  return GenerateColorAP(default_color, operation);
}

float GetBorderWidth(const CPDF_Dictionary* dict) {
  RetainPtr<const CPDF_Dictionary> border_style_dict = dict->GetDictFor("BS");
  if (border_style_dict && border_style_dict->KeyExist("W")) {
    return border_style_dict->GetFloatFor("W");
  }

  auto border_array = dict->GetArrayFor(pdfium::annotation::kBorder);
  if (border_array && border_array->size() > 2) {
    return border_array->GetFloatAt(2);
  }

  return 1;
}

RetainPtr<const CPDF_Array> GetDashArray(const CPDF_Dictionary* dict) {
  RetainPtr<const CPDF_Dictionary> border_style_dict = dict->GetDictFor("BS");
  if (border_style_dict && border_style_dict->GetByteStringFor("S") == "D") {
    return border_style_dict->GetArrayFor("D");
  }

  RetainPtr<const CPDF_Array> border_array =
      dict->GetArrayFor(pdfium::annotation::kBorder);
  if (border_array && border_array->size() == 4) {
    return border_array->GetArrayAt(3);
  }

  return nullptr;
}

ByteString GetDashPatternString(const CPDF_Dictionary* dict) {
  RetainPtr<const CPDF_Array> dash_array = GetDashArray(dict);
  if (!dash_array || dash_array->IsEmpty()) {
    return ByteString();
  }

  // Support maximum of ten elements in the dash array.
  size_t dash_arrayCount = std::min<size_t>(dash_array->size(), 10);
  fxcrt::ostringstream dash_stream;

  dash_stream << "[";
  for (size_t i = 0; i < dash_arrayCount; ++i) {
    WriteFloat(dash_stream, dash_array->GetFloatAt(i)) << " ";
  }
  dash_stream << "] 0 d\n";

  return ByteString(dash_stream);
}

ByteString GetPopupContentsString(CPDF_Document* doc,
                                  const CPDF_Dictionary& annot_dict,
                                  RetainPtr<CPDF_Font> default_font,
                                  const ByteString& font_name) {
  WideString value(annot_dict.GetUnicodeTextFor(pdfium::form_fields::kT));
  value += L'\n';
  value += annot_dict.GetUnicodeTextFor(pdfium::annotation::kContents);

  CPVT_FontMap map(doc, nullptr, std::move(default_font), font_name);
  CPVT_VariableText::Provider prd(&map);
  CPVT_VariableText vt(&prd);
  vt.SetPlateRect(annot_dict.GetRectFor(pdfium::annotation::kRect));
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

  CFX_FloatRect outer_rect1 = rect;
  outer_rect1.Deflate(kHalfWidth, kHalfWidth);
  outer_rect1.bottom += kTipDelta;

  CFX_FloatRect outer_rect2 = outer_rect1;
  outer_rect2.left += kTipDelta;
  outer_rect2.right = outer_rect2.left + kTipDelta;
  outer_rect2.top = outer_rect2.bottom - kTipDelta;
  float outer_rect2_middle = (outer_rect2.left + outer_rect2.right) / 2;

  // Draw outer boxes.
  WritePoint(app_stream, {outer_rect1.left, outer_rect1.bottom}) << " m\n";
  WritePoint(app_stream, {outer_rect1.left, outer_rect1.top}) << " l\n";
  WritePoint(app_stream, {outer_rect1.right, outer_rect1.top}) << " l\n";
  WritePoint(app_stream, {outer_rect1.right, outer_rect1.bottom}) << " l\n";
  WritePoint(app_stream, {outer_rect2.right, outer_rect2.bottom}) << " l\n";
  WritePoint(app_stream, {outer_rect2_middle, outer_rect2.top}) << " l\n";
  WritePoint(app_stream, {outer_rect2.left, outer_rect2.bottom}) << " l\n";
  WritePoint(app_stream, {outer_rect1.left, outer_rect1.bottom}) << " l\n";

  // Draw inner lines.
  CFX_FloatRect line_rect = outer_rect1;
  const float delta_x = 2;
  const float delta_y = (line_rect.top - line_rect.bottom) / 4;

  line_rect.left += delta_x;
  line_rect.right -= delta_x;
  for (int i = 0; i < 3; ++i) {
    line_rect.top -= delta_y;
    WritePoint(app_stream, {line_rect.left, line_rect.top}) << " m\n";
    WritePoint(app_stream, {line_rect.right, line_rect.top}) << " l\n";
  }
  app_stream << "B*\n";

  return ByteString(app_stream);
}

RetainPtr<CPDF_Dictionary> GenerateExtGStateDict(
    const CPDF_Dictionary& annot_dict,
    const ByteString& blend_mode) {
  auto gs_dict =
      pdfium::MakeRetain<CPDF_Dictionary>(annot_dict.GetByteStringPool());
  gs_dict->SetNewFor<CPDF_Name>("Type", "ExtGState");

  float opacity = annot_dict.KeyExist("CA") ? annot_dict.GetFloatFor("CA") : 1;
  gs_dict->SetNewFor<CPDF_Number>("CA", opacity);
  gs_dict->SetNewFor<CPDF_Number>("ca", opacity);
  gs_dict->SetNewFor<CPDF_Boolean>("AIS", false);
  gs_dict->SetNewFor<CPDF_Name>("BM", blend_mode);

  auto resources_dict =
      pdfium::MakeRetain<CPDF_Dictionary>(annot_dict.GetByteStringPool());
  resources_dict->SetFor(kGSDictName, std::move(gs_dict));
  return resources_dict;
}

RetainPtr<CPDF_Dictionary> GenerateResourcesDict(
    CPDF_Document* doc,
    RetainPtr<CPDF_Dictionary> gs_dict,
    RetainPtr<CPDF_Dictionary> font_resource_dict) {
  auto resources_dict = doc->New<CPDF_Dictionary>();
  if (gs_dict) {
    resources_dict->SetFor("ExtGState", gs_dict);
  }
  if (font_resource_dict) {
    resources_dict->SetFor("Font", font_resource_dict);
  }
  return resources_dict;
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

bool GenerateCircleAP(CPDF_Document* doc, CPDF_Dictionary* annot_dict) {
  fxcrt::ostringstream app_stream;
  app_stream << "/" << kGSDictName << " gs ";

  RetainPtr<const CPDF_Array> interior_color = annot_dict->GetArrayFor("IC");
  app_stream << GetColorStringWithDefault(
      interior_color.Get(), CFX_Color(CFX_Color::Type::kTransparent),
      PaintOperation::kFill);

  app_stream << GetColorStringWithDefault(
      annot_dict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  float border_width = GetBorderWidth(annot_dict);
  bool is_stroke_rect = border_width > 0;

  if (is_stroke_rect) {
    app_stream << border_width << " w ";
    app_stream << GetDashPatternString(annot_dict);
  }

  CFX_FloatRect rect = annot_dict->GetRectFor(pdfium::annotation::kRect);
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

  bool is_fill_rect = interior_color && !interior_color->IsEmpty();
  app_stream << GetPaintOperatorString(is_stroke_rect, is_fill_rect) << "\n";

  auto gs_dict = GenerateExtGStateDict(*annot_dict, "Normal");
  auto resources_dict = GenerateResourcesDict(doc, std::move(gs_dict), nullptr);
  GenerateAndSetAPDict(doc, annot_dict, &app_stream, std::move(resources_dict),
                       false /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateHighlightAP(CPDF_Document* doc, CPDF_Dictionary* annot_dict) {
  fxcrt::ostringstream app_stream;
  app_stream << "/" << kGSDictName << " gs ";

  app_stream << GetColorStringWithDefault(
      annot_dict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 1, 1, 0), PaintOperation::kFill);

  RetainPtr<const CPDF_Array> quad_points_array =
      annot_dict->GetArrayFor("QuadPoints");
  if (quad_points_array) {
    const size_t quad_point_count =
        CPDF_Annot::QuadPointCount(quad_points_array.Get());
    for (size_t i = 0; i < quad_point_count; ++i) {
      CFX_FloatRect rect = CPDF_Annot::RectFromQuadPoints(annot_dict, i);
      rect.Normalize();

      app_stream << rect.left << " " << rect.top << " m " << rect.right << " "
                 << rect.top << " l " << rect.right << " " << rect.bottom
                 << " l " << rect.left << " " << rect.bottom << " l h f\n";
    }
  }

  auto gs_dict = GenerateExtGStateDict(*annot_dict, "Multiply");
  auto resources_dict = GenerateResourcesDict(doc, std::move(gs_dict), nullptr);
  GenerateAndSetAPDict(doc, annot_dict, &app_stream, std::move(resources_dict),
                       true /*IsTextMarkupAnnotation*/);

  return true;
}

bool GenerateInkAP(CPDF_Document* doc, CPDF_Dictionary* annot_dict) {
  RetainPtr<const CPDF_Array> ink_list = annot_dict->GetArrayFor("InkList");
  if (!ink_list || ink_list->IsEmpty()) {
    return false;
  }

  float border_width = GetBorderWidth(annot_dict);
  const bool is_stroke = border_width > 0;
  if (!is_stroke) {
    return false;
  }

  fxcrt::ostringstream app_stream;
  app_stream << "/" << kGSDictName << " gs ";
  app_stream << GetColorStringWithDefault(
      annot_dict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  app_stream << border_width << " w ";
  app_stream << GetDashPatternString(annot_dict);

  // Set inflated rect as a new rect because paths near the border with large
  // width should not be clipped to the original rect.
  CFX_FloatRect rect = annot_dict->GetRectFor(pdfium::annotation::kRect);
  rect.Inflate(border_width / 2, border_width / 2);
  annot_dict->SetRectFor(pdfium::annotation::kRect, rect);

  for (size_t i = 0; i < ink_list->size(); i++) {
    RetainPtr<const CPDF_Array> coordinates_array = ink_list->GetArrayAt(i);
    if (!coordinates_array || coordinates_array->size() < 2) {
      continue;
    }

    app_stream << coordinates_array->GetFloatAt(0) << " "
               << coordinates_array->GetFloatAt(1) << " m ";

    for (size_t j = 0; j < coordinates_array->size() - 1; j += 2) {
      app_stream << coordinates_array->GetFloatAt(j) << " "
                 << coordinates_array->GetFloatAt(j + 1) << " l ";
    }

    app_stream << "S\n";
  }

  auto gs_dict = GenerateExtGStateDict(*annot_dict, "Normal");
  auto resources_dict = GenerateResourcesDict(doc, std::move(gs_dict), nullptr);
  GenerateAndSetAPDict(doc, annot_dict, &app_stream, std::move(resources_dict),
                       false /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateTextAP(CPDF_Document* doc, CPDF_Dictionary* annot_dict) {
  fxcrt::ostringstream app_stream;
  app_stream << "/" << kGSDictName << " gs ";

  CFX_FloatRect rect = annot_dict->GetRectFor(pdfium::annotation::kRect);
  const float note_length = 20;
  CFX_FloatRect note_rect(rect.left, rect.bottom, rect.left + note_length,
                          rect.bottom + note_length);
  annot_dict->SetRectFor(pdfium::annotation::kRect, note_rect);

  app_stream << GenerateTextSymbolAP(note_rect);

  auto gs_dict = GenerateExtGStateDict(*annot_dict, "Normal");
  auto resources_dict = GenerateResourcesDict(doc, std::move(gs_dict), nullptr);
  GenerateAndSetAPDict(doc, annot_dict, &app_stream, std::move(resources_dict),
                       false /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateUnderlineAP(CPDF_Document* doc, CPDF_Dictionary* annot_dict) {
  fxcrt::ostringstream app_stream;
  app_stream << "/" << kGSDictName << " gs ";

  app_stream << GetColorStringWithDefault(
      annot_dict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  RetainPtr<const CPDF_Array> quad_points_array =
      annot_dict->GetArrayFor("QuadPoints");
  if (quad_points_array) {
    static constexpr int kLineWidth = 1;
    app_stream << kLineWidth << " w ";
    const size_t quad_point_count =
        CPDF_Annot::QuadPointCount(quad_points_array.Get());
    for (size_t i = 0; i < quad_point_count; ++i) {
      CFX_FloatRect rect = CPDF_Annot::RectFromQuadPoints(annot_dict, i);
      rect.Normalize();
      app_stream << rect.left << " " << rect.bottom + kLineWidth << " m "
                 << rect.right << " " << rect.bottom + kLineWidth << " l S\n";
    }
  }

  auto gs_dict = GenerateExtGStateDict(*annot_dict, "Normal");
  auto resources_dict = GenerateResourcesDict(doc, std::move(gs_dict), nullptr);
  GenerateAndSetAPDict(doc, annot_dict, &app_stream, std::move(resources_dict),
                       true /*IsTextMarkupAnnotation*/);
  return true;
}

bool GeneratePopupAP(CPDF_Document* doc, CPDF_Dictionary* annot_dict) {
  fxcrt::ostringstream app_stream;
  app_stream << "/" << kGSDictName << " gs\n";

  app_stream << GenerateColorAP(CFX_Color(CFX_Color::Type::kRGB, 1, 1, 0),
                                PaintOperation::kFill);
  app_stream << GenerateColorAP(CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0),
                                PaintOperation::kStroke);

  const float border_width = 1;
  app_stream << border_width << " w\n";

  CFX_FloatRect rect = annot_dict->GetRectFor(pdfium::annotation::kRect);
  rect.Normalize();
  rect.Deflate(border_width / 2, border_width / 2);

  app_stream << rect.left << " " << rect.bottom << " " << rect.Width() << " "
             << rect.Height() << " re b\n";

  RetainPtr<CPDF_Dictionary> font_dict = GenerateFallbackFontDict(doc);
  auto* doc_page_data = CPDF_DocPageData::FromDocument(doc);
  RetainPtr<CPDF_Font> default_font = doc_page_data->GetFont(font_dict);
  if (!default_font) {
    return false;
  }

  const ByteString font_name = "FONT";
  RetainPtr<CPDF_Dictionary> resource_font_dict =
      GenerateResourceFontDict(doc, font_name, font_dict->GetObjNum());
  RetainPtr<CPDF_Dictionary> gs_dict =
      GenerateExtGStateDict(*annot_dict, "Normal");
  RetainPtr<CPDF_Dictionary> resources_dict = GenerateResourcesDict(
      doc, std::move(gs_dict), std::move(resource_font_dict));

  app_stream << GetPopupContentsString(doc, *annot_dict,
                                       std::move(default_font), font_name);
  GenerateAndSetAPDict(doc, annot_dict, &app_stream, std::move(resources_dict),
                       false /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateSquareAP(CPDF_Document* doc, CPDF_Dictionary* annot_dict) {
  fxcrt::ostringstream app_stream;
  app_stream << "/" << kGSDictName << " gs ";

  RetainPtr<const CPDF_Array> interior_color = annot_dict->GetArrayFor("IC");
  app_stream << GetColorStringWithDefault(
      interior_color.Get(), CFX_Color(CFX_Color::Type::kTransparent),
      PaintOperation::kFill);

  app_stream << GetColorStringWithDefault(
      annot_dict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  float border_width = GetBorderWidth(annot_dict);
  const bool is_stroke_rect = border_width > 0;
  if (is_stroke_rect) {
    app_stream << border_width << " w ";
    app_stream << GetDashPatternString(annot_dict);
  }

  CFX_FloatRect rect = annot_dict->GetRectFor(pdfium::annotation::kRect);
  rect.Normalize();

  if (is_stroke_rect) {
    // Deflating rect because stroking a path entails painting all points
    // whose perpendicular distance from the path in user space is less than
    // or equal to half the line width.
    rect.Deflate(border_width / 2, border_width / 2);
  }

  const bool is_fill_rect = interior_color && (interior_color->size() > 0);
  app_stream << rect.left << " " << rect.bottom << " " << rect.Width() << " "
             << rect.Height() << " re "
             << GetPaintOperatorString(is_stroke_rect, is_fill_rect) << "\n";

  auto gs_dict = GenerateExtGStateDict(*annot_dict, "Normal");
  auto resources_dict = GenerateResourcesDict(doc, std::move(gs_dict), nullptr);
  GenerateAndSetAPDict(doc, annot_dict, &app_stream, std::move(resources_dict),
                       false /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateSquigglyAP(CPDF_Document* doc, CPDF_Dictionary* annot_dict) {
  fxcrt::ostringstream app_stream;
  app_stream << "/" << kGSDictName << " gs ";

  app_stream << GetColorStringWithDefault(
      annot_dict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  RetainPtr<const CPDF_Array> quad_points_array =
      annot_dict->GetArrayFor("QuadPoints");
  if (quad_points_array) {
    static constexpr int kLineWidth = 1;
    static constexpr int kDelta = 2;
    app_stream << kLineWidth << " w ";
    const size_t quad_point_count =
        CPDF_Annot::QuadPointCount(quad_points_array.Get());
    for (size_t i = 0; i < quad_point_count; ++i) {
      CFX_FloatRect rect = CPDF_Annot::RectFromQuadPoints(annot_dict, i);
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

  auto gs_dict = GenerateExtGStateDict(*annot_dict, "Normal");
  auto resources_dict = GenerateResourcesDict(doc, std::move(gs_dict), nullptr);
  GenerateAndSetAPDict(doc, annot_dict, &app_stream, std::move(resources_dict),
                       true /*IsTextMarkupAnnotation*/);
  return true;
}

bool GenerateStrikeOutAP(CPDF_Document* doc, CPDF_Dictionary* annot_dict) {
  fxcrt::ostringstream app_stream;
  app_stream << "/" << kGSDictName << " gs ";

  app_stream << GetColorStringWithDefault(
      annot_dict->GetArrayFor(pdfium::annotation::kC).Get(),
      CFX_Color(CFX_Color::Type::kRGB, 0, 0, 0), PaintOperation::kStroke);

  RetainPtr<const CPDF_Array> quad_points_array =
      annot_dict->GetArrayFor("QuadPoints");
  if (quad_points_array) {
    const size_t quad_point_count =
        CPDF_Annot::QuadPointCount(quad_points_array.Get());
    for (size_t i = 0; i < quad_point_count; ++i) {
      CFX_FloatRect rect = CPDF_Annot::RectFromQuadPoints(annot_dict, i);
      rect.Normalize();

      float y = (rect.top + rect.bottom) / 2;
      static constexpr int kLineWidth = 1;
      app_stream << kLineWidth << " w " << rect.left << " " << y << " m "
                 << rect.right << " " << y << " l S\n";
    }
  }

  auto gs_dict = GenerateExtGStateDict(*annot_dict, "Normal");
  auto resources_dict = GenerateResourcesDict(doc, std::move(gs_dict), nullptr);
  GenerateAndSetAPDict(doc, annot_dict, &app_stream, std::move(resources_dict),
                       true /*IsTextMarkupAnnotation*/);
  return true;
}

}  // namespace

// static
void CPDF_GenerateAP::GenerateFormAP(CPDF_Document* doc,
                                     CPDF_Dictionary* annot_dict,
                                     FormType type) {
  RetainPtr<CPDF_Dictionary> root_dict = doc->GetMutableRoot();
  if (!root_dict) {
    return;
  }

  RetainPtr<CPDF_Dictionary> form_dict =
      root_dict->GetMutableDictFor("AcroForm");
  if (!form_dict) {
    return;
  }

  ByteString default_appearance_string;
  RetainPtr<const CPDF_Object> default_appearance_object =
      CPDF_FormField::GetFieldAttrForDict(annot_dict, "DA");
  if (default_appearance_object) {
    default_appearance_string = default_appearance_object->GetString();
  }
  if (default_appearance_string.IsEmpty()) {
    default_appearance_string = form_dict->GetByteStringFor("DA");
  }
  if (default_appearance_string.IsEmpty()) {
    return;
  }

  CPDF_DefaultAppearance appearance(default_appearance_string);

  float font_size = 0;
  std::optional<ByteString> font = appearance.GetFont(&font_size);
  if (!font.has_value())
    return;

  ByteString font_name = font.value();

  CFX_Color text_color = fpdfdoc::CFXColorFromString(default_appearance_string);
  RetainPtr<CPDF_Dictionary> dr_dict = form_dict->GetMutableDictFor("DR");
  if (!dr_dict) {
    return;
  }

  RetainPtr<CPDF_Dictionary> dr_font_dict = dr_dict->GetMutableDictFor("Font");
  if (!ValidateFontResourceDict(dr_font_dict.Get())) {
    return;
  }

  RetainPtr<CPDF_Dictionary> font_dict =
      dr_font_dict->GetMutableDictFor(font_name);
  if (!font_dict) {
    font_dict = GenerateFallbackFontDict(doc);
    dr_font_dict->SetNewFor<CPDF_Reference>(font_name, doc,
                                            font_dict->GetObjNum());
  }
  auto* doc_page_data = CPDF_DocPageData::FromDocument(doc);
  RetainPtr<CPDF_Font> default_font = doc_page_data->GetFont(font_dict);
  if (!default_font) {
    return;
  }

  CFX_FloatRect annot_rect = annot_dict->GetRectFor(pdfium::annotation::kRect);
  RetainPtr<const CPDF_Dictionary> mk_dict = annot_dict->GetDictFor("MK");
  const int32_t rotate =
      mk_dict ? mk_dict->GetIntegerFor(pdfium::appearance::kR) : 0;

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

  const BorderStyleInfo border_style_info =
      GetBorderStyleInfo(annot_dict->GetDictFor("BS"));
  CFX_Color border_color;
  CFX_Color background_color;
  if (mk_dict) {
    RetainPtr<const CPDF_Array> border_color_array =
        mk_dict->GetArrayFor(pdfium::appearance::kBC);
    if (border_color_array) {
      border_color = fpdfdoc::CFXColorFromArray(*border_color_array);
    }
    RetainPtr<const CPDF_Array> background_color_array =
        mk_dict->GetArrayFor(pdfium::appearance::kBG);
    if (background_color_array) {
      background_color = fpdfdoc::CFXColorFromArray(*background_color_array);
    }
  }
  fxcrt::ostringstream app_stream;
  ByteString background =
      GenerateColorAP(background_color, PaintOperation::kFill);
  if (background.GetLength() > 0) {
    app_stream << "q\n" << background;
    WriteRect(app_stream, bbox_rect) << " re f\nQ\n";
  }
  ByteString border_stream =
      GenerateBorderAP(bbox_rect, border_style_info, border_color);
  if (border_stream.GetLength() > 0) {
    app_stream << "q\n" << border_stream << "Q\n";
  }

  CFX_FloatRect body_rect = bbox_rect;
  body_rect.Deflate(border_style_info.width, border_style_info.width);

  RetainPtr<CPDF_Dictionary> ap_dict =
      annot_dict->GetOrCreateDictFor(pdfium::annotation::kAP);
  RetainPtr<CPDF_Stream> normal_stream = ap_dict->GetMutableStreamFor("N");
  RetainPtr<CPDF_Dictionary> stream_dict;
  if (normal_stream) {
    stream_dict = normal_stream->GetMutableDict();
    RetainPtr<CPDF_Dictionary> resources_dict =
        stream_dict->GetMutableDictFor("Resources");
    if (resources_dict) {
      RetainPtr<CPDF_Dictionary> font_resource_dict =
          resources_dict->GetMutableDictFor("Font");
      if (font_resource_dict) {
        if (!ValidateFontResourceDict(font_resource_dict.Get())) {
          return;
        }
      } else {
        font_resource_dict = resources_dict->SetNewFor<CPDF_Dictionary>("Font");
      }
      if (!font_resource_dict->KeyExist(font_name)) {
        font_resource_dict->SetNewFor<CPDF_Reference>(font_name, doc,
                                                      font_dict->GetObjNum());
      }
    } else {
      stream_dict->SetFor("Resources", form_dict->GetDictFor("DR")->Clone());
    }
    stream_dict->SetMatrixFor("Matrix", matrix);
    stream_dict->SetRectFor("BBox", bbox_rect);
  } else {
    normal_stream =
        doc->NewIndirect<CPDF_Stream>(pdfium::MakeRetain<CPDF_Dictionary>());
    ap_dict->SetNewFor<CPDF_Reference>("N", doc, normal_stream->GetObjNum());
  }
  CPVT_FontMap map(
      doc, stream_dict ? stream_dict->GetMutableDictFor("Resources") : nullptr,
      std::move(default_font), font_name);
  CPVT_VariableText::Provider prd(&map);

  switch (type) {
    case CPDF_GenerateAP::kTextField: {
      RetainPtr<const CPDF_Object> v_field =
          CPDF_FormField::GetFieldAttrForDict(annot_dict,
                                              pdfium::form_fields::kV);
      WideString value = v_field ? v_field->GetUnicodeText() : WideString();
      RetainPtr<const CPDF_Object> q_field =
          CPDF_FormField::GetFieldAttrForDict(annot_dict, "Q");
      const int32_t align = q_field ? q_field->GetInteger() : 0;
      RetainPtr<const CPDF_Object> ff_field =
          CPDF_FormField::GetFieldAttrForDict(annot_dict,
                                              pdfium::form_fields::kFf);
      const uint32_t flags = ff_field ? ff_field->GetInteger() : 0;
      RetainPtr<const CPDF_Object> max_len_field =
          CPDF_FormField::GetFieldAttrForDict(annot_dict, "MaxLen");
      const uint32_t max_len = max_len_field ? max_len_field->GetInteger() : 0;
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
      RetainPtr<const CPDF_Object> v_field =
          CPDF_FormField::GetFieldAttrForDict(annot_dict,
                                              pdfium::form_fields::kV);
      WideString value = v_field ? v_field->GetUnicodeText() : WideString();
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
        static const BorderStyleInfo kButtonBorderStyleInfo{
            .width = 2, .style = BorderStyle::kBeveled, .dash_pattern{3, 0, 0}};
        ByteString button_border =
            GenerateBorderAP(button_rect, kButtonBorderStyleInfo,
                             CFX_Color(CFX_Color::Type::kGray, 0));
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
      RetainPtr<const CPDF_Array> opts =
          ToArray(CPDF_FormField::GetFieldAttrForDict(annot_dict, "Opt"));
      RetainPtr<const CPDF_Array> selections =
          ToArray(CPDF_FormField::GetFieldAttrForDict(annot_dict, "I"));
      RetainPtr<const CPDF_Object> top_index =
          CPDF_FormField::GetFieldAttrForDict(annot_dict, "TI");
      const int32_t top = top_index ? top_index->GetInteger() : 0;
      fxcrt::ostringstream body_stream;
      if (opts) {
        float fy = body_rect.top;
        for (size_t i = top, sz = opts->size(); i < sz; i++) {
          if (FXSYS_IsFloatSmaller(fy, body_rect.bottom)) {
            break;
          }

          if (RetainPtr<const CPDF_Object> opt = opts->GetDirectObjectAt(i)) {
            WideString item;
            if (opt->IsString()) {
              item = opt->GetUnicodeText();
            } else if (const CPDF_Array* opt_array = opt->AsArray()) {
              RetainPtr<const CPDF_Object> opt_item =
                  opt_array->GetDirectObjectAt(1);
              if (opt_item) {
                item = opt_item->GetUnicodeText();
              }
            }
            bool is_selected = false;
            if (selections) {
              for (size_t s = 0, ssz = selections->size(); s < ssz; s++) {
                int value = selections->GetIntegerAt(s);
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

  if (!normal_stream) {
    return;
  }

  normal_stream->SetDataFromStringstreamAndRemoveFilter(&app_stream);
  stream_dict = normal_stream->GetMutableDict();
  stream_dict->SetMatrixFor("Matrix", matrix);
  stream_dict->SetRectFor("BBox", bbox_rect);
  RetainPtr<CPDF_Dictionary> resources_dict =
      stream_dict->GetMutableDictFor("Resources");
  if (!resources_dict) {
    stream_dict->SetFor("Resources", form_dict->GetDictFor("DR")->Clone());
    return;
  }

  RetainPtr<CPDF_Dictionary> font_resource_dict =
      resources_dict->GetMutableDictFor("Font");
  if (font_resource_dict) {
    if (!ValidateFontResourceDict(font_resource_dict.Get())) {
      return;
    }
  } else {
    font_resource_dict = resources_dict->SetNewFor<CPDF_Dictionary>("Font");
  }

  if (!font_resource_dict->KeyExist(font_name)) {
    font_resource_dict->SetNewFor<CPDF_Reference>(font_name, doc,
                                                  font_dict->GetObjNum());
  }
}

// static
void CPDF_GenerateAP::GenerateEmptyAP(CPDF_Document* doc,
                                      CPDF_Dictionary* annot_dict) {
  auto gs_dict = GenerateExtGStateDict(*annot_dict, "Normal");
  auto resources_dict = GenerateResourcesDict(doc, std::move(gs_dict), nullptr);

  fxcrt::ostringstream stream;
  GenerateAndSetAPDict(doc, annot_dict, &stream, std::move(resources_dict),
                       false);
}

// static
bool CPDF_GenerateAP::GenerateAnnotAP(CPDF_Document* doc,
                                      CPDF_Dictionary* annot_dict,
                                      CPDF_Annot::Subtype subtype) {
  switch (subtype) {
    case CPDF_Annot::Subtype::CIRCLE:
      return GenerateCircleAP(doc, annot_dict);
    case CPDF_Annot::Subtype::HIGHLIGHT:
      return GenerateHighlightAP(doc, annot_dict);
    case CPDF_Annot::Subtype::INK:
      return GenerateInkAP(doc, annot_dict);
    case CPDF_Annot::Subtype::POPUP:
      return GeneratePopupAP(doc, annot_dict);
    case CPDF_Annot::Subtype::SQUARE:
      return GenerateSquareAP(doc, annot_dict);
    case CPDF_Annot::Subtype::SQUIGGLY:
      return GenerateSquigglyAP(doc, annot_dict);
    case CPDF_Annot::Subtype::STRIKEOUT:
      return GenerateStrikeOutAP(doc, annot_dict);
    case CPDF_Annot::Subtype::TEXT:
      return GenerateTextAP(doc, annot_dict);
    case CPDF_Annot::Subtype::UNDERLINE:
      return GenerateUnderlineAP(doc, annot_dict);
    default:
      return false;
  }
}
