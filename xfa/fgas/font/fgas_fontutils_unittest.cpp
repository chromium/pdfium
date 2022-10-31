// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fgas/font/fgas_fontutils.h"

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/widestring.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(FGAS, GetUnicodeBitField) {
  const auto* pResult = FGAS_GetUnicodeBitField(0);
  ASSERT_TRUE(pResult);
  EXPECT_EQ(0u, pResult->wBitField);
  EXPECT_EQ(FX_CodePage::kMSWin_WesternEuropean, pResult->wCodePage);

  pResult = FGAS_GetUnicodeBitField(65535);
  EXPECT_FALSE(pResult);

  // Try arbitrary values.
  pResult = FGAS_GetUnicodeBitField(1313);
  ASSERT_TRUE(pResult);
  EXPECT_EQ(9u, pResult->wBitField);
  EXPECT_EQ(FX_CodePage::kFailure, pResult->wCodePage);

  pResult = FGAS_GetUnicodeBitField(14321);
  ASSERT_TRUE(pResult);
  EXPECT_EQ(59u, pResult->wBitField);
  EXPECT_EQ(FX_CodePage::kFailure, pResult->wCodePage);
}

TEST(FGAS, FontNameToEnglishName) {
  // These aren't found with spaces.
  WideString result = FGAS_FontNameToEnglishName(L"Myriad Pro");
  EXPECT_EQ(L"Myriad Pro", result);

  result = FGAS_FontNameToEnglishName(L"mYriad pRo");
  EXPECT_EQ(L"mYriad pRo", result);

  result = FGAS_FontNameToEnglishName(L"MyriadPro");
  EXPECT_EQ(L"MyriadPro", result);

  result = FGAS_FontNameToEnglishName(L"mYriadpRo");
  EXPECT_EQ(L"MyriadPro", result);
}

TEST(FGAS, FontInfoByFontName) {
  // And yet, these are found despite spaces.
  const auto* result = FGAS_FontInfoByFontName(L"Myriad Pro");
  EXPECT_TRUE(result);

  result = FGAS_FontInfoByFontName(L"mYriad pRo");
  EXPECT_TRUE(result);

  result = FGAS_FontInfoByFontName(L"MyriadPro");
  EXPECT_TRUE(result);

  result = FGAS_FontInfoByFontName(L"mYriadpRo");
  EXPECT_TRUE(result);
}
