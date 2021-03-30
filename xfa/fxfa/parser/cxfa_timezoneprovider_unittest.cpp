// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/cxfa_timezoneprovider.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/scoped_set_tz.h"

TEST(CXFA_TimeZoneProviderTest, HourOffsets) {
  {
    ScopedSetTZ scoped_set_tz("UTC");
    CXFA_TimeZoneProvider provider;
    EXPECT_EQ(0, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
  {
    ScopedSetTZ scoped_set_tz("UTC+1");
    CXFA_TimeZoneProvider provider;
    EXPECT_EQ(-60, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-1");
    CXFA_TimeZoneProvider provider;
    EXPECT_EQ(60, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
  {
    ScopedSetTZ scoped_set_tz("UTC+14");
    CXFA_TimeZoneProvider provider;
    EXPECT_EQ(-840, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-14");
    CXFA_TimeZoneProvider provider;
    EXPECT_EQ(840, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
}

TEST(CXFA_TimeZoneProviderTest, HalfHourOffsets) {
  {
    ScopedSetTZ scoped_set_tz("UTC+0:30");
    CXFA_TimeZoneProvider provider;
    // TODO(crbug.com/pdfium/1662): Should be -30.
    EXPECT_EQ(30, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-0:30");
    CXFA_TimeZoneProvider provider;
    EXPECT_EQ(30, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
  {
    ScopedSetTZ scoped_set_tz("UTC+1:30");
    CXFA_TimeZoneProvider provider;
    // TODO(crbug.com/pdfium/1662): Should be -90.
    EXPECT_EQ(-30, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-1:30");
    CXFA_TimeZoneProvider provider;
    EXPECT_EQ(90, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
  {
    ScopedSetTZ scoped_set_tz("UTC+9:30");
    CXFA_TimeZoneProvider provider;
    // TODO(crbug.com/pdfium/1662): Should be -570.
    EXPECT_EQ(-510, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-9:30");
    CXFA_TimeZoneProvider provider;
    EXPECT_EQ(570, FX_TimeZoneOffsetInMinutes(provider.GetTimeZone()));
  }
}
