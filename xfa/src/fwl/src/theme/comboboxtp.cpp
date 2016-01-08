// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#define FWLTHEME_CAPACITY_ComboFormHandler 8.0f
CFWL_ComboBoxTP::CFWL_ComboBoxTP() {
  m_dwThemeID = 0;
}
CFWL_ComboBoxTP::~CFWL_ComboBoxTP() {}
FX_BOOL CFWL_ComboBoxTP::IsValidWidget(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FALSE;
  return pWidget->GetClassID() == FWL_CLASSHASH_ComboBox;
}
FX_BOOL CFWL_ComboBoxTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (!pParams)
    return FALSE;
  switch (pParams->m_iPart) {
    case FWL_PART_CMB_Border: {
      DrawBorder(pParams->m_pGraphics, &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_CMB_Edge: {
      DrawEdge(pParams->m_pGraphics, pParams->m_pWidget->GetStyles(),
               &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_CMB_Background: {
      CFX_Path path;
      path.Create();
      CFX_RectF& rect = pParams->m_rtPart;
      path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
      CFX_Color cr;
      switch (pParams->m_dwStates) {
        case FWL_PARTSTATE_CMB_Selected:
          cr = FWLTHEME_COLOR_BKSelected;
          break;
        case FWL_PARTSTATE_CMB_Disabled:
          cr = FWLTHEME_COLOR_EDGERB1;
          break;
        default:
          cr = 0xFFFFFFFF;
      }
      pParams->m_pGraphics->SaveGraphState();
      pParams->m_pGraphics->SetFillColor(&cr);
      pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, &pParams->m_matrix);
      pParams->m_pGraphics->RestoreGraphState();
      break;
    }
    case FWL_PART_CMB_DropDownButton: {
      DrawDropDownButton(pParams, pParams->m_dwStates, &pParams->m_matrix);
      break;
    }
    case FWL_PART_CMB_StretcgHandler: {
      DrawStrethHandler(pParams, 0, &pParams->m_matrix);
      break;
    }
    default: { return FALSE; }
  }
  return TRUE;
}
void CFWL_ComboBoxTP::DrawStrethHandler(CFWL_ThemeBackground* pParams,
                                        FX_DWORD dwStates,
                                        CFX_Matrix* pMatrix) {
  CFX_Path path;
  path.Create();
  path.AddRectangle(pParams->m_rtPart.left, pParams->m_rtPart.top,
                    pParams->m_rtPart.width - 1, pParams->m_rtPart.height);
  CFX_Color cr(ArgbEncode(0xff, 0xff, 0, 0));
  pParams->m_pGraphics->SetFillColor(&cr);
  pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, &pParams->m_matrix);
}
void* CFWL_ComboBoxTP::GetCapacity(CFWL_ThemePart* pThemePart,
                                   FX_DWORD dwCapacity) {
  if (dwCapacity == FWL_WGTCAPACITY_CMB_ComboFormHandler) {
    m_fValue = FWLTHEME_CAPACITY_ComboFormHandler;
    return &m_fValue;
  }
  return CFWL_WidgetTP::GetCapacity(pThemePart, dwCapacity);
}
#ifdef THEME_XPSimilar
void CFWL_ComboBoxTP::DrawDropDownButton(CFWL_ThemeBackground* pParams,
                                         FX_DWORD dwStates,
                                         CFX_Matrix* pMatrix) {
  FWLTHEME_STATE eState = FWLTHEME_STATE_Normal;
  switch (dwStates) {
    case FWL_PARTSTATE_CMB_Normal: {
      eState = FWLTHEME_STATE_Normal;
      break;
    }
    case FWL_PARTSTATE_CMB_Hovered: {
      eState = FWLTHEME_STATE_Hover;
      break;
    }
    case FWL_PARTSTATE_CMB_Pressed: {
      eState = FWLTHEME_STATE_Pressed;
      break;
    }
    case FWL_PARTSTATE_CMB_Disabled: {
      eState = FWLTHEME_STATE_Disabale;
      break;
    }
    default: {}
  }
  DrawArrowBtn(pParams->m_pGraphics, &pParams->m_rtPart,
               FWLTHEME_DIRECTION_Down, eState, &pParams->m_matrix);
}
#else
void CFWL_ComboBoxTP::DrawDropDownButton(CFWL_ThemeBackground* pParams,
                                         FX_DWORD dwStates,
                                         CFX_Matrix* pMatrix) {
  FX_BOOL bPressed = ((pParams->m_dwStates & FWL_CMBPARTSTATE_Pressed) ==
                      FWL_CMBPARTSTATE_Pressed);
  FX_FLOAT fWidth = bPressed ? 1.0f : 2.0f;
  FWLTHEME_EDGE eType = bPressed ? FWLTHEME_EDGE_Flat : FWLTHEME_EDGE_Raised;
  Draw3DRect(pParams->m_pGraphics, eType, fWidth, &pParams->m_rtPart,
             FWLTHEME_COLOR_EDGELT1, FWLTHEME_COLOR_EDGELT2,
             FWLTHEME_COLOR_EDGERB1, FWLTHEME_COLOR_EDGERB2, pMatrix);
  CFX_Path path;
  path.Create();
  path.AddRectangle(pParams->m_rtPart.left + fWidth,
                    pParams->m_rtPart.top + fWidth,
                    pParams->m_rtPart.width - 2 * fWidth,
                    pParams->m_rtPart.height - 2 * fWidth);
  pParams->m_pGraphics->SaveGraphState();
  CFX_Color crFill(FWLTHEME_COLOR_Background);
  pParams->m_pGraphics->SetFillColor(&crFill);
  pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, &pParams->m_matrix);
  pParams->m_pGraphics->RestoreGraphState();
  FX_ARGB argbFill = ArgbEncode(255, 77, 97, 133);
  switch (pParams->m_dwStates & 0x03) {
    case FWL_CMBPARTSTATE_Normal: {
    }
    case FWL_CMBPARTSTATE_Hovered: {
    }
    case FWL_CMBPARTSTATE_Pressed: {
      argbFill = 0xFF000000;
      break;
    }
    case FWL_CMBPARTSTATE_Disabled: {
      argbFill = 0xFFF0F0F0;
      break;
    }
  }
  DrawArrow(pParams->m_pGraphics, &pParams->m_rtPart, FWLTHEME_DIRECTION_Down,
            argbFill, bPressed, &pParams->m_matrix);
}
#endif
