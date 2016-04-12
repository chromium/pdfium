// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_FDE_GEDEVICE_H_
#define XFA_FDE_FDE_GEDEVICE_H_

#include "xfa/fde/fde_renderdevice.h"
#include "xfa/fgas/crt/fgas_memory.h"

class CFDE_FxgeDevice : public IFDE_RenderDevice, public CFX_Target {
 public:
  CFDE_FxgeDevice(CFX_RenderDevice* pDevice, FX_BOOL bOwnerDevice);
  ~CFDE_FxgeDevice();
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
  virtual FX_BOOL DrawString(CFDE_Brush* pBrush,
                             IFX_Font* pFont,
                             const FXTEXT_CHARPOS* pCharPos,
                             int32_t iCount,
                             FX_FLOAT fFontSize,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawBezier(CFDE_Pen* pPen,
                             FX_FLOAT fPenWidth,
                             const CFX_PointF& pt1,
                             const CFX_PointF& pt2,
                             const CFX_PointF& pt3,
                             const CFX_PointF& pt4,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawCurve(CFDE_Pen* pPen,
                            FX_FLOAT fPenWidth,
                            const CFX_PointsF& points,
                            FX_BOOL bClosed,
                            FX_FLOAT fTension = 0.5f,
                            const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawEllipse(CFDE_Pen* pPen,
                              FX_FLOAT fPenWidth,
                              const CFX_RectF& rect,
                              const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawLines(CFDE_Pen* pPen,
                            FX_FLOAT fPenWidth,
                            const CFX_PointsF& points,
                            const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawLine(CFDE_Pen* pPen,
                           FX_FLOAT fPenWidth,
                           const CFX_PointF& pt1,
                           const CFX_PointF& pt2,
                           const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawPath(CFDE_Pen* pPen,
                           FX_FLOAT fPenWidth,
                           const IFDE_Path* pPath,
                           const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawPolygon(CFDE_Pen* pPen,
                              FX_FLOAT fPenWidth,
                              const CFX_PointsF& points,
                              const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL DrawRectangle(CFDE_Pen* pPen,
                                FX_FLOAT fPenWidth,
                                const CFX_RectF& rect,
                                const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillClosedCurve(CFDE_Brush* pBrush,
                                  const CFX_PointsF& points,
                                  FX_FLOAT fTension = 0.5f,
                                  const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillEllipse(CFDE_Brush* pBrush,
                              const CFX_RectF& rect,
                              const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillPath(CFDE_Brush* pBrush,
                           const IFDE_Path* pPath,
                           const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillPolygon(CFDE_Brush* pBrush,
                              const CFX_PointsF& points,
                              const CFX_Matrix* pMatrix = NULL);
  virtual FX_BOOL FillRectangle(CFDE_Brush* pBrush,
                                const CFX_RectF& rect,
                                const CFX_Matrix* pMatrix = NULL);

  FX_BOOL DrawSolidString(CFDE_Brush* pBrush,
                          IFX_Font* pFont,
                          const FXTEXT_CHARPOS* pCharPos,
                          int32_t iCount,
                          FX_FLOAT fFontSize,
                          const CFX_Matrix* pMatrix);
  FX_BOOL DrawStringPath(CFDE_Brush* pBrush,
                         IFX_Font* pFont,
                         const FXTEXT_CHARPOS* pCharPos,
                         int32_t iCount,
                         FX_FLOAT fFontSize,
                         const CFX_Matrix* pMatrix);

 protected:
  FX_BOOL CreatePen(CFDE_Pen* pPen,
                    FX_FLOAT fPenWidth,
                    CFX_GraphStateData& graphState);

  CFX_RenderDevice* m_pDevice;
  CFX_RectF m_rtClip;
  FX_BOOL m_bOwnerDevice;
  FXTEXT_CHARPOS* m_pCharPos;
  int32_t m_iCharCount;
};

#endif  // XFA_FDE_FDE_GEDEVICE_H_
