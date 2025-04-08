// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffcombobox.h"

#include <utility>
#include <vector>

#include "core/fxcrt/check.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fwl/cfwl_combobox.h"
#include "xfa/fwl/cfwl_eventselectchanged.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/parser/cxfa_para.h"

namespace {

CFWL_ComboBox* ToComboBox(CFWL_Widget* widget) {
  return static_cast<CFWL_ComboBox*>(widget);
}

const CFWL_ComboBox* ToComboBox(const CFWL_Widget* widget) {
  return static_cast<const CFWL_ComboBox*>(widget);
}

}  // namespace

CXFA_FFComboBox::CXFA_FFComboBox(CXFA_Node* pNode) : CXFA_FFDropDown(pNode) {}

CXFA_FFComboBox::~CXFA_FFComboBox() = default;

void CXFA_FFComboBox::Trace(cppgc::Visitor* visitor) const {
  CXFA_FFDropDown::Trace(visitor);
  visitor->Trace(old_delegate_);
}

CXFA_FFComboBox* CXFA_FFComboBox::AsComboBox() {
  return this;
}

CFX_RectF CXFA_FFComboBox::GetBBox(FocusOption focus) {
  if (focus == kDrawFocus) {
    return CFX_RectF();
  }
  return CXFA_FFWidget::GetBBox(kDoNotDrawFocus);
}

bool CXFA_FFComboBox::PtInActiveRect(const CFX_PointF& point) {
  auto* pComboBox = ToComboBox(GetNormalWidget());
  return pComboBox && pComboBox->GetBBox().Contains(point);
}

bool CXFA_FFComboBox::LoadWidget() {
  DCHECK(!IsLoaded());

  CFWL_ComboBox* pComboBox = cppgc::MakeGarbageCollected<CFWL_ComboBox>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp());
  SetNormalWidget(pComboBox);
  pComboBox->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pComboBox->GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pComboBox, pComboBox);
  old_delegate_ = pComboBox->GetDelegate();
  pComboBox->SetDelegate(this);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pComboBox);
    for (const auto& label : node_->GetChoiceListItems(false)) {
      pComboBox->AddString(label);
    }

    std::vector<int32_t> iSelArray = node_->GetSelectedItems();
    if (iSelArray.empty()) {
      pComboBox->SetEditText(node_->GetValue(XFA_ValuePicture::kRaw));
    } else {
      pComboBox->SetCurSel(iSelArray.front());
    }

    UpdateWidgetProperty();
  }

  return CXFA_FFField::LoadWidget();
}

void CXFA_FFComboBox::UpdateWidgetProperty() {
  auto* pComboBox = ToComboBox(GetNormalWidget());
  if (!pComboBox) {
    return;
  }

  uint32_t dwExtendedStyle = 0;
  uint32_t dwEditStyles = FWL_STYLEEXT_EDT_ReadOnly;
  dwExtendedStyle |= UpdateUIProperty();
  if (node_->IsChoiceListAllowTextEntry()) {
    dwEditStyles &= ~FWL_STYLEEXT_EDT_ReadOnly;
    dwExtendedStyle |= FWL_STYLEEXT_CMB_DropDown;
  }
  if (!node_->IsOpenAccess() || !GetDoc()->GetXFADoc()->IsInteractive()) {
    dwEditStyles |= FWL_STYLEEXT_EDT_ReadOnly;
    dwExtendedStyle |= FWL_STYLEEXT_CMB_ReadOnly;
  }
  dwExtendedStyle |= GetAlignment();
  GetNormalWidget()->ModifyStyleExts(dwExtendedStyle, 0xFFFFFFFF);

  if (!node_->IsHorizontalScrollPolicyOff()) {
    dwEditStyles |= FWL_STYLEEXT_EDT_AutoHScroll;
  }

  pComboBox->EditModifyStyleExts(dwEditStyles, 0xFFFFFFFF);
}

bool CXFA_FFComboBox::OnRButtonUp(Mask<XFA_FWL_KeyFlag> dwFlags,
                                  const CFX_PointF& point) {
  if (!CXFA_FFField::OnRButtonUp(dwFlags, point)) {
    return false;
  }

  GetDoc()->PopupMenu(this, point);
  return true;
}

bool CXFA_FFComboBox::OnKillFocus(CXFA_FFWidget* pNewWidget) {
  if (!ProcessCommittedData()) {
    UpdateFWLData();
  }

  return pNewWidget && CXFA_FFField::OnKillFocus(pNewWidget);
}

void CXFA_FFComboBox::OpenDropDownList() {
  ToComboBox(GetNormalWidget())->ShowDropDownList();
}

bool CXFA_FFComboBox::CommitData() {
  return node_->SetValue(XFA_ValuePicture::kRaw, new_value_);
}

bool CXFA_FFComboBox::IsDataChanged() {
  WideString wsText = GetCurrentText();
  if (node_->GetValue(XFA_ValuePicture::kRaw) == wsText) {
    return false;
  }

  new_value_ = std::move(wsText);
  return true;
}

void CXFA_FFComboBox::FWLEventSelChange(CXFA_EventParam* pParam) {
  pParam->type_ = XFA_EVENT_Change;
  pParam->prev_text_ = ToComboBox(GetNormalWidget())->GetEditText();
  node_->ProcessEvent(GetDocView(), XFA_AttributeValue::Change, pParam);
}

WideString CXFA_FFComboBox::GetCurrentText() const {
  auto* pFWLcombobox = ToComboBox(GetNormalWidget());
  WideString wsText = pFWLcombobox->GetEditText();
  int32_t iCursel = pFWLcombobox->GetCurSel();
  if (iCursel >= 0) {
    WideString wsSel = pFWLcombobox->GetTextByIndex(iCursel);
    if (wsSel == wsText) {
      wsText = node_->GetChoiceListItem(iCursel, true).value_or(L"");
    }
  }
  return wsText;
}

uint32_t CXFA_FFComboBox::GetAlignment() {
  CXFA_Para* para = node_->GetParaIfExists();
  if (!para) {
    return 0;
  }

  uint32_t dwExtendedStyle = 0;
  switch (para->GetHorizontalAlign()) {
    case XFA_AttributeValue::Center:
      dwExtendedStyle |=
          FWL_STYLEEXT_CMB_EditHCenter | FWL_STYLEEXT_CMB_ListItemCenterAlign;
      break;
    case XFA_AttributeValue::Justify:
      dwExtendedStyle |= FWL_STYLEEXT_CMB_EditJustified;
      break;
    case XFA_AttributeValue::JustifyAll:
      break;
    case XFA_AttributeValue::Radix:
      break;
    case XFA_AttributeValue::Right:
      break;
    default:
      dwExtendedStyle |=
          FWL_STYLEEXT_CMB_EditHNear | FWL_STYLEEXT_CMB_ListItemLeftAlign;
      break;
  }

  switch (para->GetVerticalAlign()) {
    case XFA_AttributeValue::Middle:
      dwExtendedStyle |= FWL_STYLEEXT_CMB_EditVCenter;
      break;
    case XFA_AttributeValue::Bottom:
      dwExtendedStyle |= FWL_STYLEEXT_CMB_EditVFar;
      break;
    default:
      dwExtendedStyle |= FWL_STYLEEXT_CMB_EditVNear;
      break;
  }
  return dwExtendedStyle;
}

void CXFA_FFComboBox::UpdateFWLData() {
  auto* pComboBox = ToComboBox(GetNormalWidget());
  if (!pComboBox) {
    return;
  }

  std::vector<int32_t> iSelArray = node_->GetSelectedItems();
  if (!iSelArray.empty()) {
    pComboBox->SetCurSel(iSelArray.front());
  } else {
    pComboBox->SetCurSel(-1);
    pComboBox->SetEditText(node_->GetValue(XFA_ValuePicture::kRaw));
  }
  pComboBox->Update();
}

bool CXFA_FFComboBox::CanUndo() {
  return node_->IsChoiceListAllowTextEntry() &&
         ToComboBox(GetNormalWidget())->EditCanUndo();
}

bool CXFA_FFComboBox::CanRedo() {
  return node_->IsChoiceListAllowTextEntry() &&
         ToComboBox(GetNormalWidget())->EditCanRedo();
}

bool CXFA_FFComboBox::CanCopy() {
  return ToComboBox(GetNormalWidget())->EditCanCopy();
}

bool CXFA_FFComboBox::CanCut() {
  return node_->IsOpenAccess() && node_->IsChoiceListAllowTextEntry() &&
         ToComboBox(GetNormalWidget())->EditCanCut();
}

bool CXFA_FFComboBox::CanPaste() {
  return node_->IsChoiceListAllowTextEntry() && node_->IsOpenAccess();
}

bool CXFA_FFComboBox::CanSelectAll() {
  return ToComboBox(GetNormalWidget())->EditCanSelectAll();
}

bool CXFA_FFComboBox::Undo() {
  return node_->IsChoiceListAllowTextEntry() &&
         ToComboBox(GetNormalWidget())->EditUndo();
}

bool CXFA_FFComboBox::Redo() {
  return node_->IsChoiceListAllowTextEntry() &&
         ToComboBox(GetNormalWidget())->EditRedo();
}

std::optional<WideString> CXFA_FFComboBox::Copy() {
  return ToComboBox(GetNormalWidget())->EditCopy();
}

std::optional<WideString> CXFA_FFComboBox::Cut() {
  if (!node_->IsChoiceListAllowTextEntry()) {
    return std::nullopt;
  }

  return ToComboBox(GetNormalWidget())->EditCut();
}

bool CXFA_FFComboBox::Paste(const WideString& wsPaste) {
  return node_->IsChoiceListAllowTextEntry() &&
         ToComboBox(GetNormalWidget())->EditPaste(wsPaste);
}

void CXFA_FFComboBox::SelectAll() {
  ToComboBox(GetNormalWidget())->EditSelectAll();
}

void CXFA_FFComboBox::Delete() {
  ToComboBox(GetNormalWidget())->EditDelete();
}

void CXFA_FFComboBox::DeSelect() {
  ToComboBox(GetNormalWidget())->EditDeSelect();
}

WideString CXFA_FFComboBox::GetText() {
  return GetCurrentText();
}

FormFieldType CXFA_FFComboBox::GetFormFieldType() {
  return FormFieldType::kXFA_ComboBox;
}

void CXFA_FFComboBox::SetItemState(int32_t nIndex, bool bSelected) {
  ToComboBox(GetNormalWidget())->SetCurSel(bSelected ? nIndex : -1);
  GetNormalWidget()->Update();
  InvalidateRect();
}

void CXFA_FFComboBox::InsertItem(const WideString& wsLabel, int32_t nIndex) {
  ToComboBox(GetNormalWidget())->AddString(wsLabel);
  GetNormalWidget()->Update();
  InvalidateRect();
}

void CXFA_FFComboBox::DeleteItem(int32_t nIndex) {
  if (nIndex < 0) {
    ToComboBox(GetNormalWidget())->RemoveAll();
  } else {
    ToComboBox(GetNormalWidget())->RemoveAt(nIndex);
  }

  GetNormalWidget()->Update();
  InvalidateRect();
}

void CXFA_FFComboBox::OnTextChanged(CFWL_Widget* pWidget,
                                    const WideString& wsChanged) {
  CXFA_EventParam eParam(XFA_EVENT_Unknown);
  eParam.prev_text_ = node_->GetValue(XFA_ValuePicture::kRaw);
  eParam.change_ = wsChanged;
  FWLEventSelChange(&eParam);
}

void CXFA_FFComboBox::OnSelectChanged(CFWL_Widget* pWidget, bool bLButtonUp) {
  CXFA_EventParam eParam(XFA_EVENT_Unknown);
  eParam.prev_text_ = node_->GetValue(XFA_ValuePicture::kRaw);
  FWLEventSelChange(&eParam);
  if (node_->IsChoiceListCommitOnSelect() && bLButtonUp) {
    doc_view_->SetFocusNode(nullptr);
  }
}

void CXFA_FFComboBox::OnPreOpen(CFWL_Widget* pWidget) {
  CXFA_EventParam eParam(XFA_EVENT_PreOpen);
  node_->ProcessEvent(GetDocView(), XFA_AttributeValue::PreOpen, &eParam);
}

void CXFA_FFComboBox::OnPostOpen(CFWL_Widget* pWidget) {
  CXFA_EventParam eParam(XFA_EVENT_PostOpen);
  node_->ProcessEvent(GetDocView(), XFA_AttributeValue::PostOpen, &eParam);
}

void CXFA_FFComboBox::OnProcessMessage(CFWL_Message* pMessage) {
  old_delegate_->OnProcessMessage(pMessage);
}

void CXFA_FFComboBox::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  switch (pEvent->GetType()) {
    case CFWL_Event::Type::SelectChanged: {
      auto* postEvent = static_cast<CFWL_EventSelectChanged*>(pEvent);
      OnSelectChanged(GetNormalWidget(), postEvent->GetLButtonUp());
      break;
    }
    case CFWL_Event::Type::EditChanged: {
      WideString wsChanged;
      OnTextChanged(GetNormalWidget(), wsChanged);
      break;
    }
    case CFWL_Event::Type::PreDropDown: {
      OnPreOpen(GetNormalWidget());
      break;
    }
    case CFWL_Event::Type::PostDropDown: {
      OnPostOpen(GetNormalWidget());
      break;
    }
    default:
      break;
  }
  old_delegate_->OnProcessEvent(pEvent);
}

void CXFA_FFComboBox::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                   const CFX_Matrix& matrix) {
  old_delegate_->OnDrawWidget(pGraphics, matrix);
}
