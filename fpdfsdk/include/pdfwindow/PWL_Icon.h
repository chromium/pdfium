// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_ICON_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_ICON_H_

#include "../../../core/include/fxcrt/fx_string.h"
#include "PWL_Wnd.h"

class PWL_CLASS CPWL_Image : public CPWL_Wnd
{
public:
	CPWL_Image();
	virtual ~CPWL_Image();

	virtual CFX_ByteString			GetImageAppStream();

	virtual void					GetScale(FX_FLOAT & fHScale,FX_FLOAT & fVScale);
	virtual void					GetImageOffset(FX_FLOAT & x,FX_FLOAT & y);
	virtual CPDF_Stream *			GetPDFStream();

public:
	void							SetPDFStream(CPDF_Stream* pStream);	
	void							GetImageSize(FX_FLOAT & fWidth,FX_FLOAT & fHeight);
	CPDF_Matrix						GetImageMatrix();
	CFX_ByteString					GetImageAlias();
	void							SetImageAlias(FX_LPCSTR sImageAlias);

protected:
	CPDF_Stream*					m_pPDFStream;
	CFX_ByteString					m_sImageAlias;
};

class PWL_CLASS CPWL_Icon : public CPWL_Image 
{
public:
	CPWL_Icon();
	virtual ~CPWL_Icon();

	virtual CPDF_IconFit *			GetIconFit(){return m_pIconFit;};

	virtual void					GetScale(FX_FLOAT & fHScale,FX_FLOAT & fVScale);
	virtual void					GetImageOffset(FX_FLOAT & x,FX_FLOAT & y);

	FX_INT32						GetScaleMethod();
	FX_BOOL							IsProportionalScale();
	void							GetIconPosition(FX_FLOAT & fLeft, FX_FLOAT & fBottom);
	FX_BOOL							GetFittingBounds();

	void							SetIconFit(CPDF_IconFit * pIconFit){m_pIconFit = pIconFit;};

private:
	CPDF_IconFit *					m_pIconFit;
};


#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_ICON_H_
