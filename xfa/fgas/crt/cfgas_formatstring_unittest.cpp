// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/crt/cfgas_formatstring.h"

#include <time.h>

#include <memory>

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"

class CFGAS_FormatStringTest : public testing::Test {
 public:
  CFGAS_FormatStringTest() {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    _putenv_s("TZ", "UTC");
    _tzset();
#else
    setenv("TZ", "UTC", 1);
    tzset();
#endif
    CPDF_ModuleMgr::Get()->Init();
  }

  ~CFGAS_FormatStringTest() override { CPDF_ModuleMgr::Get()->Destroy(); }

  void TearDown() override {
    fmt_.reset();
    mgr_.reset();
  }

  // Note, this re-creates the fmt on each call. If you need to multiple
  // times store it locally.
  CFGAS_FormatString* fmt(const CFX_WideString& locale) {
    mgr_ = pdfium::MakeUnique<CXFA_LocaleMgr>(nullptr, locale);
    fmt_ = pdfium::MakeUnique<CFGAS_FormatString>(mgr_.get());
    return fmt_.get();
  }

 protected:
  std::unique_ptr<CXFA_LocaleMgr> mgr_;
  std::unique_ptr<CFGAS_FormatString> fmt_;
};

// TODO(dsinclair): Looks like the formatter/parser does not handle the various
// 'g' flags.
TEST_F(CFGAS_FormatStringTest, DateFormat) {
  struct {
    const wchar_t* locale;
    const wchar_t* input;
    const wchar_t* pattern;
    const wchar_t* output;
  } tests[] = {
      {L"en", L"2002-10-25", L"MMMM DD, YYYY", L"October 25, 2002"},
      // Note, this is in the doc as 5 but it's wrong and should be 3 by the
      // example in the Picture Clause Reference section.
      {L"en", L"20040722", L"'Week of the month is' w",
       L"Week of the month is 3"},
      {L"en", L"20040722", L"e 'days after Sunday'", L"4 days after Sunday"},
      {L"en", L"20040722", L"YYYY-'W'WW-e", L"2004-W30-4"},
      {L"en", L"20040722", L"E 'days after Saturday'",
       L"5 days after Saturday"},
      {L"en", L"2000-01-01", L"EEEE, 'the' D 'of' MMMM, YYYY",
       L"Saturday, the 1 of January, 2000"},
      {L"en", L"19991202", L"MM/D/YY", L"12/2/99"},
      {L"en", L"19990110", L"MMM D, YYYY", L"Jan 10, 1999"},
      {L"en", L"19990102", L"J", L"2"},
      {L"en", L"19990102", L"JJJ", L"002"},
      {L"en", L"19990102", L"M", L"1"},
      {L"en", L"19990102", L"MMM", L"Jan"},
      {L"en", L"19990102", L"YYYY G", L"1999 AD"},
      // Week 01 of the year is the week containing Jan 04.
      // {L"en", L"19990102", L"WW", L"00"},  -- Returns 01 incorrectly
      // {L"en", L"19990104", L"WW", L"01"},  -- Returns 02 incorrectly
      // The ?*+ should format as whitespace.
      // {L"en", L"19990104", L"YYYY?*+MM", L"1999   01"},
      // {L"en", L"1999-07-16", L"date{DD/MM/YY} '('date{MMM DD, YYYY}')'",
      //  L"16/07/99 (Jul 16, 1999)"},
      {L"de_CH", L"20041030", L"D. MMMM YYYY", L"30. Oktober 2004"},
      {L"fr_CA", L"20041030", L"D MMMM YYYY", L"30 octobre 2004"},
      {L"en", L"2002-10-25", L"date(fr){DD MMMM, YYYY}", L"25 octobre, 2002"},
      {L"en", L"2002-10-25", L"date(es){EEEE, D 'de' MMMM 'de' YYYY}",
       L"viernes, 25 de octubre de 2002"},
      // {L"en", L"2002-20-25", L"date.long(fr)()", L"25 octobre, 2002"},
      // {L"ja", L"2003-11-03", L"gY/M/D", L"H15/11/3"},
      // {L"ja", L"1989-01-08", L"ggY-M-D", L"\u5e731-1-8"},
      // {L"ja", L"1989-11-03", L"gggYY/MM/DD", L"\u5e73\u621089/11/03"},

      // Full width D == U+FF24
      // I don't actually know what ideograpic numeric value for 25 is.
      // {L"ja", L"2002-20-25", L"\uff24\uff24\uff24", L" .... "},
      // {L"ja", L"2002-20-25", L"\uff24\uff24\uff24\uff24", L" ... "},
      // Full width M == U+FF2D
      // {L"ja", L"2002-20-25", L"\uff2d\uff2d\uff2d", L" ... "},
      // {L"ja", L"2002-20-25", L"\uff2d\uff2d\uff2d\uff2d", L" ... "},
      // Full width E == U+FF25
      // {L"ja", L"2002-20-25", L"\uff25", L" ... "},
      // Full width e == U+FF45
      // {L"ja", L"2002-20-25", L"\uff45", L" ... "},
      // Full width g == U+FF47
      // {L"ja", L"1989-01-08", L"\uff47\uff47YY/MM/DD", L"\u337b89/01/08"}
      // Full width Y == U+FF39
      // {L"ja", L"2002-20-25", L"\uff39\uff39\uff39", L" ... "},
      // {L"ja", L"2002-20-25", L"\uff39\uff39\uff39\uff39\uff39", L" ... "}
  };

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    CFX_WideString result;
    EXPECT_TRUE(fmt(tests[i].locale)
                    ->FormatDateTime(tests[i].input, tests[i].pattern, result,
                                     FX_DATETIMETYPE_Date));
    EXPECT_STREQ(tests[i].output, result.c_str()) << " TEST: " << i;
  }
}

TEST_F(CFGAS_FormatStringTest, TimeFormat) {
  struct {
    const wchar_t* locale;
    const wchar_t* input;
    const wchar_t* pattern;
    const wchar_t* output;
  } tests[] = {{L"en", L"11:11:11", L"h:MM A", L"11:11 AM"},
               {L"en", L"11:11:11", L"HH:MM:SS 'o''clock' A Z",
                L"11:11:11 o'clock AM GMT"},
               {L"en", L"14:30:59", L"h:MM A", L"2:30 PM"},
               {L"en", L"14:30:59", L"HH:MM:SS A Z", L"14:30:59 PM GMT"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    CFX_WideString result;
    EXPECT_TRUE(fmt(tests[i].locale)
                    ->FormatDateTime(tests[i].input, tests[i].pattern, result,
                                     FX_DATETIMETYPE_Time));
    EXPECT_STREQ(tests[i].output, result.c_str()) << " TEST: " << i;
  }
}

TEST_F(CFGAS_FormatStringTest, DateTimeFormat) {
  struct {
    const wchar_t* locale;
    const wchar_t* input;
    const wchar_t* pattern;
    const wchar_t* output;
  } tests[] = {{L"en", L"1999-07-16T10:30Z",
                L"'At' time{HH:MM Z} 'on' date{MMM DD, YYYY}",
                L"At 10:30 GMT on Jul 16, 1999"},
               {L"en", L"1999-07-16T10:30Z",
                L"time{'At' HH:MM Z} date{'on' MMM DD, YYYY}",
                L"At 10:30 GMT on Jul 16, 1999"},
               {L"en", L"1999-07-16T10:30Z",
                L"time{'At 'HH:MM Z}date{' on 'MMM DD, YYYY}",
                L"At 10:30 GMT on Jul 16, 1999"}};

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    CFX_WideString result;
    EXPECT_TRUE(fmt(tests[i].locale)
                    ->FormatDateTime(tests[i].input, tests[i].pattern, result,
                                     FX_DATETIMETYPE_TimeDate));
    EXPECT_STREQ(tests[i].output, result.c_str()) << " TEST: " << i;
  }
}

TEST_F(CFGAS_FormatStringTest, DateParse) {
  struct {
    const wchar_t* locale;
    const wchar_t* input;
    const wchar_t* pattern;
    CFX_DateTime output;
  } tests[] = {
      {L"en", L"12/2/99", L"MM/D/YY", CFX_DateTime(1999, 12, 2, 0, 0, 0, 0)},
      {L"en", L"Jan 10, 1999", L"MMM D, YYYY",
       CFX_DateTime(1999, 1, 10, 0, 0, 0, 0)},
      {L"en", L"October 25, 2002", L"MMMM DD, YYYY",
       CFX_DateTime(2002, 10, 25, 0, 0, 0, 0)},
      {L"de_CH", L"30. Oktober 2004", L"D. MMMM YYYY",
       CFX_DateTime(2004, 10, 30, 0, 0, 0, 0)},
      {L"fr_CA", L"30 octobre 2004", L"D MMMM YYYY",
       CFX_DateTime(2004, 10, 30, 0, 0, 0, 0)},
      {L"en", L"Saturday, the 1 of January, 2000",
       L"EEEE, 'the' D 'of' MMMM, YYYY", CFX_DateTime(2000, 1, 1, 0, 0, 0, 0)},
      // {L"ja", L"H15/11/3", L"gY/M/D", CFX_DateTime(2003, 11, 3, 0, 0, 0, 0)},
      // {L"ja", L"\u5e731-1-8", L"ggY-M-D", CFX_DateTime(1989, 1, 8, 0, 0, 0,
      // 0)}, {L"ja", L"\u5e73\u621089/11/03", L"gggYY/MM/DD",
      //  CFX_DateTime(1989, 11, 3, 0, 0, 0, 0)},
      // {L"ja", L"u337b99/01/08", L"\u0067\u0067YY/MM/DD",
      //  CFX_DateTime(1999, 1, 8, 0, 0, 0, 0)}
      // Full width D == U+FF24
      // I don't actually know what ideograpic numeric value for 25 is.
      // {L"ja", L"...", L"\uff24\uff24\uff24",
      //  CFX_DateTime(2002, 20, 25, 0, 0, 0, 0)},
      // {L"ja", L"...", L"\uff24\uff24\uff24\uff24",
      //  CFX_DateTime(2002, 20, 25, 0, 0, 0, 0)},
      // Full width M == U+FF2D
      // {L"ja", L"...", L"\uff2d\uff2d\uff2d",
      //  CFX_DateTime(2002, 20, 25, 0, 0, 0, 0)},
      // {L"ja", L"...", L"\uff2d\uff2d\uff2d\uff2d",
      //  CFX_DateTime(2002, 20, 25, 0, 0, 0, 0)},
      // Full width E == U+FF25
      // {L"ja", L"...", L"\uff25", CFX_DateTime(2002, 20, 25, 0, 0, 0, 0)},
      // Full width e == U+FF45
      // {L"ja", L"...", L"\uff45", CFX_DateTime(2002, 20, 25, 0, 0, 0, 0)},
      // Full width g == U+FF47
      // {L"ja", L"1989-01-08", L"\uff47\uff47YY/MM/DD", L"\u337b89/01/08"}
      // Full width Y == U+FF39
      // {L"ja", L"...", L"\uff39\uff39\uff39",
      //  CFX_DateTime(2002, 20, 25, 0, 0, 0, 0)},
      // {L"ja", L"...", L"\uff39\uff39\uff39\uff39\uff39",
      //  CFX_DateTime(2002, 20, 25, 0, 0, 0, 0)}
  };

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    CFX_DateTime result;
    EXPECT_TRUE(fmt(tests[i].locale)
                    ->ParseDateTime(tests[i].input, tests[i].pattern,
                                    FX_DATETIMETYPE_Date, &result));
    EXPECT_EQ(tests[i].output, result) << " TEST: " << i;
  }
}

TEST_F(CFGAS_FormatStringTest, SplitFormatString) {
  std::vector<CFX_WideString> results;
  fmt(L"en")->SplitFormatString(
      L"null{'No data'} | null{} | text{999*9999} | text{999*999*9999}",
      results);
  EXPECT_EQ(4UL, results.size());

  const wchar_t* patterns[] = {L"null{'No data'} ", L" null{} ",
                               L" text{999*9999} ", L" text{999*999*9999}"};

  for (size_t i = 0; i < results.size(); ++i) {
    EXPECT_STREQ(patterns[i], results[i].c_str());
  }
}

// TODO(dsinclair): Numeric parsing fails when encountering a .
// TEST_F(CFGAS_FormatStringTest, NumParse) {
//   struct {
//     const wchar_t* locale;
//     const wchar_t* input;
//     const wchar_t* pattern;
//     const wchar_t* output;
//   } tests[] = {
//       // {L"en", L"€100.00", L"num(en_GB){$z,zz9.99}", L"100"},
//   };

//   for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
//     CFX_WideString result;
//     EXPECT_TRUE(fmt(tests[i].locale)
//                     ->ParseNum(tests[i].input, tests[i].pattern, result));
//     EXPECT_STREQ(tests[i].output, result.c_str()) << " TEST: " << i;
//   }
// }

// TODO(dsinclair) Text parsing is missing support for the global modifiers:
//  ? - wildcard
//  * - zero or more whitespace
//  + - one or more whitespace
// TEST_F(CFGAS_FormatStringTest, TextParse) {
//   struct {
//     const wchar_t* locale;
//     const wchar_t* input;
//     const wchar_t* pattern;
//     const wchar_t* output;
//   } tests[] = {
//       // {L"en", L"555-1212", L"text(th_TH){999*9999}", L"5551212"},
//   };

//   for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
//     CFX_WideString result;
//     EXPECT_TRUE(fmt(tests[i].locale)
//                     ->ParseText(tests[i].input, tests[i].pattern, result));
//     EXPECT_STREQ(tests[i].output, result.c_str()) << " TEST: " << i;
//   }
// }

TEST_F(CFGAS_FormatStringTest, NullParse) {
  struct {
    const wchar_t* locale;
    const wchar_t* input;
    const wchar_t* pattern;
  } tests[] = {
      {L"en", L"", L"null{}"}, {L"en", L"No data", L"null{'No data'}"},
  };

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    EXPECT_TRUE(
        fmt(tests[i].locale)->ParseNull(tests[i].input, tests[i].pattern))
        << " TEST: " << i;
  }
}
