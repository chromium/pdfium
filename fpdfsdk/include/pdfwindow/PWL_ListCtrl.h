// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_LISTCTRL_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_LISTCTRL_H_

#include "PWL_Wnd.h"

class CPWL_ListCtrl : public CPWL_Wnd
{
public:
	CPWL_ListCtrl();
	virtual ~CPWL_ListCtrl();

	void								SetScrollPos(const CPDF_Point& point);
	CPDF_Point							GetScrollPos() const;
	CPDF_Rect							GetScrollArea() const;
	void								SetItemSpace(FX_FLOAT fSpace);
	void								SetTopSpace(FX_FLOAT fSpace);
	void								SetBottomSpace(FX_FLOAT fSpace);
	void								ResetFace();
	void								ResetContent(FX_INT32 nStart);
	FX_INT32							GetItemIndex(CPWL_Wnd* pItem);
	FX_FLOAT							GetContentsHeight(FX_FLOAT fLimitWidth);
	CPDF_Point							InToOut(const CPDF_Point& point) const;
	CPDF_Point							OutToIn(const CPDF_Point& point) const;
	CPDF_Rect							InToOut(const CPDF_Rect& rect) const;
	CPDF_Rect							OutToIn(const CPDF_Rect& rect) const;

protected:
	virtual void						RePosChildWnd();
	virtual void						DrawChildAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);

private:
	void								ResetAll(FX_BOOL bMove,FX_INT32 nStart);

	CPDF_Rect							m_rcContent;
	CPDF_Point							m_ptScroll;
	FX_FLOAT							m_fItemSpace;
	FX_FLOAT							m_fTopSpace;
	FX_FLOAT							m_fBottomSpace;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_LISTCTRL_H_
