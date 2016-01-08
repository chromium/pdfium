// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
CFWL_DateTimePickerTP::CFWL_DateTimePickerTP() {
  m_pThemeData = new DTPThemeData;
  initThemeData();
}
CFWL_DateTimePickerTP::~CFWL_DateTimePickerTP() {
  delete m_pThemeData;
}
FX_BOOL CFWL_DateTimePickerTP::IsValidWidget(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FALSE;
  return pWidget->GetClassID() == FWL_CLASSHASH_DateTimePicker;
}
FX_BOOL CFWL_DateTimePickerTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (!pParams)
    return FALSE;
  switch (pParams->m_iPart) {
    case FWL_PART_DTP_Border: {
      DrawBorder(pParams->m_pGraphics, &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_DTP_Edge: {
      DrawEdge(pParams->m_pGraphics, pParams->m_pWidget->GetStyles(),
               &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_DTP_DropDownButton: {
      DrawDropDownButton(pParams, &pParams->m_matrix);
      break;
    }
    default: {}
  }
  return TRUE;
}
#ifdef THEME_XPSimilar
void CFWL_DateTimePickerTP::DrawDropDownButton(CFWL_ThemeBackground* pParams,
                                               CFX_Matrix* pMatrix) {
  FX_DWORD dwStates = pParams->m_dwStates;
  dwStates &= 0x03;
  FWLTHEME_STATE eState = FWLTHEME_STATE_Normal;
  switch (eState & dwStates) {
    case FWL_PARTSTATE_DTP_Normal: {
      eState = FWLTHEME_STATE_Normal;
      break;
    }
    case FWL_PARTSTATE_DTP_Hovered: {
      eState = FWLTHEME_STATE_Hover;
      break;
    }
    case FWL_PARTSTATE_DTP_Pressed: {
      eState = FWLTHEME_STATE_Pressed;
      break;
    }
    case FWL_PARTSTATE_DTP_Disabled: {
      eState = FWLTHEME_STATE_Disabale;
      break;
    }
    default: {}
  }
  DrawArrowBtn(pParams->m_pGraphics, &pParams->m_rtPart,
               FWLTHEME_DIRECTION_Down, eState, pMatrix);
}
#else
void CFWL_DateTimePickerTP::DrawDropDownButton(CFWL_ThemeBackground* pParams,
                                               CFX_Matrix* pMatrix) {
  FX_BOOL bPressed = ((pParams->m_dwStates & FWL_PARTSTATE_DTP_Pressed) ==
                      FWL_PARTSTATE_DTP_Pressed);
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
  pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  pParams->m_pGraphics->RestoreGraphState();
  FX_ARGB argbFill = ArgbEncode(255, 77, 97, 133);
  switch (pParams->m_dwStates & 0x03) {
    case FWL_PARTSTATE_DTP_Normal: {
    }
    case FWL_PARTSTATE_DTP_Hovered: {
    }
    case FWL_PARTSTATE_DTP_Pressed: {
      argbFill = 0xFF000000;
      break;
    }
    case FWL_PARTSTATE_DTP_Disabled: {
      argbFill = 0xFFF0F0F0;
      break;
    }
  }
  DrawArrow(pParams->m_pGraphics, &pParams->m_rtPart, FWLTHEME_DIRECTION_Down,
            argbFill, bPressed, pMatrix);
}
#endif
void CFWL_DateTimePickerTP::initThemeData() {
  FX_DWORD* pData = (FX_DWORD*)&m_pThemeData->BoxBkColor;
  *pData++ = 0, *pData++ = 0, *pData++ = ArgbEncode(255, 220, 220, 215),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 240, 207),
  *pData++ = ArgbEncode(255, 248, 179, 48),
  *pData++ = ArgbEncode(255, 176, 176, 167),
  *pData++ = ArgbEncode(255, 241, 239, 239),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 220, 220, 215),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 240, 207),
  *pData++ = ArgbEncode(255, 248, 179, 48),
  *pData++ = ArgbEncode(255, 176, 176, 167),
  *pData++ = ArgbEncode(255, 241, 239, 239),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 220, 220, 215),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 240, 207),
  *pData++ = ArgbEncode(255, 248, 179, 48),
  *pData++ = ArgbEncode(255, 176, 176, 167),
  *pData++ = ArgbEncode(255, 241, 239, 239),
  *pData++ = ArgbEncode(255, 255, 255, 255),
  *pData++ = ArgbEncode(255, 255, 255, 255);
}
