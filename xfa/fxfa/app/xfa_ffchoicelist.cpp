// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/app/xfa_ffchoicelist.h"

#include <vector>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_combobox.h"
#include "xfa/fwl/cfwl_edit.h"
#include "xfa/fwl/cfwl_eventselectchanged.h"
#include "xfa/fwl/cfwl_listbox.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widgetproperties.h"
#include "xfa/fxfa/app/xfa_fffield.h"
#include "xfa/fxfa/app/xfa_fwladapter.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/xfa_ffdoc.h"
#include "xfa/fxfa/xfa_ffdocview.h"
#include "xfa/fxfa/xfa_ffpageview.h"
#include "xfa/fxfa/xfa_ffwidget.h"

CXFA_FFListBox::CXFA_FFListBox(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFField(pDataAcc), m_pOldDelegate(nullptr) {}

CXFA_FFListBox::~CXFA_FFListBox() {
  if (m_pNormalWidget) {
    CFWL_NoteDriver* pNoteDriver =
        m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
    pNoteDriver->UnregisterEventTarget(m_pNormalWidget);
  }
}

bool CXFA_FFListBox::LoadWidget() {
  CFWL_ListBox* pListBox = new CFWL_ListBox(
      GetFWLApp(), pdfium::MakeUnique<CFWL_WidgetProperties>(), nullptr);
  pListBox->ModifyStyles(FWL_WGTSTYLE_VScroll | FWL_WGTSTYLE_NoBackground,
                         0xFFFFFFFF);
  m_pNormalWidget = (CFWL_Widget*)pListBox;
  m_pNormalWidget->SetLayoutItem(this);

  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget, m_pNormalWidget);

  m_pOldDelegate = m_pNormalWidget->GetDelegate();
  m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();

  std::vector<CFX_WideString> wsLabelArray;
  m_pDataAcc->GetChoiceListItems(wsLabelArray, false);
  int32_t iItems = pdfium::CollectionSize<int32_t>(wsLabelArray);
  for (int32_t i = 0; i < iItems; i++) {
    pListBox->AddString(wsLabelArray[i].AsStringC());
  }
  uint32_t dwExtendedStyle = FWL_STYLEEXT_LTB_ShowScrollBarFocus;
  if (m_pDataAcc->GetChoiceListOpen() == XFA_ATTRIBUTEENUM_MultiSelect) {
    dwExtendedStyle |= FWL_STYLEEXT_LTB_MultiSelection;
  }
  dwExtendedStyle |= GetAlignment();
  m_pNormalWidget->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
  CFX_ArrayTemplate<int32_t> iSelArray;
  m_pDataAcc->GetSelectedItems(iSelArray);
  int32_t iSelCount = iSelArray.GetSize();
  for (int32_t j = 0; j < iSelCount; j++) {
    CFWL_ListItem* item = pListBox->GetItem(nullptr, iSelArray[j]);
    pListBox->SetSelItem(item, true);
  }
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}

bool CXFA_FFListBox::OnKillFocus(CXFA_FFWidget* pNewFocus) {
  if (!ProcessCommittedData())
    UpdateFWLData();
  CXFA_FFField::OnKillFocus(pNewFocus);
  return true;
}

bool CXFA_FFListBox::CommitData() {
  CFWL_ListBox* pListBox = static_cast<CFWL_ListBox*>(m_pNormalWidget);
  int32_t iSels = pListBox->CountSelItems();
  CFX_ArrayTemplate<int32_t> iSelArray;
  for (int32_t i = 0; i < iSels; ++i)
    iSelArray.Add(pListBox->GetSelIndex(i));
  m_pDataAcc->SetSelectedItems(iSelArray, true, false, true);
  return true;
}

bool CXFA_FFListBox::IsDataChanged() {
  CFX_ArrayTemplate<int32_t> iSelArray;
  m_pDataAcc->GetSelectedItems(iSelArray);
  int32_t iOldSels = iSelArray.GetSize();
  CFWL_ListBox* pListBox = (CFWL_ListBox*)m_pNormalWidget;
  int32_t iSels = pListBox->CountSelItems();
  if (iOldSels != iSels)
    return true;

  for (int32_t i = 0; i < iSels; ++i) {
    CFWL_ListItem* hlistItem = pListBox->GetItem(nullptr, iSelArray[i]);
    if (!(hlistItem->GetStates() & FWL_ITEMSTATE_LTB_Selected))
      return true;
  }
  return false;
}

uint32_t CXFA_FFListBox::GetAlignment() {
  uint32_t dwExtendedStyle = 0;
  if (CXFA_Para para = m_pDataAcc->GetPara()) {
    int32_t iHorz = para.GetHorizontalAlign();
    switch (iHorz) {
      case XFA_ATTRIBUTEENUM_Center:
        dwExtendedStyle |= FWL_STYLEEXT_LTB_CenterAlign;
        break;
      case XFA_ATTRIBUTEENUM_Justify:
        break;
      case XFA_ATTRIBUTEENUM_JustifyAll:
        break;
      case XFA_ATTRIBUTEENUM_Radix:
        break;
      case XFA_ATTRIBUTEENUM_Right:
        dwExtendedStyle |= FWL_STYLEEXT_LTB_RightAlign;
        break;
      default:
        dwExtendedStyle |= FWL_STYLEEXT_LTB_LeftAlign;
        break;
    }
  }
  return dwExtendedStyle;
}
bool CXFA_FFListBox::UpdateFWLData() {
  if (!m_pNormalWidget) {
    return false;
  }
  CFWL_ListBox* pListBox = ((CFWL_ListBox*)m_pNormalWidget);
  CFX_ArrayTemplate<CFWL_ListItem*> selItemArray;
  CFX_ArrayTemplate<int32_t> iSelArray;
  m_pDataAcc->GetSelectedItems(iSelArray);
  int32_t iSelCount = iSelArray.GetSize();
  for (int32_t j = 0; j < iSelCount; j++) {
    CFWL_ListItem* lpItemSel = pListBox->GetSelItem(iSelArray[j]);
    selItemArray.Add(lpItemSel);
  }
  pListBox->SetSelItem(pListBox->GetSelItem(-1), false);
  for (int32_t i = 0; i < iSelCount; i++) {
    ((CFWL_ListBox*)m_pNormalWidget)->SetSelItem(selItemArray[i], true);
  }
  m_pNormalWidget->Update();
  return true;
}
void CXFA_FFListBox::OnSelectChanged(
    CFWL_Widget* pWidget,
    const CFX_ArrayTemplate<int32_t>& arrSels) {
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Change;
  eParam.m_pTarget = m_pDataAcc;
  m_pDataAcc->GetValue(eParam.m_wsPrevText, XFA_VALUEPICTURE_Raw);
  CFWL_ListBox* pListBox = (CFWL_ListBox*)m_pNormalWidget;
  int32_t iSels = pListBox->CountSelItems();
  if (iSels > 0) {
    CFWL_ListItem* item = pListBox->GetSelItem(0);
    eParam.m_wsNewText = item ? item->GetText() : L"";
  }

  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_Change, &eParam);
}
void CXFA_FFListBox::SetItemState(int32_t nIndex, bool bSelected) {
  CFWL_ListItem* item = ((CFWL_ListBox*)m_pNormalWidget)->GetSelItem(nIndex);
  ((CFWL_ListBox*)m_pNormalWidget)->SetSelItem(item, bSelected);
  m_pNormalWidget->Update();
  AddInvalidateRect();
}
void CXFA_FFListBox::InsertItem(const CFX_WideStringC& wsLabel,
                                int32_t nIndex) {
  CFX_WideString wsTemp(wsLabel);
  ((CFWL_ListBox*)m_pNormalWidget)->AddString(wsTemp.AsStringC());
  m_pNormalWidget->Update();
  AddInvalidateRect();
}
void CXFA_FFListBox::DeleteItem(int32_t nIndex) {
  CFWL_ListBox* listBox = static_cast<CFWL_ListBox*>(m_pNormalWidget);
  if (nIndex < 0)
    listBox->DeleteAll();
  else
    listBox->DeleteString(listBox->GetItem(nullptr, nIndex));

  listBox->Update();
  AddInvalidateRect();
}

void CXFA_FFListBox::OnProcessMessage(CFWL_Message* pMessage) {
  m_pOldDelegate->OnProcessMessage(pMessage);
}

void CXFA_FFListBox::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  switch (pEvent->GetType()) {
    case CFWL_Event::Type::SelectChanged: {
      CFX_ArrayTemplate<int32_t> arrSels;
      OnSelectChanged(m_pNormalWidget, arrSels);
      break;
    }
    default:
      break;
  }
  m_pOldDelegate->OnProcessEvent(pEvent);
}
void CXFA_FFListBox::OnDrawWidget(CFX_Graphics* pGraphics,
                                  const CFX_Matrix* pMatrix) {
  m_pOldDelegate->OnDrawWidget(pGraphics, pMatrix);
}

CXFA_FFComboBox::CXFA_FFComboBox(CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFField(pDataAcc), m_pOldDelegate(nullptr) {}

CXFA_FFComboBox::~CXFA_FFComboBox() {}

CFX_RectF CXFA_FFComboBox::GetBBox(uint32_t dwStatus, bool bDrawFocus) {
  if (bDrawFocus)
    return CFX_RectF();
  return CXFA_FFWidget::GetBBox(dwStatus);
}

bool CXFA_FFComboBox::PtInActiveRect(const CFX_PointF& point) {
  auto pComboBox = static_cast<CFWL_ComboBox*>(m_pNormalWidget);
  return pComboBox && pComboBox->GetBBox().Contains(point);
}

bool CXFA_FFComboBox::LoadWidget() {
  CFWL_ComboBox* pComboBox = new CFWL_ComboBox(GetFWLApp());
  m_pNormalWidget = (CFWL_Widget*)pComboBox;
  m_pNormalWidget->SetLayoutItem(this);

  CFWL_NoteDriver* pNoteDriver =
      m_pNormalWidget->GetOwnerApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(m_pNormalWidget, m_pNormalWidget);

  m_pOldDelegate = m_pNormalWidget->GetDelegate();
  m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();

  std::vector<CFX_WideString> wsLabelArray;
  m_pDataAcc->GetChoiceListItems(wsLabelArray, false);
  int32_t iItems = pdfium::CollectionSize<int32_t>(wsLabelArray);
  for (int32_t i = 0; i < iItems; i++) {
    pComboBox->AddString(wsLabelArray[i].AsStringC());
  }
  CFX_ArrayTemplate<int32_t> iSelArray;
  m_pDataAcc->GetSelectedItems(iSelArray);
  int32_t iSelCount = iSelArray.GetSize();
  if (iSelCount > 0) {
    pComboBox->SetCurSel(iSelArray[0]);
  } else {
    CFX_WideString wsText;
    m_pDataAcc->GetValue(wsText, XFA_VALUEPICTURE_Raw);
    pComboBox->SetEditText(wsText);
  }
  UpdateWidgetProperty();
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}
void CXFA_FFComboBox::UpdateWidgetProperty() {
  CFWL_ComboBox* pComboBox = (CFWL_ComboBox*)m_pNormalWidget;
  if (!pComboBox) {
    return;
  }
  uint32_t dwExtendedStyle = 0;
  uint32_t dwEditStyles =
      FWL_STYLEEXT_EDT_ReadOnly | FWL_STYLEEXT_EDT_LastLineHeight;
  dwExtendedStyle |= UpdateUIProperty();
  if (m_pDataAcc->IsChoiceListAllowTextEntry()) {
    dwEditStyles &= ~FWL_STYLEEXT_EDT_ReadOnly;
    dwExtendedStyle |= FWL_STYLEEXT_CMB_DropDown;
  }
  if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open ||
      !m_pDataAcc->GetDoc()->GetXFADoc()->IsInteractive()) {
    dwEditStyles |= FWL_STYLEEXT_EDT_ReadOnly;
    dwExtendedStyle |= FWL_STYLEEXT_CMB_ReadOnly;
  }
  dwExtendedStyle |= GetAlignment();
  m_pNormalWidget->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
  if (m_pDataAcc->GetHorizontalScrollPolicy() != XFA_ATTRIBUTEENUM_Off) {
    dwEditStyles |= FWL_STYLEEXT_EDT_AutoHScroll;
  }
  pComboBox->EditModifyStylesEx(dwEditStyles, 0xFFFFFFFF);
}

bool CXFA_FFComboBox::OnRButtonUp(uint32_t dwFlags, const CFX_PointF& point) {
  if (!CXFA_FFField::OnRButtonUp(dwFlags, point))
    return false;

  GetDoc()->GetDocEnvironment()->PopupMenu(this, point);
  return true;
}

bool CXFA_FFComboBox::OnKillFocus(CXFA_FFWidget* pNewWidget) {
  bool flag = ProcessCommittedData();
  if (!flag) {
    UpdateFWLData();
  }
  CXFA_FFField::OnKillFocus(pNewWidget);
  return true;
}
void CXFA_FFComboBox::OpenDropDownList() {
  ((CFWL_ComboBox*)m_pNormalWidget)->OpenDropDownList(true);
}
bool CXFA_FFComboBox::CommitData() {
  return m_pDataAcc->SetValue(m_wsNewValue, XFA_VALUEPICTURE_Raw);
}
bool CXFA_FFComboBox::IsDataChanged() {
  CFWL_ComboBox* pFWLcombobox = ((CFWL_ComboBox*)m_pNormalWidget);
  CFX_WideString wsText = pFWLcombobox->GetEditText();
  int32_t iCursel = pFWLcombobox->GetCurSel();
  if (iCursel >= 0) {
    CFX_WideString wsSel = pFWLcombobox->GetTextByIndex(iCursel);
    if (wsSel == wsText)
      m_pDataAcc->GetChoiceListItem(wsText, iCursel, true);
  }

  CFX_WideString wsOldValue;
  m_pDataAcc->GetValue(wsOldValue, XFA_VALUEPICTURE_Raw);
  if (wsOldValue != wsText) {
    m_wsNewValue = wsText;
    return true;
  }
  return false;
}
void CXFA_FFComboBox::FWLEventSelChange(CXFA_EventParam* pParam) {
  pParam->m_eType = XFA_EVENT_Change;
  pParam->m_pTarget = m_pDataAcc;
  CFWL_ComboBox* pFWLcombobox = ((CFWL_ComboBox*)m_pNormalWidget);
  pParam->m_wsNewText = pFWLcombobox->GetEditText();
  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_Change, pParam);
}
uint32_t CXFA_FFComboBox::GetAlignment() {
  uint32_t dwExtendedStyle = 0;
  if (CXFA_Para para = m_pDataAcc->GetPara()) {
    int32_t iHorz = para.GetHorizontalAlign();
    switch (iHorz) {
      case XFA_ATTRIBUTEENUM_Center:
        dwExtendedStyle |=
            FWL_STYLEEXT_CMB_EditHCenter | FWL_STYLEEXT_CMB_ListItemCenterAlign;
        break;
      case XFA_ATTRIBUTEENUM_Justify:
        dwExtendedStyle |= FWL_STYLEEXT_CMB_EditJustified;
        break;
      case XFA_ATTRIBUTEENUM_JustifyAll:
        break;
      case XFA_ATTRIBUTEENUM_Radix:
        break;
      case XFA_ATTRIBUTEENUM_Right:
        break;
      default:
        dwExtendedStyle |=
            FWL_STYLEEXT_CMB_EditHNear | FWL_STYLEEXT_CMB_ListItemLeftAlign;
        break;
    }
    int32_t iVert = para.GetVerticalAlign();
    switch (iVert) {
      case XFA_ATTRIBUTEENUM_Middle:
        dwExtendedStyle |= FWL_STYLEEXT_CMB_EditVCenter;
        break;
      case XFA_ATTRIBUTEENUM_Bottom:
        dwExtendedStyle |= FWL_STYLEEXT_CMB_EditVFar;
        break;
      default:
        dwExtendedStyle |= FWL_STYLEEXT_CMB_EditVNear;
        break;
    }
  }
  return dwExtendedStyle;
}
bool CXFA_FFComboBox::UpdateFWLData() {
  if (!m_pNormalWidget) {
    return false;
  }
  CFX_ArrayTemplate<int32_t> iSelArray;
  m_pDataAcc->GetSelectedItems(iSelArray);
  int32_t iSelCount = iSelArray.GetSize();
  if (iSelCount > 0) {
    ((CFWL_ComboBox*)m_pNormalWidget)->SetCurSel(iSelArray[0]);
  } else {
    CFX_WideString wsText;
    ((CFWL_ComboBox*)m_pNormalWidget)->SetCurSel(-1);
    m_pDataAcc->GetValue(wsText, XFA_VALUEPICTURE_Raw);
    ((CFWL_ComboBox*)m_pNormalWidget)->SetEditText(wsText);
  }
  m_pNormalWidget->Update();
  return true;
}
bool CXFA_FFComboBox::CanUndo() {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditCanUndo();
}
bool CXFA_FFComboBox::CanRedo() {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditCanRedo();
}
bool CXFA_FFComboBox::Undo() {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditUndo();
}
bool CXFA_FFComboBox::Redo() {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditRedo();
}
bool CXFA_FFComboBox::CanCopy() {
  return ((CFWL_ComboBox*)m_pNormalWidget)->EditCanCopy();
}
bool CXFA_FFComboBox::CanCut() {
  if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open) {
    return false;
  }
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditCanCut();
}
bool CXFA_FFComboBox::CanPaste() {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         (m_pDataAcc->GetAccess() == XFA_ATTRIBUTEENUM_Open);
}
bool CXFA_FFComboBox::CanSelectAll() {
  return ((CFWL_ComboBox*)m_pNormalWidget)->EditCanSelectAll();
}
bool CXFA_FFComboBox::Copy(CFX_WideString& wsCopy) {
  return ((CFWL_ComboBox*)m_pNormalWidget)->EditCopy(wsCopy);
}
bool CXFA_FFComboBox::Cut(CFX_WideString& wsCut) {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditCut(wsCut);
}
bool CXFA_FFComboBox::Paste(const CFX_WideString& wsPaste) {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditPaste(wsPaste);
}
void CXFA_FFComboBox::SelectAll() {
  ((CFWL_ComboBox*)m_pNormalWidget)->EditSelectAll();
}
void CXFA_FFComboBox::Delete() {
  ((CFWL_ComboBox*)m_pNormalWidget)->EditDelete();
}
void CXFA_FFComboBox::DeSelect() {
  ((CFWL_ComboBox*)m_pNormalWidget)->EditDeSelect();
}
void CXFA_FFComboBox::SetItemState(int32_t nIndex, bool bSelected) {
  if (bSelected) {
    ((CFWL_ComboBox*)m_pNormalWidget)->SetCurSel(nIndex);
  } else {
    ((CFWL_ComboBox*)m_pNormalWidget)->SetCurSel(-1);
  }
  m_pNormalWidget->Update();
  AddInvalidateRect();
}
void CXFA_FFComboBox::InsertItem(const CFX_WideStringC& wsLabel,
                                 int32_t nIndex) {
  ((CFWL_ComboBox*)m_pNormalWidget)->AddString(wsLabel);
  m_pNormalWidget->Update();
  AddInvalidateRect();
}
void CXFA_FFComboBox::DeleteItem(int32_t nIndex) {
  if (nIndex < 0) {
    ((CFWL_ComboBox*)m_pNormalWidget)->RemoveAll();
  } else {
    ((CFWL_ComboBox*)m_pNormalWidget)->RemoveAt(nIndex);
  }
  m_pNormalWidget->Update();
  AddInvalidateRect();
}
void CXFA_FFComboBox::OnTextChanged(CFWL_Widget* pWidget,
                                    const CFX_WideString& wsChanged) {
  CXFA_EventParam eParam;
  m_pDataAcc->GetValue(eParam.m_wsPrevText, XFA_VALUEPICTURE_Raw);
  eParam.m_wsChange = wsChanged;
  FWLEventSelChange(&eParam);
}
void CXFA_FFComboBox::OnSelectChanged(CFWL_Widget* pWidget, bool bLButtonUp) {
  CXFA_EventParam eParam;
  m_pDataAcc->GetValue(eParam.m_wsPrevText, XFA_VALUEPICTURE_Raw);
  FWLEventSelChange(&eParam);
  if (m_pDataAcc->GetChoiceListCommitOn() == XFA_ATTRIBUTEENUM_Select &&
      bLButtonUp) {
    m_pDocView->SetFocusWidgetAcc(nullptr);
  }
}
void CXFA_FFComboBox::OnPreOpen(CFWL_Widget* pWidget) {
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_PreOpen;
  eParam.m_pTarget = m_pDataAcc;
  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_PreOpen, &eParam);
}
void CXFA_FFComboBox::OnPostOpen(CFWL_Widget* pWidget) {
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_PostOpen;
  eParam.m_pTarget = m_pDataAcc;
  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_PostOpen, &eParam);
}

void CXFA_FFComboBox::OnProcessMessage(CFWL_Message* pMessage) {
  m_pOldDelegate->OnProcessMessage(pMessage);
}

void CXFA_FFComboBox::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  switch (pEvent->GetType()) {
    case CFWL_Event::Type::SelectChanged: {
      CFWL_EventSelectChanged* postEvent =
          static_cast<CFWL_EventSelectChanged*>(pEvent);
      OnSelectChanged(m_pNormalWidget, postEvent->bLButtonUp);
      break;
    }
    case CFWL_Event::Type::EditChanged: {
      CFX_WideString wsChanged;
      OnTextChanged(m_pNormalWidget, wsChanged);
      break;
    }
    case CFWL_Event::Type::PreDropDown: {
      OnPreOpen(m_pNormalWidget);
      break;
    }
    case CFWL_Event::Type::PostDropDown: {
      OnPostOpen(m_pNormalWidget);
      break;
    }
    default:
      break;
  }
  m_pOldDelegate->OnProcessEvent(pEvent);
}

void CXFA_FFComboBox::OnDrawWidget(CFX_Graphics* pGraphics,
                                   const CFX_Matrix* pMatrix) {
  m_pOldDelegate->OnDrawWidget(pGraphics, pMatrix);
}
