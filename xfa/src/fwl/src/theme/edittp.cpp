// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
CFWL_EditTP::CFWL_EditTP() {}
CFWL_EditTP::~CFWL_EditTP() {}
FX_BOOL CFWL_EditTP::IsValidWidget(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FALSE;
  return pWidget->GetClassID() == FWL_CLASSHASH_Edit;
}
FX_BOOL CFWL_EditTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  switch (pParams->m_iPart) {
    case FWL_PART_EDT_Border: {
      DrawBorder(pParams->m_pGraphics, &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_EDT_Edge: {
      DrawEdge(pParams->m_pGraphics, pParams->m_pWidget->GetStyles(),
               &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_EDT_Background: {
      if (pParams->m_pPath) {
        CFX_Graphics* pGraphics = pParams->m_pGraphics;
        pGraphics->SaveGraphState();
        CFX_Color crSelected(FWL_GetThemeColor(m_dwThemeID) == 0
                                 ? FWLTHEME_COLOR_BKSelected
                                 : FWLTHEME_COLOR_Green_BKSelected);
        pGraphics->SetFillColor(&crSelected);
        pGraphics->FillPath(pParams->m_pPath, FXFILL_WINDING,
                            &pParams->m_matrix);
        pGraphics->RestoreGraphState();
      } else {
        FX_BOOL bStatic =
            pParams->m_dwData == FWL_PARTDATA_EDT_StaticBackground;
        CFX_Path path;
        path.Create();
        path.AddRectangle(pParams->m_rtPart.left, pParams->m_rtPart.top,
                          pParams->m_rtPart.width, pParams->m_rtPart.height);
        CFX_Color cr(FWLTHEME_COLOR_Background);
        if (!bStatic) {
          if ((pParams->m_dwStates & FWL_PARTSTATE_EDT_Disable) ==
              FWL_PARTSTATE_EDT_Disable) {
            cr.Set(FWLTHEME_COLOR_EDGERB1);
          } else if ((pParams->m_dwStates & FWL_PARTSTATE_EDT_ReadOnly) ==
                     FWL_PARTSTATE_EDT_ReadOnly) {
            cr.Set(ArgbEncode(255, 236, 233, 216));
          } else {
            cr.Set(0xFFFFFFFF);
          }
        }
        pParams->m_pGraphics->SaveGraphState();
        pParams->m_pGraphics->SetFillColor(&cr);
        pParams->m_pGraphics->FillPath(&path, FXFILL_WINDING,
                                       &pParams->m_matrix);
        pParams->m_pGraphics->RestoreGraphState();
      }
      break;
    }
    case FWL_PART_EDT_CombTextLine: {
      FX_ARGB cr = 0xFF000000;
      FX_FLOAT fWidth = 1.0f;
      CFX_Color crLine(cr);
      pParams->m_pGraphics->SetStrokeColor(&crLine);
      pParams->m_pGraphics->SetLineWidth(fWidth);
      pParams->m_pGraphics->StrokePath(pParams->m_pPath, &pParams->m_matrix);
      break;
    }
    default: { break; }
  }
  return TRUE;
}
FWL_ERR CFWL_EditTP::Initialize() {
  InitTTO();
  return CFWL_WidgetTP::Initialize();
}
FWL_ERR CFWL_EditTP::Finalize() {
  FinalizeTTO();
  return CFWL_WidgetTP::Finalize();
}
