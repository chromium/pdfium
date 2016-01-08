// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_GRAPHOBJS_IMP
#define _FDE_GRAPHOBJS_IMP
#ifndef _FDEPLUS
#include "xfa/src/foxitlib.h"
class CFDE_GEFontMgr;
class CFDE_Path : public IFDE_Path, public CFX_Target {
 public:
  virtual void Release() { delete this; }

  virtual FX_BOOL StartFigure();
  virtual FX_BOOL CloseFigure();

  virtual void AddBezier(const CFX_PointsF& points);
  virtual void AddBeziers(const CFX_PointsF& points);
  virtual void AddCurve(const CFX_PointsF& points,
                        FX_BOOL bClosed,
                        FX_FLOAT fTension = 0.5f);
  virtual void AddEllipse(const CFX_RectF& rect);
  virtual void AddLines(const CFX_PointsF& points);
  virtual void AddLine(const CFX_PointF& pt1, const CFX_PointF& pt2);
  virtual void AddPath(const IFDE_Path* pSrc, FX_BOOL bConnect);
  virtual void AddPolygon(const CFX_PointsF& points);
  virtual void AddRectangle(const CFX_RectF& rect);
  virtual void GetBBox(CFX_RectF& bbox) const;
  virtual void GetBBox(CFX_RectF& bbox,
                       FX_FLOAT fLineWidth,
                       FX_FLOAT fMiterLimit) const;
  FX_PATHPOINT* AddPoints(int32_t iCount);
  FX_PATHPOINT* GetLastPoint(int32_t iCount = 1) const;
  FX_BOOL FigureClosed() const;
  void MoveTo(FX_FLOAT fx, FX_FLOAT fy);
  void LineTo(FX_FLOAT fx, FX_FLOAT fy);
  void BezierTo(const CFX_PointF& p1,
                const CFX_PointF& p2,
                const CFX_PointF& p3);
  void ArcTo(FX_BOOL bStart,
             const CFX_RectF& rect,
             FX_FLOAT startAngle,
             FX_FLOAT endAngle);
  void MoveTo(const CFX_PointF& p0) { MoveTo(p0.x, p0.y); }
  void LineTo(const CFX_PointF& p1) { LineTo(p1.x, p1.y); }
  void GetCurveTangents(const CFX_PointsF& points,
                        CFX_PointsF& tangents,
                        FX_BOOL bClosed,
                        FX_FLOAT fTension) const;
  CFX_PathData m_Path;
};
#endif
#endif
