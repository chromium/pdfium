// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_CARET_H_
#define XFA_FWL_CORE_IFWL_CARET_H_

#include <memory>

#include "xfa/fwl/core/ifwl_timer.h"
#include "xfa/fwl/core/ifwl_widget.h"
#include "xfa/fxgraphics/cfx_color.h"

class CFWL_WidgetImpProperties;
class IFWL_Widget;
class CFWL_CaretImpDelegate;

#define FWL_STATE_CAT_HightLight 1

class IFWL_Caret : public IFWL_Widget {
 public:
  IFWL_Caret(const IFWL_App* app,
             const CFWL_WidgetImpProperties& properties,
             IFWL_Widget* pOuter);
  ~IFWL_Caret() override;

  // IFWL_Widget
  FWL_Type GetClassID() const override;
  FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = nullptr) override;

  void ShowCaret(FX_BOOL bFlag = TRUE);
  FWL_Error GetFrequency(uint32_t& elapse);
  FWL_Error SetFrequency(uint32_t elapse);
  FWL_Error SetColor(CFX_Color crFill);

 protected:
  friend class CFWL_CaretImpDelegate;

  class Timer : public IFWL_Timer {
   public:
    explicit Timer(IFWL_Caret* pCaret);
    ~Timer() override {}

    void Run(IFWL_TimerInfo* hTimer) override;
  };
  friend class IFWL_Caret::Timer;

  void DrawCaretBK(CFX_Graphics* pGraphics,
                   IFWL_ThemeProvider* pTheme,
                   const CFX_Matrix* pMatrix);

  std::unique_ptr<IFWL_Caret::Timer> m_pTimer;
  IFWL_TimerInfo* m_pTimerInfo;  // not owned.
  uint32_t m_dwElapse;
  CFX_Color m_crFill;
  FX_BOOL m_bSetColor;
};

class CFWL_CaretImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_CaretImpDelegate(IFWL_Caret* pOwner);
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix = nullptr) override;

 protected:
  IFWL_Caret* m_pOwner;
};

#endif  // XFA_FWL_CORE_IFWL_CARET_H_
