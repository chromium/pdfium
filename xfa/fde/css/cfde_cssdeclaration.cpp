// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssdeclaration.h"

#include "core/fxcrt/fx_ext.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fde/css/cfde_csscolorvalue.h"
#include "xfa/fde/css/cfde_csscustomproperty.h"
#include "xfa/fde/css/cfde_cssenumvalue.h"
#include "xfa/fde/css/cfde_cssnumbervalue.h"
#include "xfa/fde/css/cfde_csspropertyholder.h"
#include "xfa/fde/css/cfde_cssstringvalue.h"
#include "xfa/fde/css/cfde_cssvaluelist.h"
#include "xfa/fde/css/cfde_cssvaluelistparser.h"

namespace {

uint8_t Hex2Dec(uint8_t hexHigh, uint8_t hexLow) {
  return (FXSYS_toHexDigit(hexHigh) << 4) + FXSYS_toHexDigit(hexLow);
}

bool ParseCSSNumber(const FX_WCHAR* pszValue,
                    int32_t iValueLen,
                    FX_FLOAT& fValue,
                    FDE_CSSNumberType& eUnit) {
  ASSERT(pszValue && iValueLen > 0);
  int32_t iUsedLen = 0;
  fValue = FXSYS_wcstof(pszValue, iValueLen, &iUsedLen);
  if (iUsedLen <= 0)
    return false;

  iValueLen -= iUsedLen;
  pszValue += iUsedLen;
  eUnit = FDE_CSSNumberType::Number;
  if (iValueLen >= 1 && *pszValue == '%') {
    eUnit = FDE_CSSNumberType::Percent;
  } else if (iValueLen == 2) {
    const FDE_CSSLengthUnitTable* pUnit =
        FDE_GetCSSLengthUnitByName(CFX_WideStringC(pszValue, 2));
    if (pUnit)
      eUnit = pUnit->wValue;
  }
  return true;
}

}  // namespace

// static
bool CFDE_CSSDeclaration::ParseCSSString(const FX_WCHAR* pszValue,
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

// static.
bool CFDE_CSSDeclaration::ParseCSSColor(const FX_WCHAR* pszValue,
                                        int32_t iValueLen,
                                        FX_ARGB* dwColor) {
  ASSERT(pszValue && iValueLen > 0);
  ASSERT(dwColor);

  if (*pszValue == '#') {
    switch (iValueLen) {
      case 4: {
        uint8_t red = Hex2Dec((uint8_t)pszValue[1], (uint8_t)pszValue[1]);
        uint8_t green = Hex2Dec((uint8_t)pszValue[2], (uint8_t)pszValue[2]);
        uint8_t blue = Hex2Dec((uint8_t)pszValue[3], (uint8_t)pszValue[3]);
        *dwColor = ArgbEncode(255, red, green, blue);
        return true;
      }
      case 7: {
        uint8_t red = Hex2Dec((uint8_t)pszValue[1], (uint8_t)pszValue[2]);
        uint8_t green = Hex2Dec((uint8_t)pszValue[3], (uint8_t)pszValue[4]);
        uint8_t blue = Hex2Dec((uint8_t)pszValue[5], (uint8_t)pszValue[6]);
        *dwColor = ArgbEncode(255, red, green, blue);
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
      FDE_CSSNumberType eNumType;
      if (!ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
        return false;

      rgb[i] = eNumType == FDE_CSSNumberType::Percent
                   ? FXSYS_round(fValue * 2.55f)
                   : FXSYS_round(fValue);
    }
    *dwColor = ArgbEncode(255, rgb[0], rgb[1], rgb[2]);
    return true;
  }

  const FDE_CSSCOLORTABLE* pColor =
      FDE_GetCSSColorByName(CFX_WideStringC(pszValue, iValueLen));
  if (!pColor)
    return false;

  *dwColor = pColor->dwValue;
  return true;
}

CFDE_CSSDeclaration::CFDE_CSSDeclaration() {}

CFDE_CSSDeclaration::~CFDE_CSSDeclaration() {}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSDeclaration::GetProperty(
    FDE_CSSProperty eProperty,
    bool* bImportant) const {
  for (const auto& p : properties_) {
    if (p->eProperty == eProperty) {
      *bImportant = p->bImportant;
      return p->pValue;
    }
  }
  return nullptr;
}

void CFDE_CSSDeclaration::AddPropertyHolder(FDE_CSSProperty eProperty,
                                            CFX_RetainPtr<CFDE_CSSValue> pValue,
                                            bool bImportant) {
  auto pHolder = pdfium::MakeUnique<CFDE_CSSPropertyHolder>();
  pHolder->bImportant = bImportant;
  pHolder->eProperty = eProperty;
  pHolder->pValue = pValue;
  properties_.push_back(std::move(pHolder));
}

void CFDE_CSSDeclaration::AddProperty(const FDE_CSSPropertyTable* pTable,
                                      const CFX_WideStringC& value) {
  ASSERT(!value.IsEmpty());

  const FX_WCHAR* pszValue = value.c_str();
  int32_t iValueLen = value.GetLength();

  bool bImportant = false;
  if (iValueLen >= 10 && pszValue[iValueLen - 10] == '!' &&
      FXSYS_wcsnicmp(L"important", pszValue + iValueLen - 9, 9) == 0) {
    if ((iValueLen -= 10) == 0)
      return;

    bImportant = true;
  }
  const uint32_t dwType = pTable->dwType;
  switch (dwType & 0x0F) {
    case FDE_CSSVALUETYPE_Primitive: {
      static const uint32_t g_ValueGuessOrder[] = {
          FDE_CSSVALUETYPE_MaybeNumber, FDE_CSSVALUETYPE_MaybeEnum,
          FDE_CSSVALUETYPE_MaybeColor, FDE_CSSVALUETYPE_MaybeString,
      };
      static const int32_t g_ValueGuessCount =
          sizeof(g_ValueGuessOrder) / sizeof(uint32_t);
      for (int32_t i = 0; i < g_ValueGuessCount; ++i) {
        const uint32_t dwMatch = dwType & g_ValueGuessOrder[i];
        if (dwMatch == 0) {
          continue;
        }
        CFX_RetainPtr<CFDE_CSSValue> pCSSValue;
        switch (dwMatch) {
          case FDE_CSSVALUETYPE_MaybeNumber:
            pCSSValue = ParseNumber(pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeEnum:
            pCSSValue = ParseEnum(pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeColor:
            pCSSValue = ParseColor(pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeString:
            pCSSValue = ParseString(pszValue, iValueLen);
            break;
          default:
            break;
        }
        if (pCSSValue) {
          AddPropertyHolder(pTable->eName, pCSSValue, bImportant);
          return;
        }
        if (FDE_IsOnlyValue(dwType, g_ValueGuessOrder[i]))
          return;
      }
      break;
    }
    case FDE_CSSVALUETYPE_Shorthand: {
      CFX_RetainPtr<CFDE_CSSValue> pWidth;
      switch (pTable->eName) {
        case FDE_CSSProperty::Font:
          ParseFontProperty(pszValue, iValueLen, bImportant);
          return;
        case FDE_CSSProperty::Border:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(FDE_CSSProperty::BorderLeftWidth, pWidth,
                              bImportant);
            AddPropertyHolder(FDE_CSSProperty::BorderTopWidth, pWidth,
                              bImportant);
            AddPropertyHolder(FDE_CSSProperty::BorderRightWidth, pWidth,
                              bImportant);
            AddPropertyHolder(FDE_CSSProperty::BorderBottomWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case FDE_CSSProperty::BorderLeft:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(FDE_CSSProperty::BorderLeftWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case FDE_CSSProperty::BorderTop:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(FDE_CSSProperty::BorderTopWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case FDE_CSSProperty::BorderRight:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(FDE_CSSProperty::BorderRightWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case FDE_CSSProperty::BorderBottom:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(FDE_CSSProperty::BorderBottomWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        default:
          break;
      }
    } break;
    case FDE_CSSVALUETYPE_List:
      ParseValueListProperty(pTable, pszValue, iValueLen, bImportant);
      return;
    default:
      ASSERT(false);
      break;
  }
}

void CFDE_CSSDeclaration::AddProperty(const CFX_WideString& prop,
                                      const CFX_WideString& value) {
  custom_properties_.push_back(
      pdfium::MakeUnique<CFDE_CSSCustomProperty>(prop, value));
}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSDeclaration::ParseNumber(
    const FX_WCHAR* pszValue,
    int32_t iValueLen) {
  FX_FLOAT fValue;
  FDE_CSSNumberType eUnit;
  if (!ParseCSSNumber(pszValue, iValueLen, fValue, eUnit))
    return nullptr;
  return pdfium::MakeRetain<CFDE_CSSNumberValue>(eUnit, fValue);
}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSDeclaration::ParseEnum(
    const FX_WCHAR* pszValue,
    int32_t iValueLen) {
  const FDE_CSSPropertyValueTable* pValue =
      FDE_GetCSSPropertyValueByName(CFX_WideStringC(pszValue, iValueLen));
  return pValue ? pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName)
                : nullptr;
}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSDeclaration::ParseColor(
    const FX_WCHAR* pszValue,
    int32_t iValueLen) {
  FX_ARGB dwColor;
  if (!ParseCSSColor(pszValue, iValueLen, &dwColor))
    return nullptr;
  return pdfium::MakeRetain<CFDE_CSSColorValue>(dwColor);
}

CFX_RetainPtr<CFDE_CSSValue> CFDE_CSSDeclaration::ParseString(
    const FX_WCHAR* pszValue,
    int32_t iValueLen) {
  int32_t iOffset;
  if (!ParseCSSString(pszValue, iValueLen, &iOffset, &iValueLen))
    return nullptr;

  if (iValueLen <= 0)
    return nullptr;

  return pdfium::MakeRetain<CFDE_CSSStringValue>(
      CFX_WideString(pszValue + iOffset, iValueLen));
}

void CFDE_CSSDeclaration::ParseValueListProperty(
    const FDE_CSSPropertyTable* pTable,
    const FX_WCHAR* pszValue,
    int32_t iValueLen,
    bool bImportant) {
  FX_WCHAR separator =
      (pTable->eName == FDE_CSSProperty::FontFamily) ? ',' : ' ';
  CFDE_CSSValueListParser parser(pszValue, iValueLen, separator);

  const uint32_t dwType = pTable->dwType;
  FDE_CSSPrimitiveType eType;
  std::vector<CFX_RetainPtr<CFDE_CSSValue>> list;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPrimitiveType::Number:
        if (dwType & FDE_CSSVALUETYPE_MaybeNumber) {
          FX_FLOAT fValue;
          FDE_CSSNumberType eNumType;
          if (ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
            list.push_back(
                pdfium::MakeRetain<CFDE_CSSNumberValue>(eNumType, fValue));
        }
        break;
      case FDE_CSSPrimitiveType::String:
        if (dwType & FDE_CSSVALUETYPE_MaybeColor) {
          FX_ARGB dwColor;
          if (ParseCSSColor(pszValue, iValueLen, &dwColor)) {
            list.push_back(pdfium::MakeRetain<CFDE_CSSColorValue>(dwColor));
            continue;
          }
        }
        if (dwType & FDE_CSSVALUETYPE_MaybeEnum) {
          const FDE_CSSPropertyValueTable* pValue =
              FDE_GetCSSPropertyValueByName(
                  CFX_WideStringC(pszValue, iValueLen));
          if (pValue) {
            list.push_back(
                pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName));
            continue;
          }
        }
        if (dwType & FDE_CSSVALUETYPE_MaybeString) {
          list.push_back(pdfium::MakeRetain<CFDE_CSSStringValue>(
              CFX_WideString(pszValue, iValueLen)));
        }
        break;
      case FDE_CSSPrimitiveType::RGB:
        if (dwType & FDE_CSSVALUETYPE_MaybeColor) {
          FX_ARGB dwColor;
          if (ParseCSSColor(pszValue, iValueLen, &dwColor)) {
            list.push_back(pdfium::MakeRetain<CFDE_CSSColorValue>(dwColor));
          }
        }
        break;
      default:
        break;
    }
  }
  if (list.empty())
    return;

  switch (pTable->eName) {
    case FDE_CSSProperty::BorderWidth:
      Add4ValuesProperty(list, bImportant, FDE_CSSProperty::BorderLeftWidth,
                         FDE_CSSProperty::BorderTopWidth,
                         FDE_CSSProperty::BorderRightWidth,
                         FDE_CSSProperty::BorderBottomWidth);
      return;
    case FDE_CSSProperty::Margin:
      Add4ValuesProperty(list, bImportant, FDE_CSSProperty::MarginLeft,
                         FDE_CSSProperty::MarginTop,
                         FDE_CSSProperty::MarginRight,
                         FDE_CSSProperty::MarginBottom);
      return;
    case FDE_CSSProperty::Padding:
      Add4ValuesProperty(list, bImportant, FDE_CSSProperty::PaddingLeft,
                         FDE_CSSProperty::PaddingTop,
                         FDE_CSSProperty::PaddingRight,
                         FDE_CSSProperty::PaddingBottom);
      return;
    default: {
      auto pList = pdfium::MakeRetain<CFDE_CSSValueList>(list);
      AddPropertyHolder(pTable->eName, pList, bImportant);
      return;
    }
  }
}

void CFDE_CSSDeclaration::Add4ValuesProperty(
    const std::vector<CFX_RetainPtr<CFDE_CSSValue>>& list,
    bool bImportant,
    FDE_CSSProperty eLeft,
    FDE_CSSProperty eTop,
    FDE_CSSProperty eRight,
    FDE_CSSProperty eBottom) {
  switch (list.size()) {
    case 1:
      AddPropertyHolder(eLeft, list[0], bImportant);
      AddPropertyHolder(eTop, list[0], bImportant);
      AddPropertyHolder(eRight, list[0], bImportant);
      AddPropertyHolder(eBottom, list[0], bImportant);
      return;
    case 2:
      AddPropertyHolder(eLeft, list[1], bImportant);
      AddPropertyHolder(eTop, list[0], bImportant);
      AddPropertyHolder(eRight, list[1], bImportant);
      AddPropertyHolder(eBottom, list[0], bImportant);
      return;
    case 3:
      AddPropertyHolder(eLeft, list[1], bImportant);
      AddPropertyHolder(eTop, list[0], bImportant);
      AddPropertyHolder(eRight, list[1], bImportant);
      AddPropertyHolder(eBottom, list[2], bImportant);
      return;
    case 4:
      AddPropertyHolder(eLeft, list[3], bImportant);
      AddPropertyHolder(eTop, list[0], bImportant);
      AddPropertyHolder(eRight, list[1], bImportant);
      AddPropertyHolder(eBottom, list[2], bImportant);
      return;
    default:
      break;
  }
}

bool CFDE_CSSDeclaration::ParseBorderProperty(
    const FX_WCHAR* pszValue,
    int32_t iValueLen,
    CFX_RetainPtr<CFDE_CSSValue>& pWidth) const {
  pWidth.Reset(nullptr);

  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  FDE_CSSPrimitiveType eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPrimitiveType::Number: {
        if (pWidth)
          continue;

        FX_FLOAT fValue;
        FDE_CSSNumberType eNumType;
        if (ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
          pWidth = pdfium::MakeRetain<CFDE_CSSNumberValue>(eNumType, fValue);
        break;
      }
      case FDE_CSSPrimitiveType::String: {
        const FDE_CSSCOLORTABLE* pColorItem =
            FDE_GetCSSColorByName(CFX_WideStringC(pszValue, iValueLen));
        if (pColorItem)
          continue;

        const FDE_CSSPropertyValueTable* pValue =
            FDE_GetCSSPropertyValueByName(CFX_WideStringC(pszValue, iValueLen));
        if (!pValue)
          continue;

        switch (pValue->eName) {
          case FDE_CSSPropertyValue::Thin:
          case FDE_CSSPropertyValue::Thick:
          case FDE_CSSPropertyValue::Medium:
            if (!pWidth)
              pWidth = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
            break;
          default:
            break;
        }
        break;
      }
      default:
        break;
    }
  }
  if (!pWidth)
    pWidth = pdfium::MakeRetain<CFDE_CSSNumberValue>(FDE_CSSNumberType::Number,
                                                     0.0f);

  return true;
}

void CFDE_CSSDeclaration::ParseFontProperty(const FX_WCHAR* pszValue,
                                            int32_t iValueLen,
                                            bool bImportant) {
  CFDE_CSSValueListParser parser(pszValue, iValueLen, '/');
  CFX_RetainPtr<CFDE_CSSValue> pStyle;
  CFX_RetainPtr<CFDE_CSSValue> pVariant;
  CFX_RetainPtr<CFDE_CSSValue> pWeight;
  CFX_RetainPtr<CFDE_CSSValue> pFontSize;
  CFX_RetainPtr<CFDE_CSSValue> pLineHeight;
  std::vector<CFX_RetainPtr<CFDE_CSSValue>> familyList;
  FDE_CSSPrimitiveType eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPrimitiveType::String: {
        const FDE_CSSPropertyValueTable* pValue =
            FDE_GetCSSPropertyValueByName(CFX_WideStringC(pszValue, iValueLen));
        if (pValue) {
          switch (pValue->eName) {
            case FDE_CSSPropertyValue::XxSmall:
            case FDE_CSSPropertyValue::XSmall:
            case FDE_CSSPropertyValue::Small:
            case FDE_CSSPropertyValue::Medium:
            case FDE_CSSPropertyValue::Large:
            case FDE_CSSPropertyValue::XLarge:
            case FDE_CSSPropertyValue::XxLarge:
            case FDE_CSSPropertyValue::Smaller:
            case FDE_CSSPropertyValue::Larger:
              if (!pFontSize)
                pFontSize =
                    pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              continue;
            case FDE_CSSPropertyValue::Bold:
            case FDE_CSSPropertyValue::Bolder:
            case FDE_CSSPropertyValue::Lighter:
              if (!pWeight)
                pWeight = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              continue;
            case FDE_CSSPropertyValue::Italic:
            case FDE_CSSPropertyValue::Oblique:
              if (!pStyle)
                pStyle = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              continue;
            case FDE_CSSPropertyValue::SmallCaps:
              if (!pVariant)
                pVariant = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              continue;
            case FDE_CSSPropertyValue::Normal:
              if (!pStyle)
                pStyle = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              else if (!pVariant)
                pVariant = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              else if (!pWeight)
                pWeight = pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              else if (!pFontSize)
                pFontSize =
                    pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              else if (!pLineHeight)
                pLineHeight =
                    pdfium::MakeRetain<CFDE_CSSEnumValue>(pValue->eName);
              continue;
            default:
              break;
          }
        }
        if (pFontSize) {
          familyList.push_back(pdfium::MakeRetain<CFDE_CSSStringValue>(
              CFX_WideString(pszValue, iValueLen)));
        }
        parser.m_Separator = ',';
        break;
      }
      case FDE_CSSPrimitiveType::Number: {
        FX_FLOAT fValue;
        FDE_CSSNumberType eNumType;
        if (!ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
          break;
        if (eType == FDE_CSSPrimitiveType::Number) {
          switch ((int32_t)fValue) {
            case 100:
            case 200:
            case 300:
            case 400:
            case 500:
            case 600:
            case 700:
            case 800:
            case 900:
              if (!pWeight)
                pWeight = pdfium::MakeRetain<CFDE_CSSNumberValue>(
                    FDE_CSSNumberType::Number, fValue);
              continue;
          }
        }
        if (!pFontSize)
          pFontSize = pdfium::MakeRetain<CFDE_CSSNumberValue>(eNumType, fValue);
        else if (!pLineHeight)
          pLineHeight =
              pdfium::MakeRetain<CFDE_CSSNumberValue>(eNumType, fValue);
        break;
      }
      default:
        break;
    }
  }

  if (!pStyle) {
    pStyle =
        pdfium::MakeRetain<CFDE_CSSEnumValue>(FDE_CSSPropertyValue::Normal);
  }
  if (!pVariant) {
    pVariant =
        pdfium::MakeRetain<CFDE_CSSEnumValue>(FDE_CSSPropertyValue::Normal);
  }
  if (!pWeight) {
    pWeight =
        pdfium::MakeRetain<CFDE_CSSEnumValue>(FDE_CSSPropertyValue::Normal);
  }
  if (!pFontSize) {
    pFontSize =
        pdfium::MakeRetain<CFDE_CSSEnumValue>(FDE_CSSPropertyValue::Medium);
  }
  if (!pLineHeight) {
    pLineHeight =
        pdfium::MakeRetain<CFDE_CSSEnumValue>(FDE_CSSPropertyValue::Normal);
  }

  AddPropertyHolder(FDE_CSSProperty::FontStyle, pStyle, bImportant);
  AddPropertyHolder(FDE_CSSProperty::FontVariant, pVariant, bImportant);
  AddPropertyHolder(FDE_CSSProperty::FontWeight, pWeight, bImportant);
  AddPropertyHolder(FDE_CSSProperty::FontSize, pFontSize, bImportant);
  AddPropertyHolder(FDE_CSSProperty::LineHeight, pLineHeight, bImportant);
  if (!familyList.empty()) {
    auto pList = pdfium::MakeRetain<CFDE_CSSValueList>(familyList);
    AddPropertyHolder(FDE_CSSProperty::FontFamily, pList, bImportant);
  }
}

size_t CFDE_CSSDeclaration::PropertyCountForTesting() const {
  return properties_.size();
}
