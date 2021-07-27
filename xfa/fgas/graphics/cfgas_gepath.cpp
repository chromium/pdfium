// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/graphics/cfgas_gepath.h"

#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_path.h"

CFGAS_GEPath::CFGAS_GEPath() = default;

CFGAS_GEPath::~CFGAS_GEPath() = default;

void CFGAS_GEPath::Clear() {
  path_.Clear();
}

void CFGAS_GEPath::Close() {
  path_.ClosePath();
}

void CFGAS_GEPath::MoveTo(const CFX_PointF& point) {
  path_.AppendPoint(point, CFX_Path::Point::Type::kMove);
}

void CFGAS_GEPath::LineTo(const CFX_PointF& point) {
  path_.AppendPoint(point, CFX_Path::Point::Type::kLine);
}

void CFGAS_GEPath::BezierTo(const CFX_PointF& c1,
                            const CFX_PointF& c2,
                            const CFX_PointF& to) {
  path_.AppendPoint(c1, CFX_Path::Point::Type::kBezier);
  path_.AppendPoint(c2, CFX_Path::Point::Type::kBezier);
  path_.AppendPoint(to, CFX_Path::Point::Type::kBezier);
}

void CFGAS_GEPath::ArcTo(const CFX_PointF& pos,
                         const CFX_SizeF& size,
                         float start_angle,
                         float sweep_angle) {
  CFX_SizeF new_size = size / 2.0f;
  ArcToInternal(CFX_PointF(pos.x + new_size.width, pos.y + new_size.height),
                new_size, start_angle, sweep_angle);
}

void CFGAS_GEPath::ArcToInternal(const CFX_PointF& pos,
                                 const CFX_SizeF& size,
                                 float start_angle,
                                 float sweep_angle) {
  float x0 = cos(sweep_angle / 2);
  float y0 = sin(sweep_angle / 2);
  float tx = ((1.0f - x0) * 4) / (3 * 1.0f);
  float ty = y0 - ((tx * x0) / y0);

  CFX_PointF points[] = {CFX_PointF(x0 + tx, -ty), CFX_PointF(x0 + tx, ty)};
  float sn = sin(start_angle + sweep_angle / 2);
  float cs = cos(start_angle + sweep_angle / 2);

  CFX_PointF bezier;
  bezier.x = pos.x + (size.width * ((points[0].x * cs) - (points[0].y * sn)));
  bezier.y = pos.y + (size.height * ((points[0].x * sn) + (points[0].y * cs)));
  path_.AppendPoint(bezier, CFX_Path::Point::Type::kBezier);

  bezier.x = pos.x + (size.width * ((points[1].x * cs) - (points[1].y * sn)));
  bezier.y = pos.y + (size.height * ((points[1].x * sn) + (points[1].y * cs)));
  path_.AppendPoint(bezier, CFX_Path::Point::Type::kBezier);

  bezier.x = pos.x + (size.width * cos(start_angle + sweep_angle));
  bezier.y = pos.y + (size.height * sin(start_angle + sweep_angle));
  path_.AppendPoint(bezier, CFX_Path::Point::Type::kBezier);
}

void CFGAS_GEPath::AddLine(const CFX_PointF& p1, const CFX_PointF& p2) {
  path_.AppendPoint(p1, CFX_Path::Point::Type::kMove);
  path_.AppendPoint(p2, CFX_Path::Point::Type::kLine);
}

void CFGAS_GEPath::AddRectangle(float left,
                                float top,
                                float width,
                                float height) {
  path_.AppendRect(left, top, left + width, top + height);
}

void CFGAS_GEPath::AddEllipse(const CFX_RectF& rect) {
  AddArc(rect.TopLeft(), rect.Size(), 0, FXSYS_PI * 2);
}

void CFGAS_GEPath::AddArc(const CFX_PointF& original_pos,
                          const CFX_SizeF& original_size,
                          float start_angle,
                          float sweep_angle) {
  if (sweep_angle == 0)
    return;

  const float bezier_arc_angle_epsilon = 0.01f;
  while (start_angle > FXSYS_PI * 2)
    start_angle -= FXSYS_PI * 2;
  while (start_angle < 0)
    start_angle += FXSYS_PI * 2;
  if (sweep_angle >= FXSYS_PI * 2)
    sweep_angle = FXSYS_PI * 2;
  if (sweep_angle <= -FXSYS_PI * 2)
    sweep_angle = -FXSYS_PI * 2;

  CFX_SizeF size = original_size / 2;
  CFX_PointF pos(original_pos.x + size.width, original_pos.y + size.height);
  path_.AppendPoint(pos + CFX_PointF(size.width * cos(start_angle),
                                     size.height * sin(start_angle)),
                    CFX_Path::Point::Type::kMove);

  float total_sweep = 0;
  float local_sweep = 0;
  float prev_sweep = 0;
  bool done = false;
  do {
    if (sweep_angle < 0) {
      prev_sweep = total_sweep;
      local_sweep = -FXSYS_PI / 2;
      total_sweep -= FXSYS_PI / 2;
      if (total_sweep <= sweep_angle + bezier_arc_angle_epsilon) {
        local_sweep = sweep_angle - prev_sweep;
        done = true;
      }
    } else {
      prev_sweep = total_sweep;
      local_sweep = FXSYS_PI / 2;
      total_sweep += FXSYS_PI / 2;
      if (total_sweep >= sweep_angle - bezier_arc_angle_epsilon) {
        local_sweep = sweep_angle - prev_sweep;
        done = true;
      }
    }

    ArcToInternal(pos, size, start_angle, local_sweep);
    start_angle += local_sweep;
  } while (!done);
}

void CFGAS_GEPath::AddSubpath(const CFGAS_GEPath& path) {
  path_.Append(path.path_, nullptr);
}

void CFGAS_GEPath::TransformBy(const CFX_Matrix& mt) {
  path_.Transform(mt);
}
