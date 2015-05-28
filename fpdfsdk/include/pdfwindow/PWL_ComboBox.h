// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_COMBOBOX_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_COMBOBOX_H_

#include "PWL_Edit.h"
#include "PWL_ListBox.h"
#include "PWL_Wnd.h"

class CPWL_CBEdit : public CPWL_Edit
{
public:
	CPWL_CBEdit(){};
	virtual ~CPWL_CBEdit(){};
};

class PWL_CLASS CPWL_CBListBox : public CPWL_ListBox
{
public:
	CPWL_CBListBox(){};
	virtual ~CPWL_CBListBox(){};

public:
	virtual FX_BOOL				OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag);

	FX_BOOL				OnKeyDownWithExit(FX_WORD nChar, FX_BOOL & bExit, FX_DWORD nFlag);
	FX_BOOL				OnCharWithExit(FX_WORD nChar, FX_BOOL & bExit, FX_DWORD nFlag);
};

#define PWL_COMBOBOX_BUTTON_WIDTH		13

class CPWL_CBButton : public CPWL_Wnd
{
public:
	CPWL_CBButton(){};
	virtual ~CPWL_CBButton(){};

public:
	virtual void				GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream);
	virtual void				DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);

	virtual FX_BOOL				OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL				OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag);

};

class PWL_CLASS CPWL_ComboBox : public CPWL_Wnd  
{
public:
	CPWL_ComboBox();
	operator CPWL_Edit* ()		{return m_pEdit;}

public:
	virtual CFX_ByteString		GetClassName() const;
	virtual void				OnCreate(PWL_CREATEPARAM & cp);

	virtual FX_BOOL				OnKeyDown(FX_WORD nChar, FX_DWORD nFlag);	
	virtual FX_BOOL				OnChar(FX_WORD nChar, FX_DWORD nFlag);

	virtual void				OnNotify(CPWL_Wnd* pWnd, FX_DWORD msg, FX_INTPTR wParam = 0, FX_INTPTR lParam = 0);

	virtual void				CreateChildWnd(const PWL_CREATEPARAM & cp);
	virtual void				RePosChildWnd();

	virtual CPDF_Rect			GetFocusRect() const;
	
	virtual void				SetFocus();
	virtual void				KillFocus();

	FX_BOOL						IsModified() const;

public:	
	void						SetFillerNotify(IPWL_Filler_Notify* pNotify);

	CFX_WideString				GetText() const;
	void						SetText(FX_LPCWSTR text);

	void						AddString(FX_LPCWSTR string);
	FX_INT32					GetSelect() const;
	void						SetSelect(FX_INT32 nItemIndex);

	void						SetEditSel(FX_INT32 nStartChar,FX_INT32 nEndChar);
	void						GetEditSel(FX_INT32 & nStartChar, FX_INT32 & nEndChar ) const;
	void						Clear();
	void						SelectAll();
	FX_BOOL						IsPopup() const;

	void						SetSelectText();

private:	
	void						CreateEdit(const PWL_CREATEPARAM & cp);
	void						CreateButton(const PWL_CREATEPARAM & cp);
	void						CreateListBox(const PWL_CREATEPARAM & cp);

	void						SetPopup(FX_BOOL bPopup);
	
private:
	CPWL_CBEdit*				m_pEdit;
	CPWL_CBButton*				m_pButton;
	CPWL_CBListBox*				m_pList;

	FX_BOOL						m_bPopup;
	CPDF_Rect					m_rcOldWindow;
	FX_INT32					m_nPopupWhere;
	FX_INT32					m_nSelectItem;
	IPWL_Filler_Notify*			m_pFillerNotify;

public:
	void							AttachFFLData(void* pData) {m_pFormFiller = pData;}
private:
	void*							m_pFormFiller;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_COMBOBOX_H_
