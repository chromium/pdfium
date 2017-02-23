// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxgraphics/cfx_path_generator.h"

#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"

CFX_PathGenerator::CFX_PathGenerator() : m_pPathData(new CFX_PathData) {}

CFX_PathGenerator::~CFX_PathGenerator() {}

void CFX_PathGenerator::AddPathData(CFX_PathData* pPathData) {
  if (!pPathData)
    return;
  m_pPathData->Append(pPathData, nullptr);
}

void CFX_PathGenerator::MoveTo(const CFX_PointF& point) {
  m_pPathData->AppendPoint(point, FXPT_TYPE::MoveTo, false);
}

void CFX_PathGenerator::LineTo(const CFX_PointF& point) {
  m_pPathData->AppendPoint(point, FXPT_TYPE::LineTo, false);
}

void CFX_PathGenerator::BezierTo(const CFX_PointF& c1,
                                 const CFX_PointF& c2,
                                 const CFX_PointF& to) {
  m_pPathData->AppendPoint(c1, FXPT_TYPE::BezierTo, false);
  m_pPathData->AppendPoint(c2, FXPT_TYPE::BezierTo, false);
  m_pPathData->AppendPoint(to, FXPT_TYPE::BezierTo, false);
}

void CFX_PathGenerator::Close() {
  m_pPathData->ClosePath();
}

void CFX_PathGenerator::AddLine(const CFX_PointF& p1, const CFX_PointF& p2) {
  m_pPathData->AppendPoint(p1, FXPT_TYPE::MoveTo, false);
  m_pPathData->AppendPoint(p2, FXPT_TYPE::LineTo, false);
}

void CFX_PathGenerator::AddBezier(const CFX_PointF& p1,
                                  const CFX_PointF& c1,
                                  const CFX_PointF& c2,
                                  const CFX_PointF& p2) {
  m_pPathData->AppendPoint(p1, FXPT_TYPE::MoveTo, false);
  m_pPathData->AppendPoint(c1, FXPT_TYPE::BezierTo, false);
  m_pPathData->AppendPoint(c2, FXPT_TYPE::BezierTo, false);
  m_pPathData->AppendPoint(p2, FXPT_TYPE::BezierTo, false);
}

void CFX_PathGenerator::AddRectangle(FX_FLOAT x1,
                                     FX_FLOAT y1,
                                     FX_FLOAT x2,
                                     FX_FLOAT y2) {
  m_pPathData->AppendRect(x1, y1, x2, y2);
}

void CFX_PathGenerator::AddEllipse(const CFX_PointF& pos,
                                   const CFX_SizeF& size) {
  AddArc(pos, size, 0, FX_PI * 2);
}

void CFX_PathGenerator::ArcTo(const CFX_PointF& pos,
                              const CFX_SizeF& size,
                              FX_FLOAT start_angle,
                              FX_FLOAT sweep_angle) {
  FX_FLOAT x0 = FXSYS_cos(sweep_angle / 2);
  FX_FLOAT y0 = FXSYS_sin(sweep_angle / 2);
  FX_FLOAT tx = ((1.0f - x0) * 4) / (3 * 1.0f);
  FX_FLOAT ty = y0 - ((tx * x0) / y0);
  FX_FLOAT px[3], py[3];
  px[0] = x0 + tx;
  py[0] = -ty;
  px[1] = x0 + tx;
  py[1] = ty;
  FX_FLOAT sn = FXSYS_sin(start_angle + sweep_angle / 2);
  FX_FLOAT cs = FXSYS_cos(start_angle + sweep_angle / 2);

  CFX_PointF bezier;
  bezier.x = pos.x + (size.width * ((px[0] * cs) - (py[0] * sn)));
  bezier.y = pos.y + (size.height * ((px[0] * sn) + (py[0] * cs)));
  m_pPathData->AppendPoint(bezier, FXPT_TYPE::BezierTo, false);

  bezier.x = pos.x + (size.width * ((px[1] * cs) - (py[1] * sn)));
  bezier.y = pos.y + (size.height * ((px[1] * sn) + (py[1] * cs)));
  m_pPathData->AppendPoint(bezier, FXPT_TYPE::BezierTo, false);

  bezier.x = pos.x + (size.width * FXSYS_cos(start_angle + sweep_angle));
  bezier.y = pos.y + (size.height * FXSYS_sin(start_angle + sweep_angle));
  m_pPathData->AppendPoint(bezier, FXPT_TYPE::BezierTo, false);
}

void CFX_PathGenerator::AddArc(const CFX_PointF& pos,
                               const CFX_SizeF& size,
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

  m_pPathData->AppendPoint(
      pos + CFX_PointF(size.width * FXSYS_cos(start_angle),
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

    ArcTo(pos, size, start_angle, local_sweep);
    start_angle += local_sweep;
  } while (!done);
}

void CFX_PathGenerator::AddPie(const CFX_PointF& pos,
                               const CFX_SizeF& size,
                               FX_FLOAT start_angle,
                               FX_FLOAT sweep_angle) {
  if (sweep_angle == 0) {
    m_pPathData->AppendPoint(pos, FXPT_TYPE::MoveTo, false);
    m_pPathData->AppendPoint(
        pos + CFX_PointF(size.width * FXSYS_cos(start_angle),
                         size.height * FXSYS_sin(start_angle)),
        FXPT_TYPE::LineTo, false);
    return;
  }

  AddArc(pos, size, start_angle, sweep_angle);
  m_pPathData->AppendPoint(pos, FXPT_TYPE::LineTo, true);
}
