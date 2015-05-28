// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_LABEL_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_LABEL_H_

#include "PWL_Wnd.h"

class IFX_Edit;

class PWL_CLASS CPWL_Label : public CPWL_Wnd
{
public:
	CPWL_Label();
	virtual ~CPWL_Label();

	virtual CFX_ByteString			GetClassName() const;
	virtual void					SetFontSize(FX_FLOAT fFontSize);
	virtual FX_FLOAT				GetFontSize() const;

	void							SetText(FX_LPCWSTR csText);
	CFX_WideString					GetText() const;
	void							SetLimitChar(FX_INT32 nLimitChar);
	void							SetHorzScale(FX_INT32 nHorzScale);
	void							SetCharSpace(FX_FLOAT fCharSpace);
	CPDF_Rect						GetContentRect() const;
	FX_INT32						GetTotalWords();
	CFX_ByteString					GetTextAppearanceStream(const CPDF_Point & ptOffset) const;

protected:
	virtual void					OnCreated();
	virtual void					DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);
	virtual void					GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream);
	virtual void					RePosChildWnd();

private:
	void							SetParamByFlag();

	IFX_Edit*						m_pEdit;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_LABEL_H_
