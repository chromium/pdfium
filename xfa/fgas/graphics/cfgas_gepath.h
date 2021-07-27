// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_GRAPHICS_CFGAS_GEPATH_H_
#define XFA_FGAS_GRAPHICS_CFGAS_GEPATH_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxge/cfx_path.h"

class CFGAS_GEPath final {
 public:
  CFGAS_GEPath();
  ~CFGAS_GEPath();

  const CFX_Path* GetPath() const { return &path_; }

  void Clear();
  bool IsEmpty() const { return path_.GetPoints().empty(); }
  void TransformBy(const CFX_Matrix& mt);

  void Close();
  void MoveTo(const CFX_PointF& point);
  void LineTo(const CFX_PointF& point);
  void BezierTo(const CFX_PointF& c1,
                const CFX_PointF& c2,
                const CFX_PointF& to);
  void ArcTo(const CFX_PointF& pos,
             const CFX_SizeF& size,
             float startAngle,
             float sweepAngle);

  void AddLine(const CFX_PointF& p1, const CFX_PointF& p2);
  void AddRectangle(float left, float top, float width, float height);
  void AddEllipse(const CFX_RectF& rect);
  void AddArc(const CFX_PointF& pos,
              const CFX_SizeF& size,
              float startAngle,
              float sweepAngle);

  void AddSubpath(const CFGAS_GEPath& path);

 private:
  void ArcToInternal(const CFX_PointF& pos,
                     const CFX_SizeF& size,
                     float start_angle,
                     float sweep_angle);

  CFX_Path path_;
};

#endif  // XFA_FGAS_GRAPHICS_CFGAS_GEPATH_H_
