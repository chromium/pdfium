// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#define FWLTHEME_CAPACITY_CXFormBorder 3
#define FWLTHEME_CAPACITY_CYFormBorder 3
#define FWLTHEME_CAPACITY_CYNarrowCaption 18
#define FWLTHEME_CAPACITY_CYCaption 29
#define FWLTHEME_CAPACITY_BigIconSize 32
#define FWLTHEME_CAPACITY_SmallIconSize 16
#define FWLTHEME_CAPACITY_FormTextColor 0xFFFFFFFF
#define FWLTHEME_FORMBTN_Margin 5
#define FWLTHEME_FORMBTN_Span 2
#define FWLTHEME_FORMBTN_Size 21
CFWL_FormTP::CFWL_FormTP() : m_pActiveBitmap(NULL), m_pDeactivebitmap(NULL) {
  m_pThemeData = new SBThemeData;
  SetThemeData(0);
  m_rtDisLBorder.Reset();
  m_rtDisRBorder.Reset();
  m_rtDisBBorder.Reset();
  m_rtDisCaption.Reset();
}
CFWL_FormTP::~CFWL_FormTP() {
  if (m_pThemeData) {
    delete m_pThemeData;
    m_pThemeData = NULL;
  }
}
FWL_ERR CFWL_FormTP::Initialize() {
  InitTTO();
  InitCaption(TRUE);
  InitCaption(FALSE);
  return CFWL_WidgetTP::Initialize();
}
FWL_ERR CFWL_FormTP::Finalize() {
  FinalizeTTO();
  if (m_pActiveBitmap) {
    delete m_pActiveBitmap;
    m_pActiveBitmap = NULL;
  }
  if (m_pDeactivebitmap) {
    delete m_pDeactivebitmap;
    m_pDeactivebitmap = NULL;
  }
  return CFWL_WidgetTP::Finalize();
}
FX_BOOL CFWL_FormTP::IsValidWidget(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FALSE;
  FX_DWORD dwHash = pWidget->GetClassID();
  return dwHash == FWL_CLASSHASH_Form;
}
FX_DWORD CFWL_FormTP::SetThemeID(IFWL_Widget* pWidget,
                                 FX_DWORD dwThemeID,
                                 FX_BOOL bChildren) {
  if (m_pThemeData) {
    SetThemeData(FWL_GetThemeColor(dwThemeID));
  }
  InitCaption(TRUE);
  InitCaption(FALSE);
  return CFWL_WidgetTP::SetThemeID(pWidget, dwThemeID, bChildren);
}
FX_BOOL CFWL_FormTP::DrawBackground(CFWL_ThemeBackground* pParams) {
  if (!pParams)
    return FALSE;
  int32_t iActive = 0;
  if (pParams->m_dwStates & FWL_PARTSTATE_FRM_Inactive) {
    iActive = 1;
  }
  FWLTHEME_STATE eState = FWLTHEME_STATE_Normal;
  switch (pParams->m_dwStates & 0x03) {
    case FWL_PARTSTATE_FRM_Hover: {
      eState = FWLTHEME_STATE_Hover;
      break;
    }
    case FWL_PARTSTATE_FRM_Pressed: {
      eState = FWLTHEME_STATE_Pressed;
      break;
    }
    case FWL_PARTSTATE_FRM_Disabled: {
      eState = FWLTHEME_STATE_Disabale;
      break;
    }
    default: {}
  }
  switch (pParams->m_iPart) {
    case FWL_PART_FRM_Border: {
      DrawFormBorder(pParams->m_pGraphics, &pParams->m_rtPart, eState,
                     &pParams->m_matrix, iActive);
      break;
    }
    case FWL_PART_FRM_Edge: {
      DrawEdge(pParams->m_pGraphics, pParams->m_pWidget->GetStyles(),
               &pParams->m_rtPart, &pParams->m_matrix);
      break;
    }
    case FWL_PART_FRM_Background: {
      FillBackground(pParams->m_pGraphics, &pParams->m_rtPart,
                     &pParams->m_matrix);
      break;
    }
    case FWL_PART_FRM_Caption: {
      DrawCaption(pParams->m_pGraphics, &pParams->m_rtPart, eState,
                  &pParams->m_matrix, iActive);
      break;
    }
    case FWL_PART_FRM_NarrowCaption: {
      DrawNarrowCaption(pParams->m_pGraphics, &pParams->m_rtPart, eState,
                        &pParams->m_matrix, iActive);
      break;
    }
    case FWL_PART_FRM_CloseBox: {
      DrawCloseBox(pParams->m_pGraphics, &pParams->m_rtPart, eState,
                   &pParams->m_matrix, iActive);
      break;
    }
    case FWL_PART_FRM_MinimizeBox: {
      DrawMinimizeBox(pParams->m_pGraphics, &pParams->m_rtPart, eState,
                      &pParams->m_matrix, iActive);
      break;
    }
    case FWL_PART_FRM_MaximizeBox: {
      DrawMaximizeBox(pParams->m_pGraphics, &pParams->m_rtPart, eState,
                      pParams->m_dwData, &pParams->m_matrix, iActive);
      break;
    }
    case FWL_PART_FRM_Icon: {
      DrawIconImage(pParams->m_pGraphics, pParams->m_pImage, &pParams->m_rtPart,
                    eState, &pParams->m_matrix, iActive);
      break;
    }
    default: {}
  }
  return TRUE;
}
FX_BOOL CFWL_FormTP::DrawText(CFWL_ThemeText* pParams) {
  if (!m_pTextOut)
    return FALSE;
  if (pParams->m_iPart == FWL_PART_FRM_Caption) {
    m_pTextOut->SetTextColor(0xFFFFFFFF);
  } else {
    m_pTextOut->SetTextColor(0xFF000000);
  }
  return CFWL_WidgetTP::DrawText(pParams);
}
void* CFWL_FormTP::GetCapacity(CFWL_ThemePart* pThemePart,
                               FX_DWORD dwCapacity) {
  FX_BOOL bDefPro = FALSE;
  FX_BOOL bDwordVal = FALSE;
  switch (dwCapacity) {
    case FWL_WGTCAPACITY_CXBorder: {
      m_fValue = FWLTHEME_CAPACITY_CXFormBorder;
      break;
    }
    case FWL_WGTCAPACITY_CYBorder: {
      m_fValue = FWLTHEME_CAPACITY_CYFormBorder;
      break;
    }
    case FWL_WGTCAPACITY_FRM_CYCaption: {
      m_fValue = FWLTHEME_CAPACITY_CYCaption;
      break;
    }
    case FWL_WGTCAPACITY_FRM_CYNarrowCaption: {
      m_fValue = FWLTHEME_CAPACITY_CYCaption;
      break;
    }
    case FWL_WGTCAPACITY_TextColor: {
      bDwordVal = TRUE;
      m_dwValue = FWLTHEME_CAPACITY_FormTextColor;
      break;
    }
    case FWL_WGTCAPACITY_FRM_BigIcon: {
      m_fValue = FWLTHEME_CAPACITY_BigIconSize;
      break;
    }
    case FWL_WGTCAPACITY_FRM_SmallIcon: {
      m_fValue = FWLTHEME_CAPACITY_SmallIconSize;
      break;
    }
    default: { bDefPro = TRUE; }
  }
  if (!bDefPro) {
    if (bDwordVal) {
      return &m_dwValue;
    }
    return &m_fValue;
  }
  return CFWL_WidgetTP::GetCapacity(pThemePart, dwCapacity);
}
FWL_ERR CFWL_FormTP::GetPartRect(CFWL_ThemePart* pThemePart,
                                 CFX_RectF& rtPart) {
  switch (pThemePart->m_iPart) {
    case FWL_PART_FRM_CloseBox: {
      CalCloseBox(pThemePart->m_pWidget, rtPart);
      break;
    }
    case FWL_PART_FRM_MaximizeBox: {
      CalMaxBox(pThemePart->m_pWidget, rtPart);
      break;
    }
    case FWL_PART_FRM_MinimizeBox: {
      CalMinBox(pThemePart->m_pWidget, rtPart);
      break;
    }
    case FWL_PART_FRM_HeadText: {
      CalCaption(pThemePart->m_pWidget, rtPart);
      break;
    }
    case FWL_PART_FRM_Icon: {
      CalIcon(pThemePart->m_pWidget, rtPart);
      break;
    }
    default: {}
  }
  return FWL_ERR_Succeeded;
}
void CFWL_FormTP::CalCloseBox(IFWL_Widget* pWidget, CFX_RectF& rect) {
  FX_DWORD dwStyles = pWidget->GetStyles();
  CFX_RectF rtWidget;
  pWidget->GetWidgetRect(rtWidget);
  rtWidget.Offset(-rtWidget.left, -rtWidget.top);
  if (dwStyles & FWL_WGTSTYLE_CloseBox) {
    rect.Set(rtWidget.left + FWLTHEME_FORMBTN_Margin + FWLTHEME_FORMBTN_Span,
             rtWidget.top + FWLTHEME_FORMBTN_Margin, FWLTHEME_FORMBTN_Size,
             FWLTHEME_FORMBTN_Size);
  } else {
    rect.Set(rtWidget.left + FWLTHEME_FORMBTN_Margin + FWLTHEME_FORMBTN_Span,
             rtWidget.top + FWLTHEME_FORMBTN_Margin, 0, 0);
  }
}
void CFWL_FormTP::CalMaxBox(IFWL_Widget* pWidget, CFX_RectF& rect) {
  FX_DWORD dwStyles = pWidget->GetStyles();
  CFX_RectF rtWidget;
  pWidget->GetWidgetRect(rtWidget);
  rtWidget.Offset(-rtWidget.left, -rtWidget.top);
  if (dwStyles & FWL_WGTSTYLE_MaximizeBox) {
    rect.Set(rtWidget.left + FWLTHEME_FORMBTN_Margin + FWLTHEME_FORMBTN_Size +
                 FWLTHEME_FORMBTN_Span * 2,
             rtWidget.top + FWLTHEME_FORMBTN_Margin, FWLTHEME_FORMBTN_Size,
             FWLTHEME_FORMBTN_Size);
  } else {
    rect.Set(rtWidget.left + FWLTHEME_FORMBTN_Margin + FWLTHEME_FORMBTN_Size +
                 FWLTHEME_FORMBTN_Span * 2,
             rtWidget.top + FWLTHEME_FORMBTN_Margin, 0, 0);
  }
}
void CFWL_FormTP::CalMinBox(IFWL_Widget* pWidget, CFX_RectF& rect) {
  FX_DWORD dwStyles = pWidget->GetStyles();
  CFX_RectF rtWidget;
  pWidget->GetWidgetRect(rtWidget);
  rtWidget.Offset(-rtWidget.left, -rtWidget.top);
  if (dwStyles & FWL_WGTSTYLE_MinimizeBox) {
    rect.Set(rtWidget.left + FWLTHEME_FORMBTN_Margin +
                 FWLTHEME_FORMBTN_Size * 2 + FWLTHEME_FORMBTN_Span * 3,
             rtWidget.top + FWLTHEME_FORMBTN_Margin, FWLTHEME_FORMBTN_Size,
             FWLTHEME_FORMBTN_Size);
  } else {
    rect.Set(rtWidget.left + FWLTHEME_FORMBTN_Margin +
                 FWLTHEME_FORMBTN_Size * 2 + FWLTHEME_FORMBTN_Span * 3,
             rtWidget.top + FWLTHEME_FORMBTN_Margin, 0, 0);
  }
}
void CFWL_FormTP::CalCaption(IFWL_Widget* pWidget, CFX_RectF& rect) {
  CFX_RectF rtWidget;
  pWidget->GetWidgetRect(rtWidget);
  rtWidget.Offset(-rtWidget.left, -rtWidget.top);
  rect.Set(rtWidget.left + FWLTHEME_FORMBTN_Margin,
           rtWidget.top + FWLTHEME_FORMBTN_Margin - 2,
           rtWidget.width - FWLTHEME_FORMBTN_Margin * 2,
           FWLTHEME_FORMBTN_Size + 2 * FWLTHEME_FORMBTN_Margin + 4);
}
void CFWL_FormTP::CalIcon(IFWL_Widget* pWidget, CFX_RectF& rect) {}
void CFWL_FormTP::DrawFormBorder(CFX_Graphics* pGraphics,
                                 const CFX_RectF* pRect,
                                 FWLTHEME_STATE eState,
                                 CFX_Matrix* pMatrix,
                                 int32_t iActive) {
  CFX_RectF rt;
  rt.Set(pRect->left, pRect->top, 1, pRect->height);
  FX_FLOAT fBottom, fRight;
  fBottom = pRect->bottom();
  fRight = pRect->right();
  CFX_Path path;
  path.Create();
  CFX_Color clrLine;
  path.Clear();
  path.MoveTo(pRect->left, pRect->top);
  path.LineTo(pRect->left, fBottom - 1);
  path.LineTo(fRight - 1, fBottom - 1);
  path.LineTo(fRight - 1, pRect->top);
  clrLine = m_pThemeData->clrFormBorder[iActive][2];
  pGraphics->SetStrokeColor(&clrLine);
  pGraphics->StrokePath(&path, pMatrix);
  path.Clear();
  path.MoveTo(pRect->left + 1, pRect->top);
  path.LineTo(pRect->left + 1, fBottom - 2);
  path.LineTo(fRight - 2, fBottom - 2);
  path.LineTo(fRight - 2, pRect->top);
  clrLine = m_pThemeData->clrFormBorder[iActive][1];
  pGraphics->SetStrokeColor(&clrLine);
  pGraphics->StrokePath(&path, pMatrix);
  path.Clear();
  path.MoveTo(pRect->left + 2, pRect->top);
  path.LineTo(pRect->left + 2, fBottom - 3);
  path.LineTo(fRight - 3, fBottom - 3);
  path.LineTo(fRight - 3, pRect->top);
  clrLine = m_pThemeData->clrFormBorder[iActive][0];
  pGraphics->SetStrokeColor(&clrLine);
  pGraphics->StrokePath(&path, pMatrix);
  path.Clear();
  path.MoveTo(pRect->left + 3, pRect->top);
  path.LineTo(pRect->left + 3, fBottom - 4);
  path.LineTo(fRight - 4, fBottom - 4);
  path.LineTo(fRight - 4, pRect->top);
  clrLine = m_pThemeData->clrFormBorder[iActive][4];
  pGraphics->SetStrokeColor(&clrLine);
  pGraphics->StrokePath(&path, pMatrix);
  m_rtDisLBorder.Set(pRect->left, pRect->top + 29, 4, pRect->height - 29);
  m_rtDisRBorder.Set(pRect->right() - 4, pRect->top + 29, 4,
                     pRect->height - 29);
  m_rtDisBBorder.Set(pRect->left, pRect->bottom() - 4, pRect->width, 4);
  m_rtDisCaption.Set(pRect->left, pRect->top, pRect->width, 29);
}
void CFWL_FormTP::DrawCaption(CFX_Graphics* pGraphics,
                              const CFX_RectF* pRect,
                              FWLTHEME_STATE eState,
                              CFX_Matrix* pMatrix,
                              int32_t iActive) {
  CFX_RectF rt;
  FX_FLOAT fBottom, fRight;
  fBottom = pRect->bottom();
  fRight = pRect->right();
  rt.Set(pRect->left, pRect->top, pRect->width, 1);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeTop[iActive][0], &rt,
                pMatrix);
  rt.Offset(0, 1);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeTop[iActive][1], &rt,
                pMatrix);
  rt.Offset(0, 1);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeTop[iActive][2], &rt,
                pMatrix);
  rt.Set(pRect->left, pRect->bottom() - 1, pRect->width, 1);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeBottom[iActive][2], &rt,
                pMatrix);
  rt.Offset(0, -1);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeBottom[iActive][1], &rt,
                pMatrix);
  rt.Set(pRect->left, pRect->top, 1, pRect->height);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeLeft[iActive][2], &rt,
                pMatrix);
  rt.Set(pRect->left + 1, pRect->top + 1, 1, fBottom - 1);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeLeft[iActive][1], &rt,
                pMatrix);
  rt.Set(pRect->left + 2, pRect->top + 2, 1, fBottom - 2);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeLeft[iActive][0], &rt,
                pMatrix);
  rt.Set(fRight - 1, pRect->top, pRect->width, pRect->height);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeRight[iActive][2], &rt,
                pMatrix);
  rt.Set(fRight - 2, pRect->top + 1, 1, fBottom - 1);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeRight[iActive][1], &rt,
                pMatrix);
  rt.Set(fRight - 3, pRect->top + 2, 1, fBottom - 2);
  FillSoildRect(pGraphics, m_pThemeData->clrHeadEdgeRight[iActive][0], &rt,
                pMatrix);
  CFX_RectF rect(*pRect);
  rect.Set(rect.left + 3, rect.top + 3, rect.width - 6, rect.height - 5);
  if (iActive == 0) {
    pGraphics->StretchImage(m_pActiveBitmap, rect, pMatrix);
  } else {
    pGraphics->StretchImage(m_pDeactivebitmap, rect, pMatrix);
  }
}
void CFWL_FormTP::DrawNarrowCaption(CFX_Graphics* pGraphics,
                                    const CFX_RectF* pRect,
                                    FWLTHEME_STATE eState,
                                    CFX_Matrix* pMatrix,
                                    int32_t iActive) {}
void CFWL_FormTP::DrawCloseBox(CFX_Graphics* pGraphics,
                               const CFX_RectF* pRect,
                               FWLTHEME_STATE eState,
                               CFX_Matrix* pMatrix,
                               int32_t iActive) {
  FX_FLOAT fRight = pRect->right();
  FX_FLOAT fBottom = pRect->bottom();
  FX_FLOAT fWidth = pRect->width;
  FX_FLOAT fHeight = pRect->height;
  pGraphics->SaveGraphState();
  CFX_RectF rt(*pRect);
  pGraphics->SetLineWidth(1.0f);
  CFX_Path path;
  path.Create();
  path.AddRectangle(rt.left + 1, rt.top, fWidth - 2, 1);
  path.AddRectangle(rt.left, rt.top + 1, 1, fHeight - 2);
  path.AddRectangle(fRight - 1, rt.top + 1, 1, fHeight - 2);
  path.AddRectangle(rt.left + 1, fBottom - 1, fWidth - 2, 1);
  CFX_Color crFill;
  crFill = m_pThemeData->clrBtnEdgeOut[iActive];
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  path.Clear();
  path.AddRectangle(rt.left + 1, rt.top + 1, 1, 1);
  path.AddRectangle(fRight - 2, rt.top + 1, 1, 1);
  path.AddRectangle(rt.left + 1, fBottom - 2, 1, 1);
  path.AddRectangle(fRight - 2, fBottom - 2, 1, 1);
  crFill = m_pThemeData->clrBtnCornerLight[iActive][eState - 1];
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  path.Clear();
  path.AddRectangle(rt.left + 2, rt.top + 1, fWidth - 4, 1);
  path.AddRectangle(rt.left + 1, rt.top + 2, 1, fHeight - 4);
  crFill = m_pThemeData->clrCloseBtEdgeLight[iActive][eState - 1];
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  path.Clear();
  path.AddRectangle(fRight - 2, rt.top + 2, 1, fHeight - 4);
  path.AddRectangle(rt.left + 2, fBottom - 2, fWidth - 4, 1);
  crFill = m_pThemeData->clrCloseBtEdgeDark[iActive][eState - 1];
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  path.Clear();
  path.AddRectangle(pRect->left + 2, pRect->top + 2, fWidth - 4, fHeight - 4);
  DrawAxialShading(pGraphics, pRect->left + 2, pRect->top + 2, fRight - 2,
                   fBottom - 2,
                   m_pThemeData->clrCloseBtBKStart[iActive][eState - 1],
                   m_pThemeData->clrCloseBtBKEnd[iActive][eState - 1], &path,
                   FXFILL_WINDING, pMatrix);
  CFX_RectF rtX(*pRect);
  rtX.Inflate(-5, -5);
  path.Clear();
  FX_FLOAT frtXRight = rtX.right();
  FX_FLOAT frtXBottom = rtX.bottom();
  path.AddLine(rtX.left, rtX.top + 1, frtXRight - 1, frtXBottom);
  path.AddLine(rtX.left, rtX.top, frtXRight, frtXBottom);
  path.AddLine(rtX.left + 1, rtX.top, frtXRight, frtXBottom - 1);
  path.AddLine(rtX.left, frtXBottom - 1, frtXRight - 1, rtX.top);
  path.AddLine(rtX.left, frtXBottom, frtXRight, rtX.top);
  path.AddLine(rtX.left + 1, frtXBottom, frtXRight, rtX.top + 1);
  CFX_Color clrLine(0xffffffff);
  pGraphics->SetLineWidth(1.0f);
  pGraphics->SetStrokeColor(&clrLine);
  pGraphics->StrokePath(&path, pMatrix);
  pGraphics->RestoreGraphState();
}
void CFWL_FormTP::DrawMinMaxBoxCommon(CFX_Graphics* pGraphics,
                                      const CFX_RectF* pRect,
                                      FWLTHEME_STATE eState,
                                      CFX_Matrix* pMatrix,
                                      int32_t iActive) {
  pGraphics->SaveGraphState();
  FX_FLOAT fRight = pRect->right();
  FX_FLOAT fBottom = pRect->bottom();
  FX_FLOAT fWidth = pRect->width;
  FX_FLOAT fHeight = pRect->height;
  CFX_RectF rt(*pRect);
  pGraphics->SetLineWidth(1.0f);
  CFX_Path path;
  path.Create();
  path.AddRectangle(rt.left + 1, rt.top, fWidth - 2, 1);
  path.AddRectangle(rt.left, rt.top + 1, 1, fHeight - 2);
  path.AddRectangle(fRight - 1, rt.top + 1, 1, fHeight - 2);
  path.AddRectangle(rt.left + 1, fBottom - 1, fWidth - 2, 1);
  CFX_Color crFill;
  crFill = m_pThemeData->clrBtnEdgeOut[iActive];
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  path.Clear();
  path.AddRectangle(rt.left + 1, rt.top + 1, 1, 1);
  path.AddRectangle(fRight - 2, rt.top + 1, 1, 1);
  path.AddRectangle(rt.left + 1, fBottom - 2, 1, 1);
  path.AddRectangle(fRight - 2, fBottom - 2, 1, 1);
  crFill = m_pThemeData->clrBtnCornerLight[iActive][eState - 1];
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  path.Clear();
  path.AddRectangle(rt.left + 2, rt.top + 1, fWidth - 4, 1);
  path.AddRectangle(rt.left + 1, rt.top + 2, 1, fHeight - 4);
  crFill = m_pThemeData->clrNormalBtEdgeLight[iActive][eState - 1];
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  path.Clear();
  path.AddRectangle(fRight - 2, rt.top + 2, 1, fHeight - 4);
  path.AddRectangle(rt.left + 2, fBottom - 2, fWidth - 4, 1);
  crFill = m_pThemeData->clrNormalBtEdgeDark[iActive][eState - 1];
  pGraphics->SetFillColor(&crFill);
  pGraphics->FillPath(&path, FXFILL_WINDING, pMatrix);
  pGraphics->RestoreGraphState();
  path.Clear();
  path.AddRectangle(pRect->left + 2, pRect->top + 2, fWidth - 4, fHeight - 4);
  DrawAxialShading(pGraphics, pRect->left + 2, pRect->top + 2, fRight - 2,
                   fBottom - 2,
                   m_pThemeData->clrNormalBtBKStart[iActive][eState - 1],
                   m_pThemeData->clrNormalBtBKEnd[iActive][eState - 1], &path,
                   FXFILL_WINDING, pMatrix);
}
void CFWL_FormTP::DrawMinimizeBox(CFX_Graphics* pGraphics,
                                  const CFX_RectF* pRect,
                                  FWLTHEME_STATE eState,
                                  CFX_Matrix* pMatrix,
                                  int32_t iActive) {
  DrawMinMaxBoxCommon(pGraphics, pRect, eState, pMatrix);
  CFX_RectF rtMin;
  rtMin.Set(pRect->left + 5, pRect->top + 13, pRect->width - 14,
            pRect->height - 18);
  FillSoildRect(pGraphics, 0xFFFFFFFF, &rtMin, pMatrix);
}
void CFWL_FormTP::DrawMaximizeBox(CFX_Graphics* pGraphics,
                                  const CFX_RectF* pRect,
                                  FWLTHEME_STATE eState,
                                  FX_BOOL bMax,
                                  CFX_Matrix* pMatrix,
                                  int32_t iActive) {
  DrawMinMaxBoxCommon(pGraphics, pRect, eState, pMatrix);
  FX_FLOAT fWidth = pRect->width;
  FX_FLOAT fHeight = pRect->height;
  if (bMax) {
    CFX_Path path;
    path.Create();
    path.AddLine(pRect->left + 7, pRect->top + 6, pRect->left + 14,
                 pRect->top + 6);
    path.AddLine(pRect->left + 4, pRect->top + 9, pRect->left + 11,
                 pRect->top + 9);
    pGraphics->SaveGraphState();
    pGraphics->SetLineWidth(2);
    CFX_Color crStroke(0xFFFFFFFF);
    pGraphics->SetStrokeColor(&crStroke);
    pGraphics->StrokePath(&path, pMatrix);
    pGraphics->SetLineWidth(1);
    path.Clear();
    path.AddLine(pRect->left + 4, pRect->top + 10, pRect->left + 4,
                 pRect->top + 14);
    path.AddLine(pRect->left + 10, pRect->top + 10, pRect->left + 10,
                 pRect->top + 14);
    path.AddLine(pRect->left + 13, pRect->top + 7, pRect->left + 13,
                 pRect->top + 11);
    path.AddLine(pRect->left + 4, pRect->top + 14, pRect->left + 10,
                 pRect->top + 14);
    path.AddLine(pRect->left + 12, pRect->top + 11, pRect->left + 12,
                 pRect->top + 11);
    pGraphics->StrokePath(&path, pMatrix);
    pGraphics->RestoreGraphState();
  } else {
    CFX_RectF rtMax(*pRect);
    rtMax.Inflate(-5, -5);
    CFX_Path path;
    path.Create();
    path.AddRectangle(pRect->left + 5, pRect->top + 5, fWidth - 10,
                      fHeight - 10);
    path.AddRectangle(pRect->left + 6, pRect->top + 8, fWidth - 12,
                      fHeight - 14);
    pGraphics->SaveGraphState();
    CFX_Color crFill(0xFFFFFFFF);
    pGraphics->SetFillColor(&crFill);
    pGraphics->FillPath(&path, FXFILL_ALTERNATE, pMatrix);
    pGraphics->RestoreGraphState();
  }
}
void CFWL_FormTP::DrawIconImage(CFX_Graphics* pGraphics,
                                CFX_DIBitmap* pDIBitmap,
                                const CFX_RectF* pRect,
                                FWLTHEME_STATE eState,
                                CFX_Matrix* pMatrix,
                                int32_t iActive) {
  pGraphics->StretchImage(pDIBitmap, *pRect, pMatrix);
}
void CFWL_FormTP::SetThemeData(FX_DWORD dwID) {
  m_pThemeData->clrTransWhite = ArgbEncode(0x65, 255, 255, 255);
  m_pThemeData->clrCloseBtBKStart[0][0] = ArgbEncode(0xff, 240, 166, 148);
  m_pThemeData->clrCloseBtBKEnd[0][0] = ArgbEncode(0xff, 228, 61, 5);
  m_pThemeData->clrCloseBtBKStart[0][1] = ArgbEncode(0xff, 255, 184, 176);
  m_pThemeData->clrCloseBtBKEnd[0][1] = ArgbEncode(0xff, 252, 107, 71);
  m_pThemeData->clrCloseBtBKStart[0][2] = ArgbEncode(0xff, 141, 44, 20);
  m_pThemeData->clrCloseBtBKEnd[0][2] = ArgbEncode(0xff, 202, 72, 33);
  m_pThemeData->clrCloseBtEdgeLight[0][0] = ArgbEncode(0xff, 255, 122, 107);
  m_pThemeData->clrCloseBtEdgeDark[0][0] = ArgbEncode(0xff, 218, 77, 54);
  m_pThemeData->clrCloseBtEdgeLight[0][1] = ArgbEncode(0xff, 255, 93, 74);
  m_pThemeData->clrCloseBtEdgeDark[0][1] = ArgbEncode(0xff, 218, 74, 51);
  m_pThemeData->clrCloseBtEdgeLight[0][2] = ArgbEncode(0xff, 191, 61, 28);
  m_pThemeData->clrCloseBtEdgeDark[0][2] = ArgbEncode(0xff, 93, 30, 13);
  if (dwID) {
    m_pThemeData->clrHeadBK[0][0] = ArgbEncode(0xff, 194, 205, 149);
    m_pThemeData->clrHeadBK[0][1] = ArgbEncode(0xff, 170, 184, 131);
    m_pThemeData->clrHeadBK[0][2] = ArgbEncode(0xff, 168, 182, 128);
    m_pThemeData->clrHeadBK[0][3] = ArgbEncode(0xff, 194, 205, 149);
    m_pThemeData->clrHeadEdgeLeft[0][0] = ArgbEncode(0xff, 117, 141, 94);
    m_pThemeData->clrHeadEdgeLeft[0][1] = ArgbEncode(0xff, 139, 161, 105);
    m_pThemeData->clrHeadEdgeLeft[0][2] = ArgbEncode(0xff, 171, 189, 133);
    m_pThemeData->clrHeadEdgeRight[0][0] = ArgbEncode(0xff, 155, 175, 125);
    m_pThemeData->clrHeadEdgeRight[0][1] = ArgbEncode(0xff, 128, 146, 103);
    m_pThemeData->clrHeadEdgeRight[0][2] = ArgbEncode(0xff, 94, 118, 79);
    m_pThemeData->clrHeadEdgeTop[0][0] = ArgbEncode(0xff, 139, 161, 105);
    m_pThemeData->clrHeadEdgeTop[0][1] = ArgbEncode(0xff, 234, 245, 201);
    m_pThemeData->clrHeadEdgeTop[0][2] = ArgbEncode(0xff, 194, 205, 149);
    m_pThemeData->clrHeadEdgeBottom[0][0] = ArgbEncode(0xff, 175, 189, 133);
    m_pThemeData->clrHeadEdgeBottom[0][1] = ArgbEncode(0xff, 153, 168, 121);
    m_pThemeData->clrHeadEdgeBottom[0][2] = ArgbEncode(0xff, 150, 168, 103);
    m_pThemeData->clrNormalBtBKStart[0][0] = ArgbEncode(0xff, 182, 195, 162);
    m_pThemeData->clrNormalBtBKEnd[0][0] = ArgbEncode(0xff, 128, 144, 84);
    m_pThemeData->clrNormalBtBKStart[0][1] = ArgbEncode(0xff, 234, 241, 208);
    m_pThemeData->clrNormalBtBKEnd[0][1] = ArgbEncode(0xff, 169, 186, 112);
    m_pThemeData->clrNormalBtBKStart[0][2] = ArgbEncode(0xff, 199, 199, 190);
    m_pThemeData->clrNormalBtBKEnd[0][2] = ArgbEncode(0xff, 133, 148, 88);
    m_pThemeData->clrNormalBtEdgeLight[0][0] = ArgbEncode(0xff, 163, 176, 137);
    m_pThemeData->clrNormalBtEdgeDark[0][0] = ArgbEncode(0xff, 118, 135, 83);
    m_pThemeData->clrNormalBtEdgeLight[0][1] = ArgbEncode(0xff, 154, 174, 105);
    m_pThemeData->clrNormalBtEdgeDark[0][1] = ArgbEncode(0xff, 154, 174, 105);
    m_pThemeData->clrNormalBtEdgeLight[0][2] = ArgbEncode(0xff, 172, 193, 123);
    m_pThemeData->clrNormalBtEdgeDark[0][2] = ArgbEncode(0xff, 154, 174, 105);
    m_pThemeData->clrBtnCornerLight[0][0] = ArgbEncode(0xff, 220, 220, 220);
    m_pThemeData->clrBtnCornerLight[0][1] = ArgbEncode(0xff, 255, 255, 255);
    m_pThemeData->clrBtnCornerLight[0][2] = ArgbEncode(0xff, 225, 225, 225);
    m_pThemeData->clrBtnEdgeOut[0] = ArgbEncode(0xff, 255, 255, 255);
    m_pThemeData->clrFormBorder[0][0] = ArgbEncode(0xff, 117, 141, 94);
    m_pThemeData->clrFormBorder[0][1] = ArgbEncode(0xff, 139, 161, 105);
    m_pThemeData->clrFormBorder[0][2] = ArgbEncode(0xff, 171, 189, 133);
    m_pThemeData->clrFormBorder[0][3] = ArgbEncode(0xff, 164, 178, 127);
    m_pThemeData->clrFormBorder[0][4] = ArgbEncode(0xff, 255, 255, 255);
    m_pThemeData->clrFormBorderLight[0] = ArgbEncode(0xff, 171, 189, 133);
  } else {
    m_pThemeData->clrHeadBK[0][0] = ArgbEncode(0xff, 3, 114, 255);
    m_pThemeData->clrHeadBK[0][1] = ArgbEncode(0xff, 0, 85, 226);
    m_pThemeData->clrHeadBK[0][2] = ArgbEncode(0xff, 0, 85, 226);
    m_pThemeData->clrHeadBK[0][3] = ArgbEncode(0xff, 3, 114, 255);
    m_pThemeData->clrHeadEdgeLeft[0][2] = ArgbEncode(0xff, 0, 32, 200);
    m_pThemeData->clrHeadEdgeLeft[0][1] = ArgbEncode(0xff, 0, 61, 220);
    m_pThemeData->clrHeadEdgeLeft[0][0] = ArgbEncode(0xff, 0, 54, 210);
    m_pThemeData->clrHeadEdgeRight[0][0] = ArgbEncode(0xff, 0, 56, 234);
    m_pThemeData->clrHeadEdgeRight[0][1] = ArgbEncode(0xff, 0, 50, 193);
    m_pThemeData->clrHeadEdgeRight[0][2] = ArgbEncode(0xff, 0, 19, 139);
    m_pThemeData->clrHeadEdgeTop[0][0] = ArgbEncode(0xff, 0, 88, 238);
    m_pThemeData->clrHeadEdgeTop[0][1] = ArgbEncode(0xff, 63, 151, 255);
    m_pThemeData->clrHeadEdgeTop[0][2] = ArgbEncode(0xff, 3, 114, 255);
    m_pThemeData->clrHeadEdgeBottom[0][0] = ArgbEncode(0xff, 0, 96, 252);
    m_pThemeData->clrHeadEdgeBottom[0][1] = ArgbEncode(0xff, 63, 151, 255);
    m_pThemeData->clrHeadEdgeBottom[0][2] = ArgbEncode(0xff, 0, 67, 207);
    m_pThemeData->clrNormalBtBKStart[0][2] = ArgbEncode(0xff, 0, 49, 112);
    m_pThemeData->clrNormalBtBKEnd[0][2] = ArgbEncode(0xff, 0, 87, 188);
    m_pThemeData->clrNormalBtBKStart[0][0] = ArgbEncode(0xff, 154, 183, 250);
    m_pThemeData->clrNormalBtBKEnd[0][0] = ArgbEncode(0xff, 17, 110, 248);
    m_pThemeData->clrNormalBtBKStart[0][1] = ArgbEncode(0xff, 164, 194, 255);
    m_pThemeData->clrNormalBtBKEnd[0][1] = ArgbEncode(0xff, 29, 158, 255);
    m_pThemeData->clrNormalBtEdgeLight[0][0] = ArgbEncode(0xff, 68, 120, 245);
    m_pThemeData->clrNormalBtEdgeDark[0][0] = ArgbEncode(0xff, 24, 72, 187);
    m_pThemeData->clrNormalBtEdgeLight[0][1] = ArgbEncode(0xff, 72, 122, 245);
    m_pThemeData->clrNormalBtEdgeDark[0][1] = ArgbEncode(0xff, 35, 87, 195);
    m_pThemeData->clrNormalBtEdgeLight[0][2] = ArgbEncode(0xff, 60, 114, 244);
    m_pThemeData->clrNormalBtEdgeDark[0][2] = ArgbEncode(0xff, 21, 70, 185);
    m_pThemeData->clrBtnCornerLight[0][0] = ArgbEncode(0xff, 220, 220, 220);
    m_pThemeData->clrBtnCornerLight[0][1] = ArgbEncode(0xff, 255, 255, 255);
    m_pThemeData->clrBtnCornerLight[0][2] = ArgbEncode(0xff, 225, 225, 225);
    m_pThemeData->clrBtnEdgeOut[0] = ArgbEncode(0xff, 255, 255, 255);
    m_pThemeData->clrFormBorder[0][0] = ArgbEncode(0xff, 0, 72, 241);
    m_pThemeData->clrFormBorder[0][1] = ArgbEncode(0xff, 0, 61, 220);
    m_pThemeData->clrFormBorder[0][2] = ArgbEncode(0xff, 0, 30, 160);
    m_pThemeData->clrFormBorder[0][3] = ArgbEncode(0xff, 0, 19, 140);
    m_pThemeData->clrFormBorder[0][4] = ArgbEncode(0xff, 255, 255, 255);
    m_pThemeData->clrFormBorderLight[0] = ArgbEncode(0xff, 22, 106, 239);
  }
  m_pThemeData->clrCloseBtBKStart[1][0] = m_pThemeData->clrCloseBtBKStart[0][0];
  m_pThemeData->clrCloseBtBKEnd[1][0] = m_pThemeData->clrCloseBtBKEnd[0][0];
  m_pThemeData->clrCloseBtBKStart[1][1] = m_pThemeData->clrCloseBtBKStart[0][1];
  m_pThemeData->clrCloseBtBKEnd[1][1] = m_pThemeData->clrCloseBtBKEnd[0][1];
  m_pThemeData->clrCloseBtBKStart[1][2] = m_pThemeData->clrCloseBtBKStart[0][2];
  m_pThemeData->clrCloseBtBKEnd[1][2] = m_pThemeData->clrCloseBtBKEnd[0][2];
  m_pThemeData->clrCloseBtEdgeLight[1][0] =
      m_pThemeData->clrCloseBtEdgeLight[0][0];
  m_pThemeData->clrCloseBtEdgeDark[1][0] =
      m_pThemeData->clrCloseBtEdgeDark[0][0];
  m_pThemeData->clrCloseBtEdgeLight[1][1] =
      m_pThemeData->clrCloseBtEdgeLight[0][1];
  m_pThemeData->clrCloseBtEdgeDark[1][1] =
      m_pThemeData->clrCloseBtEdgeDark[0][1];
  m_pThemeData->clrCloseBtEdgeLight[1][2] =
      m_pThemeData->clrCloseBtEdgeLight[0][2];
  m_pThemeData->clrCloseBtEdgeDark[1][2] =
      m_pThemeData->clrCloseBtEdgeDark[0][2];
  m_pThemeData->clrHeadBK[1][0] = m_pThemeData->clrHeadBK[0][0];
  m_pThemeData->clrHeadBK[1][1] = m_pThemeData->clrHeadBK[0][1];
  m_pThemeData->clrHeadBK[1][2] = m_pThemeData->clrHeadBK[0][2];
  m_pThemeData->clrHeadBK[1][3] = m_pThemeData->clrHeadBK[0][3];
  m_pThemeData->clrHeadEdgeLeft[1][2] = m_pThemeData->clrHeadEdgeLeft[0][2];
  m_pThemeData->clrHeadEdgeLeft[1][1] = m_pThemeData->clrHeadEdgeLeft[0][1];
  m_pThemeData->clrHeadEdgeLeft[1][0] = m_pThemeData->clrHeadEdgeLeft[0][0];
  m_pThemeData->clrHeadEdgeRight[1][0] = m_pThemeData->clrHeadEdgeRight[0][0];
  m_pThemeData->clrHeadEdgeRight[1][1] = m_pThemeData->clrHeadEdgeRight[0][1];
  m_pThemeData->clrHeadEdgeRight[1][2] = m_pThemeData->clrHeadEdgeRight[0][2];
  m_pThemeData->clrHeadEdgeTop[1][0] = m_pThemeData->clrHeadEdgeTop[0][0];
  m_pThemeData->clrHeadEdgeTop[1][1] = m_pThemeData->clrHeadEdgeTop[0][1];
  m_pThemeData->clrHeadEdgeTop[1][2] = m_pThemeData->clrHeadEdgeTop[0][2];
  m_pThemeData->clrHeadEdgeBottom[1][0] = m_pThemeData->clrHeadEdgeBottom[0][0];
  m_pThemeData->clrHeadEdgeBottom[1][1] = m_pThemeData->clrHeadEdgeBottom[0][1];
  m_pThemeData->clrHeadEdgeBottom[1][2] = m_pThemeData->clrHeadEdgeBottom[0][2];
  m_pThemeData->clrNormalBtBKStart[1][2] =
      m_pThemeData->clrNormalBtBKStart[0][2];
  m_pThemeData->clrNormalBtBKEnd[1][2] = m_pThemeData->clrNormalBtBKEnd[0][2];
  m_pThemeData->clrNormalBtBKStart[1][0] =
      m_pThemeData->clrNormalBtBKStart[0][0];
  m_pThemeData->clrNormalBtBKEnd[1][0] = m_pThemeData->clrNormalBtBKEnd[1][0];
  m_pThemeData->clrNormalBtBKStart[1][1] =
      m_pThemeData->clrNormalBtBKStart[0][1];
  m_pThemeData->clrNormalBtBKEnd[1][1] = m_pThemeData->clrNormalBtBKEnd[0][1];
  m_pThemeData->clrNormalBtEdgeLight[1][0] =
      m_pThemeData->clrNormalBtEdgeLight[0][0];
  m_pThemeData->clrNormalBtEdgeDark[1][0] =
      m_pThemeData->clrNormalBtEdgeDark[0][0];
  m_pThemeData->clrNormalBtEdgeLight[1][1] =
      m_pThemeData->clrNormalBtEdgeLight[0][1];
  m_pThemeData->clrNormalBtEdgeDark[1][1] =
      m_pThemeData->clrNormalBtEdgeDark[0][1];
  m_pThemeData->clrNormalBtEdgeLight[1][2] =
      m_pThemeData->clrNormalBtEdgeLight[0][2];
  m_pThemeData->clrNormalBtEdgeDark[1][2] =
      m_pThemeData->clrNormalBtEdgeDark[0][2];
  m_pThemeData->clrBtnCornerLight[1][0] = m_pThemeData->clrBtnCornerLight[0][0];
  m_pThemeData->clrBtnCornerLight[1][1] = m_pThemeData->clrBtnCornerLight[0][1];
  m_pThemeData->clrBtnCornerLight[1][2] = m_pThemeData->clrBtnCornerLight[0][2];
  m_pThemeData->clrBtnEdgeOut[1] = m_pThemeData->clrBtnEdgeOut[0];
  m_pThemeData->clrFormBorder[1][0] = m_pThemeData->clrFormBorder[0][0];
  m_pThemeData->clrFormBorder[1][1] = m_pThemeData->clrFormBorder[0][1];
  m_pThemeData->clrFormBorder[1][2] = m_pThemeData->clrFormBorder[0][2];
  m_pThemeData->clrFormBorder[1][3] = m_pThemeData->clrFormBorder[0][3];
  m_pThemeData->clrFormBorder[1][4] = m_pThemeData->clrFormBorder[0][4];
  m_pThemeData->clrFormBorderLight[1] = m_pThemeData->clrFormBorderLight[0];
  DeactiveForm();
}
void CFWL_FormTP::DeactiveForm() {
  TransModeColor(m_pThemeData->clrTransWhite, m_pThemeData->clrHeadBK[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite, m_pThemeData->clrHeadBK[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite, m_pThemeData->clrHeadBK[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite, m_pThemeData->clrHeadBK[1][3]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeLeft[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeLeft[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeLeft[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeRight[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeRight[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeRight[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeTop[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeTop[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeTop[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeBottom[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeBottom[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrHeadEdgeBottom[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtBKStart[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtBKStart[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtBKStart[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtBKEnd[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtBKEnd[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtBKEnd[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtEdgeLight[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtEdgeLight[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtEdgeLight[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtEdgeDark[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtEdgeDark[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrNormalBtEdgeDark[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrBtnCornerLight[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrBtnCornerLight[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrBtnCornerLight[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite, m_pThemeData->clrBtnEdgeOut[1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrFormBorder[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrFormBorder[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrFormBorder[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrFormBorder[1][3]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrFormBorder[1][4]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrFormBorderLight[1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtBKStart[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtBKStart[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtBKStart[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtBKEnd[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtBKEnd[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtBKEnd[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtEdgeLight[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtEdgeLight[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtEdgeLight[1][2]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtEdgeDark[1][0]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtEdgeDark[1][1]);
  TransModeColor(m_pThemeData->clrTransWhite,
                 m_pThemeData->clrCloseBtEdgeDark[1][2]);
}
void CFWL_FormTP::TransModeColor(FX_ARGB clrFore, FX_ARGB& clrBack) {
  int32_t iAlfaF, iRF, iGF, iBF;
  int32_t iAlfaB, iRB, iGB, iBB;
  ArgbDecode(clrFore, iAlfaF, iRF, iGF, iBF);
  ArgbDecode(clrBack, iAlfaB, iRB, iGB, iBB);
  clrBack = ArgbEncode(0xff, iRB + (iRF - iRB) * iAlfaF / 255,
                       iGB + (iGF - iGB) * iAlfaF / 255,
                       iBB + (iBF - iBB) * iAlfaF / 255);
}
void CFWL_FormTP::InitCaption(FX_BOOL bActive) {
  if (bActive) {
    CFX_FxgeDevice dev;
    CFX_Graphics gs;
    CFX_Path path;
    path.Create();
    if (m_pActiveBitmap) {
      delete m_pActiveBitmap;
      m_pActiveBitmap = NULL;
    }
    m_pActiveBitmap = new CFX_DIBitmap;
    m_pActiveBitmap->Create(1, FWLTHEME_CAPACITY_CYCaption, FXDIB_Argb);
    dev.Attach(m_pActiveBitmap);
    gs.Create(&dev);
    path.AddRectangle(0, 0, 1, 5);
    DrawAxialShading(&gs, 0, 0, 0, 5, m_pThemeData->clrHeadBK[0][0],
                     m_pThemeData->clrHeadBK[0][1], &path);
    path.Clear();
    path.AddRectangle(0, 5, 1, 15);
    DrawAxialShading(&gs, 0, 5, 0, 20, m_pThemeData->clrHeadBK[0][1],
                     m_pThemeData->clrHeadBK[0][2], &path);
    path.Clear();
    path.AddRectangle(0, 20, 1, FWLTHEME_CAPACITY_CYCaption - 19);
    DrawAxialShading(&gs, 0, 20, 0, FWLTHEME_CAPACITY_CYCaption,
                     m_pThemeData->clrHeadBK[0][2],
                     m_pThemeData->clrHeadBK[0][3], &path);
  } else {
    CFX_FxgeDevice dev;
    CFX_Graphics gs;
    CFX_Path path;
    path.Create();
    if (m_pDeactivebitmap) {
      delete m_pDeactivebitmap;
      m_pDeactivebitmap = NULL;
    }
    m_pDeactivebitmap = new CFX_DIBitmap;
    m_pDeactivebitmap->Create(1, FWLTHEME_CAPACITY_CYCaption, FXDIB_Argb);
    dev.Attach(m_pDeactivebitmap);
    gs.Create(&dev);
    path.AddRectangle(0, 0, 1, 5);
    DrawAxialShading(&gs, 0, 0, 0, 5, m_pThemeData->clrHeadBK[1][0],
                     m_pThemeData->clrHeadBK[1][1], &path);
    path.Clear();
    path.AddRectangle(0, 5, 1, 15);
    DrawAxialShading(&gs, 0, 5, 0, 20, m_pThemeData->clrHeadBK[1][1],
                     m_pThemeData->clrHeadBK[1][2], &path);
    path.Clear();
    path.AddRectangle(0, 20, 1, FWLTHEME_CAPACITY_CYCaption - 19);
    DrawAxialShading(&gs, 0, 20, 0, FWLTHEME_CAPACITY_CYCaption,
                     m_pThemeData->clrHeadBK[1][2],
                     m_pThemeData->clrHeadBK[1][3], &path);
  }
}
