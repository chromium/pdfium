// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXGRAPHICS_CFX_PATH_GENERATOR_H_
#define XFA_FXGRAPHICS_CFX_PATH_GENERATOR_H_

#include <memory>

#include "core/fxge/cfx_pathdata.h"

class CFX_PathGenerator {
 public:
  CFX_PathGenerator();
  ~CFX_PathGenerator();

  CFX_PathData* GetPathData() const { return m_pPathData.get(); }

  void AddPathData(CFX_PathData* path_data);

  void MoveTo(const CFX_PointF& point);
  void LineTo(const CFX_PointF& point);
  void BezierTo(const CFX_PointF& c1,
                const CFX_PointF& c2,
                const CFX_PointF& to);
  void Close();
  void ArcTo(const CFX_PointF& point,
             const CFX_SizeF& size,
             FX_FLOAT start_angle,
             FX_FLOAT sweep_angle);

  void AddLine(const CFX_PointF& p1, const CFX_PointF& p2);
  void AddBezier(const CFX_PointF& p1,
                 const CFX_PointF& c1,
                 const CFX_PointF& c2,
                 const CFX_PointF& p2);
  void AddRectangle(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2);
  void AddEllipse(const CFX_PointF& point, const CFX_SizeF& size);
  void AddArc(const CFX_PointF& point,
              const CFX_SizeF& size,
              FX_FLOAT start_angle,
              FX_FLOAT sweep_angle);
  void AddPie(const CFX_PointF& point,
              const CFX_SizeF& size,
              FX_FLOAT start_angle,
              FX_FLOAT sweep_angle);

 protected:
  std::unique_ptr<CFX_PathData> m_pPathData;
};

#endif  // XFA_FXGRAPHICS_CFX_PATH_GENERATOR_H_
