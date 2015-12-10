// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_PUSHBUTTON_IMP_H
#define _FWL_PUSHBUTTON_IMP_H
class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class IFWL_Widget;
class CFWL_PushButtonImp;
class CFWL_PushButtonImpDelegate;
class CFWL_PushButtonImp : public CFWL_WidgetImp {
 public:
  CFWL_PushButtonImp(const CFWL_WidgetImpProperties& properties,
                     IFWL_Widget* pOuter);
  virtual ~CFWL_PushButtonImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR SetStates(FX_DWORD dwStates, FX_BOOL bSet = TRUE);
  virtual FWL_ERR Update();
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);

 protected:
  void DrawBkground(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    const CFX_Matrix* pMatrix);
  void DrawText(CFX_Graphics* pGraphics,
                IFWL_ThemeProvider* pTheme,
                const CFX_Matrix* pMatrix);
  FX_DWORD GetPartStates();
  void UpdateTextOutStyles();
  CFX_RectF m_rtClient;
  CFX_RectF m_rtCaption;
  FX_BOOL m_bBtnDown;
  FX_DWORD m_dwTTOStyles;
  int32_t m_iTTOAlign;
  friend class CFWL_PushButtonImpDelegate;
};
class CFWL_PushButtonImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_PushButtonImpDelegate(CFWL_PushButtonImp* pOwner);
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
  CFWL_PushButtonImp* m_pOwner;
};
#endif
