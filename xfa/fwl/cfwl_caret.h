// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_CARET_H_
#define XFA_FWL_CFWL_CARET_H_

#include <memory>

#include "xfa/fwl/cfwl_timer.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fxgraphics/cfx_color.h"

class CFWL_WidgetProperties;
class CFWL_Widget;

#define FWL_STATE_CAT_HightLight 1

class CFWL_Caret : public CFWL_Widget {
 public:
  CFWL_Caret(const CFWL_App* app,
             std::unique_ptr<CFWL_WidgetProperties> properties,
             CFWL_Widget* pOuter);
  ~CFWL_Caret() override;

  // CFWL_Widget
  FWL_Type GetClassID() const override;
  void DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix) override;
  void Update() override;

  void ShowCaret();
  void HideCaret();

 private:
  class Timer : public CFWL_Timer {
   public:
    explicit Timer(CFWL_Caret* pCaret);
    ~Timer() override {}

    void Run(CFWL_TimerInfo* hTimer) override;
  };
  friend class CFWL_Caret::Timer;

  void DrawCaretBK(CFX_Graphics* pGraphics,
                   IFWL_ThemeProvider* pTheme,
                   const CFX_Matrix* pMatrix);

  std::unique_ptr<CFWL_Caret::Timer> m_pTimer;
  CFWL_TimerInfo* m_pTimerInfo;  // not owned.
};

#endif  // XFA_FWL_CFWL_CARET_H_
