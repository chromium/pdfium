// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_RENDERDEVICE
#define _FDE_RENDERDEVICE
class IFDE_Pen;
class IFDE_Brush;
class IFDE_Image;
class CFX_DIBitmap;
class CFX_DIBSource;

typedef struct _FDE_HDEVICESTATE { void* pData; } * FDE_HDEVICESTATE;

class IFDE_RenderDevice {
 public:
  static IFDE_RenderDevice* Create(CFX_DIBitmap* pBitmap,
                                   FX_BOOL bRgbByteOrder = FALSE);
  static IFDE_RenderDevice* Create(CFX_RenderDevice* pDevice);
  virtual ~IFDE_RenderDevice() {}
  virtual void Release() = 0;

  virtual int32_t GetWidth() const = 0;
  virtual int32_t GetHeight() const = 0;
  virtual FDE_HDEVICESTATE SaveState() = 0;
  virtual void RestoreState(FDE_HDEVICESTATE hState) = 0;
  virtual FX_BOOL SetClipPath(const IFDE_Path* pClip) = 0;
  virtual IFDE_Path* GetClipPath() const = 0;
  virtual FX_BOOL SetClipRect(const CFX_RectF& rtClip) = 0;
  virtual const CFX_RectF& GetClipRect() = 0;

  virtual FX_FLOAT GetDpiX() const = 0;
  virtual FX_FLOAT GetDpiY() const = 0;

  virtual FX_BOOL DrawImage(CFX_DIBSource* pDib,
                            const CFX_RectF* pSrcRect,
                            const CFX_RectF& dstRect,
                            const CFX_Matrix* pImgMatrix = NULL,
                            const CFX_Matrix* pDevMatrix = NULL) = 0;
  virtual FX_BOOL DrawString(IFDE_Brush* pBrush,
                             IFX_Font* pFont,
                             const FXTEXT_CHARPOS* pCharPos,
                             int32_t iCount,
                             FX_FLOAT fFontSize,
                             const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL DrawBezier(IFDE_Pen* pPen,
                             FX_FLOAT fPenWidth,
                             const CFX_PointF& pt1,
                             const CFX_PointF& pt2,
                             const CFX_PointF& pt3,
                             const CFX_PointF& pt4,
                             const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL DrawCurve(IFDE_Pen* pPen,
                            FX_FLOAT fPenWidth,
                            const CFX_PointsF& points,
                            FX_BOOL bClosed,
                            FX_FLOAT fTension = 0.5f,
                            const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL DrawEllipse(IFDE_Pen* pPen,
                              FX_FLOAT fPenWidth,
                              const CFX_RectF& rect,
                              const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL DrawLines(IFDE_Pen* pPen,
                            FX_FLOAT fPenWidth,
                            const CFX_PointsF& points,
                            const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL DrawLine(IFDE_Pen* pPen,
                           FX_FLOAT fPenWidth,
                           const CFX_PointF& pt1,
                           const CFX_PointF& pt2,
                           const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL DrawPath(IFDE_Pen* pPen,
                           FX_FLOAT fPenWidth,
                           const IFDE_Path* pPath,
                           const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL DrawPolygon(IFDE_Pen* pPen,
                              FX_FLOAT fPenWidth,
                              const CFX_PointsF& points,
                              const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL DrawRectangle(IFDE_Pen* pPen,
                                FX_FLOAT fPenWidth,
                                const CFX_RectF& rect,
                                const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL FillClosedCurve(IFDE_Brush* pBrush,
                                  const CFX_PointsF& points,
                                  FX_FLOAT fTension = 0.5f,
                                  const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL FillEllipse(IFDE_Brush* pBrush,
                              const CFX_RectF& rect,
                              const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL FillPath(IFDE_Brush* pBrush,
                           const IFDE_Path* pPath,
                           const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL FillPolygon(IFDE_Brush* pBrush,
                              const CFX_PointsF& points,
                              const CFX_Matrix* pMatrix = NULL) = 0;
  virtual FX_BOOL FillRectangle(IFDE_Brush* pBrush,
                                const CFX_RectF& rect,
                                const CFX_Matrix* pMatrix = NULL) = 0;
};
#endif
