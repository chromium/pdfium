// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_edittp.h"

#include "xfa/fgas/graphics/cfgas_gecolor.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_widget.h"

CFWL_EditTP::CFWL_EditTP() = default;

CFWL_EditTP::~CFWL_EditTP() = default;

void CFWL_EditTP::DrawBackground(const CFWL_ThemeBackground& pParams) {
  switch (pParams.m_iPart) {
    case CFWL_ThemePart::Part::kBorder: {
      DrawBorder(pParams.GetGraphics(), pParams.m_PartRect, pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kBackground: {
      if (pParams.m_pPath) {
        CFGAS_GEGraphics* pGraphics = pParams.GetGraphics();
        pGraphics->SaveGraphState();
        pGraphics->SetFillColor(CFGAS_GEColor(FWLTHEME_COLOR_BKSelected));
        if (pParams.m_pPath) {
          pGraphics->FillPath(*pParams.m_pPath,
                              CFX_FillRenderOptions::FillType::kWinding,
                              pParams.m_matrix);
        }
        pGraphics->RestoreGraphState();
      } else {
        CFGAS_GEPath path;
        path.AddRectangle(pParams.m_PartRect.left, pParams.m_PartRect.top,
                          pParams.m_PartRect.width, pParams.m_PartRect.height);
        CFGAS_GEColor cr(FWLTHEME_COLOR_Background);
        if (!pParams.m_bStaticBackground) {
          if (pParams.m_dwStates & CFWL_PartState_Disabled)
            cr = CFGAS_GEColor(FWLTHEME_COLOR_EDGERB1);
          else if (pParams.m_dwStates & CFWL_PartState_ReadOnly)
            cr = CFGAS_GEColor(ArgbEncode(255, 236, 233, 216));
          else
            cr = CFGAS_GEColor(0xFFFFFFFF);
        }
        pParams.GetGraphics()->SaveGraphState();
        pParams.GetGraphics()->SetFillColor(cr);
        pParams.GetGraphics()->FillPath(
            path, CFX_FillRenderOptions::FillType::kWinding, pParams.m_matrix);
        pParams.GetGraphics()->RestoreGraphState();
      }
      break;
    }
    case CFWL_ThemePart::Part::kCombTextLine: {
      CFWL_Widget::AdapterIface* pWidget =
          pParams.GetWidget()->GetOutmost()->GetAdapterIface();
      FX_ARGB cr = 0xFF000000;
      float fWidth = 1.0f;
      pWidget->GetBorderColorAndThickness(&cr, &fWidth);
      pParams.GetGraphics()->SetStrokeColor(CFGAS_GEColor(cr));
      pParams.GetGraphics()->SetLineWidth(fWidth);
      if (pParams.m_pPath)
        pParams.GetGraphics()->StrokePath(*pParams.m_pPath, pParams.m_matrix);
      break;
    }
    default:
      break;
  }
}
