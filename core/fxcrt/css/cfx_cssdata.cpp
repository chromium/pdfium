// Copyright 2018 The PDFium Authors
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

#undef CSS_PROP____
#define CSS_PROP____(a, b, c, d) {CFX_CSSProperty::a, c, d},
const CFX_CSSData::Property kPropertyTable[] = {
#include "core/fxcrt/css/properties.inc"
};
#undef CSS_PROP____

#undef CSS_PROP_VALUE____
#define CSS_PROP_VALUE____(a, b, c) {CFX_CSSPropertyValue::a, c},
const CFX_CSSData::PropertyValue kPropertyValueTable[] = {
#include "core/fxcrt/css/property_values.inc"
};
#undef CSS_PROP_VALUE____

const CFX_CSSData::LengthUnit kLengthUnitTable[] = {
    {L"cm", CFX_CSSNumberValue::Unit::kCentiMeters},
    {L"em", CFX_CSSNumberValue::Unit::kEMS},
    {L"ex", CFX_CSSNumberValue::Unit::kEXS},
    {L"in", CFX_CSSNumberValue::Unit::kInches},
    {L"mm", CFX_CSSNumberValue::Unit::kMilliMeters},
    {L"pc", CFX_CSSNumberValue::Unit::kPicas},
    {L"pt", CFX_CSSNumberValue::Unit::kPoints},
    {L"px", CFX_CSSNumberValue::Unit::kPixels},
};

// 16 colours from CSS 2.0 + alternate spelling of grey/gray.
const CFX_CSSData::Color kColorTable[] = {
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

  uint32_t hash = FX_HashCode_GetLoweredW(name);
  auto* result = std::lower_bound(
      std::begin(kPropertyTable), std::end(kPropertyTable), hash,
      [](const CFX_CSSData::Property& iter, const uint32_t& hash) {
        return iter.dwHash < hash;
      });

  if (result != std::end(kPropertyTable) && result->dwHash == hash) {
    return result;
  }
  return nullptr;
}

const CFX_CSSData::Property* CFX_CSSData::GetPropertyByEnum(
    CFX_CSSProperty property) {
  return &kPropertyTable[static_cast<uint8_t>(property)];
}

const CFX_CSSData::PropertyValue* CFX_CSSData::GetPropertyValueByName(
    WideStringView wsName) {
  if (wsName.IsEmpty())
    return nullptr;

  uint32_t hash = FX_HashCode_GetLoweredW(wsName);
  auto* result = std::lower_bound(
      std::begin(kPropertyValueTable), std::end(kPropertyValueTable), hash,
      [](const PropertyValue& iter, const uint32_t& hash) {
        return iter.dwHash < hash;
      });

  if (result != std::end(kPropertyValueTable) && result->dwHash == hash) {
    return result;
  }
  return nullptr;
}

const CFX_CSSData::LengthUnit* CFX_CSSData::GetLengthUnitByName(
    WideStringView wsName) {
  if (wsName.IsEmpty() || wsName.GetLength() != 2)
    return nullptr;

  WideString lowerName = WideString(wsName);
  lowerName.MakeLower();

  for (auto* iter = std::begin(kLengthUnitTable);
       iter != std::end(kLengthUnitTable); ++iter) {
    if (lowerName == iter->value)
      return iter;
  }

  return nullptr;
}

const CFX_CSSData::Color* CFX_CSSData::GetColorByName(WideStringView wsName) {
  if (wsName.IsEmpty())
    return nullptr;

  WideString lowerName = WideString(wsName);
  lowerName.MakeLower();

  for (auto* iter = std::begin(kColorTable); iter != std::end(kColorTable);
       ++iter) {
    if (lowerName == iter->name)
      return iter;
  }
  return nullptr;
}
