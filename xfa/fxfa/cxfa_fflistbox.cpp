// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_fflistbox.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "core/fxcrt/check.h"
#include "core/fxcrt/stl_util.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fwl/cfwl_listbox.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/parser/cxfa_para.h"

namespace {

CFWL_ListBox* ToListBox(CFWL_Widget* widget) {
  return static_cast<CFWL_ListBox*>(widget);
}

}  // namespace

CXFA_FFListBox::CXFA_FFListBox(CXFA_Node* pNode) : CXFA_FFDropDown(pNode) {}

CXFA_FFListBox::~CXFA_FFListBox() = default;

void CXFA_FFListBox::PreFinalize() {
  if (GetNormalWidget()) {
    CFWL_NoteDriver* pNoteDriver =
        GetNormalWidget()->GetFWLApp()->GetNoteDriver();
    pNoteDriver->UnregisterEventTarget(GetNormalWidget());
  }
}

void CXFA_FFListBox::Trace(cppgc::Visitor* visitor) const {
  CXFA_FFDropDown::Trace(visitor);
  visitor->Trace(old_delegate_);
}

bool CXFA_FFListBox::LoadWidget() {
  DCHECK(!IsLoaded());

  CFWL_ListBox* pListBox = cppgc::MakeGarbageCollected<CFWL_ListBox>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp(),
      CFWL_Widget::Properties(), nullptr);
  pListBox->ModifyStyles(FWL_STYLE_WGT_VScroll | FWL_STYLE_WGT_NoBackground,
                         0xFFFFFFFF);
  SetNormalWidget(pListBox);
  pListBox->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pListBox->GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pListBox, pListBox);
  old_delegate_ = pListBox->GetDelegate();
  pListBox->SetDelegate(this);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pListBox);
    std::vector<WideString> displayables = node_->GetChoiceListItems(false);
    std::vector<WideString> settables = node_->GetChoiceListItems(true);
    if (displayables.size() > settables.size()) {
      displayables.resize(settables.size());
    }

    for (const auto& label : displayables) {
      pListBox->AddString(label);
    }

    uint32_t dwExtendedStyle = FWL_STYLEEXT_LTB_ShowScrollBarFocus;
    if (node_->IsChoiceListMultiSelect()) {
      dwExtendedStyle |= FWL_STYLEEXT_LTB_MultiSelection;
    }

    dwExtendedStyle |= GetAlignment();
    pListBox->ModifyStyleExts(dwExtendedStyle, 0xFFFFFFFF);
    for (int32_t selected : node_->GetSelectedItems()) {
      pListBox->SetSelItem(pListBox->GetItem(nullptr, selected), true);
    }
  }

  return CXFA_FFField::LoadWidget();
}

bool CXFA_FFListBox::OnKillFocus(CXFA_FFWidget* pNewFocus) {
  if (!ProcessCommittedData()) {
    UpdateFWLData();
  }

  return pNewFocus && CXFA_FFField::OnKillFocus(pNewFocus);
}

bool CXFA_FFListBox::CommitData() {
  auto* pListBox = ToListBox(GetNormalWidget());
  std::vector<int32_t> iSelArray;
  int32_t iSels = pListBox->CountSelItems();
  for (int32_t i = 0; i < iSels; ++i) {
    iSelArray.push_back(pListBox->GetSelIndex(i));
  }

  node_->SetSelectedItems(iSelArray, true, false, true);
  return true;
}

bool CXFA_FFListBox::IsDataChanged() {
  std::vector<int32_t> iSelArray = node_->GetSelectedItems();
  int32_t iOldSels = fxcrt::CollectionSize<int32_t>(iSelArray);
  auto* pListBox = ToListBox(GetNormalWidget());
  int32_t iSels = pListBox->CountSelItems();
  if (iOldSels != iSels) {
    return true;
  }

  for (int32_t i = 0; i < iSels; ++i) {
    CFWL_ListBox::Item* hlistItem = pListBox->GetItem(nullptr, iSelArray[i]);
    if (!hlistItem->IsSelected()) {
      return true;
    }
  }
  return false;
}

uint32_t CXFA_FFListBox::GetAlignment() {
  CXFA_Para* para = node_->GetParaIfExists();
  if (!para) {
    return 0;
  }

  uint32_t dwExtendedStyle = 0;
  switch (para->GetHorizontalAlign()) {
    case XFA_AttributeValue::Center:
      dwExtendedStyle |= FWL_STYLEEXT_LTB_CenterAlign;
      break;
    case XFA_AttributeValue::Justify:
      break;
    case XFA_AttributeValue::JustifyAll:
      break;
    case XFA_AttributeValue::Radix:
      break;
    case XFA_AttributeValue::Right:
      dwExtendedStyle |= FWL_STYLEEXT_LTB_RightAlign;
      break;
    default:
      dwExtendedStyle |= FWL_STYLEEXT_LTB_LeftAlign;
      break;
  }
  return dwExtendedStyle;
}

void CXFA_FFListBox::UpdateFWLData() {
  auto* pListBox = ToListBox(GetNormalWidget());
  if (!pListBox) {
    return;
  }

  std::vector<int32_t> iSelArray = node_->GetSelectedItems();
  std::vector<CFWL_ListBox::Item*> selItemArray(iSelArray.size());
  std::transform(iSelArray.begin(), iSelArray.end(), selItemArray.begin(),
                 [pListBox](int32_t val) { return pListBox->GetSelItem(val); });

  pListBox->SetSelItem(pListBox->GetSelItem(-1), false);
  for (CFWL_ListBox::Item* pItem : selItemArray) {
    pListBox->SetSelItem(pItem, true);
  }

  GetNormalWidget()->Update();
}

void CXFA_FFListBox::OnSelectChanged(CFWL_Widget* pWidget) {
  CXFA_EventParam eParam(XFA_EVENT_Change);
  eParam.prev_text_ = node_->GetValue(XFA_ValuePicture::kRaw);
  node_->ProcessEvent(GetDocView(), XFA_AttributeValue::Change, &eParam);
}

void CXFA_FFListBox::SetItemState(int32_t nIndex, bool bSelected) {
  auto* pListBox = ToListBox(GetNormalWidget());
  pListBox->SetSelItem(pListBox->GetSelItem(nIndex), bSelected);
  GetNormalWidget()->Update();
  InvalidateRect();
}

void CXFA_FFListBox::InsertItem(const WideString& wsLabel, int32_t nIndex) {
  ToListBox(GetNormalWidget())->AddString(wsLabel);
  GetNormalWidget()->Update();
  InvalidateRect();
}

void CXFA_FFListBox::DeleteItem(int32_t nIndex) {
  auto* pListBox = ToListBox(GetNormalWidget());
  if (nIndex < 0) {
    pListBox->DeleteAll();
  } else {
    pListBox->DeleteString(pListBox->GetItem(nullptr, nIndex));
  }

  pListBox->Update();
  InvalidateRect();
}

void CXFA_FFListBox::OnProcessMessage(CFWL_Message* pMessage) {
  old_delegate_->OnProcessMessage(pMessage);
}

void CXFA_FFListBox::OnProcessEvent(CFWL_Event* pEvent) {
  CXFA_FFField::OnProcessEvent(pEvent);
  switch (pEvent->GetType()) {
    case CFWL_Event::Type::SelectChanged:
      OnSelectChanged(GetNormalWidget());
      break;
    default:
      break;
  }
  old_delegate_->OnProcessEvent(pEvent);
}

void CXFA_FFListBox::OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                                  const CFX_Matrix& matrix) {
  old_delegate_->OnDrawWidget(pGraphics, matrix);
}

FormFieldType CXFA_FFListBox::GetFormFieldType() {
  return FormFieldType::kXFA_ListBox;
}
