// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXGRAPHICS_CFX_PATH_H_
#define XFA_FXGRAPHICS_CFX_PATH_H_

#include <memory>

#include "core/fxcrt/fx_system.h"
#include "xfa/fxgraphics/cfx_graphics.h"

class CFX_PathData;
class CFX_PathGenerator;

class CFX_Path final {
 public:
  CFX_Path();
  ~CFX_Path();

  FWL_Error Create();
  FWL_Error MoveTo(const CFX_PointF& point);
  FWL_Error LineTo(const CFX_PointF& point);
  FWL_Error BezierTo(const CFX_PointF& c1,
                     const CFX_PointF& c2,
                     const CFX_PointF& to);
  FWL_Error ArcTo(const CFX_PointF& pos,
                  const CFX_SizeF& size,
                  FX_FLOAT startAngle,
                  FX_FLOAT sweepAngle);
  FWL_Error Close();

  FWL_Error AddLine(const CFX_PointF& p1, const CFX_PointF& p2);
  FWL_Error AddBezier(const CFX_PointF& p1,
                      const CFX_PointF& c1,
                      const CFX_PointF& c2,
                      const CFX_PointF& p2);
  FWL_Error AddRectangle(FX_FLOAT left,
                         FX_FLOAT top,
                         FX_FLOAT width,
                         FX_FLOAT height);
  FWL_Error AddEllipse(const CFX_PointF& pos, const CFX_SizeF& size);
  FWL_Error AddEllipse(const CFX_RectF& rect);
  FWL_Error AddArc(const CFX_PointF& pos,
                   const CFX_SizeF& size,
                   FX_FLOAT startAngle,
                   FX_FLOAT sweepAngle);
  FWL_Error AddPie(const CFX_PointF& pos,
                   const CFX_SizeF& size,
                   FX_FLOAT startAngle,
                   FX_FLOAT sweepAngle);
  FWL_Error AddSubpath(CFX_Path* path);
  FWL_Error Clear();

  bool IsEmpty() const;
  CFX_PathData* GetPathData() const;

 private:
  std::unique_ptr<CFX_PathGenerator> m_generator;
};

#endif  // XFA_FXGRAPHICS_CFX_PATH_H_
