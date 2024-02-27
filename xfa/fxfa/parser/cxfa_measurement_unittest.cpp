// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/cxfa_measurement.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(CXFAMeasurementTest, ToString) {
  CXFA_Measurement measurement;

  measurement.Set(0.1f, XFA_Unit::Percent);
  EXPECT_EQ(L"0.1%", measurement.ToString());
  measurement.Set(1.0f, XFA_Unit::Em);
  EXPECT_EQ(L"1em", measurement.ToString());
  measurement.Set(1.1f, XFA_Unit::Pt);
  EXPECT_EQ(L"1.1pt", measurement.ToString());
  measurement.Set(1.0000001f, XFA_Unit::In);
  EXPECT_EQ(L"1.0000001in", measurement.ToString());
  measurement.Set(1234.0f, XFA_Unit::Pc);
  EXPECT_EQ(L"1234pc", measurement.ToString());
  measurement.Set(987654321.0123456789f, XFA_Unit::Cm);
  EXPECT_EQ(L"9.8765434e+08cm", measurement.ToString());
  measurement.Set(0.0f, XFA_Unit::Mm);
  EXPECT_EQ(L"0mm", measurement.ToString());
  measurement.Set(-2.0f, XFA_Unit::Mp);
  EXPECT_EQ(L"-2mp", measurement.ToString());
}

TEST(CXFAMeasurementTest, GetUnitFromString) {
  EXPECT_EQ(XFA_Unit::Percent, CXFA_Measurement::GetUnitFromString(L"%"));
  EXPECT_EQ(XFA_Unit::Em, CXFA_Measurement::GetUnitFromString(L"em"));
  EXPECT_EQ(XFA_Unit::Pt, CXFA_Measurement::GetUnitFromString(L"pt"));
  EXPECT_EQ(XFA_Unit::In, CXFA_Measurement::GetUnitFromString(L"in"));
  EXPECT_EQ(XFA_Unit::Pc, CXFA_Measurement::GetUnitFromString(L"pc"));
  EXPECT_EQ(XFA_Unit::Cm, CXFA_Measurement::GetUnitFromString(L"cm"));
  EXPECT_EQ(XFA_Unit::Mm, CXFA_Measurement::GetUnitFromString(L"mm"));
  EXPECT_EQ(XFA_Unit::Mp, CXFA_Measurement::GetUnitFromString(L"mp"));

  EXPECT_EQ(XFA_Unit::Unknown, CXFA_Measurement::GetUnitFromString(L""));
  EXPECT_EQ(XFA_Unit::Unknown, CXFA_Measurement::GetUnitFromString(L"foo"));
  EXPECT_EQ(XFA_Unit::Unknown, CXFA_Measurement::GetUnitFromString(L"!"));
  EXPECT_EQ(XFA_Unit::Unknown, CXFA_Measurement::GetUnitFromString(L"CM"));
  EXPECT_EQ(XFA_Unit::Unknown, CXFA_Measurement::GetUnitFromString(L"Cm"));
  EXPECT_EQ(XFA_Unit::Unknown, CXFA_Measurement::GetUnitFromString(L"cM"));
}

TEST(CXFAMeasurementTest, EqualsPrefix) {
  CXFA_Measurement no_unit(L"=5");
  EXPECT_EQ(XFA_Unit::Unknown, no_unit.GetUnit());
  EXPECT_FLOAT_EQ(5.0f, no_unit.GetValue());

  CXFA_Measurement mm_unit(L"=5mm");
  EXPECT_EQ(XFA_Unit::Mm, mm_unit.GetUnit());
  EXPECT_FLOAT_EQ(5.0f, mm_unit.GetValue());
}

TEST(CXFAMeasurementTest, NoPrefix) {
  CXFA_Measurement no_unit(L"5");
  EXPECT_EQ(XFA_Unit::Unknown, no_unit.GetUnit());
  EXPECT_FLOAT_EQ(5.0f, no_unit.GetValue());

  CXFA_Measurement mm_unit(L"5mm");
  EXPECT_EQ(XFA_Unit::Mm, mm_unit.GetUnit());
  EXPECT_FLOAT_EQ(5.0f, mm_unit.GetValue());
}

TEST(CXFAMeasurementTest, InvalidValues) {
  CXFA_Measurement empty(L"");
  EXPECT_EQ(XFA_Unit::Unknown, empty.GetUnit());
  EXPECT_FLOAT_EQ(0.0f, empty.GetValue());

  CXFA_Measurement equals(L"=");
  EXPECT_EQ(XFA_Unit::Unknown, equals.GetUnit());
  EXPECT_FLOAT_EQ(0.0f, equals.GetValue());
}
