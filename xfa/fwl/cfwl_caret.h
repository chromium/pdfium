// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_CARET_H_
#define XFA_FWL_CFWL_CARET_H_

#include <memory>

#include "core/fxcrt/cfx_timer.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fxgraphics/cxfa_gecolor.h"

class CFWL_WidgetProperties;
class CFWL_Widget;

class CFWL_Caret final : public CFWL_Widget, public CFX_Timer::CallbackIface {
 public:
  CFWL_Caret(const CFWL_App* app,
             std::unique_ptr<CFWL_WidgetProperties> properties,
             CFWL_Widget* pOuter);
  ~CFWL_Caret() override;

  // CFWL_Widget:
  FWL_Type GetClassID() const override;
  void DrawWidget(CXFA_Graphics* pGraphics, const CFX_Matrix& matrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CXFA_Graphics* pGraphics,
                    const CFX_Matrix& matrix) override;
  void Update() override;

  // CFX_Timer::CallbackIface:
  void OnTimerFired() override;

  void ShowCaret();
  void HideCaret();

 private:
  void DrawCaretBK(CXFA_Graphics* pGraphics,
                   IFWL_ThemeProvider* pTheme,
                   const CFX_Matrix* pMatrix);

  std::unique_ptr<CFX_Timer> m_pTimer;
};

#endif  // XFA_FWL_CFWL_CARET_H_
