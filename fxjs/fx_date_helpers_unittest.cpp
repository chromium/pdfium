// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/fx_date_helpers.h"

#include "testing/gtest/include/gtest/gtest.h"

namespace {

constexpr double kMilliSecondsInADay = 1000 * 60 * 60 * 24;

}  // namespace

TEST(FX_DateHelper, GetYearFromTime) {
  static constexpr struct {
    double time_ms;
    int expected_year;
  } kTests[] = {
      {-400 * kMilliSecondsInADay, 1968},
      {-1, 1969},
      {0, 1970},
      {1, 1970},
      {364.9 * kMilliSecondsInADay, 1970},
      {365.0 * kMilliSecondsInADay, 1971},
      {365.1 * kMilliSecondsInADay, 1971},
      {2 * 365.0 * kMilliSecondsInADay, 1972},
      // 1972 is a leap year, so there should be an extra day.
      {3 * 365.0 * kMilliSecondsInADay, 1972},
      {(3 * 365.0 + 1) * kMilliSecondsInADay, 1973},
  };

  for (const auto& test : kTests) {
    EXPECT_EQ(test.expected_year, FX_GetYearFromTime(test.time_ms))
        << test.time_ms;
  }
}

TEST(FX_DateHelper, GetMonthFromTime) {
  static constexpr struct {
    double time_ms;
    int expected_month;  // Zero-based.
  } kTests[] = {
      {-400 * kMilliSecondsInADay, 10},
      {-1, 11},
      {0, 0},
      {1, 0},
      {364.9 * kMilliSecondsInADay, 11},
      {365.0 * kMilliSecondsInADay, 0},
      {365.1 * kMilliSecondsInADay, 0},
      // 1972 is a leap year, so there should be an extra day.
      {2 * 365.0 * kMilliSecondsInADay, 0},
      {3 * 365.0 * kMilliSecondsInADay, 11},
      {(3 * 365.0 + 1) * kMilliSecondsInADay, 0},
      // Tests boundaries for all months in 1970 not already covered above.
      {30 * kMilliSecondsInADay, 0},
      {31 * kMilliSecondsInADay, 1},
      {58 * kMilliSecondsInADay, 1},
      {59 * kMilliSecondsInADay, 2},
      {89 * kMilliSecondsInADay, 2},
      {90 * kMilliSecondsInADay, 3},
      {119 * kMilliSecondsInADay, 3},
      {120 * kMilliSecondsInADay, 4},
      {150 * kMilliSecondsInADay, 4},
      {151 * kMilliSecondsInADay, 5},
      {180 * kMilliSecondsInADay, 5},
      {181 * kMilliSecondsInADay, 6},
      {211 * kMilliSecondsInADay, 6},
      {212 * kMilliSecondsInADay, 7},
      {242 * kMilliSecondsInADay, 7},
      {243 * kMilliSecondsInADay, 8},
      {272 * kMilliSecondsInADay, 8},
      {273 * kMilliSecondsInADay, 9},
      {303 * kMilliSecondsInADay, 9},
      {304 * kMilliSecondsInADay, 10},
      {333 * kMilliSecondsInADay, 10},
      {334 * kMilliSecondsInADay, 11},
      {364 * kMilliSecondsInADay, 11},
      // Tests boundaries for all months in 1972 not already covered above.
      {(2 * 365.0 + 30) * kMilliSecondsInADay, 0},
      {(2 * 365.0 + 31) * kMilliSecondsInADay, 1},
      {(2 * 365.0 + 59) * kMilliSecondsInADay, 1},
      {(2 * 365.0 + 60) * kMilliSecondsInADay, 2},
      {(2 * 365.0 + 90) * kMilliSecondsInADay, 2},
      {(2 * 365.0 + 91) * kMilliSecondsInADay, 3},
      {(2 * 365.0 + 120) * kMilliSecondsInADay, 3},
      {(2 * 365.0 + 121) * kMilliSecondsInADay, 4},
      {(2 * 365.0 + 151) * kMilliSecondsInADay, 4},
      {(2 * 365.0 + 152) * kMilliSecondsInADay, 5},
      {(2 * 365.0 + 181) * kMilliSecondsInADay, 5},
      {(2 * 365.0 + 182) * kMilliSecondsInADay, 6},
      {(2 * 365.0 + 212) * kMilliSecondsInADay, 6},
      {(2 * 365.0 + 213) * kMilliSecondsInADay, 7},
      {(2 * 365.0 + 243) * kMilliSecondsInADay, 7},
      {(2 * 365.0 + 244) * kMilliSecondsInADay, 8},
      {(2 * 365.0 + 273) * kMilliSecondsInADay, 8},
      {(2 * 365.0 + 274) * kMilliSecondsInADay, 9},
      {(2 * 365.0 + 304) * kMilliSecondsInADay, 9},
      {(2 * 365.0 + 305) * kMilliSecondsInADay, 10},
      {(2 * 365.0 + 334) * kMilliSecondsInADay, 10},
      {(2 * 365.0 + 335) * kMilliSecondsInADay, 11},
  };

  for (const auto& test : kTests) {
    EXPECT_EQ(test.expected_month, FX_GetMonthFromTime(test.time_ms))
        << test.time_ms;
  }
}
