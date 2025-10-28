// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_edit.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <utility>

#include "constants/ascii.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfdoc/cpvt_word.h"
#include "core/fpdfdoc/ipvt_fontmap.h"
#include "core/fxcrt/check.h"
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

CPWL_Edit::CPWL_Edit(
    const CreateParams& cp,
    std::unique_ptr<IPWL_FillerNotify::PerWindowData> pAttachedData)
    : CPWL_Wnd(cp, std::move(pAttachedData)),
      edit_impl_(std::make_unique<CPWL_EditImpl>()) {
  GetCreationParams()->eCursorType = IPWL_FillerNotify::CursorStyle::kVBeam;
}

CPWL_Edit::~CPWL_Edit() {
  DCHECK(!focus_);
}

void CPWL_Edit::SetText(const WideString& csText) {
  edit_impl_->SetText(csText);
  edit_impl_->Paint();
}

bool CPWL_Edit::RepositionChildWnd() {
  ObservedPtr<CPWL_Edit> this_observed(this);
  if (CPWL_ScrollBar* pVSB = this_observed->GetVScrollBar()) {
    CFX_FloatRect rcWindow = this_observed->old_window_rect_;
    CFX_FloatRect rcVScroll =
        CFX_FloatRect(rcWindow.right, rcWindow.bottom,
                      rcWindow.right + CPWL_ScrollBar::kWidth, rcWindow.top);
    pVSB->Move(rcVScroll, true, false);
    if (!this_observed) {
      return false;
    }
  }
  if (this_observed->caret_ && !HasFlag(PES_TEXTOVERFLOW)) {
    CFX_FloatRect rect = this_observed->GetClientRect();
    if (!rect.IsEmpty()) {
      // +1 for caret beside border
      rect.Inflate(1.0f, 1.0f);
      rect.Normalize();
    }
    this_observed->caret_->SetClipRect(rect);
  }
  this_observed->edit_impl_->SetPlateRect(GetClientRect());
  this_observed->edit_impl_->Paint();
  return true;
}

CFX_FloatRect CPWL_Edit::GetClientRect() const {
  float width = static_cast<float>(GetBorderWidth() + GetInnerBorderWidth());
  CFX_FloatRect rcClient = GetWindowRect().GetDeflated(width, width);
  CPWL_ScrollBar* pVSB = GetVScrollBar();
  if (pVSB && pVSB->IsVisible()) {
    rcClient.right -= CPWL_ScrollBar::kWidth;
  }
  return rcClient;
}

void CPWL_Edit::SetAlignFormatVerticalCenter() {
  edit_impl_->SetAlignmentV(static_cast<int32_t>(PEAV_CENTER));
  edit_impl_->Paint();
}

bool CPWL_Edit::CanSelectAll() const {
  return GetSelectWordRange() != edit_impl_->GetWholeWordRange();
}

void CPWL_Edit::OnCreated() {
  SetFontSize(GetCreationParams()->fFontSize);
  edit_impl_->SetFontMap(GetFontMap());
  edit_impl_->SetNotify(this);
  edit_impl_->Initialize();

  if (CPWL_ScrollBar* pScroll = GetVScrollBar()) {
    pScroll->RemoveFlag(PWS_AUTOTRANSPARENT);
    pScroll->SetTransparency(255);
  }

  SetParamByFlag();
  old_window_rect_ = GetWindowRect();
}

void CPWL_Edit::SetParamByFlag() {
  if (HasFlag(PES_RIGHT)) {
    edit_impl_->SetAlignmentH(2);
  } else if (HasFlag(PES_MIDDLE)) {
    edit_impl_->SetAlignmentH(1);
  } else {
    edit_impl_->SetAlignmentH(0);
  }

  if (HasFlag(PES_CENTER)) {
    edit_impl_->SetAlignmentV(1);
  } else {
    edit_impl_->SetAlignmentV(0);
  }

  if (HasFlag(PES_PASSWORD)) {
    edit_impl_->SetPasswordChar('*');
  }

  edit_impl_->SetMultiLine(HasFlag(PES_MULTILINE));
  edit_impl_->SetAutoReturn(HasFlag(PES_AUTORETURN));
  edit_impl_->SetAutoFontSize(HasFlag(PWS_AUTOFONTSIZE));
  edit_impl_->SetAutoScroll(HasFlag(PES_AUTOSCROLL));
  edit_impl_->EnableUndo(HasFlag(PES_UNDO));

  if (HasFlag(PES_TEXTOVERFLOW)) {
    SetClipRect(CFX_FloatRect());
    edit_impl_->SetTextOverflow(true);
  } else {
    if (caret_) {
      CFX_FloatRect rect = GetClientRect();
      if (!rect.IsEmpty()) {
        // +1 for caret beside border
        rect.Inflate(1.0f, 1.0f);
        rect.Normalize();
      }
      caret_->SetClipRect(rect);
    }
  }
}

void CPWL_Edit::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                   const CFX_Matrix& mtUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, mtUser2Device);

  const CFX_FloatRect rcClient = GetClientRect();
  const BorderStyle border_style = GetBorderStyle();
  const int32_t nCharArray = edit_impl_->GetCharArray();
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
    gsd.set_line_width(GetBorderWidth());
    if (border_style == BorderStyle::kDash) {
      gsd.set_dash_array({static_cast<float>(GetBorderDash().nDash),
                          static_cast<float>(GetBorderDash().nGap)});
      gsd.set_dash_phase(GetBorderDash().nPhase);
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
      pDevice->DrawPath(path, &mtUser2Device, &gsd, 0,
                        GetBorderColor().ToFXColor(255),
                        CFX_FillRenderOptions::EvenOddOptions());
    }
  }

  CFX_FloatRect rcClip;
  CPVT_WordRange wrRange = edit_impl_->GetVisibleWordRange();
  CPVT_WordRange* pRange = nullptr;
  if (!HasFlag(PES_TEXTOVERFLOW)) {
    rcClip = GetClientRect();
    pRange = &wrRange;
  }
  edit_impl_->DrawEdit(
      pDevice, mtUser2Device, GetTextColor().ToFXColor(GetTransparency()),
      rcClip, CFX_PointF(), pRange, GetFillerNotify(), GetAttachedData());
}

void CPWL_Edit::OnSetFocus() {
  ObservedPtr<CPWL_Edit> this_observed(this);
  this_observed->SetEditCaret(true);
  if (!this_observed) {
    return;
  }
  if (!this_observed->IsReadOnly()) {
    CPWL_Wnd::ProviderIface* pProvider = this_observed->GetProvider();
    if (pProvider) {
      pProvider->OnSetFocusForEdit(this);
      if (!this_observed) {
        return;
      }
    }
  }
  this_observed->focus_ = true;
}

void CPWL_Edit::OnKillFocus() {
  ObservedPtr<CPWL_Edit> this_observed(this);
  CPWL_ScrollBar* pScroll = this_observed->GetVScrollBar();
  if (pScroll && pScroll->IsVisible()) {
    if (!pScroll->SetVisible(false)) {
      return;
    }
    if (!this_observed) {
      return;
    }
    if (!this_observed->Move(this_observed->old_window_rect_, true, true)) {
      return;
    }
  }
  this_observed->edit_impl_->SelectNone();
  if (!this_observed) {
    return;
  }
  if (!this_observed->SetCaret(false, CFX_PointF(), CFX_PointF())) {
    return;
  }
  this_observed->SetCharSet(FX_Charset::kANSI);
  this_observed->focus_ = false;
}

CPVT_WordRange CPWL_Edit::GetSelectWordRange() const {
  if (!edit_impl_->IsSelected()) {
    return CPVT_WordRange();
  }

  auto [nStart, nEnd] = edit_impl_->GetSelection();

  CPVT_WordPlace wpStart = edit_impl_->WordIndexToWordPlace(nStart);
  CPVT_WordPlace wpEnd = edit_impl_->WordIndexToWordPlace(nEnd);
  return CPVT_WordRange(wpStart, wpEnd);
}

bool CPWL_Edit::IsTextFull() const {
  return edit_impl_->IsTextFull();
}

float CPWL_Edit::GetCharArrayAutoFontSize(const CPDF_Font* font,
                                          const CFX_FloatRect& rcPlate,
                                          int32_t nCharArray) {
  if (!font || font->IsStandardFont()) {
    return 0.0f;
  }

  const FX_RECT& rcBBox = font->GetFontBBox();

  CFX_FloatRect rcCell = rcPlate;
  float xdiv = rcCell.Width() / nCharArray * 1000.0f / rcBBox.Width();
  float ydiv = -rcCell.Height() * 1000.0f / rcBBox.Height();

  return xdiv < ydiv ? xdiv : ydiv;
}

void CPWL_Edit::SetCharArray(int32_t nCharArray) {
  if (!HasFlag(PES_CHARARRAY) || nCharArray <= 0) {
    return;
  }

  edit_impl_->SetCharArray(nCharArray);
  edit_impl_->SetTextOverflow(true);
  edit_impl_->Paint();

  if (!HasFlag(PWS_AUTOFONTSIZE)) {
    return;
  }

  IPVT_FontMap* font_map = GetFontMap();
  if (!font_map) {
    return;
  }

  float fFontSize = GetCharArrayAutoFontSize(font_map->GetPDFFont(0).Get(),
                                             GetClientRect(), nCharArray);
  if (fFontSize <= 0.0f) {
    return;
  }

  edit_impl_->SetAutoFontSize(false);
  edit_impl_->SetFontSize(fFontSize);
  edit_impl_->Paint();
}

void CPWL_Edit::SetLimitChar(int32_t nLimitChar) {
  edit_impl_->SetLimitChar(nLimitChar);
  edit_impl_->Paint();
}

CFX_FloatRect CPWL_Edit::GetFocusRect() const {
  return CFX_FloatRect();
}

bool CPWL_Edit::IsVScrollBarVisible() const {
  CPWL_ScrollBar* pScroll = GetVScrollBar();
  return pScroll && pScroll->IsVisible();
}

bool CPWL_Edit::OnKeyDown(FWL_VKEYCODE nKeyCode, Mask<FWL_EVENTFLAG> nFlag) {
  ObservedPtr<CPWL_Edit> this_observed(this);
  if (this_observed->mouse_down_) {
    return true;
  }
  if (nKeyCode == FWL_VKEY_Delete) {
    WideString strChange;
    WideString strChangeEx;
    auto [nSelStart, nSelEnd] = this_observed->GetSelection();
    if (nSelStart == nSelEnd) {
      nSelEnd = nSelStart + 1;
    }
    IPWL_FillerNotify::BeforeKeystrokeResult result =
        this_observed->GetFillerNotify()->OnBeforeKeyStroke(
            this_observed->GetAttachedData(), strChange, strChangeEx, nSelStart,
            nSelEnd, true, nFlag);

    if (!this_observed) {
      return false;
    }
    if (!result.rc) {
      return false;
    }
    if (result.exit) {
      return false;
    }
  }

  return this_observed->OnKeyDownInternal(nKeyCode, nFlag);
}

bool CPWL_Edit::OnChar(uint16_t nChar, Mask<FWL_EVENTFLAG> nFlag) {
  ObservedPtr<CPWL_Edit> this_observed(this);
  if (this_observed->mouse_down_) {
    return true;
  }
  if (!this_observed->IsCTRLKeyDown(nFlag)) {
    WideString swChange;
    auto [nSelStart, nSelEnd] = this_observed->GetSelection();
    switch (nChar) {
      case pdfium::ascii::kBackspace:
        if (nSelStart == nSelEnd) {
          nSelStart = nSelEnd - 1;
        }
        break;
      case pdfium::ascii::kReturn:
        break;
      default:
        swChange += nChar;
        break;
    }
    WideString strChangeEx;
    IPWL_FillerNotify::BeforeKeystrokeResult result =
        this_observed->GetFillerNotify()->OnBeforeKeyStroke(
            this_observed->GetAttachedData(), swChange, strChangeEx, nSelStart,
            nSelEnd, true, nFlag);

    if (!this_observed) {
      return false;
    }
    if (!result.rc) {
      return true;
    }
    if (result.exit) {
      return false;
    }
  }
  if (IPVT_FontMap* font_map = this_observed->GetFontMap()) {
    FX_Charset nOldCharSet = this_observed->GetCharSet();
    FX_Charset nNewCharSet =
        font_map->CharSetFromUnicode(nChar, FX_Charset::kDefault);
    if (nOldCharSet != nNewCharSet) {
      this_observed->SetCharSet(nNewCharSet);
    }
  }
  return this_observed->OnCharInternal(nChar, nFlag);
}

bool CPWL_Edit::OnMouseWheel(Mask<FWL_EVENTFLAG> nFlag,
                             const CFX_PointF& point,
                             const CFX_Vector& delta) {
  if (!HasFlag(PES_MULTILINE)) {
    return false;
  }

  CFX_PointF ptScroll = GetScrollPos();
  if (delta.y > 0) {
    ptScroll.y += GetFontSize();
  } else {
    ptScroll.y -= GetFontSize();
  }
  SetScrollPos(ptScroll);
  return true;
}

void CPWL_Edit::OnDestroy() {
  caret_.ExtractAsDangling();
}

bool CPWL_Edit::IsWndHorV() const {
  CFX_Matrix mt = GetWindowMatrix();
  return mt.Transform(CFX_PointF(1, 1)).y == mt.Transform(CFX_PointF(0, 1)).y;
}

void CPWL_Edit::SetCursor() {
  if (IsValid()) {
    GetFillerNotify()->SetCursor(IsWndHorV()
                                     ? IPWL_FillerNotify::CursorStyle::kVBeam
                                     : IPWL_FillerNotify::CursorStyle::kHBeam);
  }
}

WideString CPWL_Edit::GetSelectedText() {
  return edit_impl_->GetSelectedText();
}

void CPWL_Edit::ReplaceAndKeepSelection(const WideString& text) {
  edit_impl_->ReplaceAndKeepSelection(text);
}

void CPWL_Edit::ReplaceSelection(const WideString& text) {
  edit_impl_->ReplaceSelection(text);
}

bool CPWL_Edit::SelectAllText() {
  edit_impl_->SelectAll();
  return true;
}

void CPWL_Edit::SetScrollInfo(const PWL_SCROLL_INFO& info) {
  if (CPWL_Wnd* pChild = GetVScrollBar()) {
    pChild->SetScrollInfo(info);
  }
}

void CPWL_Edit::SetScrollPosition(float pos) {
  if (CPWL_Wnd* pChild = GetVScrollBar()) {
    pChild->SetScrollPosition(pos);
  }
}

void CPWL_Edit::ScrollWindowVertically(float pos) {
  edit_impl_->SetScrollPos(CFX_PointF(edit_impl_->GetScrollPos().x, pos));
}

void CPWL_Edit::CreateChildWnd(const CreateParams& cp) {
  if (!IsReadOnly()) {
    CreateEditCaret(cp);
  }
}

void CPWL_Edit::CreateEditCaret(const CreateParams& cp) {
  if (caret_) {
    return;
  }

  CreateParams ecp = cp;
  ecp.dwFlags = PWS_NOREFRESHCLIP;
  ecp.dwBorderWidth = 0;
  ecp.nBorderStyle = BorderStyle::kSolid;
  ecp.rcRectWnd = CFX_FloatRect();

  auto pCaret = std::make_unique<CPWL_Caret>(ecp, CloneAttachedData());
  caret_ = pCaret.get();
  caret_->SetInvalidRect(GetClientRect());
  AddChild(std::move(pCaret));
  caret_->Realize();
}

void CPWL_Edit::SetFontSize(float fFontSize) {
  edit_impl_->SetFontSize(fFontSize);
  edit_impl_->Paint();
}

float CPWL_Edit::GetFontSize() const {
  return edit_impl_->GetFontSize();
}

bool CPWL_Edit::OnKeyDownInternal(FWL_VKEYCODE nKeyCode,
                                  Mask<FWL_EVENTFLAG> nFlag) {
  if (mouse_down_) {
    return true;
  }

  CPWL_Wnd::OnKeyDown(nKeyCode, nFlag);

  if (nKeyCode == FWL_VKEY_Delete && edit_impl_->IsSelected()) {
    nKeyCode = FWL_VKEY_Unknown;
  }

  switch (nKeyCode) {
    case FWL_VKEY_Delete:
      Delete();
      return true;
    case FWL_VKEY_Up:
      edit_impl_->OnVK_UP(IsSHIFTKeyDown(nFlag));
      return true;
    case FWL_VKEY_Down:
      edit_impl_->OnVK_DOWN(IsSHIFTKeyDown(nFlag));
      return true;
    case FWL_VKEY_Left:
      edit_impl_->OnVK_LEFT(IsSHIFTKeyDown(nFlag));
      return true;
    case FWL_VKEY_Right:
      edit_impl_->OnVK_RIGHT(IsSHIFTKeyDown(nFlag));
      return true;
    case FWL_VKEY_Home:
      edit_impl_->OnVK_HOME(IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
      return true;
    case FWL_VKEY_End:
      edit_impl_->OnVK_END(IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
      return true;
    case FWL_VKEY_Unknown:
      ClearSelection();
      return true;
    default:
      return false;
  }
}

bool CPWL_Edit::OnCharInternal(uint16_t nChar, Mask<FWL_EVENTFLAG> nFlag) {
  if (mouse_down_) {
    return true;
  }

  CPWL_Wnd::OnChar(nChar, nFlag);

  // FILTER
  switch (nChar) {
    case pdfium::ascii::kNewline:
    case pdfium::ascii::kEscape:
      return false;
    default:
      break;
  }

  bool bCtrl = IsPlatformShortcutKey(nFlag);
  bool bAlt = IsALTKeyDown(nFlag);
  bool bShift = IsSHIFTKeyDown(nFlag);

  uint16_t word = nChar;

  if (bCtrl && !bAlt) {
    switch (nChar) {
      case pdfium::ascii::kControlA:
        SelectAllText();
        return true;
      case pdfium::ascii::kControlZ:
        if (bShift) {
          Redo();
        } else {
          Undo();
        }
        return true;
      default:
        if (nChar < 32) {
          return false;
        }
    }
  }

  if (IsReadOnly()) {
    return true;
  }

  edit_impl_->TypeChar(word, GetCharSet());
  return true;
}

bool CPWL_Edit::OnLButtonDown(Mask<FWL_EVENTFLAG> nFlag,
                              const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonDown(nFlag, point);
  if (HasFlag(PES_TEXTOVERFLOW) || ClientHitTest(point)) {
    if (mouse_down_ && !InvalidateRect(nullptr)) {
      return true;
    }

    mouse_down_ = true;
    SetCapture();
    edit_impl_->OnMouseDown(point, IsSHIFTKeyDown(nFlag), IsCTRLKeyDown(nFlag));
  }
  return true;
}

bool CPWL_Edit::OnLButtonUp(Mask<FWL_EVENTFLAG> nFlag,
                            const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonUp(nFlag, point);
  if (mouse_down_) {
    // can receive keybord message
    if (ClientHitTest(point) && !IsFocused()) {
      SetFocus();
    }

    ReleaseCapture();
    mouse_down_ = false;
  }
  return true;
}

bool CPWL_Edit::OnLButtonDblClk(Mask<FWL_EVENTFLAG> nFlag,
                                const CFX_PointF& point) {
  CPWL_Wnd::OnLButtonDblClk(nFlag, point);
  if (HasFlag(PES_TEXTOVERFLOW) || ClientHitTest(point)) {
    edit_impl_->SelectAll();
  }

  return true;
}

bool CPWL_Edit::OnRButtonUp(Mask<FWL_EVENTFLAG> nFlag,
                            const CFX_PointF& point) {
  if (mouse_down_) {
    return false;
  }

  CPWL_Wnd::OnRButtonUp(nFlag, point);
  if (!HasFlag(PES_TEXTOVERFLOW) && !ClientHitTest(point)) {
    return true;
  }

  SetFocus();
  return false;
}

bool CPWL_Edit::OnMouseMove(Mask<FWL_EVENTFLAG> nFlag,
                            const CFX_PointF& point) {
  CPWL_Wnd::OnMouseMove(nFlag, point);

  if (mouse_down_) {
    edit_impl_->OnMouseMove(point, false, false);
  }

  return true;
}

void CPWL_Edit::SetEditCaret(bool bVisible) {
  CFX_PointF ptHead;
  CFX_PointF ptFoot;
  if (bVisible) {
    GetCaretInfo(&ptHead, &ptFoot);
  }

  SetCaret(bVisible, ptHead, ptFoot);
  // Note, |this| may no longer be viable at this point. If more work needs to
  // be done, check the return value of SetCaret().
}

void CPWL_Edit::GetCaretInfo(CFX_PointF* ptHead, CFX_PointF* ptFoot) const {
  CPWL_EditImpl::Iterator* pIterator = edit_impl_->GetIterator();
  pIterator->SetAt(edit_impl_->GetCaret());
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
  ObservedPtr<CPWL_Edit> this_observed(this);
  if (!this_observed->caret_) {
    return true;
  }
  if (!this_observed->IsFocused() || this_observed->edit_impl_->IsSelected()) {
    bVisible = false;
  }
  this_observed->caret_->SetCaret(bVisible, ptHead, ptFoot);
  return !!this_observed;
}

WideString CPWL_Edit::GetText() {
  return edit_impl_->GetText();
}

void CPWL_Edit::SetSelection(int32_t nStartChar, int32_t nEndChar) {
  edit_impl_->SetSelection(nStartChar, nEndChar);
}

std::pair<int32_t, int32_t> CPWL_Edit::GetSelection() const {
  return edit_impl_->GetSelection();
}

void CPWL_Edit::ClearSelection() {
  if (!IsReadOnly()) {
    edit_impl_->ClearSelection();
  }
}

void CPWL_Edit::SetScrollPos(const CFX_PointF& point) {
  edit_impl_->SetScrollPos(point);
}

CFX_PointF CPWL_Edit::GetScrollPos() const {
  return edit_impl_->GetScrollPos();
}

void CPWL_Edit::Delete() {
  if (!IsReadOnly()) {
    edit_impl_->Delete();
  }
}

bool CPWL_Edit::CanUndo() {
  return !IsReadOnly() && edit_impl_->CanUndo();
}

bool CPWL_Edit::CanRedo() {
  return !IsReadOnly() && edit_impl_->CanRedo();
}

bool CPWL_Edit::Undo() {
  return CanUndo() && edit_impl_->Undo();
}

bool CPWL_Edit::Redo() {
  return CanRedo() && edit_impl_->Redo();
}

void CPWL_Edit::SetMaxUndoItemsForTest(size_t items) {
  edit_impl_->SetMaxUndoItemsForTest(items);
}

void CPWL_Edit::SetReadyToInput() {
  if (mouse_down_) {
    ReleaseCapture();
    mouse_down_ = false;
  }
}
