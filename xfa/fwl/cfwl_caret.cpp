// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_caret.h"

#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_widgetproperties.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace {

const uint32_t kBlinkPeriodMs = 600;

constexpr int kStateHighlight = (1 << 0);

}  // namespace

CFWL_Caret::CFWL_Caret(const CFWL_App* app,
                       std::unique_ptr<CFWL_WidgetProperties> properties,
                       CFWL_Widget* pOuter)
    : CFWL_Widget(app, std::move(properties), pOuter) {
  SetStates(kStateHighlight);
}

CFWL_Caret::~CFWL_Caret() = default;

FWL_Type CFWL_Caret::GetClassID() const {
  return FWL_Type::Caret;
}

void CFWL_Caret::Update() {}

void CFWL_Caret::DrawWidget(CXFA_Graphics* pGraphics,
                            const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;
  if (!m_pProperties->m_pThemeProvider)
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  if (!m_pProperties->m_pThemeProvider)
    return;

  DrawCaretBK(pGraphics, m_pProperties->m_pThemeProvider.Get(), &matrix);
}

void CFWL_Caret::ShowCaret() {
  m_pTimer = pdfium::MakeUnique<CFX_Timer>(
      GetOwnerApp()->GetAdapterNative()->GetTimerHandler(), this,
      kBlinkPeriodMs);
  RemoveStates(FWL_WGTSTATE_Invisible);
  SetStates(kStateHighlight);
}

void CFWL_Caret::HideCaret() {
  m_pTimer.reset();
  SetStates(FWL_WGTSTATE_Invisible);
}

void CFWL_Caret::DrawCaretBK(CXFA_Graphics* pGraphics,
                             IFWL_ThemeProvider* pTheme,
                             const CFX_Matrix* pMatrix) {
  if (!(m_pProperties->m_dwStates & kStateHighlight))
    return;

  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_pGraphics = pGraphics;
  param.m_rtPart = CFX_RectF(0, 0, GetWidgetRect().Size());
  param.m_iPart = CFWL_Part::Background;
  param.m_dwStates = CFWL_PartState_HightLight;
  if (pMatrix)
    param.m_matrix.Concat(*pMatrix);
  pTheme->DrawBackground(param);
}

void CFWL_Caret::OnProcessMessage(CFWL_Message* pMessage) {}

void CFWL_Caret::OnDrawWidget(CXFA_Graphics* pGraphics,
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
