// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_RENDERDEVICE_H_
#define XFA_FDE_CFDE_RENDERDEVICE_H_

#include <vector>

#include "core/fxge/cfx_renderdevice.h"
#include "xfa/fgas/font/cfgas_gefont.h"

class CFDE_Path;
class CFX_GraphStateData;

class CFDE_RenderDevice {
 public:
  explicit CFDE_RenderDevice(CFX_RenderDevice* pDevice);
  ~CFDE_RenderDevice();

  int32_t GetWidth() const;
  int32_t GetHeight() const;
  void SaveState();
  void RestoreState();
  bool SetClipPath(const CFDE_Path* pClip);
  CFDE_Path* GetClipPath() const;
  bool SetClipRect(const CFX_RectF& rtClip);
  const CFX_RectF& GetClipRect();

  float GetDpiX() const;
  float GetDpiY() const;

  bool DrawImage(const CFX_RetainPtr<CFX_DIBSource>& pDib,
                 const CFX_RectF* pSrcRect,
                 const CFX_RectF& dstRect,
                 const CFX_Matrix* pImgMatrix = nullptr,
                 const CFX_Matrix* pDevMatrix = nullptr);
  bool DrawString(FX_ARGB color,
                  const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                  const FXTEXT_CHARPOS* pCharPos,
                  int32_t iCount,
                  float fFontSize,
                  const CFX_Matrix* pMatrix = nullptr);
  bool DrawBezier(FX_ARGB color,
                  float fPenWidth,
                  const CFX_PointF& pt1,
                  const CFX_PointF& pt2,
                  const CFX_PointF& pt3,
                  const CFX_PointF& pt4,
                  const CFX_Matrix* pMatrix = nullptr);
  bool DrawCurve(FX_ARGB color,
                 float fPenWidth,
                 const std::vector<CFX_PointF>& points,
                 bool bClosed,
                 float fTension = 0.5f,
                 const CFX_Matrix* pMatrix = nullptr);
  bool DrawEllipse(FX_ARGB color,
                   float fPenWidth,
                   const CFX_RectF& rect,
                   const CFX_Matrix* pMatrix = nullptr);
  bool DrawLines(FX_ARGB color,
                 float fPenWidth,
                 const std::vector<CFX_PointF>& points,
                 const CFX_Matrix* pMatrix = nullptr);
  bool DrawLine(FX_ARGB color,
                float fPenWidth,
                const CFX_PointF& pt1,
                const CFX_PointF& pt2,
                const CFX_Matrix* pMatrix = nullptr);
  bool DrawPath(FX_ARGB color,
                float fPenWidth,
                const CFDE_Path* pPath,
                const CFX_Matrix* pMatrix = nullptr);
  bool DrawPolygon(FX_ARGB color,
                   float fPenWidth,
                   const std::vector<CFX_PointF>& points,
                   const CFX_Matrix* pMatrix = nullptr);
  bool DrawRectangle(FX_ARGB color,
                     float fPenWidth,
                     const CFX_RectF& rect,
                     const CFX_Matrix* pMatrix = nullptr);
  bool FillClosedCurve(FX_ARGB color,
                       const std::vector<CFX_PointF>& points,
                       float fTension = 0.5f,
                       const CFX_Matrix* pMatrix = nullptr);
  bool FillEllipse(FX_ARGB color,
                   const CFX_RectF& rect,
                   const CFX_Matrix* pMatrix = nullptr);
  bool FillPath(FX_ARGB color,
                const CFDE_Path* pPath,
                const CFX_Matrix* pMatrix = nullptr);
  bool FillPolygon(FX_ARGB color,
                   const std::vector<CFX_PointF>& points,
                   const CFX_Matrix* pMatrix = nullptr);
  bool FillRectangle(FX_ARGB color,
                     const CFX_RectF& rect,
                     const CFX_Matrix* pMatrix = nullptr);

  bool DrawSolidString(FX_ARGB color,
                       const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                       const FXTEXT_CHARPOS* pCharPos,
                       int32_t iCount,
                       float fFontSize,
                       const CFX_Matrix* pMatrix);
  bool DrawStringPath(FX_ARGB color,
                      const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                      const FXTEXT_CHARPOS* pCharPos,
                      int32_t iCount,
                      float fFontSize,
                      const CFX_Matrix* pMatrix);

 private:
  CFX_RenderDevice* const m_pDevice;
  CFX_RectF m_rtClip;
  int32_t m_iCharCount;
};

#endif  // XFA_FDE_CFDE_RENDERDEVICE_H_
