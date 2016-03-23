// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXGRAPHICS_CFX_PATH_H_
#define XFA_FXGRAPHICS_CFX_PATH_H_

#include "core/fxcrt/include/fx_system.h"
#include "xfa/fxgraphics/include/cfx_graphics.h"

class CFX_PathData;
class CFX_PathGenerator;

class CFX_Path {
 public:
  CFX_Path();
  virtual ~CFX_Path();

  FX_ERR Create();
  FX_ERR MoveTo(FX_FLOAT x, FX_FLOAT y);
  FX_ERR LineTo(FX_FLOAT x, FX_FLOAT y);
  FX_ERR BezierTo(FX_FLOAT ctrlX1,
                  FX_FLOAT ctrlY1,
                  FX_FLOAT ctrlX2,
                  FX_FLOAT ctrlY2,
                  FX_FLOAT toX,
                  FX_FLOAT toY);
  FX_ERR ArcTo(FX_FLOAT left,
               FX_FLOAT top,
               FX_FLOAT width,
               FX_FLOAT height,
               FX_FLOAT startAngle,
               FX_FLOAT sweepAngle);
  FX_ERR Close();

  FX_ERR AddLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2);
  FX_ERR AddBezier(FX_FLOAT startX,
                   FX_FLOAT startY,
                   FX_FLOAT ctrlX1,
                   FX_FLOAT ctrlY1,
                   FX_FLOAT ctrlX2,
                   FX_FLOAT ctrlY2,
                   FX_FLOAT endX,
                   FX_FLOAT endY);
  FX_ERR AddRectangle(FX_FLOAT left,
                      FX_FLOAT top,
                      FX_FLOAT width,
                      FX_FLOAT height);
  FX_ERR AddEllipse(FX_FLOAT left,
                    FX_FLOAT top,
                    FX_FLOAT width,
                    FX_FLOAT height);
  FX_ERR AddEllipse(const CFX_RectF& rect);
  FX_ERR AddArc(FX_FLOAT left,
                FX_FLOAT top,
                FX_FLOAT width,
                FX_FLOAT height,
                FX_FLOAT startAngle,
                FX_FLOAT sweepAngle);
  FX_ERR AddPie(FX_FLOAT left,
                FX_FLOAT top,
                FX_FLOAT width,
                FX_FLOAT height,
                FX_FLOAT startAngle,
                FX_FLOAT sweepAngle);
  FX_ERR AddSubpath(CFX_Path* path);
  FX_ERR Clear();

  FX_BOOL IsEmpty();
  CFX_PathData* GetPathData();

 private:
  CFX_PathGenerator* m_generator;
};

#endif  // XFA_FXGRAPHICS_CFX_PATH_H_
