// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_FDE_PATH_H_
#define XFA_FDE_FDE_PATH_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"

class IFDE_Path {
 public:
  static IFDE_Path* Create();
  virtual ~IFDE_Path() {}
  virtual void Release() = 0;
  virtual FX_BOOL StartFigure() = 0;
  virtual FX_BOOL CloseFigure() = 0;
  virtual void AddBezier(const CFX_PointsF& points) = 0;
  virtual void AddBeziers(const CFX_PointsF& points) = 0;
  virtual void AddCurve(const CFX_PointsF& points,
                        FX_BOOL bClosed,
                        FX_FLOAT fTension = 0.5f) = 0;
  virtual void AddEllipse(const CFX_RectF& rect) = 0;
  virtual void AddLines(const CFX_PointsF& points) = 0;
  virtual void AddLine(const CFX_PointF& pt1, const CFX_PointF& pt2) = 0;
  virtual void AddPath(const IFDE_Path* pSrc, FX_BOOL bConnect) = 0;
  virtual void AddPolygon(const CFX_PointsF& points) = 0;
  virtual void AddRectangle(const CFX_RectF& rect) = 0;
  virtual void GetBBox(CFX_RectF& bbox) const = 0;
  virtual void GetBBox(CFX_RectF& bbox,
                       FX_FLOAT fLineWidth,
                       FX_FLOAT fMiterLimit) const = 0;
};

#endif  // XFA_FDE_FDE_PATH_H_
