// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_CARET_H_
#define XFA_FWL_CFWL_CARET_H_

#include <memory>

#include "core/fxcrt/cfx_timer.h"
#include "xfa/fgas/graphics/cfgas_gecolor.h"
#include "xfa/fwl/cfwl_widget.h"

class CFWL_Caret final : public CFWL_Widget, public CFX_Timer::CallbackIface {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_Caret() override;

  // CFWL_Widget:
  FWL_Type GetClassID() const override;
  void DrawWidget(CFGAS_GEGraphics* pGraphics,
                  const CFX_Matrix& matrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;
  void Update() override;

  // CFX_Timer::CallbackIface:
  void OnTimerFired() override;

  void ShowCaret();
  void HideCaret();

 private:
  CFWL_Caret(CFWL_App* app, const Properties& properties, CFWL_Widget* pOuter);

  void DrawCaretBK(CFGAS_GEGraphics* pGraphics, const CFX_Matrix& mtMatrix);

  std::unique_ptr<CFX_Timer> m_pTimer;
};

#endif  // XFA_FWL_CFWL_CARET_H_
