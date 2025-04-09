// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_list_box.h"

#include <sstream>
#include <utility>

#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxge/cfx_renderdevice.h"
#include "fpdfsdk/pwl/cpwl_edit.h"
#include "fpdfsdk/pwl/cpwl_edit_impl.h"
#include "fpdfsdk/pwl/cpwl_scroll_bar.h"
#include "fpdfsdk/pwl/ipwl_fillernotify.h"
#include "public/fpdf_fwlevent.h"

CPWL_ListBox::CPWL_ListBox(
    const CreateParams& cp,
    std::unique_ptr<IPWL_FillerNotify::PerWindowData> pAttachedData)
    : CPWL_Wnd(cp, std::move(pAttachedData)),
      list_ctrl_(std::make_unique<CPWL_ListCtrl>()) {}

CPWL_ListBox::~CPWL_ListBox() = default;

void CPWL_ListBox::OnCreated() {
  list_ctrl_->SetFontMap(GetFontMap());
  list_ctrl_->SetNotify(this);

  SetHoverSel(HasFlag(PLBS_HOVERSEL));
  list_ctrl_->SetMultipleSel(HasFlag(PLBS_MULTIPLESEL));
  list_ctrl_->SetFontSize(GetCreationParams()->fFontSize);

  hover_sel_ = HasFlag(PLBS_HOVERSEL);
}

void CPWL_ListBox::OnDestroy() {
  // Make sure the notifier is removed from the list as we are about to
  // destroy the notifier and don't want to leave a dangling pointer.
  list_ctrl_->SetNotify(nullptr);
}

void CPWL_ListBox::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                      const CFX_Matrix& mtUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, mtUser2Device);

  CFX_FloatRect rcPlate = list_ctrl_->GetPlateRect();
  CFX_FloatRect rcList = GetListRect();
  CFX_FloatRect rcClient = GetClientRect();

  for (int32_t i = 0, sz = list_ctrl_->GetCount(); i < sz; i++) {
    CFX_FloatRect rcItem = list_ctrl_->GetItemRect(i);
    if (rcItem.bottom > rcPlate.top || rcItem.top < rcPlate.bottom) {
      continue;
    }

    CFX_PointF ptOffset(rcItem.left, (rcItem.top + rcItem.bottom) * 0.5f);
    if (CPWL_EditImpl* pEdit = list_ctrl_->GetItemEdit(i)) {
      CFX_FloatRect rcContent = pEdit->GetContentRect();
      rcItem.Intersect(rcContent.Width() > rcClient.Width() ? rcList
                                                            : rcClient);
    }

    IPWL_FillerNotify* pSysHandler = GetFillerNotify();
    if (list_ctrl_->IsItemSelected(i)) {
      if (pSysHandler->IsSelectionImplemented()) {
        list_ctrl_->GetItemEdit(i)->DrawEdit(
            pDevice, mtUser2Device, GetTextColor().ToFXColor(255), rcList,
            ptOffset, nullptr, pSysHandler, GetAttachedData());
        pSysHandler->OutputSelectedRect(GetAttachedData(), rcItem);
      } else {
        pDevice->DrawFillRect(&mtUser2Device, rcItem,
                              ArgbEncode(255, 0, 51, 113));
        list_ctrl_->GetItemEdit(i)->DrawEdit(
            pDevice, mtUser2Device, ArgbEncode(255, 255, 255, 255), rcList,
            ptOffset, nullptr, pSysHandler, GetAttachedData());
      }
    } else {
      list_ctrl_->GetItemEdit(i)->DrawEdit(
          pDevice, mtUser2Device, GetTextColor().ToFXColor(255), rcList,
          ptOffset, nullptr, pSysHandler, nullptr);
    }
  }
}

bool CPWL_ListBox::OnKeyDown(FWL_VKEYCODE nKeyCode, Mask<FWL_EVENTFLAG> nFlag) {
  CPWL_Wnd::OnKeyDown(nKeyCode, nFlag);

  switch (nKeyCode) {
    default:
      return false;
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_Left:
    case FWL_VKEY_End:
    case FWL_VKEY_Right:
      break;
  }

  switch (nKeyCode) {
    case FWL_VKEY_Up:
      list_ctrl_->OnVK_UP(IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
      break;
    case FWL_VKEY_Down:
      list_ctrl_->OnVK_DOWN(IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
      break;
    case FWL_VKEY_Home:
      list_ctrl_->OnVK_HOME(IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
      break;
    case FWL_VKEY_Left:
      list_ctrl_->OnVK_LEFT(IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
      break;
    case FWL_VKEY_End:
      list_ctrl_->OnVK_END(IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
      break;
    case FWL_VKEY_Right:
      list_ctrl_->OnVK_RIGHT(IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
      break;
    default:
      break;
  }
  OnNotifySelectionChanged(true, nFlag);
  return true;
}

bool CPWL_ListBox::OnChar(uint16_t nChar, Mask<FWL_EVENTFLAG> nFlag) {
  CPWL_Wnd::OnChar(nChar, nFlag);

  if (!list_ctrl_->OnChar(nChar, IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag))) {
    return false;
  }

  OnNotifySelectionChanged(true, nFlag);
  return true;
}

bool CPWL_ListBox::OnLButtonDown(Mask<FWL_EVENTFLAG> nFlag,
                                 const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonDown(nFlag, point);

  if (ClientHitTest(point)) {
    mouse_down_ = true;
    SetFocus();
    SetCapture();

    list_ctrl_->OnMouseDown(point, IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
  }

  return true;
}

bool CPWL_ListBox::OnLButtonUp(Mask<FWL_EVENTFLAG> nFlag,
                               const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonUp(nFlag, point);

  if (mouse_down_) {
    ReleaseCapture();
    mouse_down_ = false;
  }
  OnNotifySelectionChanged(false, nFlag);
  return true;
}

void CPWL_ListBox::SetHoverSel(bool bHoverSel) {
  hover_sel_ = bHoverSel;
}

bool CPWL_ListBox::OnMouseMove(Mask<FWL_EVENTFLAG> nFlag,
                               const CFX_PointF& point) {
  CPWL_Wnd::OnMouseMove(nFlag, point);

  if (hover_sel_ && !IsCaptureMouse() && ClientHitTest(point)) {
    list_ctrl_->Select(list_ctrl_->GetItemIndex(point));
  }
  if (mouse_down_) {
    list_ctrl_->OnMouseMove(point, IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
  }

  return true;
}

void CPWL_ListBox::SetScrollInfo(const PWL_SCROLL_INFO& info) {
  if (CPWL_Wnd* pChild = GetVScrollBar()) {
    pChild->SetScrollInfo(info);
  }
}

void CPWL_ListBox::SetScrollPosition(float pos) {
  if (CPWL_Wnd* pChild = GetVScrollBar()) {
    pChild->SetScrollPosition(pos);
  }
}

void CPWL_ListBox::ScrollWindowVertically(float pos) {
  list_ctrl_->SetScrollPos(CFX_PointF(0, pos));
}

bool CPWL_ListBox::RepositionChildWnd() {
  if (!CPWL_Wnd::RepositionChildWnd()) {
    return false;
  }

  list_ctrl_->SetPlateRect(GetListRect());
  return true;
}

bool CPWL_ListBox::OnNotifySelectionChanged(bool bKeyDown,
                                            Mask<FWL_EVENTFLAG> nFlag) {
  ObservedPtr<CPWL_ListBox> this_observed(this);
  WideString swChange = this_observed->GetText();
  WideString strChangeEx;
  int nSelStart = 0;
  int nSelEnd = pdfium::checked_cast<int>(swChange.GetLength());
  IPWL_FillerNotify::BeforeKeystrokeResult result =
      this_observed->GetFillerNotify()->OnBeforeKeyStroke(
          this_observed->GetAttachedData(), swChange, strChangeEx, nSelStart,
          nSelEnd, bKeyDown, nFlag);
  if (!this_observed) {
    return false;
  }
  return result.exit;
}

CFX_FloatRect CPWL_ListBox::GetFocusRect() const {
  if (list_ctrl_->IsMultipleSel()) {
    CFX_FloatRect rcCaret = list_ctrl_->GetItemRect(list_ctrl_->GetCaret());
    rcCaret.Intersect(GetClientRect());
    return rcCaret;
  }

  return CPWL_Wnd::GetFocusRect();
}

void CPWL_ListBox::AddString(const WideString& str) {
  list_ctrl_->AddString(str);
}

WideString CPWL_ListBox::GetText() {
  return list_ctrl_->GetText();
}

void CPWL_ListBox::SetFontSize(float fFontSize) {
  list_ctrl_->SetFontSize(fFontSize);
}

float CPWL_ListBox::GetFontSize() const {
  return list_ctrl_->GetFontSize();
}

void CPWL_ListBox::OnSetScrollInfoY(float fPlateMin,
                                    float fPlateMax,
                                    float fContentMin,
                                    float fContentMax,
                                    float fSmallStep,
                                    float fBigStep) {
  PWL_SCROLL_INFO Info;
  Info.fPlateWidth = fPlateMax - fPlateMin;
  Info.fContentMin = fContentMin;
  Info.fContentMax = fContentMax;
  Info.fSmallStep = fSmallStep;
  Info.fBigStep = fBigStep;
  SetScrollInfo(Info);

  CPWL_ScrollBar* pScroll = GetVScrollBar();
  if (!pScroll) {
    return;
  }

  if (FXSYS_IsFloatBigger(Info.fPlateWidth,
                          Info.fContentMax - Info.fContentMin) ||
      FXSYS_IsFloatEqual(Info.fPlateWidth,
                         Info.fContentMax - Info.fContentMin)) {
    if (pScroll->IsVisible() && pScroll->SetVisible(false)) {
      RepositionChildWnd();
    }
    return;
  }
  if (!pScroll->IsVisible() && pScroll->SetVisible(true)) {
    RepositionChildWnd();
  }
}

void CPWL_ListBox::OnSetScrollPosY(float fy) {
  SetScrollPosition(fy);
}

bool CPWL_ListBox::OnInvalidateRect(const CFX_FloatRect& rect) {
  return InvalidateRect(&rect);
}

void CPWL_ListBox::Select(int32_t nItemIndex) {
  list_ctrl_->Select(nItemIndex);
}

void CPWL_ListBox::Deselect(int32_t nItemIndex) {
  list_ctrl_->Deselect(nItemIndex);
}

void CPWL_ListBox::SetCaret(int32_t nItemIndex) {
  list_ctrl_->SetCaret(nItemIndex);
}

void CPWL_ListBox::SetTopVisibleIndex(int32_t nItemIndex) {
  list_ctrl_->SetTopItem(nItemIndex);
}

void CPWL_ListBox::ScrollToListItem(int32_t nItemIndex) {
  list_ctrl_->ScrollToListItem(nItemIndex);
}

bool CPWL_ListBox::IsMultipleSel() const {
  return list_ctrl_->IsMultipleSel();
}

int32_t CPWL_ListBox::GetCaretIndex() const {
  return list_ctrl_->GetCaret();
}

int32_t CPWL_ListBox::GetCurSel() const {
  return list_ctrl_->GetSelect();
}

bool CPWL_ListBox::IsItemSelected(int32_t nItemIndex) const {
  return list_ctrl_->IsItemSelected(nItemIndex);
}

int32_t CPWL_ListBox::GetTopVisibleIndex() const {
  list_ctrl_->ScrollToListItem(list_ctrl_->GetFirstSelected());
  return list_ctrl_->GetTopItem();
}

int32_t CPWL_ListBox::GetCount() const {
  return list_ctrl_->GetCount();
}

CFX_FloatRect CPWL_ListBox::GetContentRect() const {
  return list_ctrl_->GetContentRect();
}

float CPWL_ListBox::GetFirstHeight() const {
  return list_ctrl_->GetFirstHeight();
}

CFX_FloatRect CPWL_ListBox::GetListRect() const {
  float width = static_cast<float>(GetBorderWidth() + GetInnerBorderWidth());
  return GetWindowRect().GetDeflated(width, width);
}

bool CPWL_ListBox::OnMouseWheel(Mask<FWL_EVENTFLAG> nFlag,
                                const CFX_PointF& point,
                                const CFX_Vector& delta) {
  if (delta.y < 0) {
    list_ctrl_->OnVK_DOWN(IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
  } else {
    list_ctrl_->OnVK_UP(IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
  }

  OnNotifySelectionChanged(false, nFlag);
  return true;
}
