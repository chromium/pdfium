// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/ptr_util.h"

TEST(cpdf_pagecontentgenerator, ProcessRect) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->m_Path.AppendRect(10, 5, 13, 30);
  pPathObj->m_FillType = FXFILL_ALTERNATE;
  pPathObj->m_bStroke = true;
  auto pTestPage = pdfium::MakeUnique<CPDF_Page>(nullptr, nullptr, false);
  CPDF_PageContentGenerator generator(pTestPage.get());
  CFX_ByteTextBuf buf;
  generator.ProcessPath(&buf, pPathObj.get());
  EXPECT_EQ("10 5 3 25 re B*\n", buf.MakeString());

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
  generator.ProcessPath(&buf, pPathObj.get());
  EXPECT_EQ("0 0 5.2 3.78 re n\n", buf.MakeString());
}

TEST(cpdf_pagecontentgenerator, ProcessPath) {
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
  generator.ProcessPath(&buf, pPathObj.get());
  EXPECT_EQ(
      "3.102 4.67 m 5.45 0.29 l 4.24 3.15 4.65 2.98 3.456 0.24 c 10.6 11.15 l "
      "11 12.5 l 11.46 12.67 11.84 12.96 12 13.64 c h f\n",
      buf.MakeString());
}
