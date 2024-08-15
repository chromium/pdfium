// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffpushbutton.h"

#include "core/fxcrt/check.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fgas/graphics/cfgas_gecolor.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_pushbutton.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_textlayout.h"
#include "xfa/fxfa/cxfa_textprovider.h"
#include "xfa/fxfa/parser/cxfa_border.h"
#include "xfa/fxfa/parser/cxfa_button.h"
#include "xfa/fxfa/parser/cxfa_caption.h"
#include "xfa/fxfa/parser/cxfa_edge.h"

CXFA_FFPushButton::CXFA_FFPushButton(CXFA_Node* pNode, CXFA_Button* button)
    : CXFA_FFField(pNode), button_(button) {}

CXFA_FFPushButton::~CXFA_FFPushButton() = default;

void CXFA_FFPushButton::Trace(cppgc::Visitor* visitor) const {
  CXFA_FFField::Trace(visitor);
  visitor->Trace(m_pRolloverTextLayout);
  visitor->Trace(m_pDownTextLayout);
  visitor->Trace(m_pRollProvider);
  visitor->Trace(m_pDownProvider);
  visitor->Trace(m_pOldDelegate);
  visitor->Trace(button_);
}

void CXFA_FFPushButton::RenderWidget(CFGAS_GEGraphics* pGS,
                                     const CFX_Matrix& matrix,
                                     HighlightOption highlight) {
  if (!HasVisibleStatus())
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, highlight);
  RenderHighlightCaption(pGS, &mtRotate);

  CFX_RectF rtWidget = GetRectWithoutRotate();
  CFX_Matrix mt(1, 0, 0, 1, rtWidget.left, rtWidget.top);
  mt.Concat(mtRotate);
  GetApp()->GetFWLWidgetMgr()->OnDrawWidget(GetNormalWidget(), pGS, mt);
}

bool CXFA_FFPushButton::LoadWidget() {
  DCHECK(!IsLoaded());

  CFWL_PushButton* pPushButton = cppgc::MakeGarbageCollected<CFWL_PushButton>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp());
  m_pOldDelegate = pPushButton->GetDelegate();
  pPushButton->SetDelegate(this);
  SetNormalWidget(pPushButton);
  pPushButton->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pPushButton->GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pPushButton, pPushButton);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pPushButton);
    UpdateWidgetProperty();
    LoadHighlightCaption();
  }

  return CXFA_FFField::LoadWidget();
}

void CXFA_FFPushButton::UpdateWidgetProperty() {
  uint32_t dwStyleEx = 0;
  switch (button_->GetHighlight()) {
    case XFA_AttributeValue::Inverted:
      dwStyleEx = XFA_FWL_PSBSTYLEEXT_HiliteInverted;
      break;
    case XFA_AttributeValue::Outline:
      dwStyleEx = XFA_FWL_PSBSTYLEEXT_HiliteOutLine;
      break;
    case XFA_AttributeValue::Push:
      dwStyleEx = XFA_FWL_PSBSTYLEEXT_HilitePush;
      break;
    default:
      break;
  }
  GetNormalWidget()->ModifyStyleExts(dwStyleEx, 0xFFFFFFFF);
}

bool CXFA_FFPushButton::PerformLayout() {
  CXFA_FFWidget::PerformLayout();
  CFX_RectF rtWidget = GetRectWithoutRotate();

  m_UIRect = rtWidget;
  CXFA_Margin* margin = m_pNode->GetMarginIfExists();
  XFA_RectWithoutMargin(&rtWidget, margin);

  m_CaptionRect = rtWidget;

  CXFA_Caption* caption = m_pNode->GetCaptionIfExists();
  CXFA_Margin* captionMargin = caption ? caption->GetMarginIfExists() : nullptr;
  XFA_RectWithoutMargin(&m_CaptionRect, captionMargin);

  LayoutHighlightCaption();
  SetFWLRect();
  if (GetNormalWidget())
    GetNormalWidget()->Update();

  return true;
}

float CXFA_FFPushButton::GetLineWidth() {
  CXFA_Border* border = m_pNode->GetBorderIfExists();
  if (border && border->GetPresence() == XFA_AttributeValue::Visible) {
    CXFA_Edge* edge = border->GetEdgeIfExists(0);
    return edge ? edge->GetThickness() : 0;
  }
  return 0;
}

FX_ARGB CXFA_FFPushButton::GetLineColor() {
  return 0xFF000000;
}

FX_ARGB CXFA_FFPushButton::GetFillColor() {
  return 0xFFFFFFFF;
}

void CXFA_FFPushButton::LoadHighlightCaption() {
  CXFA_Caption* caption = m_pNode->GetCaptionIfExists();
  if (!caption || caption->IsHidden())
    return;

  if (m_pNode->HasButtonRollover()) {
    if (!m_pRollProvider) {
      m_pRollProvider = cppgc::MakeGarbageCollected<CXFA_TextProvider>(
          GetDoc()->GetHeap()->GetAllocationHandle(), m_pNode.Get(),
          CXFA_TextProvider::Type::kRollover);
    }
    m_pRolloverTextLayout = cppgc::MakeGarbageCollected<CXFA_TextLayout>(
        GetDoc()->GetHeap()->GetAllocationHandle(), GetDoc(), m_pRollProvider);
  }
  if (m_pNode->HasButtonDown()) {
    if (!m_pDownProvider) {
      m_pDownProvider = cppgc::MakeGarbageCollected<CXFA_TextProvider>(
          GetDoc()->GetHeap()->GetAllocationHandle(), m_pNode.Get(),
          CXFA_TextProvider::Type::kDown);
    }
    m_pDownTextLayout = cppgc::MakeGarbageCollected<CXFA_TextLayout>(
        GetDoc()->GetHeap()->GetAllocationHandle(), GetDoc(), m_pDownProvider);
  }
}

void CXFA_FFPushButton::LayoutHighlightCaption() {
  CFX_SizeF sz(m_CaptionRect.width, m_CaptionRect.height);
  LayoutCaption();
  if (m_pRolloverTextLayout)
    m_pRolloverTextLayout->Layout(sz);
  if (m_pDownTextLayout)
    m_pDownTextLayout->Layout(sz);
}

void CXFA_FFPushButton::RenderHighlightCaption(CFGAS_GEGraphics* pGS,
                                               CFX_Matrix* pMatrix) {
  CXFA_TextLayout* pCapTextLayout = m_pNode->GetCaptionTextLayout();
  CXFA_Caption* caption = m_pNode->GetCaptionIfExists();
  if (!caption || !caption->IsVisible())
    return;

  CFX_RenderDevice* pRenderDevice = pGS->GetRenderDevice();
  CFX_RectF rtClip = m_CaptionRect;
  rtClip.Intersect(GetRectWithoutRotate());
  CFX_Matrix mt(1, 0, 0, 1, m_CaptionRect.left, m_CaptionRect.top);
  if (pMatrix) {
    rtClip = pMatrix->TransformRect(rtClip);
    mt.Concat(*pMatrix);
  }

  uint32_t dwState = GetNormalWidget()->GetStates();
  if (m_pDownTextLayout && (dwState & FWL_STATE_PSB_Pressed) &&
      (dwState & FWL_STATE_PSB_Hovered)) {
    if (m_pDownTextLayout->DrawString(pRenderDevice, mt, rtClip, 0))
      return;
  } else if (m_pRolloverTextLayout && (dwState & FWL_STATE_PSB_Hovered)) {
    if (m_pRolloverTextLayout->DrawString(pRenderDevice, mt, rtClip, 0))
      return;
  }

  if (pCapTextLayout)
    pCapTextLayout->DrawString(pRenderDevice, mt, rtClip, 0);
}

void CXFA_FFPushButton::OnProcessMessage(CFWL_Message* pMessage) {
  m_pOldDelegate->OnProcessMessage(pMessage);
}

void CXFA_FFPushButton::OnProcessEvent(pdfium::CFWL_Event* pEvent) {
  m_pOldDelegate->OnProcessEvent(pEvent);
  CXFA_FFField::OnProcessEvent(pEvent);
}

void CXFA_FFPushButton::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& matrix) {
  auto* pWidget = GetNormalWidget();
  if (pWidget->GetStyleExts() & XFA_FWL_PSBSTYLEEXT_HiliteInverted) {
    if ((pWidget->GetStates() & FWL_STATE_PSB_Pressed) &&
        (pWidget->GetStates() & FWL_STATE_PSB_Hovered)) {
      CFX_RectF rtFill(0, 0, pWidget->GetWidgetRect().Size());
      float fLineWith = GetLineWidth();
      rtFill.Deflate(fLineWith, fLineWith);
      CFGAS_GEPath path;
      path.AddRectangle(rtFill.left, rtFill.top, rtFill.width, rtFill.height);
      pGraphics->SetFillColor(CFGAS_GEColor(ArgbEncode(128, 128, 255, 255)));
      pGraphics->FillPath(path, CFX_FillRenderOptions::FillType::kWinding,
                          matrix);
    }
    return;
  }

  if (pWidget->GetStyleExts() & XFA_FWL_PSBSTYLEEXT_HiliteOutLine) {
    if ((pWidget->GetStates() & FWL_STATE_PSB_Pressed) &&
        (pWidget->GetStates() & FWL_STATE_PSB_Hovered)) {
      float fLineWidth = GetLineWidth();
      pGraphics->SetStrokeColor(CFGAS_GEColor(ArgbEncode(255, 128, 255, 255)));
      pGraphics->SetLineWidth(fLineWidth);

      CFGAS_GEPath path;
      CFX_RectF rect = pWidget->GetWidgetRect();
      path.AddRectangle(0, 0, rect.width, rect.height);
      pGraphics->StrokePath(path, matrix);
    }
  }
}

FormFieldType CXFA_FFPushButton::GetFormFieldType() {
  return FormFieldType::kXFA_PushButton;
}
