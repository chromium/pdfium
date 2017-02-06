// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
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

  CPDF_Dictionary* TestGetGS(CPDF_PageContentGenerator* pGen,
                             const CFX_ByteString& name) {
    return pGen->m_pPage->m_pResources->GetDictFor("ExtGState")
        ->GetDictFor(name);
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
  pPoints[0].m_Flag = FXPT_MOVETO;
  pPoints[1].m_PointX = 5.2f;
  pPoints[1].m_PointY = 0;
  pPoints[1].m_Flag = FXPT_LINETO;
  pPoints[2].m_PointX = 5.2f;
  pPoints[2].m_PointY = 3.78f;
  pPoints[2].m_Flag = FXPT_LINETO;
  pPoints[3].m_PointX = 0;
  pPoints[3].m_PointY = 3.78f;
  pPoints[3].m_Flag = FXPT_LINETO | FXPT_CLOSEFIGURE;
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
  pPoints[0].m_Flag = FXPT_MOVETO;
  pPoints[1].m_PointX = 5.45f;
  pPoints[1].m_PointY = 0.29f;
  pPoints[1].m_Flag = FXPT_LINETO;
  pPoints[2].m_PointX = 4.24f;
  pPoints[2].m_PointY = 3.15f;
  pPoints[2].m_Flag = FXPT_BEZIERTO;
  pPoints[3].m_PointX = 4.65f;
  pPoints[3].m_PointY = 2.98f;
  pPoints[3].m_Flag = FXPT_BEZIERTO;
  pPoints[4].m_PointX = 3.456f;
  pPoints[4].m_PointY = 0.24f;
  pPoints[4].m_Flag = FXPT_BEZIERTO;
  pPoints[5].m_PointX = 10.6f;
  pPoints[5].m_PointY = 11.15f;
  pPoints[5].m_Flag = FXPT_LINETO;
  pPoints[6].m_PointX = 11;
  pPoints[6].m_PointY = 12.5f;
  pPoints[6].m_Flag = FXPT_LINETO;
  pPoints[7].m_PointX = 11.46f;
  pPoints[7].m_PointY = 12.67f;
  pPoints[7].m_Flag = FXPT_BEZIERTO;
  pPoints[8].m_PointX = 11.84f;
  pPoints[8].m_PointY = 12.96f;
  pPoints[8].m_Flag = FXPT_BEZIERTO;
  pPoints[9].m_PointX = 12;
  pPoints[9].m_PointY = 13.64f;
  pPoints[9].m_Flag = FXPT_BEZIERTO | FXPT_CLOSEFIGURE;
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
  pPoints[0].m_Flag = FXPT_MOVETO;
  pPoints[1].m_PointX = 3;
  pPoints[1].m_PointY = 4;
  pPoints[1].m_Flag = FXPT_LINETO;
  pPoints[2].m_PointX = 5;
  pPoints[2].m_PointY = 6;
  pPoints[2].m_Flag = FXPT_LINETO | FXPT_CLOSEFIGURE;
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
  CPDF_Dictionary* externalGS =
      TestGetGS(&generator, pathString.Mid(48, pathString.GetLength() - 76));
  ASSERT_TRUE(externalGS);
  EXPECT_EQ(0.5f, externalGS->GetNumberFor("ca"));
  EXPECT_EQ(0.8f, externalGS->GetNumberFor("CA"));
}
