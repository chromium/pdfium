// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FORMFILLER_FFL_TEXTFIELD_H_
#define FPDFSDK_INCLUDE_FORMFILLER_FFL_TEXTFIELD_H_

#include "FFL_FormFiller.h"

#define BF_ALIGN_LEFT			0
#define BF_ALIGN_MIDDLE			1
#define BF_ALIGN_RIGHT			2

class CBA_FontMap;

struct FFL_TextFieldState
{
	int nStart;
	int nEnd;
	CFX_WideString sValue;
};

class CFFL_TextField : public CFFL_FormFiller, public IPWL_FocusHandler, public IPWL_Edit_Notify
{
public:
	CFFL_TextField(CPDFDoc_Environment* pApp, CPDFSDK_Annot* pAnnot);
	virtual ~CFFL_TextField();
	
	virtual	PWL_CREATEPARAM		GetCreateParam();
	virtual CPWL_Wnd*			NewPDFWindow(const PWL_CREATEPARAM& cp, CPDFSDK_PageView* pPageView);

	
	virtual FX_BOOL				OnChar(CPDFSDK_Annot* pAnnot, FX_UINT nChar, FX_UINT nFlags);
	
	virtual FX_BOOL				IsDataChanged(CPDFSDK_PageView* pPageView);
	virtual void				SaveData(CPDFSDK_PageView* pPageView);
	
 	virtual void				GetActionData(CPDFSDK_PageView* pPageView, CPDF_AAction::AActionType type,
 												PDFSDK_FieldAction& fa);
 	virtual void				SetActionData(CPDFSDK_PageView* pPageView, CPDF_AAction::AActionType type, 
									const PDFSDK_FieldAction& fa);
 	virtual FX_BOOL				IsActionDataChanged(CPDF_AAction::AActionType type, const PDFSDK_FieldAction& faOld, 
 												const PDFSDK_FieldAction& faNew);
	virtual void				SaveState(CPDFSDK_PageView* pPageView);
	virtual void				RestoreState(CPDFSDK_PageView* pPageView);
	
	virtual CPWL_Wnd*			ResetPDFWindow(CPDFSDK_PageView* pPageView, FX_BOOL bRestoreValue);
	
public:
	virtual void				OnSetFocus(CPWL_Wnd* pWnd);
	virtual void				OnKillFocus(CPWL_Wnd* pWnd);
	
public:
	virtual void				OnAddUndo(CPWL_Edit* pEdit);
	
public:
	virtual FX_BOOL				CanCopy(CPDFSDK_Document* pDocument); 
	virtual FX_BOOL				CanCut(CPDFSDK_Document* pDocument); 
	virtual FX_BOOL				CanPaste(CPDFSDK_Document* pDocument); 
	
	virtual void				DoCopy(CPDFSDK_Document* pDocument); 
	virtual void				DoCut(CPDFSDK_Document* pDocument); 
	virtual void				DoPaste(CPDFSDK_Document* pDocument); 
	
private:
	CBA_FontMap*				m_pFontMap;
	FFL_TextFieldState			m_State;

};

#endif  // FPDFSDK_INCLUDE_FORMFILLER_FFL_TEXTFIELD_H_
