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

void CFX_PathGenerator::MoveTo(FX_FLOAT x, FX_FLOAT y) {
  m_pPathData->AppendPoint(x, y, FXPT_TYPE::MoveTo, false);
}

void CFX_PathGenerator::LineTo(FX_FLOAT x, FX_FLOAT y) {
  m_pPathData->AppendPoint(x, y, FXPT_TYPE::LineTo, false);
}

void CFX_PathGenerator::BezierTo(FX_FLOAT ctrl_x1,
                                 FX_FLOAT ctrl_y1,
                                 FX_FLOAT ctrl_x2,
                                 FX_FLOAT ctrl_y2,
                                 FX_FLOAT to_x,
                                 FX_FLOAT to_y) {
  m_pPathData->AppendPoint(ctrl_x1, ctrl_y1, FXPT_TYPE::BezierTo, false);
  m_pPathData->AppendPoint(ctrl_x2, ctrl_y2, FXPT_TYPE::BezierTo, false);
  m_pPathData->AppendPoint(to_x, to_y, FXPT_TYPE::BezierTo, false);
}

void CFX_PathGenerator::Close() {
  m_pPathData->ClosePath();
}

void CFX_PathGenerator::AddLine(FX_FLOAT x1,
                                FX_FLOAT y1,
                                FX_FLOAT x2,
                                FX_FLOAT y2) {
  m_pPathData->AppendPoint(x1, y1, FXPT_TYPE::MoveTo, false);
  m_pPathData->AppendPoint(x2, y2, FXPT_TYPE::LineTo, false);
}

void CFX_PathGenerator::AddBezier(FX_FLOAT start_x,
                                  FX_FLOAT start_y,
                                  FX_FLOAT ctrl_x1,
                                  FX_FLOAT ctrl_y1,
                                  FX_FLOAT ctrl_x2,
                                  FX_FLOAT ctrl_y2,
                                  FX_FLOAT end_x,
                                  FX_FLOAT end_y) {
  m_pPathData->AppendPoint(start_x, start_y, FXPT_TYPE::MoveTo, false);
  m_pPathData->AppendPoint(ctrl_x1, ctrl_y1, FXPT_TYPE::BezierTo, false);
  m_pPathData->AppendPoint(ctrl_x2, ctrl_y2, FXPT_TYPE::BezierTo, false);
  m_pPathData->AppendPoint(end_x, end_y, FXPT_TYPE::BezierTo, false);
}

void CFX_PathGenerator::AddRectangle(FX_FLOAT x1,
                                     FX_FLOAT y1,
                                     FX_FLOAT x2,
                                     FX_FLOAT y2) {
  m_pPathData->AppendRect(x1, y1, x2, y2);
}

void CFX_PathGenerator::AddEllipse(FX_FLOAT x,
                                   FX_FLOAT y,
                                   FX_FLOAT width,
                                   FX_FLOAT height) {
  AddArc(x, y, width, height, 0, FX_PI * 2);
}

void CFX_PathGenerator::ArcTo(FX_FLOAT x,
                              FX_FLOAT y,
                              FX_FLOAT width,
                              FX_FLOAT height,
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

  FX_FLOAT bezier_x, bezier_y;
  bezier_x = x + (width * ((px[0] * cs) - (py[0] * sn)));
  bezier_y = y + (height * ((px[0] * sn) + (py[0] * cs)));
  m_pPathData->AppendPoint(bezier_x, bezier_y, FXPT_TYPE::BezierTo, false);
  bezier_x = x + (width * ((px[1] * cs) - (py[1] * sn)));
  bezier_y = y + (height * ((px[1] * sn) + (py[1] * cs)));
  m_pPathData->AppendPoint(bezier_x, bezier_y, FXPT_TYPE::BezierTo, false);
  bezier_x = x + (width * FXSYS_cos(start_angle + sweep_angle));
  bezier_y = y + (height * FXSYS_sin(start_angle + sweep_angle));
  m_pPathData->AppendPoint(bezier_x, bezier_y, FXPT_TYPE::BezierTo, false);
}

void CFX_PathGenerator::AddArc(FX_FLOAT x,
                               FX_FLOAT y,
                               FX_FLOAT width,
                               FX_FLOAT height,
                               FX_FLOAT start_angle,
                               FX_FLOAT sweep_angle) {
  if (sweep_angle == 0) {
    return;
  }

  const FX_FLOAT bezier_arc_angle_epsilon = 0.01f;
  while (start_angle > FX_PI * 2) {
    start_angle -= FX_PI * 2;
  }
  while (start_angle < 0) {
    start_angle += FX_PI * 2;
  }
  if (sweep_angle >= FX_PI * 2) {
    sweep_angle = FX_PI * 2;
  }
  if (sweep_angle <= -FX_PI * 2) {
    sweep_angle = -FX_PI * 2;
  }

  m_pPathData->AppendPoint(x + (width * FXSYS_cos(start_angle)),
                           y + (height * FXSYS_sin(start_angle)),
                           FXPT_TYPE::MoveTo, false);
  FX_FLOAT total_sweep = 0, local_sweep = 0, prev_sweep = 0;
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
    ArcTo(x, y, width, height, start_angle, local_sweep);
    start_angle += local_sweep;
  } while (!done);
}

void CFX_PathGenerator::AddPie(FX_FLOAT x,
                               FX_FLOAT y,
                               FX_FLOAT width,
                               FX_FLOAT height,
                               FX_FLOAT start_angle,
                               FX_FLOAT sweep_angle) {
  if (sweep_angle == 0) {
    m_pPathData->AppendPoint(x, y, FXPT_TYPE::MoveTo, false);
    m_pPathData->AppendPoint(x + (width * FXSYS_cos(start_angle)),
                             y + (height * FXSYS_sin(start_angle)),
                             FXPT_TYPE::LineTo, false);
    return;
  }
  AddArc(x, y, width, height, start_angle, sweep_angle);
  m_pPathData->AppendPoint(x, y, FXPT_TYPE::LineTo, true);
}
