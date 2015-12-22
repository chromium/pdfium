// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_FIELD_IMP_H
#define _FXFA_FORMFILLER_FIELD_IMP_H
#define XFA_MINUI_HEIGHT 4.32f
#define XFA_DEFAULTUI_HEIGHT 2.0f
class CXFA_TextLayout;
class CXFA_FFField : public CXFA_FFWidget, public IFWL_WidgetDelegate {
 public:
  CXFA_FFField(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFField();

  virtual FX_BOOL GetBBox(CFX_RectF& rtBox,
                          FX_DWORD dwStatus,
                          FX_BOOL bDrawFocus = FALSE);
  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            FX_DWORD dwStatus = 0,
                            int32_t iRotate = 0);
  virtual FX_BOOL IsLoaded();
  virtual FX_BOOL LoadWidget();
  virtual void UnloadWidget();
  virtual FX_BOOL PerformLayout();
  virtual void UpdateFWL();
  FX_DWORD UpdateUIProperty();
  virtual FX_BOOL OnMouseEnter();
  virtual FX_BOOL OnMouseExit();
  virtual FX_BOOL OnLButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnLButtonUp(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnLButtonDblClk(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnMouseMove(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnMouseWheel(FX_DWORD dwFlags,
                               int16_t zDelta,
                               FX_FLOAT fx,
                               FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnRButtonUp(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDblClk(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);

  virtual FX_BOOL OnSetFocus(CXFA_FFWidget* pOldWidget);
  virtual FX_BOOL OnKillFocus(CXFA_FFWidget* pNewWidget);
  virtual FX_BOOL OnKeyDown(FX_DWORD dwKeyCode, FX_DWORD dwFlags);
  virtual FX_BOOL OnKeyUp(FX_DWORD dwKeyCode, FX_DWORD dwFlags);
  virtual FX_BOOL OnChar(FX_DWORD dwChar, FX_DWORD dwFlags);
  virtual FX_DWORD OnHitTest(FX_FLOAT fx, FX_FLOAT fy);
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
                     FX_DWORD dwStatus,
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
  virtual int32_t OnProcessMessage(CFWL_Message* pMessage);
  virtual FWL_ERR OnProcessEvent(CFWL_Event* pEvent);
  virtual FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = NULL);

 protected:
  CFWL_Widget* m_pNormalWidget;
  CFX_RectF m_rtUI;
  CFX_RectF m_rtCaption;
};
#endif
