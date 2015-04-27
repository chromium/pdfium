// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/formfiller/FormFiller.h"
#include "../../include/formfiller/FFL_FormFiller.h"
#include "../../include/formfiller/FFL_IFormFiller.h"
#include "../../include/formfiller/FFL_CBA_Fontmap.h"
#include "../../include/formfiller/FFL_ComboBox.h"


/* ------------------------------- CFFL_ComboBox ------------------------------- */

CFFL_ComboBox::CFFL_ComboBox(CPDFDoc_Environment* pApp, CPDFSDK_Annot* pAnnot) :
	CFFL_FormFiller(pApp, pAnnot), m_pFontMap( NULL )
{
	//m_pFontMap = new CBA_FontMap( pAnnot, GetSystemHandler() );
        m_State.nIndex = 0;
        m_State.nStart = 0;
        m_State.nEnd   = 0;
}

CFFL_ComboBox::~CFFL_ComboBox()
{
	if (m_pFontMap)
	{
		delete m_pFontMap;
		m_pFontMap = NULL;
	}

// 	for (int i=0,sz=m_IMBox.GetSize(); i<sz; i++)
// 	{
// 		delete m_IMBox.GetAt(i);
// 	}
// 
// 	m_IMBox.RemoveAll();
}

PWL_CREATEPARAM CFFL_ComboBox::GetCreateParam()
{
	PWL_CREATEPARAM cp = CFFL_FormFiller::GetCreateParam();

	ASSERT(m_pWidget != NULL);

	int nFlags = m_pWidget->GetFieldFlags();
	
	if (nFlags & FIELDFLAG_EDIT)
	{		
		cp.dwFlags |= PCBS_ALLOWCUSTOMTEXT;
	}

	/*
	if (nFlags & FIELDFLAG_COMMITONSELCHANGE)
	{		
		m_bCommitOnSelectChange = TRUE;
	}
	*/

	if (!m_pFontMap)
	{
		ASSERT(this->m_pApp != NULL);
		m_pFontMap = new CBA_FontMap(m_pWidget, GetSystemHandler());
		m_pFontMap->Initial();
	}

	cp.pFontMap = m_pFontMap;
	cp.pFocusHandler = this;

	return cp;
}

CPWL_Wnd* CFFL_ComboBox::NewPDFWindow(const PWL_CREATEPARAM& cp, CPDFSDK_PageView* pPageView)
{
	CPWL_ComboBox * pWnd = new CPWL_ComboBox();
	pWnd->AttachFFLData(this);
	pWnd->Create(cp);

	CFFL_IFormFiller* pFormFiller = m_pApp->GetIFormFiller();
	pWnd->SetFillerNotify(pFormFiller);

	FX_INT32 nCurSel = m_pWidget->GetSelectedIndex(0);
	CFX_WideString swText;
	if (nCurSel < 0)
		swText = m_pWidget->GetValue();
	else
		swText = m_pWidget->GetOptionLabel(nCurSel);

	for (FX_INT32 i=0,sz=m_pWidget->CountOptions(); i<sz; i++)
	{
		pWnd->AddString(m_pWidget->GetOptionLabel(i).c_str());
	}

	pWnd->SetSelect(nCurSel);
	pWnd->SetText(swText.c_str());
	return pWnd;
}


FX_BOOL	CFFL_ComboBox::OnChar(CPDFSDK_Annot* pAnnot, FX_UINT nChar, FX_UINT nFlags)
{
	return CFFL_FormFiller::OnChar(pAnnot, nChar, nFlags);
}

FX_BOOL	CFFL_ComboBox::IsDataChanged(CPDFSDK_PageView* pPageView)
{
	if (CPWL_ComboBox * pWnd = (CPWL_ComboBox*)GetPDFWindow(pPageView, FALSE))
	{
		FX_INT32 nCurSel = pWnd->GetSelect();

		ASSERT(m_pWidget != NULL);

		if (m_pWidget->GetFieldFlags() & FIELDFLAG_EDIT)
		{
			if (nCurSel >= 0)
			{
				return nCurSel != m_pWidget->GetSelectedIndex(0);
			}
			else
			{
				return pWnd->GetText() != m_pWidget->GetValue();
			}
		}
		else
		{
			return nCurSel != m_pWidget->GetSelectedIndex(0);
		}
	}
	
	return FALSE;
}

void CFFL_ComboBox::SaveData(CPDFSDK_PageView* pPageView)
{
	ASSERT(m_pWidget != NULL);

	if (CPWL_ComboBox* pWnd = (CPWL_ComboBox*)GetPDFWindow(pPageView, FALSE))
	{
		CFX_WideString swText = pWnd->GetText();
		FX_INT32 nCurSel = pWnd->GetSelect();

		//mantis:0004157
		FX_BOOL bSetValue = TRUE;

		if (m_pWidget->GetFieldFlags() & FIELDFLAG_EDIT)
		{
			if (nCurSel >= 0)
			{
				if (swText != m_pWidget->GetOptionLabel(nCurSel))
					bSetValue = TRUE;
				else
					bSetValue = FALSE;
			}
			else
				bSetValue = TRUE;
		}
		else
			bSetValue = FALSE;

		CFX_WideString sOldValue;
		

		if (bSetValue)
		{
			sOldValue = m_pWidget->GetValue();
			m_pWidget->SetValue(swText, FALSE);
		}
		else
		{
			m_pWidget->GetSelectedIndex(0);
			m_pWidget->SetOptionSelection(nCurSel, TRUE, FALSE);
		}

		m_pWidget->ResetFieldAppearance(TRUE);
		m_pWidget->UpdateField();
		SetChangeMark();

		m_pWidget->GetPDFPage();
		

	}
}

 void CFFL_ComboBox::GetActionData( CPDFSDK_PageView* pPageView, CPDF_AAction::AActionType type, PDFSDK_FieldAction& fa)
{
	switch (type)
	{
	case CPDF_AAction::KeyStroke:
		if (CPWL_ComboBox* pComboBox = (CPWL_ComboBox*)GetPDFWindow(pPageView, FALSE))
		{
			if (CPWL_Edit* pEdit = (CPWL_Edit*)*pComboBox)
			{
				fa.bFieldFull = pEdit->IsTextFull();	
				int nSelStart = 0;
				int nSelEnd = 0;
				pEdit->GetSel(nSelStart, nSelEnd);
				fa.nSelEnd = nSelEnd;
				fa.nSelStart = nSelStart;
				fa.sValue = pEdit->GetText();
				fa.sChangeEx = GetSelectExportText();

				if (fa.bFieldFull)
				{
					fa.sChange = L"";
					fa.sChangeEx = L"";
				}
			}
		}
		break;
	case CPDF_AAction::Validate:
		if (CPWL_ComboBox* pComboBox = (CPWL_ComboBox*)GetPDFWindow(pPageView, FALSE))
		{
			if (CPWL_Edit* pEdit = (CPWL_Edit*)*pComboBox)
			{
				fa.sValue = pEdit->GetText();
			}
		}
		break;
	case CPDF_AAction::LoseFocus:
	case CPDF_AAction::GetFocus:
		ASSERT(m_pWidget != NULL);
		fa.sValue = m_pWidget->GetValue();
		break;
	default:
		break;
	}
}



void CFFL_ComboBox::SetActionData(CPDFSDK_PageView* pPageView, CPDF_AAction::AActionType type, 
									const PDFSDK_FieldAction& fa)
{
	switch (type)
	{
	case CPDF_AAction::KeyStroke:
		if (CPWL_ComboBox* pComboBox = (CPWL_ComboBox*)GetPDFWindow(pPageView, FALSE))
		{
			if (CPWL_Edit* pEdit = (CPWL_Edit*)*pComboBox)
			{
				pEdit->SetSel(fa.nSelStart, fa.nSelEnd);
				pEdit->ReplaceSel(fa.sChange.c_str());
			}
		}
		break;
	default:
		break;
	}
}

FX_BOOL	CFFL_ComboBox::IsActionDataChanged(CPDF_AAction::AActionType type, const PDFSDK_FieldAction& faOld, 
									const PDFSDK_FieldAction& faNew)
{
	switch (type)
	{
	case CPDF_AAction::KeyStroke:
		return (!faOld.bFieldFull && faOld.nSelEnd != faNew.nSelEnd) || faOld.nSelStart != faNew.nSelStart ||
			faOld.sChange != faNew.sChange;
	default:
		break;
	}

	return FALSE;
}

void CFFL_ComboBox::SaveState(CPDFSDK_PageView* pPageView)
{
	ASSERT(pPageView != NULL);

	if (CPWL_ComboBox* pComboBox = (CPWL_ComboBox*)GetPDFWindow(pPageView, FALSE))
	{
		m_State.nIndex = pComboBox->GetSelect();

		if (CPWL_Edit* pEdit = (CPWL_Edit*)*pComboBox)
		{
			pEdit->GetSel(m_State.nStart, m_State.nEnd);
			m_State.sValue = pEdit->GetText();
		}
	}
}

void CFFL_ComboBox::RestoreState(CPDFSDK_PageView* pPageView)
{
	ASSERT(pPageView != NULL);

	if (CPWL_ComboBox* pComboBox = (CPWL_ComboBox*)GetPDFWindow(pPageView, TRUE))
	{
		if (m_State.nIndex >= 0)
			pComboBox->SetSelect(m_State.nIndex);
		else
		{
			if (CPWL_Edit* pEdit = (CPWL_Edit*)*pComboBox)
			{
				pEdit->SetText(m_State.sValue.c_str());
				pEdit->SetSel(m_State.nStart, m_State.nEnd);
			}
		}
	}
}

CPWL_Wnd* CFFL_ComboBox::ResetPDFWindow(CPDFSDK_PageView* pPageView, FX_BOOL bRestoreValue)
{
	if (bRestoreValue)
		SaveState(pPageView);
	
	DestroyPDFWindow(pPageView);
	
	CPWL_Wnd* pRet = NULL;
	
	if (bRestoreValue)
	{
		RestoreState(pPageView);
		pRet = this->GetPDFWindow(pPageView, FALSE);
	}
	else
		pRet = this->GetPDFWindow(pPageView, TRUE);
	
	m_pWidget->UpdateField();
	
	return pRet;
}

void CFFL_ComboBox::OnKeyStroke(FX_BOOL bKeyDown, FX_UINT nFlag)
{
	ASSERT(m_pWidget != NULL);
	
	int nFlags = m_pWidget->GetFieldFlags();
	
	if (nFlags & FIELDFLAG_COMMITONSELCHANGE)
	{
		if (m_bValid)
		{
			CPDFSDK_PageView* pPageView = this->GetCurPageView();
			ASSERT(pPageView != NULL);

			if (CommitData(pPageView, nFlag))
			{
				DestroyPDFWindow(pPageView);
				m_bValid = FALSE;
			}
		}
	}
}

void CFFL_ComboBox::OnSetFocus(CPWL_Wnd* pWnd)
{
	ASSERT(m_pApp != NULL);

	ASSERT(pWnd != NULL);

	if (pWnd->GetClassName() == PWL_CLASSNAME_EDIT)
	{
		CPWL_Edit* pEdit = (CPWL_Edit*)pWnd;
		pEdit->SetCharSet(134);
		pEdit->SetCodePage(936);

		pEdit->SetReadyToInput();
		CFX_WideString wsText = pEdit->GetText();
		int nCharacters = wsText.GetLength();
		CFX_ByteString bsUTFText = wsText.UTF16LE_Encode();
		unsigned short* pBuffer = (unsigned short*)bsUTFText.c_str();
		m_pApp->FFI_OnSetFieldInputFocus(m_pWidget->GetFormField(), pBuffer, nCharacters, TRUE);

 		pEdit->SetEditNotify(this);
	}
}

void CFFL_ComboBox::OnKillFocus(CPWL_Wnd* pWnd)
{
	ASSERT(m_pApp != NULL);
}

FX_BOOL	CFFL_ComboBox::CanCopy(CPDFSDK_Document* pDocument)
{
	ASSERT(pDocument != NULL);

	return FALSE;
}

FX_BOOL CFFL_ComboBox::CanCut(CPDFSDK_Document* pDocument)
{
	ASSERT(pDocument != NULL);

	return FALSE;
}

FX_BOOL	CFFL_ComboBox::CanPaste(CPDFSDK_Document* pDocument)
{
	ASSERT(pDocument != NULL);

	return FALSE;
}

void CFFL_ComboBox::DoCopy(CPDFSDK_Document* pDocument)
{
	ASSERT(pDocument != NULL);
}

void CFFL_ComboBox::DoCut(CPDFSDK_Document* pDocument)
{
	ASSERT(pDocument != NULL);
}

void CFFL_ComboBox::DoPaste(CPDFSDK_Document* pDocument)
{
	ASSERT(pDocument != NULL);
}

void CFFL_ComboBox::OnAddUndo(CPWL_Edit* pEdit)
{
	ASSERT(pEdit != NULL);
}

CFX_WideString CFFL_ComboBox::GetSelectExportText()
{
	CFX_WideString swRet;
	
	int nExport = -1;
	CPDFSDK_PageView *pPageView = GetCurPageView();
	if (CPWL_ComboBox * pComboBox = (CPWL_ComboBox*)GetPDFWindow(pPageView, FALSE))
	{
		nExport = pComboBox->GetSelect();
	}
	
	if (nExport >= 0)
	{
		if (CPDF_FormField * pFormField = m_pWidget->GetFormField())
		{
			swRet = pFormField->GetOptionValue(nExport);
			if (swRet.IsEmpty())
				swRet = pFormField->GetOptionLabel(nExport);
		}
	}
	
	return swRet;
}
