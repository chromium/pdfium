// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_combo_box.h"

#include <algorithm>
#include <utility>

#include "constants/ascii.h"
#include "fpdfsdk/pwl/cpwl_cbbutton.h"
#include "fpdfsdk/pwl/cpwl_cblistbox.h"
#include "fpdfsdk/pwl/cpwl_edit.h"
#include "fpdfsdk/pwl/ipwl_fillernotify.h"
#include "public/fpdf_fwlevent.h"

namespace {

constexpr float kComboBoxDefaultFontSize = 12.0f;
constexpr int kDefaultButtonWidth = 13;

}  // namespace

CPWL_ComboBox::CPWL_ComboBox(
    const CreateParams& cp,
    std::unique_ptr<IPWL_FillerNotify::PerWindowData> pAttachedData)
    : CPWL_Wnd(cp, std::move(pAttachedData)) {
  GetCreationParams()->dwFlags &= ~PWS_VSCROLL;
}

CPWL_ComboBox::~CPWL_ComboBox() = default;

void CPWL_ComboBox::OnDestroy() {
  // Until cleanup takes place in the virtual destructor for CPWL_Wnd
  // subclasses, implement the virtual OnDestroy method that does the
  // cleanup first, then invokes the superclass OnDestroy ... gee,
  // like a dtor would.
  list_.ExtractAsDangling();
  button_.ExtractAsDangling();
  edit_.ExtractAsDangling();
  CPWL_Wnd::OnDestroy();
}

void CPWL_ComboBox::SetFocus() {
  if (edit_) {
    edit_->SetFocus();
  }
}

void CPWL_ComboBox::KillFocus() {
  if (!SetPopup(false)) {
    return;
  }

  CPWL_Wnd::KillFocus();
}

WideString CPWL_ComboBox::GetSelectedText() {
  if (edit_) {
    return edit_->GetSelectedText();
  }

  return WideString();
}

void CPWL_ComboBox::ReplaceAndKeepSelection(const WideString& text) {
  if (edit_) {
    edit_->ReplaceAndKeepSelection(text);
  }
}

void CPWL_ComboBox::ReplaceSelection(const WideString& text) {
  if (edit_) {
    edit_->ReplaceSelection(text);
  }
}

bool CPWL_ComboBox::SelectAllText() {
  return edit_ && edit_->SelectAllText();
}

bool CPWL_ComboBox::CanUndo() {
  return edit_ && edit_->CanUndo();
}

bool CPWL_ComboBox::CanRedo() {
  return edit_ && edit_->CanRedo();
}

bool CPWL_ComboBox::Undo() {
  return edit_ && edit_->Undo();
}

bool CPWL_ComboBox::Redo() {
  return edit_ && edit_->Redo();
}

WideString CPWL_ComboBox::GetText() {
  return edit_ ? edit_->GetText() : WideString();
}

void CPWL_ComboBox::SetText(const WideString& text) {
  if (edit_) {
    edit_->SetText(text);
  }
}

void CPWL_ComboBox::AddString(const WideString& str) {
  if (list_) {
    list_->AddString(str);
  }
}

int32_t CPWL_ComboBox::GetSelect() const {
  return select_item_;
}

void CPWL_ComboBox::SetSelect(int32_t nItemIndex) {
  if (list_) {
    list_->Select(nItemIndex);
  }

  edit_->SetText(list_->GetText());
  select_item_ = nItemIndex;
}

void CPWL_ComboBox::SetEditSelection(int32_t nStartChar, int32_t nEndChar) {
  if (edit_) {
    edit_->SetSelection(nStartChar, nEndChar);
  }
}

void CPWL_ComboBox::ClearSelection() {
  if (edit_) {
    edit_->ClearSelection();
  }
}

void CPWL_ComboBox::CreateChildWnd(const CreateParams& cp) {
  CreateEdit(cp);
  CreateButton(cp);
  CreateListBox(cp);
}

void CPWL_ComboBox::CreateEdit(const CreateParams& cp) {
  if (edit_) {
    return;
  }

  CreateParams ecp = cp;
  ecp.dwFlags =
      PWS_VISIBLE | PWS_BORDER | PES_CENTER | PES_AUTOSCROLL | PES_UNDO;

  if (HasFlag(PWS_AUTOFONTSIZE)) {
    ecp.dwFlags |= PWS_AUTOFONTSIZE;
  }

  if (!HasFlag(PCBS_ALLOWCUSTOMTEXT)) {
    ecp.dwFlags |= PWS_READONLY;
  }

  ecp.rcRectWnd = CFX_FloatRect();
  ecp.dwBorderWidth = 0;
  ecp.nBorderStyle = BorderStyle::kSolid;

  auto pEdit = std::make_unique<CPWL_Edit>(ecp, CloneAttachedData());
  edit_ = pEdit.get();
  AddChild(std::move(pEdit));
  edit_->Realize();
}

void CPWL_ComboBox::CreateButton(const CreateParams& cp) {
  if (button_) {
    return;
  }

  CreateParams bcp = cp;
  bcp.dwFlags = PWS_VISIBLE | PWS_BORDER | PWS_BACKGROUND;
  bcp.sBackgroundColor = CFX_Color(CFX_Color::Type::kRGB, 220.0f / 255.0f,
                                   220.0f / 255.0f, 220.0f / 255.0f);
  bcp.sBorderColor = kDefaultBlackColor;
  bcp.dwBorderWidth = 2;
  bcp.nBorderStyle = BorderStyle::kBeveled;
  bcp.eCursorType = IPWL_FillerNotify::CursorStyle::kArrow;

  auto pButton = std::make_unique<CPWL_CBButton>(bcp, CloneAttachedData());
  button_ = pButton.get();
  AddChild(std::move(pButton));
  button_->Realize();
}

void CPWL_ComboBox::CreateListBox(const CreateParams& cp) {
  if (list_) {
    return;
  }

  CreateParams lcp = cp;
  lcp.dwFlags = PWS_BORDER | PWS_BACKGROUND | PLBS_HOVERSEL | PWS_VSCROLL;
  lcp.nBorderStyle = BorderStyle::kSolid;
  lcp.dwBorderWidth = 1;
  lcp.eCursorType = IPWL_FillerNotify::CursorStyle::kArrow;
  lcp.rcRectWnd = CFX_FloatRect();
  lcp.fFontSize =
      (cp.dwFlags & PWS_AUTOFONTSIZE) ? kComboBoxDefaultFontSize : cp.fFontSize;

  if (cp.sBorderColor.nColorType == CFX_Color::Type::kTransparent) {
    lcp.sBorderColor = kDefaultBlackColor;
  }

  if (cp.sBackgroundColor.nColorType == CFX_Color::Type::kTransparent) {
    lcp.sBackgroundColor = kDefaultWhiteColor;
  }

  auto pList = std::make_unique<CPWL_CBListBox>(lcp, CloneAttachedData());
  list_ = pList.get();
  AddChild(std::move(pList));
  list_->Realize();
}

bool CPWL_ComboBox::RepositionChildWnd() {
  ObservedPtr<CPWL_ComboBox> this_observed(this);
  const CFX_FloatRect rcClient = this_observed->GetClientRect();
  if (this_observed->is_popup_) {
    const float fOldWindowHeight = this_observed->old_window_rect_.Height();
    const float fOldClientHeight = fOldWindowHeight - GetBorderWidth() * 2;
    CFX_FloatRect rcList = CPWL_Wnd::GetWindowRect();
    CFX_FloatRect rcButton = rcClient;
    rcButton.left =
        std::max(rcButton.right - kDefaultButtonWidth, rcClient.left);
    CFX_FloatRect rcEdit = rcClient;
    rcEdit.right = std::max(rcButton.left - 1.0f, rcEdit.left);
    if (this_observed->is_bottom_) {
      rcButton.bottom = rcButton.top - fOldClientHeight;
      rcEdit.bottom = rcEdit.top - fOldClientHeight;
      rcList.top -= fOldWindowHeight;
    } else {
      rcButton.top = rcButton.bottom + fOldClientHeight;
      rcEdit.top = rcEdit.bottom + fOldClientHeight;
      rcList.bottom += fOldWindowHeight;
    }
    if (this_observed->button_) {
      this_observed->button_->Move(rcButton, true, false);
      if (!this_observed) {
        return false;
      }
    }
    if (this_observed->edit_) {
      this_observed->edit_->Move(rcEdit, true, false);
      if (!this_observed) {
        return false;
      }
    }
    if (this_observed->list_) {
      if (!this_observed->list_->SetVisible(true) || !this_observed) {
        return false;
      }
      if (!this_observed->list_->Move(rcList, true, false) || !this_observed) {
        return false;
      }
      this_observed->list_->ScrollToListItem(this_observed->select_item_);
      if (!this_observed) {
        return false;
      }
    }
    return true;
  }

  CFX_FloatRect rcButton = rcClient;
  rcButton.left = std::max(rcButton.right - kDefaultButtonWidth, rcClient.left);
  if (this_observed->button_) {
    this_observed->button_->Move(rcButton, true, false);
    if (!this_observed) {
      return false;
    }
  }

  CFX_FloatRect rcEdit = rcClient;
  rcEdit.right = std::max(rcButton.left - 1.0f, rcEdit.left);
  if (this_observed->edit_) {
    this_observed->edit_->Move(rcEdit, true, false);
    if (!this_observed) {
      return false;
    }
  }
  if (this_observed->list_) {
    if (!this_observed->list_->SetVisible(false)) {
      this_observed->list_ = nullptr;  // Gone, dangling even.
      return false;
    }
    if (!this_observed) {
      return false;
    }
  }
  return true;
}

void CPWL_ComboBox::SelectAll() {
  if (edit_ && HasFlag(PCBS_ALLOWCUSTOMTEXT)) {
    edit_->SelectAllText();
  }
}

CFX_FloatRect CPWL_ComboBox::GetFocusRect() const {
  return CFX_FloatRect();
}

bool CPWL_ComboBox::SetPopup(bool bPopup) {
  ObservedPtr<CPWL_ComboBox> this_observed(this);
  if (!this_observed->list_) {
    return true;
  }
  if (bPopup == this_observed->is_popup_) {
    return true;
  }
  float fListHeight = this_observed->list_->GetContentRect().Height();
  if (!FXSYS_IsFloatBigger(fListHeight, 0.0f)) {
    return true;
  }
  if (!bPopup) {
    this_observed->is_popup_ = false;
    return Move(this_observed->old_window_rect_, true, true);
  }
  if (GetFillerNotify()->OnPopupPreOpen(GetAttachedData(), {})) {
    return !!this_observed;
  }
  if (!this_observed) {
    return false;
  }
  float fBorderWidth = this_observed->list_->GetBorderWidth() * 2;
  float fPopupMin = 0.0f;
  if (this_observed->list_->GetCount() > 3) {
    fPopupMin = this_observed->list_->GetFirstHeight() * 3 + fBorderWidth;
  }
  float fPopupMax = fListHeight + fBorderWidth;
  bool bBottom;
  float fPopupRet;
  this_observed->GetFillerNotify()->QueryWherePopup(
      this_observed->GetAttachedData(), fPopupMin, fPopupMax, &bBottom,
      &fPopupRet);
  if (!FXSYS_IsFloatBigger(fPopupRet, 0.0f)) {
    return true;
  }
  this_observed->old_window_rect_ = this_observed->CPWL_Wnd::GetWindowRect();
  this_observed->is_popup_ = bPopup;
  this_observed->is_bottom_ = bBottom;

  CFX_FloatRect rcWindow = this_observed->old_window_rect_;
  if (bBottom) {
    rcWindow.bottom -= fPopupRet;
  } else {
    rcWindow.top += fPopupRet;
  }
  if (!this_observed->Move(rcWindow, true, true)) {
    return false;
  }
  this_observed->GetFillerNotify()->OnPopupPostOpen(
      this_observed->GetAttachedData(), {});
  return !!this_observed;
}

bool CPWL_ComboBox::OnKeyDown(FWL_VKEYCODE nKeyCode,
                              Mask<FWL_EVENTFLAG> nFlag) {
  ObservedPtr<CPWL_ComboBox> this_observed(this);
  if (!this_observed->list_) {
    return false;
  }
  if (!this_observed->edit_) {
    return false;
  }
  this_observed->select_item_ = -1;

  switch (nKeyCode) {
    case FWL_VKEY_Up:
      if (this_observed->list_->GetCurSel() > 0) {
        if (this_observed->GetFillerNotify()->OnPopupPreOpen(GetAttachedData(),
                                                             nFlag) ||
            !this_observed) {
          return false;
        }
        if (this_observed->GetFillerNotify()->OnPopupPostOpen(GetAttachedData(),
                                                              nFlag) ||
            !this_observed) {
          return false;
        }
        if (this_observed->list_->IsMovementKey(nKeyCode)) {
          if (this_observed->list_->OnMovementKeyDown(nKeyCode, nFlag) ||
              !this_observed) {
            return false;
          }
          this_observed->SetSelectText();
        }
      }
      return true;
    case FWL_VKEY_Down:
      if (this_observed->list_->GetCurSel() <
          this_observed->list_->GetCount() - 1) {
        if (this_observed->GetFillerNotify()->OnPopupPreOpen(GetAttachedData(),
                                                             nFlag) ||
            !this_observed) {
          return false;
        }
        if (this_observed->GetFillerNotify()->OnPopupPostOpen(GetAttachedData(),
                                                              nFlag) ||
            !this_observed) {
          return false;
        }
        if (this_observed->list_->IsMovementKey(nKeyCode)) {
          if (this_observed->list_->OnMovementKeyDown(nKeyCode, nFlag) ||
              !this_observed) {
            return false;
          }
          this_observed->SetSelectText();
        }
      }
      return true;
    default:
      break;
  }
  if (this_observed->HasFlag(PCBS_ALLOWCUSTOMTEXT)) {
    return this_observed->edit_->OnKeyDown(nKeyCode, nFlag);
  }
  return false;
}

bool CPWL_ComboBox::OnChar(uint16_t nChar, Mask<FWL_EVENTFLAG> nFlag) {
  ObservedPtr<CPWL_ComboBox> this_observed(this);
  if (!this_observed->list_) {
    return false;
  }
  if (!this_observed->edit_) {
    return false;
  }
  // In a combo box if the ENTER/SPACE key is pressed, show the combo box
  // options.
  switch (nChar) {
    case pdfium::ascii::kReturn:
      if (!this_observed->SetPopup(!this_observed->IsPopup())) {
        return false;
      }
      this_observed->SetSelectText();
      return true;
    case pdfium::ascii::kSpace:
      // Show the combo box options with space only if the combo box is not
      // editable
      if (!this_observed->HasFlag(PCBS_ALLOWCUSTOMTEXT)) {
        if (!this_observed->IsPopup()) {
          if (!this_observed->SetPopup(/*bPopUp=*/true)) {
            return false;
          }
          this_observed->SetSelectText();
        }
        return true;
      }
      break;
    default:
      break;
  }

  this_observed->select_item_ = -1;
  if (this_observed->HasFlag(PCBS_ALLOWCUSTOMTEXT)) {
    return this_observed->edit_->OnChar(nChar, nFlag);
  }
  if (this_observed->GetFillerNotify()->OnPopupPreOpen(GetAttachedData(),
                                                       nFlag) ||
      !this_observed) {
    return false;
  }
  if (this_observed->GetFillerNotify()->OnPopupPostOpen(GetAttachedData(),
                                                        nFlag) ||
      !this_observed) {
    return false;
  }
  if (!this_observed->list_->IsChar(nChar, nFlag)) {
    return false;
  }
  return this_observed->list_->OnCharNotify(nChar, nFlag);
}

void CPWL_ComboBox::NotifyLButtonDown(CPWL_Wnd* child, const CFX_PointF& pos) {
  if (child == button_) {
    (void)SetPopup(!is_popup_);
    // Note, |this| may no longer be viable at this point. If more work needs to
    // be done, check the return value of SetPopup().
  }
}

void CPWL_ComboBox::NotifyLButtonUp(CPWL_Wnd* child, const CFX_PointF& pos) {
  if (!edit_ || !list_ || child != list_) {
    return;
  }

  SetSelectText();
  SelectAllText();
  edit_->SetFocus();
  (void)SetPopup(false);
  // Note, |this| may no longer be viable at this point. If more work needs to
  // be done, check the return value of SetPopup().
}

bool CPWL_ComboBox::IsPopup() const {
  return is_popup_;
}

void CPWL_ComboBox::SetSelectText() {
  edit_->SelectAllText();
  edit_->ReplaceSelection(list_->GetText());
  edit_->SelectAllText();
  select_item_ = list_->GetCurSel();
}
