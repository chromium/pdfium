// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_SPECIALBUTTON_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_SPECIALBUTTON_H_

#include "PWL_Button.h"

class PWL_CLASS CPWL_PushButton : public CPWL_Button
{
public:
	CPWL_PushButton();
	virtual ~CPWL_PushButton();

	virtual CFX_ByteString		GetClassName() const;
	virtual CPDF_Rect			GetFocusRect() const;
};

class PWL_CLASS CPWL_CheckBox : public CPWL_Button
{
public:
	CPWL_CheckBox();
	virtual ~CPWL_CheckBox();

	virtual CFX_ByteString		GetClassName() const;
	virtual bool				OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag);
	virtual bool				OnChar(FX_WORD nChar, FX_DWORD nFlag);

	void						SetCheck(bool bCheck);
	bool						IsChecked() const;

private:
	bool						m_bChecked;
};

class PWL_CLASS CPWL_RadioButton : public CPWL_Button
{
public:
	CPWL_RadioButton();
	virtual ~CPWL_RadioButton();

	virtual CFX_ByteString		GetClassName() const;
	virtual bool				OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag);
	virtual bool				OnChar(FX_WORD nChar, FX_DWORD nFlag);

	void						SetCheck(bool bCheck);
	bool						IsChecked() const;

private:
	bool						m_bChecked;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_SPECIALBUTTON_H_
