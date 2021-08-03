// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_edit.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/text_char_pos.h"
#include "third_party/base/check.h"
#include "third_party/base/cxx17_backports.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_caret.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_eventtextwillchange.h"
#include "xfa/fwl/cfwl_eventvalidate.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fwl/fwl_widgetdef.h"
#include "xfa/fwl/ifwl_themeprovider.h"
#include "xfa/fwl/theme/cfwl_utils.h"

namespace {

constexpr int kEditMargin = 3;

#if defined(OS_APPLE)
constexpr FWL_KeyFlagMask kEditingModifier = FWL_KEYFLAG_Command;
#else
constexpr FWL_KeyFlagMask kEditingModifier = FWL_KEYFLAG_Ctrl;
#endif

}  // namespace

CFWL_Edit::CFWL_Edit(CFWL_App* app,
                     const Properties& properties,
                     CFWL_Widget* pOuter)
    : CFWL_Widget(app, properties, pOuter),
      m_pEditEngine(std::make_unique<CFDE_TextEditEngine>()) {
  m_pEditEngine->SetDelegate(this);
}

CFWL_Edit::~CFWL_Edit() = default;

void CFWL_Edit::PreFinalize() {
  m_pEditEngine->SetDelegate(nullptr);
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Focused)
    HideCaret(nullptr);
  CFWL_Widget::PreFinalize();
}

void CFWL_Edit::Trace(cppgc::Visitor* visitor) const {
  CFWL_Widget::Trace(visitor);
  visitor->Trace(m_pVertScrollBar);
  visitor->Trace(m_pCaret);
}

FWL_Type CFWL_Edit::GetClassID() const {
  return FWL_Type::Edit;
}

CFX_RectF CFWL_Edit::GetWidgetRect() {
  CFX_RectF rect = m_WidgetRect;
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_OuterScrollbar) {
    float scrollbarWidth = GetThemeProvider()->GetScrollBarWidth();
    if (IsShowVertScrollBar()) {
      rect.width += scrollbarWidth;
      rect.width += kEditMargin;
    }
  }
  return rect;
}

CFX_RectF CFWL_Edit::GetAutosizedWidgetRect() {
  CFX_RectF rect;
  if (m_pEditEngine->GetLength() > 0) {
    CFX_SizeF size = CalcTextSize(
        m_pEditEngine->GetText(),
        !!(m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_MultiLine));
    rect = CFX_RectF(0, 0, size);
  }
  InflateWidgetRect(rect);
  return rect;
}

void CFWL_Edit::SetStates(uint32_t dwStates) {
  if ((m_Properties.m_dwStates & FWL_STATE_WGT_Invisible) ||
      (m_Properties.m_dwStates & FWL_STATE_WGT_Disabled)) {
    HideCaret(nullptr);
  }
  CFWL_Widget::SetStates(dwStates);
}

void CFWL_Edit::Update() {
  if (IsLocked())
    return;

  Layout();
  if (m_ClientRect.IsEmpty())
    return;

  UpdateEditEngine();
  UpdateVAlignment();
  UpdateScroll();
  InitCaret();
}

FWL_WidgetHit CFWL_Edit::HitTest(const CFX_PointF& point) {
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_OuterScrollbar) {
    if (IsShowVertScrollBar()) {
      if (m_pVertScrollBar->GetWidgetRect().Contains(point))
        return FWL_WidgetHit::VScrollBar;
    }
  }
  if (m_ClientRect.Contains(point))
    return FWL_WidgetHit::Edit;
  return FWL_WidgetHit::Unknown;
}

void CFWL_Edit::DrawWidget(CFGAS_GEGraphics* pGraphics,
                           const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;

  if (m_ClientRect.IsEmpty())
    return;

  DrawContent(pGraphics, matrix);
  if (HasBorder())
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);
}

void CFWL_Edit::SetText(const WideString& wsText) {
  m_pEditEngine->Clear();
  m_pEditEngine->Insert(0, wsText,
                        CFDE_TextEditEngine::RecordOperation::kInsertRecord);
}

void CFWL_Edit::SetTextSkipNotify(const WideString& wsText) {
  m_pEditEngine->Clear();
  m_pEditEngine->Insert(0, wsText,
                        CFDE_TextEditEngine::RecordOperation::kSkipNotify);
}

int32_t CFWL_Edit::GetTextLength() const {
  return m_pEditEngine->GetLength();
}

WideString CFWL_Edit::GetText() const {
  return m_pEditEngine->GetText();
}

void CFWL_Edit::ClearText() {
  m_pEditEngine->Clear();
}

void CFWL_Edit::SelectAll() {
  m_pEditEngine->SelectAll();
}

bool CFWL_Edit::HasSelection() const {
  return m_pEditEngine->HasSelection();
}

std::pair<size_t, size_t> CFWL_Edit::GetSelection() const {
  return m_pEditEngine->GetSelection();
}

void CFWL_Edit::ClearSelection() {
  return m_pEditEngine->ClearSelection();
}

int32_t CFWL_Edit::GetLimit() const {
  return m_nLimit;
}

void CFWL_Edit::SetLimit(int32_t nLimit) {
  m_nLimit = nLimit;

  if (m_nLimit > 0) {
    m_pEditEngine->SetHasCharacterLimit(true);
    m_pEditEngine->SetCharacterLimit(nLimit);
  } else {
    m_pEditEngine->SetHasCharacterLimit(false);
  }
}

void CFWL_Edit::SetAliasChar(wchar_t wAlias) {
  m_pEditEngine->SetAliasChar(wAlias);
}

Optional<WideString> CFWL_Edit::Copy() {
  if (!m_pEditEngine->HasSelection())
    return pdfium::nullopt;

  return m_pEditEngine->GetSelectedText();
}

Optional<WideString> CFWL_Edit::Cut() {
  if (!m_pEditEngine->HasSelection())
    return pdfium::nullopt;

  WideString cut_text = m_pEditEngine->DeleteSelectedText();
  UpdateCaret();
  return cut_text;
}

bool CFWL_Edit::Paste(const WideString& wsPaste) {
  if (m_pEditEngine->HasSelection())
    m_pEditEngine->ReplaceSelectedText(wsPaste);
  else
    m_pEditEngine->Insert(m_CursorPosition, wsPaste);

  return true;
}

bool CFWL_Edit::Undo() {
  return CanUndo() && m_pEditEngine->Undo();
}

bool CFWL_Edit::Redo() {
  return CanRedo() && m_pEditEngine->Redo();
}

bool CFWL_Edit::CanUndo() {
  return m_pEditEngine->CanUndo();
}

bool CFWL_Edit::CanRedo() {
  return m_pEditEngine->CanRedo();
}

void CFWL_Edit::NotifyTextFull() {
  CFWL_Event evt(CFWL_Event::Type::TextFull, this);
  DispatchEvent(&evt);
}

void CFWL_Edit::OnCaretChanged() {
  if (m_EngineRect.IsEmpty())
    return;
  if ((m_Properties.m_dwStates & FWL_STATE_WGT_Focused) == 0)
    return;

  bool bRepaintContent = UpdateOffset();
  UpdateCaret();
  CFX_RectF rtInvalid;
  bool bRepaintScroll = false;
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_MultiLine) {
    CFWL_ScrollBar* pScroll = UpdateScroll();
    if (pScroll) {
      rtInvalid = pScroll->GetWidgetRect();
      bRepaintScroll = true;
    }
  }
  if (bRepaintContent || bRepaintScroll) {
    if (bRepaintContent)
      rtInvalid.Union(m_EngineRect);
    RepaintRect(rtInvalid);
  }
}

void CFWL_Edit::OnTextWillChange(CFDE_TextEditEngine::TextChange* change) {
  CFWL_EventTextWillChange event(this, change->text, change->previous_text,
                                 change->selection_start,
                                 change->selection_end);
  DispatchEvent(&event);

  change->text = event.GetChangeText();
  change->selection_start = event.GetSelectionStart();
  change->selection_end = event.GetSelectionEnd();
  change->cancelled = event.GetCancelled();
}

void CFWL_Edit::OnTextChanged() {
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_VAlignMask)
    UpdateVAlignment();

  LayoutScrollBar();
  RepaintRect(GetClientRect());
}

void CFWL_Edit::OnSelChanged() {
  RepaintRect(GetClientRect());
}

bool CFWL_Edit::OnValidate(const WideString& wsText) {
  CFWL_EventValidate event(this, wsText);
  DispatchEvent(&event);
  return event.GetValidate();
}

void CFWL_Edit::SetScrollOffset(float fScrollOffset) {
  m_fScrollOffsetY = fScrollOffset;
}

void CFWL_Edit::DrawContent(CFGAS_GEGraphics* pGraphics,
                            const CFX_Matrix& mtMatrix) {
  pGraphics->SaveGraphState();
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_CombText)
    pGraphics->SaveGraphState();

  CFX_RectF rtClip = m_EngineRect;
  float fOffSetX = m_EngineRect.left - m_fScrollOffsetX;
  float fOffSetY = m_EngineRect.top - m_fScrollOffsetY + m_fVAlignOffset;

  CFX_Matrix mt(1, 0, 0, 1, fOffSetX, fOffSetY);
  rtClip = mtMatrix.TransformRect(rtClip);
  mt.Concat(mtMatrix);

  bool bShowSel = !!(m_Properties.m_dwStates & FWL_STATE_WGT_Focused);
  if (bShowSel && m_pEditEngine->HasSelection()) {
    size_t sel_start;
    size_t count;
    std::tie(sel_start, count) = m_pEditEngine->GetSelection();
    std::vector<CFX_RectF> rects =
        m_pEditEngine->GetCharacterRectsInRange(sel_start, count);

    CFGAS_GEPath path;
    for (auto& rect : rects) {
      rect.left += fOffSetX;
      rect.top += fOffSetY;
      path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
    }
    pGraphics->SetClipRect(rtClip);

    CFWL_ThemeBackground param(this, pGraphics);
    param.m_matrix = mtMatrix;
    param.m_iPart = CFWL_ThemePart::Part::kBackground;
    param.m_pPath = &path;
    GetThemeProvider()->DrawBackground(param);
  }

  CFX_RenderDevice* pRenderDev = pGraphics->GetRenderDevice();
  RenderText(pRenderDev, rtClip, mt);

  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_CombText) {
    pGraphics->RestoreGraphState();

    CFGAS_GEPath path;
    int32_t iLimit = m_nLimit > 0 ? m_nLimit : 1;
    float fStep = m_EngineRect.width / iLimit;
    float fLeft = m_EngineRect.left + 1;
    for (int32_t i = 1; i < iLimit; i++) {
      fLeft += fStep;
      path.AddLine(CFX_PointF(fLeft, m_ClientRect.top),
                   CFX_PointF(fLeft, m_ClientRect.bottom()));
    }

    CFWL_ThemeBackground param(this, pGraphics);
    param.m_matrix = mtMatrix;
    param.m_iPart = CFWL_ThemePart::Part::kCombTextLine;
    param.m_pPath = &path;
    GetThemeProvider()->DrawBackground(param);
  }
  pGraphics->RestoreGraphState();
}

void CFWL_Edit::RenderText(CFX_RenderDevice* pRenderDev,
                           const CFX_RectF& clipRect,
                           const CFX_Matrix& mt) {
  DCHECK(pRenderDev);

  RetainPtr<CFGAS_GEFont> font = m_pEditEngine->GetFont();
  if (!font)
    return;

  pRenderDev->SetClip_Rect(clipRect.GetOuterRect());

  CFX_RectF rtDocClip = clipRect;
  if (rtDocClip.IsEmpty()) {
    rtDocClip.left = 0;
    rtDocClip.top = 0;
    rtDocClip.width = static_cast<float>(pRenderDev->GetWidth());
    rtDocClip.height = static_cast<float>(pRenderDev->GetHeight());
  }
  rtDocClip = mt.GetInverse().TransformRect(rtDocClip);

  for (const FDE_TEXTEDITPIECE& info : m_pEditEngine->GetTextPieces()) {
    // If this character is outside the clip, skip it.
    if (!rtDocClip.IntersectWith(info.rtPiece))
      continue;

    std::vector<TextCharPos> char_pos = m_pEditEngine->GetDisplayPos(info);
    if (char_pos.empty())
      continue;

    CFDE_TextOut::DrawString(pRenderDev, m_pEditEngine->GetFontColor(), font,
                             char_pos, m_pEditEngine->GetFontSize(), mt);
  }
}

void CFWL_Edit::UpdateEditEngine() {
  UpdateEditParams();
  UpdateEditLayout();
}

void CFWL_Edit::UpdateEditParams() {
  m_pEditEngine->SetAvailableWidth(m_EngineRect.width);
  m_pEditEngine->SetCombText(
      !!(m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_CombText));

  m_pEditEngine->EnableValidation(
      !!(m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_Validate));
  m_pEditEngine->EnablePasswordMode(
      !!(m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_Password));

  uint32_t alignment = 0;
  switch (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_HAlignMask) {
    case FWL_STYLEEXT_EDT_HNear: {
      alignment |= CFX_TxtLineAlignment_Left;
      break;
    }
    case FWL_STYLEEXT_EDT_HCenter: {
      alignment |= CFX_TxtLineAlignment_Center;
      break;
    }
    case FWL_STYLEEXT_EDT_HFar: {
      alignment |= CFX_TxtLineAlignment_Right;
      break;
    }
    default:
      break;
  }
  switch (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_HAlignModeMask) {
    case FWL_STYLEEXT_EDT_Justified: {
      alignment |= CFX_TxtLineAlignment_Justified;
      break;
    }
    default:
      break;
  }
  m_pEditEngine->SetAlignment(alignment);

  bool auto_hscroll =
      !!(m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_AutoHScroll);
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_MultiLine) {
    m_pEditEngine->EnableMultiLine(true);
    m_pEditEngine->EnableLineWrap(!auto_hscroll);
    m_pEditEngine->LimitVerticalScroll(
        (m_Properties.m_dwStyles & FWL_STYLE_WGT_VScroll) == 0 &&
        (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_AutoVScroll) == 0);
  } else {
    m_pEditEngine->EnableMultiLine(false);
    m_pEditEngine->EnableLineWrap(false);
    m_pEditEngine->LimitVerticalScroll(false);
  }
  m_pEditEngine->LimitHorizontalScroll(!auto_hscroll);

  IFWL_ThemeProvider* theme = GetThemeProvider();
  CFWL_ThemePart part(this);
  m_fFontSize = theme->GetFontSize(part);

  RetainPtr<CFGAS_GEFont> pFont = theme->GetFont(part);
  if (!pFont)
    return;

  m_pEditEngine->SetFont(pFont);
  m_pEditEngine->SetFontColor(theme->GetTextColor(part));
  m_pEditEngine->SetFontSize(m_fFontSize);
  m_pEditEngine->SetLineSpace(theme->GetLineHeight(part));
  m_pEditEngine->SetTabWidth(m_fFontSize);
  m_pEditEngine->SetVisibleLineCount(m_EngineRect.height /
                                     theme->GetLineHeight(part));
}

void CFWL_Edit::UpdateEditLayout() {
  m_pEditEngine->Layout();
}

bool CFWL_Edit::UpdateOffset() {
  CFX_RectF rtCaret = m_CaretRect;

  float fOffSetX = m_EngineRect.left - m_fScrollOffsetX;
  float fOffSetY = m_EngineRect.top - m_fScrollOffsetY + m_fVAlignOffset;
  rtCaret.Offset(fOffSetX, fOffSetY);

  const CFX_RectF& edit_bounds = m_EngineRect;
  if (edit_bounds.Contains(rtCaret)) {
    CFX_RectF contents_bounds = m_pEditEngine->GetContentsBoundingBox();
    contents_bounds.Offset(fOffSetX, fOffSetY);
    if (contents_bounds.right() < edit_bounds.right() && m_fScrollOffsetX > 0) {
      m_fScrollOffsetX += contents_bounds.right() - edit_bounds.right();
      m_fScrollOffsetX = std::max(m_fScrollOffsetX, 0.0f);
    }
    if (contents_bounds.bottom() < edit_bounds.bottom() &&
        m_fScrollOffsetY > 0) {
      m_fScrollOffsetY += contents_bounds.bottom() - edit_bounds.bottom();
      m_fScrollOffsetY = std::max(m_fScrollOffsetY, 0.0f);
    }
    return false;
  }

  float offsetX = 0.0;
  float offsetY = 0.0;
  if (rtCaret.left < edit_bounds.left)
    offsetX = rtCaret.left - edit_bounds.left;
  if (rtCaret.right() > edit_bounds.right())
    offsetX = rtCaret.right() - edit_bounds.right();
  if (rtCaret.top < edit_bounds.top)
    offsetY = rtCaret.top - edit_bounds.top;
  if (rtCaret.bottom() > edit_bounds.bottom())
    offsetY = rtCaret.bottom() - edit_bounds.bottom();

  m_fScrollOffsetX += offsetX;
  m_fScrollOffsetY += offsetY;
  if (m_fFontSize > m_EngineRect.height)
    m_fScrollOffsetY = 0;

  return true;
}

bool CFWL_Edit::UpdateOffset(CFWL_ScrollBar* pScrollBar, float fPosChanged) {
  m_fScrollOffsetY += fPosChanged;
  return true;
}

void CFWL_Edit::UpdateVAlignment() {
  IFWL_ThemeProvider* theme = GetThemeProvider();
  CFWL_ThemePart part(this);
  const CFX_SizeF pSpace = theme->GetSpaceAboveBelow(part);
  const float fSpaceAbove = pSpace.width >= 0.1f ? pSpace.width : 0.0f;
  const float fSpaceBelow = pSpace.height >= 0.1f ? pSpace.height : 0.0f;
  float fOffsetY = 0.0f;
  CFX_RectF contents_bounds = m_pEditEngine->GetContentsBoundingBox();
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_VCenter) {
    fOffsetY = (m_EngineRect.height - contents_bounds.height) / 2.0f;
    if (fOffsetY < (fSpaceAbove + fSpaceBelow) / 2.0f &&
        fSpaceAbove < fSpaceBelow) {
      return;
    }
    fOffsetY += (fSpaceAbove - fSpaceBelow) / 2.0f;
  } else if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_VFar) {
    fOffsetY = (m_EngineRect.height - contents_bounds.height);
    fOffsetY -= fSpaceBelow;
  } else {
    fOffsetY += fSpaceAbove;
  }
  m_fVAlignOffset = std::max(fOffsetY, 0.0f);
}

void CFWL_Edit::UpdateCaret() {
  CFX_RectF rtCaret = m_CaretRect;
  rtCaret.Offset(m_EngineRect.left - m_fScrollOffsetX,
                 m_EngineRect.top - m_fScrollOffsetY + m_fVAlignOffset);

  CFX_RectF rtClient = GetClientRect();
  rtCaret.Intersect(rtClient);
  if (rtCaret.left > rtClient.right()) {
    float right = rtCaret.right();
    rtCaret.left = rtClient.right() - 1;
    rtCaret.width = right - rtCaret.left;
  }

  if (m_Properties.m_dwStates & FWL_STATE_WGT_Focused && !rtCaret.IsEmpty())
    ShowCaret(&rtCaret);
  else
    HideCaret(&rtCaret);
}

CFWL_ScrollBar* CFWL_Edit::UpdateScroll() {
  bool bShowVert = m_pVertScrollBar && m_pVertScrollBar->IsVisible();
  if (!bShowVert)
    return nullptr;

  CFX_RectF contents_bounds = m_pEditEngine->GetContentsBoundingBox();
  CFX_RectF rtScroll = m_pVertScrollBar->GetWidgetRect();
  if (rtScroll.height < contents_bounds.height) {
    float fStep = m_pEditEngine->GetLineSpace();
    float fRange =
        std::max(contents_bounds.height - m_EngineRect.height, fStep);
    m_pVertScrollBar->SetRange(0.0f, fRange);
    float fPos = pdfium::clamp(m_fScrollOffsetY, 0.0f, fRange);
    m_pVertScrollBar->SetPos(fPos);
    m_pVertScrollBar->SetTrackPos(fPos);
    m_pVertScrollBar->SetPageSize(rtScroll.height);
    m_pVertScrollBar->SetStepSize(fStep);
    m_pVertScrollBar->RemoveStates(FWL_STATE_WGT_Disabled);
    m_pVertScrollBar->Update();
    return m_pVertScrollBar;
  }
  if ((m_pVertScrollBar->GetStates() & FWL_STATE_WGT_Disabled) == 0) {
    m_pVertScrollBar->SetRange(0, -1);
    m_pVertScrollBar->SetStates(FWL_STATE_WGT_Disabled);
    m_pVertScrollBar->Update();
    return m_pVertScrollBar;
  }
  return nullptr;
}

bool CFWL_Edit::IsShowVertScrollBar() const {
  const bool bShow =
      !(m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_ShowScrollbarFocus) ||
      (m_Properties.m_dwStates & FWL_STATE_WGT_Focused);
  return bShow && (m_Properties.m_dwStyles & FWL_STYLE_WGT_VScroll) &&
         (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_MultiLine) &&
         IsContentHeightOverflow();
}

bool CFWL_Edit::IsContentHeightOverflow() const {
  return m_pEditEngine->GetContentsBoundingBox().height >
         m_EngineRect.height + 1.0f;
}

void CFWL_Edit::Layout() {
  m_ClientRect = GetClientRect();
  m_EngineRect = m_ClientRect;

  IFWL_ThemeProvider* theme = GetThemeProvider();
  float fWidth = theme->GetScrollBarWidth();
  if (!GetOuter()) {
    CFWL_ThemePart part(this);
    CFX_RectF pUIMargin = theme->GetUIMargin(part);
    m_EngineRect.Deflate(pUIMargin.left, pUIMargin.top, pUIMargin.width,
                         pUIMargin.height);
  } else if (GetOuter()->GetClassID() == FWL_Type::DateTimePicker) {
    CFWL_ThemePart part(GetOuter());
    CFX_RectF pUIMargin = theme->GetUIMargin(part);
    m_EngineRect.Deflate(pUIMargin.left, pUIMargin.top, pUIMargin.width,
                         pUIMargin.height);
  }

  bool bShowVertScrollbar = IsShowVertScrollBar();
  if (bShowVertScrollbar) {
    InitVerticalScrollBar();

    CFX_RectF rtVertScr;
    if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_OuterScrollbar) {
      rtVertScr = CFX_RectF(m_ClientRect.right() + kEditMargin,
                            m_ClientRect.top, fWidth, m_ClientRect.height);
    } else {
      rtVertScr = CFX_RectF(m_ClientRect.right() - fWidth, m_ClientRect.top,
                            fWidth, m_ClientRect.height);
      m_EngineRect.width -= fWidth;
    }

    m_pVertScrollBar->SetWidgetRect(rtVertScr);
    m_pVertScrollBar->RemoveStates(FWL_STATE_WGT_Invisible);
    m_pVertScrollBar->Update();
  } else if (m_pVertScrollBar) {
    m_pVertScrollBar->SetStates(FWL_STATE_WGT_Invisible);
  }
}

void CFWL_Edit::LayoutScrollBar() {
  if (!(m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_ShowScrollbarFocus))
    return;

  bool bShowVertScrollbar = IsShowVertScrollBar();
  IFWL_ThemeProvider* theme = GetThemeProvider();
  float fWidth = theme->GetScrollBarWidth();
  if (bShowVertScrollbar) {
    if (!m_pVertScrollBar) {
      InitVerticalScrollBar();
      CFX_RectF rtVertScr;
      if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_OuterScrollbar) {
        rtVertScr = CFX_RectF(m_ClientRect.right() + kEditMargin,
                              m_ClientRect.top, fWidth, m_ClientRect.height);
      } else {
        rtVertScr = CFX_RectF(m_ClientRect.right() - fWidth, m_ClientRect.top,
                              fWidth, m_ClientRect.height);
      }
      m_pVertScrollBar->SetWidgetRect(rtVertScr);
      m_pVertScrollBar->Update();
    }
    m_pVertScrollBar->RemoveStates(FWL_STATE_WGT_Invisible);
  } else if (m_pVertScrollBar) {
    m_pVertScrollBar->SetStates(FWL_STATE_WGT_Invisible);
  }
  if (bShowVertScrollbar)
    UpdateScroll();
}

CFX_PointF CFWL_Edit::DeviceToEngine(const CFX_PointF& pt) {
  return pt + CFX_PointF(m_fScrollOffsetX - m_EngineRect.left,
                         m_fScrollOffsetY - m_EngineRect.top - m_fVAlignOffset);
}

void CFWL_Edit::InitVerticalScrollBar() {
  if (m_pVertScrollBar)
    return;

  m_pVertScrollBar = cppgc::MakeGarbageCollected<CFWL_ScrollBar>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(),
      Properties{0, FWL_STYLEEXT_SCB_Vert,
                 FWL_STATE_WGT_Disabled | FWL_STATE_WGT_Invisible},
      this);
}

void CFWL_Edit::ShowCaret(CFX_RectF* pRect) {
  if (m_pCaret) {
    m_pCaret->ShowCaret();
    if (!pRect->IsEmpty())
      m_pCaret->SetWidgetRect(*pRect);
    RepaintRect(m_EngineRect);
    return;
  }

  CFWL_Widget* pOuter = this;
  pRect->Offset(m_WidgetRect.left, m_WidgetRect.top);
  while (pOuter->GetOuter()) {
    pOuter = pOuter->GetOuter();
    CFX_RectF rtOuter = pOuter->GetWidgetRect();
    pRect->Offset(rtOuter.left, rtOuter.top);
  }

  CFWL_Widget::AdapterIface* pXFAWidget = pOuter->GetAdapterIface();
  if (!pXFAWidget)
    return;

  CFX_RectF rt = pXFAWidget->GetRotateMatrix().TransformRect(*pRect);
  pXFAWidget->DisplayCaret(true, &rt);
}

void CFWL_Edit::HideCaret(CFX_RectF* pRect) {
  if (m_pCaret) {
    m_pCaret->HideCaret();
    RepaintRect(m_EngineRect);
    return;
  }

  CFWL_Widget* pOuter = this;
  while (pOuter->GetOuter())
    pOuter = pOuter->GetOuter();

  CFWL_Widget::AdapterIface* pXFAWidget = pOuter->GetAdapterIface();
  if (!pXFAWidget)
    return;

  pXFAWidget->DisplayCaret(false, pRect);
}

void CFWL_Edit::InitCaret() {
  if (m_pCaret)
    return;

  m_pCaret = cppgc::MakeGarbageCollected<CFWL_Caret>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(), Properties(),
      this);
  m_pCaret->SetStates(m_Properties.m_dwStates);
  UpdateCursorRect();
}

void CFWL_Edit::UpdateCursorRect() {
  int32_t bidi_level;
  if (m_pEditEngine->CanGenerateCharacterInfo()) {
    std::tie(bidi_level, m_CaretRect) =
        m_pEditEngine->GetCharacterInfo(m_CursorPosition);
  } else {
    bidi_level = 0;
    m_CaretRect = CFX_RectF();
  }

  // TODO(dsinclair): This should handle bidi level  ...

  m_CaretRect.width = 1.0f;

  // TODO(hnakashima): Handle correctly edits with empty text instead of using
  // these defaults.
  if (m_CaretRect.height == 0)
    m_CaretRect.height = 8.0f;
}

void CFWL_Edit::SetCursorPosition(size_t position) {
  if (m_CursorPosition == position)
    return;

  m_CursorPosition = std::min(position, m_pEditEngine->GetLength());
  UpdateCursorRect();
  OnCaretChanged();
}

void CFWL_Edit::OnProcessMessage(CFWL_Message* pMessage) {
  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus:
      OnFocusGained();
      break;
    case CFWL_Message::Type::kKillFocus:
      OnFocusLost();
      break;
    case CFWL_Message::Type::kMouse: {
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case CFWL_MessageMouse::MouseCommand::kLeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeftButtonDblClk:
          OnButtonDoubleClick(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kMove:
          OnMouseMove(pMsg);
          break;
        case CFWL_MessageMouse::MouseCommand::kRightButtonDown:
          DoRButtonDown(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::kKey: {
      CFWL_MessageKey* pKey = static_cast<CFWL_MessageKey*>(pMessage);
      if (pKey->m_dwCmd == CFWL_MessageKey::KeyCommand::kKeyDown)
        OnKeyDown(pKey);
      else if (pKey->m_dwCmd == CFWL_MessageKey::KeyCommand::kChar)
        OnChar(pKey);
      break;
    }
    default:
      break;
  }
  // Dst target could be |this|, continue only if not destroyed by above.
  if (pMessage->GetDstTarget())
    CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_Edit::OnProcessEvent(CFWL_Event* pEvent) {
  if (!pEvent || pEvent->GetType() != CFWL_Event::Type::Scroll)
    return;

  CFWL_Widget* pSrcTarget = pEvent->GetSrcTarget();
  if ((pSrcTarget == m_pVertScrollBar && m_pVertScrollBar)) {
    CFWL_EventScroll* pScrollEvent = static_cast<CFWL_EventScroll*>(pEvent);
    OnScroll(static_cast<CFWL_ScrollBar*>(pSrcTarget),
             pScrollEvent->GetScrollCode(), pScrollEvent->GetPos());
  }
}

void CFWL_Edit::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                             const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_Edit::DoRButtonDown(CFWL_MessageMouse* pMsg) {
  SetCursorPosition(
      m_pEditEngine->GetIndexForPoint(DeviceToEngine(pMsg->m_pos)));
}

void CFWL_Edit::OnFocusGained() {
  m_Properties.m_dwStates |= FWL_STATE_WGT_Focused;
  UpdateVAlignment();
  UpdateOffset();
  UpdateCaret();
  LayoutScrollBar();
}

void CFWL_Edit::OnFocusLost() {
  bool bRepaint = false;
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Focused) {
    m_Properties.m_dwStates &= ~FWL_STATE_WGT_Focused;
    HideCaret(nullptr);
    if (HasSelection()) {
      ClearSelection();
      bRepaint = true;
    }
    UpdateOffset();
  }
  LayoutScrollBar();
  if (!bRepaint)
    return;

  RepaintRect(CFX_RectF(0, 0, m_WidgetRect.width, m_WidgetRect.height));
}

void CFWL_Edit::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  if (m_Properties.m_dwStates & FWL_STATE_WGT_Disabled)
    return;

  m_bLButtonDown = true;
  SetGrab(true);

  bool bRepaint = false;
  if (m_pEditEngine->HasSelection()) {
    m_pEditEngine->ClearSelection();
    bRepaint = true;
  }

  size_t index_at_click =
      m_pEditEngine->GetIndexForPoint(DeviceToEngine(pMsg->m_pos));

  if (index_at_click != m_CursorPosition &&
      !!(pMsg->m_dwFlags & FWL_KEYFLAG_Shift)) {
    size_t start = std::min(m_CursorPosition, index_at_click);
    size_t end = std::max(m_CursorPosition, index_at_click);

    m_pEditEngine->SetSelection(start, end - start);
    bRepaint = true;
  } else {
    SetCursorPosition(index_at_click);
  }

  if (bRepaint)
    RepaintRect(m_EngineRect);
}

void CFWL_Edit::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  m_bLButtonDown = false;
  SetGrab(false);
}

void CFWL_Edit::OnButtonDoubleClick(CFWL_MessageMouse* pMsg) {
  size_t click_idx =
      m_pEditEngine->GetIndexForPoint(DeviceToEngine(pMsg->m_pos));
  size_t start_idx;
  size_t count;
  std::tie(start_idx, count) = m_pEditEngine->BoundsForWordAt(click_idx);

  m_pEditEngine->SetSelection(start_idx, count);
  m_CursorPosition = start_idx + count;
  RepaintRect(m_EngineRect);
}

void CFWL_Edit::OnMouseMove(CFWL_MessageMouse* pMsg) {
  bool shift = !!(pMsg->m_dwFlags & FWL_KEYFLAG_Shift);
  if (!m_bLButtonDown || !shift)
    return;

  size_t old_cursor_pos = m_CursorPosition;
  SetCursorPosition(
      m_pEditEngine->GetIndexForPoint(DeviceToEngine(pMsg->m_pos)));
  if (old_cursor_pos == m_CursorPosition)
    return;

  size_t length = m_pEditEngine->GetLength();
  if (m_CursorPosition > length)
    SetCursorPosition(length);

  size_t sel_start = 0;
  size_t count = 0;
  if (m_pEditEngine->HasSelection())
    std::tie(sel_start, count) = m_pEditEngine->GetSelection();
  else
    sel_start = old_cursor_pos;

  size_t start_pos = std::min(sel_start, m_CursorPosition);
  size_t end_pos = std::max(sel_start, m_CursorPosition);
  m_pEditEngine->SetSelection(start_pos, end_pos - start_pos);
}

void CFWL_Edit::OnKeyDown(CFWL_MessageKey* pMsg) {
  bool bShift = !!(pMsg->m_dwFlags & FWL_KEYFLAG_Shift);
  bool bCtrl = !!(pMsg->m_dwFlags & FWL_KEYFLAG_Ctrl);

  size_t sel_start = m_CursorPosition;
  if (m_pEditEngine->HasSelection()) {
    size_t start_idx;
    size_t count;
    std::tie(start_idx, count) = m_pEditEngine->GetSelection();
    sel_start = start_idx;
  }

  switch (pMsg->m_dwKeyCode) {
    case XFA_FWL_VKEY_Left:
      SetCursorPosition(m_pEditEngine->GetIndexLeft(m_CursorPosition));
      break;
    case XFA_FWL_VKEY_Right:
      SetCursorPosition(m_pEditEngine->GetIndexRight(m_CursorPosition));
      break;
    case XFA_FWL_VKEY_Up:
      SetCursorPosition(m_pEditEngine->GetIndexUp(m_CursorPosition));
      break;
    case XFA_FWL_VKEY_Down:
      SetCursorPosition(m_pEditEngine->GetIndexDown(m_CursorPosition));
      break;
    case XFA_FWL_VKEY_Home:
      SetCursorPosition(
          bCtrl ? 0 : m_pEditEngine->GetIndexAtStartOfLine(m_CursorPosition));
      break;
    case XFA_FWL_VKEY_End:
      SetCursorPosition(
          bCtrl ? m_pEditEngine->GetLength()
                : m_pEditEngine->GetIndexAtEndOfLine(m_CursorPosition));
      break;
    case XFA_FWL_VKEY_Delete: {
      if ((m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_ReadOnly) ||
          (m_Properties.m_dwStates & FWL_STATE_WGT_Disabled)) {
        break;
      }

      m_pEditEngine->Delete(m_CursorPosition, 1);
      UpdateCaret();
      break;
    }
    case XFA_FWL_VKEY_Insert:
    case XFA_FWL_VKEY_F2:
    case XFA_FWL_VKEY_Tab:
    default:
      break;
  }

  // Update the selection.
  if (bShift && sel_start != m_CursorPosition) {
    m_pEditEngine->SetSelection(std::min(sel_start, m_CursorPosition),
                                std::max(sel_start, m_CursorPosition));
    RepaintRect(m_EngineRect);
  }
}

void CFWL_Edit::OnChar(CFWL_MessageKey* pMsg) {
  if ((m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_ReadOnly) ||
      (m_Properties.m_dwStates & FWL_STATE_WGT_Disabled)) {
    return;
  }

  wchar_t c = static_cast<wchar_t>(pMsg->m_dwKeyCode);
  switch (c) {
    case L'\b':
      if (m_CursorPosition > 0) {
        SetCursorPosition(m_CursorPosition - 1);
        m_pEditEngine->Delete(m_CursorPosition, 1);
        UpdateCaret();
      }
      break;
    case L'\n':
    case 27:   // Esc
    case 127:  // Delete
      break;
    case L'\t':
      m_pEditEngine->Insert(m_CursorPosition, L"\t");
      SetCursorPosition(m_CursorPosition + 1);
      break;
    case L'\r':
      if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_EDT_WantReturn) {
        m_pEditEngine->Insert(m_CursorPosition, L"\n");
        SetCursorPosition(m_CursorPosition + 1);
      }
      break;
    default: {
      if (pMsg->m_dwFlags & kEditingModifier)
        break;

      m_pEditEngine->Insert(m_CursorPosition, WideString(c));
      SetCursorPosition(m_CursorPosition + 1);
      break;
    }
  }
}

bool CFWL_Edit::OnScroll(CFWL_ScrollBar* pScrollBar,
                         CFWL_EventScroll::Code dwCode,
                         float fPos) {
  CFX_SizeF fs;
  pScrollBar->GetRange(&fs.width, &fs.height);
  float iCurPos = pScrollBar->GetPos();
  float fStep = pScrollBar->GetStepSize();
  switch (dwCode) {
    case CFWL_EventScroll::Code::Min: {
      fPos = fs.width;
      break;
    }
    case CFWL_EventScroll::Code::Max: {
      fPos = fs.height;
      break;
    }
    case CFWL_EventScroll::Code::StepBackward: {
      fPos -= fStep;
      if (fPos < fs.width + fStep / 2) {
        fPos = fs.width;
      }
      break;
    }
    case CFWL_EventScroll::Code::StepForward: {
      fPos += fStep;
      if (fPos > fs.height - fStep / 2) {
        fPos = fs.height;
      }
      break;
    }
    case CFWL_EventScroll::Code::PageBackward: {
      fPos -= pScrollBar->GetPageSize();
      if (fPos < fs.width) {
        fPos = fs.width;
      }
      break;
    }
    case CFWL_EventScroll::Code::PageForward: {
      fPos += pScrollBar->GetPageSize();
      if (fPos > fs.height) {
        fPos = fs.height;
      }
      break;
    }
    case CFWL_EventScroll::Code::Pos:
    case CFWL_EventScroll::Code::TrackPos:
    case CFWL_EventScroll::Code::None:
      break;
    case CFWL_EventScroll::Code::EndScroll:
      return false;
  }
  if (iCurPos == fPos)
    return true;

  pScrollBar->SetPos(fPos);
  pScrollBar->SetTrackPos(fPos);
  UpdateOffset(pScrollBar, fPos - iCurPos);
  UpdateCaret();

  CFX_RectF rect = GetWidgetRect();
  RepaintRect(CFX_RectF(0, 0, rect.width + 2, rect.height + 2));
  return true;
}
