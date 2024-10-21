// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/font/cpdf_cidfont.h"

#include <utility>

#include "core/fpdfapi/page/test_with_page_module.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_test_document.h"
#include "testing/gtest/include/gtest/gtest.h"

using CPDFCIDFontTest = TestWithPageModule;

TEST_F(CPDFCIDFontTest, Bug920636) {
  CPDF_TestDocument doc;
  auto font_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  font_dict->SetNewFor<CPDF_Name>("Encoding", "Identity−H");

  {
    auto descendant_fonts = pdfium::MakeRetain<CPDF_Array>();
    {
      auto descendant_font = pdfium::MakeRetain<CPDF_Dictionary>();
      descendant_font->SetNewFor<CPDF_Name>("BaseFont", "CourierStd");
      descendant_fonts->Append(std::move(descendant_font));
    }
    font_dict->SetFor("DescendantFonts", std::move(descendant_fonts));
  }

  auto font = pdfium::MakeRetain<CPDF_CIDFont>(&doc, std::move(font_dict));
  ASSERT_TRUE(font->Load());

  // It would be nice if we can test more values here. However, the glyph
  // indices are sometimes machine dependent.
  struct {
    uint32_t charcode;
    int glyph;
  } static constexpr kTestCases[] = {
      {0, 31},
      {256, 287},
      {34661, 34692},
  };

  for (const auto& test_case : kTestCases) {
    EXPECT_EQ(test_case.glyph,
              font->GlyphFromCharCode(test_case.charcode, nullptr));
  }
}
