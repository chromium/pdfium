// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"

#include <memory>
#include <utility>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/render/cpdf_docrenderdata.h"
#include "core/fxge/render_defines.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

class CPDF_PageContentGeneratorTest : public testing::Test {
 protected:
  void SetUp() override { CPDF_PageModule::Create(); }
  void TearDown() override { CPDF_PageModule::Destroy(); }

  void TestProcessPath(CPDF_PageContentGenerator* pGen,
                       std::ostringstream* buf,
                       CPDF_PathObject* pPathObj) {
    pGen->ProcessPath(buf, pPathObj);
  }

  CPDF_Dictionary* TestGetResource(CPDF_PageContentGenerator* pGen,
                                   const ByteString& type,
                                   const ByteString& name) {
    return pGen->m_pObjHolder->m_pResources->GetDictFor(type)->GetDictFor(name);
  }

  void TestProcessText(CPDF_PageContentGenerator* pGen,
                       std::ostringstream* buf,
                       CPDF_TextObject* pTextObj) {
    pGen->ProcessText(buf, pTextObj);
  }
};

TEST_F(CPDF_PageContentGeneratorTest, ProcessRect) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->set_stroke(true);
  pPathObj->set_filltype(FXFILL_ALTERNATE);
  pPathObj->path().AppendRect(10, 5, 13, 30);

  auto dummy_page_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  auto pTestPage =
      pdfium::MakeRetain<CPDF_Page>(nullptr, dummy_page_dict.Get());
  CPDF_PageContentGenerator generator(pTestPage.Get());
  std::ostringstream buf;
  TestProcessPath(&generator, &buf, pPathObj.get());
  EXPECT_EQ("q 1 0 0 1 0 0 cm 10 5 3 25 re B* Q\n", ByteString(buf));

  pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->path().AppendPoint(CFX_PointF(0, 0), FXPT_TYPE::MoveTo, false);
  pPathObj->path().AppendPoint(CFX_PointF(5.2f, 0), FXPT_TYPE::LineTo, false);
  pPathObj->path().AppendPoint(CFX_PointF(5.2f, 3.78f), FXPT_TYPE::LineTo,
                               false);
  pPathObj->path().AppendPoint(CFX_PointF(0, 3.78f), FXPT_TYPE::LineTo, true);
  buf.str("");
  TestProcessPath(&generator, &buf, pPathObj.get());
  EXPECT_EQ("q 1 0 0 1 0 0 cm 0 0 5.1999998 3.78 re n Q\n", ByteString(buf));
}

TEST_F(CPDF_PageContentGeneratorTest, BUG_937) {
  static const std::vector<float> rgb = {0.000000000000000000001f, 0.7f, 0.35f};
  RetainPtr<CPDF_ColorSpace> pCS = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB);
  {
    auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
    pPathObj->set_filltype(FXFILL_WINDING);

    // Test code in ProcessPath that generates re operator
    pPathObj->path().AppendRect(0.000000000000000000001,
                                0.000000000000000000001, 100, 100);

    pPathObj->m_ColorState.SetFillColor(pCS, rgb);
    pPathObj->m_ColorState.SetStrokeColor(pCS, rgb);
    pPathObj->m_GraphState.SetLineWidth(200000000000000000001.0);
    pPathObj->Transform(CFX_Matrix(1, 0, 0, 1, 0.000000000000000000001,
                                   200000000000000.000002));

    auto dummy_page_dict = pdfium::MakeRetain<CPDF_Dictionary>();
    auto pTestPage =
        pdfium::MakeRetain<CPDF_Page>(nullptr, dummy_page_dict.Get());
    CPDF_PageContentGenerator generator(pTestPage.Get());
    std::ostringstream buf;
    TestProcessPath(&generator, &buf, pPathObj.get());
    EXPECT_EQ(
        "q 0 0.701961 0.34902 rg 0 0.701961 0.34902 RG 200000000000000000000 w"
        " 1 0 0 1 .00000000000000000000099999997 200000000000000 cm .000000000"
        "00000000000099999997 .00000000000000000000099999997 100 100 re f Q\n",
        ByteString(buf));
  }

  {
    // Test code in ProcessPath that handles bezier operator
    auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
    pPathObj->m_ColorState.SetFillColor(pCS, rgb);
    pPathObj->m_ColorState.SetStrokeColor(pCS, rgb);
    pPathObj->m_GraphState.SetLineWidth(2.000000000000000000001);
    pPathObj->Transform(CFX_Matrix(1, 0, 0, 1, 432, 500000000000000.000002));

    pPathObj->set_filltype(FXFILL_WINDING);
    pPathObj->path().AppendPoint(CFX_PointF(0.000000000000000000001f, 4.67f),
                                 FXPT_TYPE::MoveTo, false);
    pPathObj->path().AppendPoint(
        CFX_PointF(0.000000000000000000001, 100000000000000.000002),
        FXPT_TYPE::LineTo, false);
    pPathObj->path().AppendPoint(CFX_PointF(0.0000000000001f, 3.15f),
                                 FXPT_TYPE::BezierTo, false);
    pPathObj->path().AppendPoint(CFX_PointF(3.57f, 2.98f), FXPT_TYPE::BezierTo,
                                 false);
    pPathObj->path().AppendPoint(
        CFX_PointF(53.4f, 5000000000000000000.00000000000000004),
        FXPT_TYPE::BezierTo, true);
    auto dummy_page_dict = pdfium::MakeRetain<CPDF_Dictionary>();
    auto pTestPage =
        pdfium::MakeRetain<CPDF_Page>(nullptr, dummy_page_dict.Get());
    CPDF_PageContentGenerator generator(pTestPage.Get());
    std::ostringstream buf;

    TestProcessPath(&generator, &buf, pPathObj.get());
    EXPECT_EQ(
        "q 0 0.701961 0.34902 rg 0 0.701961 0.34902 RG 2 w 1 0 0 1 432 4999999"
        "90000000 cm .00000000000000000000099999997 4.6700001 m .0000000000000"
        "0000000099999997 100000000000000 l .000000000000099999998 3.1500001 3"
        ".5699999 2.98 53.400002 5000000000000000000 c h f Q\n",
        ByteString(buf));
  }
}

TEST_F(CPDF_PageContentGeneratorTest, ProcessPath) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->set_filltype(FXFILL_WINDING);
  pPathObj->path().AppendPoint(CFX_PointF(3.102f, 4.67f), FXPT_TYPE::MoveTo,
                               false);
  pPathObj->path().AppendPoint(CFX_PointF(5.45f, 0.29f), FXPT_TYPE::LineTo,
                               false);
  pPathObj->path().AppendPoint(CFX_PointF(4.24f, 3.15f), FXPT_TYPE::BezierTo,
                               false);
  pPathObj->path().AppendPoint(CFX_PointF(4.65f, 2.98f), FXPT_TYPE::BezierTo,
                               false);
  pPathObj->path().AppendPoint(CFX_PointF(3.456f, 0.24f), FXPT_TYPE::BezierTo,
                               false);
  pPathObj->path().AppendPoint(CFX_PointF(10.6f, 11.15f), FXPT_TYPE::LineTo,
                               false);
  pPathObj->path().AppendPoint(CFX_PointF(11, 12.5f), FXPT_TYPE::LineTo, false);
  pPathObj->path().AppendPoint(CFX_PointF(11.46f, 12.67f), FXPT_TYPE::BezierTo,
                               false);
  pPathObj->path().AppendPoint(CFX_PointF(11.84f, 12.96f), FXPT_TYPE::BezierTo,
                               false);
  pPathObj->path().AppendPoint(CFX_PointF(12, 13.64f), FXPT_TYPE::BezierTo,
                               true);

  auto dummy_page_dict = pdfium::MakeRetain<CPDF_Dictionary>();
  auto pTestPage =
      pdfium::MakeRetain<CPDF_Page>(nullptr, dummy_page_dict.Get());
  CPDF_PageContentGenerator generator(pTestPage.Get());
  std::ostringstream buf;
  TestProcessPath(&generator, &buf, pPathObj.get());
  EXPECT_EQ(
      "q 1 0 0 1 0 0 cm 3.102 4.6700001 m 5.4499998 .28999999 l 4.2399998 "
      "3.1500001 4.6500001 2.98 3.4560001 .23999999 c 10.6000004 11.149999"
      "6 l 11 12.5 l 11.46 12.6700001 11.8400002 12.96 12 13.6400003 c h f"
      " Q\n",
      ByteString(buf));
}

TEST_F(CPDF_PageContentGeneratorTest, ProcessGraphics) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->set_stroke(true);
  pPathObj->set_filltype(FXFILL_WINDING);
  pPathObj->path().AppendPoint(CFX_PointF(1, 2), FXPT_TYPE::MoveTo, false);
  pPathObj->path().AppendPoint(CFX_PointF(3, 4), FXPT_TYPE::LineTo, false);
  pPathObj->path().AppendPoint(CFX_PointF(5, 6), FXPT_TYPE::LineTo, true);

  static const std::vector<float> rgb = {0.5f, 0.7f, 0.35f};
  RetainPtr<CPDF_ColorSpace> pCS = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB);
  pPathObj->m_ColorState.SetFillColor(pCS, rgb);

  static const std::vector<float> rgb2 = {1, 0.9f, 0};
  pPathObj->m_ColorState.SetStrokeColor(pCS, rgb2);
  pPathObj->m_GeneralState.SetFillAlpha(0.5f);
  pPathObj->m_GeneralState.SetStrokeAlpha(0.8f);

  auto pDoc = pdfium::MakeUnique<CPDF_Document>(
      pdfium::MakeUnique<CPDF_DocRenderData>(),
      pdfium::MakeUnique<CPDF_DocPageData>());

  pDoc->CreateNewDoc();
  CPDF_Dictionary* pPageDict = pDoc->CreateNewPage(0);
  auto pTestPage = pdfium::MakeRetain<CPDF_Page>(pDoc.get(), pPageDict);
  CPDF_PageContentGenerator generator(pTestPage.Get());
  std::ostringstream buf;
  TestProcessPath(&generator, &buf, pPathObj.get());
  ByteString pathString(buf);

  // Color RGB values used are integers divided by 255.
  EXPECT_EQ("q 0.501961 0.701961 0.34902 rg 1 0.901961 0 RG /",
            pathString.Left(48));
  EXPECT_EQ(" gs 1 0 0 1 0 0 cm 1 2 m 3 4 l 5 6 l h B Q\n",
            pathString.Right(43));
  ASSERT_GT(pathString.GetLength(), 91U);
  CPDF_Dictionary* externalGS = TestGetResource(
      &generator, "ExtGState", pathString.Mid(48, pathString.GetLength() - 91));
  ASSERT_TRUE(externalGS);
  EXPECT_EQ(0.5f, externalGS->GetNumberFor("ca"));
  EXPECT_EQ(0.8f, externalGS->GetNumberFor("CA"));

  // Same path, now with a stroke.
  pPathObj->m_GraphState.SetLineWidth(10.5f);
  buf.str("");
  TestProcessPath(&generator, &buf, pPathObj.get());
  ByteString pathString2(buf);
  EXPECT_EQ("q 0.501961 0.701961 0.34902 rg 1 0.901961 0 RG 10.5 w /",
            pathString2.Left(55));
  EXPECT_EQ(" gs 1 0 0 1 0 0 cm 1 2 m 3 4 l 5 6 l h B Q\n",
            pathString2.Right(43));

  // Compare with the previous (should use same dictionary for gs)
  EXPECT_EQ(pathString.GetLength() + 7, pathString2.GetLength());
  EXPECT_EQ(pathString.Mid(48, pathString.GetLength() - 76),
            pathString2.Mid(55, pathString2.GetLength() - 83));
}

TEST_F(CPDF_PageContentGeneratorTest, ProcessStandardText) {
  // Checking font whose font dictionary is not yet indirect object.
  auto pDoc = pdfium::MakeUnique<CPDF_Document>(
      pdfium::MakeUnique<CPDF_DocRenderData>(),
      pdfium::MakeUnique<CPDF_DocPageData>());

  pDoc->CreateNewDoc();
  CPDF_Dictionary* pPageDict = pDoc->CreateNewPage(0);
  auto pTestPage = pdfium::MakeRetain<CPDF_Page>(pDoc.get(), pPageDict);
  CPDF_PageContentGenerator generator(pTestPage.Get());
  auto pTextObj = pdfium::MakeUnique<CPDF_TextObject>();
  RetainPtr<CPDF_Font> pFont =
      CPDF_Font::GetStockFont(pDoc.get(), "Times-Roman");
  pTextObj->m_TextState.SetFont(pFont);
  pTextObj->m_TextState.SetFontSize(10.0f);

  static const std::vector<float> rgb = {0.5f, 0.7f, 0.35f};
  RetainPtr<CPDF_ColorSpace> pCS = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB);
  pTextObj->m_ColorState.SetFillColor(pCS, rgb);

  static const std::vector<float> rgb2 = {1, 0.9f, 0};
  pTextObj->m_ColorState.SetStrokeColor(pCS, rgb2);
  pTextObj->m_GeneralState.SetFillAlpha(0.5f);
  pTextObj->m_GeneralState.SetStrokeAlpha(0.8f);
  pTextObj->Transform(CFX_Matrix(1, 0, 0, 1, 100, 100));
  pTextObj->SetText("Hello World");
  std::ostringstream buf;
  TestProcessText(&generator, &buf, pTextObj.get());
  ByteString textString(buf);
  auto firstResourceAt = textString.Find('/');
  ASSERT_TRUE(firstResourceAt.has_value());
  firstResourceAt = firstResourceAt.value() + 1;
  auto secondResourceAt = textString.ReverseFind('/');
  ASSERT_TRUE(secondResourceAt.has_value());
  secondResourceAt = secondResourceAt.value() + 1;
  ByteString firstString = textString.Left(firstResourceAt.value());
  ByteString midString =
      textString.Mid(firstResourceAt.value(),
                     secondResourceAt.value() - firstResourceAt.value());
  ByteString lastString =
      textString.Right(textString.GetLength() - secondResourceAt.value());
  // q and Q must be outside the BT .. ET operations
  ByteString compareString1 =
      "q 0.501961 0.701961 0.34902 rg 1 0.901961 0 RG /";
  // Color RGB values used are integers divided by 255.
  ByteString compareString2 = " gs BT 1 0 0 1 100 100 Tm /";
  ByteString compareString3 = " 10 Tf <48656C6C6F20576F726C64> Tj ET Q\n";
  EXPECT_LT(compareString1.GetLength() + compareString2.GetLength() +
                compareString3.GetLength(),
            textString.GetLength());
  EXPECT_EQ(compareString1, firstString.Left(compareString1.GetLength()));
  EXPECT_EQ(compareString2, midString.Right(compareString2.GetLength()));
  EXPECT_EQ(compareString3, lastString.Right(compareString3.GetLength()));
  CPDF_Dictionary* externalGS = TestGetResource(
      &generator, "ExtGState",
      midString.Left(midString.GetLength() - compareString2.GetLength()));
  ASSERT_TRUE(externalGS);
  EXPECT_EQ(0.5f, externalGS->GetNumberFor("ca"));
  EXPECT_EQ(0.8f, externalGS->GetNumberFor("CA"));
  CPDF_Dictionary* fontDict = TestGetResource(
      &generator, "Font",
      lastString.Left(lastString.GetLength() - compareString3.GetLength()));
  ASSERT_TRUE(fontDict);
  EXPECT_EQ("Font", fontDict->GetStringFor("Type"));
  EXPECT_EQ("Type1", fontDict->GetStringFor("Subtype"));
  EXPECT_EQ("Times-Roman", fontDict->GetStringFor("BaseFont"));
}

TEST_F(CPDF_PageContentGeneratorTest, ProcessText) {
  // Checking font whose font dictionary is already an indirect object.
  auto pDoc = pdfium::MakeUnique<CPDF_Document>(
      pdfium::MakeUnique<CPDF_DocRenderData>(),
      pdfium::MakeUnique<CPDF_DocPageData>());
  pDoc->CreateNewDoc();

  CPDF_Dictionary* pPageDict = pDoc->CreateNewPage(0);
  auto pTestPage = pdfium::MakeRetain<CPDF_Page>(pDoc.get(), pPageDict);
  CPDF_PageContentGenerator generator(pTestPage.Get());

  std::ostringstream buf;
  {
    // Set the text object font and text
    auto pTextObj = pdfium::MakeUnique<CPDF_TextObject>();
    CPDF_Dictionary* pDict = pDoc->NewIndirect<CPDF_Dictionary>();
    pDict->SetNewFor<CPDF_Name>("Type", "Font");
    pDict->SetNewFor<CPDF_Name>("Subtype", "TrueType");

    RetainPtr<CPDF_Font> pFont = CPDF_Font::GetStockFont(pDoc.get(), "Arial");
    pDict->SetNewFor<CPDF_Name>("BaseFont", pFont->GetBaseFontName());

    CPDF_Dictionary* pDesc = pDoc->NewIndirect<CPDF_Dictionary>();
    pDesc->SetNewFor<CPDF_Name>("Type", "FontDescriptor");
    pDesc->SetNewFor<CPDF_Name>("FontName", pFont->GetBaseFontName());
    pDict->SetNewFor<CPDF_Reference>("FontDescriptor", pDoc.get(),
                                     pDesc->GetObjNum());

    RetainPtr<CPDF_Font> pLoadedFont =
        CPDF_DocPageData::FromDocument(pDoc.get())->GetFont(pDict);
    pTextObj->m_TextState.SetFont(pLoadedFont);
    pTextObj->m_TextState.SetFontSize(15.5f);
    pTextObj->SetText("I am indirect");
    TestProcessText(&generator, &buf, pTextObj.get());
  }

  ByteString textString(buf);
  auto firstResourceAt = textString.Find('/');
  ASSERT_TRUE(firstResourceAt.has_value());
  firstResourceAt = firstResourceAt.value() + 1;
  ByteString firstString = textString.Left(firstResourceAt.value());
  ByteString lastString =
      textString.Right(textString.GetLength() - firstResourceAt.value());
  // q and Q must be outside the BT .. ET operations
  ByteString compareString1 = "q BT 1 0 0 1 0 0 Tm /";
  ByteString compareString2 = " 15.5 Tf <4920616D20696E646972656374> Tj ET Q\n";
  EXPECT_LT(compareString1.GetLength() + compareString2.GetLength(),
            textString.GetLength());
  EXPECT_EQ(compareString1, textString.Left(compareString1.GetLength()));
  EXPECT_EQ(compareString2, textString.Right(compareString2.GetLength()));
  CPDF_Dictionary* fontDict = TestGetResource(
      &generator, "Font",
      textString.Mid(compareString1.GetLength(),
                     textString.GetLength() - compareString1.GetLength() -
                         compareString2.GetLength()));
  ASSERT_TRUE(fontDict);
  EXPECT_TRUE(fontDict->GetObjNum());
  EXPECT_EQ("Font", fontDict->GetStringFor("Type"));
  EXPECT_EQ("TrueType", fontDict->GetStringFor("Subtype"));
  EXPECT_EQ("Helvetica", fontDict->GetStringFor("BaseFont"));
  CPDF_Dictionary* fontDesc = fontDict->GetDictFor("FontDescriptor");
  ASSERT_TRUE(fontDesc);
  EXPECT_TRUE(fontDesc->GetObjNum());
  EXPECT_EQ("FontDescriptor", fontDesc->GetStringFor("Type"));
  EXPECT_EQ("Helvetica", fontDesc->GetStringFor("FontName"));
}

TEST_F(CPDF_PageContentGeneratorTest, ProcessEmptyForm) {
  auto pDoc = pdfium::MakeUnique<CPDF_Document>(
      pdfium::MakeUnique<CPDF_DocRenderData>(),
      pdfium::MakeUnique<CPDF_DocPageData>());
  pDoc->CreateNewDoc();
  auto pDict = pdfium::MakeRetain<CPDF_Dictionary>();
  auto pStream = pdfium::MakeRetain<CPDF_Stream>(nullptr, 0, std::move(pDict));

  // Create an empty form.
  auto pTestForm =
      pdfium::MakeUnique<CPDF_Form>(pDoc.get(), nullptr, pStream.Get());
  pTestForm->ParseContent();
  ASSERT_EQ(CPDF_PageObjectHolder::ParseState::kParsed,
            pTestForm->GetParseState());

  // The generated stream for the empty form should be an empty string.
  CPDF_PageContentGenerator generator(pTestForm.get());
  std::ostringstream buf;
  generator.ProcessPageObjects(&buf);
  EXPECT_EQ("", ByteString(buf));
}

TEST_F(CPDF_PageContentGeneratorTest, ProcessFormWithPath) {
  auto pDoc = pdfium::MakeUnique<CPDF_Document>(
      pdfium::MakeUnique<CPDF_DocRenderData>(),
      pdfium::MakeUnique<CPDF_DocPageData>());
  pDoc->CreateNewDoc();
  auto pDict = pdfium::MakeRetain<CPDF_Dictionary>();
  const char content[] =
      "q 1 0 0 1 0 0 cm 3.102 4.6700001 m 5.4500012 .28999999 "
      "l 4.2399998 3.1499999 4.65 2.98 3.456 0.24 c 3.102 4.6700001 l h f Q\n";
  size_t buf_len = FX_ArraySize(content);
  std::unique_ptr<uint8_t, FxFreeDeleter> buf(FX_Alloc(uint8_t, buf_len));
  memcpy(buf.get(), content, buf_len);
  auto pStream = pdfium::MakeRetain<CPDF_Stream>(std::move(buf), buf_len,
                                                 std::move(pDict));

  // Create a form with a non-empty stream.
  auto pTestForm =
      pdfium::MakeUnique<CPDF_Form>(pDoc.get(), nullptr, pStream.Get());
  pTestForm->ParseContent();
  ASSERT_EQ(CPDF_PageObjectHolder::ParseState::kParsed,
            pTestForm->GetParseState());

  CPDF_PageContentGenerator generator(pTestForm.get());
  std::ostringstream process_buf;
  generator.ProcessPageObjects(&process_buf);
  EXPECT_STREQ(
      "q 1 0 0 1 0 0 cm 3.102 4.6700001 m 5.4500012 .28999999 l 4.2399998 3.14"
      "99999 4.6500001 2.98 3.4560001 .24000001 c 3.102 4.6700001 l h f Q\n",
      ByteString(process_buf).c_str());
}
