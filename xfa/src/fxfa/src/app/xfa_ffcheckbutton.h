// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_CHECKBUTTON_IMP_H
#define _FXFA_FORMFILLER_CHECKBUTTON_IMP_H
class CXFA_FFExclGroup;
class CXFA_FFCheckButton : public CXFA_FFField {
 public:
  CXFA_FFCheckButton(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFCheckButton();
  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            FX_DWORD dwStatus = 0,
                            int32_t iRotate = 0);

  virtual FX_BOOL LoadWidget();
  virtual FX_BOOL PerformLayout();
  virtual FX_BOOL UpdateFWLData();
  virtual void UpdateWidgetProperty();
  virtual FX_BOOL OnLButtonUp(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  void SetFWLCheckState(XFA_CHECKSTATE eCheckState);
  virtual int32_t OnProcessMessage(CFWL_Message* pMessage);
  virtual FWL_ERR OnProcessEvent(CFWL_Event* pEvent);
  virtual FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = NULL);

 protected:
  virtual FX_BOOL CommitData();
  virtual FX_BOOL IsDataChanged();
  void CapLeftRightPlacement(CXFA_Margin mgCap);
  void AddUIMargin(int32_t iCapPlacement);
  XFA_CHECKSTATE FWLState2XFAState();
  IFWL_WidgetDelegate* m_pOldDelegate;
  CFX_RectF m_rtCheckBox;
};
#endif
