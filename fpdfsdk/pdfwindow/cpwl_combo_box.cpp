// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/cpwl_combo_box.h"

#include <algorithm>
#include <sstream>

#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "fpdfsdk/fxedit/fxet_list.h"
#include "fpdfsdk/pdfwindow/cpwl_edit.h"
#include "fpdfsdk/pdfwindow/cpwl_edit_ctrl.h"
#include "fpdfsdk/pdfwindow/cpwl_list_box.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"
#include "public/fpdf_fwlevent.h"

namespace {

constexpr float kDefaultFontSize = 12.0f;
constexpr float kTriangleHalfLength = 3.0f;

}  // namespace

bool CPWL_CBListBox::OnLButtonUp(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonUp(point, nFlag);

  if (!m_bMouseDown)
    return true;

  ReleaseCapture();
  m_bMouseDown = false;

  if (!ClientHitTest(point))
    return true;
  if (CPWL_Wnd* pParent = GetParentWindow())
    pParent->NotifyLButtonUp(this, point);

  return !OnNotifySelectionChanged(false, nFlag);
}

bool CPWL_CBListBox::IsMovementKey(uint16_t nChar) const {
  switch (nChar) {
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_Left:
    case FWL_VKEY_End:
    case FWL_VKEY_Right:
      return true;
    default:
      return false;
  }
}

bool CPWL_CBListBox::OnMovementKeyDown(uint16_t nChar, uint32_t nFlag) {
  ASSERT(IsMovementKey(nChar));

  switch (nChar) {
    case FWL_VKEY_Up:
      m_pList->OnVK_UP(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
      break;
    case FWL_VKEY_Down:
      m_pList->OnVK_DOWN(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
      break;
    case FWL_VKEY_Home:
      m_pList->OnVK_HOME(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
      break;
    case FWL_VKEY_Left:
      m_pList->OnVK_LEFT(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
      break;
    case FWL_VKEY_End:
      m_pList->OnVK_END(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
      break;
    case FWL_VKEY_Right:
      m_pList->OnVK_RIGHT(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
      break;
  }
  return OnNotifySelectionChanged(true, nFlag);
}

bool CPWL_CBListBox::IsChar(uint16_t nChar, uint32_t nFlag) const {
  return m_pList->OnChar(nChar, IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
}

bool CPWL_CBListBox::OnCharNotify(uint16_t nChar, uint32_t nFlag) {
  if (CPWL_ComboBox* pComboBox = (CPWL_ComboBox*)GetParentWindow())
    pComboBox->SetSelectText();

  return OnNotifySelectionChanged(true, nFlag);
}

void CPWL_CBButton::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                       CFX_Matrix* pUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, pUser2Device);

  CFX_FloatRect rectWnd = CPWL_Wnd::GetWindowRect();

  if (!IsVisible() || rectWnd.IsEmpty())
    return;

  CFX_PointF ptCenter = GetCenterPoint();

  CFX_PointF pt1(ptCenter.x - kTriangleHalfLength,
                 ptCenter.y + kTriangleHalfLength * 0.5f);
  CFX_PointF pt2(ptCenter.x + kTriangleHalfLength,
                 ptCenter.y + kTriangleHalfLength * 0.5f);
  CFX_PointF pt3(ptCenter.x, ptCenter.y - kTriangleHalfLength * 0.5f);

  if (IsFloatBigger(rectWnd.right - rectWnd.left, kTriangleHalfLength * 2) &&
      IsFloatBigger(rectWnd.top - rectWnd.bottom, kTriangleHalfLength)) {
    CFX_PathData path;
    path.AppendPoint(pt1, FXPT_TYPE::MoveTo, false);
    path.AppendPoint(pt2, FXPT_TYPE::LineTo, false);
    path.AppendPoint(pt3, FXPT_TYPE::LineTo, false);
    path.AppendPoint(pt1, FXPT_TYPE::LineTo, false);

    pDevice->DrawPath(&path, pUser2Device, nullptr,
                      PWL_DEFAULT_BLACKCOLOR.ToFXColor(GetTransparency()), 0,
                      FXFILL_ALTERNATE);
  }
}

bool CPWL_CBButton::OnLButtonDown(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonDown(point, nFlag);

  SetCapture();

  if (CPWL_Wnd* pParent = GetParentWindow())
    pParent->NotifyLButtonDown(this, point);

  return true;
}

bool CPWL_CBButton::OnLButtonUp(const CFX_PointF& point, uint32_t nFlag) {
  CPWL_Wnd::OnLButtonUp(point, nFlag);

  ReleaseCapture();

  return true;
}

CPWL_ComboBox::CPWL_ComboBox() {}

CPWL_ComboBox::~CPWL_ComboBox() {}

CFX_ByteString CPWL_ComboBox::GetClassName() const {
  return "CPWL_ComboBox";
}

void CPWL_ComboBox::OnCreate(PWL_CREATEPARAM& cp) {
  cp.dwFlags &= ~PWS_HSCROLL;
  cp.dwFlags &= ~PWS_VSCROLL;
}

void CPWL_ComboBox::OnDestroy() {
  // Until cleanup takes place in the virtual destructor for CPWL_Wnd
  // subclasses, implement the virtual OnDestroy method that does the
  // cleanup first, then invokes the superclass OnDestroy ... gee,
  // like a dtor would.
  m_pList.Release();
  m_pButton.Release();
  m_pEdit.Release();
  CPWL_Wnd::OnDestroy();
}

void CPWL_ComboBox::SetFocus() {
  if (m_pEdit)
    m_pEdit->SetFocus();
}

void CPWL_ComboBox::KillFocus() {
  SetPopup(false);
  CPWL_Wnd::KillFocus();
}

CFX_WideString CPWL_ComboBox::GetSelectedText() {
  if (m_pEdit)
    return m_pEdit->GetSelectedText();

  return CFX_WideString();
}

CFX_WideString CPWL_ComboBox::GetText() const {
  if (m_pEdit) {
    return m_pEdit->GetText();
  }
  return CFX_WideString();
}

void CPWL_ComboBox::SetText(const CFX_WideString& text) {
  if (m_pEdit)
    m_pEdit->SetText(text);
}

void CPWL_ComboBox::AddString(const CFX_WideString& str) {
  if (m_pList)
    m_pList->AddString(str);
}

int32_t CPWL_ComboBox::GetSelect() const {
  return m_nSelectItem;
}

void CPWL_ComboBox::SetSelect(int32_t nItemIndex) {
  if (m_pList)
    m_pList->Select(nItemIndex);

  m_pEdit->SetText(m_pList->GetText());
  m_nSelectItem = nItemIndex;
}

void CPWL_ComboBox::SetEditSel(int32_t nStartChar, int32_t nEndChar) {
  if (m_pEdit)
    m_pEdit->SetSel(nStartChar, nEndChar);
}

void CPWL_ComboBox::GetEditSel(int32_t& nStartChar, int32_t& nEndChar) const {
  nStartChar = -1;
  nEndChar = -1;

  if (m_pEdit)
    m_pEdit->GetSel(nStartChar, nEndChar);
}

void CPWL_ComboBox::Clear() {
  if (m_pEdit)
    m_pEdit->Clear();
}

void CPWL_ComboBox::CreateChildWnd(const PWL_CREATEPARAM& cp) {
  CreateEdit(cp);
  CreateButton(cp);
  CreateListBox(cp);
}

void CPWL_ComboBox::CreateEdit(const PWL_CREATEPARAM& cp) {
  if (m_pEdit)
    return;

  m_pEdit = new CPWL_Edit();
  m_pEdit->AttachFFLData(m_pFormFiller.Get());

  PWL_CREATEPARAM ecp = cp;
  ecp.pParentWnd = this;
  ecp.dwFlags = PWS_VISIBLE | PWS_CHILD | PWS_BORDER | PES_CENTER |
                PES_AUTOSCROLL | PES_UNDO;

  if (HasFlag(PWS_AUTOFONTSIZE))
    ecp.dwFlags |= PWS_AUTOFONTSIZE;

  if (!HasFlag(PCBS_ALLOWCUSTOMTEXT))
    ecp.dwFlags |= PWS_READONLY;

  ecp.rcRectWnd = CFX_FloatRect();
  ecp.dwBorderWidth = 0;
  ecp.nBorderStyle = BorderStyle::SOLID;
  m_pEdit->Create(ecp);
}

void CPWL_ComboBox::CreateButton(const PWL_CREATEPARAM& cp) {
  if (m_pButton)
    return;

  m_pButton = new CPWL_CBButton;

  PWL_CREATEPARAM bcp = cp;
  bcp.pParentWnd = this;
  bcp.dwFlags = PWS_VISIBLE | PWS_CHILD | PWS_BORDER | PWS_BACKGROUND;
  bcp.sBackgroundColor = CFX_Color(COLORTYPE_RGB, 220.0f / 255.0f,
                                   220.0f / 255.0f, 220.0f / 255.0f);
  bcp.sBorderColor = PWL_DEFAULT_BLACKCOLOR;
  bcp.dwBorderWidth = 2;
  bcp.nBorderStyle = BorderStyle::BEVELED;
  bcp.eCursorType = FXCT_ARROW;
  m_pButton->Create(bcp);
}

void CPWL_ComboBox::CreateListBox(const PWL_CREATEPARAM& cp) {
  if (m_pList)
    return;

  m_pList = new CPWL_CBListBox();
  m_pList->AttachFFLData(m_pFormFiller.Get());

  PWL_CREATEPARAM lcp = cp;
  lcp.pParentWnd = this;
  lcp.dwFlags =
      PWS_CHILD | PWS_BORDER | PWS_BACKGROUND | PLBS_HOVERSEL | PWS_VSCROLL;
  lcp.nBorderStyle = BorderStyle::SOLID;
  lcp.dwBorderWidth = 1;
  lcp.eCursorType = FXCT_ARROW;
  lcp.rcRectWnd = CFX_FloatRect();

  if (cp.dwFlags & PWS_AUTOFONTSIZE)
    lcp.fFontSize = kDefaultFontSize;
  else
    lcp.fFontSize = cp.fFontSize;

  if (cp.sBorderColor.nColorType == COLORTYPE_TRANSPARENT)
    lcp.sBorderColor = PWL_DEFAULT_BLACKCOLOR;

  if (cp.sBackgroundColor.nColorType == COLORTYPE_TRANSPARENT)
    lcp.sBackgroundColor = PWL_DEFAULT_WHITECOLOR;

  m_pList->Create(lcp);
}

void CPWL_ComboBox::RePosChildWnd() {
  const CFX_FloatRect rcClient = GetClientRect();
  if (m_bPopup) {
    const float fOldWindowHeight = m_rcOldWindow.Height();
    const float fOldClientHeight = fOldWindowHeight - GetBorderWidth() * 2;

    CFX_FloatRect rcList = CPWL_Wnd::GetWindowRect();
    CFX_FloatRect rcButton = rcClient;
    rcButton.left =
        std::max(rcButton.right - PWL_COMBOBOX_BUTTON_WIDTH, rcClient.left);
    CFX_FloatRect rcEdit = rcClient;
    rcEdit.right = std::max(rcButton.left - 1.0f, rcEdit.left);
    if (m_bBottom) {
      rcButton.bottom = rcButton.top - fOldClientHeight;
      rcEdit.bottom = rcEdit.top - fOldClientHeight;
      rcList.top -= fOldWindowHeight;
    } else {
      rcButton.top = rcButton.bottom + fOldClientHeight;
      rcEdit.top = rcEdit.bottom + fOldClientHeight;
      rcList.bottom += fOldWindowHeight;
    }

    if (m_pButton)
      m_pButton->Move(rcButton, true, false);

    if (m_pEdit)
      m_pEdit->Move(rcEdit, true, false);

    if (m_pList) {
      m_pList->SetVisible(true);
      m_pList->Move(rcList, true, false);
      m_pList->ScrollToListItem(m_nSelectItem);
    }
    return;
  }

  CFX_FloatRect rcButton = rcClient;
  rcButton.left =
      std::max(rcButton.right - PWL_COMBOBOX_BUTTON_WIDTH, rcClient.left);

  if (m_pButton)
    m_pButton->Move(rcButton, true, false);

  CFX_FloatRect rcEdit = rcClient;
  rcEdit.right = std::max(rcButton.left - 1.0f, rcEdit.left);

  if (m_pEdit)
    m_pEdit->Move(rcEdit, true, false);

  if (m_pList)
    m_pList->SetVisible(false);
}

void CPWL_ComboBox::SelectAll() {
  if (m_pEdit && HasFlag(PCBS_ALLOWCUSTOMTEXT))
    m_pEdit->SelectAll();
}

CFX_FloatRect CPWL_ComboBox::GetFocusRect() const {
  return CFX_FloatRect();
}

void CPWL_ComboBox::SetPopup(bool bPopup) {
  if (!m_pList)
    return;
  if (bPopup == m_bPopup)
    return;
  float fListHeight = m_pList->GetContentRect().Height();
  if (!IsFloatBigger(fListHeight, 0.0f))
    return;

  if (!bPopup) {
    m_bPopup = bPopup;
    Move(m_rcOldWindow, true, true);
    return;
  }

  if (!m_pFillerNotify)
    return;

#ifdef PDF_ENABLE_XFA
  if (m_pFillerNotify->OnPopupPreOpen(GetAttachedData(), 0))
    return;
#endif  // PDF_ENABLE_XFA

  float fBorderWidth = m_pList->GetBorderWidth() * 2;
  float fPopupMin = 0.0f;
  if (m_pList->GetCount() > 3)
    fPopupMin = m_pList->GetFirstHeight() * 3 + fBorderWidth;
  float fPopupMax = fListHeight + fBorderWidth;

  bool bBottom;
  float fPopupRet;
  m_pFillerNotify->QueryWherePopup(GetAttachedData(), fPopupMin, fPopupMax,
                                   &bBottom, &fPopupRet);
  if (!IsFloatBigger(fPopupRet, 0.0f))
    return;

  m_rcOldWindow = CPWL_Wnd::GetWindowRect();
  m_bPopup = bPopup;
  m_bBottom = bBottom;

  CFX_FloatRect rcWindow = m_rcOldWindow;
  if (bBottom)
    rcWindow.bottom -= fPopupRet;
  else
    rcWindow.top += fPopupRet;

  Move(rcWindow, true, true);
#ifdef PDF_ENABLE_XFA
  m_pFillerNotify->OnPopupPostOpen(GetAttachedData(), 0);
#endif  // PDF_ENABLE_XFA
}

bool CPWL_ComboBox::OnKeyDown(uint16_t nChar, uint32_t nFlag) {
  if (!m_pList)
    return false;
  if (!m_pEdit)
    return false;

  m_nSelectItem = -1;

  switch (nChar) {
    case FWL_VKEY_Up:
      if (m_pList->GetCurSel() > 0) {
#ifdef PDF_ENABLE_XFA
        if (m_pFillerNotify) {
          if (m_pFillerNotify->OnPopupPreOpen(GetAttachedData(), nFlag))
            return false;
          if (m_pFillerNotify->OnPopupPostOpen(GetAttachedData(), nFlag))
            return false;
        }
#endif  // PDF_ENABLE_XFA
        if (m_pList->IsMovementKey(nChar)) {
          if (m_pList->OnMovementKeyDown(nChar, nFlag))
            return false;
          SetSelectText();
        }
      }
      return true;
    case FWL_VKEY_Down:
      if (m_pList->GetCurSel() < m_pList->GetCount() - 1) {
#ifdef PDF_ENABLE_XFA
        if (m_pFillerNotify) {
          if (m_pFillerNotify->OnPopupPreOpen(GetAttachedData(), nFlag))
            return false;
          if (m_pFillerNotify->OnPopupPostOpen(GetAttachedData(), nFlag))
            return false;
        }
#endif  // PDF_ENABLE_XFA
        if (m_pList->IsMovementKey(nChar)) {
          if (m_pList->OnMovementKeyDown(nChar, nFlag))
            return false;
          SetSelectText();
        }
      }
      return true;
  }

  if (HasFlag(PCBS_ALLOWCUSTOMTEXT))
    return m_pEdit->OnKeyDown(nChar, nFlag);

  return false;
}

bool CPWL_ComboBox::OnChar(uint16_t nChar, uint32_t nFlag) {
  if (!m_pList)
    return false;

  if (!m_pEdit)
    return false;

  m_nSelectItem = -1;
  if (HasFlag(PCBS_ALLOWCUSTOMTEXT))
    return m_pEdit->OnChar(nChar, nFlag);

#ifdef PDF_ENABLE_XFA
  if (m_pFillerNotify) {
    if (m_pFillerNotify->OnPopupPreOpen(GetAttachedData(), nFlag))
      return false;
    if (m_pFillerNotify->OnPopupPostOpen(GetAttachedData(), nFlag))
      return false;
  }
#endif  // PDF_ENABLE_XFA
  if (!m_pList->IsChar(nChar, nFlag))
    return false;
  return m_pList->OnCharNotify(nChar, nFlag);
}

void CPWL_ComboBox::NotifyLButtonDown(CPWL_Wnd* child, const CFX_PointF& pos) {
  if (child == m_pButton)
    SetPopup(!m_bPopup);
}

void CPWL_ComboBox::NotifyLButtonUp(CPWL_Wnd* child, const CFX_PointF& pos) {
  if (!m_pEdit || !m_pList || child != m_pList)
    return;

  SetSelectText();
  SelectAll();
  m_pEdit->SetFocus();
  SetPopup(false);
}

bool CPWL_ComboBox::IsPopup() const {
  return m_bPopup;
}

void CPWL_ComboBox::SetSelectText() {
  m_pEdit->SelectAll();
  m_pEdit->ReplaceSel(m_pList->GetText());
  m_pEdit->SelectAll();
  m_nSelectItem = m_pList->GetCurSel();
}

void CPWL_ComboBox::SetFillerNotify(IPWL_Filler_Notify* pNotify) {
  m_pFillerNotify = pNotify;

  if (m_pEdit)
    m_pEdit->SetFillerNotify(pNotify);

  if (m_pList)
    m_pList->SetFillerNotify(pNotify);
}
