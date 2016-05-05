// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FFFIELD_H_
#define XFA_FXFA_APP_XFA_FFFIELD_H_

#include "xfa/fwl/core/ifwl_widgetdelegate.h"
#include "xfa/fwl/lightwidget/cfwl_widget.h"
#include "xfa/fxfa/include/xfa_ffpageview.h"
#include "xfa/fxfa/include/xfa_ffwidget.h"

#define XFA_MINUI_HEIGHT 4.32f
#define XFA_DEFAULTUI_HEIGHT 2.0f

class CXFA_FFField : public CXFA_FFWidget, public IFWL_WidgetDelegate {
 public:
  CXFA_FFField(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFField();

  virtual FX_BOOL GetBBox(CFX_RectF& rtBox,
                          uint32_t dwStatus,
                          FX_BOOL bDrawFocus = FALSE);
  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            uint32_t dwStatus = 0,
                            int32_t iRotate = 0);
  virtual FX_BOOL IsLoaded();
  virtual FX_BOOL LoadWidget();
  virtual void UnloadWidget();
  virtual FX_BOOL PerformLayout();
  virtual void UpdateFWL();
  uint32_t UpdateUIProperty();
  virtual FX_BOOL OnMouseEnter();
  virtual FX_BOOL OnMouseExit();
  virtual FX_BOOL OnLButtonDown(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnLButtonUp(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnLButtonDblClk(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnMouseMove(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnMouseWheel(uint32_t dwFlags,
                               int16_t zDelta,
                               FX_FLOAT fx,
                               FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDown(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnRButtonUp(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDblClk(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);

  virtual FX_BOOL OnSetFocus(CXFA_FFWidget* pOldWidget);
  virtual FX_BOOL OnKillFocus(CXFA_FFWidget* pNewWidget);
  virtual FX_BOOL OnKeyDown(uint32_t dwKeyCode, uint32_t dwFlags);
  virtual FX_BOOL OnKeyUp(uint32_t dwKeyCode, uint32_t dwFlags);
  virtual FX_BOOL OnChar(uint32_t dwChar, uint32_t dwFlags);
  virtual FWL_WidgetHit OnHitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnSetCursor(FX_FLOAT fx, FX_FLOAT fy);

 protected:
  virtual FX_BOOL PtInActiveRect(FX_FLOAT fx, FX_FLOAT fy);
  virtual void SetFWLRect();
  void SetFWLThemeProvider();
  CFWL_Widget* GetNormalWidget() { return m_pNormalWidget; }
  void FWLToClient(FX_FLOAT& fx, FX_FLOAT& fy);
  void LayoutCaption();
  void RenderCaption(CFX_Graphics* pGS, CFX_Matrix* pMatrix = NULL);

  int32_t CalculateOverride();
  int32_t CalculateWidgetAcc(CXFA_WidgetAcc* pAcc);
  FX_BOOL ProcessCommittedData();
  virtual FX_BOOL CommitData();
  virtual FX_BOOL IsDataChanged();
  void DrawHighlight(CFX_Graphics* pGS,
                     CFX_Matrix* pMatrix,
                     uint32_t dwStatus,
                     FX_BOOL bEllipse = FALSE);
  void DrawFocus(CFX_Graphics* pGS, CFX_Matrix* pMatrix);
  void TranslateFWLMessage(CFWL_Message* pMessage);
  void CapPlacement();
  void CapTopBottomPlacement(CXFA_Caption caption,
                             const CFX_RectF& rtWidget,
                             int32_t iCapPlacement);
  void CapLeftRightPlacement(CXFA_Caption caption,
                             const CFX_RectF& rtWidget,
                             int32_t iCapPlacement);
  void SetEditScrollOffset();

 public:
  virtual void OnProcessMessage(CFWL_Message* pMessage);
  virtual void OnProcessEvent(CFWL_Event* pEvent);
  virtual void OnDrawWidget(CFX_Graphics* pGraphics,
                            const CFX_Matrix* pMatrix = NULL);

 protected:
  CFWL_Widget* m_pNormalWidget;
  CFX_RectF m_rtUI;
  CFX_RectF m_rtCaption;
};

#endif  // XFA_FXFA_APP_XFA_FFFIELD_H_
