// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/crt/cfgas_formatstring.h"

#include <memory>

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"

class CFGAS_FormatStringTest : public testing::Test {
 public:
  CFGAS_FormatStringTest() { CPDF_ModuleMgr::Get()->Init(); }

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
      {L"de_CH", L"20041030", L"D. MMMM YYYY", L"30. Oktober 2004"},
      {L"fr_CA", L"20041030", L"D MMMM YYYY", L"30 octobre 2004"},
      // {L"ja", L"2003-11-03", L"gY/M/D", L"H15/11/3"},
      // {L"ja", L"1989-01-08", L"ggY-M-D", L"\u5e731-1-8"},
      // {L"ja", L"1989-11-03", L"gggYY/MM/DD", L"\u5e73\u621089/11/03"},
      // {L"ja", L"1989-01-08", L"\u0067\u0067YY/MM/DD", L"\u337b89/01/08"}
  };

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    CFX_WideString result;
    EXPECT_TRUE(fmt(tests[i].locale)
                    ->FormatDateTime(tests[i].input, tests[i].pattern, result,
                                     FX_DATETIMETYPE_Date));
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
  };

  for (size_t i = 0; i < FX_ArraySize(tests); ++i) {
    CFX_DateTime result;
    EXPECT_TRUE(fmt(tests[i].locale)
                    ->ParseDateTime(tests[i].input, tests[i].pattern,
                                    FX_DATETIMETYPE_Date, &result));
    EXPECT_EQ(tests[i].output, result) << " TEST: " << i;
  }
}
