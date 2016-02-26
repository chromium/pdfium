// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/pdfwindow/PWL_Button.h"
#include "fpdfsdk/include/pdfwindow/PWL_SpecialButton.h"
#include "fpdfsdk/include/pdfwindow/PWL_Utils.h"
#include "fpdfsdk/include/pdfwindow/PWL_Wnd.h"

CPWL_PushButton::CPWL_PushButton() {}

CPWL_PushButton::~CPWL_PushButton() {}

CFX_ByteString CPWL_PushButton::GetClassName() const {
  return "CPWL_PushButton";
}

CPDF_Rect CPWL_PushButton::GetFocusRect() const {
  return CPWL_Utils::DeflateRect(GetWindowRect(), (FX_FLOAT)GetBorderWidth());
}

CPWL_CheckBox::CPWL_CheckBox() : m_bChecked(FALSE) {}

CPWL_CheckBox::~CPWL_CheckBox() {}

CFX_ByteString CPWL_CheckBox::GetClassName() const {
  return "CPWL_CheckBox";
}

void CPWL_CheckBox::SetCheck(FX_BOOL bCheck) {
  m_bChecked = bCheck;
}

FX_BOOL CPWL_CheckBox::IsChecked() const {
  return m_bChecked;
}

FX_BOOL CPWL_CheckBox::OnLButtonUp(const CPDF_Point& point, FX_DWORD nFlag) {
  if (IsReadOnly())
    return FALSE;

  SetCheck(!IsChecked());
  return TRUE;
}

FX_BOOL CPWL_CheckBox::OnChar(FX_WORD nChar, FX_DWORD nFlag) {
  SetCheck(!IsChecked());
  return TRUE;
}

CPWL_RadioButton::CPWL_RadioButton() : m_bChecked(FALSE) {}

CPWL_RadioButton::~CPWL_RadioButton() {}

CFX_ByteString CPWL_RadioButton::GetClassName() const {
  return "CPWL_RadioButton";
}

FX_BOOL CPWL_RadioButton::OnLButtonUp(const CPDF_Point& point, FX_DWORD nFlag) {
  if (IsReadOnly())
    return FALSE;

  SetCheck(TRUE);
  return TRUE;
}

void CPWL_RadioButton::SetCheck(FX_BOOL bCheck) {
  m_bChecked = bCheck;
}

FX_BOOL CPWL_RadioButton::IsChecked() const {
  return m_bChecked;
}

FX_BOOL CPWL_RadioButton::OnChar(FX_WORD nChar, FX_DWORD nFlag) {
  SetCheck(TRUE);
  return TRUE;
}
