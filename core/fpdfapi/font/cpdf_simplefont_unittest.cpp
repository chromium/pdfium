// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/font/cpdf_simplefont.h"

#include <stdint.h>

#include <utility>

#include "core/fpdfapi/page/test_with_page_module.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_test_document.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/fontdata/chromefontdata/chromefontdata.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

using CPDFSimpleFontTest = TestWithPageModule;

class TestSimpleFont : public CPDF_SimpleFont {
 public:
  TestSimpleFont(CPDF_Document* doc, RetainPtr<CPDF_Dictionary> font_dict)
      : CPDF_SimpleFont(doc, std::move(font_dict)) {}
  ~TestSimpleFont() override = default;

  // CPDF_SimpleFont:
  bool Load() override { return LoadCommon(); }
  void LoadGlyphMap() override {}
};

}  // namespace

TEST_F(CPDFSimpleFontTest, BaseFontNameWithSubsetting) {
  CPDF_TestDocument doc;

  // The code being exercised requires valid font data.
  auto font_file_stream = doc.NewIndirect<CPDF_Stream>(
      DataVector<uint8_t>(std::begin(kFoxitFixedFontData),
                          std::end(kFoxitFixedFontData)),
      pdfium::MakeRetain<CPDF_Dictionary>());
  ASSERT_TRUE(font_file_stream);
  const uint32_t stream_object_number = font_file_stream->GetObjNum();
  ASSERT_GT(stream_object_number, 0u);

  auto font_descriptor_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  font_descriptor_dict->SetFor("FontFile", pdfium::MakeRetain<CPDF_Reference>(
                                               &doc, stream_object_number));

  auto font_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  font_dict->SetNewFor<CPDF_Name>("BaseFont", "CHEESE+Swiss");
  font_dict->SetFor("FontDescriptor", std::move(font_descriptor_dict));

  auto font = pdfium::MakeRetain<TestSimpleFont>(&doc, std::move(font_dict));
  ASSERT_TRUE(font->Load());
  EXPECT_EQ("Swiss", font->GetBaseFontName());
}
