// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_PATH_H_
#define XFA_FDE_CFDE_PATH_H_

#include <vector>

#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"

class CFDE_Path {
 public:
  void CloseFigure();

  void AddBezier(const std::vector<CFX_PointF>& points);
  void AddBeziers(const std::vector<CFX_PointF>& points);
  void AddCurve(const std::vector<CFX_PointF>& points,
                bool bClosed,
                FX_FLOAT fTension = 0.5f);
  void AddEllipse(const CFX_RectF& rect);
  void AddLines(const std::vector<CFX_PointF>& points);
  void AddLine(const CFX_PointF& pt1, const CFX_PointF& pt2);
  void AddPath(const CFDE_Path* pSrc, bool bConnect);
  void AddPolygon(const std::vector<CFX_PointF>& points);
  void AddRectangle(const CFX_RectF& rect);

  CFX_RectF GetBBox() const;
  CFX_RectF GetBBox(FX_FLOAT fLineWidth, FX_FLOAT fMiterLimit) const;

  bool FigureClosed() const;
  void BezierTo(const CFX_PointF& p1,
                const CFX_PointF& p2,
                const CFX_PointF& p3);
  void ArcTo(bool bStart,
             const CFX_RectF& rect,
             FX_FLOAT startAngle,
             FX_FLOAT endAngle);
  void MoveTo(const CFX_PointF& p);
  void LineTo(const CFX_PointF& p);

  void GetCurveTangents(const std::vector<CFX_PointF>& points,
                        std::vector<CFX_PointF>* tangents,
                        bool bClosed,
                        FX_FLOAT fTension) const;
  CFX_PathData m_Path;
};

#endif  // XFA_FDE_CFDE_PATH_H_
