// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"

#include "core/fxcrt/check.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_ffwidgethandler.h"
#include "xfa/fxfa/parser/cxfa_node.h"

#define CHECK_FWL_VKEY_ENUM____(name)                                 \
  static_assert(                                                      \
      static_cast<int>(name) == static_cast<int>(pdfium::XFA_##name), \
      "FWL_VKEYCODE enum mismatch")

CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Back);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Tab);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NewLine);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Clear);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Return);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Shift);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Control);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Menu);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Pause);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Capital);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Kana);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Hangul);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Junja);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Final);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Hanja);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Kanji);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Escape);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Convert);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NonConvert);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Accept);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_ModeChange);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Space);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Prior);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Next);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_End);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Home);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Left);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Up);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Right);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Down);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Select);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Print);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Execute);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Snapshot);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Insert);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Delete);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Help);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_0);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_1);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_2);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_3);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_4);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_5);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_6);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_7);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_8);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_9);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_A);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_B);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_C);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_D);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_E);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_G);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_H);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_I);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_J);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_K);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_L);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_M);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_N);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_O);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_P);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Q);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_R);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_S);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_T);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_U);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_V);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_W);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_X);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Y);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Z);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_LWin);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Command);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_RWin);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Apps);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Sleep);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NumPad0);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NumPad1);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NumPad2);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NumPad3);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NumPad4);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NumPad5);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NumPad6);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NumPad7);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NumPad8);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NumPad9);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Multiply);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Add);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Separator);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Subtract);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Decimal);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Divide);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F1);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F2);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F3);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F4);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F5);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F6);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F7);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F8);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F9);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F10);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F11);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F12);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F13);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F14);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F15);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F16);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F17);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F18);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F19);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F20);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F21);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F22);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F23);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_F24);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NunLock);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Scroll);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_LShift);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_RShift);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_LControl);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_RControl);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_LMenu);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_RMenu);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_BROWSER_Back);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_BROWSER_Forward);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_BROWSER_Refresh);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_BROWSER_Stop);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_BROWSER_Search);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_BROWSER_Favorites);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_BROWSER_Home);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_VOLUME_Mute);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_VOLUME_Down);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_VOLUME_Up);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_MEDIA_NEXT_Track);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_MEDIA_PREV_Track);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_MEDIA_Stop);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_MEDIA_PLAY_Pause);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_MEDIA_LAUNCH_Mail);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_MEDIA_LAUNCH_MEDIA_Select);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_MEDIA_LAUNCH_APP1);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_MEDIA_LAUNCH_APP2);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_1);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_Plus);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_Comma);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_Minus);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_Period);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_2);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_3);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_4);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_5);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_6);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_7);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_8);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_102);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_ProcessKey);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Packet);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Attn);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Crsel);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Exsel);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Ereof);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Play);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Zoom);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_NoName);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_PA1);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_OEM_Clear);
CHECK_FWL_VKEY_ENUM____(FWL_VKEY_Unknown);

#undef CHECK_FWL_VKEY_ENUM____

namespace {

Mask<XFA_FWL_KeyFlag> GetKeyFlags(Mask<FWL_EVENTFLAG> input) {
  Mask<XFA_FWL_KeyFlag> results;

  if (input & FWL_EVENTFLAG_ControlKey)
    results |= XFA_FWL_KeyFlag::kCtrl;
  if (input & FWL_EVENTFLAG_LeftButtonDown)
    results |= XFA_FWL_KeyFlag::kLButton;
  if (input & FWL_EVENTFLAG_MiddleButtonDown)
    results |= XFA_FWL_KeyFlag::kMButton;
  if (input & FWL_EVENTFLAG_RightButtonDown)
    results |= XFA_FWL_KeyFlag::kRButton;
  if (input & FWL_EVENTFLAG_ShiftKey)
    results |= XFA_FWL_KeyFlag::kShift;
  if (input & FWL_EVENTFLAG_AltKey)
    results |= XFA_FWL_KeyFlag::kAlt;

  return results;
}

}  // namespace

CPDFXFA_Widget::CPDFXFA_Widget(CXFA_FFWidget* pXFAFFWidget,
                               CPDFSDK_PageView* pPageView)
    : CPDFSDK_Annot(pPageView), m_pXFAFFWidget(pXFAFFWidget) {}

CPDFXFA_Widget::~CPDFXFA_Widget() = default;

CPDFXFA_Widget* CPDFXFA_Widget::AsXFAWidget() {
  return this;
}

CPDFSDK_Annot::UnsafeInputHandlers* CPDFXFA_Widget::GetUnsafeInputHandlers() {
  return this;
}

CPDF_Annot::Subtype CPDFXFA_Widget::GetAnnotSubtype() const {
  return CPDF_Annot::Subtype::XFAWIDGET;
}

CFX_FloatRect CPDFXFA_Widget::GetRect() const {
  return GetXFAFFWidget()->GetLayoutItem()->GetAbsoluteRect().ToFloatRect();
}

void CPDFXFA_Widget::OnDraw(CFX_RenderDevice* pDevice,
                            const CFX_Matrix& mtUser2Device,
                            bool bDrawAnnots) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  if (!widget_handler)
    return;

  CFGAS_GEGraphics gs(pDevice);
  bool is_highlight = GetPageView()->GetFormFillEnv()->GetFocusAnnot() != this;
  widget_handler->RenderWidget(GetXFAFFWidget(), &gs, mtUser2Device,
                               is_highlight);

  // to do highlight and shadow
}

bool CPDFXFA_Widget::DoHitTest(const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  if (!widget_handler)
    return false;

  return widget_handler->HitTest(GetXFAFFWidget(), point) !=
         FWL_WidgetHit::Unknown;
}

bool CPDFXFA_Widget::OnChangedFocus() {
  CXFA_FFDocView* doc_view = GetDocView();
  if (!doc_view)
    return false;

  CXFA_FFWidget* widget = GetXFAFFWidget();
  if (doc_view->SetFocus(widget))
    return false;

  return doc_view->GetFocusWidget() != widget;
}

CFX_FloatRect CPDFXFA_Widget::GetViewBBox() {
  CXFA_FFWidget* widget = GetXFAFFWidget();
  CXFA_Node* node = widget->GetNode();
  DCHECK(node->IsWidgetReady());

  CFX_RectF bbox =
      widget->GetBBox(node->GetFFWidgetType() == XFA_FFWidgetType::kSignature
                          ? CXFA_FFWidget::kDrawFocus
                          : CXFA_FFWidget::kDoNotDrawFocus);

  CFX_FloatRect result = bbox.ToFloatRect();
  result.Inflate(1.0f, 1.0f);
  return result;
}

void CPDFXFA_Widget::OnMouseEnter(Mask<FWL_EVENTFLAG> nFlags) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  if (widget_handler)
    widget_handler->OnMouseEnter(GetXFAFFWidget());
}

void CPDFXFA_Widget::OnMouseExit(Mask<FWL_EVENTFLAG> nFlags) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  if (widget_handler)
    widget_handler->OnMouseExit(GetXFAFFWidget());
}

bool CPDFXFA_Widget::OnLButtonDown(Mask<FWL_EVENTFLAG> nFlags,
                                   const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnLButtonDown(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnLButtonUp(Mask<FWL_EVENTFLAG> nFlags,
                                 const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnLButtonUp(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnLButtonDblClk(Mask<FWL_EVENTFLAG> nFlags,
                                     const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnLButtonDblClk(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnMouseMove(Mask<FWL_EVENTFLAG> nFlags,
                                 const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnMouseMove(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnMouseWheel(Mask<FWL_EVENTFLAG> nFlags,
                                  const CFX_PointF& point,
                                  const CFX_Vector& delta) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler &&
         widget_handler->OnMouseWheel(GetXFAFFWidget(), GetKeyFlags(nFlags),
                                      point, delta);
}

bool CPDFXFA_Widget::OnRButtonDown(Mask<FWL_EVENTFLAG> nFlags,
                                   const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnRButtonDown(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnRButtonUp(Mask<FWL_EVENTFLAG> nFlags,
                                 const CFX_PointF& point) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->OnRButtonUp(
                               GetXFAFFWidget(), GetKeyFlags(nFlags), point);
}

bool CPDFXFA_Widget::OnChar(uint32_t nChar, Mask<FWL_EVENTFLAG> nFlags) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler &&
         widget_handler->OnChar(GetXFAFFWidget(), nChar, GetKeyFlags(nFlags));
}

bool CPDFXFA_Widget::OnKeyDown(FWL_VKEYCODE nKeyCode,
                               Mask<FWL_EVENTFLAG> nFlags) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler &&
         widget_handler->OnKeyDown(GetXFAFFWidget(),
                                   static_cast<XFA_FWL_VKEYCODE>(nKeyCode),
                                   GetKeyFlags(nFlags));
}

bool CPDFXFA_Widget::OnSetFocus(Mask<FWL_EVENTFLAG> nFlags) {
  return true;
}

bool CPDFXFA_Widget::OnKillFocus(Mask<FWL_EVENTFLAG> nFlags) {
  CXFA_FFDocView* doc_view = GetDocView();
  if (doc_view)
    doc_view->SetFocus(nullptr);
  return true;
}

bool CPDFXFA_Widget::CanUndo() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->CanUndo(GetXFAFFWidget());
}

bool CPDFXFA_Widget::CanRedo() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->CanRedo(GetXFAFFWidget());
}

bool CPDFXFA_Widget::Undo() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->Undo(GetXFAFFWidget());
}

bool CPDFXFA_Widget::Redo() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->Redo(GetXFAFFWidget());
}

WideString CPDFXFA_Widget::GetText() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  if (!widget_handler)
    return WideString();
  return widget_handler->GetText(GetXFAFFWidget());
}

WideString CPDFXFA_Widget::GetSelectedText() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  if (!widget_handler)
    return WideString();
  return widget_handler->GetSelectedText(GetXFAFFWidget());
}

void CPDFXFA_Widget::ReplaceAndKeepSelection(const WideString& text) {
  // XFA does not seem to support IME input at all. Therefore we don't bother
  // to keep selection for IMEs.
  ReplaceSelection(text);
}

void CPDFXFA_Widget::ReplaceSelection(const WideString& text) {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  if (widget_handler)
    widget_handler->PasteText(GetXFAFFWidget(), text);
}

bool CPDFXFA_Widget::SelectAllText() {
  CXFA_FFWidgetHandler* widget_handler = GetWidgetHandler();
  return widget_handler && widget_handler->SelectAllText(GetXFAFFWidget());
}

bool CPDFXFA_Widget::SetIndexSelected(int index, bool selected) {
  return false;
}

bool CPDFXFA_Widget::IsIndexSelected(int index) {
  return false;
}

CXFA_FFDocView* CPDFXFA_Widget::GetDocView() {
  CXFA_FFPageView* page_view = GetXFAFFWidget()->GetPageView();
  return page_view ? page_view->GetDocView() : nullptr;
}

CXFA_FFWidgetHandler* CPDFXFA_Widget::GetWidgetHandler() {
  CXFA_FFDocView* doc_view = GetDocView();
  return doc_view ? doc_view->GetWidgetHandler() : nullptr;
}
