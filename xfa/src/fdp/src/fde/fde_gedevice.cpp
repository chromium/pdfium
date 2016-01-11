// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/foxitlib.h"
#include "fde_gedevice.h"
#include "fde_geobject.h"
#include "fde_devbasic.h"
#ifndef _FDEPLUS
#ifdef _cplusplus
exten "C" {
#endif
  FX_BOOL FDE_GetStockHatchMask(int32_t iHatchStyle, CFX_DIBitmap & hatchMask) {
    FDE_LPCHATCHDATA pData = FDE_DEVGetHatchData(iHatchStyle);
    if (!pData) {
      return FALSE;
    }
    hatchMask.Create(pData->iWidth, pData->iHeight, FXDIB_1bppMask);
    FXSYS_memcpy(hatchMask.GetBuffer(), pData->MaskBits,
                 hatchMask.GetPitch() * pData->iHeight);
    return TRUE;
  }
#ifdef _cplusplus
}
#endif
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
  FX_RECT rt((int32_t)FXSYS_floor(rtClip.left),
             (int32_t)FXSYS_floor(rtClip.top),
             (int32_t)FXSYS_ceil(rtClip.right()),
             (int32_t)FXSYS_ceil(rtClip.bottom()));
  return m_pDevice->SetClip_Rect(&rt);
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
      FX_DWORD dwFontStyle = pFont->GetFontStyles();
      CFX_Font FxFont;
      CFX_SubstFont SubstFxFont;
      FxFont.SetSubstFont(&SubstFxFont);
      SubstFxFont.m_Weight = dwFontStyle & FX_FONTSTYLE_Bold ? 700 : 400;
      SubstFxFont.m_WeightCJK = SubstFxFont.m_Weight;
      SubstFxFont.m_ItalicAngle = dwFontStyle & FX_FONTSTYLE_Italic ? -12 : 0;
      SubstFxFont.m_bItlicCJK = !!(dwFontStyle & FX_FONTSTYLE_Italic);
#endif
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
#endif
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
#endif
      }
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
      FxFont.SetSubstFont(nullptr);
      FxFont.SetFace(nullptr);
#endif
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
typedef FX_BOOL (CFDE_FxgeDevice::*pfFillPath)(IFDE_Brush* pBrush,
                                               const CFX_PathData* pPath,
                                               const CFX_Matrix* pMatrix);
static const pfFillPath gs_FillPath[] = {
    &CFDE_FxgeDevice::FillSolidPath, &CFDE_FxgeDevice::FillHatchPath,
    &CFDE_FxgeDevice::FillTexturePath, &CFDE_FxgeDevice::FillLinearGradientPath,
};
FX_BOOL CFDE_FxgeDevice::FillPath(IFDE_Brush* pBrush,
                                  const IFDE_Path* pPath,
                                  const CFX_Matrix* pMatrix) {
  CFDE_Path* pGePath = (CFDE_Path*)pPath;
  if (pGePath == NULL) {
    return FALSE;
  }
  if (pBrush == NULL) {
    return FALSE;
  }
  int32_t iType = pBrush->GetType();
  if (iType < 0 || iType > FDE_BRUSHTYPE_MAX) {
    return FALSE;
  }
  return (this->*gs_FillPath[iType])(pBrush, &pGePath->m_Path, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::FillSolidPath(IFDE_Brush* pBrush,
                                       const CFX_PathData* pPath,
                                       const CFX_Matrix* pMatrix) {
  FXSYS_assert(pPath && pBrush && pBrush->GetType() == FDE_BRUSHTYPE_Solid);
  IFDE_SolidBrush* pSolidBrush = (IFDE_SolidBrush*)pBrush;
  return m_pDevice->DrawPath(pPath, (const CFX_Matrix*)pMatrix, NULL,
                             pSolidBrush->GetColor(), 0, FXFILL_WINDING);
}
FX_BOOL CFDE_FxgeDevice::FillHatchPath(IFDE_Brush* pBrush,
                                       const CFX_PathData* pPath,
                                       const CFX_Matrix* pMatrix) {
  FXSYS_assert(pPath && pBrush && pBrush->GetType() == FDE_BRUSHTYPE_Hatch);
  IFDE_HatchBrush* pHatchBrush = (IFDE_HatchBrush*)pBrush;
  int32_t iStyle = pHatchBrush->GetHatchStyle();
  if (iStyle < FDE_HATCHSTYLE_Min || iStyle > FDE_HATCHSTYLE_Max) {
    return FALSE;
  }
  CFX_DIBitmap mask;
  if (!FDE_GetStockHatchMask(iStyle, mask)) {
    return FALSE;
  }
  FX_ARGB dwForeColor = pHatchBrush->GetColor(TRUE);
  FX_ARGB dwBackColor = pHatchBrush->GetColor(FALSE);
  CFX_FloatRect rectf = pPath->GetBoundingBox();
  if (pMatrix) {
    rectf.Transform((const CFX_Matrix*)pMatrix);
  }
  FX_RECT rect(FXSYS_round(rectf.left), FXSYS_round(rectf.top),
               FXSYS_round(rectf.right), FXSYS_round(rectf.bottom));
  m_pDevice->SaveState();
  m_pDevice->StartRendering();
  m_pDevice->SetClip_PathFill(pPath, (const CFX_Matrix*)pMatrix,
                              FXFILL_WINDING);
  m_pDevice->FillRect(&rect, dwBackColor);
  for (int32_t j = rect.bottom; j < rect.top; j += mask.GetHeight())
    for (int32_t i = rect.left; i < rect.right; i += mask.GetWidth()) {
      m_pDevice->SetBitMask(&mask, i, j, dwForeColor);
    }
  m_pDevice->EndRendering();
  m_pDevice->RestoreState();
  return TRUE;
}
FX_BOOL CFDE_FxgeDevice::FillTexturePath(IFDE_Brush* pBrush,
                                         const CFX_PathData* pPath,
                                         const CFX_Matrix* pMatrix) {
  FXSYS_assert(pPath && pBrush && pBrush->GetType() == FDE_BRUSHTYPE_Texture);
  IFDE_TextureBrush* pTextureBrush = (IFDE_TextureBrush*)pBrush;
  IFDE_Image* pImage = (IFDE_Image*)pTextureBrush->GetImage();
  if (pImage == NULL) {
    return FALSE;
  }
  CFX_Size size;
  size.Set(pImage->GetImageWidth(), pImage->GetImageHeight());
  CFX_DIBitmap bmp;
  bmp.Create(size.x, size.y, FXDIB_Argb);
  if (!pImage->StartLoadImage(&bmp, 0, 0, size.x, size.y, 0, 0, size.x,
                              size.y)) {
    return FALSE;
  }
  if (pImage->DoLoadImage() < 100) {
    return FALSE;
  }
  pImage->StopLoadImage();
  return WrapTexture(pTextureBrush->GetWrapMode(), &bmp, pPath, pMatrix);
}
FX_BOOL CFDE_FxgeDevice::WrapTexture(int32_t iWrapMode,
                                     const CFX_DIBitmap* pBitmap,
                                     const CFX_PathData* pPath,
                                     const CFX_Matrix* pMatrix) {
  CFX_FloatRect rectf = pPath->GetBoundingBox();
  if (pMatrix) {
    rectf.Transform((const CFX_Matrix*)pMatrix);
  }
  FX_RECT rect(FXSYS_round(rectf.left), FXSYS_round(rectf.top),
               FXSYS_round(rectf.right), FXSYS_round(rectf.bottom));
  rect.Normalize();
  if (rect.IsEmpty()) {
    return FALSE;
  }
  m_pDevice->SaveState();
  m_pDevice->StartRendering();
  m_pDevice->SetClip_PathFill(pPath, (const CFX_Matrix*)pMatrix,
                              FXFILL_WINDING);
  switch (iWrapMode) {
    case FDE_WRAPMODE_Tile:
    case FDE_WRAPMODE_TileFlipX:
    case FDE_WRAPMODE_TileFlipY:
    case FDE_WRAPMODE_TileFlipXY: {
      FX_BOOL bFlipX = iWrapMode == FDE_WRAPMODE_TileFlipXY ||
                       iWrapMode == FDE_WRAPMODE_TileFlipX;
      FX_BOOL bFlipY = iWrapMode == FDE_WRAPMODE_TileFlipXY ||
                       iWrapMode == FDE_WRAPMODE_TileFlipY;
      const CFX_DIBitmap* pFlip[2][2];
      pFlip[0][0] = pBitmap;
      pFlip[0][1] = bFlipX ? pBitmap->FlipImage(TRUE, FALSE) : pBitmap;
      pFlip[1][0] = bFlipY ? pBitmap->FlipImage(FALSE, TRUE) : pBitmap;
      pFlip[1][1] =
          (bFlipX || bFlipY) ? pBitmap->FlipImage(bFlipX, bFlipY) : pBitmap;
      int32_t iCounterY = 0;
      for (int32_t j = rect.top; j < rect.bottom; j += pBitmap->GetHeight()) {
        int32_t indexY = iCounterY++ % 2;
        int32_t iCounterX = 0;
        for (int32_t i = rect.left; i < rect.right; i += pBitmap->GetWidth()) {
          int32_t indexX = iCounterX++ % 2;
          m_pDevice->SetDIBits(pFlip[indexY][indexX], i, j);
        }
      }
      if (pFlip[0][1] != pFlip[0][0]) {
        delete pFlip[0][1];
      }
      if (pFlip[1][0] != pFlip[0][0]) {
        delete pFlip[1][0];
      }
      if (pFlip[1][1] != pFlip[0][0]) {
        delete pFlip[1][1];
      }
    } break;
    case FDE_WRAPMODE_Clamp: {
      m_pDevice->SetDIBits(pBitmap, rect.left, rect.bottom);
    } break;
  }
  m_pDevice->EndRendering();
  m_pDevice->RestoreState();
  return TRUE;
}
FX_BOOL CFDE_FxgeDevice::FillLinearGradientPath(IFDE_Brush* pBrush,
                                                const CFX_PathData* pPath,
                                                const CFX_Matrix* pMatrix) {
  FXSYS_assert(pPath && pBrush &&
               pBrush->GetType() == FDE_BRUSHTYPE_LinearGradient);
  IFDE_LinearGradientBrush* pLinearBrush = (IFDE_LinearGradientBrush*)pBrush;
  CFX_PointF pt0, pt1;
  pLinearBrush->GetLinearPoints(pt0, pt1);
  CFX_VectorF fDiagonal;
  fDiagonal.Set(pt0, pt1);
  FX_FLOAT fTheta = FXSYS_atan2(fDiagonal.y, fDiagonal.x);
  FX_FLOAT fLength = fDiagonal.Length();
  FX_FLOAT fTotalX = fLength / FXSYS_cos(fTheta);
  FX_FLOAT fTotalY = fLength / FXSYS_cos(FX_PI / 2 - fTheta);
  FX_FLOAT fSteps = std::max(fTotalX, fTotalY);
  FX_FLOAT dx = fTotalX / fSteps;
  FX_FLOAT dy = fTotalY / fSteps;
  FX_ARGB cr0, cr1;
  pLinearBrush->GetLinearColors(cr0, cr1);
  FX_FLOAT a0 = FXARGB_A(cr0);
  FX_FLOAT r0 = FXARGB_R(cr0);
  FX_FLOAT g0 = FXARGB_G(cr0);
  FX_FLOAT b0 = FXARGB_B(cr0);
  FX_FLOAT da = (FXARGB_A(cr1) - a0) / fSteps;
  FX_FLOAT dr = (FXARGB_R(cr1) - r0) / fSteps;
  FX_FLOAT dg = (FXARGB_G(cr1) - g0) / fSteps;
  FX_FLOAT db = (FXARGB_B(cr1) - b0) / fSteps;
  CFX_DIBitmap bmp;
  bmp.Create(FXSYS_round(FXSYS_fabs(fDiagonal.x)),
             FXSYS_round(FXSYS_fabs(fDiagonal.y)), FXDIB_Argb);
  CFX_FxgeDevice dev;
  dev.Attach(&bmp);
  pt1 = pt0;
  int32_t iSteps = FXSYS_round(FXSYS_ceil(fSteps));
  while (--iSteps >= 0) {
    cr0 = ArgbEncode(FXSYS_round(a0), FXSYS_round(r0), FXSYS_round(g0),
                     FXSYS_round(b0));
    dev.DrawCosmeticLine(pt0.x, pt0.y, pt1.x, pt1.y, cr0);
    pt1.x += dx;
    pt0.y += dy;
    a0 += da;
    r0 += dr;
    g0 += dg;
    b0 += db;
  }
  return WrapTexture(pLinearBrush->GetWrapMode(), &bmp, pPath, pMatrix);
}
#endif
