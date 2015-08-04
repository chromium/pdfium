// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_GDIPLUSDEVICE_IMP
#define _FDE_GDIPLUSDEVICE_IMP
#ifdef _FDEPLUS
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
class CFDE_GdiPath;
class CFDE_GdiDevice : public IFDE_RenderDevice, public CFX_Target {
 public:
  CFDE_GdiDevice(CFX_DIBitmap* pDib);
  ~CFDE_GdiDevice();

  virtual void Release() { delete this; }

  virtual int32_t GetWidth() const;
  virtual int32_t GetHeight() const;
  virtual FDE_HDEVICESTATE SaveState();
  virtual void RestoreState(FDE_HDEVICESTATE hState);
  virtual FX_BOOL SetClipPath(const IFDE_Path* pClip);
  virtual IFDE_Path* GetClipPath() const;
  virtual FX_BOOL SetClipRect(const CFX_RectF& rtClip);
  virtual const CFX_RectF& GetClipRect();

  virtual FX_FLOAT GetDpiX() const;
  virtual FX_FLOAT GetDpiY() const;

  virtual FX_BOOL DrawImage(CFX_DIBSource* pDib,
                            const CFX_RectF* pSrcRect,
                            const CFX_RectF& dstRect,
                            const CFX_Matrix* pImgMatrix = NULL,
                            const CFX_Matrix* pDevMatrix = NULL);
  virtual FX_BOOL DrawImage(IFDE_Image* pImg,
                            const CFX_RectF* pSrcRect,
                            const CFX_RectF& dstRect,
                            const CFX_Matrix* pImgMatrix = NULL,
                            const CFX_Matrix* pDevMatrix = NULL);
  virtual FX_BOOL DrawString(IFDE_Brush* pBrush,
                             IFX_Font* pFont,
                             const FXTEXT_CHARPOS* pCharPos,
                             int32_t iCount,
                             FX_FLOAT fFontSize,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawArc(IFDE_Pen* pPen,
                          FX_FLOAT fPenWidth,
                          const CFX_RectF& rect,
                          FX_FLOAT startAngle,
                          FX_FLOAT sweepAngle,
                          const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawBezier(IFDE_Pen* pPen,
                             FX_FLOAT fPenWidth,
                             const CFX_PointF& pt1,
                             const CFX_PointF& pt2,
                             const CFX_PointF& pt3,
                             const CFX_PointF& pt4,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawCurve(IFDE_Pen* pPen,
                            FX_FLOAT fPenWidth,
                            const CFX_PointsF& points,
                            FX_BOOL bClosed,
                            FX_FLOAT fTension = 0.5f,
                            const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawEllipse(IFDE_Pen* pPen,
                              FX_FLOAT fPenWidth,
                              const CFX_RectF& rect,
                              const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawLines(IFDE_Pen* pPen,
                            FX_FLOAT fPenWidth,
                            const CFX_PointsF& points,
                            const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawLine(IFDE_Pen* pPen,
                           FX_FLOAT fPenWidth,
                           const CFX_PointF& pt1,
                           const CFX_PointF& pt2,
                           const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawPath(IFDE_Pen* pPen,
                           FX_FLOAT fPenWidth,
                           const IFDE_Path* pPath,
                           const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawPie(IFDE_Pen* pPen,
                          FX_FLOAT fPenWidth,
                          const CFX_RectF& rect,
                          FX_FLOAT startAngle,
                          FX_FLOAT sweepAngle,
                          const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawChord(IFDE_Pen* pPen,
                            FX_FLOAT fPenWidth,
                            const CFX_RectF& rect,
                            FX_FLOAT startAngle,
                            FX_FLOAT sweepAngle,
                            const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawPolygon(IFDE_Pen* pPen,
                              FX_FLOAT fPenWidth,
                              const CFX_PointsF& points,
                              const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawRectangle(IFDE_Pen* pPen,
                                FX_FLOAT fPenWidth,
                                const CFX_RectF& rect,
                                const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawRoundRectangle(IFDE_Pen* pPen,
                                     FX_FLOAT fPenWidth,
                                     const CFX_RectF& rect,
                                     const CFX_SizeF& round,
                                     const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillClosedCurve(IFDE_Brush* pBrush,
                                  const CFX_PointsF& points,
                                  FX_FLOAT fTension = 0.5f,
                                  const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillEllipse(IFDE_Brush* pBrush,
                              const CFX_RectF& rect,
                              const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillPath(IFDE_Brush* pBrush,
                           const IFDE_Path* pPath,
                           const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillPie(IFDE_Brush* pBrush,
                          const CFX_RectF& rect,
                          FX_FLOAT startAngle,
                          FX_FLOAT sweepAngle,
                          const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillChord(IFDE_Brush* pBrush,
                            const CFX_RectF& rect,
                            FX_FLOAT startAngle,
                            FX_FLOAT sweepAngle,
                            const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillPolygon(IFDE_Brush* pBrush,
                              const CFX_PointsF& points,
                              const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillRectangle(IFDE_Brush* pBrush,
                                const CFX_RectF& rect,
                                const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillRoundRectangle(IFDE_Brush* pBrush,
                                     const CFX_RectF& rect,
                                     const CFX_SizeF& round,
                                     const CFX_Matrix* pMatrix = NULL);

 protected:
  Gdiplus::Pen* CreateGdiPen(IFDE_Pen* pPen, FX_FLOAT fPenWidth);
  void ReleaseGdiPen(Gdiplus::Pen* pGdiPen);
  Gdiplus::Brush* CreateGdiBrush(IFDE_Brush* pBrush);
  void ReleaseGdiBrush(Gdiplus::Brush* pGdiBrush);
  void ApplyMatrix(const CFX_Matrix* pMatrix);
  void RestoreMatrix(const CFX_Matrix* pMatrix);
  Gdiplus::GraphicsState m_GraphicsState;
  Gdiplus::Graphics* m_pGraphics;
  Gdiplus::Bitmap* m_pBitmap;
  uint8_t* m_pGlyphBuf;
  FX_DWORD m_dwGlyphLen;
  CFX_RectF m_rtClipRect;
  CFDE_GdiPath* m_pClipPath;
};
#endif
#endif
#endif
