// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_datetimepicker.h"

#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_eventselectchanged.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_messagesetfocus.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace pdfium {

namespace {

constexpr int kDateTimePickerHeight = 20;

}  // namespace

CFWL_DateTimePicker::CFWL_DateTimePicker(CFWL_App* app)
    : CFWL_Widget(app,
                  Properties{0, FWL_STYLEEXT_DTP_ShortDateFormat, 0},
                  nullptr),
      edit_(cppgc::MakeGarbageCollected<CFWL_DateTimeEdit>(
          app->GetHeap()->GetAllocationHandle(),
          app,
          Properties(),
          this)),
      month_cal_(cppgc::MakeGarbageCollected<CFWL_MonthCalendar>(
          app->GetHeap()->GetAllocationHandle(),
          app,
          Properties{FWL_STYLE_WGT_Popup | FWL_STYLE_WGT_Border, 0,
                     FWL_STATE_WGT_Invisible},
          this)) {
  month_cal_->SetWidgetRect(
      CFX_RectF(0, 0, month_cal_->GetAutosizedWidgetRect().Size()));

  CFWL_NoteDriver* pNoteDriver = GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(this, month_cal_);
  pNoteDriver->RegisterEventTarget(this, edit_);
}

CFWL_DateTimePicker::~CFWL_DateTimePicker() = default;

void CFWL_DateTimePicker::PreFinalize() {
  UnregisterEventTarget();
  CFWL_Widget::PreFinalize();
}

void CFWL_DateTimePicker::Trace(cppgc::Visitor* visitor) const {
  CFWL_Widget::Trace(visitor);
  visitor->Trace(edit_);
  visitor->Trace(month_cal_);
}

FWL_Type CFWL_DateTimePicker::GetClassID() const {
  return FWL_Type::DateTimePicker;
}

void CFWL_DateTimePicker::Update() {
  if (IsLocked())
    return;

  client_rect_ = GetClientRect();
  edit_->SetWidgetRect(client_rect_);
  ResetEditAlignment();
  edit_->Update();

  btn_ = GetThemeProvider()->GetScrollBarWidth();
  CFX_RectF rtMonthCal = month_cal_->GetAutosizedWidgetRect();
  CFX_RectF rtPopUp(rtMonthCal.left, rtMonthCal.top + kDateTimePickerHeight,
                    rtMonthCal.width, rtMonthCal.height);
  month_cal_->SetWidgetRect(rtPopUp);
  month_cal_->Update();
}

FWL_WidgetHit CFWL_DateTimePicker::HitTest(const CFX_PointF& point) {
  CFX_RectF rect(0, 0, widget_rect_.width, widget_rect_.height);
  if (rect.Contains(point))
    return FWL_WidgetHit::Edit;
  if (NeedsToShowButton())
    rect.width += btn_;
  if (rect.Contains(point))
    return FWL_WidgetHit::Client;
  if (IsMonthCalendarVisible()) {
    if (month_cal_->GetWidgetRect().Contains(point)) {
      return FWL_WidgetHit::Client;
    }
  }
  return FWL_WidgetHit::Unknown;
}

void CFWL_DateTimePicker::DrawWidget(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;

  if (HasBorder())
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);

  if (!btn_rect_.IsEmpty()) {
    DrawDropDownButton(pGraphics, matrix);
  }

  if (edit_) {
    CFX_RectF rtEdit = edit_->GetWidgetRect();
    CFX_Matrix mt(1, 0, 0, 1, rtEdit.left, rtEdit.top);
    mt.Concat(matrix);
    edit_->DrawWidget(pGraphics, mt);
  }
  if (!IsMonthCalendarVisible())
    return;

  CFX_RectF rtMonth = month_cal_->GetWidgetRect();
  CFX_Matrix mt(1, 0, 0, 1, rtMonth.left, rtMonth.top);
  mt.Concat(matrix);
  month_cal_->DrawWidget(pGraphics, mt);
}

void CFWL_DateTimePicker::GetCurSel(int32_t& iYear,
                                    int32_t& iMonth,
                                    int32_t& iDay) {
  iYear = year_;
  iMonth = month_;
  iDay = day_;
}

void CFWL_DateTimePicker::SetCurSel(int32_t iYear,
                                    int32_t iMonth,
                                    int32_t iDay) {
  if (iYear <= 0 || iYear >= 3000)
    return;
  if (iMonth <= 0 || iMonth >= 13)
    return;
  if (iDay <= 0 || iDay >= 32)
    return;

  year_ = iYear;
  month_ = iMonth;
  day_ = iDay;
  month_cal_->SetSelect(iYear, iMonth, iDay);
}

void CFWL_DateTimePicker::SetEditText(const WideString& wsText) {
  if (!edit_) {
    return;
  }

  edit_->SetText(wsText);
  RepaintRect(client_rect_);

  CFWL_Event ev(CFWL_Event::Type::EditChanged);
  DispatchEvent(&ev);
}

WideString CFWL_DateTimePicker::GetEditText() const {
  return edit_ ? edit_->GetText() : WideString();
}

size_t CFWL_DateTimePicker::GetEditTextLength() const {
  return edit_ ? edit_->GetTextLength() : 0;
}

CFX_RectF CFWL_DateTimePicker::GetBBox() const {
  CFX_RectF rect = widget_rect_;
  if (NeedsToShowButton())
    rect.width += btn_;
  if (!IsMonthCalendarVisible())
    return rect;

  CFX_RectF rtMonth = month_cal_->GetWidgetRect();
  rtMonth.Offset(widget_rect_.left, widget_rect_.top);
  rect.Union(rtMonth);
  return rect;
}

void CFWL_DateTimePicker::ModifyEditStyleExts(uint32_t dwStyleExtsAdded,
                                              uint32_t dwStyleExtsRemoved) {
  edit_->ModifyStyleExts(dwStyleExtsAdded, dwStyleExtsRemoved);
}

void CFWL_DateTimePicker::DrawDropDownButton(CFGAS_GEGraphics* pGraphics,
                                             const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kDropDownButton, this,
                             pGraphics);
  param.states_ = btn_state_;
  param.part_rect_ = btn_rect_;
  param.matrix_ = mtMatrix;
  GetThemeProvider()->DrawBackground(param);
}

WideString CFWL_DateTimePicker::FormatDateString(int32_t iYear,
                                                 int32_t iMonth,
                                                 int32_t iDay) {
  if (properties_.style_exts_ & FWL_STYLEEXT_DTP_ShortDateFormat) {
    return WideString::Format(L"%d-%d-%d", iYear, iMonth, iDay);
  }

  return WideString::Format(L"%d Year %d Month %d Day", iYear, iMonth, iDay);
}

void CFWL_DateTimePicker::ShowMonthCalendar() {
  if (IsMonthCalendarVisible())
    return;

  CFX_RectF rtMonthCal = month_cal_->GetAutosizedWidgetRect();
  float fPopupMin = rtMonthCal.height;
  float fPopupMax = rtMonthCal.height;
  CFX_RectF rtAnchor = widget_rect_;
  rtAnchor.width = rtMonthCal.width;
  rtMonthCal.left = client_rect_.left;
  rtMonthCal.top = rtAnchor.Height();
  GetPopupPos(fPopupMin, fPopupMax, rtAnchor, &rtMonthCal);
  month_cal_->SetWidgetRect(rtMonthCal);
  if (year_ > 0 && month_ > 0 && day_ > 0) {
    month_cal_->SetSelect(year_, month_, day_);
  }
  month_cal_->Update();
  month_cal_->RemoveStates(FWL_STATE_WGT_Invisible);

  CFWL_MessageSetFocus msg(month_cal_);
  edit_->GetDelegate()->OnProcessMessage(&msg);
  RepaintInflatedMonthCalRect();
}

void CFWL_DateTimePicker::HideMonthCalendar() {
  if (!IsMonthCalendarVisible())
    return;

  month_cal_->SetStates(FWL_STATE_WGT_Invisible);
  RepaintInflatedMonthCalRect();
}

void CFWL_DateTimePicker::RepaintInflatedMonthCalRect() {
  CFX_RectF rtInvalidate(0, 0, widget_rect_.width, widget_rect_.height);
  CFX_RectF rtCal = month_cal_->GetWidgetRect();
  rtInvalidate.Union(rtCal);
  rtInvalidate.Inflate(2, 2);
  RepaintRect(rtInvalidate);
}

bool CFWL_DateTimePicker::IsMonthCalendarVisible() const {
  return month_cal_ && month_cal_->IsVisible();
}

void CFWL_DateTimePicker::ResetEditAlignment() {
  if (!edit_) {
    return;
  }

  uint32_t dwAdd = 0;
  switch (properties_.style_exts_ & FWL_STYLEEXT_DTP_EditHAlignMask) {
    case FWL_STYLEEXT_DTP_EditHCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_HCenter;
      break;
    }
    case FWL_STYLEEXT_DTP_EditHFar: {
      dwAdd |= FWL_STYLEEXT_EDT_HFar;
      break;
    }
    default: {
      dwAdd |= FWL_STYLEEXT_EDT_HNear;
      break;
    }
  }
  switch (properties_.style_exts_ & FWL_STYLEEXT_DTP_EditVAlignMask) {
    case FWL_STYLEEXT_DTP_EditVCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_VCenter;
      break;
    }
    case FWL_STYLEEXT_DTP_EditVFar: {
      dwAdd |= FWL_STYLEEXT_EDT_VFar;
      break;
    }
    default: {
      dwAdd |= FWL_STYLEEXT_EDT_VNear;
      break;
    }
  }
  if (properties_.style_exts_ & FWL_STYLEEXT_DTP_EditJustified) {
    dwAdd |= FWL_STYLEEXT_EDT_Justified;
  }

  edit_->ModifyStyleExts(dwAdd, FWL_STYLEEXT_EDT_HAlignMask |
                                    FWL_STYLEEXT_EDT_HAlignModeMask |
                                    FWL_STYLEEXT_EDT_VAlignMask);
}

void CFWL_DateTimePicker::ProcessSelChanged(int32_t iYear,
                                            int32_t iMonth,
                                            int32_t iDay) {
  year_ = iYear;
  month_ = iMonth;
  day_ = iDay;
  edit_->SetText(FormatDateString(year_, month_, day_));
  edit_->Update();
  RepaintRect(client_rect_);

  CFWL_EventSelectChanged ev(this, year_, month_, day_);
  DispatchEvent(&ev);
}

bool CFWL_DateTimePicker::NeedsToShowButton() const {
  return properties_.states_ & FWL_STATE_WGT_Focused ||
         month_cal_->GetStates() & FWL_STATE_WGT_Focused ||
         edit_->GetStates() & FWL_STATE_WGT_Focused;
}

void CFWL_DateTimePicker::OnProcessMessage(CFWL_Message* pMessage) {
  switch (pMessage->GetType()) {
    case CFWL_Message::Type::kSetFocus:
      OnFocusGained(pMessage);
      break;
    case CFWL_Message::Type::kKillFocus:
      OnFocusLost(pMessage);
      break;
    case CFWL_Message::Type::kMouse: {
      CFWL_MessageMouse* pMouse = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMouse->cmd_) {
        case CFWL_MessageMouse::MouseCommand::kLeftButtonDown:
          OnLButtonDown(pMouse);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeftButtonUp:
          OnLButtonUp(pMouse);
          break;
        case CFWL_MessageMouse::MouseCommand::kMove:
          OnMouseMove(pMouse);
          break;
        case CFWL_MessageMouse::MouseCommand::kLeave:
          OnMouseLeave(pMouse);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::kKey: {
      if (edit_->GetStates() & FWL_STATE_WGT_Focused) {
        edit_->GetDelegate()->OnProcessMessage(pMessage);
        return;
      }
      break;
    }
    default:
      break;
  }
  // Dst target could be |this|, continue only if not destroyed by above.
  if (pMessage->GetDstTarget())
    CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_DateTimePicker::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                       const CFX_Matrix& matrix) {
  DrawWidget(pGraphics, matrix);
}

void CFWL_DateTimePicker::OnFocusGained(CFWL_Message* pMsg) {
  properties_.states_ |= FWL_STATE_WGT_Focused;
  if (edit_ && !(edit_->GetStyleExts() & FWL_STYLEEXT_EDT_ReadOnly)) {
    btn_rect_ = CFX_RectF(widget_rect_.width, 0, btn_, widget_rect_.height - 1);
  }
  CFX_RectF rtInvalidate(btn_rect_);
  pMsg->SetDstTarget(edit_);
  edit_->GetDelegate()->OnProcessMessage(pMsg);
  rtInvalidate.Inflate(2, 2);
  RepaintRect(rtInvalidate);
}

void CFWL_DateTimePicker::OnFocusLost(CFWL_Message* pMsg) {
  CFX_RectF rtInvalidate(btn_rect_);
  properties_.states_ &= ~FWL_STATE_WGT_Focused;
  btn_rect_ = CFX_RectF();
  HideMonthCalendar();
  if (edit_->GetStates() & FWL_STATE_WGT_Focused) {
    edit_->GetDelegate()->OnProcessMessage(pMsg);
  }
  rtInvalidate.Inflate(2, 2);
  RepaintRect(rtInvalidate);
}

void CFWL_DateTimePicker::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  if (!pMsg)
    return;
  if (!btn_rect_.Contains(pMsg->pos_)) {
    return;
  }

  if (IsMonthCalendarVisible()) {
    HideMonthCalendar();
    return;
  }
  ShowMonthCalendar();
  lbtn_down_ = true;
  RepaintRect(client_rect_);
}

void CFWL_DateTimePicker::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (!pMsg)
    return;

  lbtn_down_ = false;
  if (btn_rect_.Contains(pMsg->pos_)) {
    btn_state_ = CFWL_PartState::kHovered;
  } else {
    btn_state_ = CFWL_PartState::kNormal;
  }
  RepaintRect(btn_rect_);
}

void CFWL_DateTimePicker::OnMouseMove(CFWL_MessageMouse* pMsg) {
  if (!btn_rect_.Contains(pMsg->pos_)) {
    btn_state_ = CFWL_PartState::kNormal;
  }
  RepaintRect(btn_rect_);
}

void CFWL_DateTimePicker::OnMouseLeave(CFWL_MessageMouse* pMsg) {
  if (!pMsg)
    return;
  btn_state_ = CFWL_PartState::kNormal;
  RepaintRect(btn_rect_);
}

void CFWL_DateTimePicker::GetPopupPos(float fMinHeight,
                                      float fMaxHeight,
                                      const CFX_RectF& rtAnchor,
                                      CFX_RectF* pPopupRect) {
  GetWidgetMgr()->GetAdapterPopupPos(this, fMinHeight, fMaxHeight, rtAnchor,
                                     pPopupRect);
}

void CFWL_DateTimePicker::ClearText() {
  edit_->ClearText();
}

void CFWL_DateTimePicker::SelectAll() {
  edit_->SelectAll();
}

void CFWL_DateTimePicker::ClearSelection() {
  edit_->ClearSelection();
}

std::optional<WideString> CFWL_DateTimePicker::Copy() {
  return edit_->Copy();
}

std::optional<WideString> CFWL_DateTimePicker::Cut() {
  return edit_->Cut();
}

bool CFWL_DateTimePicker::Paste(const WideString& wsPaste) {
  return edit_->Paste(wsPaste);
}

bool CFWL_DateTimePicker::Undo() {
  return edit_->Undo();
}

bool CFWL_DateTimePicker::Redo() {
  return edit_->Redo();
}

bool CFWL_DateTimePicker::CanUndo() {
  return edit_->CanUndo();
}

bool CFWL_DateTimePicker::CanRedo() {
  return edit_->CanRedo();
}

}  // namespace pdfium
