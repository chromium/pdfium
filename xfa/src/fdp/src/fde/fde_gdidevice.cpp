// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "stdafx.h"
#include "fde_gdidevice.h"
#include "fde_gdiobject.h"
#ifdef _FDEPLUS
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
IFDE_RenderDevice* IFDE_RenderDevice::Create(CFX_DIBitmap* pBitmap,
                                             FX_BOOL bRgbByteOrder) {
  return new CFDE_GdiDevice(pBitmap);
}
IFDE_RenderDevice* IFDE_RenderDevice::Create(CFX_RenderDevice* pDevice) {
  return NULL;
}
CFDE_GdiDevice::CFDE_GdiDevice(CFX_DIBitmap* pBitmap)
    : m_dwGlyphLen(0),
      m_pGlyphBuf(NULL),
      m_pGraphics(NULL),
      m_pBitmap(NULL),
      m_pClipPath(NULL) {
  FXSYS_assert(pBitmap != NULL);
  BITMAPINFO bmi;
  FXSYS_memset(&bmi, 0, sizeof(BITMAPINFOHEADER));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = pBitmap->GetWidth();
  bmi.bmiHeader.biHeight = -pBitmap->GetHeight();
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = pBitmap->GetBPP();
  m_pBitmap = Gdiplus::Bitmap::FromBITMAPINFO(&bmi, pBitmap->GetBuffer());
  FXSYS_assert(m_pBitmap != NULL);
  m_pGraphics = Gdiplus::Graphics::FromImage(m_pBitmap);
  FXSYS_assert(m_pGraphics != NULL);
  m_rtClipRect.Set(0, 0, (FX_FLOAT)pBitmap->GetWidth(),
                   (FX_FLOAT)pBitmap->GetHeight());
  m_pGraphics->SetClip((const Gdiplus::RectF&)m_rtClipRect);
}
CFDE_GdiDevice::~CFDE_GdiDevice() {
  delete m_pGraphics;
  delete m_pBitmap;
  FX_Free(m_pGlyphBuf);
}
int32_t CFDE_GdiDevice::GetWidth() const {
  return m_pBitmap->GetWidth();
}
int32_t CFDE_GdiDevice::GetHeight() const {
  return m_pBitmap->GetHeight();
}
FDE_HDEVICESTATE CFDE_GdiDevice::SaveState() {
  return (FDE_HDEVICESTATE)m_pGraphics->Save();
}
void CFDE_GdiDevice::RestoreState(FDE_HDEVICESTATE hState) {
  Gdiplus::Status eRet = m_pGraphics->Restore((Gdiplus::GraphicsState)hState);
  if (eRet == Gdiplus::Ok) {
    Gdiplus::Rect rt;
    eRet = m_pGraphics->GetClipBounds(&rt);
    if (eRet == Gdiplus::Ok) {
      m_rtClipRect.Set((FX_FLOAT)rt.X, (FX_FLOAT)rt.Y, (FX_FLOAT)rt.Width,
                       (FX_FLOAT)rt.Height);
    }
  }
}
FX_BOOL CFDE_GdiDevice::SetClipRect(const CFX_RectF& rtClip) {
  m_rtClipRect = rtClip;
  return m_pGraphics->SetClip((const Gdiplus::RectF&)rtClip) == Gdiplus::Ok;
}
const CFX_RectF& CFDE_GdiDevice::GetClipRect() {
  return m_rtClipRect;
}
FX_BOOL CFDE_GdiDevice::SetClipPath(const IFDE_Path* pClip) {
  m_pClipPath = (CFDE_GdiPath*)pClip;
  Gdiplus::GraphicsPath* pPath = m_pClipPath ? &m_pClipPath->m_Path : NULL;
  return m_pGraphics->SetClip(pPath) == Gdiplus::Ok;
}
IFDE_Path* CFDE_GdiDevice::GetClipPath() const {
  return m_pClipPath;
}
FX_FLOAT CFDE_GdiDevice::GetDpiX() const {
  return m_pGraphics->GetDpiX();
}
FX_FLOAT CFDE_GdiDevice::GetDpiY() const {
  return m_pGraphics->GetDpiY();
}
FX_BOOL CFDE_GdiDevice::DrawImage(IFDE_Image* pImg,
                                  const CFX_RectF* pSrcRect,
                                  const CFX_RectF& dstRect,
                                  const CFX_Matrix* pImgMatrix,
                                  const CFX_Matrix* pDevMatrix) {
  CFDE_GdiImage* pGdiImg = (CFDE_GdiImage*)pImg;
  FXSYS_assert(pGdiImg != NULL && pGdiImg->m_pImage != NULL);
  CFX_RectF srcRect;
  if (pSrcRect) {
    srcRect = *pSrcRect;
  } else {
    srcRect.left = srcRect.top = 0;
    srcRect.width = (FX_FLOAT)pImg->GetImageWidth();
    srcRect.height = (FX_FLOAT)pImg->GetImageHeight();
  }
  CFX_Matrix matrix;
  if (pImgMatrix) {
    matrix = *pImgMatrix;
  } else {
    matrix.Reset();
  }
  matrix.Translate(dstRect.left, dstRect.top);
  matrix.Scale((dstRect.width / srcRect.width),
               (dstRect.height / srcRect.height), TRUE);
  if (pDevMatrix) {
    matrix.Concat(*pDevMatrix);
  }
  CFX_PointF dstPoints[3];
  dstPoints[0].Set(0, 0);
  dstPoints[1].Set(srcRect.width, 0);
  dstPoints[2].Set(0, srcRect.height);
  matrix.TransformPoints(dstPoints, 3);
  m_pGraphics->DrawImage(pGdiImg->m_pImage, (Gdiplus::PointF*)dstPoints, 3,
                         srcRect.left, srcRect.top, srcRect.width,
                         srcRect.height, Gdiplus::UnitPixel, NULL, NULL, NULL);
  return TRUE;
}
FX_BOOL CFDE_GdiDevice::DrawImage(CFX_DIBSource* pDib,
                                  const CFX_RectF* pSrcRect,
                                  const CFX_RectF& dstRect,
                                  const CFX_Matrix* pImgMatrix,
                                  const CFX_Matrix* pDevMatrix) {
  FXSYS_assert(pDib != NULL);
  BITMAPINFO bmi;
  FXSYS_memset(&bmi, 0, sizeof(BITMAPINFOHEADER));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = pDib->GetWidth();
  bmi.bmiHeader.biHeight = pDib->GetHeight();
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = pDib->GetBPP();
  Gdiplus::Bitmap bmp(&bmi, pDib->GetBuffer());
  CFDE_GdiImage img(&bmp);
  return DrawImage(&img, pSrcRect, dstRect, pImgMatrix, pDevMatrix);
}
FX_BOOL CFDE_GdiDevice::DrawString(IFDE_Brush* pBrush,
                                   IFX_Font* pFont,
                                   const FXTEXT_CHARPOS* pCharPos,
                                   int32_t iCount,
                                   FX_FLOAT fFontSize,
                                   const CFX_Matrix* pMatrix) {
  FXSYS_assert(pBrush != NULL && pFont != NULL && pCharPos != NULL);
  FX_ARGB argb = 0xFF000000;
  if (pBrush->GetType() == FDE_BRUSHTYPE_Solid) {
    argb = ((IFDE_SolidBrush*)pBrush)->GetColor();
  }
  CFDE_GdiFont* pGdiFont = (CFDE_GdiFont*)pFont;
  GLYPHMETRICS gm;
  MAT2 mat2;
  FX_FLOAT fScale = fFontSize / 1000.0f;
  FX_FLOAT ma, mb, mc, md;
  FX_FLOAT fx, fy;
  while (--iCount >= 0) {
    mb = mc = 0;
    ma = md = fScale;
    if (pCharPos->m_bGlyphAdjust) {
      FX_FLOAT aa =
          ma * -pCharPos->m_AdjustMatrix[0] + mb * pCharPos->m_AdjustMatrix[2];
      FX_FLOAT bb =
          -ma * pCharPos->m_AdjustMatrix[1] + mb * pCharPos->m_AdjustMatrix[3];
      FX_FLOAT cc =
          mc * -pCharPos->m_AdjustMatrix[0] + md * pCharPos->m_AdjustMatrix[2];
      FX_FLOAT dd =
          -mc * pCharPos->m_AdjustMatrix[1] + md * pCharPos->m_AdjustMatrix[3];
      ma = aa;
      mb = bb;
      mc = cc;
      md = dd;
    }
    if (pMatrix) {
      FX_FLOAT aa = ma * pMatrix->a + mb * pMatrix->c;
      FX_FLOAT bb = ma * pMatrix->b + mb * pMatrix->d;
      FX_FLOAT cc = mc * pMatrix->a + md * pMatrix->c;
      FX_FLOAT dd = mc * pMatrix->b + md * pMatrix->d;
      ma = aa;
      mb = bb;
      mc = cc;
      md = dd;
    }
    *(long*)(&mat2.eM11) = (long)(ma * 65536);
    *(long*)(&mat2.eM21) = (long)(mb * 65536);
    *(long*)(&mat2.eM12) = (long)(mc * 65536);
    *(long*)(&mat2.eM22) = (long)(md * 65536);
    FX_DWORD dwSize = pGdiFont->GetGlyphDIBits(pCharPos->m_GlyphIndex, argb,
                                               &mat2, gm, NULL, 0);
    if (dwSize > 0) {
      if (m_pGlyphBuf == NULL) {
        m_pGlyphBuf = FX_Alloc(uint8_t, dwSize);
        m_dwGlyphLen = dwSize;
      } else if (m_dwGlyphLen < dwSize) {
        m_pGlyphBuf = FX_Realloc(uint8_t, m_pGlyphBuf, dwSize);
        m_dwGlyphLen = dwSize;
      }
      pGdiFont->GetGlyphDIBits(pCharPos->m_GlyphIndex, argb, &mat2, gm,
                               m_pGlyphBuf, m_dwGlyphLen);
      Gdiplus::Bitmap bmp(gm.gmBlackBoxX, gm.gmBlackBoxY, gm.gmBlackBoxX * 4,
                          PixelFormat32bppARGB, m_pGlyphBuf);
      if (pMatrix) {
        fx = pMatrix->a * pCharPos->m_OriginX +
             pMatrix->c * pCharPos->m_OriginY + pMatrix->e;
        fy = pMatrix->b * pCharPos->m_OriginX +
             pMatrix->d * pCharPos->m_OriginY + pMatrix->f;
      } else {
        fx = pCharPos->m_OriginX;
        fy = pCharPos->m_OriginY;
      }
      m_pGraphics->DrawImage(&bmp, (FXSYS_round(fx) + gm.gmptGlyphOrigin.x),
                             (FXSYS_round(fy) - gm.gmptGlyphOrigin.y));
    }
    pCharPos++;
  }
  return TRUE;
}
FX_BOOL CFDE_GdiDevice::DrawArc(IFDE_Pen* pPen,
                                FX_FLOAT fPenWidth,
                                const CFX_RectF& rect,
                                FX_FLOAT startAngle,
                                FX_FLOAT sweepAngle,
                                const CFX_Matrix* pMatrix) {
  startAngle = FX_RAD2DEG(startAngle);
  sweepAngle = FX_RAD2DEG(sweepAngle);
  Gdiplus::Pen* pGdiPen = CreateGdiPen(pPen, fPenWidth);
  if (pGdiPen == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret =
      m_pGraphics->DrawArc(pGdiPen, rect.left, rect.top, rect.width,
                           rect.height, startAngle, sweepAngle);
  RestoreMatrix(pMatrix);
  ReleaseGdiPen(pGdiPen);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::DrawBezier(IFDE_Pen* pPen,
                                   FX_FLOAT fPenWidth,
                                   const CFX_PointF& pt1,
                                   const CFX_PointF& pt2,
                                   const CFX_PointF& pt3,
                                   const CFX_PointF& pt4,
                                   const CFX_Matrix* pMatrix) {
  Gdiplus::Pen* pGdiPen = CreateGdiPen(pPen, fPenWidth);
  if (pGdiPen == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->DrawBezier(
      pGdiPen, pt1.x, pt1.y, pt2.x, pt2.y, pt3.x, pt3.y, pt4.x, pt4.y);
  RestoreMatrix(pMatrix);
  ReleaseGdiPen(pGdiPen);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::DrawCurve(IFDE_Pen* pPen,
                                  FX_FLOAT fPenWidth,
                                  const CFX_PointsF& points,
                                  FX_BOOL bClosed,
                                  FX_FLOAT fTension,
                                  const CFX_Matrix* pMatrix) {
  Gdiplus::Pen* pGdiPen = CreateGdiPen(pPen, fPenWidth);
  if (pGdiPen == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret =
      bClosed
          ? m_pGraphics->DrawClosedCurve(
                pGdiPen, (const Gdiplus::PointF*)points.GetData(),
                points.GetSize(), fTension)
          : m_pGraphics->DrawCurve(pGdiPen,
                                   (const Gdiplus::PointF*)points.GetData(),
                                   points.GetSize(), fTension);
  RestoreMatrix(pMatrix);
  ReleaseGdiPen(pGdiPen);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::DrawEllipse(IFDE_Pen* pPen,
                                    FX_FLOAT fPenWidth,
                                    const CFX_RectF& rect,
                                    const CFX_Matrix* pMatrix) {
  Gdiplus::Pen* pGdiPen = CreateGdiPen(pPen, fPenWidth);
  if (pGdiPen == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->DrawEllipse(pGdiPen, rect.left, rect.top,
                                                 rect.width, rect.height);
  RestoreMatrix(pMatrix);
  ReleaseGdiPen(pGdiPen);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::DrawLines(IFDE_Pen* pPen,
                                  FX_FLOAT fPenWidth,
                                  const CFX_PointsF& points,
                                  const CFX_Matrix* pMatrix) {
  Gdiplus::Pen* pGdiPen = CreateGdiPen(pPen, fPenWidth);
  if (pGdiPen == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->DrawLines(
      pGdiPen, (const Gdiplus::PointF*)points.GetData(), points.GetSize());
  ApplyMatrix(pMatrix);
  ReleaseGdiPen(pGdiPen);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::DrawLine(IFDE_Pen* pPen,
                                 FX_FLOAT fPenWidth,
                                 const CFX_PointF& pt1,
                                 const CFX_PointF& pt2,
                                 const CFX_Matrix* pMatrix) {
  Gdiplus::Pen* pGdiPen = CreateGdiPen(pPen, fPenWidth);
  if (pGdiPen == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret =
      m_pGraphics->DrawLine(pGdiPen, pt1.x, pt1.y, pt2.x, pt2.y);
  RestoreMatrix(pMatrix);
  ReleaseGdiPen(pGdiPen);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::DrawPath(IFDE_Pen* pPen,
                                 FX_FLOAT fPenWidth,
                                 const IFDE_Path* pPath,
                                 const CFX_Matrix* pMatrix) {
  CFDE_GdiPath* pGdiPath = (CFDE_GdiPath*)pPath;
  if (pGdiPath == NULL) {
    return FALSE;
  }
  Gdiplus::Pen* pGdiPen = CreateGdiPen(pPen, fPenWidth);
  if (pGdiPen == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->DrawPath(pGdiPen, &pGdiPath->m_Path);
  RestoreMatrix(pMatrix);
  ReleaseGdiPen(pGdiPen);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::DrawPie(IFDE_Pen* pPen,
                                FX_FLOAT fPenWidth,
                                const CFX_RectF& rect,
                                FX_FLOAT startAngle,
                                FX_FLOAT sweepAngle,
                                const CFX_Matrix* pMatrix) {
  startAngle = FX_RAD2DEG(startAngle);
  sweepAngle = FX_RAD2DEG(sweepAngle);
  Gdiplus::Pen* pGdiPen = CreateGdiPen(pPen, fPenWidth);
  if (pGdiPen == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret =
      m_pGraphics->DrawPie(pGdiPen, rect.left, rect.top, rect.width,
                           rect.height, startAngle, sweepAngle);
  RestoreMatrix(pMatrix);
  ReleaseGdiPen(pGdiPen);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::DrawChord(IFDE_Pen* pPen,
                                  FX_FLOAT fPenWidth,
                                  const CFX_RectF& rect,
                                  FX_FLOAT startAngle,
                                  FX_FLOAT sweepAngle,
                                  const CFX_Matrix* pMatrix) {
  CFX_ArcF chord;
  chord.Set(rect, startAngle, sweepAngle);
  CFDE_GdiPath path;
  path.AddChord(chord);
  return DrawPath(pPen, fPenWidth, &path, pMatrix);
}
FX_BOOL CFDE_GdiDevice::DrawPolygon(IFDE_Pen* pPen,
                                    FX_FLOAT fPenWidth,
                                    const CFX_PointsF& points,
                                    const CFX_Matrix* pMatrix) {
  Gdiplus::Pen* pGdiPen = CreateGdiPen(pPen, fPenWidth);
  if (pGdiPen == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->DrawPolygon(
      pGdiPen, (const Gdiplus::PointF*)points.GetData(), points.GetSize());
  RestoreMatrix(pMatrix);
  ReleaseGdiPen(pGdiPen);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::DrawRectangle(IFDE_Pen* pPen,
                                      FX_FLOAT fPenWidth,
                                      const CFX_RectF& rect,
                                      const CFX_Matrix* pMatrix) {
  Gdiplus::Pen* pGdiPen = CreateGdiPen(pPen, fPenWidth);
  if (pGdiPen == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->DrawRectangle(pGdiPen, rect.left, rect.top,
                                                   rect.width, rect.height);
  RestoreMatrix(pMatrix);
  ReleaseGdiPen(pGdiPen);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::DrawRoundRectangle(IFDE_Pen* pPen,
                                           FX_FLOAT fPenWidth,
                                           const CFX_RectF& rect,
                                           const CFX_SizeF& round,
                                           const CFX_Matrix* pMatrix) {
  CFDE_GdiPath path;
  path.AddRoundRectangle(rect, round);
  return DrawPath(pPen, fPenWidth, &path, pMatrix);
}
FX_BOOL CFDE_GdiDevice::FillClosedCurve(IFDE_Brush* pBrush,
                                        const CFX_PointsF& points,
                                        FX_FLOAT fTension,
                                        const CFX_Matrix* pMatrix) {
  Gdiplus::Brush* pGdiBrush = CreateGdiBrush(pBrush);
  if (pGdiBrush == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->FillClosedCurve(
      pGdiBrush, (const Gdiplus::PointF*)points.GetData(), points.GetSize(),
      Gdiplus::FillModeAlternate, fTension);
  RestoreMatrix(pMatrix);
  ReleaseGdiBrush(pGdiBrush);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::FillEllipse(IFDE_Brush* pBrush,
                                    const CFX_RectF& rect,
                                    const CFX_Matrix* pMatrix) {
  Gdiplus::Brush* pGdiBrush = CreateGdiBrush(pBrush);
  if (pGdiBrush == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->FillEllipse(pGdiBrush, rect.left, rect.top,
                                                 rect.width, rect.height);
  RestoreMatrix(pMatrix);
  ReleaseGdiBrush(pGdiBrush);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::FillPath(IFDE_Brush* pBrush,
                                 const IFDE_Path* pPath,
                                 const CFX_Matrix* pMatrix) {
  CFDE_GdiPath* pGdiPath = (CFDE_GdiPath*)pPath;
  if (pGdiPath == NULL) {
    return FALSE;
  }
  Gdiplus::Brush* pGdiBrush = CreateGdiBrush(pBrush);
  if (pGdiBrush == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->FillPath(pGdiBrush, &pGdiPath->m_Path);
  RestoreMatrix(pMatrix);
  ReleaseGdiBrush(pGdiBrush);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::FillPie(IFDE_Brush* pBrush,
                                const CFX_RectF& rect,
                                FX_FLOAT startAngle,
                                FX_FLOAT sweepAngle,
                                const CFX_Matrix* pMatrix) {
  startAngle = FX_RAD2DEG(startAngle);
  sweepAngle = FX_RAD2DEG(sweepAngle);
  Gdiplus::Brush* pGdiBrush = CreateGdiBrush(pBrush);
  if (pGdiBrush == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret =
      m_pGraphics->FillPie(pGdiBrush, rect.left, rect.top, rect.width,
                           rect.height, startAngle, sweepAngle);
  RestoreMatrix(pMatrix);
  ReleaseGdiBrush(pGdiBrush);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::FillChord(IFDE_Brush* pBrush,
                                  const CFX_RectF& rect,
                                  FX_FLOAT startAngle,
                                  FX_FLOAT sweepAngle,
                                  const CFX_Matrix* pMatrix) {
  CFX_ArcF chord;
  chord.Set(rect, startAngle, sweepAngle);
  CFDE_GdiPath path;
  path.AddChord(chord);
  return FillPath(pBrush, &path, pMatrix);
}
FX_BOOL CFDE_GdiDevice::FillPolygon(IFDE_Brush* pBrush,
                                    const CFX_PointsF& points,
                                    const CFX_Matrix* pMatrix) {
  Gdiplus::Brush* pGdiBrush = CreateGdiBrush(pBrush);
  if (pGdiBrush == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->FillPolygon(
      pGdiBrush, (const Gdiplus::PointF*)points.GetData(), points.GetSize());
  RestoreMatrix(pMatrix);
  ReleaseGdiBrush(pGdiBrush);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::FillRectangle(IFDE_Brush* pBrush,
                                      const CFX_RectF& rect,
                                      const CFX_Matrix* pMatrix) {
  Gdiplus::Brush* pGdiBrush = CreateGdiBrush(pBrush);
  if (pGdiBrush == NULL) {
    return FALSE;
  }
  ApplyMatrix(pMatrix);
  Gdiplus::Status ret = m_pGraphics->FillRectangle(
      pGdiBrush, rect.left, rect.top, rect.width, rect.height);
  RestoreMatrix(pMatrix);
  ReleaseGdiBrush(pGdiBrush);
  return ret == Gdiplus::Ok;
}
FX_BOOL CFDE_GdiDevice::FillRoundRectangle(IFDE_Brush* pBrush,
                                           const CFX_RectF& rect,
                                           const CFX_SizeF& round,
                                           const CFX_Matrix* pMatrix) {
  CFDE_GdiPath path;
  path.AddRoundRectangle(rect, round);
  return FillPath(pBrush, &path, pMatrix);
}
Gdiplus::Pen* CFDE_GdiDevice::CreateGdiPen(IFDE_Pen* pPen, FX_FLOAT fPenWidth) {
  if (pPen == NULL || fPenWidth < 0.01f) {
    return NULL;
  }
  Gdiplus::Pen* pGdiPen = NULL;
  switch (pPen->GetType()) {
    case FDE_PENTYPE_SolidColor: {
      Gdiplus::Color gdiColor((Gdiplus::ARGB)pPen->GetColor());
      pGdiPen = new Gdiplus::Pen(gdiColor, fPenWidth);
    } break;
    case FDE_PENTYPE_HatchBrush:
    case FDE_PENTYPE_TextureBrush:
    case FDE_PENTYPE_LinearGradient: {
      Gdiplus::Brush* pGdiBrush = CreateGdiBrush(pPen->GetBrush());
      if (pGdiBrush) {
        pGdiPen = new Gdiplus::Pen(pGdiBrush, fPenWidth);
      }
    } break;
  }
  if (pGdiPen) {
    CFX_FloatArray dashArray;
    pPen->GetDashArray(dashArray);
    pGdiPen->SetDashPattern(dashArray.GetData(), dashArray.GetSize());
    pGdiPen->SetDashOffset(pPen->GetDashPhase());
    pGdiPen->SetDashStyle((Gdiplus::DashStyle)pPen->GetDashStyle());
    pGdiPen->SetStartCap((Gdiplus::LineCap)pPen->GetLineCap());
    pGdiPen->SetEndCap((Gdiplus::LineCap)pPen->GetLineCap());
    pGdiPen->SetLineJoin((Gdiplus::LineJoin)pPen->GetLineJoin());
    pGdiPen->SetMiterLimit(pPen->GetMiterLimit());
  }
  return pGdiPen;
}
void CFDE_GdiDevice::ReleaseGdiPen(Gdiplus::Pen* pGdiPen) {
  if (pGdiPen) {
    ReleaseGdiBrush(pGdiPen->GetBrush());
    delete pGdiPen;
  }
}
Gdiplus::Brush* CFDE_GdiDevice::CreateGdiBrush(IFDE_Brush* pBrush) {
  if (pBrush == NULL) {
    return NULL;
  }
  Gdiplus::Brush* pGdiBrush = NULL;
  switch (pBrush->GetType()) {
    case FDE_BRUSHTYPE_Solid: {
      IFDE_SolidBrush* pSolidBrush = (IFDE_SolidBrush*)pBrush;
      Gdiplus::Color gdiColor((Gdiplus::ARGB)pSolidBrush->GetColor());
      pGdiBrush = new Gdiplus::SolidBrush(gdiColor);
    } break;
    case FDE_BRUSHTYPE_Hatch: {
      IFDE_HatchBrush* pHatchBrush = (IFDE_HatchBrush*)pBrush;
      Gdiplus::Color foreColor((Gdiplus::ARGB)pHatchBrush->GetColor(TRUE));
      Gdiplus::Color backColor((Gdiplus::ARGB)pHatchBrush->GetColor(FALSE));
      Gdiplus::HatchStyle hatchStyle =
          (Gdiplus::HatchStyle)pHatchBrush->GetHatchStyle();
      pGdiBrush = new Gdiplus::HatchBrush(hatchStyle, foreColor, backColor);
    } break;
    case FDE_BRUSHTYPE_Texture: {
      IFDE_TextureBrush* pTextureBrush = (IFDE_TextureBrush*)pBrush;
      CFDE_GdiImage* pImgHolder = (CFDE_GdiImage*)pTextureBrush->GetImage();
      Gdiplus::Image* pGdiImage = pImgHolder ? pImgHolder->m_pImage : NULL;
      Gdiplus::WrapMode wrapMode =
          (Gdiplus::WrapMode)pTextureBrush->GetWrapMode();
      pGdiBrush = new Gdiplus::TextureBrush(pGdiImage, wrapMode);
    } break;
    case FDE_BRUSHTYPE_LinearGradient: {
      IFDE_LinearGradientBrush* pLinearBrush =
          (IFDE_LinearGradientBrush*)pBrush;
      Gdiplus::WrapMode wrapMode =
          (Gdiplus::WrapMode)pLinearBrush->GetWrapMode();
      CFX_PointF ptStart, ptEnd;
      pLinearBrush->GetLinearPoints(ptStart, ptEnd);
      FX_ARGB crStart, crEnd;
      pLinearBrush->GetLinearColors(crStart, crEnd);
      pGdiBrush = new Gdiplus::LinearGradientBrush(
          (const Gdiplus::PointF&)ptStart, (const Gdiplus::PointF&)ptEnd,
          (const Gdiplus::Color&)crStart, (const Gdiplus::Color&)crEnd);
    } break;
  }
  return pGdiBrush;
}
void CFDE_GdiDevice::ReleaseGdiBrush(Gdiplus::Brush* pGdiBrush) {
  if (pGdiBrush) {
    delete pGdiBrush;
  }
}
void CFDE_GdiDevice::ApplyMatrix(const CFX_Matrix* pMatrix) {
  if (pMatrix) {
    m_GraphicsState = m_pGraphics->Save();
    Gdiplus::Matrix gdiMatrix(pMatrix->a, pMatrix->b, pMatrix->c, pMatrix->d,
                              pMatrix->e, pMatrix->f);
    m_pGraphics->SetTransform(&gdiMatrix);
  }
}
void CFDE_GdiDevice::RestoreMatrix(const CFX_Matrix* pMatrix) {
  if (pMatrix) {
    m_pGraphics->Restore(m_GraphicsState);
  }
}
#endif
#endif
