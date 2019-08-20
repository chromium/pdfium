// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/layout/cfx_rtfbreak.h"

#include <memory>
#include <utility>

#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_gemodule.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_unit_test_support.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/layout/cfx_char.h"

class CFX_RTFBreakTest : public testing::Test {
 public:
  void SetUp() override {
    font_ =
        CFGAS_GEFont::LoadFont(L"Arial Black", 0, 0, GetGlobalFontManager());
    ASSERT_TRUE(font_.Get());
  }

  std::unique_ptr<CFX_RTFBreak> CreateBreak(uint32_t layout_styles) {
    auto rtf_break = pdfium::MakeUnique<CFX_RTFBreak>(layout_styles);
    rtf_break->SetFont(font_);
    return rtf_break;
  }

 private:
  RetainPtr<CFGAS_GEFont> font_;
};

// As soon as you get one of the control characters the break is complete
// and must be consumed before you get any more characters ....

TEST_F(CFX_RTFBreakTest, AddChars) {
  auto rtf_break = CreateBreak(FX_LAYOUTSTYLE_ExpandTab);

  WideString str(L"Input String.");
  for (wchar_t ch : str)
    EXPECT_EQ(CFX_BreakType::None, rtf_break->AppendChar(ch));

  EXPECT_EQ(CFX_BreakType::Paragraph, rtf_break->AppendChar(L'\n'));
  ASSERT_EQ(1, rtf_break->CountBreakPieces());
  EXPECT_EQ(str + L"\n", rtf_break->GetBreakPieceUnstable(0)->GetString());

  rtf_break->ClearBreakPieces();
  rtf_break->Reset();
  EXPECT_EQ(0, rtf_break->GetCurrentLineForTesting()->GetLineEnd());

  str = L"Second str.";
  for (wchar_t ch : str)
    EXPECT_EQ(CFX_BreakType::None, rtf_break->AppendChar(ch));

  // Force the end of the break at the end of the string.
  rtf_break->EndBreak(CFX_BreakType::Paragraph);
  ASSERT_EQ(1, rtf_break->CountBreakPieces());
  EXPECT_EQ(str, rtf_break->GetBreakPieceUnstable(0)->GetString());
}

TEST_F(CFX_RTFBreakTest, ControlCharacters) {
  auto rtf_break = CreateBreak(FX_LAYOUTSTYLE_ExpandTab);
  EXPECT_EQ(CFX_BreakType::Line, rtf_break->AppendChar(L'\v'));
  EXPECT_EQ(CFX_BreakType::Page, rtf_break->AppendChar(L'\f'));
  const wchar_t kUnicodeParagraphSeparator = 0x2029;
  EXPECT_EQ(CFX_BreakType::Paragraph,
            rtf_break->AppendChar(kUnicodeParagraphSeparator));
  EXPECT_EQ(CFX_BreakType::Paragraph, rtf_break->AppendChar(L'\n'));

  ASSERT_EQ(1, rtf_break->CountBreakPieces());
  EXPECT_EQ(L"\v", rtf_break->GetBreakPieceUnstable(0)->GetString());
}

TEST_F(CFX_RTFBreakTest, BidiLine) {
  auto rtf_break = CreateBreak(FX_LAYOUTSTYLE_ExpandTab);
  rtf_break->SetLineBreakTolerance(1);
  rtf_break->SetFontSize(12);

  WideString input = WideString::FromUTF8(ByteStringView("\xa\x0\xa\xa", 4));
  for (wchar_t ch : input)
    rtf_break->AppendChar(ch);

  std::vector<CFX_Char> chars =
      rtf_break->GetCurrentLineForTesting()->m_LineChars;
  CFX_Char::BidiLine(&chars, chars.size());
  EXPECT_EQ(3u, chars.size());
}
