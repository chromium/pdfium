// Copyright 2025 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/font/cpdf_truetypefont.h"

#include <stdint.h>

#include <array>
#include <utility>

#include "core/fpdfapi/page/test_with_page_module.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_test_document.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxge/fontdata/chromefontdata/chromefontdata.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

using CPDFTrueTypeFontTest = TestWithPageModule;

// The following ttf data are all identical except for the format of the single
// Unicode cmap which maps U+2E to glyph 1.

constexpr std::array<uint8_t, 508> kCmap03Ttf = {
    {0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x80, 0x00, 0x03, 0x00, 0x00,
     0x63, 0x6d, 0x61, 0x70, 0x00, 0x09, 0x00, 0x63, 0x00, 0x00, 0x01, 0x10,
     0x00, 0x00, 0x00, 0x2c, 0x67, 0x6c, 0x79, 0x66, 0x15, 0x59, 0x2e, 0xc4,
     0x00, 0x00, 0x01, 0x44, 0x00, 0x00, 0x00, 0x30, 0x68, 0x65, 0x61, 0x64,
     0xf4, 0x41, 0xf2, 0xdf, 0x00, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x36,
     0x68, 0x68, 0x65, 0x61, 0x0d, 0x3d, 0x05, 0x03, 0x00, 0x00, 0x00, 0xc4,
     0x00, 0x00, 0x00, 0x24, 0x68, 0x6d, 0x74, 0x78, 0x08, 0x39, 0x01, 0xba,
     0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x08, 0x6c, 0x6f, 0x63, 0x61,
     0x00, 0x18, 0x00, 0x0d, 0x00, 0x00, 0x01, 0x3c, 0x00, 0x00, 0x00, 0x06,
     0x6d, 0x61, 0x78, 0x70, 0x04, 0xd2, 0x04, 0x52, 0x00, 0x00, 0x00, 0xe8,
     0x00, 0x00, 0x00, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x02, 0x5f, 0x01, 0xe1,
     0x00, 0x00, 0x01, 0x74, 0x00, 0x00, 0x00, 0x88, 0x00, 0x01, 0x00, 0x00,
     0x00, 0x05, 0x02, 0x8f, 0x18, 0x96, 0xe8, 0x3b, 0x5f, 0x0f, 0x3c, 0xf5,
     0x08, 0x1b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa2, 0xe3, 0x27, 0x2a,
     0x00, 0x00, 0x00, 0x00, 0xe4, 0x73, 0x7f, 0x28, 0x00, 0xba, 0x00, 0x00,
     0x05, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x01, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x07, 0x3e, 0xfe, 0x4e,
     0x00, 0x43, 0x06, 0x00, 0x00, 0xba, 0x00, 0xb2, 0x05, 0x00, 0x00, 0x01,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04,
     0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x2f,
     0x00, 0x56, 0x00, 0x00, 0x04, 0x68, 0x04, 0x1d, 0x00, 0x00, 0x00, 0x00,
     0x06, 0x00, 0x01, 0x00, 0x02, 0x39, 0x00, 0xba, 0x00, 0x00, 0x00, 0x01,
     0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x20,
     0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2e,
     0xff, 0xff, 0x00, 0x00, 0x00, 0x2e, 0xff, 0xff, 0xff, 0xd3, 0x00, 0x01,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x18, 0x00, 0x00,
     0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x00, 0x03,
     0x00, 0x00, 0x21, 0x11, 0x21, 0x11, 0x01, 0x00, 0x04, 0x00, 0x05, 0x00,
     0xfb, 0x00, 0x00, 0x01, 0x00, 0xba, 0x00, 0x00, 0x01, 0x87, 0x00, 0xcd,
     0x00, 0x03, 0x00, 0x00, 0x33, 0x35, 0x33, 0x15, 0xba, 0xcd, 0xcd, 0xcd,
     0x00, 0x00, 0x00, 0x09, 0x00, 0x72, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x03, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x05, 0x00, 0x02, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x06, 0x00, 0x06, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x08, 0x00, 0x06, 0x00, 0x10, 0x00, 0x41, 0x00, 0x52, 0x00, 0x41,
     0x00, 0x52, 0x00, 0x56, 0x00, 0x41, 0x00, 0x4d, 0x00, 0x54, 0x00, 0x54,
     0x00, 0x4d, 0x00, 0x43}};
constexpr std::array<uint8_t, 508> kCmap31Ttf = {
    {0x00, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00, 0x80, 0x00, 0x03, 0x00, 0x00,
     0x63, 0x6d, 0x61, 0x70, 0x00, 0x0c, 0x00, 0x61, 0x00, 0x00, 0x01, 0x10,
     0x00, 0x00, 0x00, 0x2c, 0x67, 0x6c, 0x79, 0x66, 0x15, 0x59, 0x2e, 0xc4,
     0x00, 0x00, 0x01, 0x44, 0x00, 0x00, 0x00, 0x30, 0x68, 0x65, 0x61, 0x64,
     0xf4, 0x41, 0xf4, 0x22, 0x00, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x36,
     0x68, 0x68, 0x65, 0x61, 0x0d, 0x3d, 0x05, 0x03, 0x00, 0x00, 0x00, 0xc4,
     0x00, 0x00, 0x00, 0x24, 0x68, 0x6d, 0x74, 0x78, 0x08, 0x39, 0x01, 0xba,
     0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x08, 0x6c, 0x6f, 0x63, 0x61,
     0x00, 0x18, 0x00, 0x0d, 0x00, 0x00, 0x01, 0x3c, 0x00, 0x00, 0x00, 0x06,
     0x6d, 0x61, 0x78, 0x70, 0x04, 0xd2, 0x04, 0x52, 0x00, 0x00, 0x00, 0xe8,
     0x00, 0x00, 0x00, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x02, 0x5f, 0x01, 0xe1,
     0x00, 0x00, 0x01, 0x74, 0x00, 0x00, 0x00, 0x88, 0x00, 0x01, 0x00, 0x00,
     0x00, 0x05, 0x02, 0x8f, 0x18, 0x90, 0xe5, 0xb9, 0x5f, 0x0f, 0x3c, 0xf5,
     0x08, 0x1b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa2, 0xe3, 0x27, 0x2a,
     0x00, 0x00, 0x00, 0x00, 0xe4, 0x73, 0x80, 0x6b, 0x00, 0xba, 0x00, 0x00,
     0x05, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x01, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x07, 0x3e, 0xfe, 0x4e,
     0x00, 0x43, 0x06, 0x00, 0x00, 0xba, 0x00, 0xb2, 0x05, 0x00, 0x00, 0x01,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04,
     0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x2f,
     0x00, 0x56, 0x00, 0x00, 0x04, 0x68, 0x04, 0x1d, 0x00, 0x00, 0x00, 0x00,
     0x06, 0x00, 0x01, 0x00, 0x02, 0x39, 0x00, 0xba, 0x00, 0x00, 0x00, 0x01,
     0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x20,
     0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2e,
     0xff, 0xff, 0x00, 0x00, 0x00, 0x2e, 0xff, 0xff, 0xff, 0xd3, 0x00, 0x01,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x18, 0x00, 0x00,
     0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00, 0x00, 0x03,
     0x00, 0x00, 0x21, 0x11, 0x21, 0x11, 0x01, 0x00, 0x04, 0x00, 0x05, 0x00,
     0xfb, 0x00, 0x00, 0x01, 0x00, 0xba, 0x00, 0x00, 0x01, 0x87, 0x00, 0xcd,
     0x00, 0x03, 0x00, 0x00, 0x33, 0x35, 0x33, 0x15, 0xba, 0xcd, 0xcd, 0xcd,
     0x00, 0x00, 0x00, 0x09, 0x00, 0x72, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x03, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x05, 0x00, 0x02, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x06, 0x00, 0x06, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x07, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
     0x00, 0x08, 0x00, 0x06, 0x00, 0x10, 0x00, 0x41, 0x00, 0x52, 0x00, 0x41,
     0x00, 0x52, 0x00, 0x56, 0x00, 0x41, 0x00, 0x4d, 0x00, 0x54, 0x00, 0x54,
     0x00, 0x4d, 0x00, 0x43}};

}  // namespace

TEST_F(CPDFTrueTypeFontTest, AllUnicodeCmapsTreatedEqually) {
  auto make_font =
      [](CPDF_TestDocument& doc,
         pdfium::span<const uint8_t> font_data) -> RetainPtr<CPDF_Font> {
    auto font_file_stream = doc.NewIndirect<CPDF_Stream>(
        DataVector<uint8_t>(std::begin(font_data), std::end(font_data)),
        pdfium::MakeRetain<CPDF_Dictionary>());
    const uint32_t font_file_stream_object_number =
        font_file_stream->GetObjNum();
    CHECK_GT(font_file_stream_object_number, 0u);
    auto font_file_stream_ref = pdfium::MakeRetain<CPDF_Reference>(
        &doc, font_file_stream_object_number);

    auto font_descriptor_dict = pdfium::MakeRetain<CPDF_Dictionary>();
    font_descriptor_dict->SetNewFor<CPDF_Name>("Type", "FontDescriptor");
    font_descriptor_dict->SetFor("FontFile2", std::move(font_file_stream_ref));

    // The MacRoman 0x2E char code to U+01 mapping is intentionally
    // non-standard.
    static constexpr char kToUnicode[] = (R"(
/CIDInit /ProcSet findresource begin
12 dict begin
begincmap
/CIDSystemInfo <<
    /Registry (Adobe)
    /Ordering (UCS)
    /Supplement 0
>> def
/CMapName /Adobe-Identity-UCS def
/CMapType 2 def
1 begincodespacerange
<00><FF>
endcodespacerange
1 beginbfrange
<2E><2E><0001>
endbfrange
endcmap
CMapName currentdict /CMap defineresource pop
end
end)");
    auto to_unicode_stream = doc.NewIndirect<CPDF_Stream>(
        DataVector<uint8_t>(std::begin(kToUnicode), std::end(kToUnicode)),
        pdfium::MakeRetain<CPDF_Dictionary>());
    const uint32_t to_unicode_object_number = to_unicode_stream->GetObjNum();
    CHECK_GT(to_unicode_object_number, 0u);
    auto to_unicode_ref =
        pdfium::MakeRetain<CPDF_Reference>(&doc, to_unicode_object_number);

    auto font_dict = pdfium::MakeRetain<CPDF_Dictionary>();
    font_dict->SetNewFor<CPDF_Name>("BaseFont", "CHEESE+Swiss");
    font_dict->SetNewFor<CPDF_Name>("Type", "Font");
    font_dict->SetNewFor<CPDF_Name>("Subtype", "TrueType");
    font_dict->SetNewFor<CPDF_Name>("Encoding", "MacRomanEncoding");
    font_dict->SetFor("ToUnicode", std::move(to_unicode_ref));
    font_dict->SetFor("FontDescriptor", std::move(font_descriptor_dict));

    return CPDF_Font::Create(&doc, std::move(font_dict), nullptr);
  };

  CPDF_TestDocument doc;
  auto cmap31 = make_font(doc, kCmap31Ttf);
  auto cmap03 = make_font(doc, kCmap03Ttf);
  ASSERT_TRUE(cmap31);
  ASSERT_TRUE(cmap03);

  uint32_t period = 0x2E;
  bool unused_is_vert;
  int cmap31_glyph = cmap31->GlyphFromCharCode(period, &unused_is_vert);
  int cmap03_glyph = cmap03->GlyphFromCharCode(period, &unused_is_vert);

  EXPECT_EQ(cmap31_glyph, 1);
  EXPECT_EQ(cmap31_glyph, cmap03_glyph);
}
