// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_combobox.h"

#include <utility>

#include "third_party/base/ptr_util.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_combobox.h"
#include "xfa/fwl/core/ifwl_widget.h"

namespace {

IFWL_ComboBox* ToComboBox(IFWL_Widget* widget) {
  return static_cast<IFWL_ComboBox*>(widget);
}

}  // namespace

CFWL_ComboBox::CFWL_ComboBox(const CFWL_App* app) : CFWL_Widget(app) {}

CFWL_ComboBox::~CFWL_ComboBox() {}

void CFWL_ComboBox::Initialize() {
  ASSERT(!m_pIface);

  m_pIface = pdfium::MakeUnique<IFWL_ComboBox>(
      m_pApp, pdfium::MakeUnique<CFWL_WidgetProperties>(this));

  CFWL_Widget::Initialize();
}

void CFWL_ComboBox::AddString(const CFX_WideStringC& wsText) {
  if (GetWidget())
    ToComboBox(GetWidget())->AddString(wsText);
}

bool CFWL_ComboBox::RemoveAt(int32_t iIndex) {
  return GetWidget() && ToComboBox(GetWidget())->RemoveAt(iIndex);
}

void CFWL_ComboBox::RemoveAll() {
  if (GetWidget())
    ToComboBox(GetWidget())->RemoveAll();
}

void CFWL_ComboBox::GetTextByIndex(int32_t iIndex,
                                   CFX_WideString& wsText) const {
  if (!GetWidget())
    return;
  ToComboBox(GetWidget())->GetTextByIndex(iIndex, wsText);
}

int32_t CFWL_ComboBox::GetCurSel() const {
  return GetWidget() ? ToComboBox(GetWidget())->GetCurSel() : -1;
}

void CFWL_ComboBox::SetCurSel(int32_t iSel) {
  if (GetWidget())
    ToComboBox(GetWidget())->SetCurSel(iSel);
}

void CFWL_ComboBox::SetEditText(const CFX_WideString& wsText) {
  if (GetWidget())
    ToComboBox(GetWidget())->SetEditText(wsText);
}

void CFWL_ComboBox::GetEditText(CFX_WideString& wsText,
                                int32_t nStart,
                                int32_t nCount) const {
  if (GetWidget())
    ToComboBox(GetWidget())->GetEditText(wsText, nStart, nCount);
}

void CFWL_ComboBox::OpenDropDownList(bool bActivate) {
  ToComboBox(GetWidget())->OpenDropDownList(bActivate);
}

bool CFWL_ComboBox::EditCanUndo() {
  return GetWidget() ? ToComboBox(GetWidget())->EditCanUndo() : false;
}

bool CFWL_ComboBox::EditCanRedo() {
  return GetWidget() ? ToComboBox(GetWidget())->EditCanRedo() : false;
}

bool CFWL_ComboBox::EditUndo() {
  return GetWidget() ? ToComboBox(GetWidget())->EditUndo() : false;
}

bool CFWL_ComboBox::EditRedo() {
  return GetWidget() ? ToComboBox(GetWidget())->EditRedo() : false;
}

bool CFWL_ComboBox::EditCanCopy() {
  return GetWidget() ? ToComboBox(GetWidget())->EditCanCopy() : false;
}

bool CFWL_ComboBox::EditCanCut() {
  return GetWidget() ? ToComboBox(GetWidget())->EditCanCut() : false;
}

bool CFWL_ComboBox::EditCanSelectAll() {
  return GetWidget() ? ToComboBox(GetWidget())->EditCanSelectAll() : false;
}

bool CFWL_ComboBox::EditCopy(CFX_WideString& wsCopy) {
  return GetWidget() ? ToComboBox(GetWidget())->EditCopy(wsCopy) : false;
}

bool CFWL_ComboBox::EditCut(CFX_WideString& wsCut) {
  return GetWidget() ? ToComboBox(GetWidget())->EditCut(wsCut) : false;
}

bool CFWL_ComboBox::EditPaste(const CFX_WideString& wsPaste) {
  return GetWidget() ? ToComboBox(GetWidget())->EditPaste(wsPaste) : false;
}

void CFWL_ComboBox::EditSelectAll() {
  if (GetWidget())
    ToComboBox(GetWidget())->EditSelectAll();
}

void CFWL_ComboBox::EditDelete() {
  if (GetWidget())
    ToComboBox(GetWidget())->EditDelete();
}

void CFWL_ComboBox::EditDeSelect() {
  if (GetWidget())
    ToComboBox(GetWidget())->EditDeSelect();
}

void CFWL_ComboBox::GetBBox(CFX_RectF& rect) {
  if (GetWidget())
    ToComboBox(GetWidget())->GetBBox(rect);
}

void CFWL_ComboBox::EditModifyStylesEx(uint32_t dwStylesExAdded,
                                       uint32_t dwStylesExRemoved) {
  if (GetWidget()) {
    ToComboBox(GetWidget())
        ->EditModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
  }
}
