// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/pdfwindow/PWL_ListBox.h"

#include "fpdfsdk/include/pdfwindow/PWL_Edit.h"
#include "fpdfsdk/include/pdfwindow/PWL_EditCtrl.h"
#include "fpdfsdk/include/pdfwindow/PWL_ScrollBar.h"
#include "fpdfsdk/include/pdfwindow/PWL_Utils.h"
#include "fpdfsdk/include/pdfwindow/PWL_Wnd.h"
#include "public/fpdf_fwlevent.h"

#define IsFloatZero(f) ((f) < 0.0001 && (f) > -0.0001)
#define IsFloatBigger(fa, fb) ((fa) > (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatSmaller(fa, fb) ((fa) < (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatEqual(fa, fb) IsFloatZero((fa) - (fb))

CPWL_List_Notify::CPWL_List_Notify(CPWL_ListBox* pList) : m_pList(pList) {
  ASSERT(m_pList);
}

CPWL_List_Notify::~CPWL_List_Notify() {}

void CPWL_List_Notify::IOnSetScrollInfoY(FX_FLOAT fPlateMin,
                                         FX_FLOAT fPlateMax,
                                         FX_FLOAT fContentMin,
                                         FX_FLOAT fContentMax,
                                         FX_FLOAT fSmallStep,
                                         FX_FLOAT fBigStep) {
  PWL_SCROLL_INFO Info;

  Info.fPlateWidth = fPlateMax - fPlateMin;
  Info.fContentMin = fContentMin;
  Info.fContentMax = fContentMax;
  Info.fSmallStep = fSmallStep;
  Info.fBigStep = fBigStep;

  m_pList->OnNotify(m_pList, PNM_SETSCROLLINFO, SBT_VSCROLL, (intptr_t)&Info);

  if (CPWL_ScrollBar* pScroll = m_pList->GetVScrollBar()) {
    if (IsFloatBigger(Info.fPlateWidth, Info.fContentMax - Info.fContentMin) ||
        IsFloatEqual(Info.fPlateWidth, Info.fContentMax - Info.fContentMin)) {
      if (pScroll->IsVisible()) {
        pScroll->SetVisible(FALSE);
        m_pList->RePosChildWnd();
      }
    } else {
      if (!pScroll->IsVisible()) {
        pScroll->SetVisible(TRUE);
        m_pList->RePosChildWnd();
      }
    }
  }
}

void CPWL_List_Notify::IOnSetScrollPosY(FX_FLOAT fy) {
  m_pList->OnNotify(m_pList, PNM_SETSCROLLPOS, SBT_VSCROLL, (intptr_t)&fy);
}

void CPWL_List_Notify::IOnInvalidateRect(CPDF_Rect* pRect) {
  m_pList->InvalidateRect(pRect);
}

CPWL_ListBox::CPWL_ListBox()
    : m_pList(NULL),
      m_pListNotify(NULL),
      m_bMouseDown(FALSE),
      m_bHoverSel(FALSE),
      m_pFillerNotify(NULL) {
  m_pList = IFX_List::NewList();
}

CPWL_ListBox::~CPWL_ListBox() {
  IFX_List::DelList(m_pList);
  delete m_pListNotify;
  m_pListNotify = NULL;
}

CFX_ByteString CPWL_ListBox::GetClassName() const {
  return "CPWL_ListBox";
}

void CPWL_ListBox::OnCreated() {
  if (m_pList) {
    delete m_pListNotify;

    m_pList->SetFontMap(GetFontMap());
    m_pList->SetNotify(m_pListNotify = new CPWL_List_Notify(this));

    SetHoverSel(HasFlag(PLBS_HOVERSEL));
    m_pList->SetMultipleSel(HasFlag(PLBS_MULTIPLESEL));
    m_pList->SetFontSize(GetCreationParam().fFontSize);

    m_bHoverSel = HasFlag(PLBS_HOVERSEL);
  }
}

void CPWL_ListBox::OnDestroy() {
  delete m_pListNotify;
  m_pListNotify = NULL;
}

void CPWL_ListBox::GetThisAppearanceStream(CFX_ByteTextBuf& sAppStream) {
  CPWL_Wnd::GetThisAppearanceStream(sAppStream);

  CFX_ByteTextBuf sListItems;

  if (m_pList) {
    CPDF_Rect rcPlate = m_pList->GetPlateRect();
    for (int32_t i = 0, sz = m_pList->GetCount(); i < sz; i++) {
      CPDF_Rect rcItem = m_pList->GetItemRect(i);

      if (rcItem.bottom > rcPlate.top || rcItem.top < rcPlate.bottom)
        continue;

      CPDF_Point ptOffset(rcItem.left, (rcItem.top + rcItem.bottom) * 0.5f);
      if (m_pList->IsItemSelected(i)) {
        sListItems << CPWL_Utils::GetRectFillAppStream(
            rcItem, PWL_DEFAULT_SELBACKCOLOR);
        CFX_ByteString sItem =
            CPWL_Utils::GetEditAppStream(m_pList->GetItemEdit(i), ptOffset);
        if (sItem.GetLength() > 0) {
          sListItems << "BT\n"
                     << CPWL_Utils::GetColorAppStream(PWL_DEFAULT_SELTEXTCOLOR)
                     << sItem << "ET\n";
        }
      } else {
        CFX_ByteString sItem =
            CPWL_Utils::GetEditAppStream(m_pList->GetItemEdit(i), ptOffset);
        if (sItem.GetLength() > 0) {
          sListItems << "BT\n" << CPWL_Utils::GetColorAppStream(GetTextColor())
                     << sItem << "ET\n";
        }
      }
    }
  }

  if (sListItems.GetLength() > 0) {
    CFX_ByteTextBuf sClip;
    CPDF_Rect rcClient = GetClientRect();

    sClip << "q\n";
    sClip << rcClient.left << " " << rcClient.bottom << " "
          << rcClient.right - rcClient.left << " "
          << rcClient.top - rcClient.bottom << " re W n\n";

    sClip << sListItems << "Q\n";

    sAppStream << "/Tx BMC\n" << sClip << "EMC\n";
  }
}

void CPWL_ListBox::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                      CFX_Matrix* pUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, pUser2Device);

  if (m_pList) {
    CPDF_Rect rcPlate = m_pList->GetPlateRect();
    CPDF_Rect rcList = GetListRect();
    CPDF_Rect rcClient = GetClientRect();

    for (int32_t i = 0, sz = m_pList->GetCount(); i < sz; i++) {
      CPDF_Rect rcItem = m_pList->GetItemRect(i);
      if (rcItem.bottom > rcPlate.top || rcItem.top < rcPlate.bottom)
        continue;

      CPDF_Point ptOffset(rcItem.left, (rcItem.top + rcItem.bottom) * 0.5f);
      if (IFX_Edit* pEdit = m_pList->GetItemEdit(i)) {
        CPDF_Rect rcContent = pEdit->GetContentRect();
        if (rcContent.Width() > rcClient.Width())
          rcItem.Intersect(rcList);
        else
          rcItem.Intersect(rcClient);
      }

      if (m_pList->IsItemSelected(i)) {
        IFX_SystemHandler* pSysHandler = GetSystemHandler();
        if (pSysHandler && pSysHandler->IsSelectionImplemented()) {
          IFX_Edit::DrawEdit(
              pDevice, pUser2Device, m_pList->GetItemEdit(i),
              CPWL_Utils::PWLColorToFXColor(GetTextColor()),
              CPWL_Utils::PWLColorToFXColor(GetTextStrokeColor()), rcList,
              ptOffset, NULL, pSysHandler, m_pFormFiller);
          pSysHandler->OutputSelectedRect(m_pFormFiller, rcItem);
        } else {
          CPWL_Utils::DrawFillRect(pDevice, pUser2Device, rcItem,
                                   ArgbEncode(255, 0, 51, 113));
          IFX_Edit::DrawEdit(pDevice, pUser2Device, m_pList->GetItemEdit(i),
                             ArgbEncode(255, 255, 255, 255), 0, rcList,
                             ptOffset, NULL, pSysHandler, m_pFormFiller);
        }
      } else {
        IFX_SystemHandler* pSysHandler = GetSystemHandler();
        IFX_Edit::DrawEdit(pDevice, pUser2Device, m_pList->GetItemEdit(i),
                           CPWL_Utils::PWLColorToFXColor(GetTextColor()),
                           CPWL_Utils::PWLColorToFXColor(GetTextStrokeColor()),
                           rcList, ptOffset, NULL, pSysHandler, NULL);
      }
    }
  }
}

FX_BOOL CPWL_ListBox::OnKeyDown(FX_WORD nChar, FX_DWORD nFlag) {
  CPWL_Wnd::OnKeyDown(nChar, nFlag);

  if (!m_pList)
    return FALSE;

  switch (nChar) {
    default:
      return FALSE;
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Home:
    case FWL_VKEY_Left:
    case FWL_VKEY_End:
    case FWL_VKEY_Right:
      break;
  }

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
    case FWL_VKEY_Delete:
      break;
  }

  FX_BOOL bExit = FALSE;
  OnNotifySelChanged(TRUE, bExit, nFlag);

  return TRUE;
}

FX_BOOL CPWL_ListBox::OnChar(FX_WORD nChar, FX_DWORD nFlag) {
  CPWL_Wnd::OnChar(nChar, nFlag);

  if (!m_pList)
    return FALSE;

  if (!m_pList->OnChar(nChar, IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag)))
    return FALSE;

  FX_BOOL bExit = FALSE;
  OnNotifySelChanged(TRUE, bExit, nFlag);

  return TRUE;
}

FX_BOOL CPWL_ListBox::OnLButtonDown(const CPDF_Point& point, FX_DWORD nFlag) {
  CPWL_Wnd::OnLButtonDown(point, nFlag);

  if (ClientHitTest(point)) {
    m_bMouseDown = TRUE;
    SetFocus();
    SetCapture();

    if (m_pList)
      m_pList->OnMouseDown(point, IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
  }

  return TRUE;
}

FX_BOOL CPWL_ListBox::OnLButtonUp(const CPDF_Point& point, FX_DWORD nFlag) {
  CPWL_Wnd::OnLButtonUp(point, nFlag);

  if (m_bMouseDown) {
    ReleaseCapture();
    m_bMouseDown = FALSE;
  }

  FX_BOOL bExit = FALSE;
  OnNotifySelChanged(FALSE, bExit, nFlag);

  return TRUE;
}

void CPWL_ListBox::SetHoverSel(FX_BOOL bHoverSel) {
  m_bHoverSel = bHoverSel;
}

FX_BOOL CPWL_ListBox::OnMouseMove(const CPDF_Point& point, FX_DWORD nFlag) {
  CPWL_Wnd::OnMouseMove(point, nFlag);

  if (m_bHoverSel && !IsCaptureMouse() && ClientHitTest(point)) {
    if (m_pList)
      m_pList->Select(m_pList->GetItemIndex(point));
  }

  if (m_bMouseDown) {
    if (m_pList)
      m_pList->OnMouseMove(point, IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
  }

  return TRUE;
}

void CPWL_ListBox::OnNotify(CPWL_Wnd* pWnd,
                            FX_DWORD msg,
                            intptr_t wParam,
                            intptr_t lParam) {
  CPWL_Wnd::OnNotify(pWnd, msg, wParam, lParam);

  FX_FLOAT fPos;

  switch (msg) {
    case PNM_SETSCROLLINFO:
      switch (wParam) {
        case SBT_VSCROLL:
          if (CPWL_Wnd* pChild = GetVScrollBar()) {
            pChild->OnNotify(pWnd, PNM_SETSCROLLINFO, wParam, lParam);
          }
          break;
      }
      break;
    case PNM_SETSCROLLPOS:
      switch (wParam) {
        case SBT_VSCROLL:
          if (CPWL_Wnd* pChild = GetVScrollBar()) {
            pChild->OnNotify(pWnd, PNM_SETSCROLLPOS, wParam, lParam);
          }
          break;
      }
      break;
    case PNM_SCROLLWINDOW:
      fPos = *(FX_FLOAT*)lParam;
      switch (wParam) {
        case SBT_VSCROLL:
          if (m_pList)
            m_pList->SetScrollPos(CPDF_Point(0, fPos));
          break;
      }
      break;
  }
}

void CPWL_ListBox::KillFocus() {
  CPWL_Wnd::KillFocus();
}

void CPWL_ListBox::RePosChildWnd() {
  CPWL_Wnd::RePosChildWnd();

  if (m_pList)
    m_pList->SetPlateRect(GetListRect());
}

void CPWL_ListBox::OnNotifySelChanged(FX_BOOL bKeyDown,
                                      FX_BOOL& bExit,
                                      FX_DWORD nFlag) {
  if (!m_pFillerNotify)
    return;

  FX_BOOL bRC = TRUE;
  CFX_WideString swChange = GetText();
  CFX_WideString strChangeEx;
  int nSelStart = 0;
  int nSelEnd = swChange.GetLength();
  m_pFillerNotify->OnBeforeKeyStroke(GetAttachedData(), swChange, strChangeEx,
                                     nSelStart, nSelEnd, bKeyDown, bRC, bExit,
                                     nFlag);
}

CPDF_Rect CPWL_ListBox::GetFocusRect() const {
  if (m_pList && m_pList->IsMultipleSel()) {
    CPDF_Rect rcCaret = m_pList->GetItemRect(m_pList->GetCaret());
    rcCaret.Intersect(GetClientRect());
    return rcCaret;
  }

  return CPWL_Wnd::GetFocusRect();
}

void CPWL_ListBox::AddString(const FX_WCHAR* string) {
  if (m_pList) {
    m_pList->AddString(string);
  }
}

CFX_WideString CPWL_ListBox::GetText() const {
  if (m_pList)
    return m_pList->GetText();

  return L"";
}

void CPWL_ListBox::SetFontSize(FX_FLOAT fFontSize) {
  if (m_pList)
    m_pList->SetFontSize(fFontSize);
}

FX_FLOAT CPWL_ListBox::GetFontSize() const {
  if (m_pList)
    return m_pList->GetFontSize();
  return 0.0f;
}

void CPWL_ListBox::Select(int32_t nItemIndex) {
  if (m_pList)
    m_pList->Select(nItemIndex);
}

void CPWL_ListBox::SetCaret(int32_t nItemIndex) {
  if (m_pList)
    m_pList->SetCaret(nItemIndex);
}

void CPWL_ListBox::SetTopVisibleIndex(int32_t nItemIndex) {
  if (m_pList)
    m_pList->SetTopItem(nItemIndex);
}

void CPWL_ListBox::ScrollToListItem(int32_t nItemIndex) {
  if (m_pList)
    m_pList->ScrollToListItem(nItemIndex);
}

void CPWL_ListBox::ResetContent() {
  if (m_pList)
    m_pList->Empty();
}

void CPWL_ListBox::Reset() {
  if (m_pList)
    m_pList->Cancel();
}

FX_BOOL CPWL_ListBox::IsMultipleSel() const {
  if (m_pList)
    return m_pList->IsMultipleSel();

  return FALSE;
}

int32_t CPWL_ListBox::GetCaretIndex() const {
  if (m_pList)
    return m_pList->GetCaret();

  return -1;
}

int32_t CPWL_ListBox::GetCurSel() const {
  if (m_pList)
    return m_pList->GetSelect();

  return -1;
}

FX_BOOL CPWL_ListBox::IsItemSelected(int32_t nItemIndex) const {
  if (m_pList)
    return m_pList->IsItemSelected(nItemIndex);

  return FALSE;
}

int32_t CPWL_ListBox::GetTopVisibleIndex() const {
  if (m_pList) {
    m_pList->ScrollToListItem(m_pList->GetFirstSelected());
    return m_pList->GetTopItem();
  }

  return -1;
}

int32_t CPWL_ListBox::GetCount() const {
  if (m_pList)
    return m_pList->GetCount();

  return 0;
}

int32_t CPWL_ListBox::FindNext(int32_t nIndex, FX_WCHAR nChar) const {
  if (m_pList)
    return m_pList->FindNext(nIndex, nChar);

  return nIndex;
}

CPDF_Rect CPWL_ListBox::GetContentRect() const {
  if (m_pList)
    return m_pList->GetContentRect();

  return CPDF_Rect();
}

FX_FLOAT CPWL_ListBox::GetFirstHeight() const {
  if (m_pList)
    return m_pList->GetFirstHeight();

  return 0.0f;
}

CPDF_Rect CPWL_ListBox::GetListRect() const {
  return CPWL_Utils::DeflateRect(
      GetWindowRect(), (FX_FLOAT)(GetBorderWidth() + GetInnerBorderWidth()));
}

FX_BOOL CPWL_ListBox::OnMouseWheel(short zDelta,
                                   const CPDF_Point& point,
                                   FX_DWORD nFlag) {
  if (!m_pList)
    return FALSE;

  if (zDelta < 0) {
    m_pList->OnVK_DOWN(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
  } else {
    m_pList->OnVK_UP(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
  }

  FX_BOOL bExit = FALSE;
  OnNotifySelChanged(FALSE, bExit, nFlag);
  return TRUE;
}
