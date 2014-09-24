// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fsdk_define.h"
#include "../include/fsdk_mgr.h"
#include "../include/fpdf_ext.h"
#include "../include/formfiller/FFL_FormFiller.h"
#include "../include/javascript/IJavaScript.h"

#if _FX_OS_ == _FX_ANDROID_
#include "time.h"
#else
#include <ctime>
#endif

//extern CPDFDoc_Environment* g_pFormFillApp;
class CFX_SystemHandler:public IFX_SystemHandler
{
public:
	CFX_SystemHandler(CPDFDoc_Environment* pEnv):m_pEnv(pEnv),m_nCharSet(-1) {}
public:
	virtual void				InvalidateRect(FX_HWND hWnd, FX_RECT rect) ;
	virtual void				OutputSelectedRect(void* pFormFiller, CPDF_Rect& rect);

	virtual FX_BOOL				IsSelectionImplemented();

	virtual CFX_WideString		GetClipboardText(FX_HWND hWnd){return L"";}
	virtual FX_BOOL				SetClipboardText(FX_HWND hWnd, CFX_WideString string) {return FALSE;}
	
	virtual void				ClientToScreen(FX_HWND hWnd, FX_INT32& x, FX_INT32& y) {}
	virtual void				ScreenToClient(FX_HWND hWnd, FX_INT32& x, FX_INT32& y) {}

	/*cursor style
	FXCT_ARROW	
	FXCT_NESW		
	FXCT_NWSE		
	FXCT_VBEAM		
	FXCT_HBEAM		
	FXCT_HAND
	*/
	virtual void				SetCursor(FX_INT32 nCursorType);

	virtual FX_HMENU			CreatePopupMenu() {return NULL;}
	virtual FX_BOOL				AppendMenuItem(FX_HMENU hMenu, FX_INT32 nIDNewItem, CFX_WideString string) {return FALSE;}
	virtual FX_BOOL				EnableMenuItem(FX_HMENU hMenu, FX_INT32 nIDItem, FX_BOOL bEnabled) {return FALSE;}
	virtual FX_INT32			TrackPopupMenu(FX_HMENU hMenu, FX_INT32 x, FX_INT32 y, FX_HWND hParent) {return -1;}
	virtual void				DestroyMenu(FX_HMENU hMenu) {}

	virtual CFX_ByteString		GetNativeTrueTypeFont(FX_INT32 nCharset);
	virtual FX_BOOL				FindNativeTrueTypeFont(FX_INT32 nCharset, CFX_ByteString sFontFaceName);
	virtual CPDF_Font*			AddNativeTrueTypeFontToPDF(CPDF_Document* pDoc, CFX_ByteString sFontFaceName, FX_BYTE nCharset);

	virtual FX_INT32			SetTimer(FX_INT32 uElapse, TimerCallback lpTimerFunc) ;
	virtual void				KillTimer(FX_INT32 nID) ;


	virtual FX_BOOL				IsSHIFTKeyDown(FX_DWORD nFlag) {return m_pEnv->FFI_IsSHIFTKeyDown(nFlag);}
	virtual FX_BOOL				IsCTRLKeyDown(FX_DWORD nFlag) {return m_pEnv->FFI_IsCTRLKeyDown(nFlag);}
	virtual FX_BOOL				IsALTKeyDown(FX_DWORD nFlag) {return m_pEnv->FFI_IsALTKeyDown(nFlag);}
	virtual FX_BOOL				IsINSERTKeyDown(FX_DWORD nFlag) {return m_pEnv->FFI_IsINSERTKeyDown(nFlag);}

	virtual	FX_SYSTEMTIME		GetLocalTime();

	virtual FX_INT32			GetCharSet() {return m_nCharSet;}
	virtual void 				SetCharSet(FX_INT32 nCharSet) {m_nCharSet = nCharSet;}
private:
	CPDFDoc_Environment* m_pEnv;
	int		m_nCharSet;
};

void CFX_SystemHandler::SetCursor(FX_INT32 nCursorType)
{

	m_pEnv->FFI_SetCursor(nCursorType);
}

void CFX_SystemHandler::InvalidateRect(FX_HWND hWnd, FX_RECT rect)
{
	//g_pFormFillApp->FFI_Invalidate();
	CPDFSDK_Annot* pSDKAnnot = (CPDFSDK_Annot*)hWnd;
	CPDF_Page* pPage = NULL;
	CPDFSDK_PageView* pPageView = NULL;
	pPageView = pSDKAnnot->GetPageView();
	pPage = pSDKAnnot->GetPDFPage();
	if(!pPage || !pPageView)
		return;
	CPDF_Matrix page2device;
	pPageView->GetCurrentMatrix(page2device);
	CPDF_Matrix device2page;
	device2page.SetReverse(page2device);
	FX_FLOAT left, top, right,bottom;
	device2page.Transform((FX_FLOAT)rect.left, (FX_FLOAT)rect.top, left, top);
	device2page.Transform((FX_FLOAT)rect.right, (FX_FLOAT)rect.bottom, right, bottom);
// 	m_pEnv->FFI_DeviceToPage(pPage, rect.left, rect.top, (double*)&left, (double*)&top);
// 	m_pEnv->FFI_DeviceToPage(pPage, rect.right, rect.bottom, (double*)&right, (double*)&bottom);
	CPDF_Rect rcPDF(left, bottom, right, top);
	rcPDF.Normalize();

	m_pEnv->FFI_Invalidate(pPage, rcPDF.left, rcPDF.top, rcPDF.right, rcPDF.bottom);
}
void CFX_SystemHandler::OutputSelectedRect(void* pFormFiller, CPDF_Rect& rect)
{
	CFFL_FormFiller* pFFL = (CFFL_FormFiller*)pFormFiller;
	if(pFFL)
	{
		CPDF_Point leftbottom = CPDF_Point(rect.left, rect.bottom);
		CPDF_Point righttop = CPDF_Point(rect.right, rect.top);
		CPDF_Point ptA = pFFL->PWLtoFFL(leftbottom);
		CPDF_Point ptB = pFFL->PWLtoFFL(righttop);	


		CPDFSDK_Annot* pAnnot  = pFFL->GetSDKAnnot();
		ASSERT(pAnnot);
		CPDF_Page* pPage = pAnnot->GetPDFPage();
		ASSERT(pPage);
		m_pEnv->FFI_OutputSelectedRect(pPage, ptA.x, ptB.y, ptB.x, ptA.y);
	}
	
}

FX_BOOL CFX_SystemHandler::IsSelectionImplemented()
{
	if(m_pEnv)
	{
		FPDF_FORMFILLINFO* pInfo = m_pEnv->GetFormFillInfo();
		if(pInfo && pInfo->FFI_OutputSelectedRect)
			return TRUE;
	}
	return FALSE;
}

CFX_ByteString CFX_SystemHandler::GetNativeTrueTypeFont(FX_INT32 nCharset)
{
	return "";
}

FX_BOOL	CFX_SystemHandler::FindNativeTrueTypeFont(FX_INT32 nCharset, CFX_ByteString sFontFaceName) 
{
	CFX_FontMgr* pFontMgr = CFX_GEModule::Get()->GetFontMgr();
//	FXFT_Face nFace = pFontMgr->FindSubstFont(sFontFaceName,TRUE,0,0,0,0,NULL);
//	FXFT_Face nFace  = pFontMgr->m_pBuiltinMapper->FindSubstFont(sFontFaceName,TRUE,0,0,0,0,NULL);

	if(pFontMgr)
	{
		CFX_FontMapper*	pFontMapper = pFontMgr->m_pBuiltinMapper;
		if(pFontMapper)
		{	
			int nSize = pFontMapper->m_InstalledTTFonts.GetSize();
			if(nSize ==0)
			{
				pFontMapper->LoadInstalledFonts();
				nSize = pFontMapper->m_InstalledTTFonts.GetSize();
			}
			
			for(int i=0; i<nSize; i++)
			{
				if(pFontMapper->m_InstalledTTFonts[i].Compare(sFontFaceName))
					return TRUE;
			}
		}

	}

	return FALSE;
// 	pFontMgr->m_FaceMap.Lookup(sFontFaceName,pFont);
// 	return (pFont!=NULL);
}

static int CharSet2CP(int charset)
{
	if(charset == 128)
		return 932;
	else if(charset == 134)
		return 936;
	else if(charset == 129)
		return 949;
	else if(charset == 136)
		return 950;
	return 0;
}
CPDF_Font* CFX_SystemHandler::AddNativeTrueTypeFontToPDF(CPDF_Document* pDoc, CFX_ByteString sFontFaceName, 
														 FX_BYTE nCharset) 
{
	if(pDoc)
	{
		CFX_Font* pFXFont = new CFX_Font();
		pFXFont->LoadSubst(sFontFaceName,TRUE,0,0,0,CharSet2CP(nCharset),FALSE);
		CPDF_Font* pFont = pDoc->AddFont(pFXFont,nCharset,FALSE);
		delete pFXFont;
		return pFont;
	}

	return NULL;
}


FX_INT32 CFX_SystemHandler::SetTimer(FX_INT32 uElapse, TimerCallback lpTimerFunc)
{
	return m_pEnv->FFI_SetTimer(uElapse, lpTimerFunc);
}
void CFX_SystemHandler::KillTimer(FX_INT32 nID)
{
	m_pEnv->FFI_KillTimer(nID);
}

FX_SYSTEMTIME CFX_SystemHandler::GetLocalTime()
{
	return m_pEnv->FFI_GetLocalTime();
}


CJS_RuntimeFactory* GetJSRuntimeFactory()
{
	static CJS_RuntimeFactory s_JSRuntimeFactory;
	return &s_JSRuntimeFactory;
}

CPDFDoc_Environment::CPDFDoc_Environment(CPDF_Document* pDoc) :
	m_pAnnotHandlerMgr(NULL),
	m_pActionHandler(NULL),
	m_pJSRuntime(NULL),
	m_pInfo(NULL),
	m_pSDKDoc(NULL),
	m_pPDFDoc(pDoc),
	m_pIFormFiller(NULL)
{

	m_pSysHandler = NULL;
	m_pSysHandler = new CFX_SystemHandler(this);

	
	m_pJSRuntimeFactory = NULL;
	m_pJSRuntimeFactory = GetJSRuntimeFactory();
	m_pJSRuntimeFactory->AddRef();
}

CPDFDoc_Environment::~CPDFDoc_Environment()
{

	if ( m_pIFormFiller )
	{
		delete m_pIFormFiller;
		m_pIFormFiller = NULL;
	}
	if(m_pJSRuntime && m_pJSRuntimeFactory)
		m_pJSRuntimeFactory->DeleteJSRuntime(m_pJSRuntime);
	m_pJSRuntimeFactory->Release();

	if(m_pSysHandler)
	{
		delete m_pSysHandler;
		m_pSysHandler = NULL;
	}

	if(m_pAnnotHandlerMgr)
	{
		delete m_pAnnotHandlerMgr;
		m_pAnnotHandlerMgr = NULL;
	}
	if(m_pActionHandler)
	{
		delete m_pActionHandler;
		m_pActionHandler = NULL;
	}


}


IFXJS_Runtime* CPDFDoc_Environment::GetJSRuntime()
{
	if(!IsJSInitiated())
		return NULL;
	assert(m_pJSRuntimeFactory);
	if(!m_pJSRuntime)
		m_pJSRuntime = m_pJSRuntimeFactory->NewJSRuntime(this);
	return m_pJSRuntime;
}

CPDFSDK_AnnotHandlerMgr* CPDFDoc_Environment::GetAnnotHandlerMgr()
{
	if(!m_pAnnotHandlerMgr)
		m_pAnnotHandlerMgr = new CPDFSDK_AnnotHandlerMgr(this);
	return m_pAnnotHandlerMgr;
}

CPDFSDK_ActionHandler* CPDFDoc_Environment::GetActionHander()
{
	if(!m_pActionHandler)
		m_pActionHandler = new CPDFSDK_ActionHandler(this);
	return m_pActionHandler;
}

int CPDFDoc_Environment::RegAppHandle(FPDF_FORMFILLINFO* pFFinfo)
{
	m_pInfo  = pFFinfo; 
	return TRUE;
}

CPDFSDK_Document* CPDFDoc_Environment::GetCurrentDoc()
{
	return m_pSDKDoc;
}

CFFL_IFormFiller* CPDFDoc_Environment::GetIFormFiller()
{
	if(!m_pIFormFiller)
		m_pIFormFiller = new CFFL_IFormFiller(this);
	return m_pIFormFiller;
}

FX_BOOL	CPDFDoc_Environment::IsJSInitiated()
{
	if(m_pInfo)
	{
		if(m_pInfo->m_pJsPlatform)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

CPDFSDK_Document::CPDFSDK_Document(CPDF_Document* pDoc,CPDFDoc_Environment* pEnv):m_pDoc(pDoc),
						m_pInterForm(NULL),m_pEnv(pEnv),m_pOccontent(NULL),m_bChangeMask(FALSE)
{
	m_pFocusAnnot = NULL;
}

CPDFSDK_Document::~CPDFSDK_Document()
{
	FX_POSITION pos = m_pageMap.GetStartPosition();
	while (pos) {
            CPDF_Page* pPage = NULL;
            CPDFSDK_PageView* pPageView = NULL;
            m_pageMap.GetNextAssoc(pos, pPage, pPageView);
            delete pPageView;
        }
        m_pageMap.RemoveAll();
	if(m_pInterForm)
	{
		m_pInterForm->Destroy();
		m_pInterForm = NULL;
	}
	if(m_pOccontent)
	{
		delete m_pOccontent;
		m_pOccontent = NULL;
	}
}

void CPDFSDK_Document::InitPageView()
{
	int nCount = m_pDoc->GetPageCount();
	for(int i=0; i<nCount; i++)
	{
	// To do
//		CPDF_Dictionary* pDic = m_pDoc->GetPage(i);
//		m_pageMap.SetAt(pDic, pPageView);
	}
}

void CPDFSDK_Document::AddPageView(CPDF_Page* pPDFPage, CPDFSDK_PageView* pPageView)
{
	m_pageMap.SetAt(pPDFPage, pPageView);
}

CPDFSDK_PageView* CPDFSDK_Document::GetPageView(CPDF_Page* pPDFPage, FX_BOOL ReNew)
{
	CPDFSDK_PageView* pPageView = (CPDFSDK_PageView*)m_pageMap.GetValueAt(pPDFPage);
	if(pPageView != NULL)
		return pPageView;
	if(ReNew)
	{
		pPageView = new CPDFSDK_PageView(this,pPDFPage);
		m_pageMap.SetAt(pPDFPage, pPageView);
		//Delay to load all the annotations, to avoid endless loop.
		pPageView->LoadFXAnnots();
	}
	return pPageView;

}

CPDFSDK_PageView* CPDFSDK_Document::GetCurrentView()
{
	CPDF_Page * pPage = (CPDF_Page *)m_pEnv->FFI_GetCurrentPage(m_pDoc);
	if(pPage)
		return this->GetPageView(pPage, TRUE);
	return NULL;
}

CPDFSDK_PageView* CPDFSDK_Document::GetPageView(int nIndex)
{
	CPDFSDK_PageView * pTempPageView = NULL;
	CPDF_Page * pTempPage = (CPDF_Page*)m_pEnv->FFI_GetPage(m_pDoc,nIndex);
	if(!pTempPage)
		return NULL;

	m_pageMap.Lookup(pTempPage, pTempPageView);
	
	ASSERT(pTempPageView != NULL);
	
	return pTempPageView;
}

void CPDFSDK_Document:: ProcJavascriptFun()
{
	CPDF_Document* pPDFDoc = this->GetDocument();
	CPDF_DocJSActions docJS(pPDFDoc);
	int iCount = docJS.CountJSActions();
	if (iCount < 1) return;
	for (int i = 0; i < iCount; i ++)
	{
		CFX_ByteString csJSName;
		CPDF_Action jsAction = docJS.GetJSAction(i, csJSName);
		if(m_pEnv->GetActionHander())
			m_pEnv->GetActionHander()->DoAction_JavaScript(jsAction,CFX_WideString::FromLocal(csJSName),this);
	}
	
}

FX_BOOL CPDFSDK_Document::ProcOpenAction()
{
	if(!m_pDoc) return FALSE;
	
	CPDF_Dictionary* pRoot = m_pDoc->GetRoot();	
	if (!pRoot)	return FALSE;
	CPDF_Object* pOpenAction = pRoot->GetDict("OpenAction");//
	if(!pOpenAction) pOpenAction = pRoot->GetArray("OpenAction");//
	if(!pOpenAction) return FALSE;
	
	if(pOpenAction->GetType()==PDFOBJ_ARRAY)
	{	
	}
	else if(pOpenAction->GetType()==PDFOBJ_DICTIONARY)
	{	
		CPDF_Dictionary * pDict=(CPDF_Dictionary*)pOpenAction;	
		CPDF_Action Action = pDict;
		
		if(m_pEnv->GetActionHander())
			m_pEnv->GetActionHander()->DoAction_DocOpen(Action,this);		
	}	
	else
	{
		return FALSE;			
	}	
	return TRUE;
}

CPDF_OCContext*	CPDFSDK_Document::GetOCContext()
{
	if(!m_pOccontent)
		m_pOccontent = new CPDF_OCContext(m_pDoc);
	return m_pOccontent;
}

void CPDFSDK_Document::ReMovePageView(CPDF_Page* pPDFPage)
{
	CPDFSDK_PageView* pPageView = (CPDFSDK_PageView*)m_pageMap.GetValueAt(pPDFPage);
	if(pPageView && !pPageView->IsLocked())
	{
		delete pPageView;
		m_pageMap.RemoveKey(pPDFPage);
	}
}

CPDF_Page * CPDFSDK_Document::GetPage(int nIndex)
{
	CPDF_Page * pTempPage = (CPDF_Page*)m_pEnv->FFI_GetPage(m_pDoc,nIndex);
	if(!pTempPage)
		return NULL;
	return pTempPage;
}

CPDFSDK_InterForm* CPDFSDK_Document::GetInterForm()
{
	if(!m_pInterForm)
		m_pInterForm = new CPDFSDK_InterForm(this);
	return m_pInterForm;
}

void CPDFSDK_Document::UpdateAllViews(CPDFSDK_PageView* pSender, CPDFSDK_Annot* pAnnot)
{
	
	FX_POSITION pos = m_pageMap.GetStartPosition();
	CPDF_Page * pPage = NULL;
	CPDFSDK_PageView * pPageView = NULL;
	while(pos)
	{
		m_pageMap.GetNextAssoc(pos, pPage, pPageView);

		if(pPageView != pSender)
		{
			pPageView->UpdateView(pAnnot);
		}
	}
}

CPDFSDK_Annot* CPDFSDK_Document::GetFocusAnnot()
{
	return this->m_pFocusAnnot;	
}

FX_BOOL CPDFSDK_Document::SetFocusAnnot(CPDFSDK_Annot* pAnnot,FX_UINT nFlag)
{

	if(m_pFocusAnnot==pAnnot) return TRUE;
	
	if(m_pFocusAnnot)
	{
		if(!this->KillFocusAnnot(nFlag) ) return FALSE;	
	}
	CPDFSDK_PageView* pPageView = pAnnot->GetPageView();
	if(pAnnot && pPageView->IsValid())
	{
		CPDFSDK_AnnotHandlerMgr *pAnnotHandler=m_pEnv->GetAnnotHandlerMgr();

		if(pAnnotHandler&&!m_pFocusAnnot)
		{
			if (!pAnnotHandler->Annot_OnSetFocus(pAnnot,nFlag))
				return FALSE;
			if(!m_pFocusAnnot)
			{
				this->m_pFocusAnnot=pAnnot;
				return TRUE;
			}
		}			
	}
	return FALSE;
}

FX_BOOL CPDFSDK_Document::KillFocusAnnot(FX_UINT nFlag)
{
	if(m_pFocusAnnot)
	{
		CPDFSDK_AnnotHandlerMgr *pAnnotHandler=m_pEnv->GetAnnotHandlerMgr();
		if(pAnnotHandler)
		{
			CPDFSDK_Annot* pFocusAnnot = m_pFocusAnnot;
			m_pFocusAnnot = NULL;
			if(pAnnotHandler->Annot_OnKillFocus(pFocusAnnot, nFlag))
			{
				
				if(pFocusAnnot->GetType() == FX_BSTRC("Widget"))
				{
					CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)pFocusAnnot;
					int nFieldType = pWidget->GetFieldType();
					if(FIELDTYPE_TEXTFIELD == nFieldType || FIELDTYPE_COMBOBOX == nFieldType)
						m_pEnv->FFI_OnSetFieldInputFocus(NULL, NULL, 0, FALSE);
				}

				if(!m_pFocusAnnot)
					return TRUE;
			}
			else
			{
				m_pFocusAnnot = pFocusAnnot;
			}
		}
	}
	return FALSE;
}

FX_BOOL	CPDFSDK_Document::DeletePages(int nStart, int  nCount)
{
	if ( nStart < 0 || nStart >= GetPageCount() || nCount <= 0 )
	{
		return FALSE;
	}

	CPDF_Page * pTempPage = NULL;
	for ( int i = nCount-1; i >= 0; i-- )
	{
		pTempPage = GetPage(nStart+i);
		if ( pTempPage != NULL )
		{
			ReMovePageView(pTempPage);
		}
	}
	return TRUE;
}

void CPDFSDK_Document::OnCloseDocument()
{
	KillFocusAnnot();
}

FX_BOOL CPDFSDK_Document::GetPermissions(int nFlag)
{
	FX_DWORD dwPermissions = m_pDoc->GetUserPermissions();
	return dwPermissions&nFlag;
}

IFXJS_Runtime * CPDFSDK_Document::GetJsRuntime()
{
	ASSERT(m_pEnv!=NULL);
	return m_pEnv->GetJSRuntime();
}

CFX_WideString	CPDFSDK_Document::GetPath() 
{
	ASSERT(m_pEnv != NULL);
	return m_pEnv->JS_docGetFilePath();
}


CPDFSDK_PageView::CPDFSDK_PageView(CPDFSDK_Document* pSDKDoc,CPDF_Page* page):m_page(page),m_pSDKDoc(pSDKDoc)
{
	CPDFSDK_InterForm* pInterForm = pSDKDoc->GetInterForm();
	if(pInterForm)
	{
		CPDF_InterForm* pPDFInterForm = pInterForm->GetInterForm();
		pPDFInterForm->FixPageFields(page);
	}
        m_page->SetPrivateData((FX_LPVOID)m_page, (FX_LPVOID)this, NULL);
	m_fxAnnotArray.RemoveAll();

	m_bEnterWidget = FALSE;
	m_bExitWidget = FALSE;
	m_bOnWidget = FALSE;
	m_CaptureWidget = NULL;
	m_bValid = FALSE;
        m_bLocked = FALSE;
        m_bTakeOverPage = FALSE;
}

CPDFSDK_PageView::~CPDFSDK_PageView()
{
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();	
	int nAnnotCount = m_fxAnnotArray.GetSize();

	for (int i=0; i<nAnnotCount; i++)
	{
		CPDFSDK_Annot* pAnnot = (CPDFSDK_Annot*)m_fxAnnotArray.GetAt(i);
		//if there is a focused annot on the page, we should kill the focus first.
		if(pAnnot == m_pSDKDoc->GetFocusAnnot())
			KillFocusAnnot();
		CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
		ASSERT(pAnnotHandlerMgr);
		pAnnotHandlerMgr->ReleaseAnnot(pAnnot);
	}
	m_fxAnnotArray.RemoveAll();
	if(m_pAnnotList)
	{
		delete m_pAnnotList;
		m_pAnnotList = NULL;
	}
        m_page->RemovePrivateData((FX_LPVOID)m_page);
        if(m_bTakeOverPage) {
            delete m_page;
        }
}

void CPDFSDK_PageView::PageView_OnDraw(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,CPDF_RenderOptions* pOptions)
{
	m_curMatrix = *pUser2Device;
	
	//	m_pAnnotList->DisplayAnnots(m_page, pDevice, pUser2Device, FALSE, pOptions);
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	CPDFSDK_AnnotIterator annotIterator(this, TRUE);
	CPDFSDK_Annot * pSDKAnnot=NULL;
	int index=-1;
	while((pSDKAnnot = annotIterator.Next(index)))
	{
		CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
		ASSERT(pAnnotHandlerMgr);
		pAnnotHandlerMgr->Annot_OnDraw(this, pSDKAnnot, pDevice, pUser2Device, 0);
	}

}

CPDF_Annot* CPDFSDK_PageView::GetPDFAnnotAtPoint(FX_FLOAT pageX, FX_FLOAT pageY)
{
	
	int nCount = m_pAnnotList->Count();
	for(int i = 0 ; i<nCount; i++)
	{
		CPDF_Annot* pAnnot = m_pAnnotList->GetAt(i);
		CFX_FloatRect annotRect;
		pAnnot->GetRect(annotRect);
		if(annotRect.Contains(pageX, pageY))
			return pAnnot;
	}
	return NULL;
}

CPDF_Annot* CPDFSDK_PageView::GetPDFWidgetAtPoint(FX_FLOAT pageX, FX_FLOAT pageY)
{
	
	int nCount = m_pAnnotList->Count();
	for(int i = 0 ; i<nCount; i++)
	{
		CPDF_Annot* pAnnot = m_pAnnotList->GetAt(i);
		if(pAnnot->GetSubType() == "Widget")
		{
			CFX_FloatRect annotRect;
			pAnnot->GetRect(annotRect);
			if(annotRect.Contains(pageX, pageY))
				return pAnnot;
		}
	}
	return NULL;
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFXAnnotAtPoint(FX_FLOAT pageX, FX_FLOAT pageY)
{

	CPDFSDK_AnnotIterator annotIterator(this, FALSE);
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	CPDFSDK_AnnotHandlerMgr* pAnnotMgr = pEnv->GetAnnotHandlerMgr();
	CPDFSDK_Annot* pSDKAnnot = NULL;
	int index = -1;
	while((pSDKAnnot = annotIterator.Next(index)))
	{
		CPDF_Rect rc = pAnnotMgr->Annot_OnGetViewBBox(this, pSDKAnnot);
		if(rc.Contains(pageX, pageY))
			return pSDKAnnot;
	}

	return NULL;
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFXWidgetAtPoint(FX_FLOAT pageX, FX_FLOAT pageY)
{

	CPDFSDK_AnnotIterator annotIterator(this, FALSE);
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	CPDFSDK_AnnotHandlerMgr* pAnnotMgr = pEnv->GetAnnotHandlerMgr();
	CPDFSDK_Annot* pSDKAnnot = NULL;
	int index = -1;
	while((pSDKAnnot = annotIterator.Next(index)))
	{
		if(pSDKAnnot->GetType() == "Widget")
		{	
			pAnnotMgr->Annot_OnGetViewBBox(this, pSDKAnnot);
			CPDF_Point point(pageX, pageY);
			if (pAnnotMgr->Annot_OnHitTest(this, pSDKAnnot, point))
//			if(rc.Contains(pageX, pageY))
				return pSDKAnnot;
		}
	}
	
	return NULL;
}


FX_BOOL CPDFSDK_PageView::Annot_HasAppearance(CPDF_Annot* pAnnot)
{
	CPDF_Dictionary* pAnnotDic = pAnnot->m_pAnnotDict;
	if(pAnnotDic)
		return	pAnnotDic->KeyExist("AS");
	return FALSE;
}

CPDFSDK_Annot*	CPDFSDK_PageView::AddAnnot(CPDF_Annot * pPDFAnnot)
{
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	ASSERT(pEnv);
	CPDFSDK_AnnotHandlerMgr * pAnnotHandler= pEnv->GetAnnotHandlerMgr();	
	
	CPDFSDK_Annot* pSDKAnnot =NULL;
	
	if(pAnnotHandler)
	{
		pSDKAnnot = pAnnotHandler->NewAnnot(pPDFAnnot, this);
	}
	if(!pSDKAnnot)	 
		return NULL;

	m_fxAnnotArray.Add(pSDKAnnot);	
		
	if(pAnnotHandler)
	{	 		 	 
		pAnnotHandler->Annot_OnCreate(pSDKAnnot);
		
	}

	 return pSDKAnnot;
}

CPDFSDK_Annot* CPDFSDK_PageView::AddAnnot(CPDF_Dictionary * pDict)
{
	if(pDict)  
		return this->AddAnnot(pDict->GetString("Subtype"),pDict);
	 return NULL;
}

CPDFSDK_Annot* CPDFSDK_PageView::AddAnnot(FX_LPCSTR lpSubType,CPDF_Dictionary * pDict)
{
	return NULL;
}

FX_BOOL  CPDFSDK_PageView::DeleteAnnot(CPDFSDK_Annot* pAnnot)
{
	return FALSE;
}

CPDF_Document* CPDFSDK_PageView::GetPDFDocument()
{
	if(m_page)
	{
		return m_page->m_pDocument;
	}
	return NULL;
}

int	CPDFSDK_PageView::CountAnnots()
{
	return m_pAnnotList->Count();
}

CPDFSDK_Annot*	CPDFSDK_PageView::GetAnnot(int nIndex)
{
	int nCount = m_fxAnnotArray.GetSize();
	if ( nIndex < 0 || nIndex >= nCount )
	{
		return NULL;
	}

	return (CPDFSDK_Annot*)m_fxAnnotArray.GetAt(nIndex);
}

CPDFSDK_Annot* CPDFSDK_PageView::GetAnnotByDict(CPDF_Dictionary * pDict)
{
	int nCount = m_fxAnnotArray.GetSize();
 	for(int i=0; i<nCount; i++)
 	{
		CPDFSDK_Annot* pAnnot = (CPDFSDK_Annot*)m_fxAnnotArray.GetAt(i);
 		if(pDict==pAnnot->GetPDFAnnot()->m_pAnnotDict) 
 			return pAnnot;
 	}
	return NULL;
}

FX_BOOL CPDFSDK_PageView::OnLButtonDown(const CPDF_Point & point, FX_UINT nFlag)
{
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	ASSERT(pEnv);
	CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y);
	if(!pFXAnnot)
	{
		KillFocusAnnot(nFlag);
	}
	else
	{
		CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
		ASSERT(pAnnotHandlerMgr);

		FX_BOOL bRet = pAnnotHandlerMgr->Annot_OnLButtonDown(this, pFXAnnot, nFlag,point);
 		if(bRet)
 		{
 			SetFocusAnnot(pFXAnnot);
 		}
		return bRet;
	}
	return FALSE;
}


FX_BOOL CPDFSDK_PageView::OnLButtonUp(const CPDF_Point & point, FX_UINT nFlag)
{
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	ASSERT(pEnv);
	CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
	ASSERT(pAnnotHandlerMgr);
	CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y);
	CPDFSDK_Annot* pFocusAnnot = GetFocusAnnot();
	FX_BOOL bRet  = FALSE;
	if(pFocusAnnot && pFocusAnnot != pFXAnnot)
	{
		//Last focus Annot gets a chance to handle the event.
		bRet = pAnnotHandlerMgr->Annot_OnLButtonUp(this, pFocusAnnot, nFlag,point);
	}
	if(pFXAnnot && !bRet)
	{
		bRet = pAnnotHandlerMgr->Annot_OnLButtonUp(this, pFXAnnot, nFlag,point);
		return bRet;
	}
	return bRet;
}

FX_BOOL CPDFSDK_PageView::OnMouseMove(const CPDF_Point & point, int nFlag)
{

	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
	ASSERT(pAnnotHandlerMgr);
	if(CPDFSDK_Annot* pFXAnnot = GetFXWidgetAtPoint(point.x, point.y))
	{
		if(m_CaptureWidget && m_CaptureWidget != pFXAnnot)
		{
			m_bExitWidget = TRUE;
			m_bEnterWidget = FALSE;
			pAnnotHandlerMgr->Annot_OnMouseExit(this, m_CaptureWidget, nFlag);
		}
		m_CaptureWidget = (CPDFSDK_Widget*)pFXAnnot;
		m_bOnWidget = TRUE;
		if(!m_bEnterWidget)
		{
			m_bEnterWidget = TRUE;
			m_bExitWidget = FALSE;
			pAnnotHandlerMgr->Annot_OnMouseEnter(this, pFXAnnot,nFlag);
		}
		pAnnotHandlerMgr->Annot_OnMouseMove(this, pFXAnnot, nFlag, point);
		return TRUE;
	}
	else
	{
		if(m_bOnWidget)
		{	
			m_bOnWidget = FALSE;
			m_bExitWidget = TRUE;
			m_bEnterWidget = FALSE;
			if(m_CaptureWidget)
			{
				pAnnotHandlerMgr->Annot_OnMouseExit(this, m_CaptureWidget, nFlag);
				m_CaptureWidget = NULL;
			}
		}
		return FALSE;
	}
	
	return FALSE;;
}

FX_BOOL CPDFSDK_PageView::OnMouseWheel(double deltaX, double deltaY,const CPDF_Point& point, int nFlag)
{
	if(CPDFSDK_Annot* pAnnot = GetFXWidgetAtPoint(point.x, point.y))
	{
		CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
		CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
		ASSERT(pAnnotHandlerMgr);
		return pAnnotHandlerMgr->Annot_OnMouseWheel(this, pAnnot, nFlag, (int)deltaY, point);
	}
	return FALSE;

}

FX_BOOL CPDFSDK_PageView::OnChar(int nChar, FX_UINT nFlag)
{
	if(CPDFSDK_Annot* pAnnot = GetFocusAnnot())
	{
		CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
		CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
		ASSERT(pAnnotHandlerMgr);
		return pAnnotHandlerMgr->Annot_OnChar(pAnnot, nChar, nFlag);
	}

	return FALSE;
}

FX_BOOL CPDFSDK_PageView::OnKeyDown(int nKeyCode, int nFlag)
{
	if(CPDFSDK_Annot* pAnnot = GetFocusAnnot())
	{
		CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
		CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
		ASSERT(pAnnotHandlerMgr);
		return pAnnotHandlerMgr->Annot_OnKeyDown(pAnnot, nKeyCode, nFlag);
	}
	return FALSE;
}

FX_BOOL CPDFSDK_PageView::OnKeyUp(int nKeyCode, int nFlag)
{
// 	if(CPDFSDK_Annot* pAnnot = GetFocusAnnot())
// 	{
// 		CFFL_IFormFiller* pIFormFiller = g_pFormFillApp->GetIFormFiller();
// 		return pIFormFiller->OnKeyUp(pAnnot, nKeyCode, nFlag);
// 	}
	return FALSE;
}

extern void CheckUnSupportAnnot(CPDF_Document * pDoc, CPDF_Annot* pPDFAnnot);

void CPDFSDK_PageView::LoadFXAnnots()
{
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();

	FX_BOOL enableAPUpdate = CPDF_InterForm::UpdatingAPEnabled();
	//Disable the default AP construction.
	CPDF_InterForm::EnableUpdateAP(FALSE);
	m_pAnnotList = new CPDF_AnnotList(m_page);
	CPDF_InterForm::EnableUpdateAP(enableAPUpdate);
	int nCount = m_pAnnotList->Count();
        SetLock(TRUE);
	for(int i=0; i<nCount; i++)
	{
		CPDF_Annot* pPDFAnnot = m_pAnnotList->GetAt(i);
		CPDF_Document * pDoc = this->GetPDFDocument();
		
		CheckUnSupportAnnot(pDoc, pPDFAnnot);

		CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();
		ASSERT(pAnnotHandlerMgr != NULL);

		if(pAnnotHandlerMgr)
		{
			CPDFSDK_Annot* pAnnot = pAnnotHandlerMgr->NewAnnot(pPDFAnnot, this);
			if(!pAnnot)
				continue;
			m_fxAnnotArray.Add(pAnnot);

			pAnnotHandlerMgr->Annot_OnLoad(pAnnot);
		}

	}
        SetLock(FALSE);
}

void	CPDFSDK_PageView::UpdateRects(CFX_RectArray& rects)
{
	for(int i=0; i<rects.GetSize(); i++)
	{
		CPDF_Rect rc = rects.GetAt(i);
		CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
		pEnv->FFI_Invalidate(m_page, rc.left, rc.top, rc.right, rc.bottom);
	}
}

void CPDFSDK_PageView::UpdateView(CPDFSDK_Annot* pAnnot)
{
	CPDF_Rect rcWindow;

 	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();	
// 	CPDFSDK_AnnotHandlerMgr* pAnnotHandler = pEnv->GetAnnotHandlerMgr();
	
	rcWindow = pAnnot->GetRect();//pAnnotHandler->Annot_OnGetViewBBox(this,pAnnot);	
	pEnv->FFI_Invalidate(m_page, rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom);

}

int CPDFSDK_PageView::GetPageIndex()
{
	if(m_page)
	{
		CPDF_Dictionary* pDic = m_page->m_pFormDict;
		CPDF_Document* pDoc = m_pSDKDoc->GetDocument();
		if(pDoc && pDic)
		{
			return pDoc->GetPageIndex(pDic->GetObjNum());
		}
	}
	return -1;
}

FX_BOOL	CPDFSDK_PageView::IsValidAnnot(FX_LPVOID p)
{
	if (p == NULL) return FALSE;
	int iCount = m_pAnnotList->Count();
	for (int i = 0; i < iCount; i++)
	{
		if (m_pAnnotList->GetAt(i) == p)
			return TRUE;
	}
	return FALSE;
}


CPDFSDK_Annot* CPDFSDK_PageView::GetFocusAnnot()
{
	CPDFSDK_Annot* pFocusAnnot = m_pSDKDoc->GetFocusAnnot();
	if(!pFocusAnnot)
		return NULL;
	
	for(int i=0; i<m_fxAnnotArray.GetSize(); i++)
	{
		CPDFSDK_Annot* pAnnot = (CPDFSDK_Annot*)m_fxAnnotArray.GetAt(i);
		if(pAnnot == pFocusAnnot)
			return pAnnot;
	}
	return NULL;
}

