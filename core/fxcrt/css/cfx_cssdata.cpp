// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssdata.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/css/cfx_cssstyleselector.h"
#include "core/fxcrt/css/cfx_cssvaluelistparser.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"

namespace {

#undef PROP
#define PROP(a, b, c, d) a, c, d

const CFX_CSSData::Property propertyTable[] = {
    {PROP(CFX_CSSProperty::BorderLeft,
          L"border-left",
          0x04080036,
          CFX_CSSVALUETYPE_Shorthand)},
    {PROP(CFX_CSSProperty::Top,
          L"top",
          0x0BEDAF33,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::Margin,
          L"margin",
          0x0CB016BE,
          CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::TextIndent,
          L"text-indent",
          0x169ADB74,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::Right,
          L"right",
          0x193ADE3E,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::PaddingLeft,
          L"padding-left",
          0x228CF02F,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::MarginLeft,
          L"margin-left",
          0x297C5656,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber |
              CFX_CSSVALUETYPE_MaybeEnum)},
    {PROP(CFX_CSSProperty::Border,
          L"border",
          0x2A23349E,
          CFX_CSSVALUETYPE_Shorthand)},
    {PROP(CFX_CSSProperty::BorderTop,
          L"border-top",
          0x2B866ADE,
          CFX_CSSVALUETYPE_Shorthand)},
    {PROP(CFX_CSSProperty::Bottom,
          L"bottom",
          0x399F02B5,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::PaddingRight,
          L"padding-right",
          0x3F616AC2,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::BorderBottom,
          L"border-bottom",
          0x452CE780,
          CFX_CSSVALUETYPE_Shorthand)},
    {PROP(CFX_CSSProperty::FontFamily,
          L"font-family",
          0x574686E6,
          CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeString)},
    {PROP(CFX_CSSProperty::FontWeight,
          L"font-weight",
          0x6692F60C,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::Color,
          L"color",
          0x6E67921F,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeColor)},
    {PROP(CFX_CSSProperty::LetterSpacing,
          L"letter-spacing",
          0x70536102,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::TextAlign,
          L"text-align",
          0x7553F1BD,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum)},
    {PROP(CFX_CSSProperty::BorderRightWidth,
          L"border-right-width",
          0x8F5A6036,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::VerticalAlign,
          L"vertical-align",
          0x934A87D2,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::PaddingTop,
          L"padding-top",
          0x959D22B7,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::FontVariant,
          L"font-variant",
          0x9C785779,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum)},
    {PROP(CFX_CSSProperty::BorderWidth,
          L"border-width",
          0xA8DE4FEB,
          CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::BorderBottomWidth,
          L"border-bottom-width",
          0xAE41204D,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::BorderRight,
          L"border-right",
          0xB78E9EA9,
          CFX_CSSVALUETYPE_Shorthand)},
    {PROP(CFX_CSSProperty::FontSize,
          L"font-size",
          0xB93956DF,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::BorderSpacing,
          L"border-spacing",
          0xC72030F0,
          CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::FontStyle,
          L"font-style",
          0xCB1950F5,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum)},
    {PROP(CFX_CSSProperty::Font,
          L"font",
          0xCD308B77,
          CFX_CSSVALUETYPE_Shorthand)},
    {PROP(CFX_CSSProperty::LineHeight,
          L"line-height",
          0xCFCACE2E,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::MarginRight,
          L"margin-right",
          0xD13C58C9,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber |
              CFX_CSSVALUETYPE_MaybeEnum)},
    {PROP(CFX_CSSProperty::BorderLeftWidth,
          L"border-left-width",
          0xD1E93D83,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::Display,
          L"display",
          0xD4224C36,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum)},
    {PROP(CFX_CSSProperty::PaddingBottom,
          L"padding-bottom",
          0xE555B3B9,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::BorderTopWidth,
          L"border-top-width",
          0xED2CB62B,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::WordSpacing,
          L"word-spacing",
          0xEDA63BAE,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::Left,
          L"left",
          0xF5AD782B,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
              CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::TextDecoration,
          L"text-decoration",
          0xF7C634BA,
          CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeEnum)},
    {PROP(CFX_CSSProperty::Padding,
          L"padding",
          0xF8C373F7,
          CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeNumber)},
    {PROP(CFX_CSSProperty::MarginBottom,
          L"margin-bottom",
          0xF93485A0,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber |
              CFX_CSSVALUETYPE_MaybeEnum)},
    {PROP(CFX_CSSProperty::MarginTop,
          L"margin-top",
          0xFE51DCFE,
          CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber |
              CFX_CSSVALUETYPE_MaybeEnum)},
};

#undef PROP

#undef PVAL
#define PVAL(a, b, c) a, c

const CFX_CSSData::PropertyValue propertyValueTable[] = {
    {PVAL(CFX_CSSPropertyValue::Bolder, L"bolder", 0x009F1058)},
    {PVAL(CFX_CSSPropertyValue::None, L"none", 0x048B6670)},
    {PVAL(CFX_CSSPropertyValue::Dot, L"dot", 0x0A48CB27)},
    {PVAL(CFX_CSSPropertyValue::Sub, L"sub", 0x0BD37FAA)},
    {PVAL(CFX_CSSPropertyValue::Top, L"top", 0x0BEDAF33)},
    {PVAL(CFX_CSSPropertyValue::Right, L"right", 0x193ADE3E)},
    {PVAL(CFX_CSSPropertyValue::Normal, L"normal", 0x247CF3E9)},
    {PVAL(CFX_CSSPropertyValue::Auto, L"auto", 0x2B35B6D9)},
    {PVAL(CFX_CSSPropertyValue::Text, L"text", 0x2D08AF85)},
    {PVAL(CFX_CSSPropertyValue::XSmall, L"x-small", 0x2D2FCAFE)},
    {PVAL(CFX_CSSPropertyValue::Thin, L"thin", 0x2D574D53)},
    {PVAL(CFX_CSSPropertyValue::Small, L"small", 0x316A3739)},
    {PVAL(CFX_CSSPropertyValue::Bottom, L"bottom", 0x399F02B5)},
    {PVAL(CFX_CSSPropertyValue::Underline, L"underline", 0x3A0273A6)},
    {PVAL(CFX_CSSPropertyValue::Double, L"double", 0x3D98515B)},
    {PVAL(CFX_CSSPropertyValue::Lighter, L"lighter", 0x45BEB7AF)},
    {PVAL(CFX_CSSPropertyValue::Oblique, L"oblique", 0x53EBDDB1)},
    {PVAL(CFX_CSSPropertyValue::Super, L"super", 0x6A4F842F)},
    {PVAL(CFX_CSSPropertyValue::Center, L"center", 0x6C51AFC1)},
    {PVAL(CFX_CSSPropertyValue::XxLarge, L"xx-large", 0x70BB1508)},
    {PVAL(CFX_CSSPropertyValue::Smaller, L"smaller", 0x849769F0)},
    {PVAL(CFX_CSSPropertyValue::Baseline, L"baseline", 0x87436BA3)},
    {PVAL(CFX_CSSPropertyValue::Thick, L"thick", 0x8CC35EB3)},
    {PVAL(CFX_CSSPropertyValue::Justify, L"justify", 0x8D269CAE)},
    {PVAL(CFX_CSSPropertyValue::Middle, L"middle", 0x947FA00F)},
    {PVAL(CFX_CSSPropertyValue::Medium, L"medium", 0xA084A381)},
    {PVAL(CFX_CSSPropertyValue::ListItem, L"list-item", 0xA32382B8)},
    {PVAL(CFX_CSSPropertyValue::XxSmall, L"xx-small", 0xADE1FC76)},
    {PVAL(CFX_CSSPropertyValue::Bold, L"bold", 0xB18313A1)},
    {PVAL(CFX_CSSPropertyValue::SmallCaps, L"small-caps", 0xB299428D)},
    {PVAL(CFX_CSSPropertyValue::Inline, L"inline", 0xC02D649F)},
    {PVAL(CFX_CSSPropertyValue::Overline, L"overline", 0xC0EC9FA4)},
    {PVAL(CFX_CSSPropertyValue::TextBottom, L"text-bottom", 0xC7D08D87)},
    {PVAL(CFX_CSSPropertyValue::Larger, L"larger", 0xCD3C409D)},
    {PVAL(CFX_CSSPropertyValue::InlineTable, L"inline-table", 0xD131F494)},
    {PVAL(CFX_CSSPropertyValue::InlineBlock, L"inline-block", 0xD26A8BD7)},
    {PVAL(CFX_CSSPropertyValue::Blink, L"blink", 0xDC36E390)},
    {PVAL(CFX_CSSPropertyValue::Block, L"block", 0xDCD480AB)},
    {PVAL(CFX_CSSPropertyValue::Italic, L"italic", 0xE31D5396)},
    {PVAL(CFX_CSSPropertyValue::LineThrough, L"line-through", 0xE4C5A276)},
    {PVAL(CFX_CSSPropertyValue::XLarge, L"x-large", 0xF008E390)},
    {PVAL(CFX_CSSPropertyValue::Large, L"large", 0xF4434FCB)},
    {PVAL(CFX_CSSPropertyValue::Left, L"left", 0xF5AD782B)},
    {PVAL(CFX_CSSPropertyValue::TextTop, L"text-top", 0xFCB58D45)},
};

#undef PVAL

const CFX_CSSData::LengthUnit lengthUnitTable[] = {
    {L"cm", CFX_CSSNumberType::CentiMeters}, {L"em", CFX_CSSNumberType::EMS},
    {L"ex", CFX_CSSNumberType::EXS},         {L"in", CFX_CSSNumberType::Inches},
    {L"mm", CFX_CSSNumberType::MilliMeters}, {L"pc", CFX_CSSNumberType::Picas},
    {L"pt", CFX_CSSNumberType::Points},      {L"px", CFX_CSSNumberType::Pixels},
};

// 16 colours from CSS 2.0 + alternate spelling of grey/gray.
const CFX_CSSData::Color colorTable[] = {
    {L"aqua", 0xff00ffff},    {L"black", 0xff000000}, {L"blue", 0xff0000ff},
    {L"fuchsia", 0xffff00ff}, {L"gray", 0xff808080},  {L"green", 0xff008000},
    {L"grey", 0xff808080},    {L"lime", 0xff00ff00},  {L"maroon", 0xff800000},
    {L"navy", 0xff000080},    {L"olive", 0xff808000}, {L"orange", 0xffffa500},
    {L"purple", 0xff800080},  {L"red", 0xffff0000},   {L"silver", 0xffc0c0c0},
    {L"teal", 0xff008080},    {L"white", 0xffffffff}, {L"yellow", 0xffffff00},
};

}  // namespace

const CFX_CSSData::Property* CFX_CSSData::GetPropertyByName(
    WideStringView name) {
  if (name.IsEmpty())
    return nullptr;

  uint32_t hash = FX_HashCode_GetW(name, true);
  auto* result =
      std::lower_bound(std::begin(propertyTable), std::end(propertyTable), hash,
                       [](const CFX_CSSData::Property& iter,
                          const uint32_t& hash) { return iter.dwHash < hash; });

  if (result != std::end(propertyTable) && result->dwHash == hash)
    return result;
  return nullptr;
}

const CFX_CSSData::Property* CFX_CSSData::GetPropertyByEnum(
    CFX_CSSProperty property) {
  return &propertyTable[static_cast<uint8_t>(property)];
}

const CFX_CSSData::PropertyValue* CFX_CSSData::GetPropertyValueByName(
    WideStringView wsName) {
  if (wsName.IsEmpty())
    return nullptr;

  uint32_t hash = FX_HashCode_GetW(wsName, true);
  auto* result = std::lower_bound(
      std::begin(propertyValueTable), std::end(propertyValueTable), hash,
      [](const PropertyValue& iter, const uint32_t& hash) {
        return iter.dwHash < hash;
      });

  if (result != std::end(propertyValueTable) && result->dwHash == hash)
    return result;
  return nullptr;
}

const CFX_CSSData::LengthUnit* CFX_CSSData::GetLengthUnitByName(
    WideStringView wsName) {
  if (wsName.IsEmpty() || wsName.GetLength() != 2)
    return nullptr;

  WideString lowerName = WideString(wsName);
  lowerName.MakeLower();

  for (auto* iter = std::begin(lengthUnitTable);
       iter != std::end(lengthUnitTable); ++iter) {
    if (lowerName.Compare(iter->value) == 0)
      return iter;
  }

  return nullptr;
}

const CFX_CSSData::Color* CFX_CSSData::GetColorByName(WideStringView wsName) {
  if (wsName.IsEmpty())
    return nullptr;

  WideString lowerName = WideString(wsName);
  lowerName.MakeLower();

  for (auto* iter = std::begin(colorTable); iter != std::end(colorTable);
       ++iter) {
    if (lowerName.Compare(iter->name) == 0)
      return iter;
  }
  return nullptr;
}
