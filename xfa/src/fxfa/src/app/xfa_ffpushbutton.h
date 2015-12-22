// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_PUSHBUTTON_IMP_H
#define _FXFA_FORMFILLER_PUSHBUTTON_IMP_H
#define XFA_FWL_PSBSTYLEEXT_HiliteNone (0L << 0)
#define XFA_FWL_PSBSTYLEEXT_HiliteInverted (1L << 0)
#define XFA_FWL_PSBSTYLEEXT_HilitePush (2L << 0)
#define XFA_FWL_PSBSTYLEEXT_HiliteOutLine (4L << 0)
class CXFA_FFPushButton : public CXFA_FFField {
 public:
  CXFA_FFPushButton(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFPushButton();
  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            FX_DWORD dwStatus = 0,
                            int32_t iRotate = 0);
  virtual FX_BOOL LoadWidget();
  virtual void UnloadWidget();
  virtual FX_BOOL PerformLayout();
  virtual void UpdateWidgetProperty();
  virtual int32_t OnProcessMessage(CFWL_Message* pMessage);
  virtual FWL_ERR OnProcessEvent(CFWL_Event* pEvent);
  virtual FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = NULL);

 protected:
  void LoadHighlightCaption();
  void LayoutHighlightCaption();
  void RenderHighlightCaption(CFX_Graphics* pGS, CFX_Matrix* pMatrix = NULL);
  FX_FLOAT GetLineWidth();
  FX_ARGB GetLineColor();
  FX_ARGB GetFillColor();
  CXFA_TextLayout* m_pRolloverTextLayout;
  CXFA_TextLayout* m_pDownTextLayout;
  CXFA_TextProvider* m_pDownProvider;
  CXFA_TextProvider* m_pRollProvider;
  IFWL_WidgetDelegate* m_pOldDelegate;
};
#endif
