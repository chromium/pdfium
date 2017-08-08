// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssdatatable.h"

#include <utility>

#include "core/fxcrt/css/cfx_cssstyleselector.h"
#include "core/fxcrt/css/cfx_cssvaluelistparser.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"

static const CFX_CSSPropertyTable g_CFX_CSSProperties[] = {
    {CFX_CSSProperty::BorderLeft, L"border-left", 0x04080036,
     CFX_CSSVALUETYPE_Shorthand},
    {CFX_CSSProperty::Top, L"top", 0x0BEDAF33,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::Margin, L"margin", 0x0CB016BE,
     CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::TextIndent, L"text-indent", 0x169ADB74,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::Right, L"right", 0x193ADE3E,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::PaddingLeft, L"padding-left", 0x228CF02F,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::MarginLeft, L"margin-left", 0x297C5656,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber |
         CFX_CSSVALUETYPE_MaybeEnum},
    {CFX_CSSProperty::Border, L"border", 0x2A23349E,
     CFX_CSSVALUETYPE_Shorthand},
    {CFX_CSSProperty::BorderTop, L"border-top", 0x2B866ADE,
     CFX_CSSVALUETYPE_Shorthand},
    {CFX_CSSProperty::Bottom, L"bottom", 0x399F02B5,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::PaddingRight, L"padding-right", 0x3F616AC2,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::BorderBottom, L"border-bottom", 0x452CE780,
     CFX_CSSVALUETYPE_Shorthand},
    {CFX_CSSProperty::FontFamily, L"font-family", 0x574686E6,
     CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeString},
    {CFX_CSSProperty::FontWeight, L"font-weight", 0x6692F60C,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::Color, L"color", 0x6E67921F,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeColor},
    {CFX_CSSProperty::LetterSpacing, L"letter-spacing", 0x70536102,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::TextAlign, L"text-align", 0x7553F1BD,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum},
    {CFX_CSSProperty::BorderRightWidth, L"border-right-width", 0x8F5A6036,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::VerticalAlign, L"vertical-align", 0x934A87D2,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::PaddingTop, L"padding-top", 0x959D22B7,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::FontVariant, L"font-variant", 0x9C785779,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum},
    {CFX_CSSProperty::BorderWidth, L"border-width", 0xA8DE4FEB,
     CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::BorderBottomWidth, L"border-bottom-width", 0xAE41204D,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::BorderRight, L"border-right", 0xB78E9EA9,
     CFX_CSSVALUETYPE_Shorthand},
    {CFX_CSSProperty::FontSize, L"font-size", 0xB93956DF,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::BorderSpacing, L"border-spacing", 0xC72030F0,
     CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::FontStyle, L"font-style", 0xCB1950F5,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum},
    {CFX_CSSProperty::Font, L"font", 0xCD308B77, CFX_CSSVALUETYPE_Shorthand},
    {CFX_CSSProperty::LineHeight, L"line-height", 0xCFCACE2E,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::MarginRight, L"margin-right", 0xD13C58C9,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber |
         CFX_CSSVALUETYPE_MaybeEnum},
    {CFX_CSSProperty::BorderLeftWidth, L"border-left-width", 0xD1E93D83,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::Display, L"display", 0xD4224C36,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum},
    {CFX_CSSProperty::PaddingBottom, L"padding-bottom", 0xE555B3B9,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::BorderTopWidth, L"border-top-width", 0xED2CB62B,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::WordSpacing, L"word-spacing", 0xEDA63BAE,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::Left, L"left", 0xF5AD782B,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeEnum |
         CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::TextDecoration, L"text-decoration", 0xF7C634BA,
     CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeEnum},
    {CFX_CSSProperty::Padding, L"padding", 0xF8C373F7,
     CFX_CSSVALUETYPE_List | CFX_CSSVALUETYPE_MaybeNumber},
    {CFX_CSSProperty::MarginBottom, L"margin-bottom", 0xF93485A0,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber |
         CFX_CSSVALUETYPE_MaybeEnum},
    {CFX_CSSProperty::MarginTop, L"margin-top", 0xFE51DCFE,
     CFX_CSSVALUETYPE_Primitive | CFX_CSSVALUETYPE_MaybeNumber |
         CFX_CSSVALUETYPE_MaybeEnum},
};
const int32_t g_iCSSPropertyCount =
    sizeof(g_CFX_CSSProperties) / sizeof(CFX_CSSPropertyTable);
static_assert(g_iCSSPropertyCount ==
                  static_cast<int32_t>(CFX_CSSProperty::LAST_MARKER),
              "Property table differs in size from property enum");

static const CFX_CSSPropertyValueTable g_CFX_CSSPropertyValues[] = {
    {CFX_CSSPropertyValue::Bolder, L"bolder", 0x009F1058},
    {CFX_CSSPropertyValue::None, L"none", 0x048B6670},
    {CFX_CSSPropertyValue::Dot, L"dot", 0x0A48CB27},
    {CFX_CSSPropertyValue::Sub, L"sub", 0x0BD37FAA},
    {CFX_CSSPropertyValue::Top, L"top", 0x0BEDAF33},
    {CFX_CSSPropertyValue::Right, L"right", 0x193ADE3E},
    {CFX_CSSPropertyValue::Normal, L"normal", 0x247CF3E9},
    {CFX_CSSPropertyValue::Auto, L"auto", 0x2B35B6D9},
    {CFX_CSSPropertyValue::Text, L"text", 0x2D08AF85},
    {CFX_CSSPropertyValue::XSmall, L"x-small", 0x2D2FCAFE},
    {CFX_CSSPropertyValue::Thin, L"thin", 0x2D574D53},
    {CFX_CSSPropertyValue::Small, L"small", 0x316A3739},
    {CFX_CSSPropertyValue::Bottom, L"bottom", 0x399F02B5},
    {CFX_CSSPropertyValue::Underline, L"underline", 0x3A0273A6},
    {CFX_CSSPropertyValue::Double, L"double", 0x3D98515B},
    {CFX_CSSPropertyValue::Lighter, L"lighter", 0x45BEB7AF},
    {CFX_CSSPropertyValue::Oblique, L"oblique", 0x53EBDDB1},
    {CFX_CSSPropertyValue::Super, L"super", 0x6A4F842F},
    {CFX_CSSPropertyValue::Center, L"center", 0x6C51AFC1},
    {CFX_CSSPropertyValue::XxLarge, L"xx-large", 0x70BB1508},
    {CFX_CSSPropertyValue::Smaller, L"smaller", 0x849769F0},
    {CFX_CSSPropertyValue::Baseline, L"baseline", 0x87436BA3},
    {CFX_CSSPropertyValue::Thick, L"thick", 0x8CC35EB3},
    {CFX_CSSPropertyValue::Justify, L"justify", 0x8D269CAE},
    {CFX_CSSPropertyValue::Middle, L"middle", 0x947FA00F},
    {CFX_CSSPropertyValue::Medium, L"medium", 0xA084A381},
    {CFX_CSSPropertyValue::ListItem, L"list-item", 0xA32382B8},
    {CFX_CSSPropertyValue::XxSmall, L"xx-small", 0xADE1FC76},
    {CFX_CSSPropertyValue::Bold, L"bold", 0xB18313A1},
    {CFX_CSSPropertyValue::SmallCaps, L"small-caps", 0xB299428D},
    {CFX_CSSPropertyValue::Inline, L"inline", 0xC02D649F},
    {CFX_CSSPropertyValue::Overline, L"overline", 0xC0EC9FA4},
    {CFX_CSSPropertyValue::TextBottom, L"text-bottom", 0xC7D08D87},
    {CFX_CSSPropertyValue::Larger, L"larger", 0xCD3C409D},
    {CFX_CSSPropertyValue::InlineTable, L"inline-table", 0xD131F494},
    {CFX_CSSPropertyValue::InlineBlock, L"inline-block", 0xD26A8BD7},
    {CFX_CSSPropertyValue::Blink, L"blink", 0xDC36E390},
    {CFX_CSSPropertyValue::Block, L"block", 0xDCD480AB},
    {CFX_CSSPropertyValue::Italic, L"italic", 0xE31D5396},
    {CFX_CSSPropertyValue::LineThrough, L"line-through", 0xE4C5A276},
    {CFX_CSSPropertyValue::XLarge, L"x-large", 0xF008E390},
    {CFX_CSSPropertyValue::Large, L"large", 0xF4434FCB},
    {CFX_CSSPropertyValue::Left, L"left", 0xF5AD782B},
    {CFX_CSSPropertyValue::TextTop, L"text-top", 0xFCB58D45},
};
const int32_t g_iCSSPropertyValueCount =
    sizeof(g_CFX_CSSPropertyValues) / sizeof(CFX_CSSPropertyValueTable);
static_assert(g_iCSSPropertyValueCount ==
                  static_cast<int32_t>(CFX_CSSPropertyValue::LAST_MARKER),
              "Property value table differs in size from property value enum");

static const CFX_CSSLengthUnitTable g_CFX_CSSLengthUnits[] = {
    {0x0672, CFX_CSSNumberType::EMS},
    {0x067D, CFX_CSSNumberType::EXS},
    {0x1AF7, CFX_CSSNumberType::Inches},
    {0x2F7A, CFX_CSSNumberType::MilliMeters},
    {0x3ED3, CFX_CSSNumberType::Picas},
    {0x3EE4, CFX_CSSNumberType::Points},
    {0x3EE8, CFX_CSSNumberType::Pixels},
    {0xFC30, CFX_CSSNumberType::CentiMeters},
};

static const CFX_CSSCOLORTABLE g_CFX_CSSColors[] = {
    {0x031B47FE, 0xff000080}, {0x0BB8DF5B, 0xffff0000},
    {0x0D82A78C, 0xff800000}, {0x2ACC82E8, 0xff00ffff},
    {0x2D083986, 0xff008080}, {0x4A6A6195, 0xffc0c0c0},
    {0x546A8EF3, 0xff808080}, {0x65C9169C, 0xffffa500},
    {0x8422BB61, 0xffffffff}, {0x9271A558, 0xff800080},
    {0xA65A3EE3, 0xffff00ff}, {0xB1345708, 0xff0000ff},
    {0xB6D2CF1F, 0xff808000}, {0xD19B5E1C, 0xffffff00},
    {0xDB64391D, 0xff000000}, {0xF616D507, 0xff00ff00},
    {0xF6EFFF31, 0xff008000},
};

const CFX_CSSPropertyTable* CFX_GetCSSPropertyByName(
    const CFX_WideStringC& wsName) {
  ASSERT(!wsName.IsEmpty());
  uint32_t dwHash = FX_HashCode_GetW(wsName, true);
  int32_t iEnd = g_iCSSPropertyCount;
  int32_t iMid, iStart = 0;
  uint32_t dwMid;
  do {
    iMid = (iStart + iEnd) / 2;
    dwMid = g_CFX_CSSProperties[iMid].dwHash;
    if (dwHash == dwMid) {
      return g_CFX_CSSProperties + iMid;
    } else if (dwHash > dwMid) {
      iStart = iMid + 1;
    } else {
      iEnd = iMid - 1;
    }
  } while (iStart <= iEnd);
  return nullptr;
}

const CFX_CSSPropertyTable* CFX_GetCSSPropertyByEnum(CFX_CSSProperty eName) {
  return g_CFX_CSSProperties + static_cast<int>(eName);
}

const CFX_CSSPropertyValueTable* CFX_GetCSSPropertyValueByName(
    const CFX_WideStringC& wsName) {
  ASSERT(!wsName.IsEmpty());
  uint32_t dwHash = FX_HashCode_GetW(wsName, true);
  int32_t iEnd = g_iCSSPropertyValueCount;
  int32_t iMid, iStart = 0;
  uint32_t dwMid;
  do {
    iMid = (iStart + iEnd) / 2;
    dwMid = g_CFX_CSSPropertyValues[iMid].dwHash;
    if (dwHash == dwMid) {
      return g_CFX_CSSPropertyValues + iMid;
    } else if (dwHash > dwMid) {
      iStart = iMid + 1;
    } else {
      iEnd = iMid - 1;
    }
  } while (iStart <= iEnd);
  return nullptr;
}

const CFX_CSSLengthUnitTable* CFX_GetCSSLengthUnitByName(
    const CFX_WideStringC& wsName) {
  ASSERT(!wsName.IsEmpty());
  uint16_t wHash = FX_HashCode_GetW(wsName, true);
  int32_t iEnd =
      sizeof(g_CFX_CSSLengthUnits) / sizeof(CFX_CSSLengthUnitTable) - 1;
  int32_t iMid, iStart = 0;
  uint16_t wMid;
  do {
    iMid = (iStart + iEnd) / 2;
    wMid = g_CFX_CSSLengthUnits[iMid].wHash;
    if (wHash == wMid) {
      return g_CFX_CSSLengthUnits + iMid;
    } else if (wHash > wMid) {
      iStart = iMid + 1;
    } else {
      iEnd = iMid - 1;
    }
  } while (iStart <= iEnd);
  return nullptr;
}

const CFX_CSSCOLORTABLE* CFX_GetCSSColorByName(const CFX_WideStringC& wsName) {
  ASSERT(!wsName.IsEmpty());
  uint32_t dwHash = FX_HashCode_GetW(wsName, true);
  int32_t iEnd = sizeof(g_CFX_CSSColors) / sizeof(CFX_CSSCOLORTABLE) - 1;
  int32_t iMid, iStart = 0;
  uint32_t dwMid;
  do {
    iMid = (iStart + iEnd) / 2;
    dwMid = g_CFX_CSSColors[iMid].dwHash;
    if (dwHash == dwMid) {
      return g_CFX_CSSColors + iMid;
    } else if (dwHash > dwMid) {
      iStart = iMid + 1;
    } else {
      iEnd = iMid - 1;
    }
  } while (iStart <= iEnd);
  return nullptr;
}
