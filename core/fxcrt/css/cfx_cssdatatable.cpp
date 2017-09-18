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

const CFX_CSSPropertyTable* CFX_GetCSSPropertyByName(
    const WideStringView& wsName) {
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
