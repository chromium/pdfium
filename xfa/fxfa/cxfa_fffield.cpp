// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_fffield.h"

#include <algorithm>
#include <utility>

#include "constants/ascii.h"
#include "core/fxcrt/check.h"
#include "xfa/fgas/graphics/cfgas_gecolor.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_eventmouse.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagekillfocus.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_messagemousewheel.h"
#include "xfa/fwl/cfwl_messagesetfocus.h"
#include "xfa/fwl/cfwl_picturebox.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fwl/fwl_widgetdef.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fwltheme.h"
#include "xfa/fxfa/cxfa_textlayout.h"
#include "xfa/fxfa/parser/cxfa_border.h"
#include "xfa/fxfa/parser/cxfa_calculate.h"
#include "xfa/fxfa/parser/cxfa_caption.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_script.h"

namespace {

constexpr float kMinUIHeight = 4.32f;
constexpr float kDefaultUIHeight = 2.0f;

}  // namespace

CXFA_FFField::CXFA_FFField(CXFA_Node* pNode) : CXFA_FFWidget(pNode) {}

CXFA_FFField::~CXFA_FFField() = default;

CXFA_FFDropDown* CXFA_FFField::AsDropDown() {
  return nullptr;
}

CXFA_FFField* CXFA_FFField::AsField() {
  return this;
}

void CXFA_FFField::Trace(cppgc::Visitor* visitor) const {
  CXFA_FFWidget::Trace(visitor);
  visitor->Trace(normal_widget_);
}

CFX_RectF CXFA_FFField::GetBBox(FocusOption focus) {
  if (focus == kDoNotDrawFocus)
    return CXFA_FFWidget::GetBBox(kDoNotDrawFocus);

  switch (node_->GetFFWidgetType()) {
    case XFA_FFWidgetType::kButton:
    case XFA_FFWidgetType::kCheckButton:
    case XFA_FFWidgetType::kImageEdit:
    case XFA_FFWidgetType::kSignature:
    case XFA_FFWidgetType::kChoiceList:
      return GetRotateMatrix().TransformRect(uirect_);
    default:
      return CFX_RectF();
  }
}

void CXFA_FFField::RenderWidget(CFGAS_GEGraphics* pGS,
                                const CFX_Matrix& matrix,
                                HighlightOption highlight) {
  if (!HasVisibleStatus())
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, highlight);
  DrawBorder(pGS, node_->GetUIBorder(), uirect_, mtRotate);
  RenderCaption(pGS, mtRotate);
  DrawHighlight(pGS, mtRotate, highlight, kSquareShape);

  CFX_RectF rtWidget = GetNormalWidget()->GetWidgetRect();
  CFX_Matrix mt(1, 0, 0, 1, rtWidget.left, rtWidget.top);
  mt.Concat(mtRotate);
  GetApp()->GetFWLWidgetMgr()->OnDrawWidget(GetNormalWidget(), pGS, mt);
}

void CXFA_FFField::DrawHighlight(CFGAS_GEGraphics* pGS,
                                 const CFX_Matrix& pMatrix,
                                 HighlightOption highlight,
                                 ShapeOption shape) {
  if (highlight == kNoHighlight)
    return;

  if (uirect_.IsEmpty() || !GetDoc()->GetXFADoc()->IsInteractive() ||
      !node_->IsOpenAccess()) {
    return;
  }
  pGS->SetFillColor(CFGAS_GEColor(GetDoc()->GetHighlightColor()));
  CFGAS_GEPath path;
  if (shape == kRoundShape)
    path.AddEllipse(uirect_);
  else
    path.AddRectangle(uirect_.left, uirect_.top, uirect_.width, uirect_.height);

  pGS->FillPath(path, CFX_FillRenderOptions::FillType::kWinding, pMatrix);
}

CFWL_Widget* CXFA_FFField::GetNormalWidget() {
  return normal_widget_;
}

const CFWL_Widget* CXFA_FFField::GetNormalWidget() const {
  return normal_widget_;
}

void CXFA_FFField::SetNormalWidget(CFWL_Widget* widget) {
  normal_widget_ = widget;
}

bool CXFA_FFField::IsLoaded() {
  return GetNormalWidget() && CXFA_FFWidget::IsLoaded();
}

bool CXFA_FFField::LoadWidget() {
  node_->LoadCaption(GetDoc());
  PerformLayout();
  return true;
}

void CXFA_FFField::SetEditScrollOffset() {
  XFA_FFWidgetType eType = node_->GetFFWidgetType();
  if (eType != XFA_FFWidgetType::kTextEdit &&
      eType != XFA_FFWidgetType::kNumericEdit &&
      eType != XFA_FFWidgetType::kPasswordEdit) {
    return;
  }

  float fScrollOffset = 0;
  CXFA_ContentLayoutItem* pItem = GetLayoutItem()->GetPrev();
  CXFA_FFField* pPrev = pItem ? ToField(pItem->GetFFWidget()) : nullptr;
  if (pPrev)
    fScrollOffset = -(node_->GetUIMargin().top);

  while (pPrev) {
    fScrollOffset += pPrev->uirect_.height;
    pItem = pPrev->GetLayoutItem()->GetPrev();
    pPrev = pItem ? ToField(pItem->GetFFWidget()) : nullptr;
  }
  static_cast<CFWL_Edit*>(GetNormalWidget())->SetScrollOffset(fScrollOffset);
}

void CXFA_FFField::PerformLayout() {
  CXFA_FFWidget::PerformLayout();
  CapPlacement();
  LayoutCaption();
  SetFWLRect();
  SetEditScrollOffset();
  if (GetNormalWidget()) {
    GetNormalWidget()->Update();
  }
}

void CXFA_FFField::CapPlacement() {
  CFX_RectF rtWidget = GetRectWithoutRotate();
  CXFA_Margin* margin = node_->GetMarginIfExists();
  if (margin) {
    CXFA_ContentLayoutItem* pItem = GetLayoutItem();
    float fLeftInset = margin->GetLeftInset();
    float fRightInset = margin->GetRightInset();
    float fTopInset = margin->GetTopInset();
    float fBottomInset = margin->GetBottomInset();
    if (!pItem->GetPrev() && !pItem->GetNext()) {
      rtWidget.Deflate(fLeftInset, fTopInset, fRightInset, fBottomInset);
    } else {
      if (!pItem->GetPrev())
        rtWidget.Deflate(fLeftInset, fTopInset, fRightInset, 0);
      else if (!pItem->GetNext())
        rtWidget.Deflate(fLeftInset, 0, fRightInset, fBottomInset);
      else
        rtWidget.Deflate(fLeftInset, 0, fRightInset, 0);
    }
  }

  XFA_AttributeValue iCapPlacement = XFA_AttributeValue::Unknown;
  float fCapReserve = 0;
  CXFA_Caption* caption = node_->GetCaptionIfExists();
  if (caption && !caption->IsHidden()) {
    iCapPlacement = caption->GetPlacementType();
    if ((iCapPlacement == XFA_AttributeValue::Top &&
         GetLayoutItem()->GetPrev()) ||
        (iCapPlacement == XFA_AttributeValue::Bottom &&
         GetLayoutItem()->GetNext())) {
      caption_rect_ = CFX_RectF();
    } else {
      fCapReserve = caption->GetReserve();
      if (iCapPlacement == XFA_AttributeValue::Top ||
          iCapPlacement == XFA_AttributeValue::Bottom) {
        fCapReserve = std::min(fCapReserve, rtWidget.height);
      } else {
        fCapReserve = std::min(fCapReserve, rtWidget.width);
      }
      CXFA_ContentLayoutItem* pItem = GetLayoutItem();
      if (!pItem->GetPrev() && !pItem->GetNext()) {
        caption_rect_ = rtWidget;
      } else {
        pItem = pItem->GetFirst();
        caption_rect_ = pItem->GetAbsoluteRect();
        pItem = pItem->GetNext();
        while (pItem) {
          caption_rect_.height += pItem->GetAbsoluteRect().Height();
          pItem = pItem->GetNext();
        }
        XFA_RectWithoutMargin(&caption_rect_, margin);
      }

      CXFA_TextLayout* pCapTextLayout = node_->GetCaptionTextLayout();
      if (fCapReserve <= 0 && pCapTextLayout) {
        CFX_SizeF minSize;
        CFX_SizeF maxSize;
        CFX_SizeF size = pCapTextLayout->CalcSize(minSize, maxSize);
        if (iCapPlacement == XFA_AttributeValue::Top ||
            iCapPlacement == XFA_AttributeValue::Bottom) {
          fCapReserve = size.height;
        } else {
          fCapReserve = size.width;
        }
      }
    }
  }

  uirect_ = rtWidget;
  CXFA_Margin* capMargin = caption ? caption->GetMarginIfExists() : nullptr;
  switch (iCapPlacement) {
    case XFA_AttributeValue::Left: {
      caption_rect_.width = fCapReserve;
      CapLeftRightPlacement(capMargin, rtWidget, iCapPlacement);
      uirect_.width -= fCapReserve;
      uirect_.left += fCapReserve;
      break;
    }
    case XFA_AttributeValue::Top: {
      caption_rect_.height = fCapReserve;
      CapTopBottomPlacement(capMargin, rtWidget, iCapPlacement);
      uirect_.top += fCapReserve;
      uirect_.height -= fCapReserve;
      break;
    }
    case XFA_AttributeValue::Right: {
      caption_rect_.left = caption_rect_.right() - fCapReserve;
      caption_rect_.width = fCapReserve;
      CapLeftRightPlacement(capMargin, rtWidget, iCapPlacement);
      uirect_.width -= fCapReserve;
      break;
    }
    case XFA_AttributeValue::Bottom: {
      caption_rect_.top = caption_rect_.bottom() - fCapReserve;
      caption_rect_.height = fCapReserve;
      CapTopBottomPlacement(capMargin, rtWidget, iCapPlacement);
      uirect_.height -= fCapReserve;
      break;
    }
    case XFA_AttributeValue::Inline:
      break;
    default:
      break;
  }

  CXFA_Border* borderUI = node_->GetUIBorder();
  if (borderUI) {
    CXFA_Margin* borderMargin = borderUI->GetMarginIfExists();
    XFA_RectWithoutMargin(&uirect_, borderMargin);
  }
  uirect_.Normalize();
}

void CXFA_FFField::CapTopBottomPlacement(const CXFA_Margin* margin,
                                         const CFX_RectF& rtWidget,
                                         XFA_AttributeValue iCapPlacement) {
  CFX_RectF rtUIMargin = node_->GetUIMargin();
  caption_rect_.left += rtUIMargin.left;
  if (margin) {
    XFA_RectWithoutMargin(&caption_rect_, margin);
    if (caption_rect_.height < 0) {
      caption_rect_.top += caption_rect_.height;
    }
  }

  float fWidth = rtUIMargin.left + rtUIMargin.width;
  float fHeight = caption_rect_.height + rtUIMargin.top + rtUIMargin.height;
  if (fWidth > rtWidget.width)
    uirect_.width += fWidth - rtWidget.width;

  if (fHeight == kDefaultUIHeight && uirect_.height < kMinUIHeight) {
    uirect_.height = kMinUIHeight;
    caption_rect_.top += rtUIMargin.top + rtUIMargin.height;
  } else if (fHeight > rtWidget.height) {
    uirect_.height += fHeight - rtWidget.height;
    if (iCapPlacement == XFA_AttributeValue::Bottom)
      caption_rect_.top += fHeight - rtWidget.height;
  }
}

void CXFA_FFField::CapLeftRightPlacement(const CXFA_Margin* margin,
                                         const CFX_RectF& rtWidget,
                                         XFA_AttributeValue iCapPlacement) {
  CFX_RectF rtUIMargin = node_->GetUIMargin();
  caption_rect_.top += rtUIMargin.top;
  caption_rect_.height -= rtUIMargin.top;
  if (margin) {
    XFA_RectWithoutMargin(&caption_rect_, margin);
    if (caption_rect_.height < 0) {
      caption_rect_.top += caption_rect_.height;
    }
  }

  float fWidth = caption_rect_.width + rtUIMargin.left + rtUIMargin.width;
  float fHeight = rtUIMargin.top + rtUIMargin.height;
  if (fWidth > rtWidget.width) {
    uirect_.width += fWidth - rtWidget.width;
    if (iCapPlacement == XFA_AttributeValue::Right)
      caption_rect_.left += fWidth - rtWidget.width;
  }

  if (fHeight == kDefaultUIHeight && uirect_.height < kMinUIHeight) {
    uirect_.height = kMinUIHeight;
    caption_rect_.top += rtUIMargin.top + rtUIMargin.height;
  } else if (fHeight > rtWidget.height) {
    uirect_.height += fHeight - rtWidget.height;
  }
}

void CXFA_FFField::UpdateFWL() {
  if (GetNormalWidget())
    GetNormalWidget()->Update();
}

uint32_t CXFA_FFField::UpdateUIProperty() {
  CXFA_Node* pUiNode = node_->GetUIChildNode();
  if (pUiNode && pUiNode->GetElementType() == XFA_Element::DefaultUi)
    return FWL_STYLEEXT_EDT_ReadOnly;
  return 0;
}

void CXFA_FFField::SetFWLRect() {
  if (!GetNormalWidget())
    return;

  CFX_RectF rtUi = uirect_;
  rtUi.width = std::max(rtUi.width, 1.0f);
  if (!GetDoc()->GetXFADoc()->IsInteractive()) {
    float fFontSize = node_->GetFontSize();
    rtUi.height = std::max(rtUi.height, fFontSize);
  }
  GetNormalWidget()->SetWidgetRect(rtUi);
}

bool CXFA_FFField::OnMouseEnter() {
  if (!GetNormalWidget())
    return false;

  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kEnter,
                        Mask<XFA_FWL_KeyFlag>(), CFX_PointF());
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFField::OnMouseExit() {
  if (!GetNormalWidget())
    return false;

  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kLeave,
                        Mask<XFA_FWL_KeyFlag>(), CFX_PointF());
  SendMessageToFWLWidget(&msg);
  return true;
}

CFX_PointF CXFA_FFField::FWLToClient(const CFX_PointF& point) {
  return GetNormalWidget()
             ? point - GetNormalWidget()->GetWidgetRect().TopLeft()
             : point;
}

bool CXFA_FFField::AcceptsFocusOnButtonDown(
    Mask<XFA_FWL_KeyFlag> dwFlags,
    const CFX_PointF& point,
    CFWL_MessageMouse::MouseCommand command) {
  if (!GetNormalWidget())
    return false;
  if (!node_->IsOpenAccess() || !GetDoc()->GetXFADoc()->IsInteractive()) {
    return false;
  }
  if (!PtInActiveRect(point))
    return false;

  return true;
}

bool CXFA_FFField::OnLButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                                 const CFX_PointF& point) {
  SetButtonDown(true);
  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kLeftButtonDown,
                        dwFlags, FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFField::OnLButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                               const CFX_PointF& point) {
  if (!GetNormalWidget())
    return false;
  if (!IsButtonDown())
    return false;

  SetButtonDown(false);

  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kLeftButtonUp, dwFlags,
                        FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFField::OnLButtonDblClk(Mask<XFA_FWL_KeyFlag> dwFlags,
                                   const CFX_PointF& point) {
  if (!GetNormalWidget())
    return false;

  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kLeftButtonDblClk,
                        dwFlags, FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFField::OnMouseMove(Mask<XFA_FWL_KeyFlag> dwFlags,
                               const CFX_PointF& point) {
  if (!GetNormalWidget())
    return false;

  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kMove, dwFlags,
                        FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFField::OnMouseWheel(Mask<XFA_FWL_KeyFlag> dwFlags,
                                const CFX_PointF& point,
                                const CFX_Vector& delta) {
  if (!GetNormalWidget())
    return false;

  CFWL_MessageMouseWheel msg(GetNormalWidget(), FWLToClient(point), delta);
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFField::OnRButtonDown(Mask<XFA_FWL_KeyFlag> dwFlags,
                                 const CFX_PointF& point) {
  SetButtonDown(true);

  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kRightButtonDown,
                        dwFlags, FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFField::OnRButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                               const CFX_PointF& point) {
  if (!GetNormalWidget())
    return false;
  if (!IsButtonDown())
    return false;

  SetButtonDown(false);
  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kRightButtonUp,
                        dwFlags, FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFField::OnRButtonDblClk(Mask<XFA_FWL_KeyFlag> dwFlags,
                                   const CFX_PointF& point) {
  if (!GetNormalWidget())
    return false;

  CFWL_MessageMouse msg(GetNormalWidget(),
                        CFWL_MessageMouse::MouseCommand::kRightButtonDblClk,
                        dwFlags, FWLToClient(point));
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFField::OnSetFocus(CXFA_FFWidget* pOldWidget) {
  if (!CXFA_FFWidget::OnSetFocus(pOldWidget))
    return false;

  if (!GetNormalWidget())
    return false;

  CFWL_MessageSetFocus msg(GetNormalWidget());
  SendMessageToFWLWidget(&msg);
  GetLayoutItem()->SetStatusBits(XFA_WidgetStatus::kFocused);
  InvalidateRect();

  return true;
}

bool CXFA_FFField::OnKillFocus(CXFA_FFWidget* pNewWidget) {
  if (GetNormalWidget()) {
    CFWL_MessageKillFocus msg(GetNormalWidget());
    SendMessageToFWLWidget(&msg);
    GetLayoutItem()->ClearStatusBits(XFA_WidgetStatus::kFocused);
    InvalidateRect();
  }
  return pNewWidget && CXFA_FFWidget::OnKillFocus(pNewWidget);
}

bool CXFA_FFField::OnKeyDown(XFA_FWL_VKEYCODE dwKeyCode,
                             Mask<XFA_FWL_KeyFlag> dwFlags) {
  if (!GetNormalWidget() || !GetDoc()->GetXFADoc()->IsInteractive())
    return false;

  CFWL_MessageKey msg(GetNormalWidget(), CFWL_MessageKey::KeyCommand::kKeyDown,
                      dwFlags, dwKeyCode);
  SendMessageToFWLWidget(&msg);
  return true;
}

bool CXFA_FFField::OnChar(uint32_t dwChar, Mask<XFA_FWL_KeyFlag> dwFlags) {
  if (!GetDoc()->GetXFADoc()->IsInteractive())
    return false;
  if (dwChar == pdfium::ascii::kTab)
    return true;
  if (!GetNormalWidget())
    return false;
  if (!node_->IsOpenAccess()) {
    return false;
  }

  CFWL_MessageKey msg(GetNormalWidget(), CFWL_MessageKey::KeyCommand::kChar,
                      dwFlags, dwChar);
  SendMessageToFWLWidget(&msg);
  return true;
}

FWL_WidgetHit CXFA_FFField::HitTest(const CFX_PointF& point) {
  auto* pNorm = GetNormalWidget();
  if (pNorm && pNorm->HitTest(FWLToClient(point)) != FWL_WidgetHit::Unknown)
    return FWL_WidgetHit::Client;
  if (!GetRectWithoutRotate().Contains(point))
    return FWL_WidgetHit::Unknown;
  if (caption_rect_.Contains(point)) {
    return FWL_WidgetHit::Titlebar;
  }
  return FWL_WidgetHit::Border;
}

bool CXFA_FFField::PtInActiveRect(const CFX_PointF& point) {
  return GetNormalWidget() &&
         GetNormalWidget()->GetWidgetRect().Contains(point);
}

void CXFA_FFField::LayoutCaption() {
  CXFA_TextLayout* pCapTextLayout = node_->GetCaptionTextLayout();
  if (!pCapTextLayout)
    return;

  float fHeight = pCapTextLayout->Layout(caption_rect_.Size());
  caption_rect_.height = std::max(caption_rect_.height, fHeight);
}

void CXFA_FFField::RenderCaption(CFGAS_GEGraphics* pGS,
                                 const CFX_Matrix& pMatrix) {
  CXFA_TextLayout* pCapTextLayout = node_->GetCaptionTextLayout();
  if (!pCapTextLayout)
    return;

  CXFA_Caption* caption = node_->GetCaptionIfExists();
  if (!caption || !caption->IsVisible())
    return;

  if (!pCapTextLayout->IsLoaded())
    pCapTextLayout->Layout(caption_rect_.Size());

  CFX_RectF rtClip = caption_rect_;
  rtClip.Intersect(GetRectWithoutRotate());
  CFX_RenderDevice* pRenderDevice = pGS->GetRenderDevice();
  CFX_Matrix mt(1, 0, 0, 1, caption_rect_.left, caption_rect_.top);
  rtClip = pMatrix.TransformRect(rtClip);
  mt.Concat(pMatrix);
  pCapTextLayout->DrawString(pRenderDevice, mt, rtClip, 0);
}

bool CXFA_FFField::ProcessCommittedData() {
  if (!node_->IsOpenAccess()) {
    return false;
  }
  if (!IsDataChanged())
    return false;

  doc_view_->SetChangeMark();
  doc_view_->AddValidateNode(node_.Get());

  if (CalculateOverride() != 1)
    return false;
  return CommitData();
}

int32_t CXFA_FFField::CalculateOverride() {
  CXFA_Node* exclNode = node_->GetExclGroupIfExists();
  if (!exclNode || !exclNode->IsWidgetReady())
    return CalculateNode(node_.Get());
  if (CalculateNode(exclNode) == 0)
    return 0;

  CXFA_Node* pNode = exclNode->GetExclGroupFirstMember();
  if (!pNode)
    return 1;

  while (pNode) {
    if (!pNode->IsWidgetReady())
      return 1;
    if (CalculateNode(pNode) == 0)
      return 0;

    pNode = pNode->GetExclGroupNextMember(pNode);
  }
  return 1;
}

int32_t CXFA_FFField::CalculateNode(CXFA_Node* pNode) {
  CXFA_Calculate* calc = pNode->GetCalculateIfExists();
  if (!calc)
    return 1;

  XFA_VERSION version = GetDoc()->GetXFADoc()->GetCurVersionMode();
  switch (calc->GetOverride()) {
    case XFA_AttributeValue::Error: {
      if (version <= XFA_VERSION_204)
        return 1;

      CXFA_FFApp::CallbackIface* pAppProvider = GetAppProvider();
      if (pAppProvider) {
        pAppProvider->MsgBox(
            WideString::FromASCII("You are not allowed to modify this field."),
            WideString::FromASCII("Calculate Override"),
            static_cast<uint32_t>(AlertIcon::kWarning),
            static_cast<uint32_t>(AlertButton::kOK));
      }
      return 0;
    }
    case XFA_AttributeValue::Warning: {
      if (version <= XFA_VERSION_204) {
        CXFA_Script* script = calc->GetScriptIfExists();
        if (!script || script->GetExpression().IsEmpty())
          return 1;
      }

      if (pNode->IsUserInteractive())
        return 1;

      CXFA_FFApp::CallbackIface* pAppProvider = GetAppProvider();
      if (!pAppProvider)
        return 0;

      WideString wsMessage = calc->GetMessageText();
      if (!wsMessage.IsEmpty())
        wsMessage += L"\r\n";
      wsMessage +=
          WideString::FromASCII("Are you sure you want to modify this field?");

      if (pAppProvider->MsgBox(wsMessage,
                               WideString::FromASCII("Calculate Override"),
                               static_cast<uint32_t>(AlertIcon::kWarning),
                               static_cast<uint32_t>(AlertButton::kYesNo)) ==
          static_cast<uint32_t>(AlertReturn::kYes)) {
        pNode->SetFlag(XFA_NodeFlag::kUserInteractive);
        return 1;
      }
      return 0;
    }
    case XFA_AttributeValue::Ignore:
      return 0;
    case XFA_AttributeValue::Disabled:
      pNode->SetFlag(XFA_NodeFlag::kUserInteractive);
      return 1;
    default:
      return 1;
  }
}

bool CXFA_FFField::CommitData() {
  return false;
}

bool CXFA_FFField::IsDataChanged() {
  return false;
}

void CXFA_FFField::SendMessageToFWLWidget(CFWL_Message* pMessage) {
  DCHECK(pMessage);
  GetApp()->GetFWLWidgetMgr()->OnProcessMessageToForm(pMessage);
}

void CXFA_FFField::OnProcessMessage(CFWL_Message* pMessage) {}

void CXFA_FFField::OnProcessEvent(CFWL_Event* pEvent) {
  switch (pEvent->GetType()) {
    case CFWL_Event::Type::Mouse: {
      CFWL_EventMouse* event = static_cast<CFWL_EventMouse*>(pEvent);
      CFWL_MessageMouse::MouseCommand cmd = event->GetCommand();
      if (cmd == CFWL_MessageMouse::MouseCommand::kEnter) {
        CXFA_EventParam eParam(XFA_EVENT_MouseEnter);
        node_->ProcessEvent(GetDocView(), XFA_AttributeValue::MouseEnter,
                            &eParam);
      } else if (cmd == CFWL_MessageMouse::MouseCommand::kLeave) {
        CXFA_EventParam eParam(XFA_EVENT_MouseExit);
        node_->ProcessEvent(GetDocView(), XFA_AttributeValue::MouseExit,
                            &eParam);
      } else if (cmd == CFWL_MessageMouse::MouseCommand::kLeftButtonDown) {
        CXFA_EventParam eParam(XFA_EVENT_MouseDown);
        node_->ProcessEvent(GetDocView(), XFA_AttributeValue::MouseDown,
                            &eParam);
      } else if (cmd == CFWL_MessageMouse::MouseCommand::kLeftButtonUp) {
        CXFA_EventParam eParam(XFA_EVENT_MouseUp);
        node_->ProcessEvent(GetDocView(), XFA_AttributeValue::MouseUp, &eParam);
      }
      break;
    }
    case CFWL_Event::Type::Click: {
      CXFA_EventParam eParam(XFA_EVENT_Click);
      node_->ProcessEvent(GetDocView(), XFA_AttributeValue::Click, &eParam);
      break;
    }
    default:
      break;
  }
}

void CXFA_FFField::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                const CFX_Matrix& matrix) {}
