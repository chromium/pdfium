// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_SPINBUTTON_IMP_H
#define _FWL_SPINBUTTON_IMP_H
class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class IFWL_Widget;
class IFWL_Timer;
class CFWL_SpinButtonImp;
class CFWL_SpinButtonImpDelegate;
class CFWL_SpinButtonImp : public CFWL_WidgetImp, public IFWL_Timer {
 public:
  CFWL_SpinButtonImp(const CFWL_WidgetImpProperties& properties,
                     IFWL_Widget* pOuter);
  ~CFWL_SpinButtonImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR Update();
  virtual FX_DWORD HitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);
  virtual int32_t Run(FWL_HTIMER hTimer);
  FWL_ERR EnableButton(FX_BOOL bEnable, FX_BOOL bUp = TRUE);
  FX_BOOL IsButtonEnable(FX_BOOL bUp = TRUE);

 protected:
  void DrawUpButton(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    const CFX_Matrix* pMatrix);
  void DrawDownButton(CFX_Graphics* pGraphics,
                      IFWL_ThemeProvider* pTheme,
                      const CFX_Matrix* pMatrix);
  CFX_RectF m_rtClient;
  CFX_RectF m_rtUpButton;
  CFX_RectF m_rtDnButton;
  FX_DWORD m_dwUpState;
  FX_DWORD m_dwDnState;
  int32_t m_iButtonIndex;
  FX_BOOL m_bLButtonDwn;
  FWL_HTIMER m_hTimer;
  friend class CFWL_SpinButtonImpDelegate;
};
class CFWL_SpinButtonImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_SpinButtonImpDelegate(CFWL_SpinButtonImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnProcessEvent(CFWL_Event* pEvent) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;

 protected:
  void OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnMouseLeave(CFWL_MsgMouse* pMsg);
  void OnKeyDown(CFWL_MsgKey* pMsg);
  CFWL_SpinButtonImp* m_pOwner;
};
#endif
