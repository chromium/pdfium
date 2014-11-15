// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fpdfview.h"
#include "../include/fpdfformfill.h"
#include "../include/fsdk_define.h"
#include "../include/fsdk_mgr.h"


#include "../include/javascript/IJavaScript.h"


DLLEXPORT int STDCALL FPDPage_HasFormFieldAtPoint(FPDF_FORMHANDLE hHandle, FPDF_PAGE page,double page_x, double page_y)
{
	if(!page || !hHandle)
		return -1;
	CPDF_Page * pPage = (CPDF_Page*) page;

	CPDF_InterForm * pInterForm = NULL;
	pInterForm = new CPDF_InterForm(pPage->m_pDocument,FALSE);
	if (!pInterForm)
		return -1;
	CPDF_FormControl* pFormCtrl = pInterForm->GetControlAtPoint(pPage, (FX_FLOAT)page_x, (FX_FLOAT)page_y);
	if(!pFormCtrl)
	{
		delete pInterForm;
		return -1;
	}
	CPDF_FormField* pFormField = pFormCtrl->GetField();
	if(!pFormField)
	{
		delete pInterForm;
		return -1;
	}
	
	int nType = pFormField->GetFieldType();
	delete pInterForm;
	return nType;
}

DLLEXPORT FPDF_FORMHANDLE STDCALL FPDFDOC_InitFormFillEnvironment(FPDF_DOCUMENT document, FPDF_FORMFILLINFO* formInfo)
{
	if(!document || !formInfo || formInfo->version!=1)
		return NULL;
	CPDF_Document * pDocument = (CPDF_Document*) document;
 	CPDFDoc_Environment * pEnv = NULL;
	pEnv = new CPDFDoc_Environment(pDocument);
	if (!pEnv)
		return NULL;
	pEnv->RegAppHandle(formInfo);

	if(pEnv->GetPDFDocument())
	{
		CPDFSDK_Document* pSDKDoc = new CPDFSDK_Document(pEnv->GetPDFDocument(), pEnv);
		if(pSDKDoc)
			pEnv->SetCurrentDoc(pSDKDoc);
	}
	return pEnv;
}

DLLEXPORT void STDCALL FPDFDOC_ExitFormFillEnvironment(FPDF_FORMHANDLE hHandle)
{
	if(!hHandle)
		return; 
	CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(pSDKDoc)
	{
		((CPDFDoc_Environment*)hHandle)->SetCurrentDoc(NULL);
		delete pSDKDoc;
	}
	delete (CPDFDoc_Environment*)hHandle;
	hHandle = NULL;
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnMouseMove(FPDF_FORMHANDLE hHandle, FPDF_PAGE page, int modifier, double page_x, double page_y)
{	
	if (!hHandle || !page)
		return FALSE;
// 	CPDF_Page * pPage = (CPDF_Page*) page;
// 	CPDF_Document * pDoc = pPage->m_pDocument;
//	CPDFDoc_Environment* pEnv = (CPDFDoc_Environment*)hHandle;
	CPDFSDK_Document* pFXDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(!pFXDoc)
		return FALSE;
	CPDFSDK_PageView* pPageView = pFXDoc->GetPageView((CPDF_Page*)page);
	if(!pPageView)
		return FALSE;
	
// 	double page_x = 0;
// 	double page_y = 0;
//	pEnv->FFI_DeviceToPage(page, point_x, point_y, &page_x, &page_y);
	CPDF_Point pt((FX_FLOAT)page_x, (FX_FLOAT)page_y);
	return pPageView->OnMouseMove(pt, modifier);
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnLButtonDown(FPDF_FORMHANDLE hHandle, FPDF_PAGE page, int modifier, double page_x, double page_y)
{
	if (!hHandle || !page)
		return FALSE;
	CPDFSDK_Document* pFXDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(!pFXDoc)
		return FALSE;
	CPDFSDK_PageView* pPageView = pFXDoc->GetPageView((CPDF_Page*)page);
	if(!pPageView)
		return FALSE;
// 	double page_x = 0;
// 	double page_y = 0;
// 	pEnv->FFI_DeviceToPage(page, point_x, point_y, &page_x, &page_y);
	CPDF_Point pt((FX_FLOAT)page_x, (FX_FLOAT)page_y);
 	return pPageView->OnLButtonDown(pt, modifier);
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnLButtonUp(FPDF_FORMHANDLE hHandle, FPDF_PAGE page, int modifier, double page_x, double page_y)
{
	if (!hHandle || !page)
		return FALSE;
	CPDFSDK_Document* pFXDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(!pFXDoc)
		return FALSE;
	CPDFSDK_PageView* pPageView = pFXDoc->GetPageView((CPDF_Page*)page);
	if(!pPageView)
		return FALSE;
// 	double page_x = 0;
// 	double page_y = 0;
// 	pEnv->FFI_DeviceToPage(page, point_x, point_y, &page_x, &page_y);
	CPDF_Point pt((FX_FLOAT)page_x, (FX_FLOAT)page_y);
	return pPageView->OnLButtonUp(pt, modifier);
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnKeyDown(FPDF_FORMHANDLE hHandle, FPDF_PAGE page, int nKeyCode, int modifier)
{
	if (!hHandle || !page)
		return FALSE;
	CPDFSDK_Document* pFXDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(!pFXDoc)
		return FALSE;
	CPDFSDK_PageView* pPageView = pFXDoc->GetPageView((CPDF_Page*)page);
	if(!pPageView)
		return FALSE;
	
	
	return pPageView->OnKeyDown(nKeyCode, modifier);
}

DLLEXPORT FPDF_BOOL STDCALL FORM_OnKeyUp(FPDF_FORMHANDLE hHandle, FPDF_PAGE page, int nKeyCode, int modifier)
{
	if (!hHandle || !page)
		return FALSE;
	CPDFSDK_Document* pFXDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(!pFXDoc)
		return FALSE;
	CPDFSDK_PageView* pPageView = pFXDoc->GetPageView((CPDF_Page*)page);
	if(!pPageView)
		return FALSE;
	
	
	return pPageView->OnKeyUp(nKeyCode, modifier);
}


DLLEXPORT FPDF_BOOL STDCALL FORM_OnChar(FPDF_FORMHANDLE hHandle, FPDF_PAGE page, int nChar,  int modifier)
{
	if (!hHandle || !page)
		return FALSE;
	CPDFSDK_Document* pFXDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(!pFXDoc)
		return FALSE;
	CPDFSDK_PageView* pPageView = pFXDoc->GetPageView((CPDF_Page*)page);
	if(!pPageView)
		return FALSE;
	return pPageView->OnChar(nChar, modifier);

}

DLLEXPORT FPDF_BOOL STDCALL FORM_ForceToKillFocus(FPDF_FORMHANDLE hHandle)
{
	if(!hHandle)
		return FALSE;
	CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(!pSDKDoc)
		return FALSE;
	//Kill the current focus. 
	return pSDKDoc->KillFocusAnnot(0);
}

DLLEXPORT void STDCALL FPDF_FFLDraw(FPDF_FORMHANDLE hHandle, FPDF_BITMAP bitmap, FPDF_PAGE page, int start_x, int start_y, 
												  int size_x, int size_y, int rotate, int flags)
{
	if (!hHandle || !page)
		return ;
	CPDF_Page* pPage = (CPDF_Page*)page;
	
	CPDF_RenderOptions options;
	if (flags & FPDF_LCD_TEXT)
		options.m_Flags |= RENDER_CLEARTYPE;
	else
		options.m_Flags &= ~RENDER_CLEARTYPE;
	
	//Grayscale output
	if (flags & FPDF_GRAYSCALE)
	{
		options.m_ColorMode = RENDER_COLOR_GRAY;
		options.m_ForeColor = 0;
		options.m_BackColor = 0xffffff;
	}
	
	options.m_AddFlags = flags >> 8;

	options.m_pOCContext = FX_NEW CPDF_OCContext(pPage->m_pDocument);

	//FXMT_CSLOCK_OBJ(&pPage->m_PageLock);
	
	CFX_AffineMatrix matrix;
	pPage->GetDisplayMatrix(matrix, start_x, start_y, size_x, size_y, rotate); 
	
	FX_RECT clip;
	clip.left = start_x;
	clip.right = start_x + size_x;
	clip.top = start_y;
	clip.bottom = start_y + size_y;

#ifdef _SKIA_SUPPORT_
	CFX_SkiaDevice* pDevice = FX_NEW CFX_SkiaDevice;
#else
	CFX_FxgeDevice* pDevice = NULL;
	pDevice = FX_NEW CFX_FxgeDevice;
#endif

	if (!pDevice)
		return;
	pDevice->Attach((CFX_DIBitmap*)bitmap);
	pDevice->SaveState();
	pDevice->SetClip_Rect(&clip);
	

	CPDF_RenderContext* pContext = NULL;
	pContext = FX_NEW CPDF_RenderContext;
	if (!pContext)
	{
		delete pDevice;
		pDevice = NULL;
		return;
	}


//	CPDF_Document* pDoc = pPage->m_pDocument;
	CPDFDoc_Environment* pEnv = (CPDFDoc_Environment*)hHandle;
	CPDFSDK_Document* pFXDoc = pEnv->GetCurrentDoc();
	if(!pFXDoc)
	{
		delete pContext;
		delete pDevice;
		pContext = NULL;
		pDevice = NULL;
		return;
	}
	if(CPDFSDK_PageView* pPageView = pFXDoc->GetPageView(pPage))
	{
		pPageView->PageView_OnDraw(pDevice, &matrix, &options);
	}
	pDevice->RestoreState();

	if(options.m_pOCContext)
	{
		delete options.m_pOCContext;
		options.m_pOCContext = NULL;
	}
	if(pContext)
	{
		delete pContext;
		pContext = NULL;
	}
	if(pDevice)
	{
		delete pDevice;
		pDevice = NULL;
	}

}

DLLEXPORT void STDCALL FPDF_SetFormFieldHighlightColor(FPDF_FORMHANDLE hHandle, int fieldType, unsigned long color)
{
	if (!hHandle)
		return;
//	CPDFDoc_Environment* pEnv = (CPDFDoc_Environment* )hHandle;
	CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(pSDKDoc)
	{
		if(CPDFSDK_InterForm* pInterForm = pSDKDoc->GetInterForm())
		{
			pInterForm->SetHighlightColor(color, fieldType);
		}
	
	}

}

DLLEXPORT void STDCALL FPDF_SetFormFieldHighlightAlpha(FPDF_FORMHANDLE hHandle, unsigned char alpha)
{
	if (!hHandle)
		return;
	CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(pSDKDoc)
	{
		if(CPDFSDK_InterForm* pInterForm = pSDKDoc->GetInterForm())
			pInterForm->SetHighlightAlpha(alpha);
	}
}

DLLEXPORT void STDCALL FPDF_RemoveFormFieldHighlight(FPDF_FORMHANDLE hHandle)
{
	if (!hHandle)
		return;
	CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(pSDKDoc)
	{
		if(CPDFSDK_InterForm* pInterForm = pSDKDoc->GetInterForm())
			pInterForm->RemoveAllHighLight();
	}
}

DLLEXPORT void STDCALL FORM_OnAfterLoadPage(FPDF_PAGE page, FPDF_FORMHANDLE hHandle)
{
	if(!hHandle || !page)
		return;
	CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(!pSDKDoc)
		return;
	CPDF_Page* pPage = (CPDF_Page*)page;
	CPDFSDK_PageView* pPageView = pSDKDoc->GetPageView(pPage, TRUE);
	if(pPageView)
	{
		pPageView->SetValid(TRUE);
	}	
}

DLLEXPORT void STDCALL FORM_OnBeforeClosePage(FPDF_PAGE page, FPDF_FORMHANDLE hHandle)
{
	if(!hHandle || !page)
		return;
	CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	CPDF_Page* pPage = (CPDF_Page*)page;
	CPDFSDK_PageView* pPageView = pSDKDoc->GetPageView(pPage, FALSE);
	if(pPageView)
	{
		pPageView->SetValid(FALSE);
		// ReMovePageView() takes care of the delete for us.
		pSDKDoc->ReMovePageView(pPage);
	}
}
DLLEXPORT void STDCALL FORM_DoDocumentJSAction(FPDF_FORMHANDLE hHandle)
{
	if(!hHandle)
		return;
	if( CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc())
	{
		pSDKDoc->InitPageView();
		if(((CPDFDoc_Environment*)hHandle)->IsJSInitiated())
			pSDKDoc->ProcJavascriptFun();
	}	
}

DLLEXPORT void STDCALL FORM_DoDocumentOpenAction(FPDF_FORMHANDLE hHandle)
{
	if(!hHandle)
		return;
	if( CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc())
	{
		if(((CPDFDoc_Environment*)hHandle)->IsJSInitiated())
			pSDKDoc->ProcOpenAction();
	}
}
DLLEXPORT void STDCALL FORM_DoDocumentAAction(FPDF_FORMHANDLE hHandle, int aaType)
{
	if(!hHandle)
		return;
	CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	if(pSDKDoc)
	{
		CPDF_Document* pDoc = pSDKDoc->GetDocument();
		CPDF_Dictionary* pDic = pDoc->GetRoot();
		if (!pDic)
			return;
		CPDF_AAction aa = pDic->GetDict(FX_BSTRC("AA"));
		
		if(aa.ActionExist((CPDF_AAction::AActionType)aaType))
		{
			CPDF_Action action = aa.GetAction((CPDF_AAction::AActionType)aaType);
			CPDFSDK_ActionHandler *pActionHandler = ((CPDFDoc_Environment*)hHandle)->GetActionHander();
			ASSERT(pActionHandler != NULL);
			pActionHandler->DoAction_Document(action, (CPDF_AAction::AActionType)aaType, pSDKDoc);
		}
	}
}
DLLEXPORT void STDCALL FORM_DoPageAAction(FPDF_PAGE page, FPDF_FORMHANDLE hHandle, int aaType)
{
	if(!hHandle || !page)
		return;
	CPDFSDK_Document* pSDKDoc = ((CPDFDoc_Environment*)hHandle)->GetCurrentDoc();
	CPDF_Page* pPage = (CPDF_Page*)page;
	CPDFSDK_PageView* pPageView = pSDKDoc->GetPageView(pPage, FALSE);
	if(pPageView)
	{
		CPDFDoc_Environment *pEnv = pSDKDoc->GetEnv();
		ASSERT(pEnv != NULL);
			
		CPDFSDK_ActionHandler *pActionHandler = pEnv->GetActionHander();
		ASSERT(pActionHandler != NULL);
		
		CPDF_Dictionary *pPageDict = pPage->m_pFormDict;
		ASSERT(pPageDict != NULL);
		 
		CPDF_AAction aa = pPageDict->GetDict(FX_BSTRC("AA"));

		FX_BOOL bExistOAAction = FALSE;
		FX_BOOL bExistCAAction = FALSE;
		if (FPDFPAGE_AACTION_OPEN == aaType)
		{
			bExistOAAction = aa.ActionExist(CPDF_AAction::OpenPage);
			if (bExistOAAction)
			{
				CPDF_Action action = aa.GetAction(CPDF_AAction::OpenPage);
				pActionHandler->DoAction_Page(action, CPDF_AAction::OpenPage, pSDKDoc);
			}
		}
		else
		{
			bExistCAAction = aa.ActionExist(CPDF_AAction::ClosePage);
			if (bExistCAAction)
			{
				CPDF_Action action = aa.GetAction(CPDF_AAction::ClosePage);
				pActionHandler->DoAction_Page(action, CPDF_AAction::ClosePage, pSDKDoc);
			}
		}
	}
}


