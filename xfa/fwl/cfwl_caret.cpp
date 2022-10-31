// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_caret.h"

#include <utility>

#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace {

const uint32_t kBlinkPeriodMs = 600;

constexpr int kStateHighlight = (1 << 0);

}  // namespace

CFWL_Caret::CFWL_Caret(CFWL_App* app,
                       const Properties& properties,
                       CFWL_Widget* pOuter)
    : CFWL_Widget(app, properties, pOuter) {
  SetStates(kStateHighlight);
}

CFWL_Caret::~CFWL_Caret() = default;

FWL_Type CFWL_Caret::GetClassID() const {
  return FWL_Type::Caret;
}

void CFWL_Caret::Update() {}

void CFWL_Caret::DrawWidget(CFGAS_GEGraphics* pGraphics,
                            const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;

  DrawCaretBK(pGraphics, matrix);
}

void CFWL_Caret::ShowCaret() {
  m_pTimer = std::make_unique<CFX_Timer>(GetFWLApp()->GetTimerHandler(), this,
                                         kBlinkPeriodMs);
  RemoveStates(FWL_STATE_WGT_Invisible);
  SetStates(kStateHighlight);
}

void CFWL_Caret::HideCaret() {
  m_pTimer.reset();
  SetStates(FWL_STATE_WGT_Invisible);
}

void CFWL_Caret::DrawCaretBK(CFGAS_GEGraphics* pGraphics,
                             const CFX_Matrix& mtMatrix) {
  if (!(m_Properties.m_dwStates & kStateHighlight))
    return;

  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kBackground, this,
                             pGraphics);
  param.m_PartRect = CFX_RectF(0, 0, GetWidgetRect().Size());
  param.m_dwStates = CFWL_PartState::kHightLight;
  param.m_matrix = mtMatrix;
  GetThemeProvider()->DrawBackground(param);
}

void CFWL_Caret::OnProcessMessage(CFWL_Message* pMessage) {}

void CFWL_Caret::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                              const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_Caret::OnTimerFired() {
  if (!(GetStates() & kStateHighlight))
    SetStates(kStateHighlight);
  else
    RemoveStates(kStateHighlight);

  CFX_RectF rt = GetWidgetRect();
  RepaintRect(CFX_RectF(0, 0, rt.width + 1, rt.height));
}
