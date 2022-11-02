// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxge/win32/cfx_psrenderer.h"

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "core/fxge/win32/cfx_psfonttracker.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/base/span.h"

namespace {

DataVector<uint8_t> FakeA85Encode(pdfium::span<const uint8_t> src_span) {
  return DataVector<uint8_t>({'d', 'u', 'm', 'm', 'y', 'a', '8', '5'});
}

class TestWriteStream final : public IFX_RetainableWriteStream {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  // IFX_RetainableWriteStream:
  bool WriteBlock(pdfium::span<const uint8_t> buffer) override {
    data_.insert(data_.end(), buffer.begin(), buffer.end());
    return true;
  }

  pdfium::span<const uint8_t> GetSpan() const { return data_; }

 private:
  DataVector<uint8_t> data_;
};

}  // namespace

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

TEST(PSRendererTest, DrawDIBits) {
  static constexpr char kExpectedOutput[] = R"(
save
/im/initmatrix load def
/n/newpath load def/m/moveto load def/l/lineto load def/c/curveto load def/h/closepath load def
/f/fill load def/F/eofill load def/s/stroke load def/W/clip load def/W*/eoclip load def
/rg/setrgbcolor load def/k/setcmykcolor load def
/J/setlinecap load def/j/setlinejoin load def/w/setlinewidth load def/M/setmiterlimit load def/d/setdash load def
/q/gsave load def/Q/grestore load def/iM/imagemask load def
/Tj/show load def/Ff/findfont load def/Fs/scalefont load def/Sf/setfont load def
/cm/concat load def/Cm/currentmatrix load def/mx/matrix load def/sm/setmatrix load def
q
[1 0 0 1 0 0]cm 10 2 1[10 0 0 -2 0 2]currentfile/ASCII85Decode filter false 1 colorimage
dummya85
Q

restore
)";
  auto output_stream = pdfium::MakeRetain<TestWriteStream>();

  {
    constexpr int kWidth = 10;
    constexpr int kHeight = 2;
    CFX_PSFontTracker font_tracker;
    const EncoderIface encoder_interface{&FakeA85Encode, nullptr, nullptr,
                                         nullptr, nullptr};
    CFX_PSRenderer renderer(&font_tracker, &encoder_interface);
    renderer.Init(output_stream, CFX_PSRenderer::RenderingLevel::kLevel2,
                  kWidth, kHeight);

    auto bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
    bool result = bitmap->Create(kWidth, kHeight, FXDIB_Format::k1bppRgb);
    ASSERT_TRUE(result);
    bitmap->Clear(0);
    ASSERT_TRUE(renderer.DrawDIBits(bitmap, /*color=*/0, CFX_Matrix(),
                                    FXDIB_ResampleOptions()));
  }

  ByteString output(output_stream->GetSpan());
  EXPECT_STREQ(output.c_str(), kExpectedOutput);
}
