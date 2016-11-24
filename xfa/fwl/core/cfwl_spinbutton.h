// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_SPINBUTTON_H_
#define XFA_FWL_CORE_CFWL_SPINBUTTON_H_

#include <memory>

#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/cfwl_timer.h"
#include "xfa/fwl/core/ifwl_widget.h"
#include "xfa/fxfa/cxfa_eventparam.h"

#define FWL_STYLEEXE_SPB_Vert (1L << 0)

class CFWL_MsgMouse;
class CFWL_WidgetProperties;

class CFWL_SpinButton : public IFWL_Widget {
 public:
  CFWL_SpinButton(const CFWL_App* app,
                  std::unique_ptr<CFWL_WidgetProperties> properties);
  ~CFWL_SpinButton() override;

  // IFWL_Widget
  FWL_Type GetClassID() const override;
  void GetWidgetRect(CFX_RectF& rect, bool bAutoSize = false) override;
  void Update() override;
  FWL_WidgetHit HitTest(FX_FLOAT fx, FX_FLOAT fy) override;
  void DrawWidget(CFX_Graphics* pGraphics,
                  const CFX_Matrix* pMatrix = nullptr) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix) override;

 private:
  class Timer : public CFWL_Timer {
   public:
    explicit Timer(CFWL_SpinButton* pToolTip);
    ~Timer() override {}

    void Run(CFWL_TimerInfo* pTimerInfo) override;
  };
  friend class CFWL_SpinButton::Timer;

  void EnableButton(bool bEnable, bool bUp = true);
  bool IsButtonEnabled(bool bUp = true);
  void DrawUpButton(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    const CFX_Matrix* pMatrix);
  void DrawDownButton(CFX_Graphics* pGraphics,
                      IFWL_ThemeProvider* pTheme,
                      const CFX_Matrix* pMatrix);
  void OnFocusChanged(CFWL_Message* pMsg, bool bSet);
  void OnLButtonDown(CFWL_MsgMouse* pMsg);
  void OnLButtonUp(CFWL_MsgMouse* pMsg);
  void OnMouseMove(CFWL_MsgMouse* pMsg);
  void OnMouseLeave(CFWL_MsgMouse* pMsg);
  void OnKeyDown(CFWL_MsgKey* pMsg);

  CFX_RectF m_rtClient;
  CFX_RectF m_rtUpButton;
  CFX_RectF m_rtDnButton;
  uint32_t m_dwUpState;
  uint32_t m_dwDnState;
  int32_t m_iButtonIndex;
  bool m_bLButtonDwn;
  CFWL_TimerInfo* m_pTimerInfo;
  CFWL_SpinButton::Timer m_Timer;
};

#endif  // XFA_FWL_CORE_CFWL_SPINBUTTON_H_
