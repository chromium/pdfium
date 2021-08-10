// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_carettp.h"

#include "xfa/fgas/graphics/cfgas_gecolor.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fwl/cfwl_caret.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_widget.h"

CFWL_CaretTP::CFWL_CaretTP() = default;

CFWL_CaretTP::~CFWL_CaretTP() = default;

void CFWL_CaretTP::DrawBackground(const CFWL_ThemeBackground& pParams) {
  switch (pParams.m_iPart) {
    case CFWL_ThemePart::Part::kBackground: {
      if (!(pParams.m_dwStates & CFWL_PartState::kHightLight))
        return;

      DrawCaretBK(pParams.GetGraphics(), pParams.m_dwStates, pParams.m_PartRect,
                  pParams.m_matrix);
      break;
    }
    default:
      break;
  }
}

void CFWL_CaretTP::DrawCaretBK(CFGAS_GEGraphics* pGraphics,
                               Mask<CFWL_PartState> dwStates,
                               const CFX_RectF& rect,
                               const CFX_Matrix& matrix) {
  CFGAS_GEPath path;
  path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
  pGraphics->SetFillColor(CFGAS_GEColor(ArgbEncode(255, 0, 0, 0)));
  pGraphics->FillPath(path, CFX_FillRenderOptions::FillType::kWinding, matrix);
}
