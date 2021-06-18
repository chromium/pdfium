// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_edit.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <utility>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfdoc/cpvt_word.h"
#include "core/fpdfdoc/ipvt_fontmap.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/fx_font.h"
#include "fpdfsdk/pwl/cpwl_caret.h"
#include "fpdfsdk/pwl/cpwl_edit_impl.h"
#include "fpdfsdk/pwl/cpwl_scroll_bar.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "fpdfsdk/pwl/ipwl_fillernotify.h"
#include "public/fpdf_fwlevent.h"
#include "third_party/base/check.h"

CPWL_Edit::CPWL_Edit(
    const CreateParams& cp,
    std::unique_ptr<IPWL_SystemHandler::PerWindowData> pAttachedData)
    : CPWL_Wnd(cp, std::move(pAttachedData)),
      m_pEditImpl(std::make_unique<CPWL_EditImpl>()) {
  GetCreationParams()->eCursorType = IPWL_SystemHandler::CursorStyle::kVBeam;
}

CPWL_Edit::~CPWL_Edit() {
  DCHECK(!m_bFocus);
}

void CPWL_Edit::SetText(const WideString& csText) {
  m_pEditImpl->SetTextAndPaint(csText);
}

bool CPWL_Edit::RePosChildWnd() {
  if (CPWL_ScrollBar* pVSB = GetVScrollBar()) {
    CFX_FloatRect rcWindow = m_rcOldWindow;
    CFX_FloatRect rcVScroll =
        CFX_FloatRect(rcWindow.right, rcWindow.bottom,
                      rcWindow.right + PWL_SCROLLBAR_WIDTH, rcWindow.top);

    ObservedPtr<CPWL_Edit> thisObserved(this);
    pVSB->Move(rcVScroll, true, false);
    if (!thisObserved)
      return false;
  }

  if (m_pCaret && !HasFlag(PES_TEXTOVERFLOW)) {
    CFX_FloatRect rect = GetClientRect();
    if (!rect.IsEmpty()) {
      // +1 for caret beside border
      rect.Inflate(1.0f, 1.0f);
      rect.Normalize();
    }
    m_pCaret->SetClipRect(rect);
  }

  m_pEditImpl->SetPlateRectAndPaint(GetClientRect());
  return true;
}

CFX_FloatRect CPWL_Edit::GetClientRect() const {
  float width = static_cast<float>(GetBorderWidth() + GetInnerBorderWidth());
  CFX_FloatRect rcClient = GetWindowRect().GetDeflated(width, width);
  if (CPWL_ScrollBar* pVSB = GetVScrollBar()) {
    if (pVSB->IsVisible()) {
      rcClient.right -= PWL_SCROLLBAR_WIDTH;
    }
  }

  return rcClient;
}

void CPWL_Edit::SetAlignFormatVerticalCenter() {
  m_pEditImpl->SetAlignmentVAndPaint(static_cast<int32_t>(PEAV_CENTER));
}

bool CPWL_Edit::CanSelectAll() const {
  return GetSelectWordRange() != m_pEditImpl->GetWholeWordRange();
}

bool CPWL_Edit::CanCopy() const {
  return !HasFlag(PES_PASSWORD) && m_pEditImpl->IsSelected();
}

bool CPWL_Edit::CanCut() const {
  return CanCopy() && !IsReadOnly();
}

void CPWL_Edit::CutText() {
  if (!CanCut())
    return;
  m_pEditImpl->ClearSelection();
}

void CPWL_Edit::OnCreated() {
  SetFontSize(GetCreationParams()->fFontSize);
  m_pEditImpl->SetFontMap(GetFontMap());
  m_pEditImpl->SetNotify(this);
  m_pEditImpl->Initialize();

  if (CPWL_ScrollBar* pScroll = GetVScrollBar()) {
    pScroll->RemoveFlag(PWS_AUTOTRANSPARENT);
    pScroll->SetTransparency(255);
  }

  SetParamByFlag();
  m_rcOldWindow = GetWindowRect();
}

void CPWL_Edit::SetParamByFlag() {
  if (HasFlag(PES_RIGHT)) {
    m_pEditImpl->SetAlignmentH(2);
  } else if (HasFlag(PES_MIDDLE)) {
    m_pEditImpl->SetAlignmentH(1);
  } else {
    m_pEditImpl->SetAlignmentH(0);
  }

  if (HasFlag(PES_CENTER)) {
    m_pEditImpl->SetAlignmentV(1);
  } else {
    m_pEditImpl->SetAlignmentV(0);
  }

  if (HasFlag(PES_PASSWORD)) {
    m_pEditImpl->SetPasswordChar('*');
  }

  m_pEditImpl->SetMultiLine(HasFlag(PES_MULTILINE));
  m_pEditImpl->SetAutoReturn(HasFlag(PES_AUTORETURN));
  m_pEditImpl->SetAutoFontSize(HasFlag(PWS_AUTOFONTSIZE));
  m_pEditImpl->SetAutoScroll(HasFlag(PES_AUTOSCROLL));
  m_pEditImpl->EnableUndo(HasFlag(PES_UNDO));

  if (HasFlag(PES_TEXTOVERFLOW)) {
    SetClipRect(CFX_FloatRect());
    m_pEditImpl->SetTextOverflow(true);
  } else {
    if (m_pCaret) {
      CFX_FloatRect rect = GetClientRect();
      if (!rect.IsEmpty()) {
        // +1 for caret beside border
        rect.Inflate(1.0f, 1.0f);
        rect.Normalize();
      }
      m_pCaret->SetClipRect(rect);
    }
  }
}

void CPWL_Edit::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                   const CFX_Matrix& mtUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, mtUser2Device);

  const CFX_FloatRect rcClient = GetClientRect();
  const BorderStyle border_style = GetBorderStyle();
  const int32_t nCharArray = m_pEditImpl->GetCharArray();
  bool draw_border = nCharArray > 0 && (border_style == BorderStyle::kSolid ||
                                        border_style == BorderStyle::kDash);
  if (draw_border) {
    FX_SAFE_INT32 nCharArraySafe = nCharArray;
    nCharArraySafe -= 1;
    nCharArraySafe *= 2;
    draw_border = nCharArraySafe.IsValid();
  }

  if (draw_border) {
    CFX_GraphStateData gsd;
    gsd.m_LineWidth = GetBorderWidth();
    if (border_style == BorderStyle::kDash) {
      gsd.m_DashArray = {static_cast<float>(GetBorderDash().nDash),
                         static_cast<float>(GetBorderDash().nGap)};
      gsd.m_DashPhase = GetBorderDash().nPhase;
    }

    const float width = (rcClient.right - rcClient.left) / nCharArray;
    CFX_Path path;
    CFX_PointF bottom(0, rcClient.bottom);
    CFX_PointF top(0, rcClient.top);
    for (int32_t i = 0; i < nCharArray - 1; ++i) {
      bottom.x = rcClient.left + width * (i + 1);
      top.x = bottom.x;
      path.AppendPoint(bottom, CFX_Path::Point::Type::kMove);
      path.AppendPoint(top, CFX_Path::Point::Type::kLine);
    }
    if (!path.GetPoints().empty()) {
      pDevice->DrawPath(&path, &mtUser2Device, &gsd, 0,
                        GetBorderColor().ToFXColor(255),
                        CFX_FillRenderOptions::EvenOddOptions());
    }
  }

  CFX_FloatRect rcClip;
  CPVT_WordRange wrRange = m_pEditImpl->GetVisibleWordRange();
  CPVT_WordRange* pRange = nullptr;
  if (!HasFlag(PES_TEXTOVERFLOW)) {
    rcClip = GetClientRect();
    pRange = &wrRange;
  }

  CPWL_EditImpl::DrawEdit(pDevice, mtUser2Device, m_pEditImpl.get(),
                          GetTextColor().ToFXColor(GetTransparency()), rcClip,
                          CFX_PointF(), pRange, GetSystemHandler(),
                          m_pFormFiller.Get());
}


void CPWL_Edit::OnSetFocus() {
  ObservedPtr<CPWL_Edit> observed_ptr(this);
  SetEditCaret(true);
  if (!observed_ptr)
    return;

  if (!IsReadOnly()) {
    if (CPWL_Wnd::FocusHandlerIface* pFocusHandler = GetFocusHandler()) {
      pFocusHandler->OnSetFocus(this);
      if (!observed_ptr)
        return;
    }
  }
  m_bFocus = true;
}

void CPWL_Edit::OnKillFocus() {
  ObservedPtr<CPWL_Edit> observed_ptr(this);
  CPWL_ScrollBar* pScroll = GetVScrollBar();
  if (pScroll && pScroll->IsVisible()) {
    pScroll->SetVisible(false);
    if (!observed_ptr)
      return;

    if (!Move(m_rcOldWindow, true, true))
      return;
  }

  m_pEditImpl->SelectNone();
  if (!observed_ptr)
    return;

  if (!SetCaret(false, CFX_PointF(), CFX_PointF()))
    return;

  SetCharSet(FX_CHARSET_ANSI);
  m_bFocus = false;
}

void CPWL_Edit::SetCharSpace(float fCharSpace) {
  m_pEditImpl->SetCharSpaceAndPaint(fCharSpace);
}

CPVT_WordRange CPWL_Edit::GetSelectWordRange() const {
  if (!m_pEditImpl->IsSelected())
    return CPVT_WordRange();

  int32_t nStart;
  int32_t nEnd;
  std::tie(nStart, nEnd) = m_pEditImpl->GetSelection();

  CPVT_WordPlace wpStart = m_pEditImpl->WordIndexToWordPlace(nStart);
  CPVT_WordPlace wpEnd = m_pEditImpl->WordIndexToWordPlace(nEnd);
  return CPVT_WordRange(wpStart, wpEnd);
}

bool CPWL_Edit::IsTextFull() const {
  return m_pEditImpl->IsTextFull();
}

float CPWL_Edit::GetCharArrayAutoFontSize(const CPDF_Font* pFont,
                                          const CFX_FloatRect& rcPlate,
                                          int32_t nCharArray) {
  if (!pFont || pFont->IsStandardFont())
    return 0.0f;

  const FX_RECT& rcBBox = pFont->GetFontBBox();

  CFX_FloatRect rcCell = rcPlate;
  float xdiv = rcCell.Width() / nCharArray * 1000.0f / rcBBox.Width();
  float ydiv = -rcCell.Height() * 1000.0f / rcBBox.Height();

  return xdiv < ydiv ? xdiv : ydiv;
}

void CPWL_Edit::SetCharArray(int32_t nCharArray) {
  if (!HasFlag(PES_CHARARRAY) || nCharArray <= 0)
    return;

  m_pEditImpl->SetCharArrayAndPaint(nCharArray);
  m_pEditImpl->SetTextOverflowAndPaint(true);

  if (!HasFlag(PWS_AUTOFONTSIZE))
    return;

  IPVT_FontMap* pFontMap = GetFontMap();
  if (!pFontMap)
    return;

  float fFontSize = GetCharArrayAutoFontSize(pFontMap->GetPDFFont(0).Get(),
                                             GetClientRect(), nCharArray);
  if (fFontSize <= 0.0f)
    return;

  m_pEditImpl->SetAutoFontSizeAndPaint(false);
  m_pEditImpl->SetFontSizeAndPaint(fFontSize);
}

void CPWL_Edit::SetLimitChar(int32_t nLimitChar) {
  m_pEditImpl->SetLimitCharAndPaint(nLimitChar);
}

CFX_FloatRect CPWL_Edit::GetFocusRect() const {
  return CFX_FloatRect();
}

bool CPWL_Edit::IsVScrollBarVisible() const {
  CPWL_ScrollBar* pScroll = GetVScrollBar();
  return pScroll && pScroll->IsVisible();
}

bool CPWL_Edit::OnKeyDown(uint16_t nChar, uint32_t nFlag) {
  if (m_bMouseDown)
    return true;

  if (nChar == FWL_VKEY_Delete) {
    if (m_pFillerNotify) {
      WideString strChange;
      WideString strChangeEx;

      int nSelStart;
      int nSelEnd;
      std::tie(nSelStart, nSelEnd) = GetSelection();

      if (nSelStart == nSelEnd)
        nSelEnd = nSelStart + 1;

      ObservedPtr<CPWL_Wnd> thisObserved(this);

      bool bRC;
      bool bExit;
      std::tie(bRC, bExit) = m_pFillerNotify->OnBeforeKeyStroke(
          GetAttachedData(), strChange, strChangeEx, nSelStart, nSelEnd, true,
          nFlag);

      if (!thisObserved)
        return false;

      if (!bRC)
        return false;
      if (bExit)
        return false;
    }
  }

  bool bRet = OnKeyDownInternal(nChar, nFlag);

  // In case of implementation swallow the OnKeyDown event.
  if (IsProceedtoOnChar(nChar, nFlag))
    return true;

  return bRet;
}

// static
bool CPWL_Edit::IsProceedtoOnChar(uint16_t nKeyCode, uint32_t nFlag) {
  bool bCtrl = IsPlatformShortcutKey(nFlag);
  bool bAlt = IsALTpressed(nFlag);
  if (bCtrl && !bAlt) {
    // hot keys for edit control.
    switch (nKeyCode) {
      case 'C':
      case 'V':
      case 'X':
      case 'A':
      case 'Z':
        return true;
      default:
        break;
    }
  }
  // control characters.
  switch (nKeyCode) {
    case FWL_VKEY_Escape:
    case FWL_VKEY_Back:
    case FWL_VKEY_Return:
    case FWL_VKEY_Space:
      return true;
    default:
      return false;
  }
}

bool CPWL_Edit::OnChar(uint16_t nChar, uint32_t nFlag) {
  if (m_bMouseDown)
    return true;

  bool bRC = true;
  bool bExit = false;

  if (!IsCTRLpressed(nFlag)) {
    if (m_pFillerNotify) {
      WideString swChange;

      int nSelStart;
      int nSelEnd;
      std::tie(nSelStart, nSelEnd) = GetSelection();

      switch (nChar) {
        case FWL_VKEY_Back:
          if (nSelStart == nSelEnd)
            nSelStart = nSelEnd - 1;
          break;
        case FWL_VKEY_Return:
          break;
        default:
          swChange += nChar;
          break;
      }

      ObservedPtr<CPWL_Wnd> thisObserved(this);

      WideString strChangeEx;
      std::tie(bRC, bExit) = m_pFillerNotify->OnBeforeKeyStroke(
          GetAttachedData(), swChange, strChangeEx, nSelStart, nSelEnd, true,
          nFlag);

      if (!thisObserved)
        return false;
    }
  }

  if (!bRC)
    return true;
  if (bExit)
    return false;

  if (IPVT_FontMap* pFontMap = GetFontMap()) {
    int32_t nOldCharSet = GetCharSet();
    int32_t nNewCharSet =
        pFontMap->CharSetFromUnicode(nChar, FX_CHARSET_Default);
    if (nOldCharSet != nNewCharSet) {
      SetCharSet(nNewCharSet);
    }
  }

  return OnCharInternal(nChar, nFlag);
}

bool CPWL_Edit::OnMouseWheel(uint32_t nFlag,
                             const CFX_PointF& point,
                             const CFX_Vector& delta) {
  if (!HasFlag(PES_MULTILINE))
    return false;

  CFX_PointF ptScroll = GetScrollPos();
  if (delta.y > 0)
    ptScroll.y += GetFontSize();
  else
    ptScroll.y -= GetFontSize();
  SetScrollPos(ptScroll);
  return true;
}

void CPWL_Edit::OnDestroy() {
  m_pCaret.Release();
}

bool CPWL_Edit::IsWndHorV() const {
  CFX_Matrix mt = GetWindowMatrix();
  return mt.Transform(CFX_PointF(1, 1)).y == mt.Transform(CFX_PointF(0, 1)).y;
}

void CPWL_Edit::SetCursor() {
  if (IsValid()) {
    GetSystemHandler()->SetCursor(
        IsWndHorV() ? IPWL_SystemHandler::CursorStyle::kVBeam
                    : IPWL_SystemHandler::CursorStyle::kHBeam);
  }
}

WideString CPWL_Edit::GetSelectedText() {
  return m_pEditImpl->GetSelectedText();
}

void CPWL_Edit::ReplaceSelection(const WideString& text) {
  m_pEditImpl->ReplaceSelection(text);
}

bool CPWL_Edit::SelectAllText() {
  m_pEditImpl->SelectAll();
  return true;
}

void CPWL_Edit::SetScrollInfo(const PWL_SCROLL_INFO& info) {
  if (CPWL_Wnd* pChild = GetVScrollBar())
    pChild->SetScrollInfo(info);
}

void CPWL_Edit::SetScrollPosition(float pos) {
  if (CPWL_Wnd* pChild = GetVScrollBar())
    pChild->SetScrollPosition(pos);
}

void CPWL_Edit::ScrollWindowVertically(float pos) {
  m_pEditImpl->SetScrollPos(CFX_PointF(m_pEditImpl->GetScrollPos().x, pos));
}

void CPWL_Edit::CreateChildWnd(const CreateParams& cp) {
  if (!IsReadOnly())
    CreateEditCaret(cp);
}

void CPWL_Edit::CreateEditCaret(const CreateParams& cp) {
  if (m_pCaret)
    return;

  CreateParams ecp = cp;
  ecp.dwFlags = PWS_NOREFRESHCLIP;
  ecp.dwBorderWidth = 0;
  ecp.nBorderStyle = BorderStyle::kSolid;
  ecp.rcRectWnd = CFX_FloatRect();

  auto pCaret = std::make_unique<CPWL_Caret>(ecp, CloneAttachedData());
  m_pCaret = pCaret.get();
  m_pCaret->SetInvalidRect(GetClientRect());
  AddChild(std::move(pCaret));
  m_pCaret->Realize();
}

void CPWL_Edit::SetFontSize(float fFontSize) {
  m_pEditImpl->SetFontSizeAndPaint(fFontSize);
}

float CPWL_Edit::GetFontSize() const {
  return m_pEditImpl->GetFontSize();
}

bool CPWL_Edit::OnKeyDownInternal(uint16_t nChar, uint32_t nFlag) {
  if (m_bMouseDown)
    return true;

  bool bRet = CPWL_Wnd::OnKeyDown(nChar, nFlag);

  // FILTER
  switch (nChar) {
    default:
      return false;
    case FWL_VKEY_Delete:
    case FWL_VKEY_Up:
    case FWL_VKEY_Down:
    case FWL_VKEY_Left:
    case FWL_VKEY_Right:
    case FWL_VKEY_Home:
    case FWL_VKEY_End:
    case FWL_VKEY_Insert:
    case 'C':
    case 'V':
    case 'X':
    case 'A':
    case 'Z':
    case 'c':
    case 'v':
    case 'x':
    case 'a':
    case 'z':
      break;
  }

  if (nChar == FWL_VKEY_Delete && m_pEditImpl->IsSelected())
    nChar = FWL_VKEY_Unknown;

  switch (nChar) {
    case FWL_VKEY_Delete:
      Delete();
      return true;
    case FWL_VKEY_Insert:
      if (IsSHIFTpressed(nFlag))
        PasteText();
      return true;
    case FWL_VKEY_Up:
      m_pEditImpl->OnVK_UP(IsSHIFTpressed(nFlag), false);
      return true;
    case FWL_VKEY_Down:
      m_pEditImpl->OnVK_DOWN(IsSHIFTpressed(nFlag), false);
      return true;
    case FWL_VKEY_Left:
      m_pEditImpl->OnVK_LEFT(IsSHIFTpressed(nFlag), false);
      return true;
    case FWL_VKEY_Right:
      m_pEditImpl->OnVK_RIGHT(IsSHIFTpressed(nFlag), false);
      return true;
    case FWL_VKEY_Home:
      m_pEditImpl->OnVK_HOME(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
      return true;
    case FWL_VKEY_End:
      m_pEditImpl->OnVK_END(IsSHIFTpressed(nFlag), IsCTRLpressed(nFlag));
      return true;
    case FWL_VKEY_Unknown:
      if (!IsSHIFTpressed(nFlag))
        ClearSelection();
      else
        CutText();
      return true;
    default:
      break;
  }

  return bRet;
}

bool CPWL_Edit::OnCharInternal(uint16_t nChar, uint32_t nFlag) {
  if (m_bMouseDown)
    return true;

  CPWL_Wnd::OnChar(nChar, nFlag);

  // FILTER
  switch (nChar) {
    case 0x0A:
    case 0x1B:
      return false;
    default:
      break;
  }

  bool bCtrl = IsPlatformShortcutKey(nFlag);
  bool bAlt = IsALTpressed(nFlag);
  bool bShift = IsSHIFTpressed(nFlag);

  uint16_t word = nChar;

  if (bCtrl && !bAlt) {
    switch (nChar) {
      case 'C' - 'A' + 1:
        CopyText();
        return true;
      case 'V' - 'A' + 1:
        PasteText();
        return true;
      case 'X' - 'A' + 1:
        CutText();
        return true;
      case 'A' - 'A' + 1:
        SelectAllText();
        return true;
      case 'Z' - 'A' + 1:
        if (bShift)
          Redo();
        else
          Undo();
        return true;
      default:
        if (nChar < 32)
          return false;
    }
  }

  if (IsReadOnly())
    return true;

  if (m_pEditImpl->IsSelected() && word == FWL_VKEY_Back)
    word = FWL_VKEY_Unknown;

  ClearSelection();

  switch (word) {
    case FWL_VKEY_Back:
      Backspace();
      break;
    case FWL_VKEY_Return:
      InsertReturn();
      break;
    case FWL_VKEY_Unknown:
      break;
    default:
      InsertWord(word, GetCharSet());
      break;
  }

  return true;
}

bool CPWL_Edit::OnLButtonDown(uint32_t nFlag, const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonDown(nFlag, point);
  if (HasFlag(PES_TEXTOVERFLOW) || ClientHitTest(point)) {
    if (m_bMouseDown && !InvalidateRect(nullptr))
      return true;

    m_bMouseDown = true;
    SetCapture();
    m_pEditImpl->OnMouseDown(point, IsSHIFTpressed(nFlag),
                             IsCTRLpressed(nFlag));
  }
  return true;
}

bool CPWL_Edit::OnLButtonUp(uint32_t nFlag, const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonUp(nFlag, point);
  if (m_bMouseDown) {
    // can receive keybord message
    if (ClientHitTest(point) && !IsFocused())
      SetFocus();

    ReleaseCapture();
    m_bMouseDown = false;
  }
  return true;
}

bool CPWL_Edit::OnLButtonDblClk(uint32_t nFlag, const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonDblClk(nFlag, point);
  if (HasFlag(PES_TEXTOVERFLOW) || ClientHitTest(point))
    m_pEditImpl->SelectAll();

  return true;
}

bool CPWL_Edit::OnRButtonUp(uint32_t nFlag, const CFX_PointF& point) {
  if (m_bMouseDown)
    return false;

  CPWL_Wnd::OnRButtonUp(nFlag, point);
  if (!HasFlag(PES_TEXTOVERFLOW) && !ClientHitTest(point))
    return true;

  SetFocus();
  return false;
}

bool CPWL_Edit::OnMouseMove(uint32_t nFlag, const CFX_PointF& point) {
  CPWL_Wnd::OnMouseMove(nFlag, point);

  if (m_bMouseDown)
    m_pEditImpl->OnMouseMove(point, false, false);

  return true;
}

void CPWL_Edit::SetEditCaret(bool bVisible) {
  CFX_PointF ptHead;
  CFX_PointF ptFoot;
  if (bVisible)
    GetCaretInfo(&ptHead, &ptFoot);

  SetCaret(bVisible, ptHead, ptFoot);
  // Note, |this| may no longer be viable at this point. If more work needs to
  // be done, check the return value of SetCaret().
}

void CPWL_Edit::GetCaretInfo(CFX_PointF* ptHead, CFX_PointF* ptFoot) const {
  CPWL_EditImpl::Iterator* pIterator = m_pEditImpl->GetIterator();
  pIterator->SetAt(m_pEditImpl->GetCaret());
  CPVT_Word word;
  CPVT_Line line;
  if (pIterator->GetWord(word)) {
    ptHead->x = word.ptWord.x + word.fWidth;
    ptHead->y = word.ptWord.y + word.fAscent;
    ptFoot->x = word.ptWord.x + word.fWidth;
    ptFoot->y = word.ptWord.y + word.fDescent;
  } else if (pIterator->GetLine(line)) {
    ptHead->x = line.ptLine.x;
    ptHead->y = line.ptLine.y + line.fLineAscent;
    ptFoot->x = line.ptLine.x;
    ptFoot->y = line.ptLine.y + line.fLineDescent;
  }
}

bool CPWL_Edit::SetCaret(bool bVisible,
                         const CFX_PointF& ptHead,
                         const CFX_PointF& ptFoot) {
  if (!m_pCaret)
    return true;

  if (!IsFocused() || m_pEditImpl->IsSelected())
    bVisible = false;

  ObservedPtr<CPWL_Edit> thisObserved(this);
  m_pCaret->SetCaret(bVisible, ptHead, ptFoot);
  if (!thisObserved)
    return false;

  return true;
}

WideString CPWL_Edit::GetText() {
  return m_pEditImpl->GetText();
}

void CPWL_Edit::SetSelection(int32_t nStartChar, int32_t nEndChar) {
  m_pEditImpl->SetSelection(nStartChar, nEndChar);
}

std::pair<int32_t, int32_t> CPWL_Edit::GetSelection() const {
  return m_pEditImpl->GetSelection();
}

void CPWL_Edit::ClearSelection() {
  if (!IsReadOnly())
    m_pEditImpl->ClearSelection();
}

void CPWL_Edit::SetScrollPos(const CFX_PointF& point) {
  m_pEditImpl->SetScrollPos(point);
}

CFX_PointF CPWL_Edit::GetScrollPos() const {
  return m_pEditImpl->GetScrollPos();
}

void CPWL_Edit::CopyText() {}

void CPWL_Edit::PasteText() {}

void CPWL_Edit::InsertWord(uint16_t word, int32_t nCharset) {
  if (!IsReadOnly())
    m_pEditImpl->InsertWord(word, nCharset);
}

void CPWL_Edit::InsertReturn() {
  if (!IsReadOnly())
    m_pEditImpl->InsertReturn();
}

void CPWL_Edit::Delete() {
  if (!IsReadOnly())
    m_pEditImpl->Delete();
}

void CPWL_Edit::Backspace() {
  if (!IsReadOnly())
    m_pEditImpl->Backspace();
}

bool CPWL_Edit::CanUndo() {
  return !IsReadOnly() && m_pEditImpl->CanUndo();
}

bool CPWL_Edit::CanRedo() {
  return !IsReadOnly() && m_pEditImpl->CanRedo();
}

bool CPWL_Edit::Undo() {
  return CanUndo() && m_pEditImpl->Undo();
}

bool CPWL_Edit::Redo() {
  return CanRedo() && m_pEditImpl->Redo();
}

void CPWL_Edit::SetReadyToInput() {
  if (m_bMouseDown) {
    ReleaseCapture();
    m_bMouseDown = false;
  }
}
