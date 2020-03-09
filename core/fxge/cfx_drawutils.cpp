// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_drawutils.h"

#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"

// static
void CFX_DrawUtils::DrawFocusRect(CFX_RenderDevice* render_device,
                                  const CFX_Matrix& user_to_device,
                                  const CFX_FloatRect& view_bounding_box) {
  ASSERT(render_device);
  CFX_PathData path;
  path.AppendPoint(CFX_PointF(view_bounding_box.left, view_bounding_box.top),
                   FXPT_TYPE::MoveTo, /*closeFigure=*/false);
  path.AppendPoint(CFX_PointF(view_bounding_box.left, view_bounding_box.bottom),
                   FXPT_TYPE::LineTo, /*closeFigure=*/false);
  path.AppendPoint(
      CFX_PointF(view_bounding_box.right, view_bounding_box.bottom),
      FXPT_TYPE::LineTo, /*closeFigure=*/false);
  path.AppendPoint(CFX_PointF(view_bounding_box.right, view_bounding_box.top),
                   FXPT_TYPE::LineTo, /*closeFigure=*/false);
  path.AppendPoint(CFX_PointF(view_bounding_box.left, view_bounding_box.top),
                   FXPT_TYPE::LineTo, /*closeFigure=*/false);

  CFX_GraphStateData graph_state_data;
  graph_state_data.m_DashArray = {1.0f};
  graph_state_data.m_DashPhase = 0;
  graph_state_data.m_LineWidth = 1.0f;

  render_device->DrawPath(&path, &user_to_device, &graph_state_data, 0,
                          ArgbEncode(255, 0, 0, 0), FXFILL_ALTERNATE);
}
