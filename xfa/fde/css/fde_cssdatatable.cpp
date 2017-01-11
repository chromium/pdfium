// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_cssdatatable.h"

#include "core/fxcrt/fx_ext.h"
#include "xfa/fgas/crt/fgas_codepage.h"

namespace {

uint8_t Hex2Dec(uint8_t hexHigh, uint8_t hexLow) {
  return (FXSYS_toHexDigit(hexHigh) << 4) + FXSYS_toHexDigit(hexLow);
}

}  // namespace

bool FDE_CSSLengthToFloat(const FDE_CSSLENGTH& len,
                          FX_FLOAT fPercentBase,
                          FX_FLOAT& fResult) {
  switch (len.GetUnit()) {
    case FDE_CSSLengthUnit::Point:
      fResult = len.GetValue();
      return true;
    case FDE_CSSLengthUnit::Percent:
      fResult = len.GetValue() * fPercentBase;
      return true;
    default:
      return false;
  }
}
CFX_FloatRect FDE_CSSBoundaryToRect(IFDE_CSSBoundaryStyle* pBoundStyle,
                                    FX_FLOAT fContainerWidth,
                                    bool bPadding,
                                    bool bBorder,
                                    bool bMargin) {
  FX_FLOAT fResult;
  const FDE_CSSRECT* pRect;
  CFX_FloatRect rect(0, 0, 0, 0);
  if (bPadding) {
    pRect = pBoundStyle->GetPaddingWidth();
    if (pRect) {
      if (FDE_CSSLengthToFloat(pRect->left, fContainerWidth, fResult)) {
        rect.left += fResult;
      }
      if (FDE_CSSLengthToFloat(pRect->top, fContainerWidth, fResult)) {
        rect.top += fResult;
      }
      if (FDE_CSSLengthToFloat(pRect->right, fContainerWidth, fResult)) {
        rect.right += fResult;
      }
      if (FDE_CSSLengthToFloat(pRect->bottom, fContainerWidth, fResult)) {
        rect.bottom += fResult;
      }
    }
  }
  if (bBorder) {
    pRect = pBoundStyle->GetBorderWidth();
    if (pRect) {
      if (FDE_CSSLengthToFloat(pRect->left, fContainerWidth, fResult)) {
        rect.left += fResult;
      }
      if (FDE_CSSLengthToFloat(pRect->top, fContainerWidth, fResult)) {
        rect.top += fResult;
      }
      if (FDE_CSSLengthToFloat(pRect->right, fContainerWidth, fResult)) {
        rect.right += fResult;
      }
      if (FDE_CSSLengthToFloat(pRect->bottom, fContainerWidth, fResult)) {
        rect.bottom += fResult;
      }
    }
  }
  if (bMargin) {
    pRect = pBoundStyle->GetMarginWidth();
    if (pRect) {
      if (FDE_CSSLengthToFloat(pRect->left, fContainerWidth, fResult)) {
        rect.left += fResult;
      }
      if (FDE_CSSLengthToFloat(pRect->top, fContainerWidth, fResult)) {
        rect.top += fResult;
      }
      if (FDE_CSSLengthToFloat(pRect->right, fContainerWidth, fResult)) {
        rect.right += fResult;
      }
      if (FDE_CSSLengthToFloat(pRect->bottom, fContainerWidth, fResult)) {
        rect.bottom += fResult;
      }
    }
  }
  return rect;
}
uint32_t FDE_CSSFontStyleToFDE(IFDE_CSSFontStyle* pFontStyle) {
  uint32_t dwFontStyle = FX_FONTSTYLE_Normal;
  if (pFontStyle->GetFontStyle() == FDE_CSSFontStyle::Italic) {
    dwFontStyle |= FX_FONTSTYLE_Italic;
  }
  if (pFontStyle->GetFontWeight() >= 700) {
    dwFontStyle |= FX_FONTSTYLE_Bold;
  }
  return dwFontStyle;
}

static const FDE_CSSPropertyTable g_FDE_CSSProperties[] = {
    {FDE_CSSProperty::WritingMode, L"writing-mode", 0x01878076,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::ColumnRuleWidth, L"column-rule-width", 0x0200FB00,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderLeft, L"border-left", 0x04080036,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::ColumnRule, L"column-rule", 0x04C83DF3,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::Height, L"height", 0x05A5C519,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::CounterReset, L"counter-reset", 0x0894F9B0,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber | FDE_CSSVALUETYPE_MaybeString},
    {FDE_CSSProperty::Content, L"content", 0x097BE91B,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeURI | FDE_CSSVALUETYPE_MaybeString},
    {FDE_CSSProperty::RubyPosition, L"ruby-position", 0x09ACD024,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BackgroundColor, L"background-color", 0x09E8E8AC,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeColor},
    {FDE_CSSProperty::Width, L"width", 0x0A8A8F80,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Src, L"src", 0x0BD37048,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeURI},
    {FDE_CSSProperty::Top, L"top", 0x0BEDAF33,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Margin, L"margin", 0x0CB016BE,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderColor, L"border-color", 0x0CBB528A,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeColor},
    {FDE_CSSProperty::Widows, L"widows", 0x1026C59D,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderBottomColor, L"border-bottom-color", 0x121E22EC,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeColor},
    {FDE_CSSProperty::TextIndent, L"text-indent", 0x169ADB74,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Right, L"right", 0x193ADE3E,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::TextEmphasisStyle, L"text-emphasis-style", 0x20DBAF4A,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeString},
    {FDE_CSSProperty::PaddingLeft, L"padding-left", 0x228CF02F,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::ColumnWidth, L"column-width", 0x24C9AC9B,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::MarginLeft, L"margin-left", 0x297C5656,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber |
         FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::Border, L"border", 0x2A23349E,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::BorderTop, L"border-top", 0x2B866ADE,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::RubyOverhang, L"ruby-overhang", 0x2CCA0D89,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::PageBreakBefore, L"page-break-before", 0x3119B36F,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::MaxHeight, L"max-height", 0x343597EC,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::MinWidth, L"min-width", 0x35832871,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderLeftColor, L"border-left-color", 0x35C64022,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeColor},
    {FDE_CSSProperty::Bottom, L"bottom", 0x399F02B5,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Quotes, L"quotes", 0x3D8C6A01,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeString},
    {FDE_CSSProperty::MaxWidth, L"max-width", 0x3EA274F3,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::PaddingRight, L"padding-right", 0x3F616AC2,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::ListStyleImage, L"list-style-image", 0x42A8A86A,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeURI},
    {FDE_CSSProperty::WhiteSpace, L"white-space", 0x42F0429A,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderBottom, L"border-bottom", 0x452CE780,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::ListStyleType, L"list-style-type", 0x48094789,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::WordBreak, L"word-break", 0x4D74A3CE,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::OverflowX, L"overflow-x", 0x4ECEBF99,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::OverflowY, L"overflow-y", 0x4ECEBF9A,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderTopColor, L"border-top-color", 0x5109B8CA,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeColor},
    {FDE_CSSProperty::FontFamily, L"font-family", 0x574686E6,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeString},
    {FDE_CSSProperty::Cursor, L"cursor", 0x59DFCA5E,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeString},
    {FDE_CSSProperty::RubyAlign, L"ruby-align", 0x6077BDFA,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::ColumnRuleColor, L"column-rule-color", 0x65DDFD9F,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeColor},
    {FDE_CSSProperty::FontWeight, L"font-weight", 0x6692F60C,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderRightStyle, L"border-right-style", 0x6920DDA7,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::MinHeight, L"min-height", 0x6AAE312A,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Color, L"color", 0x6E67921F,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeColor},
    {FDE_CSSProperty::LetterSpacing, L"letter-spacing", 0x70536102,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::EmptyCells, L"empty-cells", 0x7531528F,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::TextAlign, L"text-align", 0x7553F1BD,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::RubySpan, L"ruby-span", 0x76FCFCE1,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeFunction},
    {FDE_CSSProperty::Position, L"position", 0x814F82B5,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderStyle, L"border-style", 0x82A4CD5C,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderBottomStyle, L"border-bottom-style", 0x88079DBE,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderCollapse, L"border-collapse", 0x8883C7FE,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::ColumnCount, L"column-count", 0x89936A64,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderRightWidth, L"border-right-width", 0x8F5A6036,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::UnicodeBidi, L"unicode-bidi", 0x91670F6C,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::VerticalAlign, L"vertical-align", 0x934A87D2,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::PaddingTop, L"padding-top", 0x959D22B7,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Columns, L"columns", 0x96FA5D81,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::Overflow, L"overflow", 0x97B76B54,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::TableLayout, L"table-layout", 0x9B1CB4B3,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::FontVariant, L"font-variant", 0x9C785779,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::ListStyle, L"list-style", 0x9E6C471A,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::BackgroundPosition, L"background-position", 0xA8846D22,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderWidth, L"border-width", 0xA8DE4FEB,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::TextEmphasisColor, L"text-emphasis-color", 0xAAF23478,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeColor},
    {FDE_CSSProperty::BorderLeftStyle, L"border-left-style", 0xABAFBAF4,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::PageBreakInside, L"page-break-inside", 0xACB695F8,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::TextEmphasis, L"text-emphasis", 0xAD0E580C,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::BorderBottomWidth, L"border-bottom-width", 0xAE41204D,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::ColumnGap, L"column-gap", 0xB5C1BA73,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Orphans, L"orphans", 0xB716467B,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderRight, L"border-right", 0xB78E9EA9,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::FontSize, L"font-size", 0xB93956DF,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::PageBreakAfter, L"page-break-after", 0xBC358AEE,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::CaptionSide, L"caption-side", 0xC03F3560,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BackgroundRepeat, L"background-repeat", 0xC2C2FDCE,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderTopStyle, L"border-top-style", 0xC6F3339C,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderSpacing, L"border-spacing", 0xC72030F0,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::TextTransform, L"text-transform", 0xC88EEA6E,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::FontStyle, L"font-style", 0xCB1950F5,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::Font, L"font", 0xCD308B77, FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::LineHeight, L"line-height", 0xCFCACE2E,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::MarginRight, L"margin-right", 0xD13C58C9,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber |
         FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::Float, L"float", 0xD1532876,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BorderLeftWidth, L"border-left-width", 0xD1E93D83,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::Display, L"display", 0xD4224C36,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::Clear, L"clear", 0xD8ED1467,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::ColumnRuleStyle, L"column-rule-style", 0xDBC77871,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::TextCombine, L"text-combine", 0xDC5207CF,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::ListStylePosition, L"list-style-position", 0xE1A1DE3C,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::Visibility, L"visibility", 0xE29F5168,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::PaddingBottom, L"padding-bottom", 0xE555B3B9,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BackgroundAttachment, L"background-attachment",
     0xE77981F6, FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::BackgroundImage, L"background-image", 0xE9AEB710,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeURI},
    {FDE_CSSProperty::LineBreak, L"line-break", 0xEA2D1D9A,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    {FDE_CSSProperty::Background, L"background", 0xEB49DD40,
     FDE_CSSVALUETYPE_Shorthand},
    {FDE_CSSProperty::BorderTopWidth, L"border-top-width", 0xED2CB62B,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::WordSpacing, L"word-spacing", 0xEDA63BAE,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber},
    {FDE_CSSProperty::BorderRightColor, L"border-right-color", 0xF33762D5,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeColor},
    {FDE_CSSProperty::CounterIncrement, L"counter-increment", 0xF4CFB1B2,
     FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum |
         FDE_CSSVALUETYPE_MaybeNumber | FDE_CSSVALUETYPE_MaybeString},
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
    {FDE_CSSProperty::Direction, L"direction", 0xFE746E61,
     FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
};
const int32_t g_iCSSPropertyCount =
    sizeof(g_FDE_CSSProperties) / sizeof(FDE_CSSPropertyTable);

static const FDE_CSSPropertyValueTable g_FDE_CSSPropertyValues[] = {
    {FDE_CSSPropertyValue::Bolder, L"bolder", 0x009F1058},
    {FDE_CSSPropertyValue::LowerLatin, L"lower-latin", 0x016014CE},
    {FDE_CSSPropertyValue::Lowercase, L"lowercase", 0x02ACB805},
    {FDE_CSSPropertyValue::LowerGreek, L"lower-greek", 0x03D81D64},
    {FDE_CSSPropertyValue::Sesame, L"sesame", 0x0432ECDE},
    {FDE_CSSPropertyValue::None, L"none", 0x048B6670},
    {FDE_CSSPropertyValue::NwResize, L"nw-resize", 0x054B4BE4},
    {FDE_CSSPropertyValue::WResize, L"w-resize", 0x0A2F8D76},
    {FDE_CSSPropertyValue::Dot, L"dot", 0x0A48CB27},
    {FDE_CSSPropertyValue::End, L"end", 0x0A631437},
    {FDE_CSSPropertyValue::Ltr, L"ltr", 0x0B1B56D2},
    {FDE_CSSPropertyValue::Pre, L"pre", 0x0B848587},
    {FDE_CSSPropertyValue::Rtl, L"rtl", 0x0BB92C52},
    {FDE_CSSPropertyValue::Sub, L"sub", 0x0BD37FAA},
    {FDE_CSSPropertyValue::Top, L"top", 0x0BEDAF33},
    {FDE_CSSPropertyValue::Visible, L"visible", 0x0F55D7EE},
    {FDE_CSSPropertyValue::Filled, L"filled", 0x10827DD0},
    {FDE_CSSPropertyValue::SwResize, L"sw-resize", 0x10B548E9},
    {FDE_CSSPropertyValue::NoRepeat, L"no-repeat", 0x1235C18B},
    {FDE_CSSPropertyValue::Default, L"default", 0x14DA2125},
    {FDE_CSSPropertyValue::Transparent, L"transparent", 0x17B64DB2},
    {FDE_CSSPropertyValue::Ridge, L"ridge", 0x18EBEE4B},
    {FDE_CSSPropertyValue::Right, L"right", 0x193ADE3E},
    {FDE_CSSPropertyValue::HorizontalTb, L"horizontal-tb", 0x1A66A86D},
    {FDE_CSSPropertyValue::DistributeLetter, L"distribute-letter", 0x1EDBD75C},
    {FDE_CSSPropertyValue::DoubleCircle, L"double-circle", 0x1FF082BA},
    {FDE_CSSPropertyValue::Ruby, L"ruby", 0x20D66C02},
    {FDE_CSSPropertyValue::Collapse, L"collapse", 0x2128D673},
    {FDE_CSSPropertyValue::Normal, L"normal", 0x247CF3E9},
    {FDE_CSSPropertyValue::Avoid, L"avoid", 0x24E684B3},
    {FDE_CSSPropertyValue::UpperRoman, L"upper-roman", 0x28BAC2B6},
    {FDE_CSSPropertyValue::Auto, L"auto", 0x2B35B6D9},
    {FDE_CSSPropertyValue::Text, L"text", 0x2D08AF85},
    {FDE_CSSPropertyValue::XSmall, L"x-small", 0x2D2FCAFE},
    {FDE_CSSPropertyValue::Thin, L"thin", 0x2D574D53},
    {FDE_CSSPropertyValue::Repeat, L"repeat", 0x306614A1},
    {FDE_CSSPropertyValue::Small, L"small", 0x316A3739},
    {FDE_CSSPropertyValue::NeResize, L"ne-resize", 0x31FD5E12},
    {FDE_CSSPropertyValue::NoContent, L"no-content", 0x33A1C545},
    {FDE_CSSPropertyValue::Outside, L"outside", 0x36DF693D},
    {FDE_CSSPropertyValue::EResize, L"e-resize", 0x36E19FA4},
    {FDE_CSSPropertyValue::TableRow, L"table-row", 0x3912A02D},
    {FDE_CSSPropertyValue::Bottom, L"bottom", 0x399F02B5},
    {FDE_CSSPropertyValue::Underline, L"underline", 0x3A0273A6},
    {FDE_CSSPropertyValue::CjkIdeographic, L"cjk-ideographic", 0x3A641CC4},
    {FDE_CSSPropertyValue::SeResize, L"se-resize", 0x3D675B17},
    {FDE_CSSPropertyValue::Fixed, L"fixed", 0x3D7DEB10},
    {FDE_CSSPropertyValue::Double, L"double", 0x3D98515B},
    {FDE_CSSPropertyValue::Solid, L"solid", 0x40623B5B},
    {FDE_CSSPropertyValue::RubyBaseGroup, L"ruby-base-group", 0x41014E84},
    {FDE_CSSPropertyValue::OpenQuote, L"open-quote", 0x44A41E8D},
    {FDE_CSSPropertyValue::Lighter, L"lighter", 0x45BEB7AF},
    {FDE_CSSPropertyValue::LowerRoman, L"lower-roman", 0x5044D253},
    {FDE_CSSPropertyValue::Strict, L"strict", 0x52F4EBD9},
    {FDE_CSSPropertyValue::TableCaption, L"table-caption", 0x5325CD63},
    {FDE_CSSPropertyValue::Oblique, L"oblique", 0x53EBDDB1},
    {FDE_CSSPropertyValue::Decimal, L"decimal", 0x54034C2F},
    {FDE_CSSPropertyValue::Loose, L"loose", 0x54D3A1E2},
    {FDE_CSSPropertyValue::Hebrew, L"hebrew", 0x565792DD},
    {FDE_CSSPropertyValue::Hidden, L"hidden", 0x573CB40C},
    {FDE_CSSPropertyValue::Dashed, L"dashed", 0x58A3DD29},
    {FDE_CSSPropertyValue::Embed, L"embed", 0x59C8F27D},
    {FDE_CSSPropertyValue::TableRowGroup, L"table-row-group", 0x5A43BD07},
    {FDE_CSSPropertyValue::TableColumn, L"table-column", 0x5E705DA3},
    {FDE_CSSPropertyValue::Static, L"static", 0x5E7555E8},
    {FDE_CSSPropertyValue::Outset, L"outset", 0x61236164},
    {FDE_CSSPropertyValue::DecimalLeadingZero, L"decimal-leading-zero",
     0x61DFC55D},
    {FDE_CSSPropertyValue::KeepWords, L"keep-words", 0x63964801},
    {FDE_CSSPropertyValue::KatakanaIroha, L"katakana-iroha", 0x65D7C91C},
    {FDE_CSSPropertyValue::Super, L"super", 0x6A4F842F},
    {FDE_CSSPropertyValue::Center, L"center", 0x6C51AFC1},
    {FDE_CSSPropertyValue::TableHeaderGroup, L"table-header-group", 0x706103D8},
    {FDE_CSSPropertyValue::Inside, L"inside", 0x709CB0FC},
    {FDE_CSSPropertyValue::XxLarge, L"xx-large", 0x70BB1508},
    {FDE_CSSPropertyValue::Triangle, L"triangle", 0x7524EDF6},
    {FDE_CSSPropertyValue::RubyTextGroup, L"ruby-text-group", 0x78C2B98E},
    {FDE_CSSPropertyValue::Circle, L"circle", 0x7ABEC0D2},
    {FDE_CSSPropertyValue::Hiragana, L"hiragana", 0x7BF5E25B},
    {FDE_CSSPropertyValue::RepeatX, L"repeat-x", 0x7C8F3226},
    {FDE_CSSPropertyValue::RepeatY, L"repeat-y", 0x7C8F3227},
    {FDE_CSSPropertyValue::Move, L"move", 0x7DA03417},
    {FDE_CSSPropertyValue::HiraganaIroha, L"hiragana-iroha", 0x7EE863FB},
    {FDE_CSSPropertyValue::RubyBase, L"ruby-base", 0x7FD1B1EA},
    {FDE_CSSPropertyValue::Scroll, L"scroll", 0x84787AEF},
    {FDE_CSSPropertyValue::Smaller, L"smaller", 0x849769F0},
    {FDE_CSSPropertyValue::TableFooterGroup, L"table-footer-group", 0x85BDD97E},
    {FDE_CSSPropertyValue::Baseline, L"baseline", 0x87436BA3},
    {FDE_CSSPropertyValue::Separate, L"separate", 0x877C66B5},
    {FDE_CSSPropertyValue::Armenian, L"armenian", 0x889BE4EB},
    {FDE_CSSPropertyValue::Open, L"open", 0x8B90E1F2},
    {FDE_CSSPropertyValue::Relative, L"relative", 0x8C995B5C},
    {FDE_CSSPropertyValue::Thick, L"thick", 0x8CC35EB3},
    {FDE_CSSPropertyValue::Justify, L"justify", 0x8D269CAE},
    {FDE_CSSPropertyValue::Middle, L"middle", 0x947FA00F},
    {FDE_CSSPropertyValue::Always, L"always", 0x959AB231},
    {FDE_CSSPropertyValue::DistributeSpace, L"distribute-space", 0x97A20E58},
    {FDE_CSSPropertyValue::LineEdge, L"line-edge", 0x9A845D2A},
    {FDE_CSSPropertyValue::PreWrap, L"pre-wrap", 0x9D59588E},
    {FDE_CSSPropertyValue::Medium, L"medium", 0xA084A381},
    {FDE_CSSPropertyValue::NResize, L"n-resize", 0xA088968D},
    {FDE_CSSPropertyValue::ListItem, L"list-item", 0xA32382B8},
    {FDE_CSSPropertyValue::Show, L"show", 0xA66C10C1},
    {FDE_CSSPropertyValue::Currentcolor, L"currentColor", 0xA7883922},
    {FDE_CSSPropertyValue::NoCloseQuote, L"no-close-quote", 0xA79CBFFB},
    {FDE_CSSPropertyValue::VerticalLr, L"vertical-lr", 0xA8673F65},
    {FDE_CSSPropertyValue::VerticalRl, L"vertical-rl", 0xA8675E25},
    {FDE_CSSPropertyValue::Pointer, L"pointer", 0xA90929C1},
    {FDE_CSSPropertyValue::XxSmall, L"xx-small", 0xADE1FC76},
    {FDE_CSSPropertyValue::Bold, L"bold", 0xB18313A1},
    {FDE_CSSPropertyValue::Both, L"both", 0xB1833CAD},
    {FDE_CSSPropertyValue::SmallCaps, L"small-caps", 0xB299428D},
    {FDE_CSSPropertyValue::Katakana, L"katakana", 0xB421A4BC},
    {FDE_CSSPropertyValue::After, L"after", 0xB6B44172},
    {FDE_CSSPropertyValue::Horizontal, L"horizontal", 0xB7732DEA},
    {FDE_CSSPropertyValue::Dotted, L"dotted", 0xB88652A4},
    {FDE_CSSPropertyValue::Disc, L"disc", 0xBEBC18C3},
    {FDE_CSSPropertyValue::Georgian, L"georgian", 0xBEF99E8C},
    {FDE_CSSPropertyValue::Inline, L"inline", 0xC02D649F},
    {FDE_CSSPropertyValue::Overline, L"overline", 0xC0EC9FA4},
    {FDE_CSSPropertyValue::Wait, L"wait", 0xC1613BB5},
    {FDE_CSSPropertyValue::BreakAll, L"break-all", 0xC3145BAB},
    {FDE_CSSPropertyValue::UpperAlpha, L"upper-alpha", 0xC52D4A9F},
    {FDE_CSSPropertyValue::Capitalize, L"capitalize", 0xC5321D46},
    {FDE_CSSPropertyValue::Nowrap, L"nowrap", 0xC7994417},
    {FDE_CSSPropertyValue::TextBottom, L"text-bottom", 0xC7D08D87},
    {FDE_CSSPropertyValue::NoOpenQuote, L"no-open-quote", 0xC8CD7877},
    {FDE_CSSPropertyValue::Groove, L"groove", 0xCB24A412},
    {FDE_CSSPropertyValue::Progress, L"progress", 0xCD1D9835},
    {FDE_CSSPropertyValue::Larger, L"larger", 0xCD3C409D},
    {FDE_CSSPropertyValue::CloseQuote, L"close-quote", 0xCF8696D1},
    {FDE_CSSPropertyValue::TableCell, L"table-cell", 0xCFB5E595},
    {FDE_CSSPropertyValue::PreLine, L"pre-line", 0xD04FEDBC},
    {FDE_CSSPropertyValue::Absolute, L"absolute", 0xD0B2D55F},
    {FDE_CSSPropertyValue::InlineTable, L"inline-table", 0xD131F494},
    {FDE_CSSPropertyValue::BidiOverride, L"bidi-override", 0xD161FDE5},
    {FDE_CSSPropertyValue::InlineBlock, L"inline-block", 0xD26A8BD7},
    {FDE_CSSPropertyValue::Inset, L"inset", 0xD6F23243},
    {FDE_CSSPropertyValue::Crosshair, L"crosshair", 0xD6F8018E},
    {FDE_CSSPropertyValue::UpperLatin, L"upper-latin", 0xD9D60531},
    {FDE_CSSPropertyValue::Help, L"help", 0xDA002969},
    {FDE_CSSPropertyValue::Hide, L"hide", 0xDA69395A},
    {FDE_CSSPropertyValue::Uppercase, L"uppercase", 0xDAD595A8},
    {FDE_CSSPropertyValue::SResize, L"s-resize", 0xDB3AADF2},
    {FDE_CSSPropertyValue::Table, L"table", 0xDB9BE968},
    {FDE_CSSPropertyValue::Blink, L"blink", 0xDC36E390},
    {FDE_CSSPropertyValue::Block, L"block", 0xDCD480AB},
    {FDE_CSSPropertyValue::Start, L"start", 0xE1D9D5AE},
    {FDE_CSSPropertyValue::TableColumnGroup, L"table-column-group", 0xE2258EFD},
    {FDE_CSSPropertyValue::Italic, L"italic", 0xE31D5396},
    {FDE_CSSPropertyValue::LineThrough, L"line-through", 0xE4C5A276},
    {FDE_CSSPropertyValue::KeepAll, L"keep-all", 0xE704A72B},
    {FDE_CSSPropertyValue::LowerAlpha, L"lower-alpha", 0xECB75A3C},
    {FDE_CSSPropertyValue::RunIn, L"run-in", 0xEEC930B9},
    {FDE_CSSPropertyValue::Square, L"square", 0xEF85D351},
    {FDE_CSSPropertyValue::XLarge, L"x-large", 0xF008E390},
    {FDE_CSSPropertyValue::Large, L"large", 0xF4434FCB},
    {FDE_CSSPropertyValue::Before, L"before", 0xF4FFCE73},
    {FDE_CSSPropertyValue::Left, L"left", 0xF5AD782B},
    {FDE_CSSPropertyValue::TextTop, L"text-top", 0xFCB58D45},
    {FDE_CSSPropertyValue::RubyText, L"ruby-text", 0xFCC77174},
    {FDE_CSSPropertyValue::NoDisplay, L"no-display", 0xFE482860},
};
const int32_t g_iCSSPropertyValueCount =
    sizeof(g_FDE_CSSPropertyValues) / sizeof(FDE_CSSPropertyValueTable);

static const FDE_CSSMEDIATYPETABLE g_FDE_CSSMediaTypes[] = {
    {0xF09, 0x02},  {0x4880, 0x20}, {0x536A, 0x80},
    {0x741D, 0x10}, {0x76ED, 0x08}, {0x7CFB, 0x01},
    {0x9578, 0x04}, {0xC8E1, 0x40}, {0xD0F9, 0xFF},
};
static const FDE_CSSLengthUnitTable g_FDE_CSSLengthUnits[] = {
    {0x0672, FDE_CSSPrimitiveType::EMS},
    {0x067D, FDE_CSSPrimitiveType::EXS},
    {0x1AF7, FDE_CSSPrimitiveType::Inches},
    {0x2F7A, FDE_CSSPrimitiveType::MilliMeters},
    {0x3ED3, FDE_CSSPrimitiveType::Picas},
    {0x3EE4, FDE_CSSPrimitiveType::Points},
    {0x3EE8, FDE_CSSPrimitiveType::Pixels},
    {0xFC30, FDE_CSSPrimitiveType::CentiMeters},
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

static const FDE_CSSPseudoTable g_FDE_CSSPseudoType[] = {
    {FDE_CSSPseudo::After, L":after", 0x16EE1FEC},
    {FDE_CSSPseudo::Before, L":before", 0x7DCDDE2D},
};

const FDE_CSSPseudoTable* FDE_GetCSSPseudoByEnum(FDE_CSSPseudo ePseudo) {
  return g_FDE_CSSPseudoType + static_cast<int>(ePseudo);
}

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

const FDE_CSSPropertyValueTable* FDE_GetCSSPropertyValueByEnum(
    FDE_CSSPropertyValue eName) {
  return g_FDE_CSSPropertyValues + static_cast<int>(eName);
}

const FDE_CSSMEDIATYPETABLE* FDE_GetCSSMediaTypeByName(
    const CFX_WideStringC& wsName) {
  ASSERT(!wsName.IsEmpty());
  uint16_t wHash = FX_HashCode_GetW(wsName, true);
  int32_t iEnd =
      sizeof(g_FDE_CSSMediaTypes) / sizeof(FDE_CSSMEDIATYPETABLE) - 1;
  int32_t iMid, iStart = 0;
  uint16_t uMid;
  do {
    iMid = (iStart + iEnd) / 2;
    uMid = g_FDE_CSSMediaTypes[iMid].wHash;
    if (wHash == uMid) {
      return g_FDE_CSSMediaTypes + iMid;
    } else if (wHash > uMid) {
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

bool FDE_ParseCSSNumber(const FX_WCHAR* pszValue,
                        int32_t iValueLen,
                        FX_FLOAT& fValue,
                        FDE_CSSPrimitiveType& eUnit) {
  ASSERT(pszValue && iValueLen > 0);
  int32_t iUsedLen = 0;
  fValue = FXSYS_wcstof(pszValue, iValueLen, &iUsedLen);
  if (iUsedLen <= 0)
    return false;

  iValueLen -= iUsedLen;
  pszValue += iUsedLen;
  eUnit = FDE_CSSPrimitiveType::Number;
  if (iValueLen >= 1 && *pszValue == '%') {
    eUnit = FDE_CSSPrimitiveType::Percent;
  } else if (iValueLen == 2) {
    const FDE_CSSLengthUnitTable* pUnit =
        FDE_GetCSSLengthUnitByName(CFX_WideStringC(pszValue, 2));
    if (pUnit)
      eUnit = pUnit->wValue;
  }
  return true;
}

bool FDE_ParseCSSString(const FX_WCHAR* pszValue,
                        int32_t iValueLen,
                        int32_t* iOffset,
                        int32_t* iLength) {
  ASSERT(pszValue && iValueLen > 0);
  *iOffset = 0;
  *iLength = iValueLen;
  if (iValueLen >= 2) {
    FX_WCHAR first = pszValue[0], last = pszValue[iValueLen - 1];
    if ((first == '\"' && last == '\"') || (first == '\'' && last == '\'')) {
      *iOffset = 1;
      *iLength -= 2;
    }
  }
  return iValueLen > 0;
}

bool FDE_ParseCSSURI(const FX_WCHAR* pszValue,
                     int32_t* iOffset,
                     int32_t* iLength) {
  ASSERT(pszValue && *iLength > 0);
  if (*iLength < 6 || pszValue[*iLength - 1] != ')' ||
      FXSYS_wcsnicmp(L"url(", pszValue, 4)) {
    return false;
  }
  if (FDE_ParseCSSString(pszValue + 4, *iLength - 5, iOffset, iLength)) {
    *iOffset += 4;
    return true;
  }
  return false;
}

bool FDE_ParseCSSColor(const FX_WCHAR* pszValue,
                       int32_t iValueLen,
                       FX_ARGB& dwColor) {
  ASSERT(pszValue && iValueLen > 0);

  if (*pszValue == '#') {
    switch (iValueLen) {
      case 4: {
        uint8_t red = Hex2Dec((uint8_t)pszValue[1], (uint8_t)pszValue[1]);
        uint8_t green = Hex2Dec((uint8_t)pszValue[2], (uint8_t)pszValue[2]);
        uint8_t blue = Hex2Dec((uint8_t)pszValue[3], (uint8_t)pszValue[3]);
        dwColor = ArgbEncode(255, red, green, blue);
        return true;
      }
      case 7: {
        uint8_t red = Hex2Dec((uint8_t)pszValue[1], (uint8_t)pszValue[2]);
        uint8_t green = Hex2Dec((uint8_t)pszValue[3], (uint8_t)pszValue[4]);
        uint8_t blue = Hex2Dec((uint8_t)pszValue[5], (uint8_t)pszValue[6]);
        dwColor = ArgbEncode(255, red, green, blue);
        return true;
      }
      default:
        return false;
    }
  }

  if (iValueLen >= 10) {
    if (pszValue[iValueLen - 1] != ')' || FXSYS_wcsnicmp(L"rgb(", pszValue, 4))
      return false;

    uint8_t rgb[3] = {0};
    FX_FLOAT fValue;
    FDE_CSSPrimitiveType eType;
    CFDE_CSSValueListParser list(pszValue + 4, iValueLen - 5, ',');
    for (int32_t i = 0; i < 3; ++i) {
      if (!list.NextValue(eType, pszValue, iValueLen))
        return false;
      if (eType != FDE_CSSPrimitiveType::Number)
        return false;
      if (!FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eType))
        return false;

      rgb[i] = eType == FDE_CSSPrimitiveType::Percent
                   ? FXSYS_round(fValue * 2.55f)
                   : FXSYS_round(fValue);
    }
    dwColor = ArgbEncode(255, rgb[0], rgb[1], rgb[2]);
    return true;
  }

  const FDE_CSSCOLORTABLE* pColor =
      FDE_GetCSSColorByName(CFX_WideStringC(pszValue, iValueLen));
  if (!pColor)
    return false;

  dwColor = pColor->dwValue;
  return true;
}

CFDE_CSSValueList::CFDE_CSSValueList(const CFDE_CSSValueArray& list) {
  m_iCount = list.GetSize();
  int32_t iByteCount = m_iCount * sizeof(IFDE_CSSValue*);
  m_ppList = (IFDE_CSSValue**)FX_Alloc(uint8_t, iByteCount);
  FXSYS_memcpy(m_ppList, list.GetData(), iByteCount);
}

int32_t CFDE_CSSValueList::CountValues() const {
  return m_iCount;
}

IFDE_CSSValue* CFDE_CSSValueList::GetValue(int32_t index) const {
  return m_ppList[index];
}
bool CFDE_CSSValueListParser::NextValue(FDE_CSSPrimitiveType& eType,
                                        const FX_WCHAR*& pStart,
                                        int32_t& iLength) {
  while (m_pCur < m_pEnd && (*m_pCur <= ' ' || *m_pCur == m_Separator)) {
    ++m_pCur;
  }
  if (m_pCur >= m_pEnd) {
    return false;
  }
  eType = FDE_CSSPrimitiveType::Unknown;
  pStart = m_pCur;
  iLength = 0;
  FX_WCHAR wch = *m_pCur;
  if (wch == '#') {
    iLength = SkipTo(' ');
    if (iLength == 4 || iLength == 7) {
      eType = FDE_CSSPrimitiveType::RGB;
    }
  } else if ((wch >= '0' && wch <= '9') || wch == '.' || wch == '-' ||
             wch == '+') {
    while (m_pCur < m_pEnd && (*m_pCur > ' ' && *m_pCur != m_Separator)) {
      ++m_pCur;
    }
    iLength = m_pCur - pStart;
    if (iLength > 0) {
      eType = FDE_CSSPrimitiveType::Number;
    }
  } else if (wch == '\"' || wch == '\'') {
    pStart++;
    iLength = SkipTo(wch) - 1;
    m_pCur++;
    eType = FDE_CSSPrimitiveType::String;
  } else if (m_pEnd - m_pCur > 5 && m_pCur[3] == '(') {
    if (FXSYS_wcsnicmp(L"url", m_pCur, 3) == 0) {
      wch = m_pCur[4];
      if (wch == '\"' || wch == '\'') {
        pStart += 5;
        iLength = SkipTo(wch) - 6;
        m_pCur += 2;
      } else {
        pStart += 4;
        iLength = SkipTo(')') - 4;
        m_pCur++;
      }
      eType = FDE_CSSPrimitiveType::URI;
    } else if (FXSYS_wcsnicmp(L"rgb", m_pCur, 3) == 0) {
      iLength = SkipTo(')') + 1;
      m_pCur++;
      eType = FDE_CSSPrimitiveType::RGB;
    }
  } else {
    iLength = SkipTo(m_Separator, true, true);
    eType = FDE_CSSPrimitiveType::String;
  }
  return m_pCur <= m_pEnd && iLength > 0;
}
int32_t CFDE_CSSValueListParser::SkipTo(FX_WCHAR wch,
                                        bool bWSSeparator,
                                        bool bBrContinue) {
  const FX_WCHAR* pStart = m_pCur;
  if (!bBrContinue) {
    if (bWSSeparator) {
      while ((++m_pCur < m_pEnd) && (*m_pCur != wch) && (*m_pCur > ' ')) {
        continue;
      }
    } else {
      while (++m_pCur < m_pEnd && *m_pCur != wch) {
        continue;
      }
    }

  } else {
    int32_t iBracketCount = 0;
    if (bWSSeparator) {
      while ((m_pCur < m_pEnd) && (*m_pCur != wch) && (*m_pCur > ' ')) {
        if (*m_pCur == '(') {
          iBracketCount++;
        } else if (*m_pCur == ')') {
          iBracketCount--;
        }
        m_pCur++;
      }
    } else {
      while (m_pCur < m_pEnd && *m_pCur != wch) {
        if (*m_pCur == '(') {
          iBracketCount++;
        } else if (*m_pCur == ')') {
          iBracketCount--;
        }
        m_pCur++;
      }
    }
    while (iBracketCount > 0 && m_pCur < m_pEnd) {
      if (*m_pCur == ')') {
        iBracketCount--;
      }
      m_pCur++;
    }
  }
  return m_pCur - pStart;
}

CFDE_CSSPrimitiveValue::CFDE_CSSPrimitiveValue(
    const CFDE_CSSPrimitiveValue& src) = default;

CFDE_CSSPrimitiveValue::CFDE_CSSPrimitiveValue(FX_ARGB color)
    : m_eType(FDE_CSSPrimitiveType::RGB), m_dwColor(color) {}

CFDE_CSSPrimitiveValue::CFDE_CSSPrimitiveValue(FDE_CSSPropertyValue eValue)
    : m_eType(FDE_CSSPrimitiveType::Enum), m_eEnum(eValue) {}

CFDE_CSSPrimitiveValue::CFDE_CSSPrimitiveValue(FDE_CSSPrimitiveType eType,
                                               FX_FLOAT fValue)
    : m_eType(eType), m_fNumber(fValue) {}

CFDE_CSSPrimitiveValue::CFDE_CSSPrimitiveValue(FDE_CSSPrimitiveType eType,
                                               const FX_WCHAR* pValue)
    : m_eType(eType), m_pString(pValue) {
  ASSERT(m_pString);
}

CFDE_CSSPrimitiveValue::CFDE_CSSPrimitiveValue(CFDE_CSSFunction* pFunction)
    : m_eType(FDE_CSSPrimitiveType::Function), m_pFunction(pFunction) {}

FDE_CSSPrimitiveType CFDE_CSSPrimitiveValue::GetPrimitiveType() const {
  return m_eType;
}

FX_ARGB CFDE_CSSPrimitiveValue::GetRGBColor() const {
  ASSERT(m_eType == FDE_CSSPrimitiveType::RGB);
  return m_dwColor;
}

FX_FLOAT CFDE_CSSPrimitiveValue::GetFloat() const {
  ASSERT(m_eType >= FDE_CSSPrimitiveType::Number &&
         m_eType <= FDE_CSSPrimitiveType::Picas);
  return m_fNumber;
}

const FX_WCHAR* CFDE_CSSPrimitiveValue::GetString(int32_t& iLength) const {
  ASSERT(m_eType >= FDE_CSSPrimitiveType::String &&
         m_eType <= FDE_CSSPrimitiveType::URI);
  iLength = FXSYS_wcslen(m_pString);
  return m_pString;
}

FDE_CSSPropertyValue CFDE_CSSPrimitiveValue::GetEnum() const {
  ASSERT(m_eType == FDE_CSSPrimitiveType::Enum);
  return m_eEnum;
}

const FX_WCHAR* CFDE_CSSPrimitiveValue::GetFuncName() const {
  ASSERT(m_eType == FDE_CSSPrimitiveType::Function);
  return m_pFunction->GetFuncName();
}

int32_t CFDE_CSSPrimitiveValue::CountArgs() const {
  ASSERT(m_eType == FDE_CSSPrimitiveType::Function);
  return m_pFunction->CountArgs();
}

IFDE_CSSValue* CFDE_CSSPrimitiveValue::GetArgs(int32_t index) const {
  ASSERT(m_eType == FDE_CSSPrimitiveType::Function);
  return m_pFunction->GetArgs(index);
}
