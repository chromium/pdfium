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
      m_pEdit(cppgc::MakeGarbageCollected<CFWL_DateTimeEdit>(
          app->GetHeap()->GetAllocationHandle(),
          app,
          Properties(),
          this)),
      m_pMonthCal(cppgc::MakeGarbageCollected<CFWL_MonthCalendar>(
          app->GetHeap()->GetAllocationHandle(),
          app,
          Properties{FWL_STYLE_WGT_Popup | FWL_STYLE_WGT_Border, 0,
                     FWL_STATE_WGT_Invisible},
          this)) {
  m_pMonthCal->SetWidgetRect(
      CFX_RectF(0, 0, m_pMonthCal->GetAutosizedWidgetRect().Size()));

  CFWL_NoteDriver* pNoteDriver = GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(this, m_pMonthCal);
  pNoteDriver->RegisterEventTarget(this, m_pEdit);
}

CFWL_DateTimePicker::~CFWL_DateTimePicker() = default;

void CFWL_DateTimePicker::PreFinalize() {
  UnregisterEventTarget();
  CFWL_Widget::PreFinalize();
}

void CFWL_DateTimePicker::Trace(cppgc::Visitor* visitor) const {
  CFWL_Widget::Trace(visitor);
  visitor->Trace(m_pEdit);
  visitor->Trace(m_pMonthCal);
}

FWL_Type CFWL_DateTimePicker::GetClassID() const {
  return FWL_Type::DateTimePicker;
}

void CFWL_DateTimePicker::Update() {
  if (IsLocked())
    return;

  m_ClientRect = GetClientRect();
  m_pEdit->SetWidgetRect(m_ClientRect);
  ResetEditAlignment();
  m_pEdit->Update();

  m_fBtn = GetThemeProvider()->GetScrollBarWidth();
  CFX_RectF rtMonthCal = m_pMonthCal->GetAutosizedWidgetRect();
  CFX_RectF rtPopUp(rtMonthCal.left, rtMonthCal.top + kDateTimePickerHeight,
                    rtMonthCal.width, rtMonthCal.height);
  m_pMonthCal->SetWidgetRect(rtPopUp);
  m_pMonthCal->Update();
}

FWL_WidgetHit CFWL_DateTimePicker::HitTest(const CFX_PointF& point) {
  CFX_RectF rect(0, 0, m_WidgetRect.width, m_WidgetRect.height);
  if (rect.Contains(point))
    return FWL_WidgetHit::Edit;
  if (NeedsToShowButton())
    rect.width += m_fBtn;
  if (rect.Contains(point))
    return FWL_WidgetHit::Client;
  if (IsMonthCalendarVisible()) {
    if (m_pMonthCal->GetWidgetRect().Contains(point))
      return FWL_WidgetHit::Client;
  }
  return FWL_WidgetHit::Unknown;
}

void CFWL_DateTimePicker::DrawWidget(CFGAS_GEGraphics* pGraphics,
                                     const CFX_Matrix& matrix) {
  if (!pGraphics)
    return;

  if (HasBorder())
    DrawBorder(pGraphics, CFWL_ThemePart::Part::kBorder, matrix);

  if (!m_BtnRect.IsEmpty())
    DrawDropDownButton(pGraphics, matrix);

  if (m_pEdit) {
    CFX_RectF rtEdit = m_pEdit->GetWidgetRect();
    CFX_Matrix mt(1, 0, 0, 1, rtEdit.left, rtEdit.top);
    mt.Concat(matrix);
    m_pEdit->DrawWidget(pGraphics, mt);
  }
  if (!IsMonthCalendarVisible())
    return;

  CFX_RectF rtMonth = m_pMonthCal->GetWidgetRect();
  CFX_Matrix mt(1, 0, 0, 1, rtMonth.left, rtMonth.top);
  mt.Concat(matrix);
  m_pMonthCal->DrawWidget(pGraphics, mt);
}

void CFWL_DateTimePicker::GetCurSel(int32_t& iYear,
                                    int32_t& iMonth,
                                    int32_t& iDay) {
  iYear = m_iYear;
  iMonth = m_iMonth;
  iDay = m_iDay;
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

  m_iYear = iYear;
  m_iMonth = iMonth;
  m_iDay = iDay;
  m_pMonthCal->SetSelect(iYear, iMonth, iDay);
}

void CFWL_DateTimePicker::SetEditText(const WideString& wsText) {
  if (!m_pEdit)
    return;

  m_pEdit->SetText(wsText);
  RepaintRect(m_ClientRect);

  CFWL_Event ev(CFWL_Event::Type::EditChanged);
  DispatchEvent(&ev);
}

WideString CFWL_DateTimePicker::GetEditText() const {
  return m_pEdit ? m_pEdit->GetText() : WideString();
}

size_t CFWL_DateTimePicker::GetEditTextLength() const {
  return m_pEdit ? m_pEdit->GetTextLength() : 0;
}

CFX_RectF CFWL_DateTimePicker::GetBBox() const {
  CFX_RectF rect = m_WidgetRect;
  if (NeedsToShowButton())
    rect.width += m_fBtn;
  if (!IsMonthCalendarVisible())
    return rect;

  CFX_RectF rtMonth = m_pMonthCal->GetWidgetRect();
  rtMonth.Offset(m_WidgetRect.left, m_WidgetRect.top);
  rect.Union(rtMonth);
  return rect;
}

void CFWL_DateTimePicker::ModifyEditStyleExts(uint32_t dwStyleExtsAdded,
                                              uint32_t dwStyleExtsRemoved) {
  m_pEdit->ModifyStyleExts(dwStyleExtsAdded, dwStyleExtsRemoved);
}

void CFWL_DateTimePicker::DrawDropDownButton(CFGAS_GEGraphics* pGraphics,
                                             const CFX_Matrix& mtMatrix) {
  CFWL_ThemeBackground param(CFWL_ThemePart::Part::kDropDownButton, this,
                             pGraphics);
  param.m_dwStates = m_iBtnState;
  param.m_PartRect = m_BtnRect;
  param.m_matrix = mtMatrix;
  GetThemeProvider()->DrawBackground(param);
}

WideString CFWL_DateTimePicker::FormatDateString(int32_t iYear,
                                                 int32_t iMonth,
                                                 int32_t iDay) {
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_DTP_ShortDateFormat)
    return WideString::Format(L"%d-%d-%d", iYear, iMonth, iDay);

  return WideString::Format(L"%d Year %d Month %d Day", iYear, iMonth, iDay);
}

void CFWL_DateTimePicker::ShowMonthCalendar() {
  if (IsMonthCalendarVisible())
    return;

  CFX_RectF rtMonthCal = m_pMonthCal->GetAutosizedWidgetRect();
  float fPopupMin = rtMonthCal.height;
  float fPopupMax = rtMonthCal.height;
  CFX_RectF rtAnchor = m_WidgetRect;
  rtAnchor.width = rtMonthCal.width;
  rtMonthCal.left = m_ClientRect.left;
  rtMonthCal.top = rtAnchor.Height();
  GetPopupPos(fPopupMin, fPopupMax, rtAnchor, &rtMonthCal);
  m_pMonthCal->SetWidgetRect(rtMonthCal);
  if (m_iYear > 0 && m_iMonth > 0 && m_iDay > 0)
    m_pMonthCal->SetSelect(m_iYear, m_iMonth, m_iDay);
  m_pMonthCal->Update();
  m_pMonthCal->RemoveStates(FWL_STATE_WGT_Invisible);

  CFWL_MessageSetFocus msg(m_pMonthCal);
  m_pEdit->GetDelegate()->OnProcessMessage(&msg);
  RepaintInflatedMonthCalRect();
}

void CFWL_DateTimePicker::HideMonthCalendar() {
  if (!IsMonthCalendarVisible())
    return;

  m_pMonthCal->SetStates(FWL_STATE_WGT_Invisible);
  RepaintInflatedMonthCalRect();
}

void CFWL_DateTimePicker::RepaintInflatedMonthCalRect() {
  CFX_RectF rtInvalidate(0, 0, m_WidgetRect.width, m_WidgetRect.height);
  CFX_RectF rtCal = m_pMonthCal->GetWidgetRect();
  rtInvalidate.Union(rtCal);
  rtInvalidate.Inflate(2, 2);
  RepaintRect(rtInvalidate);
}

bool CFWL_DateTimePicker::IsMonthCalendarVisible() const {
  return m_pMonthCal && m_pMonthCal->IsVisible();
}

void CFWL_DateTimePicker::ResetEditAlignment() {
  if (!m_pEdit)
    return;

  uint32_t dwAdd = 0;
  switch (m_Properties.m_dwStyleExts & FWL_STYLEEXT_DTP_EditHAlignMask) {
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
  switch (m_Properties.m_dwStyleExts & FWL_STYLEEXT_DTP_EditVAlignMask) {
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
  if (m_Properties.m_dwStyleExts & FWL_STYLEEXT_DTP_EditJustified)
    dwAdd |= FWL_STYLEEXT_EDT_Justified;

  m_pEdit->ModifyStyleExts(dwAdd, FWL_STYLEEXT_EDT_HAlignMask |
                                      FWL_STYLEEXT_EDT_HAlignModeMask |
                                      FWL_STYLEEXT_EDT_VAlignMask);
}

void CFWL_DateTimePicker::ProcessSelChanged(int32_t iYear,
                                            int32_t iMonth,
                                            int32_t iDay) {
  m_iYear = iYear;
  m_iMonth = iMonth;
  m_iDay = iDay;
  m_pEdit->SetText(FormatDateString(m_iYear, m_iMonth, m_iDay));
  m_pEdit->Update();
  RepaintRect(m_ClientRect);

  CFWL_EventSelectChanged ev(this, m_iYear, m_iMonth, m_iDay);
  DispatchEvent(&ev);
}

bool CFWL_DateTimePicker::NeedsToShowButton() const {
  return m_Properties.m_dwStates & FWL_STATE_WGT_Focused ||
         m_pMonthCal->GetStates() & FWL_STATE_WGT_Focused ||
         m_pEdit->GetStates() & FWL_STATE_WGT_Focused;
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
      switch (pMouse->m_dwCmd) {
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
      if (m_pEdit->GetStates() & FWL_STATE_WGT_Focused) {
        m_pEdit->GetDelegate()->OnProcessMessage(pMessage);
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
  m_Properties.m_dwStates |= FWL_STATE_WGT_Focused;
  if (m_pEdit && !(m_pEdit->GetStyleExts() & FWL_STYLEEXT_EDT_ReadOnly)) {
    m_BtnRect =
        CFX_RectF(m_WidgetRect.width, 0, m_fBtn, m_WidgetRect.height - 1);
  }
  CFX_RectF rtInvalidate(m_BtnRect);
  pMsg->SetDstTarget(m_pEdit);
  m_pEdit->GetDelegate()->OnProcessMessage(pMsg);
  rtInvalidate.Inflate(2, 2);
  RepaintRect(rtInvalidate);
}

void CFWL_DateTimePicker::OnFocusLost(CFWL_Message* pMsg) {
  CFX_RectF rtInvalidate(m_BtnRect);
  m_Properties.m_dwStates &= ~FWL_STATE_WGT_Focused;
  m_BtnRect = CFX_RectF();
  HideMonthCalendar();
  if (m_pEdit->GetStates() & FWL_STATE_WGT_Focused)
    m_pEdit->GetDelegate()->OnProcessMessage(pMsg);
  rtInvalidate.Inflate(2, 2);
  RepaintRect(rtInvalidate);
}

void CFWL_DateTimePicker::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  if (!pMsg)
    return;
  if (!m_BtnRect.Contains(pMsg->m_pos))
    return;

  if (IsMonthCalendarVisible()) {
    HideMonthCalendar();
    return;
  }
  ShowMonthCalendar();
  m_bLBtnDown = true;
  RepaintRect(m_ClientRect);
}

void CFWL_DateTimePicker::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  if (!pMsg)
    return;

  m_bLBtnDown = false;
  if (m_BtnRect.Contains(pMsg->m_pos))
    m_iBtnState = CFWL_PartState::kHovered;
  else
    m_iBtnState = CFWL_PartState::kNormal;
  RepaintRect(m_BtnRect);
}

void CFWL_DateTimePicker::OnMouseMove(CFWL_MessageMouse* pMsg) {
  if (!m_BtnRect.Contains(pMsg->m_pos))
    m_iBtnState = CFWL_PartState::kNormal;
  RepaintRect(m_BtnRect);
}

void CFWL_DateTimePicker::OnMouseLeave(CFWL_MessageMouse* pMsg) {
  if (!pMsg)
    return;
  m_iBtnState = CFWL_PartState::kNormal;
  RepaintRect(m_BtnRect);
}

void CFWL_DateTimePicker::GetPopupPos(float fMinHeight,
                                      float fMaxHeight,
                                      const CFX_RectF& rtAnchor,
                                      CFX_RectF* pPopupRect) {
  GetWidgetMgr()->GetAdapterPopupPos(this, fMinHeight, fMaxHeight, rtAnchor,
                                     pPopupRect);
}

void CFWL_DateTimePicker::ClearText() {
  m_pEdit->ClearText();
}

void CFWL_DateTimePicker::SelectAll() {
  m_pEdit->SelectAll();
}

void CFWL_DateTimePicker::ClearSelection() {
  m_pEdit->ClearSelection();
}

std::optional<WideString> CFWL_DateTimePicker::Copy() {
  return m_pEdit->Copy();
}

std::optional<WideString> CFWL_DateTimePicker::Cut() {
  return m_pEdit->Cut();
}

bool CFWL_DateTimePicker::Paste(const WideString& wsPaste) {
  return m_pEdit->Paste(wsPaste);
}

bool CFWL_DateTimePicker::Undo() {
  return m_pEdit->Undo();
}

bool CFWL_DateTimePicker::Redo() {
  return m_pEdit->Redo();
}

bool CFWL_DateTimePicker::CanUndo() {
  return m_pEdit->CanUndo();
}

bool CFWL_DateTimePicker::CanRedo() {
  return m_pEdit->CanRedo();
}

}  // namespace pdfium
