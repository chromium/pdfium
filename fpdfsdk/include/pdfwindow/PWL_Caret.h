// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_CARET_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_CARET_H_

#include "PWL_Wnd.h"

struct PWL_CARET_INFO
{
public:
	PWL_CARET_INFO() : bVisible(FALSE), ptHead(0,0), ptFoot(0,0)
	{
	}

	FX_BOOL						bVisible;
	CPDF_Point					ptHead;
	CPDF_Point					ptFoot;
};


class CPWL_Caret : public CPWL_Wnd
{
public:
	CPWL_Caret();
	virtual ~CPWL_Caret();

	virtual CFX_ByteString		GetClassName() const;
	virtual void				GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream);
	virtual void				DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);
	virtual void				InvalidateRect(CPDF_Rect * pRect = NULL);
	virtual void				SetVisible(FX_BOOL bVisible) {}
	virtual	void				TimerProc();
	void						SetCaret(FX_BOOL bVisible, const CPDF_Point & ptHead, const CPDF_Point & ptFoot);	
	CFX_ByteString				GetCaretAppearanceStream(const CPDF_Point & ptOffset);
	void						SetInvalidRect(CPDF_Rect rc) {m_rcInvalid = rc;}

private:
	void						GetCaretApp(CFX_ByteTextBuf & sAppStream,const CPDF_Point & ptOffset);
	CPDF_Rect					GetCaretRect() const;

	FX_BOOL						m_bFlash;
	CPDF_Point					m_ptHead;
	CPDF_Point					m_ptFoot;
	FX_FLOAT					m_fWidth;
	FX_INT32					m_nDelay;
	CPDF_Rect					m_rcInvalid;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_CARET_H_
