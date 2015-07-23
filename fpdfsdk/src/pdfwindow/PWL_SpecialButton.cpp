// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/pdfwindow/PDFWindow.h"
#include "../../include/pdfwindow/PWL_Wnd.h"
#include "../../include/pdfwindow/PWL_Button.h"
#include "../../include/pdfwindow/PWL_SpecialButton.h"
#include "../../include/pdfwindow/PWL_Utils.h"

/* --------------------------- CPWL_PushButton ---------------------------- */

CPWL_PushButton::CPWL_PushButton()
{
}

CPWL_PushButton::~CPWL_PushButton()
{
}

CFX_ByteString CPWL_PushButton::GetClassName() const
{
	return "CPWL_PushButton";
}

CPDF_Rect CPWL_PushButton::GetFocusRect() const
{
    return CPWL_Utils::DeflateRect(GetWindowRect(), (FX_FLOAT)GetBorderWidth());
}

/* --------------------------- CPWL_CheckBox ---------------------------- */

CPWL_CheckBox::CPWL_CheckBox() : m_bChecked(false)
{
}

CPWL_CheckBox::~CPWL_CheckBox()
{
}

CFX_ByteString CPWL_CheckBox::GetClassName() const
{
	return "CPWL_CheckBox";
}

void CPWL_CheckBox::SetCheck(bool bCheck)
{
	m_bChecked = bCheck;
}

bool CPWL_CheckBox::IsChecked() const
{
	return m_bChecked;
}

bool CPWL_CheckBox::OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag)
{
	if (IsReadOnly()) return false;

	SetCheck(!IsChecked());
	return true;
}

bool CPWL_CheckBox::OnChar(FX_WORD nChar, FX_DWORD nFlag)
{
	SetCheck(!IsChecked());
	return true;
}

/* --------------------------- CPWL_RadioButton ---------------------------- */

CPWL_RadioButton::CPWL_RadioButton() : m_bChecked(false)
{
}

CPWL_RadioButton::~CPWL_RadioButton()
{
}

CFX_ByteString CPWL_RadioButton::GetClassName() const
{
	return "CPWL_RadioButton";
}

bool	CPWL_RadioButton::OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag)
{
	if (IsReadOnly()) return false;

	SetCheck(true);
	return true;
}

void CPWL_RadioButton::SetCheck(bool bCheck)
{
	m_bChecked = bCheck;
}

bool CPWL_RadioButton::IsChecked() const
{
	return m_bChecked;
}

bool CPWL_RadioButton::OnChar(FX_WORD nChar, FX_DWORD nFlag)
{
	SetCheck(true);
	return true;
}

