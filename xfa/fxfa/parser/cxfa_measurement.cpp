// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_measurement.h"

#include "core/fxcrt/fx_extension.h"

namespace {

constexpr float kPtToInch = 72;
constexpr float kPtToCm = kPtToInch / 2.54f;
constexpr float kPtToMm = kPtToCm / 10;
constexpr float kPtToMp = 0.001f;
constexpr float kPtToPc = 12;

}  // namespace

CXFA_Measurement::CXFA_Measurement(const WideStringView& wsMeasure) {
  SetString(wsMeasure);
}

CXFA_Measurement::CXFA_Measurement() {
  Set(-1, XFA_UNIT_Unknown);
}

CXFA_Measurement::CXFA_Measurement(float fValue, XFA_UNIT eUnit) {
  Set(fValue, eUnit);
}

void CXFA_Measurement::SetString(const WideStringView& wsMeasure) {
  if (wsMeasure.IsEmpty()) {
    m_fValue = 0;
    m_eUnit = XFA_UNIT_Unknown;
    return;
  }
  int32_t iUsedLen = 0;
  int32_t iOffset = (wsMeasure[0] == L'=') ? 1 : 0;
  float fValue = FXSYS_wcstof(wsMeasure.unterminated_c_str() + iOffset,
                              wsMeasure.GetLength() - iOffset, &iUsedLen);
  XFA_UNIT eUnit = GetUnitFromString(
      wsMeasure.Right(wsMeasure.GetLength() - (iOffset + iUsedLen)));
  Set(fValue, eUnit);
}

bool CXFA_Measurement::ToString(WideString* wsMeasure) const {
  switch (GetUnit()) {
    case XFA_UNIT_Mm:
      wsMeasure->Format(L"%.8gmm", GetValue());
      return true;
    case XFA_UNIT_Pt:
      wsMeasure->Format(L"%.8gpt", GetValue());
      return true;
    case XFA_UNIT_In:
      wsMeasure->Format(L"%.8gin", GetValue());
      return true;
    case XFA_UNIT_Cm:
      wsMeasure->Format(L"%.8gcm", GetValue());
      return true;
    case XFA_UNIT_Mp:
      wsMeasure->Format(L"%.8gmp", GetValue());
      return true;
    case XFA_UNIT_Pc:
      wsMeasure->Format(L"%.8gpc", GetValue());
      return true;
    case XFA_UNIT_Em:
      wsMeasure->Format(L"%.8gem", GetValue());
      return true;
    case XFA_UNIT_Percent:
      wsMeasure->Format(L"%.8g%%", GetValue());
      return true;
    default:
      wsMeasure->Format(L"%.8g", GetValue());
      return false;
  }
}

float CXFA_Measurement::ToUnit(XFA_UNIT eUnit) const {
  float f;
  return ToUnitInternal(eUnit, &f) ? f : 0;
}

bool CXFA_Measurement::ToUnitInternal(XFA_UNIT eUnit, float* fValue) const {
  *fValue = GetValue();
  XFA_UNIT eFrom = GetUnit();
  if (eFrom == eUnit)
    return true;

  switch (eFrom) {
    case XFA_UNIT_Pt:
      break;
    case XFA_UNIT_Mm:
      *fValue *= kPtToMm;
      break;
    case XFA_UNIT_In:
      *fValue *= kPtToInch;
      break;
    case XFA_UNIT_Cm:
      *fValue *= kPtToCm;
      break;
    case XFA_UNIT_Mp:
      *fValue *= kPtToMp;
      break;
    case XFA_UNIT_Pc:
      *fValue *= kPtToPc;
      break;
    default:
      *fValue = 0;
      return false;
  }
  switch (eUnit) {
    case XFA_UNIT_Pt:
      return true;
    case XFA_UNIT_Mm:
      *fValue /= kPtToMm;
      return true;
    case XFA_UNIT_In:
      *fValue /= kPtToInch;
      return true;
    case XFA_UNIT_Cm:
      *fValue /= kPtToCm;
      return true;
    case XFA_UNIT_Mp:
      *fValue /= kPtToMp;
      return true;
    case XFA_UNIT_Pc:
      *fValue /= kPtToPc;
      return true;
    default:
      NOTREACHED();
      return false;
  }
}

// static
XFA_UNIT CXFA_Measurement::GetUnitFromString(const WideStringView& wsUnit) {
  if (wsUnit == L"mm")
    return XFA_UNIT_Mm;
  if (wsUnit == L"pt")
    return XFA_UNIT_Pt;
  if (wsUnit == L"in")
    return XFA_UNIT_In;
  if (wsUnit == L"cm")
    return XFA_UNIT_Cm;
  if (wsUnit == L"pc")
    return XFA_UNIT_Pc;
  if (wsUnit == L"mp")
    return XFA_UNIT_Mp;
  if (wsUnit == L"em")
    return XFA_UNIT_Em;
  if (wsUnit == L"%")
    return XFA_UNIT_Percent;
  return XFA_UNIT_Unknown;
}
