// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/win32/cfx_psrenderer.h"
#include "core/fxcrt/bytestring.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/base/span.h"

TEST(PSRendererTest, GenerateType42SfntData) {
  absl::optional<ByteString> result;

  result = CFX_PSRenderer::GenerateType42SfntDataForTesting("empty", {});
  EXPECT_FALSE(result.has_value());

  constexpr uint8_t kOddByteCountTestData[] = {0, 32, 55};
  static constexpr char kExpectedOddByteCountResult[] = R"(/odd_sfnts [
<
002037
>
] def
)";
  result = CFX_PSRenderer::GenerateType42SfntDataForTesting(
      "odd", kOddByteCountTestData);
  ASSERT_TRUE(result.has_value());
  EXPECT_STREQ(kExpectedOddByteCountResult, result.value().c_str());

  // Requires padding.
  constexpr uint8_t kEvenByteCountTestData[] = {0, 32, 66, 77};
  static constexpr char kExpectedEvenByteCountResult[] = R"(/even_sfnts [
<
0020424D00
>
] def
)";
  result = CFX_PSRenderer::GenerateType42SfntDataForTesting(
      "even", kEvenByteCountTestData);
  ASSERT_TRUE(result.has_value());
  EXPECT_STREQ(kExpectedEvenByteCountResult, result.value().c_str());
}

TEST(PSRendererTest, GenerateType42FontDictionary) {
  ByteString result;

  static constexpr char kExpected1DescendantFontResult[] = R"(8 dict begin
/FontType 42 def
/FontMatrix [1 0 0 1 0 0] def
/FontName /1descendant_0 def
/Encoding 3 array
dup 0 /c00 put
dup 1 /c01 put
dup 2 /c02 put
readonly def
/FontBBox [1 2 3 4] def
/PaintType 0 def
/CharStrings 4 dict dup begin
/.notdef 0 def
/c00 0 def
/c01 1 def
/c02 2 def
end readonly def
/sfnts 1descendant_sfnts def
FontName currentdict end definefont pop
6 dict begin
/FontName /1descendant def
/FontType 0 def
/FontMatrix [1 0 0 1 0 0] def
/FMapType 2 def
/Encoding [
0
] def
/FDepVector [
/1descendant_0 findfont
] def
FontName currentdict end definefont pop
%%EndResource
)";
  result = CFX_PSRenderer::GenerateType42FontDictionaryForTesting(
      "1descendant", FX_RECT(1, 2, 3, 4), /*num_glyphs=*/3,
      /*glyphs_per_descendant_font=*/3);
  EXPECT_STREQ(kExpected1DescendantFontResult, result.c_str());

  static constexpr char kExpected2DescendantFontResult[] = R"(8 dict begin
/FontType 42 def
/FontMatrix [1 0 0 1 0 0] def
/FontName /2descendant_0 def
/Encoding 3 array
dup 0 /c00 put
dup 1 /c01 put
dup 2 /c02 put
readonly def
/FontBBox [12 -5 34 199] def
/PaintType 0 def
/CharStrings 4 dict dup begin
/.notdef 0 def
/c00 0 def
/c01 1 def
/c02 2 def
end readonly def
/sfnts 2descendant_sfnts def
FontName currentdict end definefont pop
8 dict begin
/FontType 42 def
/FontMatrix [1 0 0 1 0 0] def
/FontName /2descendant_1 def
/Encoding 3 array
dup 0 /c00 put
dup 1 /c01 put
readonly def
/FontBBox [12 -5 34 199] def
/PaintType 0 def
/CharStrings 4 dict dup begin
/.notdef 0 def
/c00 3 def
/c01 4 def
end readonly def
/sfnts 2descendant_sfnts def
FontName currentdict end definefont pop
6 dict begin
/FontName /2descendant def
/FontType 0 def
/FontMatrix [1 0 0 1 0 0] def
/FMapType 2 def
/Encoding [
0
1
] def
/FDepVector [
/2descendant_0 findfont
/2descendant_1 findfont
] def
FontName currentdict end definefont pop
%%EndResource
)";
  result = CFX_PSRenderer::GenerateType42FontDictionaryForTesting(
      "2descendant", FX_RECT(12, -5, 34, 199), /*num_glyphs=*/5,
      /*glyphs_per_descendant_font=*/3);
  EXPECT_STREQ(kExpected2DescendantFontResult, result.c_str());
}
