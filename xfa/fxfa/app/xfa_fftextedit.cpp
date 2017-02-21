// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/xfa_fftextedit.h"

#include <vector>

#include "xfa/fwl/cfwl_datetimepicker.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_eventcheckword.h"
#include "xfa/fwl/cfwl_eventselectchanged.h"
#include "xfa/fwl/cfwl_eventtextchanged.h"
#include "xfa/fwl/cfwl_eventvalidate.h"
#include "xfa/fwl/cfwl_messagekillfocus.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_messagesetfocus.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/app/xfa_fffield.h"
#include "xfa/fxfa/app/xfa_fwladapter.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/parser/xfa_localevalue.h"
#include "xfa/fxfa/xfa_ffapp.h"
#include "xfa/fxfa/xfa_ffdoc.h"
#include "xfa/fxfa/xfa_ffdocview.h"
#include "xfa/fxfa/xfa_ffpageview.h"
#include "xfa/fxfa/xfa_ffwidget.h"

CXFA_FFTextEdit::CXFA_FFTextEdit(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFField(pDataAcc), m_pOldDelegate(nullptr) {}

CXFA_FFTextEdit::~CXFA_FFTextEdit() {
  if (m_pNormalWidget) {
    CFWL_NoteDriver* pNoteDriver =
        m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
    pNoteDriver->UnregisterEventTarget(m_pNormalWidget);
  }
}

bool CXFA_FFTextEdit::LoadWidget() {
  CFWL_Edit* pFWLEdit = new CFWL_Edit(
      GetFWLApp(), pdfium::MakeUnique<CFWL_WidgetProperties>(), nullptr);
  m_pNormalWidget = pFWLEdit;
  m_pNormalWidget->SetLayoutItem(this);

  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget, m_pNormalWidget);

  m_pOldDelegate = m_pNormalWidget->GetDelegate();
  m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();

  UpdateWidgetProperty();
  CFX_WideString wsText;
  m_pDataAcc->GetValue(wsText, XFA_VALUEPICTURE_Display);
  pFWLEdit->SetText(wsText);
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}
void CXFA_FFTextEdit::UpdateWidgetProperty() {
  CFWL_Edit* pWidget = (CFWL_Edit*)m_pNormalWidget;
  if (!pWidget) {
    return;
  }
  uint32_t dwStyle = 0;
  uint32_t dwExtendedStyle = FWL_STYLEEXT_EDT_ShowScrollbarFocus |
                             FWL_STYLEEXT_EDT_OuterScrollbar |
                             FWL_STYLEEXT_EDT_LastLineHeight;
  dwExtendedStyle |= UpdateUIProperty();
  if (m_pDataAcc->IsMultiLine()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_MultiLine | FWL_STYLEEXT_EDT_WantReturn;
    if (m_pDataAcc->GetVerticalScrollPolicy() != XFA_ATTRIBUTEENUM_Off) {
      dwStyle |= FWL_WGTSTYLE_VScroll;
      dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoVScroll;
    }
  } else if (m_pDataAcc->GetHorizontalScrollPolicy() != XFA_ATTRIBUTEENUM_Off) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoHScroll;
  }
  if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open ||
      !m_pDataAcc->GetDoc()->GetXFADoc()->IsInteractive()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_ReadOnly;
    dwExtendedStyle |= FWL_STYLEEXT_EDT_MultiLine;
  }
  XFA_Element eType = XFA_Element::Unknown;
  int32_t iMaxChars = m_pDataAcc->GetMaxChars(eType);
  if (eType == XFA_Element::ExData) {
    iMaxChars = 0;
  }
  int32_t iNumCells = m_pDataAcc->GetNumberOfCells();
  if (iNumCells == 0) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetLimit(iMaxChars > 0 ? iMaxChars : 1);
  } else if (iNumCells > 0) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetLimit(iNumCells);
  } else {
    pWidget->SetLimit(iMaxChars);
  }
  dwExtendedStyle |= GetAlignment();
  m_pNormalWidget->ModifyStyles(dwStyle, 0xFFFFFFFF);
  m_pNormalWidget->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
}

bool CXFA_FFTextEdit::OnLButtonDown(uint32_t dwFlags, const CFX_PointF& point) {
  if (!PtInActiveRect(point))
    return false;
  if (!IsFocused()) {
    m_dwStatus |= XFA_WidgetStatus_Focused;
    UpdateFWLData();
    AddInvalidateRect();
  }

  SetButtonDown(true);
  CFWL_MessageMouse ms(nullptr, m_pNormalWidget);
  ms.m_dwCmd = FWL_MouseCommand::LeftButtonDown;
  ms.m_dwFlags = dwFlags;
  ms.m_pos = FWLToClient(point);
  TranslateFWLMessage(&ms);
  return true;
}

bool CXFA_FFTextEdit::OnRButtonDown(uint32_t dwFlags, const CFX_PointF& point) {
  if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open)
    return false;
  if (!PtInActiveRect(point))
    return false;
  if (!IsFocused()) {
    m_dwStatus |= XFA_WidgetStatus_Focused;
    UpdateFWLData();
    AddInvalidateRect();
  }

  SetButtonDown(true);
  CFWL_MessageMouse ms(nullptr, nullptr);
  ms.m_dwCmd = FWL_MouseCommand::RightButtonDown;
  ms.m_dwFlags = dwFlags;
  ms.m_pos = FWLToClient(point);
  TranslateFWLMessage(&ms);
  return true;
}

bool CXFA_FFTextEdit::OnRButtonUp(uint32_t dwFlags, const CFX_PointF& point) {
  if (!CXFA_FFField::OnRButtonUp(dwFlags, point))
    return false;

  GetDoc()->GetDocEnvironment()->PopupMenu(this, point);
  return true;
}

bool CXFA_FFTextEdit::OnSetFocus(CXFA_FFWidget* pOldWidget) {
  m_dwStatus &= ~XFA_WidgetStatus_TextEditValueChanged;
  if (!IsFocused()) {
    m_dwStatus |= XFA_WidgetStatus_Focused;
    UpdateFWLData();
    AddInvalidateRect();
  }
  CXFA_FFWidget::OnSetFocus(pOldWidget);
  CFWL_MessageSetFocus ms(nullptr, m_pNormalWidget);
  TranslateFWLMessage(&ms);
  return true;
}
bool CXFA_FFTextEdit::OnKillFocus(CXFA_FFWidget* pNewWidget) {
  CFWL_MessageKillFocus ms(nullptr, m_pNormalWidget);
  TranslateFWLMessage(&ms);
  m_dwStatus &= ~XFA_WidgetStatus_Focused;
  SetEditScrollOffset();
  ProcessCommittedData();
  UpdateFWLData();
  AddInvalidateRect();
  CXFA_FFWidget::OnKillFocus(pNewWidget);
  m_dwStatus &= ~XFA_WidgetStatus_TextEditValueChanged;
  return true;
}
bool CXFA_FFTextEdit::CommitData() {
  CFX_WideString wsText = static_cast<CFWL_Edit*>(m_pNormalWidget)->GetText();
  if (m_pDataAcc->SetValue(wsText, XFA_VALUEPICTURE_Edit)) {
    m_pDataAcc->UpdateUIDisplay(this);
    return true;
  }
  ValidateNumberField(wsText);
  return false;
}
void CXFA_FFTextEdit::ValidateNumberField(const CFX_WideString& wsText) {
  CXFA_WidgetAcc* pAcc = GetDataAcc();
  if (pAcc && pAcc->GetUIType() == XFA_Element::NumericEdit) {
    IXFA_AppProvider* pAppProvider = GetApp()->GetAppProvider();
    if (pAppProvider) {
      CFX_WideString wsSomField;
      pAcc->GetNode()->GetSOMExpression(wsSomField);

      CFX_WideString wsMessage;
      wsMessage.Format(L"%s can not contain %s", wsText.c_str(),
                       wsSomField.c_str());
      pAppProvider->MsgBox(wsMessage, pAppProvider->GetAppTitle(),
                           XFA_MBICON_Error, XFA_MB_OK);
    }
  }
}
bool CXFA_FFTextEdit::IsDataChanged() {
  return (m_dwStatus & XFA_WidgetStatus_TextEditValueChanged) != 0;
}
uint32_t CXFA_FFTextEdit::GetAlignment() {
  uint32_t dwExtendedStyle = 0;
  if (CXFA_Para para = m_pDataAcc->GetPara()) {
    int32_t iHorz = para.GetHorizontalAlign();
    switch (iHorz) {
      case XFA_ATTRIBUTEENUM_Center:
        dwExtendedStyle |= FWL_STYLEEXT_EDT_HCenter;
        break;
      case XFA_ATTRIBUTEENUM_Justify:
        dwExtendedStyle |= FWL_STYLEEXT_EDT_Justified;
        break;
      case XFA_ATTRIBUTEENUM_JustifyAll:
        break;
      case XFA_ATTRIBUTEENUM_Radix:
        break;
      case XFA_ATTRIBUTEENUM_Right:
        dwExtendedStyle |= FWL_STYLEEXT_EDT_HFar;
        break;
      default:
        dwExtendedStyle |= FWL_STYLEEXT_EDT_HNear;
        break;
    }
    int32_t iVert = para.GetVerticalAlign();
    switch (iVert) {
      case XFA_ATTRIBUTEENUM_Middle:
        dwExtendedStyle |= FWL_STYLEEXT_EDT_VCenter;
        break;
      case XFA_ATTRIBUTEENUM_Bottom:
        dwExtendedStyle |= FWL_STYLEEXT_EDT_VFar;
        break;
      default:
        dwExtendedStyle |= FWL_STYLEEXT_EDT_VNear;
        break;
    }
  }
  return dwExtendedStyle;
}
bool CXFA_FFTextEdit::UpdateFWLData() {
  if (!m_pNormalWidget) {
    return false;
  }
  XFA_VALUEPICTURE eType = XFA_VALUEPICTURE_Display;
  if (IsFocused()) {
    eType = XFA_VALUEPICTURE_Edit;
  }
  bool bUpdate = false;
  if (m_pDataAcc->GetUIType() == XFA_Element::TextEdit &&
      m_pDataAcc->GetNumberOfCells() < 0) {
    XFA_Element elementType = XFA_Element::Unknown;
    int32_t iMaxChars = m_pDataAcc->GetMaxChars(elementType);
    if (elementType == XFA_Element::ExData) {
      iMaxChars = eType == XFA_VALUEPICTURE_Edit ? iMaxChars : 0;
    }
    if (((CFWL_Edit*)m_pNormalWidget)->GetLimit() != iMaxChars) {
      ((CFWL_Edit*)m_pNormalWidget)->SetLimit(iMaxChars);
      bUpdate = true;
    }
  }
  if (m_pDataAcc->GetUIType() == XFA_Element::Barcode) {
    int32_t nDataLen = 0;
    if (eType == XFA_VALUEPICTURE_Edit)
      m_pDataAcc->GetBarcodeAttribute_DataLength(nDataLen);
    static_cast<CFWL_Edit*>(m_pNormalWidget)->SetLimit(nDataLen);
    bUpdate = true;
  }
  CFX_WideString wsText;
  m_pDataAcc->GetValue(wsText, eType);
  CFX_WideString wsOldText =
      static_cast<CFWL_Edit*>(m_pNormalWidget)->GetText();
  if (wsText != wsOldText || (eType == XFA_VALUEPICTURE_Edit && bUpdate)) {
    ((CFWL_Edit*)m_pNormalWidget)->SetText(wsText);
    bUpdate = true;
  }
  if (bUpdate) {
    m_pNormalWidget->Update();
  }
  return true;
}
void CXFA_FFTextEdit::OnTextChanged(CFWL_Widget* pWidget,
                                    const CFX_WideString& wsChanged,
                                    const CFX_WideString& wsPrevText) {
  m_dwStatus |= XFA_WidgetStatus_TextEditValueChanged;
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Change;
  eParam.m_wsChange = wsChanged;
  eParam.m_pTarget = m_pDataAcc;
  eParam.m_wsPrevText = wsPrevText;
  CFWL_Edit* pEdit = ((CFWL_Edit*)m_pNormalWidget);
  if (m_pDataAcc->GetUIType() == XFA_Element::DateTimeEdit) {
    CFWL_DateTimePicker* pDateTime = (CFWL_DateTimePicker*)pEdit;
    eParam.m_wsNewText = pDateTime->GetEditText();
    int32_t iSels = pDateTime->CountSelRanges();
    if (iSels) {
      eParam.m_iSelEnd = pDateTime->GetSelRange(0, &eParam.m_iSelStart);
    }
  } else {
    eParam.m_wsNewText = pEdit->GetText();
    int32_t iSels = pEdit->CountSelRanges();
    if (iSels) {
      eParam.m_iSelEnd = pEdit->GetSelRange(0, &eParam.m_iSelStart);
    }
  }
  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_Change, &eParam);
}
void CXFA_FFTextEdit::OnTextFull(CFWL_Widget* pWidget) {
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Full;
  eParam.m_pTarget = m_pDataAcc;
  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_Full, &eParam);
}

bool CXFA_FFTextEdit::CheckWord(const CFX_ByteStringC& sWord) {
  if (sWord.IsEmpty() || m_pDataAcc->GetUIType() != XFA_Element::TextEdit)
    return true;
  return false;
}

void CXFA_FFTextEdit::OnProcessMessage(CFWL_Message* pMessage) {
  m_pOldDelegate->OnProcessMessage(pMessage);
}

void CXFA_FFTextEdit::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  switch (pEvent->GetType()) {
    case CFWL_Event::Type::TextChanged: {
      CFWL_EventTextChanged* event =
          static_cast<CFWL_EventTextChanged*>(pEvent);
      CFX_WideString wsChange;
      OnTextChanged(m_pNormalWidget, wsChange, event->wsPrevText);
      break;
    }
    case CFWL_Event::Type::TextFull: {
      OnTextFull(m_pNormalWidget);
      break;
    }
    case CFWL_Event::Type::CheckWord: {
      CFX_WideString wstr(L"FWL_EVENT_DTP_SelectChanged");
      CFWL_EventCheckWord* event = static_cast<CFWL_EventCheckWord*>(pEvent);
      event->bCheckWord = CheckWord(event->bsWord.AsStringC());
      break;
    }
    default:
      break;
  }
  m_pOldDelegate->OnProcessEvent(pEvent);
}

void CXFA_FFTextEdit::OnDrawWidget(CFX_Graphics* pGraphics,
                                   const CFX_Matrix* pMatrix) {
  m_pOldDelegate->OnDrawWidget(pGraphics, pMatrix);
}

CXFA_FFNumericEdit::CXFA_FFNumericEdit(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFTextEdit(pDataAcc) {}

CXFA_FFNumericEdit::~CXFA_FFNumericEdit() {}

bool CXFA_FFNumericEdit::LoadWidget() {
  CFWL_Edit* pWidget = new CFWL_Edit(
      GetFWLApp(), pdfium::MakeUnique<CFWL_WidgetProperties>(), nullptr);
  m_pNormalWidget = pWidget;

  m_pNormalWidget->SetLayoutItem(this);
  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget, m_pNormalWidget);

  m_pOldDelegate = m_pNormalWidget->GetDelegate();
  m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();

  CFX_WideString wsText;
  m_pDataAcc->GetValue(wsText, XFA_VALUEPICTURE_Display);
  pWidget->SetText(wsText);
  UpdateWidgetProperty();
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}
void CXFA_FFNumericEdit::UpdateWidgetProperty() {
  CFWL_Edit* pWidget = (CFWL_Edit*)m_pNormalWidget;
  if (!pWidget) {
    return;
  }
  uint32_t dwExtendedStyle =
      FWL_STYLEEXT_EDT_ShowScrollbarFocus | FWL_STYLEEXT_EDT_OuterScrollbar |
      FWL_STYLEEXT_EDT_Validate | FWL_STYLEEXT_EDT_Number |
      FWL_STYLEEXT_EDT_LastLineHeight;
  dwExtendedStyle |= UpdateUIProperty();
  if (m_pDataAcc->GetHorizontalScrollPolicy() != XFA_ATTRIBUTEENUM_Off) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoHScroll;
  }
  int32_t iNumCells = m_pDataAcc->GetNumberOfCells();
  if (iNumCells > 0) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetLimit(iNumCells);
  }
  dwExtendedStyle |= GetAlignment();
  if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open ||
      !m_pDataAcc->GetDoc()->GetXFADoc()->IsInteractive()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_ReadOnly;
  }
  m_pNormalWidget->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
}

void CXFA_FFNumericEdit::OnProcessEvent(CFWL_Event* pEvent) {
  if (pEvent->GetType() == CFWL_Event::Type::Validate) {
    CFWL_EventValidate* event = static_cast<CFWL_EventValidate*>(pEvent);
    event->bValidate = OnValidate(m_pNormalWidget, event->wsInsert);
    return;
  }
  CXFA_FFTextEdit::OnProcessEvent(pEvent);
}

bool CXFA_FFNumericEdit::OnValidate(CFWL_Widget* pWidget,
                                    CFX_WideString& wsText) {
  CFX_WideString wsPattern;
  m_pDataAcc->GetPictureContent(wsPattern, XFA_VALUEPICTURE_Edit);
  if (!wsPattern.IsEmpty()) {
    return true;
  }
  int32_t iLeads = 0;
  m_pDataAcc->GetLeadDigits(iLeads);
  int32_t iFracs = 0;
  m_pDataAcc->GetFracDigits(iFracs);
  CFX_WideString wsFormat;
  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(m_pDataAcc);
  widgetValue.GetNumbericFormat(wsFormat, iLeads, iFracs);
  return widgetValue.ValidateNumericTemp(wsText, wsFormat,
                                         m_pDataAcc->GetLocal());
}

CXFA_FFPasswordEdit::CXFA_FFPasswordEdit(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFTextEdit(pDataAcc) {}

CXFA_FFPasswordEdit::~CXFA_FFPasswordEdit() {}

bool CXFA_FFPasswordEdit::LoadWidget() {
  CFWL_Edit* pWidget = new CFWL_Edit(
      GetFWLApp(), pdfium::MakeUnique<CFWL_WidgetProperties>(), nullptr);
  m_pNormalWidget = pWidget;
  m_pNormalWidget->SetLayoutItem(this);

  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget, m_pNormalWidget);

  m_pOldDelegate = m_pNormalWidget->GetDelegate();
  m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();

  CFX_WideString wsText;
  m_pDataAcc->GetValue(wsText, XFA_VALUEPICTURE_Display);
  pWidget->SetText(wsText);
  UpdateWidgetProperty();
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}
void CXFA_FFPasswordEdit::UpdateWidgetProperty() {
  CFWL_Edit* pWidget = (CFWL_Edit*)m_pNormalWidget;
  if (!pWidget) {
    return;
  }
  uint32_t dwExtendedStyle =
      FWL_STYLEEXT_EDT_ShowScrollbarFocus | FWL_STYLEEXT_EDT_OuterScrollbar |
      FWL_STYLEEXT_EDT_Password | FWL_STYLEEXT_EDT_LastLineHeight;
  dwExtendedStyle |= UpdateUIProperty();
  CFX_WideString wsPassWord;
  m_pDataAcc->GetPasswordChar(wsPassWord);
  if (!wsPassWord.IsEmpty()) {
    pWidget->SetAliasChar(wsPassWord.GetAt(0));
  }
  if (m_pDataAcc->GetHorizontalScrollPolicy() != XFA_ATTRIBUTEENUM_Off) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_AutoHScroll;
  }
  if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open ||
      !m_pDataAcc->GetDoc()->GetXFADoc()->IsInteractive()) {
    dwExtendedStyle |= FWL_STYLEEXT_EDT_ReadOnly;
  }
  dwExtendedStyle |= GetAlignment();
  m_pNormalWidget->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
}
CXFA_FFDateTimeEdit::CXFA_FFDateTimeEdit(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFTextEdit(pDataAcc) {}

CXFA_FFDateTimeEdit::~CXFA_FFDateTimeEdit() {}

CFX_RectF CXFA_FFDateTimeEdit::GetBBox(uint32_t dwStatus, bool bDrawFocus) {
  if (bDrawFocus)
    return CFX_RectF();
  return CXFA_FFWidget::GetBBox(dwStatus);
}

bool CXFA_FFDateTimeEdit::PtInActiveRect(const CFX_PointF& point) {
  return m_pNormalWidget &&
         static_cast<CFWL_DateTimePicker*>(m_pNormalWidget)
             ->GetBBox()
             .Contains(point);
}

bool CXFA_FFDateTimeEdit::LoadWidget() {
  CFWL_DateTimePicker* pWidget = new CFWL_DateTimePicker(GetFWLApp());
  m_pNormalWidget = pWidget;
  m_pNormalWidget->SetLayoutItem(this);
  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget, m_pNormalWidget);

  m_pOldDelegate = m_pNormalWidget->GetDelegate();
  m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();

  CFX_WideString wsText;
  m_pDataAcc->GetValue(wsText, XFA_VALUEPICTURE_Display);
  pWidget->SetEditText(wsText);
  if (CXFA_Value value = m_pDataAcc->GetFormValue()) {
    switch (value.GetChildValueClassID()) {
      case XFA_Element::Date: {
        if (!wsText.IsEmpty()) {
          CXFA_LocaleValue lcValue = XFA_GetLocaleValue(m_pDataAcc);
          CFX_Unitime date = lcValue.GetDate();
          if ((FX_UNITIME)date != 0) {
            pWidget->SetCurSel(date.GetYear(), date.GetMonth(), date.GetDay());
          }
        }
      } break;
      default:
        break;
    }
  }
  UpdateWidgetProperty();
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}
void CXFA_FFDateTimeEdit::UpdateWidgetProperty() {
  CFWL_DateTimePicker* pWidget = (CFWL_DateTimePicker*)m_pNormalWidget;
  if (!pWidget) {
    return;
  }
  uint32_t dwExtendedStyle = FWL_STYLEEXT_DTP_ShortDateFormat;
  dwExtendedStyle |= UpdateUIProperty();
  dwExtendedStyle |= GetAlignment();
  m_pNormalWidget->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
  uint32_t dwEditStyles = FWL_STYLEEXT_EDT_LastLineHeight;
  int32_t iNumCells = m_pDataAcc->GetNumberOfCells();
  if (iNumCells > 0) {
    dwEditStyles |= FWL_STYLEEXT_EDT_CombText;
    pWidget->SetEditLimit(iNumCells);
  }
  if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open ||
      !m_pDataAcc->GetDoc()->GetXFADoc()->IsInteractive()) {
    dwEditStyles |= FWL_STYLEEXT_EDT_ReadOnly;
  }
  if (m_pDataAcc->GetHorizontalScrollPolicy() != XFA_ATTRIBUTEENUM_Off) {
    dwEditStyles |= FWL_STYLEEXT_EDT_AutoHScroll;
  }
  pWidget->ModifyEditStylesEx(dwEditStyles, 0xFFFFFFFF);
}
uint32_t CXFA_FFDateTimeEdit::GetAlignment() {
  uint32_t dwExtendedStyle = 0;
  if (CXFA_Para para = m_pDataAcc->GetPara()) {
    int32_t iHorz = para.GetHorizontalAlign();
    switch (iHorz) {
      case XFA_ATTRIBUTEENUM_Center:
        dwExtendedStyle |= FWL_STYLEEXT_DTP_EditHCenter;
        break;
      case XFA_ATTRIBUTEENUM_Justify:
        dwExtendedStyle |= FWL_STYLEEXT_DTP_EditJustified;
        break;
      case XFA_ATTRIBUTEENUM_JustifyAll:
        break;
      case XFA_ATTRIBUTEENUM_Radix:
        break;
      case XFA_ATTRIBUTEENUM_Right:
        dwExtendedStyle |= FWL_STYLEEXT_DTP_EditHFar;
        break;
      default:
        dwExtendedStyle |= FWL_STYLEEXT_DTP_EditHNear;
        break;
    }
    int32_t iVert = para.GetVerticalAlign();
    switch (iVert) {
      case XFA_ATTRIBUTEENUM_Middle:
        dwExtendedStyle |= FWL_STYLEEXT_DTP_EditVCenter;
        break;
      case XFA_ATTRIBUTEENUM_Bottom:
        dwExtendedStyle |= FWL_STYLEEXT_DTP_EditVFar;
        break;
      default:
        dwExtendedStyle |= FWL_STYLEEXT_DTP_EditVNear;
        break;
    }
  }
  return dwExtendedStyle;
}
bool CXFA_FFDateTimeEdit::CommitData() {
  CFX_WideString wsText =
      static_cast<CFWL_DateTimePicker*>(m_pNormalWidget)->GetEditText();
  if (m_pDataAcc->SetValue(wsText, XFA_VALUEPICTURE_Edit)) {
    m_pDataAcc->UpdateUIDisplay(this);
    return true;
  }
  return false;
}
bool CXFA_FFDateTimeEdit::UpdateFWLData() {
  if (!m_pNormalWidget) {
    return false;
  }
  XFA_VALUEPICTURE eType = XFA_VALUEPICTURE_Display;
  if (IsFocused()) {
    eType = XFA_VALUEPICTURE_Edit;
  }
  CFX_WideString wsText;
  m_pDataAcc->GetValue(wsText, eType);
  ((CFWL_DateTimePicker*)m_pNormalWidget)->SetEditText(wsText);
  if (IsFocused() && !wsText.IsEmpty()) {
    CXFA_LocaleValue lcValue = XFA_GetLocaleValue(m_pDataAcc);
    CFX_Unitime date = lcValue.GetDate();
    if (lcValue.IsValid()) {
      if ((FX_UNITIME)date != 0) {
        ((CFWL_DateTimePicker*)m_pNormalWidget)
            ->SetCurSel(date.GetYear(), date.GetMonth(), date.GetDay());
      }
    }
  }
  m_pNormalWidget->Update();
  return true;
}
bool CXFA_FFDateTimeEdit::IsDataChanged() {
  if (m_dwStatus & XFA_WidgetStatus_TextEditValueChanged) {
    return true;
  }
  CFX_WideString wsText =
      static_cast<CFWL_DateTimePicker*>(m_pNormalWidget)->GetEditText();
  CFX_WideString wsOldValue;
  m_pDataAcc->GetValue(wsOldValue, XFA_VALUEPICTURE_Edit);
  return wsOldValue != wsText;
}

void CXFA_FFDateTimeEdit::OnSelectChanged(CFWL_Widget* pWidget,
                                          int32_t iYear,
                                          int32_t iMonth,
                                          int32_t iDay) {
  CFX_WideString wsPicture;
  m_pDataAcc->GetPictureContent(wsPicture, XFA_VALUEPICTURE_Edit);
  CXFA_LocaleValue date(XFA_VT_DATE, GetDoc()->GetXFADoc()->GetLocalMgr());
  CFX_Unitime dt;
  dt.Set(iYear, iMonth, iDay);
  date.SetDate(dt);
  CFX_WideString wsDate;
  date.FormatPatterns(wsDate, wsPicture, m_pDataAcc->GetLocal(),
                      XFA_VALUEPICTURE_Edit);
  CFWL_DateTimePicker* pDateTime = (CFWL_DateTimePicker*)m_pNormalWidget;
  pDateTime->SetEditText(wsDate);
  pDateTime->Update();
  GetDoc()->GetDocEnvironment()->SetFocusWidget(GetDoc(), nullptr);
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Change;
  eParam.m_pTarget = m_pDataAcc;
  m_pDataAcc->GetValue(eParam.m_wsNewText, XFA_VALUEPICTURE_Raw);
  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_Change, &eParam);
}

void CXFA_FFDateTimeEdit::OnProcessEvent(CFWL_Event* pEvent) {
  if (pEvent->GetType() == CFWL_Event::Type::SelectChanged) {
    CFWL_EventSelectChanged* event =
        static_cast<CFWL_EventSelectChanged*>(pEvent);
    OnSelectChanged(m_pNormalWidget, event->iYear, event->iMonth, event->iDay);
    return;
  }
  CXFA_FFTextEdit::OnProcessEvent(pEvent);
}
