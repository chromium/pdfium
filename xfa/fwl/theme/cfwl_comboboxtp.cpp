// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_comboboxtp.h"

#include "xfa/fgas/graphics/cfgas_gecolor.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fwl/cfwl_combobox.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/ifwl_themeprovider.h"

CFWL_ComboBoxTP::CFWL_ComboBoxTP() = default;

CFWL_ComboBoxTP::~CFWL_ComboBoxTP() = default;

void CFWL_ComboBoxTP::DrawBackground(const CFWL_ThemeBackground& pParams) {
  switch (pParams.GetPart()) {
    case CFWL_ThemePart::Part::kBorder: {
      DrawBorder(pParams.GetGraphics(), pParams.m_PartRect, pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kBackground: {
      CFGAS_GEPath path;
      const CFX_RectF& rect = pParams.m_PartRect;
      path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
      FX_ARGB argb_color;
      if (pParams.m_dwStates & CFWL_PartState::kSelected)
        argb_color = FWLTHEME_COLOR_BKSelected;
      else if (pParams.m_dwStates & CFWL_PartState::kDisabled)
        argb_color = FWLTHEME_COLOR_EDGERB1;
      else
        argb_color = 0xFFFFFFFF;

      CFGAS_GEGraphics::StateRestorer restorer(pParams.GetGraphics());
      pParams.GetGraphics()->SetFillColor(CFGAS_GEColor(argb_color));
      pParams.GetGraphics()->FillPath(
          path, CFX_FillRenderOptions::FillType::kWinding, pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kDropDownButton: {
      DrawArrowBtn(pParams.GetGraphics(), pParams.m_PartRect,
                   FWLTHEME_DIRECTION::kDown, pParams.GetThemeState(),
                   pParams.m_matrix);
      break;
    }
    default:
      break;
  }
}
