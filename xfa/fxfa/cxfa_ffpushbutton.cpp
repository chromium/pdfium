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
  visitor->Trace(rollover_text_layout_);
  visitor->Trace(down_text_layout_);
  visitor->Trace(roll_provider_);
  visitor->Trace(down_provider_);
  visitor->Trace(old_delegate_);
  visitor->Trace(button_);
}

void CXFA_FFPushButton::RenderWidget(CFGAS_GEGraphics* pGS,
                                     const CFX_Matrix& matrix,
                                     HighlightOption highlight) {
  if (!HasVisibleStatus()) {
    return;
  }

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
  old_delegate_ = pPushButton->GetDelegate();
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

void CXFA_FFPushButton::PerformLayout() {
  CXFA_FFWidget::PerformLayout();
  CFX_RectF rtWidget = GetRectWithoutRotate();

  uirect_ = rtWidget;
  CXFA_Margin* margin = node_->GetMarginIfExists();
  XFA_RectWithoutMargin(&rtWidget, margin);

  caption_rect_ = rtWidget;

  CXFA_Caption* caption = node_->GetCaptionIfExists();
  CXFA_Margin* captionMargin = caption ? caption->GetMarginIfExists() : nullptr;
  XFA_RectWithoutMargin(&caption_rect_, captionMargin);

  LayoutHighlightCaption();
  SetFWLRect();
  if (GetNormalWidget()) {
    GetNormalWidget()->Update();
  }
}

float CXFA_FFPushButton::GetLineWidth() {
  CXFA_Border* border = node_->GetBorderIfExists();
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
  CXFA_Caption* caption = node_->GetCaptionIfExists();
  if (!caption || caption->IsHidden()) {
    return;
  }

  if (node_->HasButtonRollover()) {
    if (!roll_provider_) {
      roll_provider_ = cppgc::MakeGarbageCollected<CXFA_TextProvider>(
          GetDoc()->GetHeap()->GetAllocationHandle(), node_.Get(),
          CXFA_TextProvider::Type::kRollover);
    }
    rollover_text_layout_ = cppgc::MakeGarbageCollected<CXFA_TextLayout>(
        GetDoc()->GetHeap()->GetAllocationHandle(), GetDoc(), roll_provider_);
  }
  if (node_->HasButtonDown()) {
    if (!down_provider_) {
      down_provider_ = cppgc::MakeGarbageCollected<CXFA_TextProvider>(
          GetDoc()->GetHeap()->GetAllocationHandle(), node_.Get(),
          CXFA_TextProvider::Type::kDown);
    }
    down_text_layout_ = cppgc::MakeGarbageCollected<CXFA_TextLayout>(
        GetDoc()->GetHeap()->GetAllocationHandle(), GetDoc(), down_provider_);
  }
}

void CXFA_FFPushButton::LayoutHighlightCaption() {
  CFX_SizeF sz(caption_rect_.width, caption_rect_.height);
  LayoutCaption();
  if (rollover_text_layout_) {
    rollover_text_layout_->Layout(sz);
  }
  if (down_text_layout_) {
    down_text_layout_->Layout(sz);
  }
}

void CXFA_FFPushButton::RenderHighlightCaption(CFGAS_GEGraphics* pGS,
                                               CFX_Matrix* pMatrix) {
  CXFA_TextLayout* pCapTextLayout = node_->GetCaptionTextLayout();
  CXFA_Caption* caption = node_->GetCaptionIfExists();
  if (!caption || !caption->IsVisible()) {
    return;
  }

  CFX_RenderDevice* pRenderDevice = pGS->GetRenderDevice();
  CFX_RectF rtClip = caption_rect_;
  rtClip.Intersect(GetRectWithoutRotate());
  CFX_Matrix mt(1, 0, 0, 1, caption_rect_.left, caption_rect_.top);
  if (pMatrix) {
    rtClip = pMatrix->TransformRect(rtClip);
    mt.Concat(*pMatrix);
  }

  uint32_t dwState = GetNormalWidget()->GetStates();
  if (down_text_layout_ && (dwState & FWL_STATE_PSB_Pressed) &&
      (dwState & FWL_STATE_PSB_Hovered)) {
    if (down_text_layout_->DrawString(pRenderDevice, mt, rtClip, 0)) {
      return;
    }
  } else if (rollover_text_layout_ && (dwState & FWL_STATE_PSB_Hovered)) {
    if (rollover_text_layout_->DrawString(pRenderDevice, mt, rtClip, 0)) {
      return;
    }
  }

  if (pCapTextLayout) {
    pCapTextLayout->DrawString(pRenderDevice, mt, rtClip, 0);
  }
}

void CXFA_FFPushButton::OnProcessMessage(CFWL_Message* pMessage) {
  old_delegate_->OnProcessMessage(pMessage);
}

void CXFA_FFPushButton::OnProcessEvent(pdfium::CFWL_Event* pEvent) {
  old_delegate_->OnProcessEvent(pEvent);
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
