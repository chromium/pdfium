// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/css/cfx_cssdeclaration.h"

#include "core/fxcrt/css/cfx_csscolorvalue.h"
#include "core/fxcrt/css/cfx_csscustomproperty.h"
#include "core/fxcrt/css/cfx_cssenumvalue.h"
#include "core/fxcrt/css/cfx_cssnumbervalue.h"
#include "core/fxcrt/css/cfx_csspropertyholder.h"
#include "core/fxcrt/css/cfx_cssstringvalue.h"
#include "core/fxcrt/css/cfx_cssvaluelist.h"
#include "core/fxcrt/css/cfx_cssvaluelistparser.h"
#include "core/fxcrt/fx_extension.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"

namespace {

uint8_t Hex2Dec(uint8_t hexHigh, uint8_t hexLow) {
  return (FXSYS_HexCharToInt(hexHigh) << 4) + FXSYS_HexCharToInt(hexLow);
}

struct CFX_CSSPropertyValueEntry {
  CFX_CSSPropertyValue eName;
  const wchar_t* pszName;
  uint32_t dwHash;
};

const CFX_CSSPropertyValueEntry propertyValueTable[] = {
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

struct CFX_CSSLengthUnitEntry {
  const wchar_t* value;
  CFX_CSSNumberType type;
};
const CFX_CSSLengthUnitEntry lengthUnitTable[] = {
    {L"cm", CFX_CSSNumberType::CentiMeters}, {L"em", CFX_CSSNumberType::EMS},
    {L"ex", CFX_CSSNumberType::EXS},         {L"in", CFX_CSSNumberType::Inches},
    {L"mm", CFX_CSSNumberType::MilliMeters}, {L"pc", CFX_CSSNumberType::Picas},
    {L"pt", CFX_CSSNumberType::Points},      {L"px", CFX_CSSNumberType::Pixels},
};

struct CFX_CSSColorEntry {
  const wchar_t* name;
  FX_ARGB value;
};
// 16 colours from CSS 2.0 + alternate spelling of grey/gray.
const CFX_CSSColorEntry colorTable[] = {
    {L"aqua", 0xff00ffff},    {L"black", 0xff000000}, {L"blue", 0xff0000ff},
    {L"fuchsia", 0xffff00ff}, {L"gray", 0xff808080},  {L"green", 0xff008000},
    {L"grey", 0xff808080},    {L"lime", 0xff00ff00},  {L"maroon", 0xff800000},
    {L"navy", 0xff000080},    {L"olive", 0xff808000}, {L"orange", 0xffffa500},
    {L"purple", 0xff800080},  {L"red", 0xffff0000},   {L"silver", 0xffc0c0c0},
    {L"teal", 0xff008080},    {L"white", 0xffffffff}, {L"yellow", 0xffffff00},
};

const CFX_CSSPropertyValueEntry* GetCSSPropertyValueByName(
    WideStringView wsName) {
  if (wsName.IsEmpty())
    return nullptr;

  uint32_t hash = FX_HashCode_GetW(wsName, true);
  auto* result = std::lower_bound(
      std::begin(propertyValueTable), std::end(propertyValueTable), hash,
      [](const CFX_CSSPropertyValueEntry& iter, const uint32_t& hash) {
        return iter.dwHash < hash;
      });
  if (result != std::end(propertyValueTable) && result->dwHash == hash)
    return result;
  return nullptr;
}

const CFX_CSSLengthUnitEntry* GetCSSLengthUnitByName(WideStringView wsName) {
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

const CFX_CSSColorEntry* GetCSSColorByName(WideStringView wsName) {
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

bool ParseCSSNumber(const wchar_t* pszValue,
                    int32_t iValueLen,
                    float& fValue,
                    CFX_CSSNumberType& eUnit) {
  ASSERT(pszValue && iValueLen > 0);
  int32_t iUsedLen = 0;
  fValue = FXSYS_wcstof(pszValue, iValueLen, &iUsedLen);
  if (iUsedLen <= 0)
    return false;

  iValueLen -= iUsedLen;
  pszValue += iUsedLen;
  eUnit = CFX_CSSNumberType::Number;
  if (iValueLen >= 1 && *pszValue == '%') {
    eUnit = CFX_CSSNumberType::Percent;
  } else if (iValueLen == 2) {
    const CFX_CSSLengthUnitEntry* pUnit =
        GetCSSLengthUnitByName(WideStringView(pszValue, 2));
    if (pUnit)
      eUnit = pUnit->type;
  }
  return true;
}

}  // namespace

// static
bool CFX_CSSDeclaration::ParseCSSString(const wchar_t* pszValue,
                                        int32_t iValueLen,
                                        int32_t* iOffset,
                                        int32_t* iLength) {
  ASSERT(pszValue && iValueLen > 0);
  *iOffset = 0;
  *iLength = iValueLen;
  if (iValueLen >= 2) {
    wchar_t first = pszValue[0], last = pszValue[iValueLen - 1];
    if ((first == '\"' && last == '\"') || (first == '\'' && last == '\'')) {
      *iOffset = 1;
      *iLength -= 2;
    }
  }
  return iValueLen > 0;
}

// static.
bool CFX_CSSDeclaration::ParseCSSColor(const wchar_t* pszValue,
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
    float fValue;
    CFX_CSSPrimitiveType eType;
    CFX_CSSValueListParser list(pszValue + 4, iValueLen - 5, ',');
    for (int32_t i = 0; i < 3; ++i) {
      if (!list.NextValue(&eType, &pszValue, &iValueLen))
        return false;
      if (eType != CFX_CSSPrimitiveType::Number)
        return false;
      CFX_CSSNumberType eNumType;
      if (!ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
        return false;

      rgb[i] = eNumType == CFX_CSSNumberType::Percent
                   ? FXSYS_round(fValue * 2.55f)
                   : FXSYS_round(fValue);
    }
    *dwColor = ArgbEncode(255, rgb[0], rgb[1], rgb[2]);
    return true;
  }

  const CFX_CSSColorEntry* pColor =
      GetCSSColorByName(WideStringView(pszValue, iValueLen));
  if (!pColor)
    return false;

  *dwColor = pColor->value;
  return true;
}

CFX_CSSDeclaration::CFX_CSSDeclaration() {}

CFX_CSSDeclaration::~CFX_CSSDeclaration() {}

RetainPtr<CFX_CSSValue> CFX_CSSDeclaration::GetProperty(
    CFX_CSSProperty eProperty,
    bool* bImportant) const {
  for (const auto& p : properties_) {
    if (p->eProperty == eProperty) {
      *bImportant = p->bImportant;
      return p->pValue;
    }
  }
  return nullptr;
}

void CFX_CSSDeclaration::AddPropertyHolder(CFX_CSSProperty eProperty,
                                           RetainPtr<CFX_CSSValue> pValue,
                                           bool bImportant) {
  auto pHolder = pdfium::MakeUnique<CFX_CSSPropertyHolder>();
  pHolder->bImportant = bImportant;
  pHolder->eProperty = eProperty;
  pHolder->pValue = pValue;
  properties_.push_back(std::move(pHolder));
}

void CFX_CSSDeclaration::AddProperty(const CFX_CSSPropertyTable::Entry* pEntry,
                                     const WideStringView& value) {
  ASSERT(!value.IsEmpty());

  const wchar_t* pszValue = value.unterminated_c_str();
  int32_t iValueLen = value.GetLength();
  bool bImportant = false;
  if (iValueLen >= 10 && pszValue[iValueLen - 10] == '!' &&
      FXSYS_wcsnicmp(L"important", pszValue + iValueLen - 9, 9) == 0) {
    if ((iValueLen -= 10) == 0)
      return;

    bImportant = true;
  }
  const uint32_t dwType = pEntry->dwType;
  switch (dwType & 0x0F) {
    case CFX_CSSVALUETYPE_Primitive: {
      static const uint32_t g_ValueGuessOrder[] = {
          CFX_CSSVALUETYPE_MaybeNumber, CFX_CSSVALUETYPE_MaybeEnum,
          CFX_CSSVALUETYPE_MaybeColor, CFX_CSSVALUETYPE_MaybeString,
      };
      static const int32_t g_ValueGuessCount =
          sizeof(g_ValueGuessOrder) / sizeof(uint32_t);
      for (int32_t i = 0; i < g_ValueGuessCount; ++i) {
        const uint32_t dwMatch = dwType & g_ValueGuessOrder[i];
        if (dwMatch == 0) {
          continue;
        }
        RetainPtr<CFX_CSSValue> pCSSValue;
        switch (dwMatch) {
          case CFX_CSSVALUETYPE_MaybeNumber:
            pCSSValue = ParseNumber(pszValue, iValueLen);
            break;
          case CFX_CSSVALUETYPE_MaybeEnum:
            pCSSValue = ParseEnum(pszValue, iValueLen);
            break;
          case CFX_CSSVALUETYPE_MaybeColor:
            pCSSValue = ParseColor(pszValue, iValueLen);
            break;
          case CFX_CSSVALUETYPE_MaybeString:
            pCSSValue = ParseString(pszValue, iValueLen);
            break;
          default:
            break;
        }
        if (pCSSValue) {
          AddPropertyHolder(pEntry->eName, pCSSValue, bImportant);
          return;
        }

        if ((dwType & ~(g_ValueGuessOrder[i])) == CFX_CSSVALUETYPE_Primitive)
          return;
      }
      break;
    }
    case CFX_CSSVALUETYPE_Shorthand: {
      RetainPtr<CFX_CSSValue> pWidth;
      switch (pEntry->eName) {
        case CFX_CSSProperty::Font:
          ParseFontProperty(pszValue, iValueLen, bImportant);
          return;
        case CFX_CSSProperty::Border:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(CFX_CSSProperty::BorderLeftWidth, pWidth,
                              bImportant);
            AddPropertyHolder(CFX_CSSProperty::BorderTopWidth, pWidth,
                              bImportant);
            AddPropertyHolder(CFX_CSSProperty::BorderRightWidth, pWidth,
                              bImportant);
            AddPropertyHolder(CFX_CSSProperty::BorderBottomWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case CFX_CSSProperty::BorderLeft:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(CFX_CSSProperty::BorderLeftWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case CFX_CSSProperty::BorderTop:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(CFX_CSSProperty::BorderTopWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case CFX_CSSProperty::BorderRight:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(CFX_CSSProperty::BorderRightWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        case CFX_CSSProperty::BorderBottom:
          if (ParseBorderProperty(pszValue, iValueLen, pWidth)) {
            AddPropertyHolder(CFX_CSSProperty::BorderBottomWidth, pWidth,
                              bImportant);
            return;
          }
          break;
        default:
          break;
      }
    } break;
    case CFX_CSSVALUETYPE_List:
      ParseValueListProperty(pEntry, pszValue, iValueLen, bImportant);
      return;
    default:
      NOTREACHED();
      break;
  }
}

void CFX_CSSDeclaration::AddProperty(const WideString& prop,
                                     const WideString& value) {
  custom_properties_.push_back(
      pdfium::MakeUnique<CFX_CSSCustomProperty>(prop, value));
}

RetainPtr<CFX_CSSValue> CFX_CSSDeclaration::ParseNumber(const wchar_t* pszValue,
                                                        int32_t iValueLen) {
  float fValue;
  CFX_CSSNumberType eUnit;
  if (!ParseCSSNumber(pszValue, iValueLen, fValue, eUnit))
    return nullptr;
  return pdfium::MakeRetain<CFX_CSSNumberValue>(eUnit, fValue);
}

RetainPtr<CFX_CSSValue> CFX_CSSDeclaration::ParseEnum(const wchar_t* pszValue,
                                                      int32_t iValueLen) {
  const CFX_CSSPropertyValueEntry* pValue =
      GetCSSPropertyValueByName(WideStringView(pszValue, iValueLen));
  return pValue ? pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName) : nullptr;
}

RetainPtr<CFX_CSSValue> CFX_CSSDeclaration::ParseColor(const wchar_t* pszValue,
                                                       int32_t iValueLen) {
  FX_ARGB dwColor;
  if (!ParseCSSColor(pszValue, iValueLen, &dwColor))
    return nullptr;
  return pdfium::MakeRetain<CFX_CSSColorValue>(dwColor);
}

RetainPtr<CFX_CSSValue> CFX_CSSDeclaration::ParseString(const wchar_t* pszValue,
                                                        int32_t iValueLen) {
  int32_t iOffset;
  if (!ParseCSSString(pszValue, iValueLen, &iOffset, &iValueLen))
    return nullptr;

  if (iValueLen <= 0)
    return nullptr;

  return pdfium::MakeRetain<CFX_CSSStringValue>(
      WideString(pszValue + iOffset, iValueLen));
}

void CFX_CSSDeclaration::ParseValueListProperty(
    const CFX_CSSPropertyTable::Entry* pEntry,
    const wchar_t* pszValue,
    int32_t iValueLen,
    bool bImportant) {
  wchar_t separator =
      (pEntry->eName == CFX_CSSProperty::FontFamily) ? ',' : ' ';
  CFX_CSSValueListParser parser(pszValue, iValueLen, separator);

  const uint32_t dwType = pEntry->dwType;
  CFX_CSSPrimitiveType eType;
  std::vector<RetainPtr<CFX_CSSValue>> list;
  while (parser.NextValue(&eType, &pszValue, &iValueLen)) {
    switch (eType) {
      case CFX_CSSPrimitiveType::Number:
        if (dwType & CFX_CSSVALUETYPE_MaybeNumber) {
          float fValue;
          CFX_CSSNumberType eNumType;
          if (ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
            list.push_back(
                pdfium::MakeRetain<CFX_CSSNumberValue>(eNumType, fValue));
        }
        break;
      case CFX_CSSPrimitiveType::String:
        if (dwType & CFX_CSSVALUETYPE_MaybeColor) {
          FX_ARGB dwColor;
          if (ParseCSSColor(pszValue, iValueLen, &dwColor)) {
            list.push_back(pdfium::MakeRetain<CFX_CSSColorValue>(dwColor));
            continue;
          }
        }
        if (dwType & CFX_CSSVALUETYPE_MaybeEnum) {
          const CFX_CSSPropertyValueEntry* pValue =
              GetCSSPropertyValueByName(WideStringView(pszValue, iValueLen));
          if (pValue) {
            list.push_back(pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName));
            continue;
          }
        }
        if (dwType & CFX_CSSVALUETYPE_MaybeString) {
          list.push_back(pdfium::MakeRetain<CFX_CSSStringValue>(
              WideString(pszValue, iValueLen)));
        }
        break;
      case CFX_CSSPrimitiveType::RGB:
        if (dwType & CFX_CSSVALUETYPE_MaybeColor) {
          FX_ARGB dwColor;
          if (ParseCSSColor(pszValue, iValueLen, &dwColor)) {
            list.push_back(pdfium::MakeRetain<CFX_CSSColorValue>(dwColor));
          }
        }
        break;
      default:
        break;
    }
  }
  if (list.empty())
    return;

  switch (pEntry->eName) {
    case CFX_CSSProperty::BorderWidth:
      Add4ValuesProperty(list, bImportant, CFX_CSSProperty::BorderLeftWidth,
                         CFX_CSSProperty::BorderTopWidth,
                         CFX_CSSProperty::BorderRightWidth,
                         CFX_CSSProperty::BorderBottomWidth);
      return;
    case CFX_CSSProperty::Margin:
      Add4ValuesProperty(list, bImportant, CFX_CSSProperty::MarginLeft,
                         CFX_CSSProperty::MarginTop,
                         CFX_CSSProperty::MarginRight,
                         CFX_CSSProperty::MarginBottom);
      return;
    case CFX_CSSProperty::Padding:
      Add4ValuesProperty(list, bImportant, CFX_CSSProperty::PaddingLeft,
                         CFX_CSSProperty::PaddingTop,
                         CFX_CSSProperty::PaddingRight,
                         CFX_CSSProperty::PaddingBottom);
      return;
    default: {
      auto pList = pdfium::MakeRetain<CFX_CSSValueList>(list);
      AddPropertyHolder(pEntry->eName, pList, bImportant);
      return;
    }
  }
}

void CFX_CSSDeclaration::Add4ValuesProperty(
    const std::vector<RetainPtr<CFX_CSSValue>>& list,
    bool bImportant,
    CFX_CSSProperty eLeft,
    CFX_CSSProperty eTop,
    CFX_CSSProperty eRight,
    CFX_CSSProperty eBottom) {
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

bool CFX_CSSDeclaration::ParseBorderProperty(
    const wchar_t* pszValue,
    int32_t iValueLen,
    RetainPtr<CFX_CSSValue>& pWidth) const {
  pWidth.Reset(nullptr);

  CFX_CSSValueListParser parser(pszValue, iValueLen, ' ');
  CFX_CSSPrimitiveType eType;
  while (parser.NextValue(&eType, &pszValue, &iValueLen)) {
    switch (eType) {
      case CFX_CSSPrimitiveType::Number: {
        if (pWidth)
          continue;

        float fValue;
        CFX_CSSNumberType eNumType;
        if (ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
          pWidth = pdfium::MakeRetain<CFX_CSSNumberValue>(eNumType, fValue);
        break;
      }
      case CFX_CSSPrimitiveType::String: {
        const CFX_CSSColorEntry* pColorItem =
            GetCSSColorByName(WideStringView(pszValue, iValueLen));
        if (pColorItem)
          continue;

        const CFX_CSSPropertyValueEntry* pValue =
            GetCSSPropertyValueByName(WideStringView(pszValue, iValueLen));
        if (!pValue)
          continue;

        switch (pValue->eName) {
          case CFX_CSSPropertyValue::Thin:
          case CFX_CSSPropertyValue::Thick:
          case CFX_CSSPropertyValue::Medium:
            if (!pWidth)
              pWidth = pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName);
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
    pWidth =
        pdfium::MakeRetain<CFX_CSSNumberValue>(CFX_CSSNumberType::Number, 0.0f);

  return true;
}

void CFX_CSSDeclaration::ParseFontProperty(const wchar_t* pszValue,
                                           int32_t iValueLen,
                                           bool bImportant) {
  CFX_CSSValueListParser parser(pszValue, iValueLen, '/');
  RetainPtr<CFX_CSSValue> pStyle;
  RetainPtr<CFX_CSSValue> pVariant;
  RetainPtr<CFX_CSSValue> pWeight;
  RetainPtr<CFX_CSSValue> pFontSize;
  RetainPtr<CFX_CSSValue> pLineHeight;
  std::vector<RetainPtr<CFX_CSSValue>> familyList;
  CFX_CSSPrimitiveType eType;
  while (parser.NextValue(&eType, &pszValue, &iValueLen)) {
    switch (eType) {
      case CFX_CSSPrimitiveType::String: {
        const CFX_CSSPropertyValueEntry* pValue =
            GetCSSPropertyValueByName(WideStringView(pszValue, iValueLen));
        if (pValue) {
          switch (pValue->eName) {
            case CFX_CSSPropertyValue::XxSmall:
            case CFX_CSSPropertyValue::XSmall:
            case CFX_CSSPropertyValue::Small:
            case CFX_CSSPropertyValue::Medium:
            case CFX_CSSPropertyValue::Large:
            case CFX_CSSPropertyValue::XLarge:
            case CFX_CSSPropertyValue::XxLarge:
            case CFX_CSSPropertyValue::Smaller:
            case CFX_CSSPropertyValue::Larger:
              if (!pFontSize)
                pFontSize = pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName);
              continue;
            case CFX_CSSPropertyValue::Bold:
            case CFX_CSSPropertyValue::Bolder:
            case CFX_CSSPropertyValue::Lighter:
              if (!pWeight)
                pWeight = pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName);
              continue;
            case CFX_CSSPropertyValue::Italic:
            case CFX_CSSPropertyValue::Oblique:
              if (!pStyle)
                pStyle = pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName);
              continue;
            case CFX_CSSPropertyValue::SmallCaps:
              if (!pVariant)
                pVariant = pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName);
              continue;
            case CFX_CSSPropertyValue::Normal:
              if (!pStyle)
                pStyle = pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName);
              else if (!pVariant)
                pVariant = pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName);
              else if (!pWeight)
                pWeight = pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName);
              else if (!pFontSize)
                pFontSize = pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName);
              else if (!pLineHeight)
                pLineHeight =
                    pdfium::MakeRetain<CFX_CSSEnumValue>(pValue->eName);
              continue;
            default:
              break;
          }
        }
        if (pFontSize) {
          familyList.push_back(pdfium::MakeRetain<CFX_CSSStringValue>(
              WideString(pszValue, iValueLen)));
        }
        parser.UseCommaSeparator();
        break;
      }
      case CFX_CSSPrimitiveType::Number: {
        float fValue;
        CFX_CSSNumberType eNumType;
        if (!ParseCSSNumber(pszValue, iValueLen, fValue, eNumType))
          break;
        if (eType == CFX_CSSPrimitiveType::Number) {
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
                pWeight = pdfium::MakeRetain<CFX_CSSNumberValue>(
                    CFX_CSSNumberType::Number, fValue);
              continue;
          }
        }
        if (!pFontSize)
          pFontSize = pdfium::MakeRetain<CFX_CSSNumberValue>(eNumType, fValue);
        else if (!pLineHeight)
          pLineHeight =
              pdfium::MakeRetain<CFX_CSSNumberValue>(eNumType, fValue);
        break;
      }
      default:
        break;
    }
  }

  if (!pStyle) {
    pStyle = pdfium::MakeRetain<CFX_CSSEnumValue>(CFX_CSSPropertyValue::Normal);
  }
  if (!pVariant) {
    pVariant =
        pdfium::MakeRetain<CFX_CSSEnumValue>(CFX_CSSPropertyValue::Normal);
  }
  if (!pWeight) {
    pWeight =
        pdfium::MakeRetain<CFX_CSSEnumValue>(CFX_CSSPropertyValue::Normal);
  }
  if (!pFontSize) {
    pFontSize =
        pdfium::MakeRetain<CFX_CSSEnumValue>(CFX_CSSPropertyValue::Medium);
  }
  if (!pLineHeight) {
    pLineHeight =
        pdfium::MakeRetain<CFX_CSSEnumValue>(CFX_CSSPropertyValue::Normal);
  }

  AddPropertyHolder(CFX_CSSProperty::FontStyle, pStyle, bImportant);
  AddPropertyHolder(CFX_CSSProperty::FontVariant, pVariant, bImportant);
  AddPropertyHolder(CFX_CSSProperty::FontWeight, pWeight, bImportant);
  AddPropertyHolder(CFX_CSSProperty::FontSize, pFontSize, bImportant);
  AddPropertyHolder(CFX_CSSProperty::LineHeight, pLineHeight, bImportant);
  if (!familyList.empty()) {
    auto pList = pdfium::MakeRetain<CFX_CSSValueList>(familyList);
    AddPropertyHolder(CFX_CSSProperty::FontFamily, pList, bImportant);
  }
}

size_t CFX_CSSDeclaration::PropertyCountForTesting() const {
  return properties_.size();
}
