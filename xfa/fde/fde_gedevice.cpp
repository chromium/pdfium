// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/fde_gedevice.h"

#include <algorithm>

#include "xfa/fde/fde_brush.h"
#include "xfa/fde/fde_geobject.h"
#include "xfa/fde/fde_image.h"
#include "xfa/fde/fde_object.h"
#include "xfa/fde/fde_pen.h"

IFDE_RenderDevice* IFDE_RenderDevice::Create(CFX_DIBitmap* pBitmap,
                                             FX_BOOL bRgbByteOrder) {
  if (pBitmap == NULL) {
    return NULL;
  }
  CFX_FxgeDevice* pDevice = new CFX_FxgeDevice;
  pDevice->Attach(pBitmap, 0, bRgbByteOrder);
  return new CFDE_FxgeDevice(pDevice, TRUE);
}
IFDE_RenderDevice* IFDE_RenderDevice::Create(CFX_RenderDevice* pDevice) {
  return pDevice ? new CFDE_FxgeDevice(pDevice, FALSE) : nullptr;
}
CFDE_FxgeDevice::CFDE_FxgeDevice(CFX_RenderDevice* pDevice,
                                 FX_BOOL bOwnerDevice)
    : m_pDevice(pDevice),
      m_bOwnerDevice(bOwnerDevice),
      m_pCharPos(NULL),
      m_iCharCount(0) {
  FXSYS_assert(pDevice != NULL);
  FX_RECT rt = m_pDevice->GetClipBox();
  m_rtClip.Set((FX_FLOAT)rt.left, (FX_FLOAT)rt.top, (FX_FLOAT)rt.Width(),
               (FX_FLOAT)rt.Height());
}
CFDE_FxgeDevice::~CFDE_FxgeDevice() {
  FX_Free(m_pCharPos);
  if (m_bOwnerDevice)
    delete m_pDevice;
}
int32_t CFDE_FxgeDevice::GetWidth() const {
  return m_pDevice->GetWidth();
}
int32_t CFDE_FxgeDevice::GetHeight() const {
  return m_pDevice->GetHeight();
}
FDE_HDEVICESTATE CFDE_FxgeDevice::SaveState() {
  m_pDevice->SaveState();
  return NULL;
}
void CFDE_FxgeDevice::RestoreState(FDE_HDEVICESTATE hState) {
  m_pDevice->RestoreState();
  const FX_RECT& rt = m_pDevice->GetClipBox();
  m_rtClip.Set((FX_FLOAT)rt.left, (FX_FLOAT)rt.top, (FX_FLOAT)rt.Width(),
               (FX_FLOAT)rt.Height());
}
FX_BOOL CFDE_FxgeDevice::SetClipRect(const CFX_RectF& rtClip) {
  m_rtClip = rtClip;
  return m_pDevice->SetClip_Rect(FX_RECT((int32_t)FXSYS_floor(rtClip.left),
                                         (int32_t)FXSYS_floor(rtClip.top),
                                         (int32_t)FXSYS_ceil(rtClip.right()),
                                         (int32_t)FXSYS_ceil(rtClip.bottom())));
}
const CFX_RectF& CFDE_FxgeDevice::GetClipRect() {
  return m_rtClip;
}
FX_BOOL CFDE_FxgeDevice::SetClipPath(const IFDE_Path* pClip) {
  return FALSE;
}
IFDE_Path* CFDE_FxgeDevice::GetClipPath() const {
  return NULL;
}
FX_FLOAT CFDE_FxgeDevice::GetDpiX() const {
  return 96;
}
FX_FLOAT CFDE_FxgeDevice::GetDpiY() const {
  return 96;
}
FX_BOOL CFDE_FxgeDevice::DrawImage(CFX_DIBSource* pDib,
                                   const CFX_RectF* pSrcRect,
                                   const CFX_RectF& dstRect,
                                   const CFX_Matrix* pImgMatrix,
                                   const CFX_Matrix* pDevMatrix) {
  FXSYS_assert(pDib != NULL);
  CFX_RectF srcRect;
  if (pSrcRect) {
    srcRect = *pSrcRect;
  } else {
    srcRect.Set(0, 0, (FX_FLOAT)pDib->GetWidth(), (FX_FLOAT)pDib->GetHeight());
  }
  if (srcRect.IsEmpty()) {
    return FALSE;
  }
  CFX_Matrix dib2fxdev;
  if (pImgMatrix) {
    dib2fxdev = *pImgMatrix;
  } else {
    dib2fxdev.SetIdentity();
  }
  dib2fxdev.a = dstRect.width;
  dib2fxdev.d = -dstRect.height;
  dib2fxdev.e = dstRect.left;
  dib2fxdev.f = dstRect.bottom();
  if (pDevMatrix) {
    dib2fxdev.Concat(*pDevMatrix);
  }
  void* handle = NULL;
  m_pDevice->StartDIBits(pDib, 255, 0, (const CFX_Matrix*)&dib2fxdev, 0,
                         handle);
  while (m_pDevice->ContinueDIBits(handle, NULL)) {
  }
  m_pDevice->CancelDIBits(handle);
  return handle != NULL;
}
FX_BOOL CFDE_FxgeDevice::DrawString(IFDE_Brush* pBrush,
                                    IFX_Font* pFont,
                                    const FXTEXT_CHARPOS* pCharPos,
                                    int32_t iCount,
                                    FX_FLOAT fFontSize,
                                    const CFX_Matrix* pMatrix) {
  FXSYS_assert(pBrush != NULL && pFont != NULL && pCharPos != NULL &&
               iCount > 0);
  CFX_FontCache* pCache = CFX_GEModule::Get()->GetFontCache();
  CFX_Font* pFxFont = (CFX_Font*)pFont->GetDevFont();
  switch (pBrush->GetType()) {
    case FDE_BRUSHTYPE_Solid: {
      FX_ARGB argb = ((IFDE_SolidBrush*)pBrush)->GetColor();
      if ((pFont->GetFontStyles() & FX_FONTSTYLE_Italic) != 0 &&
          !pFxFont->IsItalic()) {
        FXTEXT_CHARPOS* pCP = (FXTEXT_CHARPOS*)pCharPos;
        FX_FLOAT* pAM;
        for (int32_t i = 0; i < iCount; ++i) {
          static const FX_FLOAT mc = 0.267949f;
          pAM = pCP->m_AdjustMatrix;
          pAM[2] = mc * pAM[0] + pAM[2];
          pAM[3] = mc * pAM[1] + pAM[3];
          pCP++;
        }
      }
      FXTEXT_CHARPOS* pCP = (FXTEXT_CHARPOS*)pCharPos;
      IFX_Font* pCurFont = NULL;
      IFX_Font* pSTFont = NULL;
      FXTEXT_CHARPOS* pCurCP = NULL;
      int32_t iCurCount = 0;
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
      uint32_t dwFontStyle = pFont->GetFontStyles();
      CFX_Font FxFont;
      CFX_SubstFont SubstFxFont;
      FxFont.SetSubstFont(&SubstFxFont);
      SubstFxFont.m_Weight = dwFontStyle & FX_FONTSTYLE_Bold ? 700 : 400;
      SubstFxFont.m_WeightCJK = SubstFxFont.m_Weight;
      SubstFxFont.m_ItalicAngle = dwFontStyle & FX_FONTSTYLE_Italic ? -12 : 0;
      SubstFxFont.m_bItlicCJK = !!(dwFontStyle & FX_FONTSTYLE_Italic);
#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
      for (int32_t i = 0; i < iCount; ++i) {
        pSTFont = pFont->GetSubstFont((int32_t)pCP->m_GlyphIndex);
        pCP->m_GlyphIndex &= 0x00FFFFFF;
        pCP->m_bFontStyle = FALSE;
        if (pCurFont != pSTFont) {
          if (pCurFont != NULL) {
            pFxFont = (CFX_Font*)pCurFont->GetDevFont();
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
            FxFont.SetFace(pFxFont->GetFace());
            m_pDevice->DrawNormalText(iCurCount, pCurCP, &FxFont, pCache,
                                      -fFontSize, (const CFX_Matrix*)pMatrix,
                                      argb, FXTEXT_CLEARTYPE);
#else
            m_pDevice->DrawNormalText(iCurCount, pCurCP, pFxFont, pCache,
                                      -fFontSize, (const CFX_Matrix*)pMatrix,
                                      argb, FXTEXT_CLEARTYPE);
#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
          }
          pCurFont = pSTFont;
          pCurCP = pCP;
          iCurCount = 1;
        } else {
          iCurCount++;
        }
        pCP++;
      }
      if (pCurFont != NULL && iCurCount) {
        pFxFont = (CFX_Font*)pCurFont->GetDevFont();
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
        FxFont.SetFace(pFxFont->GetFace());
        FX_BOOL bRet = m_pDevice->DrawNormalText(
            iCurCount, pCurCP, &FxFont, pCache, -fFontSize,
            (const CFX_Matrix*)pMatrix, argb, FXTEXT_CLEARTYPE);
        FxFont.SetSubstFont(nullptr);
        FxFont.SetFace(nullptr);
        return bRet;
#else
        return m_pDevice->DrawNormalText(iCurCount, pCurCP, pFxFont, pCache,
                                         -fFontSize, (const CFX_Matrix*)pMatrix,
                                         argb, FXTEXT_CLEARTYPE);
#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
      }
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
      FxFont.SetSubstFont(nullptr);
      FxFont.SetFace(nullptr);
#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
      return TRUE;
    } break;
    default:
      return FALSE;
  }
}
FX_BOOL CFDE_FxgeDevice::DrawBezier(IFDE_Pen* pPen,
                                    FX_FLOAT fPenWidth,
                                    const CFX_PointF& pt1,
                                    const CFX_PointF& pt2,
                                    const CFX_PointF& pt3,
                                    const CFX_PointF& pt4,
                                    const CFX_Matrix* pMatrix) {
  CFX_PointsF points;
  points.Add(pt1);
  points.Add(pt2);
  points.Add(pt3);
  points.Add(pt4);
  CFDE_Path path;
  path.AddBezier(points);
  return DrawPath(pPen, fPenWidth, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::DrawCurve(IFDE_Pen* pPen,
                                   FX_FLOAT fPenWidth,
                                   const CFX_PointsF& points,
                                   FX_BOOL bClosed,
                                   FX_FLOAT fTension,
                                   const CFX_Matrix* pMatrix) {
  CFDE_Path path;
  path.AddCurve(points, bClosed, fTension);
  return DrawPath(pPen, fPenWidth, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::DrawEllipse(IFDE_Pen* pPen,
                                     FX_FLOAT fPenWidth,
                                     const CFX_RectF& rect,
                                     const CFX_Matrix* pMatrix) {
  CFDE_Path path;
  path.AddEllipse(rect);
  return DrawPath(pPen, fPenWidth, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::DrawLines(IFDE_Pen* pPen,
                                   FX_FLOAT fPenWidth,
                                   const CFX_PointsF& points,
                                   const CFX_Matrix* pMatrix) {
  CFDE_Path path;
  path.AddLines(points);
  return DrawPath(pPen, fPenWidth, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::DrawLine(IFDE_Pen* pPen,
                                  FX_FLOAT fPenWidth,
                                  const CFX_PointF& pt1,
                                  const CFX_PointF& pt2,
                                  const CFX_Matrix* pMatrix) {
  CFDE_Path path;
  path.AddLine(pt1, pt2);
  return DrawPath(pPen, fPenWidth, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::DrawPath(IFDE_Pen* pPen,
                                  FX_FLOAT fPenWidth,
                                  const IFDE_Path* pPath,
                                  const CFX_Matrix* pMatrix) {
  CFDE_Path* pGePath = (CFDE_Path*)pPath;
  if (pGePath == NULL) {
    return FALSE;
  }
  CFX_GraphStateData graphState;
  if (!CreatePen(pPen, fPenWidth, graphState)) {
    return FALSE;
  }
  return m_pDevice->DrawPath(&pGePath->m_Path, (const CFX_Matrix*)pMatrix,
                             &graphState, 0, pPen->GetColor(), 0);
}
FX_BOOL CFDE_FxgeDevice::DrawPolygon(IFDE_Pen* pPen,
                                     FX_FLOAT fPenWidth,
                                     const CFX_PointsF& points,
                                     const CFX_Matrix* pMatrix) {
  CFDE_Path path;
  path.AddPolygon(points);
  return DrawPath(pPen, fPenWidth, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::DrawRectangle(IFDE_Pen* pPen,
                                       FX_FLOAT fPenWidth,
                                       const CFX_RectF& rect,
                                       const CFX_Matrix* pMatrix) {
  CFDE_Path path;
  path.AddRectangle(rect);
  return DrawPath(pPen, fPenWidth, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::FillClosedCurve(IFDE_Brush* pBrush,
                                         const CFX_PointsF& points,
                                         FX_FLOAT fTension,
                                         const CFX_Matrix* pMatrix) {
  CFDE_Path path;
  path.AddCurve(points, TRUE, fTension);
  return FillPath(pBrush, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::FillEllipse(IFDE_Brush* pBrush,
                                     const CFX_RectF& rect,
                                     const CFX_Matrix* pMatrix) {
  CFDE_Path path;
  path.AddEllipse(rect);
  return FillPath(pBrush, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::FillPolygon(IFDE_Brush* pBrush,
                                     const CFX_PointsF& points,
                                     const CFX_Matrix* pMatrix) {
  CFDE_Path path;
  path.AddPolygon(points);
  return FillPath(pBrush, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::FillRectangle(IFDE_Brush* pBrush,
                                       const CFX_RectF& rect,
                                       const CFX_Matrix* pMatrix) {
  CFDE_Path path;
  path.AddRectangle(rect);
  return FillPath(pBrush, &path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::CreatePen(IFDE_Pen* pPen,
                                   FX_FLOAT fPenWidth,
                                   CFX_GraphStateData& graphState) {
  if (pPen == NULL) {
    return FALSE;
  }
  graphState.m_LineCap = (CFX_GraphStateData::LineCap)pPen->GetLineCap();
  graphState.m_LineJoin = (CFX_GraphStateData::LineJoin)pPen->GetLineJoin();
  graphState.m_LineWidth = fPenWidth;
  graphState.m_MiterLimit = pPen->GetMiterLimit();
  graphState.m_DashPhase = pPen->GetDashPhase();
  CFX_FloatArray dashArray;
  switch (pPen->GetDashStyle()) {
    case FDE_DASHSTYLE_Dash:
      dashArray.Add(3);
      dashArray.Add(1);
      break;
    case FDE_DASHSTYLE_Dot:
      dashArray.Add(1);
      dashArray.Add(1);
      break;
    case FDE_DASHSTYLE_DashDot:
      dashArray.Add(3);
      dashArray.Add(1);
      dashArray.Add(1);
      dashArray.Add(1);
      break;
    case FDE_DASHSTYLE_DashDotDot:
      dashArray.Add(3);
      dashArray.Add(1);
      dashArray.Add(1);
      dashArray.Add(1);
      dashArray.Add(1);
      dashArray.Add(1);
      break;
    case FDE_DASHSTYLE_Customized:
      pPen->GetDashArray(dashArray);
      break;
  }
  int32_t iDashCount = dashArray.GetSize();
  if (iDashCount > 0) {
    graphState.SetDashCount(iDashCount);
    for (int32_t i = 0; i < iDashCount; ++i) {
      graphState.m_DashArray[i] = dashArray[i] * fPenWidth;
    }
  }
  return TRUE;
}

FX_BOOL CFDE_FxgeDevice::FillPath(IFDE_Brush* pBrush,
                                  const IFDE_Path* pPath,
                                  const CFX_Matrix* pMatrix) {
  CFDE_Path* pGePath = (CFDE_Path*)pPath;
  if (!pGePath)
    return FALSE;

  if (!pBrush)
    return FALSE;

  return FillSolidPath(pBrush, &pGePath->m_Path, pMatrix);
}

FX_BOOL CFDE_FxgeDevice::FillSolidPath(IFDE_Brush* pBrush,
                                       const CFX_PathData* pPath,
                                       const CFX_Matrix* pMatrix) {
  FXSYS_assert(pPath && pBrush && pBrush->GetType() == FDE_BRUSHTYPE_Solid);

  IFDE_SolidBrush* pSolidBrush = static_cast<IFDE_SolidBrush*>(pBrush);
  return m_pDevice->DrawPath(pPath, (const CFX_Matrix*)pMatrix, NULL,
                             pSolidBrush->GetColor(), 0, FXFILL_WINDING);
}
