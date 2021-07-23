// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fgas/layout/cfgas_txtbreak.h"

#include <memory>
#include <utility>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_font.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfgas_char.h"

class CFGAS_TxtBreakTest : public testing::Test {
 public:
  void SetUp() override {
    font_ = CFGAS_GEFont::LoadFont(L"Arial Black", 0, FX_CodePage::kDefANSI);
    ASSERT_TRUE(font_);
  }

  std::unique_ptr<CFGAS_TxtBreak> CreateBreak() {
    auto txt_break = std::make_unique<CFGAS_TxtBreak>();
    txt_break->SetFont(font_);
    return txt_break;
  }

 private:
  RetainPtr<CFGAS_GEFont> font_;
};

TEST_F(CFGAS_TxtBreakTest, BidiLine) {
  auto txt_break = CreateBreak();
  txt_break->SetLineBreakTolerance(1);
  txt_break->SetFontSize(12);

  WideString input = WideString::FromUTF8(ByteStringView("\xa\x0\xa\xa", 4));
  for (wchar_t ch : input)
    txt_break->AppendChar(ch);

  std::vector<CFGAS_Char> chars =
      txt_break->GetCurrentLineForTesting()->m_LineChars;
  CFGAS_Char::BidiLine(&chars, chars.size());
  EXPECT_EQ(3u, chars.size());
}
