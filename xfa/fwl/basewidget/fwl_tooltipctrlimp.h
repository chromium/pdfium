// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_BASEWIDGET_FWL_TOOLTIPCTRLIMP_H_
#define XFA_FWL_BASEWIDGET_FWL_TOOLTIPCTRLIMP_H_

#include "xfa/fwl/core/fwl_formimp.h"
#include "xfa/fwl/core/fwl_widgetimp.h"
#include "xfa/fwl/core/ifwl_timer.h"

class CFWL_WidgetImpProperties;
class IFWL_Widget;
class CFWL_ToolTipImpDelegate;

class CFWL_ToolTipImp : public CFWL_FormImp {
 public:
  CFWL_ToolTipImp(const CFWL_WidgetImpProperties& properties,
                  IFWL_Widget* pOuter);
  virtual ~CFWL_ToolTipImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual uint32_t GetClassID() const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR Update();
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FWL_ERR SetStates(uint32_t dwStates, FX_BOOL bSet);
  virtual FWL_ERR GetClientRect(CFX_RectF& rect);
  FWL_ERR SetAnchor(const CFX_RectF& rtAnchor);
  FWL_ERR Show();
  FWL_ERR Hide();

 protected:
  void DrawBkground(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    const CFX_Matrix* pMatrix);
  void DrawText(CFX_Graphics* pGraphics,
                IFWL_ThemeProvider* pTheme,
                const CFX_Matrix* pMatrix);
  void UpdateTextOutStyles();
  void RefreshToolTipPos();
  class CFWL_ToolTipTimer : public IFWL_Timer {
   public:
    CFWL_ToolTipTimer() {}
    ~CFWL_ToolTipTimer() {}
    CFWL_ToolTipTimer(CFWL_ToolTipImp* pToolTip);
    virtual int32_t Run(FWL_HTIMER hTimer);
    CFWL_ToolTipImp* m_pToolTip;
  };
  CFX_RectF m_rtClient;
  CFX_RectF m_rtCaption;
  FX_BOOL m_bBtnDown;
  uint32_t m_dwTTOStyles;
  int32_t m_iTTOAlign;
  CFX_RectF m_rtAnchor;
  FWL_HTIMER m_hTimerShow;
  FWL_HTIMER m_hTimerHide;
  CFWL_ToolTipTimer* m_pTimer;
  CFWL_ToolTipTimer m_TimerShow;
  CFWL_ToolTipTimer m_TimerHide;
  friend class CFWL_ToolTipImpDelegate;
  friend class CFWL_ToolTipTimer;
};
class CFWL_ToolTipImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_ToolTipImpDelegate(CFWL_ToolTipImp* pOwner);
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
  CFWL_ToolTipImp* m_pOwner;
};

#endif  // XFA_FWL_BASEWIDGET_FWL_TOOLTIPCTRLIMP_H_
