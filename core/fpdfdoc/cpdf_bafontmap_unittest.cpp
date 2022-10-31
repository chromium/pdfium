// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_bafontmap.h"

#include <utility>

#include "build/build_config.h"
#include "constants/annotation_common.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/test_with_page_module.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fpdfapi/parser/cpdf_test_document.h"

using BAFontMapTest = TestWithPageModule;

TEST_F(BAFontMapTest, DefaultFont) {
  // Without any font resources, CPDF_BAFontMap generates a default font.
  CPDF_TestDocument doc;

  auto annot_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  annot_dict->SetNewFor<CPDF_Name>(pdfium::annotation::kSubtype, "Widget");
  annot_dict->SetNewFor<CPDF_String>("DA", "0 0 0 rg /F1 12 Tf",
                                     /*bHex=*/false);

  CPDF_BAFontMap font_map(&doc, std::move(annot_dict), "N");
#if !BUILDFLAG(IS_WIN)
  // TODO(thestig): Figure out why this does not work on Windows.
  EXPECT_EQ(font_map.GetPDFFontAlias(0), "Helvetica_00");
#endif
  RetainPtr<CPDF_Font> font = font_map.GetPDFFont(0);
  ASSERT_TRUE(font);
  EXPECT_TRUE(font->IsType1Font());
  EXPECT_EQ(font->GetBaseFontName(), "Helvetica");
}

TEST_F(BAFontMapTest, Bug853238) {
  CPDF_TestDocument doc;
  auto root_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  auto acroform_dict = root_dict->SetNewFor<CPDF_Dictionary>("AcroForm");
  auto annot_dr_dict = acroform_dict->SetNewFor<CPDF_Dictionary>("DR");
  auto annot_font_dict = annot_dr_dict->SetNewFor<CPDF_Dictionary>("Font");
  auto annot_font_f1_dict = annot_font_dict->SetNewFor<CPDF_Dictionary>("F1");
  annot_font_f1_dict->SetNewFor<CPDF_Name>("Type", "Font");
  annot_font_f1_dict->SetNewFor<CPDF_Name>("Subtype", "Type1");
  annot_font_f1_dict->SetNewFor<CPDF_Name>("BaseFont", "Times-Roman");
  doc.SetRoot(root_dict);

  auto annot_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  annot_dict->SetNewFor<CPDF_Name>(pdfium::annotation::kSubtype, "Widget");
  annot_dict->SetNewFor<CPDF_String>("DA", "0 0 0 rg /F1 12 Tf",
                                     /*bHex=*/false);

  CPDF_BAFontMap font_map(&doc, std::move(annot_dict), "N");
  EXPECT_EQ(font_map.GetPDFFontAlias(0), "F1");
  RetainPtr<CPDF_Font> font = font_map.GetPDFFont(0);
  ASSERT_TRUE(font);
  EXPECT_TRUE(font->IsType1Font());
  EXPECT_EQ(font->GetBaseFontName(), "Times-Roman");
}
