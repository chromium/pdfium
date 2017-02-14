// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

class CPDF_PageContentGeneratorTest : public testing::Test {
 protected:
  void SetUp() override { CPDF_ModuleMgr::Get()->InitPageModule(); }

  void TearDown() override { CPDF_ModuleMgr::Destroy(); }

  void TestProcessPath(CPDF_PageContentGenerator* pGen,
                       CFX_ByteTextBuf* buf,
                       CPDF_PathObject* pPathObj) {
    pGen->ProcessPath(buf, pPathObj);
  }

  CPDF_Dictionary* TestGetResource(CPDF_PageContentGenerator* pGen,
                                   const CFX_ByteString& type,
                                   const CFX_ByteString& name) {
    return pGen->m_pPage->m_pResources->GetDictFor(type)->GetDictFor(name);
  }

  void TestProcessText(CPDF_PageContentGenerator* pGen,
                       CFX_ByteTextBuf* buf,
                       CPDF_TextObject* pTextObj) {
    pGen->ProcessText(buf, pTextObj);
  }
};

TEST_F(CPDF_PageContentGeneratorTest, ProcessRect) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->m_Path.AppendRect(10, 5, 13, 30);
  pPathObj->m_FillType = FXFILL_ALTERNATE;
  pPathObj->m_bStroke = true;
  auto pTestPage = pdfium::MakeUnique<CPDF_Page>(nullptr, nullptr, false);
  CPDF_PageContentGenerator generator(pTestPage.get());
  CFX_ByteTextBuf buf;
  TestProcessPath(&generator, &buf, pPathObj.get());
  EXPECT_EQ("q 10 5 3 25 re B* Q\n", buf.MakeString());

  pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->m_Path.SetPointCount(4);
  FX_PATHPOINT* pPoints = pPathObj->m_Path.GetMutablePoints();
  pPoints[0].m_PointX = 0;
  pPoints[0].m_PointY = 0;
  pPoints[0].m_Type = FXPT_TYPE::MoveTo;
  pPoints[0].m_CloseFigure = false;
  pPoints[1].m_PointX = 5.2f;
  pPoints[1].m_PointY = 0;
  pPoints[1].m_Type = FXPT_TYPE::LineTo;
  pPoints[1].m_CloseFigure = false;
  pPoints[2].m_PointX = 5.2f;
  pPoints[2].m_PointY = 3.78f;
  pPoints[2].m_Type = FXPT_TYPE::LineTo;
  pPoints[2].m_CloseFigure = false;
  pPoints[3].m_PointX = 0;
  pPoints[3].m_PointY = 3.78f;
  pPoints[3].m_Type = FXPT_TYPE::LineTo;
  pPoints[3].m_CloseFigure = true;
  pPathObj->m_FillType = 0;
  pPathObj->m_bStroke = false;
  buf.Clear();
  TestProcessPath(&generator, &buf, pPathObj.get());
  EXPECT_EQ("q 0 0 5.2 3.78 re n Q\n", buf.MakeString());
}

TEST_F(CPDF_PageContentGeneratorTest, ProcessPath) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->m_Path.SetPointCount(10);
  FX_PATHPOINT* pPoints = pPathObj->m_Path.GetMutablePoints();
  pPoints[0].m_PointX = 3.102f;
  pPoints[0].m_PointY = 4.67f;
  pPoints[0].m_Type = FXPT_TYPE::MoveTo;
  pPoints[0].m_CloseFigure = false;
  pPoints[1].m_PointX = 5.45f;
  pPoints[1].m_PointY = 0.29f;
  pPoints[1].m_Type = FXPT_TYPE::LineTo;
  pPoints[1].m_CloseFigure = false;
  pPoints[2].m_PointX = 4.24f;
  pPoints[2].m_PointY = 3.15f;
  pPoints[2].m_Type = FXPT_TYPE::BezierTo;
  pPoints[2].m_CloseFigure = false;
  pPoints[3].m_PointX = 4.65f;
  pPoints[3].m_PointY = 2.98f;
  pPoints[3].m_Type = FXPT_TYPE::BezierTo;
  pPoints[3].m_CloseFigure = false;
  pPoints[4].m_PointX = 3.456f;
  pPoints[4].m_PointY = 0.24f;
  pPoints[4].m_Type = FXPT_TYPE::BezierTo;
  pPoints[4].m_CloseFigure = false;
  pPoints[5].m_PointX = 10.6f;
  pPoints[5].m_PointY = 11.15f;
  pPoints[5].m_Type = FXPT_TYPE::LineTo;
  pPoints[5].m_CloseFigure = false;
  pPoints[6].m_PointX = 11;
  pPoints[6].m_PointY = 12.5f;
  pPoints[6].m_Type = FXPT_TYPE::LineTo;
  pPoints[6].m_CloseFigure = false;
  pPoints[7].m_PointX = 11.46f;
  pPoints[7].m_PointY = 12.67f;
  pPoints[7].m_Type = FXPT_TYPE::BezierTo;
  pPoints[7].m_CloseFigure = false;
  pPoints[8].m_PointX = 11.84f;
  pPoints[8].m_PointY = 12.96f;
  pPoints[8].m_Type = FXPT_TYPE::BezierTo;
  pPoints[8].m_CloseFigure = false;
  pPoints[9].m_PointX = 12;
  pPoints[9].m_PointY = 13.64f;
  pPoints[9].m_Type = FXPT_TYPE::BezierTo;
  pPoints[9].m_CloseFigure = true;
  pPathObj->m_FillType = FXFILL_WINDING;
  pPathObj->m_bStroke = false;
  auto pTestPage = pdfium::MakeUnique<CPDF_Page>(nullptr, nullptr, false);
  CPDF_PageContentGenerator generator(pTestPage.get());
  CFX_ByteTextBuf buf;
  TestProcessPath(&generator, &buf, pPathObj.get());
  EXPECT_EQ(
      "q 3.102 4.67 m 5.45 0.29 l 4.24 3.15 4.65 2.98 3.456 0.24 c 10.6 11.15 "
      "l 11 12.5 l 11.46 12.67 11.84 12.96 12 13.64 c h f Q\n",
      buf.MakeString());
}

TEST_F(CPDF_PageContentGeneratorTest, ProcessGraphics) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->m_Path.SetPointCount(3);
  FX_PATHPOINT* pPoints = pPathObj->m_Path.GetMutablePoints();
  pPoints[0].m_PointX = 1;
  pPoints[0].m_PointY = 2;
  pPoints[0].m_Type = FXPT_TYPE::MoveTo;
  pPoints[0].m_CloseFigure = false;
  pPoints[1].m_PointX = 3;
  pPoints[1].m_PointY = 4;
  pPoints[1].m_Type = FXPT_TYPE::LineTo;
  pPoints[1].m_CloseFigure = false;
  pPoints[2].m_PointX = 5;
  pPoints[2].m_PointY = 6;
  pPoints[2].m_Type = FXPT_TYPE::LineTo;
  pPoints[2].m_CloseFigure = true;
  pPathObj->m_FillType = FXFILL_WINDING;
  pPathObj->m_bStroke = true;
  FX_FLOAT rgb[3] = {0.5f, 0.7f, 0.35f};
  CPDF_ColorSpace* pCS = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB);
  pPathObj->m_ColorState.SetFillColor(pCS, rgb, 3);
  FX_FLOAT rgb2[3] = {1, 0.9f, 0};
  pPathObj->m_ColorState.SetStrokeColor(pCS, rgb2, 3);
  pPathObj->m_GeneralState.SetFillAlpha(0.5f);
  pPathObj->m_GeneralState.SetStrokeAlpha(0.8f);
  auto pDoc = pdfium::MakeUnique<CPDF_Document>(nullptr);
  pDoc->CreateNewDoc();
  CPDF_Dictionary* pPageDict = pDoc->CreateNewPage(0);
  auto pTestPage = pdfium::MakeUnique<CPDF_Page>(pDoc.get(), pPageDict, false);
  CPDF_PageContentGenerator generator(pTestPage.get());
  CFX_ByteTextBuf buf;
  TestProcessPath(&generator, &buf, pPathObj.get());
  CFX_ByteString pathString = buf.MakeString();
  // Color RGB values used are integers divided by 255.
  EXPECT_EQ("q 0.501961 0.701961 0.34902 rg 1 0.901961 0 RG /",
            pathString.Left(48));
  EXPECT_EQ(" gs 1 2 m 3 4 l 5 6 l h B Q\n", pathString.Right(28));
  ASSERT_TRUE(pathString.GetLength() > 76);
  CPDF_Dictionary* externalGS = TestGetResource(
      &generator, "ExtGState", pathString.Mid(48, pathString.GetLength() - 76));
  ASSERT_TRUE(externalGS);
  EXPECT_EQ(0.5f, externalGS->GetNumberFor("ca"));
  EXPECT_EQ(0.8f, externalGS->GetNumberFor("CA"));

  // Same path, now with a stroke.
  pPathObj->m_GraphState.SetLineWidth(10.5f);
  buf.Clear();
  TestProcessPath(&generator, &buf, pPathObj.get());
  CFX_ByteString pathString2 = buf.MakeString();
  EXPECT_EQ("q 0.501961 0.701961 0.34902 rg 1 0.901961 0 RG 10.5 w /",
            pathString2.Left(55));
  EXPECT_EQ(" gs 1 2 m 3 4 l 5 6 l h B Q\n", pathString2.Right(28));
  // Compare with the previous (should use same dictionary for gs)
  EXPECT_EQ(pathString.GetLength() + 7, pathString2.GetLength());
  EXPECT_EQ(pathString.Mid(48, pathString.GetLength() - 76),
            pathString2.Mid(55, pathString2.GetLength() - 83));
}

TEST_F(CPDF_PageContentGeneratorTest, ProcessText) {
  auto pDoc = pdfium::MakeUnique<CPDF_Document>(nullptr);
  pDoc->CreateNewDoc();
  CPDF_Dictionary* pPageDict = pDoc->CreateNewPage(0);
  auto pTestPage = pdfium::MakeUnique<CPDF_Page>(pDoc.get(), pPageDict, false);
  CPDF_PageContentGenerator generator(pTestPage.get());
  auto pTextObj = pdfium::MakeUnique<CPDF_TextObject>();
  CPDF_Font* pFont = CPDF_Font::GetStockFont(pDoc.get(), "Times-Roman");
  pTextObj->m_TextState.SetFont(pFont);
  pTextObj->m_TextState.SetFontSize(10.0f);
  pTextObj->Transform(CFX_Matrix(1, 0, 0, 1, 100, 100));
  pTextObj->SetText("Hello World");
  CFX_ByteTextBuf buf;
  TestProcessText(&generator, &buf, pTextObj.get());
  CFX_ByteString textString = buf.MakeString();
  EXPECT_LT(61, textString.GetLength());
  EXPECT_EQ("BT 1 0 0 1 100 100 Tm /", textString.Left(23));
  EXPECT_EQ(" 10 Tf <48656C6C6F20576F726C64> Tj ET\n", textString.Right(38));
  CPDF_Dictionary* fontDict = TestGetResource(
      &generator, "Font", textString.Mid(23, textString.GetLength() - 61));
  ASSERT_TRUE(fontDict);
  EXPECT_EQ("Font", fontDict->GetStringFor("Type"));
  EXPECT_EQ("Type1", fontDict->GetStringFor("Subtype"));
  EXPECT_EQ("Times-Roman", fontDict->GetStringFor("BaseFont"));
}
