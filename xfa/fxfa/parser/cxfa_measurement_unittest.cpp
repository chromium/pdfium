// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/cxfa_measurement.h"

#include "testing/gtest/include/gtest/gtest.h"

TEST(CXFAMeasurementTest, ToString) {
  CXFA_Measurement measurement;

  measurement.Set(0.1f, XFA_Unit::Percent);
  EXPECT_STREQ(L"0.1%", measurement.ToString().c_str());
  measurement.Set(1.0f, XFA_Unit::Em);
  EXPECT_STREQ(L"1em", measurement.ToString().c_str());
  measurement.Set(1.1f, XFA_Unit::Pt);
  EXPECT_STREQ(L"1.1pt", measurement.ToString().c_str());
  measurement.Set(1.0000001f, XFA_Unit::In);
  EXPECT_STREQ(L"1.0000001in", measurement.ToString().c_str());
  measurement.Set(1234.0f, XFA_Unit::Pc);
  EXPECT_STREQ(L"1234pc", measurement.ToString().c_str());
  measurement.Set(987654321.0123456789f, XFA_Unit::Cm);
  EXPECT_STREQ(L"9.8765434e+08cm", measurement.ToString().c_str());
  measurement.Set(0.0f, XFA_Unit::Mm);
  EXPECT_STREQ(L"0mm", measurement.ToString().c_str());
  measurement.Set(-2.0f, XFA_Unit::Mp);
  EXPECT_STREQ(L"-2mp", measurement.ToString().c_str());
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
