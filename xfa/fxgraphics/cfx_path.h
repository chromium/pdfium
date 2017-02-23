// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXGRAPHICS_CFX_PATH_H_
#define XFA_FXGRAPHICS_CFX_PATH_H_

#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_pathdata.h"
#include "xfa/fxgraphics/cfx_graphics.h"

class CFX_Path final {
 public:
  CFX_Path();
  ~CFX_Path();

  const CFX_PathData* GetPathData() const { return &data_; }

  void Clear();
  bool IsEmpty() const { return data_.GetPoints().empty(); }
  void TransformBy(const CFX_Matrix& mt);

  void Close();
  void MoveTo(const CFX_PointF& point);
  void LineTo(const CFX_PointF& point);
  void BezierTo(const CFX_PointF& c1,
                const CFX_PointF& c2,
                const CFX_PointF& to);
  void ArcTo(const CFX_PointF& pos,
             const CFX_SizeF& size,
             FX_FLOAT startAngle,
             FX_FLOAT sweepAngle);

  void AddLine(const CFX_PointF& p1, const CFX_PointF& p2);
  void AddRectangle(FX_FLOAT left,
                    FX_FLOAT top,
                    FX_FLOAT width,
                    FX_FLOAT height);
  void AddEllipse(const CFX_RectF& rect);
  void AddArc(const CFX_PointF& pos,
              const CFX_SizeF& size,
              FX_FLOAT startAngle,
              FX_FLOAT sweepAngle);

  void AddSubpath(CFX_Path* path);

 private:
  void ArcToInternal(const CFX_PointF& pos,
                     const CFX_SizeF& size,
                     FX_FLOAT start_angle,
                     FX_FLOAT sweep_angle);

  CFX_PathData data_;
};

#endif  // XFA_FXGRAPHICS_CFX_PATH_H_
