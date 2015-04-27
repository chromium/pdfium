// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/formfiller/FFL_FormFiller.h"
#include "../../include/formfiller/FFL_IFormFiller.h"
#include "../../include/formfiller/FFL_CheckBox.h"
#include "../../include/formfiller/FFL_ComboBox.h"
#include "../../include/formfiller/FFL_ListBox.h"
#include "../../include/formfiller/FFL_PushButton.h"
#include "../../include/formfiller/FFL_RadioButton.h"
#include "../../include/formfiller/FFL_TextField.h"

#define FFL_MAXLISTBOXHEIGHT		140.0f

// HHOOK CFFL_IFormFiller::m_hookSheet = NULL;
// MSG CFFL_IFormFiller::g_Msg;

/* ----------------------------- CFFL_IFormFiller ----------------------------- */

CFFL_IFormFiller::CFFL_IFormFiller(CPDFDoc_Environment* pApp) : 
	m_pApp(pApp),
	m_bNotifying(FALSE)
{
}

CFFL_IFormFiller::~CFFL_IFormFiller()
{
	FX_POSITION pos = m_Maps.GetStartPosition();
	while (pos)
	{
		CPDFSDK_Annot * pAnnot = NULL;
		CFFL_FormFiller * pFormFiller = NULL;
		m_Maps.GetNextAssoc(pos,pAnnot,pFormFiller);
		delete pFormFiller;
	}
	m_Maps.RemoveAll();
}

FX_BOOL	CFFL_IFormFiller::Annot_HitTest(CPDFSDK_PageView* pPageView,CPDFSDK_Annot* pAnnot, CPDF_Point point)
{
	CPDF_Rect rc = pAnnot->GetRect();
	if(rc.Contains(point.x, point.y))
		return TRUE;
	return FALSE;
}

FX_RECT CFFL_IFormFiller::GetViewBBox(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot)
{
	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		return pFormFiller->GetViewBBox(pPageView, pAnnot);
	}
	else
	{
		ASSERT(pPageView != NULL);
		ASSERT(pAnnot != NULL);

		CPDF_Annot* pPDFAnnot = pAnnot->GetPDFAnnot();
		ASSERT(pPDFAnnot != NULL);

		CPDF_Rect rcAnnot;
		pPDFAnnot->GetRect(rcAnnot);

// 		CRect rcWin;
// 		pPageView->DocToWindow(rcAnnot, rcWin);
		CPDF_Rect rcWin = CPWL_Utils::InflateRect(rcAnnot,1);
//		rcWin.InflateRect(1, 1);

		return rcWin.GetOutterRect();
	}
}

void CFFL_IFormFiller::OnDraw(CPDFSDK_PageView* pPageView, /*HDC hDC,*/ CPDFSDK_Annot* pAnnot, 
						CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
						/*const CRect& rcWindow,*/ FX_DWORD dwFlags)
{
	ASSERT(pPageView != NULL);
	CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;

	if (IsVisible(pWidget))
	{
		if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
		{
 			if (pFormFiller->IsValid())
 			{
				pFormFiller->OnDraw(pPageView, pAnnot, pDevice, pUser2Device, dwFlags);
				
				pAnnot->GetPDFPage();
				

				CPDFSDK_Document* pDocument = m_pApp->GetCurrentDoc();
				ASSERT(pDocument != NULL);

				if (pDocument->GetFocusAnnot() == pAnnot)
				{
					CPDF_Rect rcFocus = pFormFiller->GetFocusBox(pPageView);
					if (!rcFocus.IsEmpty())
					{
						CFX_PathData path;
						
						path.SetPointCount(5);
						path.SetPoint(0, rcFocus.left,  rcFocus.top, FXPT_MOVETO);
						path.SetPoint(1, rcFocus.left,  rcFocus.bottom, FXPT_LINETO);
						path.SetPoint(2, rcFocus.right,  rcFocus.bottom, FXPT_LINETO);
						path.SetPoint(3, rcFocus.right,  rcFocus.top, FXPT_LINETO);
						path.SetPoint(4, rcFocus.left,  rcFocus.top, FXPT_LINETO);
						
						CFX_GraphStateData gsd;
						gsd.SetDashCount(1);				
						gsd.m_DashArray[0] = 1.0f;
						gsd.m_DashPhase = 0;	
						
						gsd.m_LineWidth = 1.0f;
						pDevice->DrawPath(&path, pUser2Device, &gsd, 0, ArgbEncode(255,0,0,0), FXFILL_ALTERNATE);

					//	::DrawFocusRect(hDC, &rcFocus);	
					}
				}

				return;
			}
		}

		if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
			pFormFiller->OnDrawDeactive(pPageView, pAnnot, pDevice, pUser2Device, dwFlags);
		else
			pWidget->DrawAppearance(pDevice, pUser2Device, CPDF_Annot::Normal, NULL);

		if (!IsReadOnly(pWidget) && IsFillingAllowed(pWidget))
		{
			pWidget->DrawShadow(pDevice, pPageView);
		}
	
	}
}

void CFFL_IFormFiller::OnCreate(CPDFSDK_Annot* pAnnot)
{
	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		pFormFiller->OnCreate(pAnnot);
	}
}

void CFFL_IFormFiller::OnLoad(CPDFSDK_Annot* pAnnot)
{
	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		pFormFiller->OnLoad(pAnnot);
	}
}

void CFFL_IFormFiller::OnDelete(CPDFSDK_Annot* pAnnot)
{
	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		pFormFiller->OnDelete(pAnnot);
	}

	UnRegisterFormFiller(pAnnot);
}

void CFFL_IFormFiller::OnMouseEnter(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlag)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");
	
	if (!m_bNotifying)
	{
		CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;
		if (pWidget->GetAAction(CPDF_AAction::CursorEnter))
		{
			m_bNotifying = TRUE;
			
			int nValueAge = pWidget->GetValueAge();

			pWidget->ClearAppModified();
			
			ASSERT(pPageView != NULL);
			
			
			
			PDFSDK_FieldAction fa;
			fa.bModifier = m_pApp->FFI_IsCTRLKeyDown(nFlag);
 			fa.bShift = m_pApp->FFI_IsSHIFTKeyDown(nFlag);
			pWidget->OnAAction(CPDF_AAction::CursorEnter, fa, pPageView );
			m_bNotifying = FALSE;
			
			//if ( !IsValidAnnot(pPageView, pAnnot) ) return;
			
			if (pWidget->IsAppModified())
			{
				if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget, FALSE))
				{
					pFormFiller->ResetPDFWindow(pPageView, pWidget->GetValueAge() == nValueAge);
				}
			}
		}
	}
	
	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, TRUE))
	{
		pFormFiller->OnMouseEnter(pPageView, pAnnot);
	}
}

void CFFL_IFormFiller::OnMouseExit(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlag)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");
	
	if (!m_bNotifying)
	{
		CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;
		if (pWidget->GetAAction(CPDF_AAction::CursorExit))
		{
			m_bNotifying = TRUE;
			pWidget->GetAppearanceAge();
			int nValueAge = pWidget->GetValueAge();
			pWidget->ClearAppModified();
			
			ASSERT(pPageView != NULL);
			
			
			
			PDFSDK_FieldAction fa;
			fa.bModifier = m_pApp->FFI_IsCTRLKeyDown(nFlag);
 			fa.bShift = m_pApp->FFI_IsSHIFTKeyDown(nFlag);
			
			pWidget->OnAAction(CPDF_AAction::CursorExit, fa, pPageView);
			m_bNotifying = FALSE;
			
			//if (!IsValidAnnot(pPageView, pAnnot)) return;
			
			if (pWidget->IsAppModified())
			{
				if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget, FALSE))
				{
					pFormFiller->ResetPDFWindow(pPageView, nValueAge == pWidget->GetValueAge());
				}
			}
		}
	}
	
	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		pFormFiller->OnMouseExit(pPageView, pAnnot);
	}
}

FX_BOOL	CFFL_IFormFiller::OnLButtonDown(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");
	
	if (!m_bNotifying)
	{
		CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;
		if (Annot_HitTest(pPageView, pAnnot, point) && pWidget->GetAAction(CPDF_AAction::ButtonDown))
		{
			m_bNotifying = TRUE;
			pWidget->GetAppearanceAge();
			int nValueAge = pWidget->GetValueAge();
			pWidget->ClearAppModified();
			
			ASSERT(pPageView != NULL);
			
			
			
			PDFSDK_FieldAction fa;
			fa.bModifier = m_pApp->FFI_IsCTRLKeyDown(nFlags);
 			fa.bShift = m_pApp->FFI_IsSHIFTKeyDown(nFlags);
			pWidget->OnAAction(CPDF_AAction::ButtonDown, fa, pPageView);
			m_bNotifying = FALSE;
			
			if (!IsValidAnnot(pPageView, pAnnot)) return TRUE;
			
			if (pWidget->IsAppModified())
			{
				if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget, FALSE))
				{
					pFormFiller->ResetPDFWindow(pPageView, nValueAge == pWidget->GetValueAge());
				}
			}
		}
	}
	
	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		return pFormFiller->OnLButtonDown(pPageView, pAnnot, nFlags, point);
	}
	
	return FALSE;
}

FX_BOOL	CFFL_IFormFiller::OnLButtonUp(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");
	
	CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;
	// 	CReader_Page* pPage = pAnnot->GetPage();
	// 	ASSERT(pPage != NULL);
	CPDFSDK_Document* pDocument = m_pApp->GetCurrentDoc();
	ASSERT(pDocument != NULL);		
	
	switch (pWidget->GetFieldType())
	{
	case FIELDTYPE_PUSHBUTTON:
	case FIELDTYPE_CHECKBOX:
	case FIELDTYPE_RADIOBUTTON:
		if (GetViewBBox(pPageView, pAnnot).Contains((int)point.x, (int)point.y))
		{
			pDocument->SetFocusAnnot(pAnnot);
		}
		break;
	default:
		pDocument->SetFocusAnnot(pAnnot);
		break;
	}
	
	FX_BOOL bRet = FALSE;
	
	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		bRet = pFormFiller->OnLButtonUp(pPageView, pAnnot, nFlags, point);
	}

	if (pDocument->GetFocusAnnot() == pAnnot)
	{
		FX_BOOL bExit = FALSE;
		FX_BOOL bReset = FALSE;
		OnButtonUp(pWidget, pPageView, bReset, bExit,nFlags);
		if (bExit) return TRUE;
	}
	return bRet;
}

void CFFL_IFormFiller::OnButtonUp(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, FX_BOOL& bReset, FX_BOOL& bExit,FX_UINT nFlag)
{
	ASSERT(pWidget != NULL);
	
	if (!m_bNotifying)
	{
		if (pWidget->GetAAction(CPDF_AAction::ButtonUp))
		{
			m_bNotifying = TRUE;
			int nAge = pWidget->GetAppearanceAge();
			int nValueAge = pWidget->GetValueAge();
			
			ASSERT(pPageView != NULL);
// 			CReader_DocView* pDocView = pPageView->GetDocView();
// 			ASSERT(pDocView != NULL);
			
			
			
			PDFSDK_FieldAction fa;
			fa.bModifier = m_pApp->FFI_IsCTRLKeyDown(nFlag);
 			fa.bShift = m_pApp->FFI_IsSHIFTKeyDown(nFlag);

			pWidget->OnAAction(CPDF_AAction::ButtonUp, fa, pPageView);
			m_bNotifying = FALSE;
			
			if (!IsValidAnnot(pPageView, pWidget))
			{
				bExit = TRUE;
				return;
			}
			
			if (nAge != pWidget->GetAppearanceAge())
			{
				if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget, FALSE))
				{
					pFormFiller->ResetPDFWindow(pPageView, nValueAge == pWidget->GetValueAge());
				}
				
				bReset = TRUE;
			}
		}
	}
}

FX_BOOL	CFFL_IFormFiller::OnLButtonDblClk(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		return pFormFiller->OnLButtonDblClk(pPageView, pAnnot, nFlags, point);
	}

	return FALSE;
}

FX_BOOL	CFFL_IFormFiller::OnMouseMove(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	//change cursor
	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, TRUE))
	{
		return pFormFiller->OnMouseMove(pPageView, pAnnot, nFlags, point);
	}

	return FALSE;
}

FX_BOOL	CFFL_IFormFiller::OnMouseWheel(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, short zDelta, const CPDF_Point& point)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		return pFormFiller->OnMouseWheel(pPageView, pAnnot, nFlags, zDelta, point);
	}

	return FALSE;
}

FX_BOOL	CFFL_IFormFiller::OnRButtonDown(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		return pFormFiller->OnRButtonDown(pPageView, pAnnot, nFlags, point);
	}

	return FALSE;
}

FX_BOOL	CFFL_IFormFiller::OnRButtonUp(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		return pFormFiller->OnRButtonUp(pPageView, pAnnot, nFlags, point);
	}

	return FALSE;
}

FX_BOOL	CFFL_IFormFiller::OnRButtonDblClk(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		return pFormFiller->OnRButtonDblClk(pPageView, pAnnot, nFlags, point);
	}

	return FALSE;
}

FX_BOOL	CFFL_IFormFiller::OnKeyDown(CPDFSDK_Annot* pAnnot, FX_UINT nKeyCode, FX_UINT nFlags)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		return pFormFiller->OnKeyDown(pAnnot, nKeyCode, nFlags);	
	}

	return FALSE;
}

FX_BOOL	CFFL_IFormFiller::OnChar(CPDFSDK_Annot* pAnnot, FX_UINT nChar, FX_UINT nFlags)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (nChar == FWL_VKEY_Tab) return TRUE;

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		return pFormFiller->OnChar(pAnnot, nChar, nFlags);
	}

	return FALSE;
}

void CFFL_IFormFiller::OnDeSelected(CPDFSDK_Annot* pAnnot)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		pFormFiller->OnDeSelected(pAnnot);
	}
}

void CFFL_IFormFiller::OnSelected(CPDFSDK_Annot* pAnnot)
{
	ASSERT(pAnnot != NULL);
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		pFormFiller->OnSelected(pAnnot);
	}
}

FX_BOOL CFFL_IFormFiller::OnSetFocus(CPDFSDK_Annot* pAnnot,FX_UINT nFlag)
{
	if(!pAnnot) return FALSE;
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (!m_bNotifying)
	{
		CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;
 		if (pWidget->GetAAction(CPDF_AAction::GetFocus))
 		{
  			m_bNotifying = TRUE;
			pWidget->GetAppearanceAge();
			int nValueAge = pWidget->GetValueAge();
 			pWidget->ClearAppModified();
 
 
 			CPDFSDK_PageView* pPageView = pAnnot->GetPageView();
 			ASSERT(pPageView != NULL);
 			
 			PDFSDK_FieldAction fa;
			fa.bModifier = m_pApp->FFI_IsCTRLKeyDown(nFlag);
 			fa.bShift = m_pApp->FFI_IsSHIFTKeyDown(nFlag);

 
 			CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget, TRUE);
 			if(!pFormFiller) return FALSE;
 			pFormFiller->GetActionData(pPageView, CPDF_AAction::GetFocus, fa);
 
 			pWidget->OnAAction(CPDF_AAction::GetFocus, fa, pPageView);
 			m_bNotifying = FALSE;
 			
 //			if (!IsValidAnnot(m_pApp, pDocument, pDocView, pPageView, pAnnot)) return FALSE;
 
 			if (pWidget->IsAppModified())
 			{
 				if (CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget, FALSE))
 				{
 					pFormFiller->ResetPDFWindow(pPageView, nValueAge == pWidget->GetValueAge());
 				}
 			}
		}
	}
	
	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, TRUE))
	{
		if (pFormFiller->OnSetFocus(pAnnot, nFlag))
		{
			return TRUE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

FX_BOOL	CFFL_IFormFiller::OnKillFocus(CPDFSDK_Annot* pAnnot,FX_UINT nFlag)
{
	if(!pAnnot) return FALSE;
	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pAnnot, FALSE))
	{
		if (pFormFiller->OnKillFocus(pAnnot, nFlag))
		{
 			if (!m_bNotifying)
 			{
 				CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;
 				if (pWidget->GetAAction(CPDF_AAction::LoseFocus))
 				{
 					m_bNotifying = TRUE;
 					pWidget->ClearAppModified();
 
 					CPDFSDK_PageView* pPageView = pWidget->GetPageView();
 					ASSERT(pPageView != NULL);
 
 					PDFSDK_FieldAction fa;
					fa.bModifier = m_pApp->FFI_IsCTRLKeyDown(nFlag);
 					fa.bShift = m_pApp->FFI_IsSHIFTKeyDown(nFlag);
 
 					pFormFiller->GetActionData(pPageView, CPDF_AAction::LoseFocus, fa);
 
 					pWidget->OnAAction(CPDF_AAction::LoseFocus, fa, pPageView);
 					m_bNotifying = FALSE;
 
 				}
 			}
		}
		else
			return FALSE;
	}

	return TRUE;
}

FX_BOOL	CFFL_IFormFiller::IsVisible(CPDFSDK_Widget* pWidget)
{
	return pWidget->IsVisible();
}

FX_BOOL	CFFL_IFormFiller::IsReadOnly(CPDFSDK_Widget* pWidget)
{
	ASSERT(pWidget != NULL);

	int nFieldFlags = pWidget->GetFieldFlags();

	return (nFieldFlags & FIELDFLAG_READONLY) == FIELDFLAG_READONLY;
}

FX_BOOL	CFFL_IFormFiller::IsFillingAllowed(CPDFSDK_Widget* pWidget)
{
	ASSERT(pWidget != NULL);

	if (pWidget->GetFieldType() == FIELDTYPE_PUSHBUTTON)
		return TRUE;
 	else
 	{
 		CPDF_Page* pPage = pWidget->GetPDFPage();
 		ASSERT(pPage != NULL);
 
 		CPDF_Document* pDocument = pPage->m_pDocument;
 		ASSERT(pDocument != NULL);
 
		FX_DWORD dwPermissions = pDocument->GetUserPermissions();
 		return (dwPermissions&FPDFPERM_FILL_FORM) || 
 				(dwPermissions&FPDFPERM_ANNOT_FORM) || 
 			(dwPermissions&FPDFPERM_MODIFY);
 	}
	return TRUE;	
}

CFFL_FormFiller* CFFL_IFormFiller::GetFormFiller(CPDFSDK_Annot* pAnnot, FX_BOOL bRegister)
{
// 	ASSERT(pAnnot != NULL);
// 	ASSERT(pAnnot->GetPDFAnnot()->GetSubType() == "Widget");

	CFFL_FormFiller * pFormFiller = NULL;
	m_Maps.Lookup(pAnnot, pFormFiller);

	if (pFormFiller)
		return pFormFiller;

	if (bRegister)
	{
		CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pAnnot;		

		int nFieldType = pWidget->GetFieldType();
		switch(nFieldType)
		{
 		case FIELDTYPE_PUSHBUTTON:
 			pFormFiller = new CFFL_PushButton(m_pApp, pWidget);
 			break;
		case FIELDTYPE_CHECKBOX:
			pFormFiller = new CFFL_CheckBox(m_pApp, pWidget);
			break;
 		case FIELDTYPE_RADIOBUTTON:
 			pFormFiller = new CFFL_RadioButton(m_pApp, pWidget);
 			break;
 		case FIELDTYPE_TEXTFIELD:
			pFormFiller = new CFFL_TextField(m_pApp, pWidget);
			break;
		case FIELDTYPE_LISTBOX:
			pFormFiller = new CFFL_ListBox(m_pApp, pWidget);
			break;
		case FIELDTYPE_COMBOBOX:
			pFormFiller = new CFFL_ComboBox(m_pApp, pWidget);
			break;
		case FIELDTYPE_UNKNOWN:
		default:
			pFormFiller = NULL;
			break;
		}

		if (pFormFiller)
		{
			m_Maps.SetAt(pAnnot, pFormFiller);
		}
	}

	return pFormFiller;
}

void CFFL_IFormFiller::RemoveFormFiller(CPDFSDK_Annot* pAnnot)
{
	if ( pAnnot != NULL )
	{
		UnRegisterFormFiller( pAnnot );
	}
}

void CFFL_IFormFiller::UnRegisterFormFiller(CPDFSDK_Annot* pAnnot)
{
	CFFL_FormFiller * pFormFiller = NULL;

	if (m_Maps.Lookup(pAnnot,pFormFiller))
	{
		if (pFormFiller)
			delete pFormFiller;
		m_Maps.RemoveKey(pAnnot);
	}
}

void CFFL_IFormFiller::SetFocusAnnotTab(CPDFSDK_Annot* pWidget, FX_BOOL bSameField, FX_BOOL bNext)
{

}

void CFFL_IFormFiller::QueryWherePopup(void* pPrivateData, FX_FLOAT fPopupMin,FX_FLOAT fPopupMax, FX_INT32 & nRet, FX_FLOAT & fPopupRet)
{
	ASSERT(pPrivateData != NULL);

	CFFL_PrivateData* pData = (CFFL_PrivateData*)pPrivateData;

	


	CPDF_Rect rcPageView(0,0,0,0);
	rcPageView.right = pData->pWidget->GetPDFPage()->GetPageWidth();
	rcPageView.bottom = pData->pWidget->GetPDFPage()->GetPageHeight();
	rcPageView.Normalize();


	ASSERT(pData->pWidget != NULL);
	CPDF_Rect rcAnnot = pData->pWidget->GetRect();

	FX_FLOAT fTop = 0.0f;
	FX_FLOAT fBottom = 0.0f;

	CPDFSDK_Widget * pWidget = (CPDFSDK_Widget*)pData->pWidget;
	switch (pWidget->GetRotate() / 90)
	{
	default:
	case 0:
		fTop = rcPageView.top - rcAnnot.top;
		fBottom = rcAnnot.bottom - rcPageView.bottom;
		break;
	case 1:
		fTop = rcAnnot.left - rcPageView.left;
		fBottom = rcPageView.right - rcAnnot.right;
		break;
	case 2:
		fTop = rcAnnot.bottom - rcPageView.bottom;
		fBottom = rcPageView.top - rcAnnot.top;
		break;
	case 3:
		fTop = rcPageView.right - rcAnnot.right;
		fBottom = rcAnnot.left - rcPageView.left;
		break;
	}

	FX_FLOAT fFactHeight = 0;
	FX_BOOL bBottom = TRUE;
	FX_FLOAT fMaxListBoxHeight = 0;
	if (fPopupMax > FFL_MAXLISTBOXHEIGHT)
	{
		if (fPopupMin > FFL_MAXLISTBOXHEIGHT)
		{
			fMaxListBoxHeight = fPopupMin;
		}
		else
		{
			fMaxListBoxHeight = FFL_MAXLISTBOXHEIGHT;
		}
	}
	else
		fMaxListBoxHeight = fPopupMax;

	if (fBottom > fMaxListBoxHeight)
	{
		fFactHeight = fMaxListBoxHeight;
		bBottom = TRUE;
	}
	else
	{
		if (fTop > fMaxListBoxHeight)
		{
			fFactHeight = fMaxListBoxHeight;
			bBottom = FALSE;
		}
		else
		{
			if (fTop > fBottom)
			{
				fFactHeight = fTop;
				bBottom = FALSE;
			}
			else
			{
				fFactHeight = fBottom;
				bBottom = TRUE;
			}
		}
	}

	nRet = bBottom ? 0 : 1;
	fPopupRet = fFactHeight;
}

void CFFL_IFormFiller::OnSetWindowRect(void* pPrivateData, const CPDF_Rect & rcWindow)
{
	ASSERT(pPrivateData != NULL);

	CFFL_PrivateData* pData = (CFFL_PrivateData*)pPrivateData;

	if (CFFL_FormFiller* pFormFiller = GetFormFiller(pData->pWidget, TRUE))
	{

		CPDF_Rect rcOld = pFormFiller->PWLtoFFL(pFormFiller->GetWindowRect(pData->pPageView));
		CPDF_Rect rcNew = pFormFiller->PWLtoFFL(rcWindow);
		pFormFiller->SetWindowRect(pData->pPageView, rcWindow);

		CPDF_Rect unRect = rcOld;
		unRect.Union(rcNew);
		//FX_RECT rcRect = unRect.GetOutterRect();
		unRect.left = (FX_FLOAT)(unRect.left - 0.5);
		unRect.right = (FX_FLOAT)(unRect.right + 0.5);
		unRect.top = (FX_FLOAT)(unRect.top + 0.5);
		unRect.bottom = (FX_FLOAT)(unRect.bottom -0.5);
		m_pApp->FFI_Invalidate(pData->pWidget->GetPDFPage(), unRect.left, unRect.top, unRect.right, unRect.bottom);
	}
}

void CFFL_IFormFiller::OnKeyStroke(FX_BOOL bEditOrList, void* pPrivateData, FX_INT32 nKeyCode, CFX_WideString& strChange, 
								   const CFX_WideString& strChangeEx, FX_BOOL bKeyDown, 
								   FX_BOOL & bRC, FX_BOOL & bExit)
{
	ASSERT(pPrivateData != NULL);
	CFFL_PrivateData* pData = (CFFL_PrivateData*)pPrivateData;
	ASSERT(pData->pWidget != NULL);

	CFFL_FormFiller* pFormFiller = GetFormFiller(pData->pWidget, FALSE);
	ASSERT(pFormFiller != NULL);

	pFormFiller->OnKeyStroke(bKeyDown);
}

void CFFL_IFormFiller::OnKeyStrokeCommit(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, FX_BOOL& bRC, FX_BOOL& bExit, FX_DWORD nFlag)
{
	if (!m_bNotifying)
	{
		ASSERT(pWidget != NULL);
		if (pWidget->GetAAction(CPDF_AAction::KeyStroke))
		{
			m_bNotifying = TRUE;
			pWidget->ClearAppModified();

			ASSERT(pPageView != NULL);
// 			CReader_DocView* pDocView = pPageView->GetDocView();
// 			ASSERT(pDocView != NULL);
			
		

			PDFSDK_FieldAction fa;
			fa.bModifier = m_pApp->FFI_IsCTRLKeyDown(nFlag);
 			fa.bShift = m_pApp->FFI_IsSHIFTKeyDown(nFlag);
			fa.bWillCommit = TRUE;
			fa.nCommitKey = GetCommitKey();
			fa.bKeyDown = GetKeyDown();
			fa.bRC = TRUE;

			CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget, FALSE);
			ASSERT(pFormFiller != NULL);

			pFormFiller->GetActionData(pPageView, CPDF_AAction::KeyStroke, fa);
			pFormFiller->SaveState(pPageView);

			PDFSDK_FieldAction faOld = fa;
			pWidget->OnAAction(CPDF_AAction::KeyStroke, fa, pPageView);

			bRC = fa.bRC;
//			bExit = !IsValidAnnot(m_pApp, pDocument, pDocView, pPageView, pWidget);

			m_bNotifying = FALSE;
		}
	}
}

void CFFL_IFormFiller::OnValidate(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, FX_BOOL& bRC, FX_BOOL& bExit, FX_DWORD nFlag)
{
	if (!m_bNotifying)
	{
		ASSERT(pWidget != NULL);
		if (pWidget->GetAAction(CPDF_AAction::Validate))
		{
			m_bNotifying = TRUE;
			pWidget->ClearAppModified();

			ASSERT(pPageView != NULL);
// 			CReader_DocView* pDocView = pPageView->GetDocView();
// 			ASSERT(pDocView != NULL);
			
			

			PDFSDK_FieldAction fa;
			fa.bModifier = m_pApp->FFI_IsCTRLKeyDown(nFlag);
 			fa.bShift = m_pApp->FFI_IsSHIFTKeyDown(nFlag);
			fa.bKeyDown = GetKeyDown();
			fa.bRC = TRUE;

			CFFL_FormFiller* pFormFiller = GetFormFiller(pWidget, FALSE);
			ASSERT(pFormFiller != NULL);

			pFormFiller->GetActionData(pPageView, CPDF_AAction::Validate, fa);
			pFormFiller->SaveState(pPageView);

			PDFSDK_FieldAction faOld = fa;
			pWidget->OnAAction(CPDF_AAction::Validate, fa, pPageView);

			bRC = fa.bRC;
//			bExit = !IsValidAnnot(m_pApp, pDocument, pDocView, pPageView, pWidget);

			m_bNotifying = FALSE;
		}
	}
}

void CFFL_IFormFiller::OnCalculate(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, FX_BOOL& bExit, FX_DWORD nFlag)
{
	if (!m_bNotifying)
	{
		ASSERT(pWidget != NULL);
		ASSERT(pPageView != NULL);
// 		CReader_DocView* pDocView = pPageView->GetDocView();
// 		ASSERT(pDocView != NULL);
		CPDFSDK_Document* pDocument = pPageView->GetSDKDocument();
		ASSERT(pDocument != NULL);

		CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pDocument->GetInterForm();
		ASSERT(pInterForm != NULL);

		pInterForm->OnCalculate(pWidget->GetFormField());

//		bExit = !IsValidAnnot(m_pApp, pDocument, pDocView, pPageView, pWidget);

		m_bNotifying = FALSE;
	}
}

void CFFL_IFormFiller::OnFormat(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, FX_BOOL& bExit, FX_DWORD nFlag)
{
	if (!m_bNotifying)
	{
		ASSERT(pWidget != NULL);
		ASSERT(pPageView != NULL);
// 		CReader_DocView* pDocView = pPageView->GetDocView();
// 		ASSERT(pDocView != NULL);
		CPDFSDK_Document* pDocument = pPageView->GetSDKDocument();
		ASSERT(pDocument != NULL);

		CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pDocument->GetInterForm();
		ASSERT(pInterForm != NULL);

		FX_BOOL bFormated = FALSE;
		CFX_WideString sValue = pInterForm->OnFormat(pWidget->GetFormField(), GetCommitKey(), bFormated);

//		bExit = !IsValidAnnot(m_pApp, pDocument, pDocView, pPageView, pWidget);

		if (bExit) return;

		if (bFormated)
		{
			pInterForm->ResetFieldAppearance(pWidget->GetFormField(), sValue, TRUE);
			pInterForm->UpdateField(pWidget->GetFormField());
		}

		m_bNotifying = FALSE;
	}
}

// LRESULT CALLBACK CFFL_IFormFiller::FFL_WndProc(
// 									  int code,       // hook code
// 									  WPARAM wParam,  // virtual-key code
// 									  LPARAM lParam   // keystroke-message information
// 										)
// {
// 	if (code != HC_ACTION)
// 	{
// 		return CallNextHookEx (m_hookSheet, code, wParam, lParam);
// 	}
// 
// 	FXSYS_memcpy(&g_Msg, (void*)lParam, sizeof(MSG));	
// 
// 	return 0;
// }

// MSG	CFFL_IFormFiller::GetLastMessage()
// {
// 	return g_Msg;
// }

int CFFL_IFormFiller::GetCommitKey()
{
//	MSG msg = CFFL_IFormFiller::GetLastMessage();

	int nCommitKey = 0;
// 	switch (msg.message)
// 	{
// 	case WM_LBUTTONDOWN:
// 	case WM_LBUTTONUP:
// 		nCommitKey = 1;
// 		break;
// 	case WM_KEYDOWN:
// 		switch (msg.wParam)
// 		{
// 		case VK_RETURN:
// 			nCommitKey = 2;
// 			break;
// 		case VK_TAB:
// 			nCommitKey = 3;
// 			break;
// 		}
// 		break;
// 	}

	return nCommitKey;
}

FX_BOOL CFFL_IFormFiller::GetKeyDown()
{
	return TRUE;
// 	MSG msg = CFFL_IFormFiller::GetLastMessage();
// 
// 	return msg.message == WM_KEYDOWN || msg.message == WM_CHAR;
}

FX_BOOL	CFFL_IFormFiller::IsValidAnnot(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot)
{

	ASSERT(pPageView != NULL);
	ASSERT(pAnnot != NULL);

	if(pPageView)
		return pPageView->IsValidAnnot(pAnnot->GetPDFAnnot());
	else
		return FALSE;
}

void CFFL_IFormFiller::BeforeUndo(CPDFSDK_Document* pDocument)
{

}

void CFFL_IFormFiller::BeforeRedo(CPDFSDK_Document* pDocument)
{
	BeforeUndo(pDocument);
}

void CFFL_IFormFiller::AfterUndo(CPDFSDK_Document* pDocument)
{
}

void CFFL_IFormFiller::AfterRedo(CPDFSDK_Document* pDocument)
{
}

FX_BOOL	CFFL_IFormFiller::CanCopy(CPDFSDK_Document* pDocument)
{

	return FALSE;
}

FX_BOOL	CFFL_IFormFiller::CanCut(CPDFSDK_Document* pDocument)
{

	return FALSE;
}

FX_BOOL	CFFL_IFormFiller::CanPaste(CPDFSDK_Document* pDocument)
{

	return FALSE;
}

void CFFL_IFormFiller::DoCopy(CPDFSDK_Document* pDocument)
{
}

void CFFL_IFormFiller::DoCut(CPDFSDK_Document* pDocument)
{
}

void CFFL_IFormFiller::DoPaste(CPDFSDK_Document* pDocument)
{

}
void CFFL_IFormFiller::OnBeforeKeyStroke(FX_BOOL bEditOrList, void* pPrivateData, FX_INT32 nKeyCode,
											  CFX_WideString & strChange, const CFX_WideString& strChangeEx, 
											  int nSelStart, int nSelEnd,
										FX_BOOL bKeyDown, FX_BOOL & bRC, FX_BOOL & bExit, FX_DWORD nFlag)
{
	ASSERT(pPrivateData != NULL);
	CFFL_PrivateData* pData = (CFFL_PrivateData*)pPrivateData;
	ASSERT(pData->pWidget != NULL);
	
	CFFL_FormFiller* pFormFiller = GetFormFiller(pData->pWidget, FALSE);
	ASSERT(pFormFiller != NULL);
	
	if (!m_bNotifying)
	{
		if (pData->pWidget->GetAAction(CPDF_AAction::KeyStroke))
		{
			m_bNotifying = TRUE;
			int nAge = pData->pWidget->GetAppearanceAge();
			int nValueAge = pData->pWidget->GetValueAge();

			ASSERT(pData->pPageView != NULL);
			CPDFSDK_Document* pDocument  = pData->pPageView->GetSDKDocument();
			
			PDFSDK_FieldAction fa;
 			fa.bModifier = m_pApp->FFI_IsCTRLKeyDown(nFlag);
 			fa.bShift = m_pApp->FFI_IsSHIFTKeyDown(nFlag);
			fa.sChange = strChange;
			fa.sChangeEx = strChangeEx;
			fa.bKeyDown = bKeyDown;
			fa.bWillCommit = FALSE;
			fa.bRC = TRUE;
			fa.nSelStart = nSelStart;
			fa.nSelEnd = nSelEnd;


			pFormFiller->GetActionData(pData->pPageView, CPDF_AAction::KeyStroke, fa);
			pFormFiller->SaveState(pData->pPageView);
			
			if (pData->pWidget->OnAAction(CPDF_AAction::KeyStroke, fa, pData->pPageView))
			{
				if (!IsValidAnnot(pData->pPageView, pData->pWidget))
				{
					bExit = TRUE;
					m_bNotifying = FALSE;
					return;
				}
				
				if (nAge != pData->pWidget->GetAppearanceAge())
				{
					CPWL_Wnd* pWnd = pFormFiller->ResetPDFWindow(pData->pPageView, nValueAge == pData->pWidget->GetValueAge());
					pData = (CFFL_PrivateData*)pWnd->GetAttachedData();
					bExit = TRUE;
				}
				
				if (fa.bRC)
				{
					pFormFiller->SetActionData(pData->pPageView, CPDF_AAction::KeyStroke, fa);
					bRC = FALSE;
				}
				else
				{
					pFormFiller->RestoreState(pData->pPageView);
					bRC = FALSE;
				}
				
				if (pDocument->GetFocusAnnot() != pData->pWidget)
				{
					pFormFiller->CommitData(pData->pPageView,nFlag);
					bExit = TRUE;
				}
			}
			else
			{			
				if (!IsValidAnnot(pData->pPageView, pData->pWidget))
				{
					bExit = TRUE;
					m_bNotifying = FALSE;
					return;
				}
			}
			
			m_bNotifying = FALSE;
		}
	}
}

void	CFFL_IFormFiller::OnAfterKeyStroke(FX_BOOL bEditOrList, void* pPrivateData, FX_BOOL & bExit,FX_DWORD nFlag) 
{
	ASSERT(pPrivateData != NULL);
	CFFL_PrivateData* pData = (CFFL_PrivateData*)pPrivateData;
	ASSERT(pData->pWidget != NULL);
	
	CFFL_FormFiller* pFormFiller = GetFormFiller(pData->pWidget, FALSE);
	ASSERT(pFormFiller != NULL);
	
	if (!bEditOrList)
		pFormFiller->OnKeyStroke(bExit);
}
