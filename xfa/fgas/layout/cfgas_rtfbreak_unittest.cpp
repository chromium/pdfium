// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfgas_rtfbreak.h"

#include <memory>
#include <utility>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_gemodule.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfgas_char.h"

class CFGAS_RTFBreakTest : public testing::Test {
 public:
  void SetUp() override {
    const wchar_t kFontFamily[] = L"Arimo Bold";
    font_ = CFGAS_GEFont::LoadFont(kFontFamily, 0, FX_CodePage::kDefANSI);
    ASSERT_TRUE(font_);
  }

  std::unique_ptr<CFGAS_RTFBreak> CreateBreak(
      Mask<CFGAS_Break::LayoutStyle> layout_styles) {
    auto rtf_break = std::make_unique<CFGAS_RTFBreak>(layout_styles);
    rtf_break->SetFont(font_);
    return rtf_break;
  }

 private:
  RetainPtr<CFGAS_GEFont> font_;
};

// As soon as you get one of the control characters the break is complete
// and must be consumed before you get any more characters ....

TEST_F(CFGAS_RTFBreakTest, AddChars) {
  auto rtf_break = CreateBreak(CFGAS_Break::LayoutStyle::kExpandTab);
  WideString str(L"Input String.");
  for (wchar_t ch : str)
    EXPECT_EQ(CFGAS_Char::BreakType::kNone, rtf_break->AppendChar(ch));

  EXPECT_EQ(CFGAS_Char::BreakType::kParagraph, rtf_break->AppendChar(L'\n'));
  ASSERT_EQ(1, rtf_break->CountBreakPieces());
  EXPECT_EQ(str + L"\n", rtf_break->GetBreakPieceUnstable(0)->GetString());

  rtf_break->ClearBreakPieces();
  rtf_break->Reset();
  EXPECT_EQ(0, rtf_break->GetCurrentLineForTesting()->GetLineEnd());

  str = L"Second str.";
  for (wchar_t ch : str)
    EXPECT_EQ(CFGAS_Char::BreakType::kNone, rtf_break->AppendChar(ch));

  // Force the end of the break at the end of the string.
  rtf_break->EndBreak(CFGAS_Char::BreakType::kParagraph);
  ASSERT_EQ(1, rtf_break->CountBreakPieces());
  EXPECT_EQ(str, rtf_break->GetBreakPieceUnstable(0)->GetString());
}

TEST_F(CFGAS_RTFBreakTest, ControlCharacters) {
  auto rtf_break = CreateBreak(CFGAS_Break::LayoutStyle::kExpandTab);
  EXPECT_EQ(CFGAS_Char::BreakType::kLine, rtf_break->AppendChar(L'\v'));
  EXPECT_EQ(CFGAS_Char::BreakType::kPage, rtf_break->AppendChar(L'\f'));
  EXPECT_EQ(CFGAS_Char::BreakType::kParagraph,
            rtf_break->AppendChar(pdfium::unicode::kParagraphSeparator));
  EXPECT_EQ(CFGAS_Char::BreakType::kParagraph, rtf_break->AppendChar(L'\n'));

  ASSERT_EQ(1, rtf_break->CountBreakPieces());
  EXPECT_EQ(L"\v", rtf_break->GetBreakPieceUnstable(0)->GetString());
}

TEST_F(CFGAS_RTFBreakTest, BidiLine) {
  auto rtf_break = CreateBreak(CFGAS_Break::LayoutStyle::kExpandTab);
  rtf_break->SetLineBreakTolerance(1);
  rtf_break->SetFontSize(12);

  WideString input = WideString::FromUTF8(ByteStringView("\xa\x0\xa\xa", 4));
  for (wchar_t ch : input)
    rtf_break->AppendChar(ch);

  std::vector<CFGAS_Char> chars =
      rtf_break->GetCurrentLineForTesting()->m_LineChars;
  CFGAS_Char::BidiLine(&chars, chars.size());
  EXPECT_EQ(3u, chars.size());
}
