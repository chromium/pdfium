// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_SIGNATURE_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_SIGNATURE_H_

#include "PWL_Icon.h"
#include "PWL_Wnd.h"

class CFX_DIBSource;
class CPWL_Label;

class CPWL_Signature_Image : public CPWL_Image
{
public:
	CPWL_Signature_Image();
	virtual ~CPWL_Signature_Image();

	void								SetImage(CFX_DIBSource* pImage);
	CFX_DIBSource*						GetImage();

protected:
	virtual void						DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);
	virtual void						GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream);

	virtual void						GetScale(FX_FLOAT & fHScale,FX_FLOAT & fVScale);

private:
	CFX_DIBSource*						m_pImage;
};

class PWL_CLASS CPWL_Signature : public CPWL_Wnd
{
public:
	CPWL_Signature();
	virtual ~CPWL_Signature();

	void								SetText(FX_LPCWSTR sText);
	void								SetDescription(FX_LPCWSTR string);
	void								SetImage(CFX_DIBSource* pImage);
	void								SetImageStream(CPDF_Stream * pStream, FX_LPCSTR sImageAlias);

	void								SetTextFlag(FX_BOOL bTextExist);
	void								SetImageFlag(FX_BOOL bImageExist);
	void								SetFoxitFlag(FX_BOOL bFlagExist);

protected:
	virtual void						RePosChildWnd();
	virtual void						CreateChildWnd(const PWL_CREATEPARAM & cp);

	virtual void						DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);
	virtual void						GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream);

private:
	CPWL_Label*							m_pText;
	CPWL_Label*							m_pDescription;
	CPWL_Signature_Image*				m_pImage;

	FX_BOOL								m_bTextExist;
	FX_BOOL								m_bImageExist;
	FX_BOOL								m_bFlagExist;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_SIGNATURE_H_
