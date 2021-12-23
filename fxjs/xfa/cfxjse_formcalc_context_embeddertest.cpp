// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <math.h>

#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_isolatetracker.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/scoped_set_tz.h"
#include "testing/xfa_js_embedder_test.h"
#include "third_party/base/cxx17_backports.h"
#include "xfa/fxfa/cxfa_eventparam.h"

class CFXJSE_FormCalcContextEmbedderTest : public XFAJSEmbedderTest {
 public:
  CFXJSE_FormCalcContextEmbedderTest() = default;
  ~CFXJSE_FormCalcContextEmbedderTest() override = default;

 protected:
  CFXJSE_Context* GetJseContext() {
    return GetScriptContext()->GetJseContext();
  }

  void ExecuteExpectError(ByteStringView input) {
    EXPECT_FALSE(Execute(input)) << "Program: " << input;
  }

  void ExecuteExpectNull(ByteStringView input) {
    EXPECT_TRUE(Execute(input)) << "Program: " << input;

    CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
    EXPECT_TRUE(fxv8::IsNull(GetValue())) << "Program: " << input;
  }

  void ExecuteExpectBool(ByteStringView input, bool expected) {
    EXPECT_TRUE(Execute(input)) << "Program: " << input;

    CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
    v8::Local<v8::Value> value = GetValue();

    // Yes, bools might be integers, somehow.
    EXPECT_TRUE(fxv8::IsBoolean(value) || fxv8::IsInteger(value))
        << "Program: " << input;
    EXPECT_EQ(expected, fxv8::ReentrantToBooleanHelper(isolate(), value))
        << "Program: " << input;
  }

  void ExecuteExpectInt32(ByteStringView input, int32_t expected) {
    EXPECT_TRUE(Execute(input)) << "Program: " << input;

    CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
    v8::Local<v8::Value> value = GetValue();
    EXPECT_TRUE(fxv8::IsInteger(value)) << "Program: " << input;
    EXPECT_EQ(expected, fxv8::ReentrantToInt32Helper(isolate(), value))
        << "Program: " << input;
  }

  void ExecuteExpectFloat(ByteStringView input, float expected) {
    EXPECT_TRUE(Execute(input)) << "Program: " << input;

    CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
    v8::Local<v8::Value> value = GetValue();
    EXPECT_TRUE(fxv8::IsNumber(value));
    EXPECT_FLOAT_EQ(expected, fxv8::ReentrantToFloatHelper(isolate(), value))
        << "Program: " << input;
  }

  void ExecuteExpectFloatNear(ByteStringView input, float expected) {
    constexpr float kPrecision = 0.000001f;

    EXPECT_TRUE(Execute(input)) << "Program: " << input;

    CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
    v8::Local<v8::Value> value = GetValue();
    EXPECT_TRUE(fxv8::IsNumber(value));
    EXPECT_NEAR(expected, fxv8::ReentrantToFloatHelper(isolate(), value),
                kPrecision)
        << "Program: " << input;
  }

  void ExecuteExpectNaN(ByteStringView input) {
    EXPECT_TRUE(Execute(input)) << "Program: " << input;

    CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
    v8::Local<v8::Value> value = GetValue();
    EXPECT_TRUE(fxv8::IsNumber(value));
    EXPECT_TRUE(isnan(fxv8::ReentrantToDoubleHelper(isolate(), value)));
  }

  void ExecuteExpectString(ByteStringView input, const char* expected) {
    EXPECT_TRUE(Execute(input)) << "Program: " << input;

    CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
    v8::Local<v8::Value> value = GetValue();
    EXPECT_TRUE(fxv8::IsString(value));
    EXPECT_STREQ(expected,
                 fxv8::ReentrantToByteStringHelper(isolate(), value).c_str())
        << "Program: " << input;
  }
};

// TODO(dsinclair): Comment out tests are broken and need to be fixed.

TEST_F(CFXJSE_FormCalcContextEmbedderTest, TranslateEmpty) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  const char input[] = "";
  EXPECT_TRUE(Execute(input));
  // TODO(dsinclair): This should probably throw as a blank formcalc script
  // is invalid.
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, TranslateNumber) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));
  ExecuteExpectInt32("123", 123);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Numeric) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("123 + 456", 579);
  ExecuteExpectInt32("2 - 3 * 10 / 2 + 7", -6);
  ExecuteExpectInt32("10 * 3 + 5 * 4", 50);
  ExecuteExpectInt32("(5 - \"abc\") * 3", 15);
  ExecuteExpectInt32("\"100\" / 10e1", 1);
  ExecuteExpectInt32("5 + null + 3", 8);
#if 0
  // TODO(thestig): Investigate these cases.
  ExecuteExpectInt32(
      "if (\"abc\") then\n"
      "  10\n"
      "else\n"
      "  20\n"
      "endif",
      20);
  ExecuteExpectInt32("3 / 0 + 1", 0);
#endif
  ExecuteExpectInt32("-(17)", -17);
  ExecuteExpectInt32("-(-17)", 17);
  ExecuteExpectInt32("+(17)", 17);
  ExecuteExpectInt32("+(-17)", -17);
  ExecuteExpectInt32("if (1 < 2) then\n1\nendif", 1);
  ExecuteExpectInt32(
      "if (\"abc\" > \"def\") then\n"
      "  1 and 0\n"
      "else\n"
      "  0\n"
      "endif",
      0);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Strings) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("\"abc\"", "abc");
  ExecuteExpectString(
      "concat(\"The total is \", 2, \" dollars and \", 57, \" cents.\")",
      "The total is 2 dollars and 57 cents.");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Booleans) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectBool("0 and 1 or 2 > 1", true);
  ExecuteExpectBool("2 < 3 not 1 == 1", false);
  ExecuteExpectBool("\"abc\" | 2", true);
  ExecuteExpectBool("1 or 0", true);
  ExecuteExpectBool("0 | 0", false);
  ExecuteExpectBool("0 or 1 | 0 or 0", true);
  ExecuteExpectBool("1 and 0", false);
  ExecuteExpectBool("0 and 1 & 0 and 0", false);
  ExecuteExpectBool("not(\"true\")", true);
  ExecuteExpectBool("not(1)", false);
  ExecuteExpectBool("3 == 3", true);
  ExecuteExpectBool("3 <> 4", true);
  ExecuteExpectBool("\"abc\" eq \"def\"", false);
  ExecuteExpectBool("\"def\" ne \"abc\"", true);
  ExecuteExpectBool("5 + 5 == 10", true);
  ExecuteExpectBool("5 + 5 <> \"10\"", false);
  ExecuteExpectBool("3 < 3", false);
  ExecuteExpectBool("3 > 4", false);
  ExecuteExpectBool("\"abc\" <= \"def\"", true);
  ExecuteExpectBool("\"def\" > \"abc\"", true);
  ExecuteExpectBool("12 >= 12", true);
  ExecuteExpectBool("\"true\" < \"false\"", false);
#if 0
  // TODO(thestig): Investigate this case.
  // Confirm with Reader.
  ExecuteExpectBool("0 & 0", true);
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Abs) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("Abs(1.03)", 1.03f);
  ExecuteExpectFloat("Abs(-1.03)", 1.03f);
  ExecuteExpectFloat("Abs(0)", 0.0f);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Avg) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("Avg(0, 32, 16)", 16.0f);
  ExecuteExpectFloat("Avg(2.5, 17, null)", 9.75f);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Ceil) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("Ceil(2.5875)", 3);
  ExecuteExpectInt32("Ceil(-5.9)", -5);
  ExecuteExpectInt32("Ceil(\"abc\")", 0);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Count) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("Count(\"Tony\", \"Blue\", 41)", 3);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Floor) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("Floor(21.3409873)", 21);
  ExecuteExpectInt32("Floor(5.999965342)", 5);
  ExecuteExpectInt32("Floor(3.2 * 15)", 48);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Max) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("Max(234, 15, 107)", 234);
  ExecuteExpectInt32("Max(\"abc\", 15, \"Tony Blue\")", 15);
  ExecuteExpectInt32("Max(\"abc\")", 0);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Min) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("Min(234, 15, 107)", 15);
#if 0
  // TODO(thestig): Investigate these cases.
  // Verify with Reader; This should have a return value of 0.
  ExecuteExpectInt32("Min(\"abc\", 15, \"Tony Blue\")", 15);
#endif
  ExecuteExpectInt32("Min(\"abc\")", 0);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Mod) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("Mod(64, -3)", 1);
  ExecuteExpectInt32("Mod(-13, 3)", -1);
  ExecuteExpectInt32("Mod(\"abc\", 2)", 0);

  ExecuteExpectNaN("Mod(10, NaN)");
  ExecuteExpectNaN("Mod(10, Infinity)");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Round) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("Round(12.389764537, 4)", 12.3898f);
  ExecuteExpectFloat("Round(20/3, 2)", 6.67f);
  ExecuteExpectFloat("Round(8.9897, \"abc\")", 9.0f);
  ExecuteExpectFloat("Round(FV(400, 0.10/12, 30*12), 2)", 904195.17f);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Sum) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("Sum(2, 4, 6, 8)", 20);
  ExecuteExpectInt32("Sum(-2, 4, -6, 8)", 4);
  ExecuteExpectInt32("Sum(4, 16, \"abc\", 19)", 39);
}

// TEST_F(CFXJSE_FormCalcContextEmbedderTest, DISABLED_Date) {
//   ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));
//
//   TODO(dsinclair): Make compatible with windows.
//   time_t seconds = time(nullptr);
//   int days = seconds / (60 * 60 * 24);

//   EXPECT_TRUE(Execute("Date()"));

//   v8::Local<v8::Value> value = GetValue();
//   EXPECT_TRUE(value->IsNumber());
//   EXPECT_EQ(days, value->ToInteger());
// }

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Date2Num) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("Date2Num(\"1/1/1900\", \"D/M/YYYY\")", 1);
  ExecuteExpectInt32("Date2Num(\"03/15/96\", \"MM/DD/YY\")", 35138);
  ExecuteExpectInt32("Date2Num(\"96-08-20\", \"YY-MM-DD\", \"fr_FR\")", 35296);
  ExecuteExpectInt32(
      "Date2Num(\"1/3/00\", \"D/M/YY\") - Date2Num(\"1/2/00\", \"D/M/YY\")",
      29);
#if 0
  // TODO(thestig): Investigate these cases.
  ExecuteExpectInt32("Date2Num(\"Mar 15, 1996\")", 35138);
  ExecuteExpectInt32("Date2Num(\"Aug 1, 1996\", \"MMM D, YYYY\")", 35277);
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, DateFmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("DateFmt(3, \"de_DE\")", "D. MMMM YYYY");
#if 0
  // TODO(thestig): Investigate these cases.
  ExecuteExpectString("DateFmt(1)", "M/D/YY");
  ExecuteExpectString("DateFmt(2, \"fr_CA\")", "YY-MM-DD");
  ExecuteExpectString("DateFmt(4, \"fr_FR\")", "EEE D' MMMM YYYY");
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, IsoDate2Num) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("IsoDate2Num(\"1900\")", 1);
  ExecuteExpectInt32("IsoDate2Num(\"1900-01\")", 1);
  ExecuteExpectInt32("IsoDate2Num(\"1900-01-01\")", 1);
  ExecuteExpectInt32("IsoDate2Num(\"19960315T20:20:20\")", 35138);
  ExecuteExpectInt32("IsoDate2Num(\"2000-03-01\") - IsoDate2Num(\"20000201\")",
                     29);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, DISABLED_IsoTime2Num) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("IsoTime2Num(\"00:00:00Z\")", 1);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, LocalDateFmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("LocalDateFmt(3, \"de_CH\")", "t. MMMM jjjj");
  ExecuteExpectString("LocalDateFmt(4, \"fr_FR\")", "EEEE j MMMM aaaa");
#if 0
  // TODO(thestig): Investigate these cases.
  ExecuteExpectString("LocalDateFmt(1, \"de_DE\")", "tt.MM.uu");
  ExecuteExpectString("LocalDateFmt(2, \"fr_CA\")", "aa-MM-jj");
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, DISABLED_LocalTimeFmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("LocalTimeFmt(1, \"de_DE\")", "HH:mm");
  ExecuteExpectString("LocalTimeFmt(2, \"fr_CA\")", "HH:mm::ss");
  ExecuteExpectString("LocalTimeFmt(3, \"de_CH\")", "HH:mm:ss z");
  ExecuteExpectString("LocalTimeFmt(4, \"fr_FR\")", "HH' h 'mm z");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Num2Date) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Num2Date(1, \"DD/MM/YYYY\")", "01/01/1900");
  ExecuteExpectString("Num2Date(35139, \"DD-MMM-YYYY\", \"de_DE\")",
                      "16-Mrz-1996");
#if 0
  // TODO(thestig): Investigate this case.
  ExecuteExpectString(
      "Num2Date(Date2Num(\"Mar 15, 2000\") - Date2Num(\"98-03-15\", "
      "\"YY-MM-DD\", \"fr_CA\"))",
      "Jan 1, 1902");
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, DISABLED_Num2GMTime) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  // Broken on Windows only.
  ExecuteExpectString("Num2GMTime(1, \"HH:MM:SS\")", "00:00:00");
  // Below broken on other platforms.
  ExecuteExpectString("Num2GMTime(65593001, \"HH:MM:SS Z\")", "18:13:13 GMT");
  ExecuteExpectString("Num2GMTime(43993001, TimeFmt(4, \"de_DE\"), \"de_DE\")",
                      "12.13 Uhr GMT");
}

// TODO(dsinclair): Broken on Mac ...
TEST_F(CFXJSE_FormCalcContextEmbedderTest, DISABLED_Num2Time) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Num2Time(1, \"HH:MM:SS\")", "00:00:00");
}

// TEST_F(CFXJSE_FormCalcContextEmbedderTest, DISABLED_Time) {
//   ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));
//   TODO(dsinclair): Make compatible with windows.
//   struct timeval tp;
//   gettimeofday(&tp, nullptr);

//   EXPECT_TRUE(Execute("Time()"));

//   v8::Local<v8::Value> value = GetValue();
//   EXPECT_TRUE(value->IsInteger());
//   EXPECT_EQ(tp.tv_sec * 1000L + tp.tv_usec / 1000, value->ToInteger())
//       << "Program: Time()";
// }

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Time2Num) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("Time2Num(\"00:00:00 GMT\", \"HH:MM:SS Z\")", 1);
  ExecuteExpectInt32("Time2Num(\"00:00:01 GMT\", \"HH:MM:SS Z\")", 1001);
  ExecuteExpectInt32("Time2Num(\"00:01:00 GMT\", \"HH:MM:SS Z\")", 60001);
  ExecuteExpectInt32("Time2Num(\"01:00:00 GMT\", \"HH:MM:SS Z\")", 3600001);
  ExecuteExpectInt32("Time2Num(\"23:59:59 GMT\", \"HH:MM:SS Z\")", 86399001);
  // https://crbug.com/pdfium/1257
  ExecuteExpectInt32("Time2Num(\"\", \"\", 1)", 0);
  ExecuteExpectInt32("Time2Num(\"13:13:13 GMT\", \"HH:MM:SS Z\", \"fr_FR\")",
                     47593001);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Time2NumWithTZ) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  static constexpr const char* kTimeZones[] = {
      "UTC+14",   "UTC-14",   "UTC+9:30", "UTC-0:30",
      "UTC+0:30", "UTC-0:01", "UTC+0:01"};
  for (const char* tz : kTimeZones) {
    ScopedSetTZ scoped_set_tz(tz);
    ExecuteExpectInt32("Time2Num(\"00:00:00 GMT\", \"HH:MM:SS Z\")", 1);
    ExecuteExpectInt32("Time2Num(\"11:59:59 GMT\", \"HH:MM:SS Z\")", 43199001);
    ExecuteExpectInt32("Time2Num(\"12:00:00 GMT\", \"HH:MM:SS Z\")", 43200001);
    ExecuteExpectInt32("Time2Num(\"23:59:59 GMT\", \"HH:MM:SS Z\")", 86399001);
  }
  {
    ScopedSetTZ scoped_set_tz("UTC-3:00");
    ExecuteExpectInt32("Time2Num(\"1:13:13 PM\")", 36793001);
    ExecuteExpectInt32(
        "Time2Num(\"13:13:13 GMT\", \"HH:MM:SS Z\") - "
        "Time2Num(\"13:13:13\", \"HH:MM:SS\")",
        10800000);
  }
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, TimeFmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("TimeFmt(2, \"fr_CA\")", "HH:MM:SS");
  ExecuteExpectString("TimeFmt(3, \"fr_FR\")", "HH:MM:SS Z");
#if 0
  // TODO(thestig): Investigate these cases.
  ExecuteExpectString("TimeFmt(1)", "h::MM A");
  ExecuteExpectString("TimeFmt(4, \"de_DE\")", "H.MM' Uhr 'Z");
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Apr) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloatNear("Apr(35000, 269.50, 360)", 0.08515404566f);
  ExecuteExpectFloatNear("Apr(210000 * 0.75, 850 + 110, 25 * 26)",
                         0.07161332404f);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, CTerm) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("CTerm(0.10, 500000, 12000)", 39.13224648502f);
#if 0
  // TODO(thestig): Investigate these cases.
  ExecuteExpectFloat("CTerm(0.02, 1000, 100)", 116.2767474515f);
  ExecuteExpectFloat("CTerm(0.0275 + 0.0025, 1000000, 55000 * 0.10)",
                     176.02226044975f);
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, FV) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("FV(400, 0.10 / 12, 30 * 12)", 904195.16991842445f);
  ExecuteExpectFloat("FV(1000, 0.075 / 4, 10 * 4)", 58791.96145535981f);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, IPmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("IPmt(30000, 0.085, 295.50, 7, 3)", 624.8839283142f);
  ExecuteExpectFloat("IPmt(160000, 0.0475, 980, 24, 12)", 7103.80833569485f);
  ExecuteExpectFloat("IPmt(15000, 0.065, 65.50, 15, 1)", 0.0f);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, NPV) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("NPV(0.065, 5000)", 4694.83568075117f);
  ExecuteExpectFloat("NPV(0.10, 500, 1500, 4000, 10000)", 11529.60863329007f);
  ExecuteExpectFloat("NPV(0.0275 / 12, 50, 60, 40, 100, 25)", 273.14193838457f);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Pmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("Pmt(25000, 0.085, 12)", 3403.82145169876f);
#if 0
  // TODO(thestig): Investigate this case.
  ExecuteExpectFloat("Pmt(150000, 0.0475 / 12, 25 * 12)", 855.17604207164f);
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, PPmt) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("PPmt(30000, 0.085, 295.50, 7, 3)", 261.6160716858f);
  ExecuteExpectFloat("PPmt(160000, 0.0475, 980, 24, 12)", 4656.19166430515f);
#if 0
  // TODO(thestig): Investigate this case.
  ExecuteExpectFloat("PPmt(15000, 0.065, 65.50, 15, 1)", 0.0f);
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, PV) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("PV(400, 0.10 / 12, 30 * 12)", 45580.32799074439f);
#if 0
  // TODO(thestig): Investigate this case.
  ExecuteExpectFloat("PV(1000, 0.075 / 4, 10 * 4)", 58791.96145535981f);
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Rate) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloatNear("Rate(12000, 8000, 5)", 0.0844717712f);
  ExecuteExpectFloatNear("Rate(10000, 0.25 * 5000, 4 * 12)", 0.04427378243f);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Term) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("Term(2500, 0.0275 + 0.0025, 5000)", 1.97128786369f);
#if 0
  // TODO(thestig): Investigate this case.
  ExecuteExpectFloat("Term(475, .05, 1500)", 3.00477517728f);
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Choose) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Choose(3, \"Taxes\", \"Price\", \"Person\", \"Teller\")",
                      "Person");
  ExecuteExpectString("Choose(2, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)", "9");
  ExecuteExpectString(
      "Choose(20/3, \"A\", \"B\", \"C\", \"D\", \"E\", \"F\", \"G\", \"H\")",
      "F");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Exists) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));
  ExecuteExpectBool("Exists(\"hello world\")", false);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, HasValue) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectBool("HasValue(2)", true);
  ExecuteExpectBool("HasValue(\" \")", false);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Oneof) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectBool("Oneof(3, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)", true);
  ExecuteExpectBool(
      "Oneof(\"John\", \"Bill\", \"Gary\", \"Joan\", \"John\", \"Lisa\")",
      true);
  ExecuteExpectBool("Oneof(3, 1, 25)", false);
  ExecuteExpectBool("Oneof(3, 3, null)", true);
  ExecuteExpectBool("Oneof(3, null, null)", false);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Within) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectBool("Within(\"C\", \"A\", \"D\")", true);
  ExecuteExpectBool("Within(1.5, 0, 2)", true);
  ExecuteExpectBool("Within(-1, 0, 2)", false);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Eval) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("eval(\"10*3+5*4\")", 50);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, DISABLED_Null) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Null()", "null");
  ExecuteExpectString("Concat(\"ABC\", Null(), \"DEF\")", "ABCDEF");
  ExecuteExpectInt32("Null() + 5", 5);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Ref) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Ref(\"10*3+5*4\")", "10*3+5*4");
  ExecuteExpectString("Ref(\"hello\")", "hello");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, UnitType) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("UnitType(\"36 in\")", "in");
  ExecuteExpectString("UnitType(\"2.54centimeters\")", "cm");
  ExecuteExpectString("UnitType(\"picas\")", "pt");
  ExecuteExpectString("UnitType(\"2.cm\")", "cm");
  ExecuteExpectString("UnitType(\"2.zero cm\")", "in");
  ExecuteExpectString("UnitType(\"kilometers\")", "in");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, UnitValue) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectFloat("UnitValue(\"2in\")", 2.0f);
  ExecuteExpectFloat("UnitValue(\"2in\", \"cm\")", 5.08f);
#if 0
  // TODO(thestig): Investigate these cases.
  // Should the UnitType cases move into the UnitType test case?
  ExecuteExpectFloat("UnitValue(\"6\", \"pt\")", 432f);
  ExecuteExpectFloat("UnitType(\"A\", \"cm\")", 0.0f);
  ExecuteExpectFloat("UnitType(\"5.08cm\", \"kilograms\")", 2.0f);
#endif
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, At) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectInt32("At(\"ABCDEFGH\", \"AB\")", 1);
  ExecuteExpectInt32("At(\"ABCDEFGH\", \"F\")", 6);
  ExecuteExpectInt32("At(23412931298471, 29)", 5);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Concat) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Concat(\"ABC\", \"DEF\")", "ABCDEF");
  ExecuteExpectString("Concat(\"Tony\", Space(1), \"Blue\")", "Tony Blue");
  ExecuteExpectString("Concat(\"You owe \", WordNum(1154.67, 2), \".\")",
                      "You owe One Thousand One Hundred Fifty-four Dollars And "
                      "Sixty-seven Cents.");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Decode) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  // HTML
  ExecuteExpectString(R"(Decode("", "html"))", "");
  ExecuteExpectString(R"(Decode("abc&Acirc;xyz", "html"))", "abc\xC3\x82xyz");
  ExecuteExpectString(R"(Decode("abc&NoneSuchButVeryLongIndeed;", "html"))",
                      "abc");
  ExecuteExpectString(R"(Decode("&#x0041;&AElig;&Aacute;", "html"))",
                      "A\xC3\x86\xC3\x81");
  ExecuteExpectString(R"(Decode("xyz&#", "html"))", "xyz");
  ExecuteExpectString(R"(Decode("|&zzzzzz;|", "html"))", "||");

  // XML
  ExecuteExpectString(R"(Decode("", "xml"))", "");
  ExecuteExpectString(R"(Decode("~!@#$%%^&amp;*()_+|`", "xml"))",
                      "~!@#$%%^&*()_+|`");
  ExecuteExpectString(R"(Decode("abc&nonesuchbutverylongindeed;", "xml"))",
                      "abc");
  ExecuteExpectString(R"(Decode("&quot;&#x45;&lt;&gt;[].&apos;", "xml"))",
                      "\"E<>[].'");
  ExecuteExpectString(R"(Decode("xyz&#", "xml"))", "xyz");
  ExecuteExpectString(R"(Decode("|&zzzzzz;|", "xml"))", "||");

  // URL
  ExecuteExpectString(R"(Decode("", "url"))", "");
  ExecuteExpectString(R"(Decode("~%26^&*()_+|`{", "url"))", "~&^&*()_+|`{");
  ExecuteExpectString(R"(Decode("~%26^&*()_+|`{", "mbogo"))", "~&^&*()_+|`{");
  ExecuteExpectString(R"(Decode("~%26^&*()_+|`{"))", "~&^&*()_+|`{");
  ExecuteExpectString(R"(Decode("~%~~"))", "");
  ExecuteExpectString(R"(Decode("?%~"))", "");
  ExecuteExpectString(R"(Decode("?%"))", "?");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Encode) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Encode(\"X/~&^*<=>?|\")",
                      "X%2f%7e%26%5e*%3c%3d%3e%3f%7c");
  ExecuteExpectString("Encode(\"X/~&^*<=>?|\", \"mbogo\")",
                      "X%2f%7e%26%5e*%3c%3d%3e%3f%7c");
  ExecuteExpectString("Encode(\"X/~&^*<=>?|\", \"url\")",
                      "X%2f%7e%26%5e*%3c%3d%3e%3f%7c");
  ExecuteExpectString("Encode(\"X/~&^*<=>?|\", \"xml\")",
                      "X/~&amp;^*&lt;=&gt;?|");
  ExecuteExpectString("Encode(\"X/~&^*<=>?|\", \"html\")",
                      "X/~&amp;^*&lt;=&gt;?|");

  ExecuteExpectString("Encode(\"\\u0022\\u00f5\\ufed0\", \"url\")",
                      "%22%f5%fe%d0");
  ExecuteExpectString("Encode(\"\\u0022\\u00f4\\ufed0\", \"xml\")",
                      "&quot;&#xf4;&#xfed0;");
  ExecuteExpectString("Encode(\"\\u0022\\u00f5\\ufed0\", \"html\")",
                      "&quot;&otilde;&#xfed0;");

#if !defined(OS_WIN)
  // Windows wchar_t isn't wide enough to handle these anyways.
  // TODO(tsepez): fix surrogate encodings.
  ExecuteExpectString("Encode(\"\\uD83D\\uDCA9\", \"url\")", "%01%f4%a9");
  ExecuteExpectString("Encode(\"\\uD83D\\uDCA9\", \"xml\")", "");
  ExecuteExpectString("Encode(\"\\uD83D\\uDCA9\", \"html\")", "");
#endif  // !defined(OS_WIN)
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, DISABLED_Format) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Format(\"MMM D, YYYY\", \"20020901\")", "Sep 1, 2002");
  ExecuteExpectString("Format(\"$9,999,999.99\", 1234567.89)", "$1,234,567.89");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Left) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Left(\"ABCDEFGH\", 3)", "ABC");
  ExecuteExpectString("Left(\"Tony Blue\", 5)", "Tony ");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Len) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectBool("Len(\"ABCDEFGH\")", 8);
  ExecuteExpectBool("Len(4)", 1);
  ExecuteExpectBool("Len(Str(4.532, 6, 4))", 6);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Lower) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Lower(\"ABC\")", "abc");
  ExecuteExpectString("Lower(\"21 Main St.\")", "21 main st.");
  ExecuteExpectString("Lower(15)", "15");
}

// This is testing for an OOB read, so will likely only fail under ASAN.
TEST_F(CFXJSE_FormCalcContextEmbedderTest, bug_854623) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  const uint8_t test_string[] = {
      0x4c, 0x6f, 0x77, 0x65, 0x72, 0x28, 0x22, 0xc3,
      0x85, 0xc3, 0x85, 0xc3, 0x85, 0x22, 0x29};  // Lower("ÅÅÅ")
  Execute(ByteString(test_string, sizeof(test_string)).AsStringView());
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Ltrim) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Ltrim(\"   ABCD\")", "ABCD");
  ExecuteExpectString("Ltrim(Rtrim(\"    Tony Blue    \"))", "Tony Blue");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, DISABLED_Parse) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Parse(\"MMM D, YYYY\", \"Sep 1, 2002\")", "2002-09-01");
  ExecuteExpectFloat("Parse(\"$9,999,999.99\", \"$1,234,567.89\")",
                     1234567.89f);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Replace) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Replace(\"Tony Blue\", \"Tony\", \"Chris\")",
                      "Chris Blue");
  ExecuteExpectString("Replace(\"ABCDEFGH\", \"D\")", "ABCEFGH");
  ExecuteExpectString("Replace(\"ABCDEFGH\", \"d\")", "ABCDEFGH");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Right) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Right(\"ABCDEFGH\", 3)", "FGH");
  ExecuteExpectString("Right(\"Tony Blue\", 5)", " Blue");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Rtrim) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Rtrim(\"ABCD   \")", "ABCD");
  ExecuteExpectString("Rtrim(\"Tony Blue      \t\")", "Tony Blue");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Space) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Space(5)", "     ");
  ExecuteExpectString("Concat(\"Tony\", Space(1), \"Blue\")", "Tony Blue");

  // Error cases.
  ExecuteExpectError("Space(15654909)");
  ExecuteExpectError("Space(99999999)");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Str) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Str(2.456)", "         2");
  ExecuteExpectString("Str(4.532, 6, 4)", "4.5320");
  ExecuteExpectString("Str(234.458, 4)", " 234");
  ExecuteExpectString("Str(31.2345, 4, 2)", "****");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Stuff) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Stuff(\"TonyBlue\", 5, 0, \" \")", "Tony Blue");
  ExecuteExpectString("Stuff(\"ABCDEFGH\", 4, 2)", "ABCFGH");
  ExecuteExpectString("Stuff(\"members-list@myweb.com\", 0, 0, \"cc:\")",
                      "cc:members-list@myweb.com");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Substr) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  // Test wrong number of parameters.
  ExecuteExpectError("Substr()");
  ExecuteExpectError("Substr(1)");
  ExecuteExpectError("Substr(1, 2)");
  ExecuteExpectError("Substr(1, 2, 3, 4)");

  // Test null input.
  ExecuteExpectNull("Substr(null, 0, 4)");
  ExecuteExpectNull("Substr(\"ABCDEFG\", null, 4)");
  ExecuteExpectNull("Substr(\"ABCDEFG\", 0, null)");
  ExecuteExpectNull("Substr(null, null, 4)");
  ExecuteExpectNull("Substr(null, 0, null)");
  ExecuteExpectNull("Substr(\"ABCDEFG\", null, null)");
  ExecuteExpectNull("Substr(null, null, null)");

  ExecuteExpectString("Substr(\"ABCDEFG\", -1, 4)", "ABCD");
  ExecuteExpectString("Substr(\"ABCDEFG\", 0, 4)", "ABCD");
  ExecuteExpectString("Substr(\"ABCDEFG\", 3, 4)", "CDEF");
  ExecuteExpectString("Substr(\"ABCDEFG\", 4, 4)", "DEFG");
  ExecuteExpectString("Substr(\"ABCDEFG\", 5, 4)", "EFG");
  ExecuteExpectString("Substr(\"ABCDEFG\", 6, 4)", "FG");
  ExecuteExpectString("Substr(\"ABCDEFG\", 7, 4)", "G");
  ExecuteExpectString("Substr(\"ABCDEFG\", 8, 4)", "");
  ExecuteExpectString("Substr(\"ABCDEFG\", 5, -1)", "");
  ExecuteExpectString("Substr(\"ABCDEFG\", 5, 0)", "");
  ExecuteExpectString("Substr(\"ABCDEFG\", 5, 1)", "E");
  ExecuteExpectString("Substr(\"abcdefghi\", 5, 3)", "efg");
  ExecuteExpectString("Substr(3214, 2, 1)", "2");
  ExecuteExpectString("Substr(\"21 Waterloo St.\", 4, 5)", "Water");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Uuid) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));
  EXPECT_TRUE(Execute("Uuid()"));

  CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
  v8::Local<v8::Value> value = GetValue();
  EXPECT_TRUE(fxv8::IsString(value));
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Upper) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  ExecuteExpectString("Upper(\"abc\")", "ABC");
  ExecuteExpectString("Upper(\"21 Main St.\")", "21 MAIN ST.");
  ExecuteExpectString("Upper(15)", "15");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, WordNum) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

#if 0
  // TODO(thestig): Investigate these cases.
  // This looks like it's wrong in the Formcalc document.
  ExecuteExpectString("WordNum(123.45)", "One Hundred and Twenty-three");
  ExecuteExpectString("WordNum(123.45, 1)", "One Hundred and Twenty-three Dollars");
#endif
  ExecuteExpectString(
      "WordNum(1154.67, 2)",
      "One Thousand One Hundred Fifty-four Dollars And Sixty-seven Cents");
  ExecuteExpectString("WordNum(43, 2)", "Forty-three Dollars And Zero Cents");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Get) {
  // TODO(dsinclair): Is this supported?
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Post) {
  // TODO(dsinclair): Is this supported?
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, Put) {
  // TODO(dsinclair): Is this supported?
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, InvalidFunctions) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  EXPECT_FALSE(ExecuteSilenceFailure("F()"));
  EXPECT_FALSE(ExecuteSilenceFailure("()"));
  EXPECT_FALSE(ExecuteSilenceFailure("()()()"));
  EXPECT_FALSE(ExecuteSilenceFailure("Round(2.0)()"));
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, MethodCall) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  const char test[] = {"$form.form1.TextField11.getAttribute(\"h\")"};
  ExecuteExpectString(test, "12.7mm");
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, GetXFAEventChange) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  CXFA_EventParam params;
  params.m_wsChange = L"changed";

  CFXJSE_Engine* context = GetScriptContext();
  context->SetEventParam(&params);

  const char test[] = {"xfa.event.change"};
  ExecuteExpectString(test, "changed");
  context->SetEventParam(nullptr);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, SetXFAEventChange) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  CXFA_EventParam params;
  CFXJSE_Engine* context = GetScriptContext();
  context->SetEventParam(&params);

  const char test[] = {"xfa.event.change = \"changed\""};
  EXPECT_TRUE(Execute(test));
  EXPECT_EQ(L"changed", params.m_wsChange);
  context->SetEventParam(nullptr);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, SetXFAEventFullTextFails) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  CXFA_EventParam params;
  params.m_wsFullText = L"Original Full Text";

  CFXJSE_Engine* context = GetScriptContext();
  context->SetEventParam(&params);

  const char test[] = {"xfa.event.fullText = \"Changed Full Text\""};
  EXPECT_TRUE(Execute(test));
  EXPECT_EQ(L"Original Full Text", params.m_wsFullText);
  context->SetEventParam(nullptr);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, EventChangeSelection) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  CXFA_EventParam params;
  params.m_wsPrevText = L"1234";
  params.m_iSelStart = 1;
  params.m_iSelEnd = 3;

  CFXJSE_Engine* context = GetScriptContext();
  context->SetEventParam(&params);

  // Moving end to start works fine.
  EXPECT_TRUE(Execute("xfa.event.selEnd = \"1\""));
  EXPECT_EQ(1, params.m_iSelStart);
  EXPECT_EQ(1, params.m_iSelEnd);

  // Moving end before end, forces start to move in response.
  EXPECT_TRUE(Execute("xfa.event.selEnd = \"0\""));
  EXPECT_EQ(0, params.m_iSelStart);
  EXPECT_EQ(0, params.m_iSelEnd);

  // Negatives not allowed
  EXPECT_TRUE(Execute("xfa.event.selEnd = \"-1\""));
  EXPECT_EQ(0, params.m_iSelStart);
  EXPECT_EQ(0, params.m_iSelEnd);

  // Negatives not allowed
  EXPECT_TRUE(Execute("xfa.event.selStart = \"-1\""));
  EXPECT_EQ(0, params.m_iSelStart);
  EXPECT_EQ(0, params.m_iSelEnd);

  params.m_iSelEnd = 1;

  // Moving start to end works fine.
  EXPECT_TRUE(Execute("xfa.event.selStart = \"1\""));
  EXPECT_EQ(1, params.m_iSelStart);
  EXPECT_EQ(1, params.m_iSelEnd);

  // Moving start after end moves end.
  EXPECT_TRUE(Execute("xfa.event.selStart = \"2\""));
  EXPECT_EQ(2, params.m_iSelStart);
  EXPECT_EQ(2, params.m_iSelEnd);

  // Setting End past end of string clamps to string length;
  EXPECT_TRUE(Execute("xfa.event.selEnd = \"20\""));
  EXPECT_EQ(2, params.m_iSelStart);
  EXPECT_EQ(4, params.m_iSelEnd);

  // Setting Start past end of string clamps to string length;
  EXPECT_TRUE(Execute("xfa.event.selStart = \"20\""));
  EXPECT_EQ(4, params.m_iSelStart);
  EXPECT_EQ(4, params.m_iSelEnd);

  context->SetEventParam(nullptr);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, XFAEventCancelAction) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  CXFA_EventParam params;
  params.m_bCancelAction = false;

  CFXJSE_Engine* context = GetScriptContext();
  context->SetEventParam(&params);
  ExecuteExpectBool("xfa.event.cancelAction", false);
  EXPECT_TRUE(Execute("xfa.event.cancelAction = \"true\""));
  EXPECT_TRUE(params.m_bCancelAction);
  context->SetEventParam(nullptr);
}

TEST_F(CFXJSE_FormCalcContextEmbedderTest, ComplexTextChangeEvent) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));

  CXFA_EventParam params;
  params.m_wsChange = L"g";
  params.m_wsPrevText = L"abcd";
  params.m_iSelStart = 1;
  params.m_iSelEnd = 3;

  CFXJSE_Engine* context = GetScriptContext();
  context->SetEventParam(&params);

  EXPECT_EQ(L"abcd", params.m_wsPrevText);
  EXPECT_EQ(L"agd", params.GetNewText());
  EXPECT_EQ(L"g", params.m_wsChange);
  EXPECT_EQ(1, params.m_iSelStart);
  EXPECT_EQ(3, params.m_iSelEnd);

  const char change_event[] = {"xfa.event.change = \"xyz\""};
  EXPECT_TRUE(Execute(change_event));

  EXPECT_EQ(L"abcd", params.m_wsPrevText);
  EXPECT_EQ(L"xyz", params.m_wsChange);
  EXPECT_EQ(L"axyzd", params.GetNewText());
  EXPECT_EQ(1, params.m_iSelStart);
  EXPECT_EQ(3, params.m_iSelEnd);

  const char sel_event[] = {"xfa.event.selEnd = \"1\""};
  EXPECT_TRUE(Execute(sel_event));

  EXPECT_EQ(L"abcd", params.m_wsPrevText);
  EXPECT_EQ(L"xyz", params.m_wsChange);
  EXPECT_EQ(L"axyzbcd", params.GetNewText());
  EXPECT_EQ(1, params.m_iSelStart);
  EXPECT_EQ(1, params.m_iSelEnd);

  context->SetEventParam(nullptr);
}

// Should not crash.
TEST_F(CFXJSE_FormCalcContextEmbedderTest, BUG_1223) {
  ASSERT_TRUE(OpenDocument("simple_xfa.pdf"));
  EXPECT_TRUE(Execute("!.somExpression=0"));
}
