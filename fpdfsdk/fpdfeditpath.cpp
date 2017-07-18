// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_edit.h"

#include "core/fpdfapi/page/cpdf_path.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fxcrt/fx_system.h"
#include "fpdfsdk/fsdk_define.h"
#include "third_party/base/ptr_util.h"

// These checks are here because core/ and public/ cannot depend on each other.
static_assert(CFX_GraphStateData::LineCapButt == FPDF_LINECAP_BUTT,
              "CFX_GraphStateData::LineCapButt value mismatch");
static_assert(CFX_GraphStateData::LineCapRound == FPDF_LINECAP_ROUND,
              "CFX_GraphStateData::LineCapRound value mismatch");
static_assert(CFX_GraphStateData::LineCapSquare ==
                  FPDF_LINECAP_PROJECTING_SQUARE,
              "CFX_GraphStateData::LineCapSquare value mismatch");

static_assert(CFX_GraphStateData::LineJoinMiter == FPDF_LINEJOIN_MITER,
              "CFX_GraphStateData::LineJoinMiter value mismatch");
static_assert(CFX_GraphStateData::LineJoinRound == FPDF_LINEJOIN_ROUND,
              "CFX_GraphStateData::LineJoinRound value mismatch");
static_assert(CFX_GraphStateData::LineJoinBevel == FPDF_LINEJOIN_BEVEL,
              "CFX_GraphStateData::LineJoinBevel value mismatch");

DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_CreateNewPath(float x, float y) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->m_Path.AppendPoint(CFX_PointF(x, y), FXPT_TYPE::MoveTo, false);
  pPathObj->DefaultStates();
  return pPathObj.release();  // Caller takes ownership.
}

DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPageObj_CreateNewRect(float x,
                                                            float y,
                                                            float w,
                                                            float h) {
  auto pPathObj = pdfium::MakeUnique<CPDF_PathObject>();
  pPathObj->m_Path.AppendRect(x, y, x + w, y + h);
  pPathObj->DefaultStates();
  return pPathObj.release();  // Caller takes ownership.
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPath_SetStrokeColor(FPDF_PAGEOBJECT path,
                                                    unsigned int R,
                                                    unsigned int G,
                                                    unsigned int B,
                                                    unsigned int A) {
  if (!path || R > 255 || G > 255 || B > 255 || A > 255)
    return false;

  float rgb[3] = {R / 255.f, G / 255.f, B / 255.f};
  auto* pPathObj = CPDFPathObjectFromFPDFPageObject(path);
  pPathObj->m_GeneralState.SetStrokeAlpha(A / 255.f);
  pPathObj->m_ColorState.SetStrokeColor(
      CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB), rgb, 3);
  pPathObj->SetDirty(true);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPath_GetStrokeColor(FPDF_PAGEOBJECT path,
                                                    unsigned int* R,
                                                    unsigned int* G,
                                                    unsigned int* B,
                                                    unsigned int* A) {
  if (!path || !R || !G || !B || !A)
    return false;

  auto* pPathObj = CPDFPathObjectFromFPDFPageObject(path);
  uint32_t strokeRGB = pPathObj->m_ColorState.GetStrokeRGB();
  *R = FXSYS_GetRValue(strokeRGB);
  *G = FXSYS_GetGValue(strokeRGB);
  *B = FXSYS_GetBValue(strokeRGB);
  *A = static_cast<unsigned int>(pPathObj->m_GeneralState.GetStrokeAlpha() *
                                 255.f);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPath_SetStrokeWidth(FPDF_PAGEOBJECT path,
                                                    float width) {
  if (!path || width < 0.0f)
    return false;

  auto* pPathObj = CPDFPathObjectFromFPDFPageObject(path);
  pPathObj->m_GraphState.SetLineWidth(width);
  pPathObj->SetDirty(true);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPath_SetFillColor(FPDF_PAGEOBJECT path,
                                                  unsigned int R,
                                                  unsigned int G,
                                                  unsigned int B,
                                                  unsigned int A) {
  return FPDFPageObj_SetFillColor(path, R, G, B, A);
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPath_GetFillColor(FPDF_PAGEOBJECT path,
                                                  unsigned int* R,
                                                  unsigned int* G,
                                                  unsigned int* B,
                                                  unsigned int* A) {
  if (!path || !R || !G || !B || !A)
    return false;

  auto* pPathObj = CPDFPathObjectFromFPDFPageObject(path);
  uint32_t fillRGB = pPathObj->m_ColorState.GetFillRGB();
  *R = FXSYS_GetRValue(fillRGB);
  *G = FXSYS_GetGValue(fillRGB);
  *B = FXSYS_GetBValue(fillRGB);
  *A = static_cast<unsigned int>(pPathObj->m_GeneralState.GetFillAlpha() *
                                 255.f);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPath_MoveTo(FPDF_PAGEOBJECT path,
                                            float x,
                                            float y) {
  if (!path)
    return false;

  auto* pPathObj = CPDFPathObjectFromFPDFPageObject(path);
  pPathObj->m_Path.AppendPoint(CFX_PointF(x, y), FXPT_TYPE::MoveTo, false);
  pPathObj->SetDirty(true);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPath_LineTo(FPDF_PAGEOBJECT path,
                                            float x,
                                            float y) {
  if (!path)
    return false;

  auto* pPathObj = CPDFPathObjectFromFPDFPageObject(path);
  pPathObj->m_Path.AppendPoint(CFX_PointF(x, y), FXPT_TYPE::LineTo, false);
  pPathObj->SetDirty(true);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPath_BezierTo(FPDF_PAGEOBJECT path,
                                              float x1,
                                              float y1,
                                              float x2,
                                              float y2,
                                              float x3,
                                              float y3) {
  if (!path)
    return false;

  auto* pPathObj = CPDFPathObjectFromFPDFPageObject(path);
  pPathObj->m_Path.AppendPoint(CFX_PointF(x1, y1), FXPT_TYPE::BezierTo, false);
  pPathObj->m_Path.AppendPoint(CFX_PointF(x2, y2), FXPT_TYPE::BezierTo, false);
  pPathObj->m_Path.AppendPoint(CFX_PointF(x3, y3), FXPT_TYPE::BezierTo, false);
  pPathObj->SetDirty(true);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPath_Close(FPDF_PAGEOBJECT path) {
  if (!path)
    return false;

  auto* pPathObj = CPDFPathObjectFromFPDFPageObject(path);
  if (pPathObj->m_Path.GetPoints().empty())
    return false;

  pPathObj->m_Path.ClosePath();
  pPathObj->SetDirty(true);
  return true;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPath_SetDrawMode(FPDF_PAGEOBJECT path,
                                                 int fillmode,
                                                 FPDF_BOOL stroke) {
  if (!path)
    return false;

  auto* pPathObj = CPDFPathObjectFromFPDFPageObject(path);

  if (fillmode == FPDF_FILLMODE_ALTERNATE)
    pPathObj->m_FillType = FXFILL_ALTERNATE;
  else if (fillmode == FPDF_FILLMODE_WINDING)
    pPathObj->m_FillType = FXFILL_WINDING;
  else
    pPathObj->m_FillType = 0;
  pPathObj->m_bStroke = stroke != 0;
  pPathObj->SetDirty(true);
  return true;
}

DLLEXPORT void STDCALL FPDFPath_SetLineJoin(FPDF_PAGEOBJECT path,
                                            int line_join) {
  if (!path)
    return;
  if (line_join <
          static_cast<int>(CFX_GraphStateData::LineJoin::LineJoinMiter) ||
      line_join >
          static_cast<int>(CFX_GraphStateData::LineJoin::LineJoinBevel)) {
    return;
  }
  auto* pPathObj = CPDFPageObjectFromFPDFPageObject(path);
  CFX_GraphStateData::LineJoin lineJoin =
      static_cast<CFX_GraphStateData::LineJoin>(line_join);
  pPathObj->m_GraphState.SetLineJoin(lineJoin);
  pPathObj->SetDirty(true);
}

DLLEXPORT void STDCALL FPDFPath_SetLineCap(FPDF_PAGEOBJECT path, int line_cap) {
  if (!path)
    return;
  if (line_cap < static_cast<int>(CFX_GraphStateData::LineCap::LineCapButt) ||
      line_cap > static_cast<int>(CFX_GraphStateData::LineCap::LineCapSquare)) {
    return;
  }
  auto* pPathObj = CPDFPageObjectFromFPDFPageObject(path);
  CFX_GraphStateData::LineCap lineCap =
      static_cast<CFX_GraphStateData::LineCap>(line_cap);
  pPathObj->m_GraphState.SetLineCap(lineCap);
  pPathObj->SetDirty(true);
}
