// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_fwladapter.h"
#include "xfa_ffwidget.h"
#include "xfa_fffield.h"
#include "xfa_ffpageview.h"
#include "xfa_ffdocview.h"
#include "xfa_ffchoicelist.h"
#include "xfa_ffdoc.h"
CXFA_FFListBox::CXFA_FFListBox(CXFA_FFPageView* pPageView,
                               CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFField(pPageView, pDataAcc), m_pOldDelegate(NULL) {}
CXFA_FFListBox::~CXFA_FFListBox() {
  if (m_pNormalWidget) {
    IFWL_Widget* pWidget = m_pNormalWidget->GetWidget();
    IFWL_NoteDriver* pNoteDriver = FWL_GetApp()->GetNoteDriver();
    pNoteDriver->UnregisterEventTarget(pWidget);
  }
}
FX_BOOL CXFA_FFListBox::LoadWidget() {
  CFWL_ListBox* pListBox = CFWL_ListBox::Create();
  pListBox->Initialize();
  pListBox->ModifyStyles(FWL_WGTSTYLE_VScroll | FWL_WGTSTYLE_NoBackground,
                         0xFFFFFFFF);
  m_pNormalWidget = (CFWL_Widget*)pListBox;
  IFWL_Widget* pWidget = m_pNormalWidget->GetWidget();
  m_pNormalWidget->SetPrivateData(pWidget, this, NULL);
  IFWL_NoteDriver* pNoteDriver = FWL_GetApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pWidget, pWidget);
  m_pOldDelegate = m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();
  CFX_WideStringArray wsLabelArray;
  m_pDataAcc->GetChoiceListItems(wsLabelArray, FALSE);
  int32_t iItems = wsLabelArray.GetSize();
  for (int32_t i = 0; i < iItems; i++) {
    pListBox->AddString(wsLabelArray[i]);
  }
  FX_DWORD dwExtendedStyle = FWL_STYLEEXT_LTB_ShowScrollBarFocus;
  if (m_pDataAcc->GetChoiceListOpen() == XFA_ATTRIBUTEENUM_MultiSelect) {
    dwExtendedStyle |= FWL_STYLEEXT_LTB_MultiSelection;
  }
  dwExtendedStyle |= GetAlignment();
  m_pNormalWidget->ModifyStylesEx(dwExtendedStyle, 0xFFFFFFFF);
  CFX_Int32Array iSelArray;
  m_pDataAcc->GetSelectedItems(iSelArray);
  int32_t iSelCount = iSelArray.GetSize();
  for (int32_t j = 0; j < iSelCount; j++) {
    FWL_HLISTITEM item = pListBox->GetItem(iSelArray[j]);
    pListBox->SetSelItem(item, TRUE);
  }
  m_pNormalWidget->UnlockUpdate();
  return CXFA_FFField::LoadWidget();
}
FX_BOOL CXFA_FFListBox::OnKillFocus(CXFA_FFWidget* pNewFocus) {
  FX_BOOL flag = ProcessCommittedData();
  if (!flag) {
    UpdateFWLData();
  }
  CXFA_FFField::OnKillFocus(pNewFocus);
  return TRUE;
}
FX_BOOL CXFA_FFListBox::CommitData() {
  FXSYS_assert(m_pNormalWidget != NULL);
  CFWL_ListBox* pListBox = (CFWL_ListBox*)m_pNormalWidget;
  int32_t iSels = pListBox->CountSelItems();
  CFX_Int32Array iSelArray;
  for (int32_t i = 0; i < iSels; i++) {
    iSelArray.Add(pListBox->GetSelIndex(i));
  }
  m_pDataAcc->SetSelectdItems(iSelArray, TRUE);
  return TRUE;
}
FX_BOOL CXFA_FFListBox::IsDataChanged() {
  CFX_Int32Array iSelArray;
  m_pDataAcc->GetSelectedItems(iSelArray);
  int32_t iOldSels = iSelArray.GetSize();
  CFWL_ListBox* pListBox = (CFWL_ListBox*)m_pNormalWidget;
  int32_t iSels = pListBox->CountSelItems();
  if (iOldSels == iSels) {
    int32_t iIndex = 0;
    for (; iIndex < iSels; iIndex++) {
      FWL_HLISTITEM hlistItem = pListBox->GetItem(iSelArray[iIndex]);
      if (!(pListBox->GetItemStates(hlistItem) && FWL_ITEMSTATE_LTB_Selected)) {
        break;
      }
    }
    if (iIndex == iSels) {
      return FALSE;
    }
  }
  return TRUE;
}
FX_DWORD CXFA_FFListBox::GetAlignment() {
  FX_DWORD dwExtendedStyle = 0;
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
FX_BOOL CXFA_FFListBox::UpdateFWLData() {
  if (!m_pNormalWidget) {
    return FALSE;
  }
  CFWL_ListBox* pListBox = ((CFWL_ListBox*)m_pNormalWidget);
  CFX_ArrayTemplate<FWL_HLISTITEM> selItemArray;
  CFX_Int32Array iSelArray;
  m_pDataAcc->GetSelectedItems(iSelArray);
  int32_t iSelCount = iSelArray.GetSize();
  for (int32_t j = 0; j < iSelCount; j++) {
    FWL_HLISTITEM lpItemSel = pListBox->GetSelItem(iSelArray[j]);
    selItemArray.Add(lpItemSel);
  }
  pListBox->SetSelItem(pListBox->GetSelItem(-1), FALSE);
  for (int32_t i = 0; i < iSelCount; i++) {
    ((CFWL_ListBox*)m_pNormalWidget)->SetSelItem(selItemArray[i], TRUE);
  }
  m_pNormalWidget->Update();
  return TRUE;
}
void CXFA_FFListBox::OnSelectChanged(IFWL_Widget* pWidget,
                                     const CFX_Int32Array& arrSels) {
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_Change;
  eParam.m_pTarget = m_pDataAcc;
  m_pDataAcc->GetValue(eParam.m_wsPrevText, XFA_VALUEPICTURE_Raw);
  CFWL_ListBox* pListBox = (CFWL_ListBox*)m_pNormalWidget;
  int32_t iSels = pListBox->CountSelItems();
  if (iSels > 0) {
    pListBox->GetItemText(pListBox->GetSelItem(0), eParam.m_wsNewText);
  }
  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_Change, &eParam);
}
void CXFA_FFListBox::SetItemState(int32_t nIndex, FX_BOOL bSelected) {
  FWL_HLISTITEM item = ((CFWL_ListBox*)m_pNormalWidget)->GetSelItem(nIndex);
  ((CFWL_ListBox*)m_pNormalWidget)->SetSelItem(item, bSelected);
  m_pNormalWidget->Update();
  AddInvalidateRect();
}
void CXFA_FFListBox::InsertItem(const CFX_WideStringC& wsLabel,
                                int32_t nIndex) {
  CFX_WideString wsTemp(wsLabel);
  ((CFWL_ListBox*)m_pNormalWidget)->AddString(wsTemp);
  m_pNormalWidget->Update();
  AddInvalidateRect();
}
void CXFA_FFListBox::DeleteItem(int32_t nIndex) {
  if (nIndex < 0) {
    ((CFWL_ListBox*)m_pNormalWidget)->DeleteAll();
  } else {
    ((CFWL_ListBox*)m_pNormalWidget)
        ->DeleteString(((CFWL_ListBox*)m_pNormalWidget)->GetItem(nIndex));
  }
  m_pNormalWidget->Update();
  AddInvalidateRect();
}
int32_t CXFA_FFListBox::OnProcessMessage(CFWL_Message* pMessage) {
  return m_pOldDelegate->OnProcessMessage(pMessage);
}
FWL_ERR CXFA_FFListBox::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  FX_DWORD dwEventID = pEvent->GetClassID();
  switch (dwEventID) {
    case FWL_EVTHASH_LTB_SelChanged: {
      CFX_Int32Array arrSels;
      OnSelectChanged(m_pNormalWidget->GetWidget(), arrSels);
      break;
    }
    default: {}
  }
  return m_pOldDelegate->OnProcessEvent(pEvent);
}
FWL_ERR CXFA_FFListBox::OnDrawWidget(CFX_Graphics* pGraphics,
                                     const CFX_Matrix* pMatrix) {
  return m_pOldDelegate->OnDrawWidget(pGraphics, pMatrix);
}
CXFA_FFComboBox::CXFA_FFComboBox(CXFA_FFPageView* pPageView,
                                 CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFField(pPageView, pDataAcc), m_pOldDelegate(NULL) {}
CXFA_FFComboBox::~CXFA_FFComboBox() {}
FX_BOOL CXFA_FFComboBox::GetBBox(CFX_RectF& rtBox,
                                 FX_DWORD dwStatus,
                                 FX_BOOL bDrawFocus) {
  if (bDrawFocus) {
    return FALSE;
  }
#ifndef _XFA_EMB
  return CXFA_FFWidget::GetBBox(rtBox, dwStatus);
#endif
  GetRectWithoutRotate(rtBox);
  if (m_pNormalWidget) {
    CFX_RectF rtWidget;
    ((CFWL_ComboBox*)m_pNormalWidget)->GetBBox(rtWidget);
    rtBox.Union(rtWidget);
  }
  CFX_Matrix mt;
  GetRotateMatrix(mt);
  mt.TransformRect(rtBox);
  return TRUE;
}
FX_BOOL CXFA_FFComboBox::PtInActiveRect(FX_FLOAT fx, FX_FLOAT fy) {
  if (!m_pNormalWidget) {
    return FALSE;
  }
  CFX_RectF rtWidget;
  ((CFWL_ComboBox*)m_pNormalWidget)->GetBBox(rtWidget);
  if (rtWidget.Contains(fx, fy)) {
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CXFA_FFComboBox::LoadWidget() {
  CFWL_ComboBox* pComboBox = CFWL_ComboBox::Create();
  pComboBox->Initialize();
  m_pNormalWidget = (CFWL_Widget*)pComboBox;
  IFWL_Widget* pWidget = m_pNormalWidget->GetWidget();
  m_pNormalWidget->SetPrivateData(pWidget, this, NULL);
  IFWL_NoteDriver* pNoteDriver = FWL_GetApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pWidget, pWidget);
  m_pOldDelegate = m_pNormalWidget->SetDelegate(this);
  m_pNormalWidget->LockUpdate();
  CFX_WideStringArray wsLabelArray;
  m_pDataAcc->GetChoiceListItems(wsLabelArray, FALSE);
  int32_t iItems = wsLabelArray.GetSize();
  for (int32_t i = 0; i < iItems; i++) {
    pComboBox->AddString(wsLabelArray[i]);
  }
  CFX_Int32Array iSelArray;
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
  FX_DWORD dwExtendedStyle = 0;
  FX_DWORD dwEditStyles =
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
FX_BOOL CXFA_FFComboBox::OnRButtonUp(FX_DWORD dwFlags,
                                     FX_FLOAT fx,
                                     FX_FLOAT fy) {
  if (!CXFA_FFField::OnRButtonUp(dwFlags, fx, fy)) {
    return FALSE;
  }
  CFX_PointF pt;
  pt.Set(fx, fy);
  GetDoc()->GetDocProvider()->PopupMenu(this, pt, NULL);
  return TRUE;
}
FX_BOOL CXFA_FFComboBox::OnKillFocus(CXFA_FFWidget* pNewWidget) {
  FX_BOOL flag = ProcessCommittedData();
  if (!flag) {
    UpdateFWLData();
  }
  CXFA_FFField::OnKillFocus(pNewWidget);
  return TRUE;
}
void CXFA_FFComboBox::OpenDropDownList() {
  ((CFWL_ComboBox*)m_pNormalWidget)->OpenDropDownList(TRUE);
}
FX_BOOL CXFA_FFComboBox::CommitData() {
  return m_pDataAcc->SetValue(m_wsNewValue, XFA_VALUEPICTURE_Raw);
}
FX_BOOL CXFA_FFComboBox::IsDataChanged() {
  CFWL_ComboBox* pFWLcombobox = ((CFWL_ComboBox*)m_pNormalWidget);
  CFX_WideString wsText;
  pFWLcombobox->GetEditText(wsText);
  int32_t iCursel = pFWLcombobox->GetCurSel();
  if (iCursel >= 0) {
    CFX_WideString wsSel;
    pFWLcombobox->GetTextByIndex(iCursel, wsSel);
    if (wsSel == wsText) {
      m_pDataAcc->GetChoiceListItem(wsText, iCursel, TRUE);
    }
  }
  CFX_WideString wsOldValue;
  m_pDataAcc->GetValue(wsOldValue, XFA_VALUEPICTURE_Raw);
  if (wsOldValue != wsText) {
    m_wsNewValue = wsText;
    return TRUE;
  }
  return FALSE;
}
void CXFA_FFComboBox::FWLEventSelChange(CXFA_EventParam* pParam) {
  pParam->m_eType = XFA_EVENT_Change;
  pParam->m_pTarget = m_pDataAcc;
  CFWL_ComboBox* pFWLcombobox = ((CFWL_ComboBox*)m_pNormalWidget);
  pFWLcombobox->GetEditText(pParam->m_wsNewText);
  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_Change, pParam);
}
FX_DWORD CXFA_FFComboBox::GetAlignment() {
  FX_DWORD dwExtendedStyle = 0;
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
FX_BOOL CXFA_FFComboBox::UpdateFWLData() {
  if (!m_pNormalWidget) {
    return FALSE;
  }
  CFX_Int32Array iSelArray;
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
  return TRUE;
}
FX_BOOL CXFA_FFComboBox::CanUndo() {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditCanUndo();
}
FX_BOOL CXFA_FFComboBox::CanRedo() {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditCanRedo();
}
FX_BOOL CXFA_FFComboBox::Undo() {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditUndo();
}
FX_BOOL CXFA_FFComboBox::Redo() {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditRedo();
}
FX_BOOL CXFA_FFComboBox::CanCopy() {
  return ((CFWL_ComboBox*)m_pNormalWidget)->EditCanCopy();
}
FX_BOOL CXFA_FFComboBox::CanCut() {
  if (m_pDataAcc->GetAccess() != XFA_ATTRIBUTEENUM_Open) {
    return FALSE;
  }
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditCanCut();
}
FX_BOOL CXFA_FFComboBox::CanPaste() {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         (m_pDataAcc->GetAccess() == XFA_ATTRIBUTEENUM_Open);
}
FX_BOOL CXFA_FFComboBox::CanSelectAll() {
  return ((CFWL_ComboBox*)m_pNormalWidget)->EditCanSelectAll();
}
FX_BOOL CXFA_FFComboBox::Copy(CFX_WideString& wsCopy) {
  return ((CFWL_ComboBox*)m_pNormalWidget)->EditCopy(wsCopy);
}
FX_BOOL CXFA_FFComboBox::Cut(CFX_WideString& wsCut) {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditCut(wsCut);
}
FX_BOOL CXFA_FFComboBox::Paste(const CFX_WideString& wsPaste) {
  return m_pDataAcc->IsChoiceListAllowTextEntry() &&
         ((CFWL_ComboBox*)m_pNormalWidget)->EditPaste(wsPaste);
}
FX_BOOL CXFA_FFComboBox::SelectAll() {
  return ((CFWL_ComboBox*)m_pNormalWidget)->EditSelectAll();
}
FX_BOOL CXFA_FFComboBox::Delete() {
  return ((CFWL_ComboBox*)m_pNormalWidget)->EditDelete();
}
FX_BOOL CXFA_FFComboBox::DeSelect() {
  return ((CFWL_ComboBox*)m_pNormalWidget)->EditDeSelect();
}
void CXFA_FFComboBox::SetItemState(int32_t nIndex, FX_BOOL bSelected) {
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
void CXFA_FFComboBox::OnTextChanged(IFWL_Widget* pWidget,
                                    const CFX_WideString& wsChanged) {
  CXFA_EventParam eParam;
  m_pDataAcc->GetValue(eParam.m_wsPrevText, XFA_VALUEPICTURE_Raw);
  eParam.m_wsChange = wsChanged;
  FWLEventSelChange(&eParam);
}
void CXFA_FFComboBox::OnSelectChanged(IFWL_Widget* pWidget,
                                      const CFX_Int32Array& arrSels,
                                      FX_BOOL bLButtonUp) {
  CXFA_EventParam eParam;
  m_pDataAcc->GetValue(eParam.m_wsPrevText, XFA_VALUEPICTURE_Raw);
  FWLEventSelChange(&eParam);
  if (m_pDataAcc->GetChoiceListCommitOn() == XFA_ATTRIBUTEENUM_Select &&
      bLButtonUp) {
    m_pDocView->SetFocusWidgetAcc(NULL);
  }
}
void CXFA_FFComboBox::OnPreOpen(IFWL_Widget* pWidget) {
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_PreOpen;
  eParam.m_pTarget = m_pDataAcc;
  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_PreOpen, &eParam);
}
void CXFA_FFComboBox::OnPostOpen(IFWL_Widget* pWidget) {
  CXFA_EventParam eParam;
  eParam.m_eType = XFA_EVENT_PostOpen;
  eParam.m_pTarget = m_pDataAcc;
  m_pDataAcc->ProcessEvent(XFA_ATTRIBUTEENUM_PostOpen, &eParam);
}
void CXFA_FFComboBox::OnAddDoRecord(IFWL_Widget* pWidget) {
  GetDoc()->GetDocProvider()->AddDoRecord(this);
}
int32_t CXFA_FFComboBox::OnProcessMessage(CFWL_Message* pMessage) {
  return m_pOldDelegate->OnProcessMessage(pMessage);
}
FWL_ERR CXFA_FFComboBox::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  FX_DWORD dwEventID = pEvent->GetClassID();
  switch (dwEventID) {
    case FWL_EVTHASH_CMB_SelChanged: {
      CFWL_EvtCmbSelChanged* postEvent = (CFWL_EvtCmbSelChanged*)pEvent;
      OnSelectChanged(m_pNormalWidget->GetWidget(), postEvent->iArraySels,
                      postEvent->bLButtonUp);
      break;
    }
    case FWL_EVTHASH_CMB_EditChanged: {
      CFX_WideString wsChanged;
      OnTextChanged(m_pNormalWidget->GetWidget(), wsChanged);
      break;
    }
    case FWL_EVTHASH_CMB_PreDropDown: {
      OnPreOpen(m_pNormalWidget->GetWidget());
      break;
    }
    case FWL_EVTHASH_CMB_PostDropDown: {
      OnPostOpen(m_pNormalWidget->GetWidget());
      break;
    }
    default: {}
  }
  return m_pOldDelegate->OnProcessEvent(pEvent);
}
FWL_ERR CXFA_FFComboBox::OnDrawWidget(CFX_Graphics* pGraphics,
                                      const CFX_Matrix* pMatrix) {
  return m_pOldDelegate->OnDrawWidget(pGraphics, pMatrix);
}
