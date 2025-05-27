// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_edit.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/text_char_pos.h"
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

namespace pdfium {

namespace {

constexpr int kEditMargin = 3;

#if BUILDFLAG(IS_APPLE)
constexpr XFA_FWL_KeyFlag kEditingModifier = XFA_FWL_KeyFlag::kCommand;
#else
constexpr XFA_FWL_KeyFlag kEditingModifier = XFA_FWL_KeyFlag::kCtrl;
#endif

}  // namespace

CFWL_Edit::CFWL_Edit(CFWL_App* app,
                     const Properties& properties,
                     CFWL_Widget* pOuter)
    : CFWL_Widget(app, properties, pOuter),
      edit_engine_(std::make_unique<CFDE_TextEditEngine>()) {
  edit_engine_->SetDelegate(this);
}

CFWL_Edit::~CFWL_Edit() = default;

void CFWL_Edit::PreFinalize() {
  edit_engine_->SetDelegate(nullptr);
  if (properties_.states_ & FWL_STATE_WGT_Focused) {
    HideCaret(nullptr);
  }
  CFWL_Widget::PreFinalize();
}

void CFWL_Edit::Trace(cppgc::Visitor* visitor) const {
  CFWL_Widget::Trace(visitor);
  visitor->Trace(vert_scroll_bar_);
  visitor->Trace(caret_);
}

FWL_Type CFWL_Edit::GetClassID() const {
  return FWL_Type::Edit;
}

CFX_RectF CFWL_Edit::GetWidgetRect() {
  CFX_RectF rect = widget_rect_;
  if (properties_.style_exts_ & FWL_STYLEEXT_EDT_OuterScrollbar) {
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
  if (edit_engine_->GetLength() > 0) {
    CFX_SizeF size =
        CalcTextSize(edit_engine_->GetText(),
                     !!(properties_.style_exts_ & FWL_STYLEEXT_EDT_MultiLine));
    rect = CFX_RectF(0, 0, size);
  }
  InflateWidgetRect(rect);
  return rect;
}

void CFWL_Edit::SetStates(uint32_t dwStates) {
  if ((properties_.states_ & FWL_STATE_WGT_Invisible) ||
      (properties_.states_ & FWL_STATE_WGT_Disabled)) {
    HideCaret(nullptr);
  }
  CFWL_Widget::SetStates(dwStates);
}

void CFWL_Edit::Update() {
  if (IsLocked()) {
    return;
  }

  Layout();
  if (client_rect_.IsEmpty()) {
    return;
  }

  UpdateEditEngine();
  UpdateVAlignment();
  UpdateScroll();
  InitCaret();
}

FWL_WidgetHit CFWL_Edit::HitTest(const CFX_PointF& point) {
  if (properties_.style_exts_ & FWL_STYLEEXT_EDT_OuterScrollbar) {
    if (IsShowVertScrollBar()) {
      if (vert_scroll_bar_->GetWidgetRect().Contains(point)) {
        return FWL_WidgetHit::VScrollBar;
      }
    }
  }
  if (client_rect_.Contains(point)) {
    return FWL_WidgetHit::Edit;
  }
  return FWL_WidgetHit::Unknown;
}

void CFWL_Edit::DrawWidget(CFGAS_GEGraphics* pGraphics,
                           const CFX_Matrix& matrix) {
  if (!pGraphics) {
    return;
  }

  if (client_rect_.IsEmpty()) {
    return;
  }

  DrawContent(pGraphics, matrix);
  if (HasBorder()) {
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);
  }
}

void CFWL_Edit::SetText(const WideString& wsText) {
  edit_engine_->Clear();
  edit_engine_->Insert(0, wsText,
                       CFDE_TextEditEngine::RecordOperation::kInsertRecord);
}

void CFWL_Edit::SetTextSkipNotify(const WideString& wsText) {
  edit_engine_->Clear();
  edit_engine_->Insert(0, wsText,
                       CFDE_TextEditEngine::RecordOperation::kSkipNotify);
}

size_t CFWL_Edit::GetTextLength() const {
  return edit_engine_->GetLength();
}

WideString CFWL_Edit::GetText() const {
  return edit_engine_->GetText();
}

void CFWL_Edit::ClearText() {
  edit_engine_->Clear();
}

void CFWL_Edit::SelectAll() {
  edit_engine_->SelectAll();
}

bool CFWL_Edit::HasSelection() const {
  return edit_engine_->HasSelection();
}

std::pair<size_t, size_t> CFWL_Edit::GetSelection() const {
  return edit_engine_->GetSelection();
}

void CFWL_Edit::ClearSelection() {
  return edit_engine_->ClearSelection();
}

int32_t CFWL_Edit::GetLimit() const {
  return limit_;
}

void CFWL_Edit::SetLimit(int32_t nLimit) {
  limit_ = nLimit;

  if (limit_ > 0) {
    edit_engine_->SetHasCharacterLimit(true);
    edit_engine_->SetCharacterLimit(nLimit);
  } else {
    edit_engine_->SetHasCharacterLimit(false);
  }
}

void CFWL_Edit::SetAliasChar(wchar_t wAlias) {
  edit_engine_->SetAliasChar(wAlias);
}

std::optional<WideString> CFWL_Edit::Copy() {
  if (!edit_engine_->HasSelection()) {
    return std::nullopt;
  }

  return edit_engine_->GetSelectedText();
}

std::optional<WideString> CFWL_Edit::Cut() {
  if (!edit_engine_->HasSelection()) {
    return std::nullopt;
  }

  WideString cut_text = edit_engine_->DeleteSelectedText();
  UpdateCaret();
  return cut_text;
}

bool CFWL_Edit::Paste(const WideString& wsPaste) {
  if (edit_engine_->HasSelection()) {
    edit_engine_->ReplaceSelectedText(wsPaste);
  } else {
    edit_engine_->Insert(cursor_position_, wsPaste);
  }

  return true;
}

bool CFWL_Edit::Undo() {
  return CanUndo() && edit_engine_->Undo();
}

bool CFWL_Edit::Redo() {
  return CanRedo() && edit_engine_->Redo();
}

bool CFWL_Edit::CanUndo() {
  return edit_engine_->CanUndo();
}

bool CFWL_Edit::CanRedo() {
  return edit_engine_->CanRedo();
}

void CFWL_Edit::NotifyTextFull() {
  CFWL_Event evt(CFWL_Event::Type::TextFull, this);
  DispatchEvent(&evt);
}

void CFWL_Edit::OnCaretChanged() {
  if (engine_rect_.IsEmpty()) {
    return;
  }
  if ((properties_.states_ & FWL_STATE_WGT_Focused) == 0) {
    return;
  }

  bool bRepaintContent = UpdateOffset();
  UpdateCaret();
  CFX_RectF rtInvalid;
  bool bRepaintScroll = false;
  if (properties_.style_exts_ & FWL_STYLEEXT_EDT_MultiLine) {
    CFWL_ScrollBar* pScroll = UpdateScroll();
    if (pScroll) {
      rtInvalid = pScroll->GetWidgetRect();
      bRepaintScroll = true;
    }
  }
  if (bRepaintContent || bRepaintScroll) {
    if (bRepaintContent) {
      rtInvalid.Union(engine_rect_);
    }
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
  if (properties_.style_exts_ & FWL_STYLEEXT_EDT_VAlignMask) {
    UpdateVAlignment();
  }

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
  scroll_offset_y_ = fScrollOffset;
}

void CFWL_Edit::DrawContent(CFGAS_GEGraphics* pGraphics,
                            const CFX_Matrix& mtMatrix) {
  DrawContentNonComb(pGraphics, mtMatrix);
  if (properties_.style_exts_ & FWL_STYLEEXT_EDT_CombText) {
    CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
    CFGAS_GEPath path;
    const int32_t iLimit = limit_ > 0 ? limit_ : 1;
    const float fStep = engine_rect_.width / iLimit;
    float fLeft = engine_rect_.left + 1;
    for (int32_t i = 1; i < iLimit; i++) {
      fLeft += fStep;
      path.AddLine(CFX_PointF(fLeft, client_rect_.top),
                   CFX_PointF(fLeft, client_rect_.bottom()));
    }
    CFWL_ThemeBackground param(CFWL_ThemePart::Part::kCombTextLine, this,
                               pGraphics);
    param.matrix_ = mtMatrix;
    param.SetPath(&path);
    GetThemeProvider()->DrawBackground(param);
  }
}

void CFWL_Edit::DrawContentNonComb(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& mtMatrix) {
  CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
  CFX_RectF rtClip = engine_rect_;
  float fOffSetX = engine_rect_.left - scroll_offset_x_;
  float fOffSetY = engine_rect_.top - scroll_offset_y_ + valign_offset_;
  CFX_Matrix mt(1, 0, 0, 1, fOffSetX, fOffSetY);
  rtClip = mtMatrix.TransformRect(rtClip);
  mt.Concat(mtMatrix);

  bool bShowSel = !!(properties_.states_ & FWL_STATE_WGT_Focused);
  if (bShowSel && edit_engine_->HasSelection()) {
    auto [sel_start, count] = edit_engine_->GetSelection();
    std::vector<CFX_RectF> rects = edit_engine_->GetCharacterRectsInRange(
        checked_cast<int32_t>(sel_start), checked_cast<int32_t>(count));

    CFGAS_GEPath path;
    for (auto& rect : rects) {
      rect.left += fOffSetX;
      rect.top += fOffSetY;
      path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
    }
    pGraphics->SetClipRect(rtClip);

    CFWL_ThemeBackground param(CFWL_ThemePart::Part::kBackground, this,
                               pGraphics);
    param.matrix_ = mtMatrix;
    param.SetPath(&path);
    GetThemeProvider()->DrawBackground(param);
  }

  CFX_RenderDevice* pRenderDev = pGraphics->GetRenderDevice();
  RenderText(pRenderDev, rtClip, mt);
}

void CFWL_Edit::RenderText(CFX_RenderDevice* pRenderDev,
                           const CFX_RectF& clipRect,
                           const CFX_Matrix& mt) {
  DCHECK(pRenderDev);

  RetainPtr<CFGAS_GEFont> font = edit_engine_->GetFont();
  if (!font) {
    return;
  }

  pRenderDev->SetClip_Rect(clipRect.GetOuterRect());

  CFX_RectF rtDocClip = clipRect;
  if (rtDocClip.IsEmpty()) {
    rtDocClip.left = 0;
    rtDocClip.top = 0;
    rtDocClip.width = static_cast<float>(pRenderDev->GetWidth());
    rtDocClip.height = static_cast<float>(pRenderDev->GetHeight());
  }
  rtDocClip = mt.GetInverse().TransformRect(rtDocClip);

  for (const FDE_TEXTEDITPIECE& info : edit_engine_->GetTextPieces()) {
    // If this character is outside the clip, skip it.
    if (!rtDocClip.IntersectWith(info.rtPiece)) {
      continue;
    }

    std::vector<TextCharPos> char_pos = edit_engine_->GetDisplayPos(info);
    if (char_pos.empty()) {
      continue;
    }

    CFDE_TextOut::DrawString(pRenderDev, edit_engine_->GetFontColor(), font,
                             char_pos, edit_engine_->GetFontSize(), mt);
  }
}

void CFWL_Edit::UpdateEditEngine() {
  UpdateEditParams();
  UpdateEditLayout();
}

void CFWL_Edit::UpdateEditParams() {
  edit_engine_->SetAvailableWidth(engine_rect_.width);
  edit_engine_->SetCombText(
      !!(properties_.style_exts_ & FWL_STYLEEXT_EDT_CombText));

  edit_engine_->EnableValidation(
      !!(properties_.style_exts_ & FWL_STYLEEXT_EDT_Validate));
  edit_engine_->EnablePasswordMode(
      !!(properties_.style_exts_ & FWL_STYLEEXT_EDT_Password));

  uint32_t alignment = 0;
  switch (properties_.style_exts_ & FWL_STYLEEXT_EDT_HAlignMask) {
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
  switch (properties_.style_exts_ & FWL_STYLEEXT_EDT_HAlignModeMask) {
    case FWL_STYLEEXT_EDT_Justified: {
      alignment |= CFX_TxtLineAlignment_Justified;
      break;
    }
    default:
      break;
  }
  edit_engine_->SetAlignment(alignment);

  bool auto_hscroll =
      !!(properties_.style_exts_ & FWL_STYLEEXT_EDT_AutoHScroll);
  if (properties_.style_exts_ & FWL_STYLEEXT_EDT_MultiLine) {
    edit_engine_->EnableMultiLine(true);
    edit_engine_->EnableLineWrap(!auto_hscroll);
    edit_engine_->LimitVerticalScroll(
        (properties_.styles_ & FWL_STYLE_WGT_VScroll) == 0 &&
        (properties_.style_exts_ & FWL_STYLEEXT_EDT_AutoVScroll) == 0);
  } else {
    edit_engine_->EnableMultiLine(false);
    edit_engine_->EnableLineWrap(false);
    edit_engine_->LimitVerticalScroll(false);
  }
  edit_engine_->LimitHorizontalScroll(!auto_hscroll);

  IFWL_ThemeProvider* theme = GetThemeProvider();
  CFWL_ThemePart part(CFWL_ThemePart::Part::kNone, this);
  font_size_ = theme->GetFontSize(part);

  RetainPtr<CFGAS_GEFont> font = theme->GetFont(part);
  if (!font) {
    return;
  }

  edit_engine_->SetFont(font);
  edit_engine_->SetFontColor(theme->GetTextColor(part));
  edit_engine_->SetFontSize(font_size_);
  edit_engine_->SetLineSpace(theme->GetLineHeight(part));
  edit_engine_->SetTabWidth(font_size_);
  edit_engine_->SetVisibleLineCount(engine_rect_.height /
                                    theme->GetLineHeight(part));
}

void CFWL_Edit::UpdateEditLayout() {
  edit_engine_->Layout();
}

bool CFWL_Edit::UpdateOffset() {
  CFX_RectF rtCaret = caret_rect_;

  float fOffSetX = engine_rect_.left - scroll_offset_x_;
  float fOffSetY = engine_rect_.top - scroll_offset_y_ + valign_offset_;
  rtCaret.Offset(fOffSetX, fOffSetY);

  const CFX_RectF& edit_bounds = engine_rect_;
  if (edit_bounds.Contains(rtCaret)) {
    CFX_RectF contents_bounds = edit_engine_->GetContentsBoundingBox();
    contents_bounds.Offset(fOffSetX, fOffSetY);
    if (contents_bounds.right() < edit_bounds.right() && scroll_offset_x_ > 0) {
      scroll_offset_x_ += contents_bounds.right() - edit_bounds.right();
      scroll_offset_x_ = std::max(scroll_offset_x_, 0.0f);
    }
    if (contents_bounds.bottom() < edit_bounds.bottom() &&
        scroll_offset_y_ > 0) {
      scroll_offset_y_ += contents_bounds.bottom() - edit_bounds.bottom();
      scroll_offset_y_ = std::max(scroll_offset_y_, 0.0f);
    }
    return false;
  }

  float offsetX = 0.0;
  float offsetY = 0.0;
  if (rtCaret.left < edit_bounds.left) {
    offsetX = rtCaret.left - edit_bounds.left;
  }
  if (rtCaret.right() > edit_bounds.right()) {
    offsetX = rtCaret.right() - edit_bounds.right();
  }
  if (rtCaret.top < edit_bounds.top) {
    offsetY = rtCaret.top - edit_bounds.top;
  }
  if (rtCaret.bottom() > edit_bounds.bottom()) {
    offsetY = rtCaret.bottom() - edit_bounds.bottom();
  }

  scroll_offset_x_ += offsetX;
  scroll_offset_y_ += offsetY;
  if (font_size_ > engine_rect_.height) {
    scroll_offset_y_ = 0;
  }

  return true;
}

bool CFWL_Edit::UpdateOffset(CFWL_ScrollBar* pScrollBar, float fPosChanged) {
  scroll_offset_y_ += fPosChanged;
  return true;
}

void CFWL_Edit::UpdateVAlignment() {
  IFWL_ThemeProvider* theme = GetThemeProvider();
  CFWL_ThemePart part(CFWL_ThemePart::Part::kNone, this);
  const CFX_SizeF pSpace = theme->GetSpaceAboveBelow(part);
  const float fSpaceAbove = pSpace.width >= 0.1f ? pSpace.width : 0.0f;
  const float fSpaceBelow = pSpace.height >= 0.1f ? pSpace.height : 0.0f;
  float fOffsetY = 0.0f;
  CFX_RectF contents_bounds = edit_engine_->GetContentsBoundingBox();
  if (properties_.style_exts_ & FWL_STYLEEXT_EDT_VCenter) {
    fOffsetY = (engine_rect_.height - contents_bounds.height) / 2.0f;
    if (fOffsetY < (fSpaceAbove + fSpaceBelow) / 2.0f &&
        fSpaceAbove < fSpaceBelow) {
      return;
    }
    fOffsetY += (fSpaceAbove - fSpaceBelow) / 2.0f;
  } else if (properties_.style_exts_ & FWL_STYLEEXT_EDT_VFar) {
    fOffsetY = (engine_rect_.height - contents_bounds.height);
    fOffsetY -= fSpaceBelow;
  } else {
    fOffsetY += fSpaceAbove;
  }
  valign_offset_ = std::max(fOffsetY, 0.0f);
}

void CFWL_Edit::UpdateCaret() {
  CFX_RectF rtCaret = caret_rect_;
  rtCaret.Offset(engine_rect_.left - scroll_offset_x_,
                 engine_rect_.top - scroll_offset_y_ + valign_offset_);

  CFX_RectF rtClient = GetClientRect();
  rtCaret.Intersect(rtClient);
  if (rtCaret.left > rtClient.right()) {
    float right = rtCaret.right();
    rtCaret.left = rtClient.right() - 1;
    rtCaret.width = right - rtCaret.left;
  }

  if (properties_.states_ & FWL_STATE_WGT_Focused && !rtCaret.IsEmpty()) {
    ShowCaret(&rtCaret);
  } else {
    HideCaret(&rtCaret);
  }
}

CFWL_ScrollBar* CFWL_Edit::UpdateScroll() {
  bool bShowVert = vert_scroll_bar_ && vert_scroll_bar_->IsVisible();
  if (!bShowVert) {
    return nullptr;
  }

  CFX_RectF contents_bounds = edit_engine_->GetContentsBoundingBox();
  CFX_RectF rtScroll = vert_scroll_bar_->GetWidgetRect();
  if (rtScroll.height < contents_bounds.height) {
    float fStep = edit_engine_->GetLineSpace();
    float fRange =
        std::max(contents_bounds.height - engine_rect_.height, fStep);
    vert_scroll_bar_->SetRange(0.0f, fRange);
    float fPos = std::clamp(scroll_offset_y_, 0.0f, fRange);
    vert_scroll_bar_->SetPos(fPos);
    vert_scroll_bar_->SetTrackPos(fPos);
    vert_scroll_bar_->SetPageSize(rtScroll.height);
    vert_scroll_bar_->SetStepSize(fStep);
    vert_scroll_bar_->RemoveStates(FWL_STATE_WGT_Disabled);
    vert_scroll_bar_->Update();
    return vert_scroll_bar_;
  }
  if ((vert_scroll_bar_->GetStates() & FWL_STATE_WGT_Disabled) == 0) {
    vert_scroll_bar_->SetRange(0, -1);
    vert_scroll_bar_->SetStates(FWL_STATE_WGT_Disabled);
    vert_scroll_bar_->Update();
    return vert_scroll_bar_;
  }
  return nullptr;
}

bool CFWL_Edit::IsShowVertScrollBar() const {
  const bool bShow =
      !(properties_.style_exts_ & FWL_STYLEEXT_EDT_ShowScrollbarFocus) ||
      (properties_.states_ & FWL_STATE_WGT_Focused);
  return bShow && (properties_.styles_ & FWL_STYLE_WGT_VScroll) &&
         (properties_.style_exts_ & FWL_STYLEEXT_EDT_MultiLine) &&
         IsContentHeightOverflow();
}

bool CFWL_Edit::IsContentHeightOverflow() const {
  return edit_engine_->GetContentsBoundingBox().height >
         engine_rect_.height + 1.0f;
}

void CFWL_Edit::Layout() {
  client_rect_ = GetClientRect();
  engine_rect_ = client_rect_;

  IFWL_ThemeProvider* theme = GetThemeProvider();
  float fWidth = theme->GetScrollBarWidth();
  if (!GetOuter()) {
    CFWL_ThemePart part(CFWL_ThemePart::Part::kNone, this);
    CFX_RectF pUIMargin = theme->GetUIMargin(part);
    engine_rect_.Deflate(pUIMargin.left, pUIMargin.top, pUIMargin.width,
                         pUIMargin.height);
  } else if (GetOuter()->GetClassID() == FWL_Type::DateTimePicker) {
    CFWL_ThemePart part(CFWL_ThemePart::Part::kNone, GetOuter());
    CFX_RectF pUIMargin = theme->GetUIMargin(part);
    engine_rect_.Deflate(pUIMargin.left, pUIMargin.top, pUIMargin.width,
                         pUIMargin.height);
  }

  bool bShowVertScrollbar = IsShowVertScrollBar();
  if (bShowVertScrollbar) {
    InitVerticalScrollBar();

    CFX_RectF rtVertScr;
    if (properties_.style_exts_ & FWL_STYLEEXT_EDT_OuterScrollbar) {
      rtVertScr = CFX_RectF(client_rect_.right() + kEditMargin,
                            client_rect_.top, fWidth, client_rect_.height);
    } else {
      rtVertScr = CFX_RectF(client_rect_.right() - fWidth, client_rect_.top,
                            fWidth, client_rect_.height);
      engine_rect_.width -= fWidth;
    }

    vert_scroll_bar_->SetWidgetRect(rtVertScr);
    vert_scroll_bar_->RemoveStates(FWL_STATE_WGT_Invisible);
    vert_scroll_bar_->Update();
  } else if (vert_scroll_bar_) {
    vert_scroll_bar_->SetStates(FWL_STATE_WGT_Invisible);
  }
}

void CFWL_Edit::LayoutScrollBar() {
  if (!(properties_.style_exts_ & FWL_STYLEEXT_EDT_ShowScrollbarFocus)) {
    return;
  }

  bool bShowVertScrollbar = IsShowVertScrollBar();
  IFWL_ThemeProvider* theme = GetThemeProvider();
  float fWidth = theme->GetScrollBarWidth();
  if (bShowVertScrollbar) {
    if (!vert_scroll_bar_) {
      InitVerticalScrollBar();
      CFX_RectF rtVertScr;
      if (properties_.style_exts_ & FWL_STYLEEXT_EDT_OuterScrollbar) {
        rtVertScr = CFX_RectF(client_rect_.right() + kEditMargin,
                              client_rect_.top, fWidth, client_rect_.height);
      } else {
        rtVertScr = CFX_RectF(client_rect_.right() - fWidth, client_rect_.top,
                              fWidth, client_rect_.height);
      }
      vert_scroll_bar_->SetWidgetRect(rtVertScr);
      vert_scroll_bar_->Update();
    }
    vert_scroll_bar_->RemoveStates(FWL_STATE_WGT_Invisible);
  } else if (vert_scroll_bar_) {
    vert_scroll_bar_->SetStates(FWL_STATE_WGT_Invisible);
  }
  if (bShowVertScrollbar) {
    UpdateScroll();
  }
}

CFX_PointF CFWL_Edit::DeviceToEngine(const CFX_PointF& pt) {
  return pt + CFX_PointF(scroll_offset_x_ - engine_rect_.left,
                         scroll_offset_y_ - engine_rect_.top - valign_offset_);
}

void CFWL_Edit::InitVerticalScrollBar() {
  if (vert_scroll_bar_) {
    return;
  }

  vert_scroll_bar_ = cppgc::MakeGarbageCollected<CFWL_ScrollBar>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(),
      Properties{0, FWL_STYLEEXT_SCB_Vert,
                 FWL_STATE_WGT_Disabled | FWL_STATE_WGT_Invisible},
      this);
}

void CFWL_Edit::ShowCaret(CFX_RectF* pRect) {
  if (caret_) {
    caret_->ShowCaret();
    if (!pRect->IsEmpty()) {
      caret_->SetWidgetRect(*pRect);
    }
    RepaintRect(engine_rect_);
    return;
  }

  CFWL_Widget* pOuter = this;
  pRect->Offset(widget_rect_.left, widget_rect_.top);
  while (pOuter->GetOuter()) {
    pOuter = pOuter->GetOuter();
    CFX_RectF rtOuter = pOuter->GetWidgetRect();
    pRect->Offset(rtOuter.left, rtOuter.top);
  }

  CFWL_Widget::AdapterIface* pXFAWidget = pOuter->GetAdapterIface();
  if (!pXFAWidget) {
    return;
  }

  CFX_RectF rt = pXFAWidget->GetRotateMatrix().TransformRect(*pRect);
  pXFAWidget->DisplayCaret(true, &rt);
}

void CFWL_Edit::HideCaret(CFX_RectF* pRect) {
  if (caret_) {
    caret_->HideCaret();
    RepaintRect(engine_rect_);
    return;
  }

  CFWL_Widget* pOuter = this;
  while (pOuter->GetOuter()) {
    pOuter = pOuter->GetOuter();
  }

  CFWL_Widget::AdapterIface* pXFAWidget = pOuter->GetAdapterIface();
  if (!pXFAWidget) {
    return;
  }

  pXFAWidget->DisplayCaret(false, pRect);
}

void CFWL_Edit::InitCaret() {
  if (caret_) {
    return;
  }

  caret_ = cppgc::MakeGarbageCollected<CFWL_Caret>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(), Properties(),
      this);
  caret_->SetStates(properties_.states_);
  UpdateCursorRect();
}

void CFWL_Edit::UpdateCursorRect() {
  int32_t bidi_level;
  if (edit_engine_->CanGenerateCharacterInfo()) {
    std::tie(bidi_level, caret_rect_) =
        edit_engine_->GetCharacterInfo(checked_cast<int32_t>(cursor_position_));
  } else {
    bidi_level = 0;
    caret_rect_ = CFX_RectF();
  }

  // TODO(dsinclair): This should handle bidi level  ...

  caret_rect_.width = 1.0f;

  // TODO(hnakashima): Handle correctly edits with empty text instead of using
  // these defaults.
  if (caret_rect_.height == 0) {
    caret_rect_.height = 8.0f;
  }
}

void CFWL_Edit::SetCursorPosition(size_t position) {
  if (cursor_position_ == position) {
    return;
  }

  cursor_position_ = std::min(position, edit_engine_->GetLength());
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
      switch (pMsg->cmd_) {
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
      if (pKey->cmd_ == CFWL_MessageKey::KeyCommand::kKeyDown) {
        OnKeyDown(pKey);
      } else if (pKey->cmd_ == CFWL_MessageKey::KeyCommand::kChar) {
        OnChar(pKey);
      }
      break;
    }
    default:
      break;
  }
  // Dst target could be |this|, continue only if not destroyed by above.
  if (pMessage->GetDstTarget()) {
    CFWL_Widget::OnProcessMessage(pMessage);
  }
}

void CFWL_Edit::OnProcessEvent(CFWL_Event* pEvent) {
  if (!pEvent || pEvent->GetType() != CFWL_Event::Type::Scroll) {
    return;
  }

  CFWL_Widget* pSrcTarget = pEvent->GetSrcTarget();
  if ((pSrcTarget == vert_scroll_bar_ && vert_scroll_bar_)) {
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
  SetCursorPosition(edit_engine_->GetIndexForPoint(DeviceToEngine(pMsg->pos_)));
}

void CFWL_Edit::OnFocusGained() {
  properties_.states_ |= FWL_STATE_WGT_Focused;
  UpdateVAlignment();
  UpdateOffset();
  UpdateCaret();
  LayoutScrollBar();
}

void CFWL_Edit::OnFocusLost() {
  bool bRepaint = false;
  if (properties_.states_ & FWL_STATE_WGT_Focused) {
    properties_.states_ &= ~FWL_STATE_WGT_Focused;
    HideCaret(nullptr);
    if (HasSelection()) {
      ClearSelection();
      bRepaint = true;
    }
    UpdateOffset();
  }
  LayoutScrollBar();
  if (!bRepaint) {
    return;
  }

  RepaintRect(CFX_RectF(0, 0, widget_rect_.width, widget_rect_.height));
}

void CFWL_Edit::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  if (properties_.states_ & FWL_STATE_WGT_Disabled) {
    return;
  }

  lbutton_down_ = true;
  SetGrab(true);

  bool bRepaint = false;
  if (edit_engine_->HasSelection()) {
    edit_engine_->ClearSelection();
    bRepaint = true;
  }

  size_t index_at_click =
      edit_engine_->GetIndexForPoint(DeviceToEngine(pMsg->pos_));

  if (index_at_click != cursor_position_ &&
      !!(pMsg->flags_ & XFA_FWL_KeyFlag::kShift)) {
    size_t start = std::min(cursor_position_, index_at_click);
    size_t end = std::max(cursor_position_, index_at_click);

    edit_engine_->SetSelection(start, end - start);
    bRepaint = true;
  } else {
    SetCursorPosition(index_at_click);
  }

  if (bRepaint) {
    RepaintRect(engine_rect_);
  }
}

void CFWL_Edit::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  lbutton_down_ = false;
  SetGrab(false);
}

void CFWL_Edit::OnButtonDoubleClick(CFWL_MessageMouse* pMsg) {
  size_t click_idx = edit_engine_->GetIndexForPoint(DeviceToEngine(pMsg->pos_));
  auto [start_idx, count] = edit_engine_->BoundsForWordAt(click_idx);

  edit_engine_->SetSelection(start_idx, count);
  cursor_position_ = start_idx + count;
  RepaintRect(engine_rect_);
}

void CFWL_Edit::OnMouseMove(CFWL_MessageMouse* pMsg) {
  bool shift = !!(pMsg->flags_ & XFA_FWL_KeyFlag::kShift);
  if (!lbutton_down_ || !shift) {
    return;
  }

  size_t old_cursor_pos = cursor_position_;
  SetCursorPosition(edit_engine_->GetIndexForPoint(DeviceToEngine(pMsg->pos_)));
  if (old_cursor_pos == cursor_position_) {
    return;
  }

  size_t length = edit_engine_->GetLength();
  if (cursor_position_ > length) {
    SetCursorPosition(length);
  }

  size_t sel_start = 0;
  size_t count = 0;
  if (edit_engine_->HasSelection()) {
    std::tie(sel_start, count) = edit_engine_->GetSelection();
  } else {
    sel_start = old_cursor_pos;
  }

  size_t start_pos = std::min(sel_start, cursor_position_);
  size_t end_pos = std::max(sel_start, cursor_position_);
  edit_engine_->SetSelection(start_pos, end_pos - start_pos);
}

void CFWL_Edit::OnKeyDown(CFWL_MessageKey* pMsg) {
  bool bShift = !!(pMsg->flags_ & XFA_FWL_KeyFlag::kShift);
  bool bCtrl = !!(pMsg->flags_ & XFA_FWL_KeyFlag::kCtrl);

  size_t sel_start = cursor_position_;
  if (edit_engine_->HasSelection()) {
    auto [start_idx, count] = edit_engine_->GetSelection();
    sel_start = start_idx;
  }

  switch (pMsg->key_code_or_char_) {
    case XFA_FWL_VKEY_Left:
      SetCursorPosition(edit_engine_->GetIndexLeft(cursor_position_));
      break;
    case XFA_FWL_VKEY_Right:
      SetCursorPosition(edit_engine_->GetIndexRight(cursor_position_));
      break;
    case XFA_FWL_VKEY_Up:
      SetCursorPosition(edit_engine_->GetIndexUp(cursor_position_));
      break;
    case XFA_FWL_VKEY_Down:
      SetCursorPosition(edit_engine_->GetIndexDown(cursor_position_));
      break;
    case XFA_FWL_VKEY_Home:
      SetCursorPosition(
          bCtrl ? 0 : edit_engine_->GetIndexAtStartOfLine(cursor_position_));
      break;
    case XFA_FWL_VKEY_End:
      SetCursorPosition(
          bCtrl ? edit_engine_->GetLength()
                : edit_engine_->GetIndexAtEndOfLine(cursor_position_));
      break;
    case XFA_FWL_VKEY_Delete: {
      if ((properties_.style_exts_ & FWL_STYLEEXT_EDT_ReadOnly) ||
          (properties_.states_ & FWL_STATE_WGT_Disabled)) {
        break;
      }

      edit_engine_->Delete(cursor_position_, 1);
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
  if (bShift && sel_start != cursor_position_) {
    edit_engine_->SetSelection(std::min(sel_start, cursor_position_),
                               std::max(sel_start, cursor_position_));
    RepaintRect(engine_rect_);
  }
}

void CFWL_Edit::OnChar(CFWL_MessageKey* pMsg) {
  if ((properties_.style_exts_ & FWL_STYLEEXT_EDT_ReadOnly) ||
      (properties_.states_ & FWL_STATE_WGT_Disabled)) {
    return;
  }

  wchar_t c = static_cast<wchar_t>(pMsg->key_code_or_char_);
  switch (c) {
    case L'\b':
      if (cursor_position_ > 0) {
        SetCursorPosition(cursor_position_ - 1);
        edit_engine_->Delete(cursor_position_, 1);
        UpdateCaret();
      }
      break;
    case L'\n':
    case 27:   // Esc
    case 127:  // Delete
      break;
    case L'\t':
      edit_engine_->Insert(cursor_position_, L"\t");
      SetCursorPosition(cursor_position_ + 1);
      break;
    case L'\r':
      if (properties_.style_exts_ & FWL_STYLEEXT_EDT_WantReturn) {
        edit_engine_->Insert(cursor_position_, L"\n");
        SetCursorPosition(cursor_position_ + 1);
      }
      break;
    default: {
      if (pMsg->flags_ & kEditingModifier) {
        break;
      }

      edit_engine_->Insert(cursor_position_, WideString(c));
      SetCursorPosition(cursor_position_ + 1);
      break;
    }
  }
}

bool CFWL_Edit::OnScroll(CFWL_ScrollBar* pScrollBar,
                         CFWL_EventScroll::Code dwCode,
                         float fPos) {
  float fMin;
  float fMax;
  pScrollBar->GetRange(&fMin, &fMax);
  float iCurPos = pScrollBar->GetPos();
  float fStep = pScrollBar->GetStepSize();
  switch (dwCode) {
    case CFWL_EventScroll::Code::Min: {
      fPos = fMin;
      break;
    }
    case CFWL_EventScroll::Code::Max: {
      fPos = fMax;
      break;
    }
    case CFWL_EventScroll::Code::StepBackward: {
      fPos -= fStep;
      if (fPos < fMin + fStep / 2) {
        fPos = fMin;
      }
      break;
    }
    case CFWL_EventScroll::Code::StepForward: {
      fPos += fStep;
      if (fPos > fMax - fStep / 2) {
        fPos = fMax;
      }
      break;
    }
    case CFWL_EventScroll::Code::PageBackward: {
      fPos -= pScrollBar->GetPageSize();
      if (fPos < fMin) {
        fPos = fMin;
      }
      break;
    }
    case CFWL_EventScroll::Code::PageForward: {
      fPos += pScrollBar->GetPageSize();
      if (fPos > fMax) {
        fPos = fMax;
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
  if (iCurPos == fPos) {
    return true;
  }

  pScrollBar->SetPos(fPos);
  pScrollBar->SetTrackPos(fPos);
  UpdateOffset(pScrollBar, fPos - iCurPos);
  UpdateCaret();

  CFX_RectF rect = GetWidgetRect();
  RepaintRect(CFX_RectF(0, 0, rect.width + 2, rect.height + 2));
  return true;
}

}  // namespace pdfium
