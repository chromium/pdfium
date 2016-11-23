// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_listbox.h"

#include <memory>
#include <utility>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fwl/core/ifwl_listbox.h"

namespace {

IFWL_ListBox* ToListBox(IFWL_Widget* widget) {
  return static_cast<IFWL_ListBox*>(widget);
}

}  // namespace

CFWL_ListBox::CFWL_ListBox(const CFWL_App* app) : CFWL_Widget(app) {}

CFWL_ListBox::~CFWL_ListBox() {}

void CFWL_ListBox::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_ListBox>(
      m_pApp, pdfium::MakeUnique<CFWL_WidgetProperties>(), nullptr);

  CFWL_Widget::Initialize();
}

CFWL_ListItem* CFWL_ListBox::GetItem(const IFWL_Widget* pWidget,
                                     int32_t nIndex) const {
  return GetWidget() ? ToListBox(GetWidget())->GetItem(pWidget, nIndex)
                     : nullptr;
}

void CFWL_ListBox::GetItemText(IFWL_Widget* pWidget,
                               CFWL_ListItem* pItem,
                               CFX_WideString& wsText) {
  if (GetWidget())
    ToListBox(GetWidget())->GetItemText(pWidget, pItem, wsText);
}

CFWL_ListItem* CFWL_ListBox::AddString(const CFX_WideStringC& wsAdd,
                                       bool bSelect) {
  return GetWidget() ? ToListBox(GetWidget())->AddString(wsAdd, bSelect)
                     : nullptr;
}

bool CFWL_ListBox::DeleteString(CFWL_ListItem* pItem) {
  return GetWidget() && ToListBox(GetWidget())->DeleteString(pItem);
}

void CFWL_ListBox::DeleteAll() {
  if (GetWidget())
    ToListBox(GetWidget())->DeleteAll();
}

uint32_t CFWL_ListBox::GetItemStates(CFWL_ListItem* pItem) {
  if (!pItem)
    return 0;
  return pItem->m_dwStates | pItem->m_dwCheckState;
}

int32_t CFWL_ListBox::CountSelItems() {
  return GetWidget() ? ToListBox(GetWidget())->CountSelItems() : 0;
}

CFWL_ListItem* CFWL_ListBox::GetSelItem(int32_t nIndexSel) {
  return GetWidget() ? ToListBox(GetWidget())->GetSelItem(nIndexSel) : nullptr;
}

int32_t CFWL_ListBox::GetSelIndex(int32_t nIndex) {
  return GetWidget() ? ToListBox(GetWidget())->GetSelIndex(nIndex) : 0;
}

void CFWL_ListBox::SetSelItem(CFWL_ListItem* pItem, bool bSelect) {
  if (GetWidget())
    ToListBox(GetWidget())->SetSelItem(pItem, bSelect);
}
