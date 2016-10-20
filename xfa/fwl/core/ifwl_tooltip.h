// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_TOOLTIP_H_
#define XFA_FWL_CORE_IFWL_TOOLTIP_H_

#include "xfa/fwl/core/ifwl_form.h"
#include "xfa/fwl/core/ifwl_timer.h"

class CFWL_WidgetImpProperties;
class IFWL_Widget;
class CFWL_ToolTipImpDelegate;

#define FWL_CLASS_ToolTip L"FWL_TOOLTIP"
#define FWL_STYLEEXT_TTP_Rectangle (0L << 3)
#define FWL_STYLEEXT_TTP_RoundCorner (1L << 3)
#define FWL_STYLEEXT_TTP_Balloon (1L << 4)
#define FWL_STYLEEXT_TTP_Multiline (1L << 5)
#define FWL_STYLEEXT_TTP_NoAnchor (1L << 6)

class IFWL_ToolTipDP : public IFWL_DataProvider {
 public:
  // IFWL_DataProvider
  FWL_Error GetCaption(IFWL_Widget* pWidget,
                       CFX_WideString& wsCaption) override = 0;

  virtual int32_t GetInitialDelay(IFWL_Widget* pWidget) = 0;
  virtual int32_t GetAutoPopDelay(IFWL_Widget* pWidget) = 0;
  virtual CFX_DIBitmap* GetToolTipIcon(IFWL_Widget* pWidget) = 0;
  virtual CFX_SizeF GetToolTipIconSize(IFWL_Widget* pWidget) = 0;
};

class IFWL_ToolTip : public IFWL_Form {
 public:
  static IFWL_ToolTip* Create(const CFWL_WidgetImpProperties& properties,
                              IFWL_Widget* pOuter);

  IFWL_ToolTip(const CFWL_WidgetImpProperties& properties, IFWL_Widget* pOuter);
  ~IFWL_ToolTip() override;

  // IFWL_Widget
  FWL_Error GetClassName(CFX_WideString& wsClass) const override;
  FWL_Type GetClassID() const override;
  FWL_Error Initialize() override;
  FWL_Error Finalize() override;
  FWL_Error GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE) override;
  FWL_Error Update() override;
  FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr) override;
  void SetStates(uint32_t dwStates, FX_BOOL bSet) override;
  FWL_Error GetClientRect(CFX_RectF& rect) override;

  void SetAnchor(const CFX_RectF& rtAnchor);
  void Show();
  void Hide();

 protected:
  friend class CFWL_ToolTipImpDelegate;
  friend class CFWL_ToolTipTimer;

  class CFWL_ToolTipTimer : public IFWL_Timer {
   public:
    CFWL_ToolTipTimer() {}
    explicit CFWL_ToolTipTimer(IFWL_ToolTip* pToolTip);
    ~CFWL_ToolTipTimer() override {}

    void Run(IFWL_TimerInfo* pTimerInfo) override;

    IFWL_ToolTip* m_pToolTip;
  };

  void DrawBkground(CFX_Graphics* pGraphics,
                    IFWL_ThemeProvider* pTheme,
                    const CFX_Matrix* pMatrix);
  void DrawText(CFX_Graphics* pGraphics,
                IFWL_ThemeProvider* pTheme,
                const CFX_Matrix* pMatrix);
  void UpdateTextOutStyles();
  void RefreshToolTipPos();

  CFX_RectF m_rtClient;
  CFX_RectF m_rtCaption;
  FX_BOOL m_bBtnDown;
  uint32_t m_dwTTOStyles;
  int32_t m_iTTOAlign;
  CFX_RectF m_rtAnchor;
  IFWL_TimerInfo* m_pTimerInfoShow;
  IFWL_TimerInfo* m_pTimerInfoHide;
  CFWL_ToolTipTimer m_TimerShow;
  CFWL_ToolTipTimer m_TimerHide;
};

class CFWL_ToolTipImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_ToolTipImpDelegate(IFWL_ToolTip* pOwner);
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
  IFWL_ToolTip* m_pOwner;
};

#endif  // XFA_FWL_CORE_IFWL_TOOLTIP_H_
