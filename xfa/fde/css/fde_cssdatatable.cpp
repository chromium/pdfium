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

static const FDE_CSSMEDIATYPETABLE g_FDE_CSSMediaTypes[] = {
    {0xF09, FDE_CSSMEDIATYPE_Emboss},    {FDE_CSSMEDIATYPE_Screen},
    {0x536A, FDE_CSSMEDIATYPE_TV},       {0x741D, FDE_CSSMEDIATYPE_Projection},
    {0x76ED, FDE_CSSMEDIATYPE_Print},    {0x7CFB, FDE_CSSMEDIATYPE_Braille},
    {0x9578, FDE_CSSMEDIATYPE_Handheld}, {0xC8E1, FDE_CSSMEDIATYPE_TTY},
    {0xD0F9, FDE_CSSMEDIATYPE_ALL},
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
