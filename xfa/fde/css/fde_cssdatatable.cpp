// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_cssdatatable.h"

#include <utility>

#include "core/fxcrt/fx_ext.h"
#include "xfa/fde/css/cfde_cssstyleselector.h"
#include "xfa/fde/css/cfde_cssvaluelistparser.h"
#include "xfa/fgas/crt/fgas_codepage.h"

static const FDE_CSSPropertyTable g_FDE_CSSProperties[] = {
    {FDE_CSSProperty::BorderLeft, L"border-left", 0x04080036,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::Top, L"top", 0x0BEDAF33,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Margin, L"margin", 0x0CB016BE,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::TextIndent, L"text-indent", 0x169ADB74,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Right, L"right", 0x193ADE3E,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::PaddingLeft, L"padding-left", 0x228CF02F,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::MarginLeft, L"margin-left", 0x297C5656,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber |
         FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::Border, L"border", 0x2A23349E,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::BorderTop, L"border-top", 0x2B866ADE,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::Bottom, L"bottom", 0x399F02B5,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::PaddingRight, L"padding-right", 0x3F616AC2,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderBottom, L"border-bottom", 0x452CE780,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::FontFamily, L"font-family", 0x574686E6,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeString},
    {FDE_CSSProperty::FontWeight, L"font-weight", 0x6692F60C,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Color, L"color", 0x6E67921F,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeColor},
    {FDE_CSSProperty::LetterSpacing, L"letter-spacing", 0x70536102,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::TextAlign, L"text-align", 0x7553F1BD,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderRightWidth, L"border-right-width", 0x8F5A6036,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::VerticalAlign, L"vertical-align", 0x934A87D2,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::PaddingTop, L"padding-top", 0x959D22B7,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::FontVariant, L"font-variant", 0x9C785779,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderWidth, L"border-width", 0xA8DE4FEB,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderBottomWidth, L"border-bottom-width", 0xAE41204D,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderRight, L"border-right", 0xB78E9EA9,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::FontSize, L"font-size", 0xB93956DF,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderSpacing, L"border-spacing", 0xC72030F0,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::FontStyle, L"font-style", 0xCB1950F5,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::Font, L"font", 0xCD308B77, FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::LineHeight, L"line-height", 0xCFCACE2E,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::MarginRight, L"margin-right", 0xD13C58C9,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber |
         FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderLeftWidth, L"border-left-width", 0xD1E93D83,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Display, L"display", 0xD4224C36,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::PaddingBottom, L"padding-bottom", 0xE555B3B9,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderTopWidth, L"border-top-width", 0xED2CB62B,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::WordSpacing, L"word-spacing", 0xEDA63BAE,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Left, L"left", 0xF5AD782B,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::TextDecoration, L"text-decoration", 0xF7C634BA,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::Padding, L"padding", 0xF8C373F7,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::MarginBottom, L"margin-bottom", 0xF93485A0,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber |
         FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::MarginTop, L"margin-top", 0xFE51DCFE,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber |
         FDE_CSSVALUETYPE_MaybeEnum},
};
const int32_t g_iCSSPropertyCount =
    sizeof(g_FDE_CSSProperties) / sizeof(FDE_CSSPropertyTable);
static_assert(g_iCSSPropertyCount ==
                  static_cast<int32_t>(FDE_CSSProperty::LAST_MARKER),
              "Property table differs in size from property enum");

static const FDE_CSSPropertyValueTable g_FDE_CSSPropertyValues[] = {
    {FDE_CSSPropertyValue::Bolder, L"bolder", 0x009F1058},
    {FDE_CSSPropertyValue::None, L"none", 0x048B6670},
    {FDE_CSSPropertyValue::Dot, L"dot", 0x0A48CB27},
    {FDE_CSSPropertyValue::Sub, L"sub", 0x0BD37FAA},
    {FDE_CSSPropertyValue::Top, L"top", 0x0BEDAF33},
    {FDE_CSSPropertyValue::Right, L"right", 0x193ADE3E},
    {FDE_CSSPropertyValue::Normal, L"normal", 0x247CF3E9},
    {FDE_CSSPropertyValue::Auto, L"auto", 0x2B35B6D9},
    {FDE_CSSPropertyValue::Text, L"text", 0x2D08AF85},
    {FDE_CSSPropertyValue::XSmall, L"x-small", 0x2D2FCAFE},
    {FDE_CSSPropertyValue::Thin, L"thin", 0x2D574D53},
    {FDE_CSSPropertyValue::Small, L"small", 0x316A3739},
    {FDE_CSSPropertyValue::Bottom, L"bottom", 0x399F02B5},
    {FDE_CSSPropertyValue::Underline, L"underline", 0x3A0273A6},
    {FDE_CSSPropertyValue::Double, L"double", 0x3D98515B},
    {FDE_CSSPropertyValue::Lighter, L"lighter", 0x45BEB7AF},
    {FDE_CSSPropertyValue::Oblique, L"oblique", 0x53EBDDB1},
    {FDE_CSSPropertyValue::Super, L"super", 0x6A4F842F},
    {FDE_CSSPropertyValue::Center, L"center", 0x6C51AFC1},
    {FDE_CSSPropertyValue::XxLarge, L"xx-large", 0x70BB1508},
    {FDE_CSSPropertyValue::Smaller, L"smaller", 0x849769F0},
    {FDE_CSSPropertyValue::Baseline, L"baseline", 0x87436BA3},
    {FDE_CSSPropertyValue::Thick, L"thick", 0x8CC35EB3},
    {FDE_CSSPropertyValue::Justify, L"justify", 0x8D269CAE},
    {FDE_CSSPropertyValue::Middle, L"middle", 0x947FA00F},
    {FDE_CSSPropertyValue::Medium, L"medium", 0xA084A381},
    {FDE_CSSPropertyValue::ListItem, L"list-item", 0xA32382B8},
    {FDE_CSSPropertyValue::XxSmall, L"xx-small", 0xADE1FC76},
    {FDE_CSSPropertyValue::Bold, L"bold", 0xB18313A1},
    {FDE_CSSPropertyValue::SmallCaps, L"small-caps", 0xB299428D},
    {FDE_CSSPropertyValue::Inline, L"inline", 0xC02D649F},
    {FDE_CSSPropertyValue::Overline, L"overline", 0xC0EC9FA4},
    {FDE_CSSPropertyValue::TextBottom, L"text-bottom", 0xC7D08D87},
    {FDE_CSSPropertyValue::Larger, L"larger", 0xCD3C409D},
    {FDE_CSSPropertyValue::InlineTable, L"inline-table", 0xD131F494},
    {FDE_CSSPropertyValue::InlineBlock, L"inline-block", 0xD26A8BD7},
    {FDE_CSSPropertyValue::Blink, L"blink", 0xDC36E390},
    {FDE_CSSPropertyValue::Block, L"block", 0xDCD480AB},
    {FDE_CSSPropertyValue::Italic, L"italic", 0xE31D5396},
    {FDE_CSSPropertyValue::LineThrough, L"line-through", 0xE4C5A276},
    {FDE_CSSPropertyValue::XLarge, L"x-large", 0xF008E390},
    {FDE_CSSPropertyValue::Large, L"large", 0xF4434FCB},
    {FDE_CSSPropertyValue::Left, L"left", 0xF5AD782B},
    {FDE_CSSPropertyValue::TextTop, L"text-top", 0xFCB58D45},
};
const int32_t g_iCSSPropertyValueCount =
    sizeof(g_FDE_CSSPropertyValues) / sizeof(FDE_CSSPropertyValueTable);
static_assert(g_iCSSPropertyValueCount ==
                  static_cast<int32_t>(FDE_CSSPropertyValue::LAST_MARKER),
              "Property value table differs in size from property value enum");

static const FDE_CSSLengthUnitTable g_FDE_CSSLengthUnits[] = {
    {0x0672, FDE_CSSNumberType::EMS},
    {0x067D, FDE_CSSNumberType::EXS},
    {0x1AF7, FDE_CSSNumberType::Inches},
    {0x2F7A, FDE_CSSNumberType::MilliMeters},
    {0x3ED3, FDE_CSSNumberType::Picas},
    {0x3EE4, FDE_CSSNumberType::Points},
    {0x3EE8, FDE_CSSNumberType::Pixels},
    {0xFC30, FDE_CSSNumberType::CentiMeters},
};

static const FDE_CSSCOLORTABLE g_FDE_CSSColors[] = {
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

const FDE_CSSPropertyTable* FDE_GetCSSPropertyByName(
    const CFX_WideStringC& wsName) {
  ASSERT(!wsName.IsEmpty());
  uint32_t dwHash = FX_HashCode_GetW(wsName, true);
  int32_t iEnd = g_iCSSPropertyCount;
  int32_t iMid, iStart = 0;
  uint32_t dwMid;
  do {
    iMid = (iStart + iEnd) / 2;
    dwMid = g_FDE_CSSProperties[iMid].dwHash;
    if (dwHash == dwMid) {
      return g_FDE_CSSProperties + iMid;
    } else if (dwHash > dwMid) {
      iStart = iMid + 1;
    } else {
      iEnd = iMid - 1;
    }
  } while (iStart <= iEnd);
  return nullptr;
}

const FDE_CSSPropertyTable* FDE_GetCSSPropertyByEnum(FDE_CSSProperty eName) {
  return g_FDE_CSSProperties + static_cast<int>(eName);
}

const FDE_CSSPropertyValueTable* FDE_GetCSSPropertyValueByName(
    const CFX_WideStringC& wsName) {
  ASSERT(!wsName.IsEmpty());
  uint32_t dwHash = FX_HashCode_GetW(wsName, true);
  int32_t iEnd = g_iCSSPropertyValueCount;
  int32_t iMid, iStart = 0;
  uint32_t dwMid;
  do {
    iMid = (iStart + iEnd) / 2;
    dwMid = g_FDE_CSSPropertyValues[iMid].dwHash;
    if (dwHash == dwMid) {
      return g_FDE_CSSPropertyValues + iMid;
    } else if (dwHash > dwMid) {
      iStart = iMid + 1;
    } else {
      iEnd = iMid - 1;
    }
  } while (iStart <= iEnd);
  return nullptr;
}

const FDE_CSSLengthUnitTable* FDE_GetCSSLengthUnitByName(
    const CFX_WideStringC& wsName) {
  ASSERT(!wsName.IsEmpty());
  uint16_t wHash = FX_HashCode_GetW(wsName, true);
  int32_t iEnd =
      sizeof(g_FDE_CSSLengthUnits) / sizeof(FDE_CSSLengthUnitTable) - 1;
  int32_t iMid, iStart = 0;
  uint16_t wMid;
  do {
    iMid = (iStart + iEnd) / 2;
    wMid = g_FDE_CSSLengthUnits[iMid].wHash;
    if (wHash == wMid) {
      return g_FDE_CSSLengthUnits + iMid;
    } else if (wHash > wMid) {
      iStart = iMid + 1;
    } else {
      iEnd = iMid - 1;
    }
  } while (iStart <= iEnd);
  return nullptr;
}

const FDE_CSSCOLORTABLE* FDE_GetCSSColorByName(const CFX_WideStringC& wsName) {
  ASSERT(!wsName.IsEmpty());
  uint32_t dwHash = FX_HashCode_GetW(wsName, true);
  int32_t iEnd = sizeof(g_FDE_CSSColors) / sizeof(FDE_CSSCOLORTABLE) - 1;
  int32_t iMid, iStart = 0;
  uint32_t dwMid;
  do {
    iMid = (iStart + iEnd) / 2;
    dwMid = g_FDE_CSSColors[iMid].dwHash;
    if (dwHash == dwMid) {
      return g_FDE_CSSColors + iMid;
    } else if (dwHash > dwMid) {
      iStart = iMid + 1;
    } else {
      iEnd = iMid - 1;
    }
  } while (iStart <= iEnd);
  return nullptr;
}
