// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_SPINBUTTON_H_
#define XFA_FWL_CORE_IFWL_SPINBUTTON_H_

#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/ifwl_timer.h"
#include "xfa/fwl/core/ifwl_widget.h"
#include "xfa/fxfa/cxfa_eventparam.h"

#define FWL_STYLEEXE_SPB_Vert (1L << 0)

class CFWL_MsgMouse;
class CFWL_SpinButtonImpDelegate;
class CFWL_WidgetImpProperties;

FWL_EVENT_DEF(CFWL_EvtSpbClick, CFWL_EventType::Click, FX_BOOL m_bUp;)

class IFWL_SpinButton : public IFWL_Widget {
 public:
  explicit IFWL_SpinButton(const IFWL_App* app,
                           const CFWL_WidgetImpProperties& properties);
  ~IFWL_SpinButton() override;

  // IFWL_Widget
  void Initialize() override;
  void Finalize() override;
  FWL_Type GetClassID() const override;
  FWL_Error GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE) override;
  FWL_Error Update() override;
  FWL_WidgetHit HitTest(FX_FLOAT fx, FX_FLOAT fy) override;
  FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr) override;

  FWL_Error EnableButton(FX_BOOL bEnable, FX_BOOL bUp = TRUE);
  FX_BOOL IsButtonEnable(FX_BOOL bUp = TRUE);

 protected:
  friend class CFWL_SpinButtonImpDelegate;

  class Timer : public IFWL_Timer {
   public:
    explicit Timer(IFWL_SpinButton* pToolTip);
    ~Timer() override {}

    void Run(IFWL_TimerInfo* pTimerInfo) override;
  };
  friend class IFWL_SpinButton::Timer;

  void DrawUpButton(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    const CFX_Matrix* pMatrix);
  void DrawDownButton(CFX_Graphics* pGraphics,
                      IFWL_ThemeProvider* pTheme,
                      const CFX_Matrix* pMatrix);

  CFX_RectF m_rtClient;
  CFX_RectF m_rtUpButton;
  CFX_RectF m_rtDnButton;
  uint32_t m_dwUpState;
  uint32_t m_dwDnState;
  int32_t m_iButtonIndex;
  FX_BOOL m_bLButtonDwn;
  IFWL_TimerInfo* m_pTimerInfo;
  IFWL_SpinButton::Timer m_Timer;
};

class CFWL_SpinButtonImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_SpinButtonImpDelegate(IFWL_SpinButton* pOwner);
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix = nullptr) override;

 protected:
  void OnFocusChanged(CFWL_Message* pMsg, FX_BOOL bSet = TRUE);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnMouseLeave(CFWL_MsgMouse* pMsg);
  void OnKeyDown(CFWL_MsgKey* pMsg);
  IFWL_SpinButton* m_pOwner;
};

#endif  // XFA_FWL_CORE_IFWL_SPINBUTTON_H_
