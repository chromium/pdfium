// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxgraphics/cfx_path.h"

#include "core/fxge/cfx_pathdata.h"
#include "third_party/base/ptr_util.h"

CFX_Path::CFX_Path() {}

CFX_Path::~CFX_Path() {}

void CFX_Path::Clear() {
  data_.Clear();
}

void CFX_Path::Close() {
  data_.ClosePath();
}

void CFX_Path::MoveTo(const CFX_PointF& point) {
  data_.AppendPoint(point, FXPT_TYPE::MoveTo, false);
}

void CFX_Path::LineTo(const CFX_PointF& point) {
  data_.AppendPoint(point, FXPT_TYPE::LineTo, false);
}

void CFX_Path::BezierTo(const CFX_PointF& c1,
                        const CFX_PointF& c2,
                        const CFX_PointF& to) {
  data_.AppendPoint(c1, FXPT_TYPE::BezierTo, false);
  data_.AppendPoint(c2, FXPT_TYPE::BezierTo, false);
  data_.AppendPoint(to, FXPT_TYPE::BezierTo, false);
}

void CFX_Path::ArcTo(const CFX_PointF& pos,
                     const CFX_SizeF& size,
                     FX_FLOAT start_angle,
                     FX_FLOAT sweep_angle) {
  CFX_SizeF new_size = size / 2.0f;
  ArcToInternal(CFX_PointF(pos.x + new_size.width, pos.y + new_size.height),
                new_size, start_angle, sweep_angle);
}

void CFX_Path::ArcToInternal(const CFX_PointF& pos,
                             const CFX_SizeF& size,
                             FX_FLOAT start_angle,
                             FX_FLOAT sweep_angle) {
  FX_FLOAT x0 = FXSYS_cos(sweep_angle / 2);
  FX_FLOAT y0 = FXSYS_sin(sweep_angle / 2);
  FX_FLOAT tx = ((1.0f - x0) * 4) / (3 * 1.0f);
  FX_FLOAT ty = y0 - ((tx * x0) / y0);

  CFX_PointF points[] = {CFX_PointF(x0 + tx, -ty), CFX_PointF(x0 + tx, ty)};
  FX_FLOAT sn = FXSYS_sin(start_angle + sweep_angle / 2);
  FX_FLOAT cs = FXSYS_cos(start_angle + sweep_angle / 2);

  CFX_PointF bezier;
  bezier.x = pos.x + (size.width * ((points[0].x * cs) - (points[0].y * sn)));
  bezier.y = pos.y + (size.height * ((points[0].x * sn) + (points[0].y * cs)));
  data_.AppendPoint(bezier, FXPT_TYPE::BezierTo, false);

  bezier.x = pos.x + (size.width * ((points[1].x * cs) - (points[1].y * sn)));
  bezier.y = pos.y + (size.height * ((points[1].x * sn) + (points[1].y * cs)));
  data_.AppendPoint(bezier, FXPT_TYPE::BezierTo, false);

  bezier.x = pos.x + (size.width * FXSYS_cos(start_angle + sweep_angle));
  bezier.y = pos.y + (size.height * FXSYS_sin(start_angle + sweep_angle));
  data_.AppendPoint(bezier, FXPT_TYPE::BezierTo, false);
}

void CFX_Path::AddLine(const CFX_PointF& p1, const CFX_PointF& p2) {
  data_.AppendPoint(p1, FXPT_TYPE::MoveTo, false);
  data_.AppendPoint(p2, FXPT_TYPE::LineTo, false);
}

void CFX_Path::AddRectangle(FX_FLOAT left,
                            FX_FLOAT top,
                            FX_FLOAT width,
                            FX_FLOAT height) {
  data_.AppendRect(left, top, left + width, top + height);
}

void CFX_Path::AddEllipse(const CFX_RectF& rect) {
  AddArc(rect.TopLeft(), rect.Size(), 0, FX_PI * 2);
}

void CFX_Path::AddArc(const CFX_PointF& original_pos,
                      const CFX_SizeF& original_size,
                      FX_FLOAT start_angle,
                      FX_FLOAT sweep_angle) {
  if (sweep_angle == 0)
    return;

  const FX_FLOAT bezier_arc_angle_epsilon = 0.01f;
  while (start_angle > FX_PI * 2)
    start_angle -= FX_PI * 2;
  while (start_angle < 0)
    start_angle += FX_PI * 2;
  if (sweep_angle >= FX_PI * 2)
    sweep_angle = FX_PI * 2;
  if (sweep_angle <= -FX_PI * 2)
    sweep_angle = -FX_PI * 2;

  CFX_SizeF size = original_size / 2;
  CFX_PointF pos(original_pos.x + size.width, original_pos.y + size.height);
  data_.AppendPoint(pos + CFX_PointF(size.width * FXSYS_cos(start_angle),
                                     size.height * FXSYS_sin(start_angle)),
                    FXPT_TYPE::MoveTo, false);

  FX_FLOAT total_sweep = 0;
  FX_FLOAT local_sweep = 0;
  FX_FLOAT prev_sweep = 0;
  bool done = false;
  do {
    if (sweep_angle < 0) {
      prev_sweep = total_sweep;
      local_sweep = -FX_PI / 2;
      total_sweep -= FX_PI / 2;
      if (total_sweep <= sweep_angle + bezier_arc_angle_epsilon) {
        local_sweep = sweep_angle - prev_sweep;
        done = true;
      }
    } else {
      prev_sweep = total_sweep;
      local_sweep = FX_PI / 2;
      total_sweep += FX_PI / 2;
      if (total_sweep >= sweep_angle - bezier_arc_angle_epsilon) {
        local_sweep = sweep_angle - prev_sweep;
        done = true;
      }
    }

    ArcToInternal(pos, size, start_angle, local_sweep);
    start_angle += local_sweep;
  } while (!done);
}

void CFX_Path::AddSubpath(CFX_Path* path) {
  if (!path)
    return;
  data_.Append(&path->data_, nullptr);
}

void CFX_Path::TransformBy(const CFX_Matrix& mt) {
  data_.Transform(&mt);
}
