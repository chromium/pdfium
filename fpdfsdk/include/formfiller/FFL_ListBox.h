// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FORMFILLER_FFL_LISTBOX_H_
#define FPDFSDK_INCLUDE_FORMFILLER_FFL_LISTBOX_H_

#include "FFL_FormFiller.h"

class  CBA_FontMap;

class CFFL_ListBox : public CFFL_FormFiller
{
public:
	CFFL_ListBox(CPDFDoc_Environment* pApp, CPDFSDK_Annot* pWidget);
	virtual ~CFFL_ListBox();

	virtual	PWL_CREATEPARAM		GetCreateParam();
	virtual CPWL_Wnd*			NewPDFWindow(const PWL_CREATEPARAM& cp, CPDFSDK_PageView* pPageView);

	virtual FX_BOOL				OnChar(CPDFSDK_Annot* pAnnot, FX_UINT nChar, FX_UINT nFlags);

	virtual FX_BOOL				IsDataChanged(CPDFSDK_PageView* pPageView);
	virtual void				SaveData(CPDFSDK_PageView* pPageView);

 	virtual void				GetActionData(CPDFSDK_PageView* pPageView, CPDF_AAction::AActionType type,
 									PDFSDK_FieldAction& fa);
 	virtual void				SetActionData(CPDFSDK_PageView* pPageView, CPDF_AAction::AActionType type, 
 									const PDFSDK_FieldAction& fa);

	virtual void				SaveState(CPDFSDK_PageView* pPageView);
	virtual void				RestoreState(CPDFSDK_PageView* pPageView);

	virtual CPWL_Wnd*			ResetPDFWindow(CPDFSDK_PageView* pPageView, FX_BOOL bRestoreValue);
	virtual void				OnKeyStroke(FX_BOOL bKeyDown, FX_DWORD nFlag);

private:
	CBA_FontMap*					m_pFontMap;
	CFX_MapPtrTemplate<int, void*>	m_OriginSelections;
	CFX_ArrayTemplate<int>			m_State;
};


#endif  // FPDFSDK_INCLUDE_FORMFILLER_FFL_LISTBOX_H_
