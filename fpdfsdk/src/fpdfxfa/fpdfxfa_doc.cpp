// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fsdk_define.h"
#include "../../include/fpdfxfa/fpdfxfa_doc.h"
#include "../../include/fpdfformfill.h"
#include "../../include/fsdk_mgr.h"
#include "../../include/fpdfxfa/fpdfxfa_app.h"
#include "../../include/fpdfxfa/fpdfxfa_util.h"
#include "../../include/fpdfxfa/fpdfxfa_page.h"
#include "../../include/javascript/IJavaScript.h"



#define IDS_XFA_StringMonth_April "April"
#define IDS_XFA_StringMonth_May "May"
#define IDS_XFA_StringMonth_June "June"
#define IDS_XFA_StringMonth_July "July"
#define IDS_XFA_StringMonth_Aug "August"
#define IDS_XFA_StringMonth_Sept "September"
#define IDS_XFA_StringMonth_Oct "October"
#define IDS_XFA_StringMonth_Nov "November"
#define IDS_XFA_StringMonth_Dec "December"
#define IDS_XFA_String_Today    "Today"
#define IDS_XFA_ValidateLimit   "Message limit exceeded. Remaining %d validation errors not reported."
#define IDS_XFA_Validate_Input  "At least one required field was empty. Please fill in the required fields\r\n(highlighted) before continuing."

// submit
#define FXFA_CONFIG				0x00000001
#define FXFA_TEMPLATE			0x00000010
#define FXFA_LOCALESET			0x00000100
#define FXFA_DATASETS			0x00001000
#define FXFA_XMPMETA			0x00010000
#define FXFA_XFDF				0x00100000
#define FXFA_FORM				0x01000000
#define FXFA_PDF				0x10000000

#ifndef _WIN32
extern void SetLastError(int err);

extern int GetLastError();
#endif

CPDFXFA_Document::CPDFXFA_Document(CPDF_Document* pPDFDoc, CPDFXFA_App* pProvider) :
	m_pPDFDoc(pPDFDoc),
	m_pApp(pProvider),
	m_pXFADoc(NULL),
	m_pXFADocView(NULL),
	m_iDocType(DOCTYPE_PDF),
	m_pJSContext(NULL),
	m_pSDKDoc(NULL)
{
	m_XFAPageList.RemoveAll();
}

CPDFXFA_Document::~CPDFXFA_Document()
{
	if (m_pPDFDoc)
	{
		CPDF_Parser* pParser = (CPDF_Parser*)m_pPDFDoc->GetParser();
		if (pParser == NULL) 
		{
		 	delete m_pPDFDoc;
	 	}else 
		{
	 		delete pParser;
		}
		m_pPDFDoc = NULL;
	}
	if (m_pXFADoc)
	{
		IXFA_App* pApp = m_pApp->GetXFAApp();
		if (pApp) 
		{
			IXFA_DocHandler* pDocHandler = pApp->GetDocHandler();
			if (pDocHandler)
			{
				pDocHandler->CloseDoc(m_pXFADoc);
				pDocHandler->ReleaseDoc(m_pXFADoc);
				m_pXFADoc = NULL;
			}	
		}
	}

	if (m_pJSContext)
	{
		if (m_pSDKDoc && m_pSDKDoc->GetEnv())
		{
			m_pSDKDoc->GetEnv()->GetJSRuntime()->ReleaseContext(m_pJSContext);
			m_pJSContext = NULL;
		}
	}

	
	if (m_pSDKDoc)
		delete m_pSDKDoc;
	m_pSDKDoc = NULL;
}

FX_BOOL CPDFXFA_Document::LoadXFADoc()
{
	if (!m_pPDFDoc) 
		return FALSE;

	m_XFAPageList.RemoveAll();

	int iDocType = DOCTYPE_PDF;
	FX_BOOL hasXFAField = FPDF_HasXFAField(m_pPDFDoc, iDocType);

	if (hasXFAField)
	{
		IXFA_App* pApp = m_pApp->GetXFAApp();
		if (pApp)
		{
			m_pXFADoc = pApp->CreateDoc(this, m_pPDFDoc);
			if (!m_pXFADoc)
			{
				SetLastError(FPDF_ERR_XFALOAD);
				return FALSE;
			}

			IXFA_DocHandler* pDocHandler = pApp->GetDocHandler();
			if (pDocHandler)
			{
				int iStatus = pDocHandler->StartLoad(m_pXFADoc);
				iStatus = pDocHandler->DoLoad(m_pXFADoc, NULL);
				if (iStatus != 100) 
				{
					pDocHandler->CloseDoc(m_pXFADoc);
					pDocHandler->ReleaseDoc(m_pXFADoc);
					m_pXFADoc = NULL;

					SetLastError(FPDF_ERR_XFALOAD);
					return FALSE;
				}
				pDocHandler->StopLoad(m_pXFADoc);
				pDocHandler->SetJSERuntime(m_pXFADoc, m_pApp->GetJSERuntime());

				if (pDocHandler->GetDocType(m_pXFADoc) == XFA_DOCTYPE_Dynamic)
					m_iDocType = DOCTYPE_DYNIMIC_XFA;
				else
					m_iDocType = DOCTYPE_STATIC_XFA;

				m_pXFADocView = pDocHandler->CreateDocView(m_pXFADoc, XFA_DOCVIEW_View);
				FXSYS_assert(m_pXFADocView);

				if (m_pXFADocView->StartLayout() < 0) 
				{
					pDocHandler->CloseDoc(m_pXFADoc);
					pDocHandler->ReleaseDoc(m_pXFADoc);
					m_pXFADoc = NULL;

					SetLastError(FPDF_ERR_XFALAYOUT);
					return FALSE;
				}
				else
				{
					m_pXFADocView->DoLayout(NULL);
					m_pXFADocView->StopLayout();

					return TRUE;
				}				
			}

			return FALSE;
		}

		return FALSE;
	}
	
	return TRUE;
}

int CPDFXFA_Document::GetPageCount()
{
	if (!m_pPDFDoc && !m_pXFADoc)
		return 0;

	switch (m_iDocType)
	{
	case DOCTYPE_PDF:
	case DOCTYPE_STATIC_XFA:
		if (m_pPDFDoc)
			return m_pPDFDoc->GetPageCount();
	case DOCTYPE_DYNIMIC_XFA:
		if (m_pXFADoc)
			return m_pXFADocView->CountPageViews();
	default:
		return 0;
	}

	return 0;
}

CPDFXFA_Page* CPDFXFA_Document::GetPage(int page_index)
{
	if (!m_pPDFDoc && !m_pXFADoc)
		return NULL;

	CPDFXFA_Page* pPage = NULL;
	if (m_XFAPageList.GetSize())
	{
		pPage = m_XFAPageList.GetAt(page_index);
		if (pPage)
			pPage->AddRef();
	}
	else 
	{
		m_XFAPageList.SetSize(GetPageCount());
	}

	if (!pPage) 
	{
		pPage = FX_NEW CPDFXFA_Page(this, page_index);
		FX_BOOL bRet = pPage->LoadPage();
		if (!bRet) {
			delete pPage;
			return NULL;
		}

		m_XFAPageList.SetAt(page_index, pPage);
	}

	return pPage;
}

CPDFXFA_Page* CPDFXFA_Document::GetPage(IXFA_PageView* pPage)
{
	if (!pPage)
		return NULL;

	if (!m_pXFADoc)
		return NULL;

	if (m_iDocType != DOCTYPE_DYNIMIC_XFA)
		return NULL;

	int nSize = m_XFAPageList.GetSize();
	for (int i=0; i<nSize; i++)
	{
		CPDFXFA_Page* pTempPage = m_XFAPageList.GetAt(i);
		if (!pTempPage) continue;
		if (pTempPage->GetXFAPageView() && pTempPage->GetXFAPageView() == pPage)
			return pTempPage;
	}

	return NULL;
}

void CPDFXFA_Document::RemovePage(CPDFXFA_Page* page)
{
	m_XFAPageList.SetAt(page->GetPageIndex(), NULL);
}

CPDFSDK_Document* CPDFXFA_Document::GetSDKDocument(CPDFDoc_Environment* pFormFillEnv)
{
	if (!pFormFillEnv)
		return m_pSDKDoc;

	if (m_pSDKDoc)
		return m_pSDKDoc;

	m_pSDKDoc = new CPDFSDK_Document(this, pFormFillEnv);
	if (!m_pSDKDoc)
		return NULL;

	return m_pSDKDoc;
}

void CPDFXFA_Document::ReleaseSDKDoc()
{
	if (m_pSDKDoc)
		delete m_pSDKDoc;

	m_pSDKDoc = NULL;
}

void CPDFXFA_Document::FXRect2PDFRect(const CFX_RectF& fxRectF, CPDF_Rect& pdfRect)
{
	pdfRect.left = fxRectF.left;
	pdfRect.top = fxRectF.bottom();
	pdfRect.right = fxRectF.right();
	pdfRect.bottom = fxRectF.top;
}

//////////////////////////////////////////////////////////////////////////
void CPDFXFA_Document::SetChangeMark(XFA_HDOC hDoc)
{
	if (hDoc == m_pXFADoc && m_pSDKDoc)
	{
		m_pSDKDoc->SetChangeMark();
	}
}

FX_BOOL CPDFXFA_Document::GetChangeMark(XFA_HDOC hDoc)
{
	if (hDoc == m_pXFADoc && m_pSDKDoc)
		return m_pSDKDoc->GetChangeMark();
	return FALSE;
}

void CPDFXFA_Document::InvalidateRect(IXFA_PageView* pPageView, const CFX_RectF& rt, FX_DWORD dwFlags /* = 0 */)
{
	if (!m_pXFADoc || !m_pSDKDoc)
		return;

	if (m_iDocType != DOCTYPE_DYNIMIC_XFA)
		return;

	CPDF_Rect rcPage;
	FXRect2PDFRect(rt, rcPage);

	CPDFXFA_Page* pPage = GetPage(pPageView);

	if (pPage == NULL)
		return;

	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (!pEnv)
		return;

	pEnv->FFI_Invalidate((FPDF_PAGE)pPage, rcPage.left, rcPage.bottom, rcPage.right, rcPage.top);
}

void CPDFXFA_Document::InvalidateRect(XFA_HWIDGET hWidget, FX_DWORD dwFlags /* = 0 */)
{
	if (!hWidget)
		return;

	if (!m_pXFADoc || !m_pSDKDoc || !m_pXFADocView)
		return;
	
	if (m_iDocType != DOCTYPE_DYNIMIC_XFA)
		return;

	IXFA_WidgetHandler* pWidgetHandler = m_pXFADocView->GetWidgetHandler();
	if (!pWidgetHandler)
		return;

	IXFA_PageView* pPageView = pWidgetHandler->GetPageView(hWidget);
	if (!pPageView)
		return;

	CFX_RectF rect;
	pWidgetHandler->GetRect(hWidget, rect);
	InvalidateRect(pPageView, rect, dwFlags);
}

void CPDFXFA_Document::DisplayCaret(XFA_HWIDGET hWidget, FX_BOOL bVisible, const CFX_RectF* pRtAnchor)
{
	if (!hWidget || pRtAnchor == NULL) 
		return;

	if (!m_pXFADoc || !m_pSDKDoc || !m_pXFADocView)
		return;

	if (m_iDocType != DOCTYPE_DYNIMIC_XFA)
		return;

	IXFA_WidgetHandler* pWidgetHandler = m_pXFADocView->GetWidgetHandler();
	if (!pWidgetHandler)
		return;

	IXFA_PageView* pPageView = pWidgetHandler->GetPageView(hWidget);
	if (!pPageView)
		return;

	CPDFXFA_Page* pPage = GetPage(pPageView);

	if (pPage == NULL)
		return;

	CPDF_Rect rcCaret;
	FXRect2PDFRect(*pRtAnchor, rcCaret);

	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (!pEnv)
		return;

	pEnv->FFI_DisplayCaret((FPDF_PAGE)pPage, bVisible, rcCaret.left, rcCaret.top, rcCaret.right, rcCaret.bottom);

}

FX_BOOL CPDFXFA_Document::GetPopupPos(XFA_HWIDGET hWidget, FX_FLOAT fMinPopup, FX_FLOAT fMaxPopup, const CFX_RectF &rtAnchor, CFX_RectF &rtPopup)
{
	if (NULL == hWidget)
	{
		return FALSE;
	}
	IXFA_PageView* pXFAPageView = m_pXFADocView->GetWidgetHandler()->GetPageView(hWidget);
	if (NULL == pXFAPageView)
	{
		return FALSE;
	}
	CPDFXFA_Page* pPage = GetPage(pXFAPageView);
	if (pPage == NULL)
		return FALSE;

	CXFA_WidgetAcc* pWidgetAcc = m_pXFADocView->GetWidgetHandler()->GetDataAcc(hWidget);

	int nRotate = pWidgetAcc->GetRotate();
	
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return FALSE;
	FS_RECTF pageViewRect;
	pEnv->FFI_GetPageViewRect(pPage, pageViewRect);

	CPDF_Rect rcAnchor;

	rcAnchor.left = rtAnchor.left;
	rcAnchor.top = rtAnchor.bottom();
	rcAnchor.right = rtAnchor.right();
	rcAnchor.bottom = rtAnchor.top;
	
	int t1,t2,t;
	FX_DWORD dwPos; FX_FLOAT fPoupHeight;
	switch(nRotate)
	{
		
	case 90:
		{
			t1 = (int)(pageViewRect.right - rcAnchor.right);
 			t2 = (int)(rcAnchor.left  - pageViewRect.left);
			if (rcAnchor.bottom < pageViewRect.bottom) 
			{
				rtPopup.left += rcAnchor.bottom - pageViewRect.bottom;
			}
		 
			break;
		}
		
	case 180:
		{
			t2 = (int)(pageViewRect.top - rcAnchor.top);
			t1 = (int)(rcAnchor.bottom - pageViewRect.bottom); 
			if (rcAnchor.left < pageViewRect.left) 
			{
				rtPopup.left += rcAnchor.left - pageViewRect.left; 
			}
			break;
		}
	case 270:
		{
			t1 = (int)(rcAnchor.left  - pageViewRect.left);
			t2 = (int)(pageViewRect.right - rcAnchor.right);
 		
			if (rcAnchor.top > pageViewRect.top) 
			{
				rtPopup.left -= rcAnchor.top - pageViewRect.top; 
			}
			break;
		}
	case 0:
	default:
		{
			t1 = (int)(pageViewRect.top - rcAnchor.top);
			t2 = (int)(rcAnchor.bottom - pageViewRect.bottom); 
			if (rcAnchor.right > pageViewRect.right) 
			{
				rtPopup.left -= rcAnchor.right - pageViewRect.right; 
			}
			break;
		}
		
	}

	if (t1 <= 0 && t2 <= 0)
	{
		return FALSE;
	}
	if (t1 <= 0)
	{
		t = t2;
		dwPos = 1;
	}
	else if(t2 <= 0)
	{
		t = t1;
		dwPos = 0;
	}
	else if (t1 > t2 )
	{
		t = t1;
		dwPos = 0;
	}
	else 
	{
		t = t2;
		dwPos = 1;
	}
	if (t<fMinPopup)
	{
		fPoupHeight = fMinPopup;
	}
	else if (t > fMaxPopup)
	{
		fPoupHeight = fMaxPopup;
	}
	else
	{
		fPoupHeight = (FX_FLOAT)t;
	}

	switch(nRotate)
	{
		
	case 0:
	case 180:
		{
			if (dwPos == 0)
			{
				rtPopup.top = rtAnchor.height;
				rtPopup.height = fPoupHeight;
			}
			else
			{
				rtPopup.top = - fPoupHeight;
				rtPopup.height = fPoupHeight;
			}
			break;
		}
	case  90:
	case  270:
		{
			if (dwPos == 0)
			{
				rtPopup.top = rtAnchor.width;
				rtPopup.height = fPoupHeight;
			}
			else
			{
				rtPopup.top = - fPoupHeight;
				rtPopup.height = fPoupHeight;
			}
			break;
		}
	default:
			break;
	}
	
	return TRUE;
}

FX_BOOL	CPDFXFA_Document::PopupMenu(XFA_HWIDGET hWidget, CFX_PointF ptPopup, const CFX_RectF* pRectExclude)
{
	if (NULL == hWidget)
	{
		return FALSE;
	}
	IXFA_PageView* pXFAPageView = m_pXFADocView->GetWidgetHandler()->GetPageView(hWidget);
	if (pXFAPageView == NULL)
		return FALSE;
	CPDFXFA_Page* pPage = GetPage(pXFAPageView);

	if (pPage == NULL)
		return FALSE;

	int menuFlag = 0;

	IXFA_MenuHandler* pXFAMenuHander = m_pApp->GetXFAApp()->GetMenuHandler();
	if (pXFAMenuHander->CanUndo(hWidget))
		menuFlag |= FXFA_MEMU_UNDO;
	if (pXFAMenuHander->CanRedo(hWidget))
		menuFlag |= FXFA_MEMU_REDO;
	if (pXFAMenuHander->CanPaste(hWidget))
		menuFlag |= FXFA_MEMU_PASTE;
	if (pXFAMenuHander->CanCopy(hWidget))
		menuFlag |= FXFA_MEMU_COPY;
	if (pXFAMenuHander->CanCut(hWidget))
		menuFlag |= FXFA_MEMU_CUT;
	if (pXFAMenuHander->CanSelectAll(hWidget))
		menuFlag |= FXFA_MEMU_SELECTALL;


	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return FALSE;

	return pEnv->FFI_PopupMenu(pPage, hWidget, menuFlag, ptPopup, NULL);
}

void CPDFXFA_Document::PageViewEvent(IXFA_PageView* pPageView, FX_DWORD dwFlags)
{
	if (m_iDocType != DOCTYPE_DYNIMIC_XFA)
		return;

	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return;

	CPDFXFA_Page* pPage = GetPage(pPageView);
	if (pPage == NULL)
		return;

	if (dwFlags == FXFA_PAGEVIEWEVENT_POSTADDED)
	{
		//pEnv->FFI_PageEvent(pPage, FXFA_PAGEVIEWEVENT_POSTADDED);
	}
	else if (dwFlags == FXFA_PAGEVIEWEVENT_POSTREMOVED)
	{
		//pEnv->FFI_PageEvent(pPage, FXFA_PAGEVIEWEVENT_POSTREMOVED);
		//RemovePage(pPage);
		//delete pPage;
	}
}

void CPDFXFA_Document::WidgetEvent(XFA_HWIDGET hWidget, CXFA_WidgetAcc* pWidgetData, FX_DWORD dwEvent, FX_LPVOID pParam, FX_LPVOID pAdditional)
{
	if (m_iDocType != DOCTYPE_DYNIMIC_XFA || NULL == hWidget)
		return;
	
	int pageViewCount = m_pSDKDoc->GetPageViewCount();

	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return;

	if (NULL == hWidget) return;

	IXFA_PageView* pPageView = m_pXFADocView->GetWidgetHandler()->GetPageView(hWidget);

	if (pPageView == NULL)
		return;
	CPDFXFA_Page* pXFAPage = GetPage(pPageView);
	if (pXFAPage == NULL)
		return;

	CPDFSDK_PageView* pSdkPageView = m_pSDKDoc->GetPageView(pXFAPage);

	CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr = pEnv->GetAnnotHandlerMgr();


	if (dwEvent == XFA_WIDGETEVENT_PostAdded)
	{
// 			CPDFSDK_Annot* pAnnot = pAnnotHandlerMgr->NewAnnot(hWidget, pSdkPageView);
// 			pAnnotHandlerMgr->Annot_OnLoad(pAnnot);

			//pEnv->FFI_WidgetEvent(hWidget, XFA_WIDGETEVENT_PostAdded);
// 		IXFA_PageView* pOldPageView = (IXFA_PageView*)pAdditional;
// 		if (pOldPageView)
// 		{
// 			CPDFXFA_Page* pDestPage = m_pSDKDoc->GetPageView((IXFA_PageView*)pOldPageView);
// 			ASSERT(pDestPage);
// 			CPDFSDK_Annot* pAnnot = pDestPage->GetAnnotByXFAWidget(hWidget);
// 			if (pAnnot)
// 			{
// 				if (m_pSDKDoc->GetFocusAnnot() == pAnnot)
// 				{
// 					m_pSDKDoc->SetFocusAnnot(NULL);
// 				}
// 				pDestPage->DeleteAnnot(pAnnot);
// 			}
// 		}
		pSdkPageView->AddAnnot(hWidget);

	}
	else if (dwEvent == XFA_WIDGETEVENT_PreRemoved)
	{	
		CPDFSDK_Annot* pAnnot = pSdkPageView->GetAnnotByXFAWidget(hWidget);
		if (pAnnot) {
			pSdkPageView->DeleteAnnot(pAnnot);
			//pEnv->FFI_WidgetEvent(hWidget, XFA_WIDGETEVENT_PreRemoved);
		}
	}
}

FX_INT32 CPDFXFA_Document::CountPages(XFA_HDOC hDoc)
{
	if (hDoc == m_pXFADoc && m_pSDKDoc)
	{
		return GetPageCount();
	}
	return 0;
}
FX_INT32 CPDFXFA_Document::GetCurrentPage(XFA_HDOC hDoc)
{
	if (hDoc != m_pXFADoc || !m_pSDKDoc)
		return -1;
	if (m_iDocType != DOCTYPE_DYNIMIC_XFA)
		return -1;

	
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return -1;

	return pEnv->FFI_GetCurrentPageIndex(this);
}
void CPDFXFA_Document::SetCurrentPage(XFA_HDOC hDoc, FX_INT32 iCurPage)
{
	if (hDoc != m_pXFADoc || !m_pSDKDoc)
		return;
	if (m_iDocType != DOCTYPE_DYNIMIC_XFA)
		return;
		
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return;

	pEnv->FFI_SetCurrentPage(this, iCurPage);
}
FX_BOOL	CPDFXFA_Document::IsCalculationsEnabled(XFA_HDOC hDoc)
{
	if (hDoc != m_pXFADoc || !m_pSDKDoc)
		return FALSE;
	if (m_pSDKDoc->GetInterForm())
		return m_pSDKDoc->GetInterForm()->IsXfaCalculateEnabled();

	return FALSE;

}
void CPDFXFA_Document::SetCalculationsEnabled(XFA_HDOC hDoc, FX_BOOL bEnabled)
{
	if (hDoc != m_pXFADoc || !m_pSDKDoc)
		return;
	if (m_pSDKDoc->GetInterForm())
		m_pSDKDoc->GetInterForm()->XfaEnableCalculate(bEnabled);
}

void CPDFXFA_Document::GetTitle(XFA_HDOC hDoc, CFX_WideString &wsTitle)
{
	if (hDoc != m_pXFADoc)
		return;
	if (m_pPDFDoc == NULL)
		return;
	CPDF_Dictionary* pInfoDict = m_pPDFDoc->GetInfo();

	if (pInfoDict == NULL)
		return;
	
	CFX_ByteString csTitle = pInfoDict->GetString("Title");
    wsTitle = wsTitle.FromLocal(csTitle.GetBuffer(csTitle.GetLength()));
	csTitle.ReleaseBuffer(csTitle.GetLength());
}
void CPDFXFA_Document::SetTitle(XFA_HDOC hDoc, FX_WSTR wsTitle)
{
	if (hDoc != m_pXFADoc)
		return;
	if (m_pPDFDoc == NULL)
		return;
	CPDF_Dictionary* pInfoDict = m_pPDFDoc->GetInfo();

	if (pInfoDict == NULL)
		return;
	pInfoDict->SetAt("Title", FX_NEW CPDF_String(wsTitle));
}
void CPDFXFA_Document::ExportData(XFA_HDOC hDoc, FX_WSTR wsFilePath, FX_BOOL bXDP)
{
	if (hDoc != m_pXFADoc)
		return;
	if (m_iDocType != DOCTYPE_DYNIMIC_XFA && m_iDocType != DOCTYPE_STATIC_XFA)
		return;
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return;
	int fileType = bXDP?FXFA_SAVEAS_XDP:FXFA_SAVEAS_XML;
	CFX_ByteString bs = CFX_WideString(wsFilePath).UTF16LE_Encode();

	if (wsFilePath.IsEmpty()) {
		if (!pEnv->GetFormFillInfo() || pEnv->GetFormFillInfo()->m_pJsPlatform == NULL)
			return;
		CFX_WideString filepath = pEnv->JS_fieldBrowse();
		bs = filepath.UTF16LE_Encode();
	}
	int len = bs.GetLength()/sizeof(unsigned short);
	FPDF_FILEHANDLER* pFileHandler = pEnv->FFI_OpenFile(bXDP?FXFA_SAVEAS_XDP:FXFA_SAVEAS_XML, (FPDF_WIDESTRING)bs.GetBuffer(len*sizeof(unsigned short)), "wb");
	bs.ReleaseBuffer(len*sizeof(unsigned short));

	if (pFileHandler == NULL)
		return;

	CFPDF_FileStream fileWrite(pFileHandler);

	IXFA_DocHandler *pXFADocHander = m_pApp->GetXFAApp()->GetDocHandler();
	CFX_ByteString content;
	if (fileType == FXFA_SAVEAS_XML)
	{
		content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
		fileWrite.WriteBlock((FX_LPCSTR)content, fileWrite.GetSize(), content.GetLength());
		CFX_WideStringC data(L"data");
		if( pXFADocHander->SavePackage(m_pXFADocView->GetDoc(),data, &fileWrite))
		{
			NULL;
		}
	}
	/*else if (fileType == FXFA_FILE_STATIC_XDP)
	{
		content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
		fileWrite.WriteBlock((FX_LPCSTR)content, fileWrite.GetSize(), content.GetLength());
		CFX_WideStringC data(L"data");
		if( pXFADocHander->SavePackage(m_pXFADocView->GetDoc(), data, &fileWrite))
		{
			NULL;
		}
		CFX_WideString wPath = pEnv->FFI_GetFilePath(pFileHandler);
// 		CFX_WideString wPath;
// 		wPath.FromUTF16LE(filePath);
		CFX_ByteString bPath = wPath.UTF8Encode();
		CFX_ByteString szFormat = "\n<pdf href=\"%s\" xmlns=\"http://ns.adobe.com/xdp/pdf/\"/>";
		content.Format(szFormat,(char*)(FX_LPCSTR)bPath);
		fileWrite.WriteBlock((FX_LPCSTR)content,fileWrite.GetSize(), content.GetLength());
	}
	*/
	else if (fileType == FXFA_SAVEAS_XDP)
	{	
		if (m_pPDFDoc == NULL)
			return;
		CPDF_Dictionary* pRoot = m_pPDFDoc->GetRoot();
		if (pRoot == NULL)
			return;
		CPDF_Dictionary* pAcroForm = pRoot->GetDict("AcroForm");
		if (NULL == pAcroForm)
			return;
		CPDF_Object* pXFA = pAcroForm->GetElement("XFA");
		if (pXFA == NULL)
			return;
		if (pXFA->GetType() != PDFOBJ_ARRAY)
			return;
		CPDF_Array* pArray = pXFA->GetArray();
		if (NULL == pArray)
			return;
		int size = pArray->GetCount();
		int iFormIndex = -1;
		int iDataSetsIndex = -1;
		for (int i=1; i<size;i+=2)
		{
			CPDF_Object* pPDFObj = pArray->GetElement(i);
			CPDF_Object* pPrePDFObj = pArray->GetElement(i-1);
			if(pPrePDFObj->GetType() != PDFOBJ_STRING)
				continue;
			if (pPDFObj->GetType() != PDFOBJ_REFERENCE)
				continue;
			CPDF_Object* pDirectObj = pPDFObj->GetDirect();
			if(pDirectObj->GetType() != PDFOBJ_STREAM)
				continue;
			if (pPrePDFObj->GetString()=="form")
			{
				CFX_WideStringC form(L"form");
				pXFADocHander->SavePackage(m_pXFADocView->GetDoc(), form, &fileWrite);
			}
			else if (pPrePDFObj->GetString()=="datasets")
			{
				CFX_WideStringC datasets(L"datasets");
				pXFADocHander->SavePackage(m_pXFADocView->GetDoc(), datasets, &fileWrite);
			}
			else 
			{	
				if (i == size-1)
				{
					//CFX_WideString wPath = pEnv->FFI_GetFilePath(pFileHandler);
					CFX_WideString wPath = CFX_WideString::FromUTF16LE((unsigned short*)(FX_LPCSTR)bs, bs.GetLength()/sizeof(unsigned short));
					CFX_ByteString bPath = wPath.UTF8Encode();
					CFX_ByteString szFormat = "\n<pdf href=\"%s\" xmlns=\"http://ns.adobe.com/xdp/pdf/\"/>";
					content.Format(szFormat,(char*)(FX_LPCSTR)bPath);
					fileWrite.WriteBlock((FX_LPCSTR)content,fileWrite.GetSize(), content.GetLength());
				}

				CPDF_Stream* pStream = (CPDF_Stream*)pDirectObj;
				CPDF_StreamAcc* pAcc = FX_NEW CPDF_StreamAcc;
				pAcc->LoadAllData(pStream);
				fileWrite.WriteBlock(pAcc->GetData(), fileWrite.GetSize(), pAcc->GetSize());
				delete pAcc;
			}
		}
	}
	FX_BOOL bError= fileWrite.Flush();
}
void CPDFXFA_Document::ImportData(XFA_HDOC hDoc, FX_WSTR wsFilePath)
{
	//TODO...
}

void CPDFXFA_Document::GotoURL(XFA_HDOC hDoc, FX_WSTR bsURL, FX_BOOL bAppend)
{
	if (hDoc != m_pXFADoc)
		return;

	if (m_iDocType != DOCTYPE_DYNIMIC_XFA)
		return;
		
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return;
	
	CFX_WideStringC str(bsURL.GetPtr());
	
	pEnv->FFI_GotoURL(this, str, bAppend);

}

FX_BOOL	CPDFXFA_Document::IsValidationsEnabled(XFA_HDOC hDoc)
{
	if (hDoc != m_pXFADoc || !m_pSDKDoc)
		return FALSE;
	if (m_pSDKDoc->GetInterForm())
		return m_pSDKDoc->GetInterForm()->IsXfaValidationsEnabled();

	return TRUE;
}
void CPDFXFA_Document::SetValidationsEnabled(XFA_HDOC hDoc, FX_BOOL bEnabled)
{
	if (hDoc != m_pXFADoc || !m_pSDKDoc)
		return;
	if (m_pSDKDoc->GetInterForm())
		m_pSDKDoc->GetInterForm()->XfaSetValidationsEnabled(bEnabled);
}
void  CPDFXFA_Document::SetFocusWidget(XFA_HDOC hDoc, XFA_HWIDGET hWidget)
{
	if (hDoc != m_pXFADoc)
		return;

	if (NULL == hWidget) {
		m_pSDKDoc->SetFocusAnnot(NULL);
		return;
	}

	int pageViewCount = m_pSDKDoc->GetPageViewCount();
	for (int i = 0; i < pageViewCount; i++)
	{
		CPDFSDK_PageView* pPageView = m_pSDKDoc->GetPageView(i);
		if (pPageView == NULL)
			continue;
		CPDFSDK_Annot* pAnnot = pPageView->GetAnnotByXFAWidget(hWidget);
		if (pAnnot) {
			m_pSDKDoc->SetFocusAnnot(pAnnot);
			break;
		}
	}
}
void CPDFXFA_Document::Print(XFA_HDOC hDoc, FX_INT32 nStartPage, FX_INT32 nEndPage, FX_DWORD dwOptions)
{
	if (hDoc != m_pXFADoc)
		return;
		
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return;

	if (!pEnv->GetFormFillInfo() || pEnv->GetFormFillInfo()->m_pJsPlatform == NULL)
		return;
	if (pEnv->GetFormFillInfo()->m_pJsPlatform->Doc_print == NULL)
		return;
	pEnv->GetFormFillInfo()->m_pJsPlatform->Doc_print(pEnv->GetFormFillInfo()->m_pJsPlatform,dwOptions&XFA_PRINTOPT_ShowDialog,
		nStartPage,nEndPage,dwOptions&XFA_PRINTOPT_CanCancel,dwOptions&XFA_PRINTOPT_ShrinkPage,dwOptions&XFA_PRINTOPT_AsImage,
		dwOptions&XFA_PRINTOPT_ReverseOrder,dwOptions&XFA_PRINTOPT_PrintAnnot);
}

void CPDFXFA_Document::GetURL(XFA_HDOC hDoc, CFX_WideString &wsDocURL)
{
	if (hDoc != m_pXFADoc)
		return;

	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return;

	pEnv->FFI_GetURL(this, wsDocURL);
}

FX_ARGB	CPDFXFA_Document::GetHighlightColor(XFA_HDOC hDoc)
{
	if (hDoc != m_pXFADoc)
		return 0;
	if (m_pSDKDoc)
	{
		if(CPDFSDK_InterForm* pInterForm = m_pSDKDoc->GetInterForm())
		{
			FX_COLORREF color = pInterForm->GetHighlightColor(FPDF_FORMFIELD_XFA);
			FX_BYTE alpha = pInterForm->GetHighlightAlpha();
			FX_ARGB argb = ArgbEncode((int)alpha, color);
			return argb;
		}	
	}
	return 0;
}

void CPDFXFA_Document::AddDoRecord(XFA_HWIDGET hWidget)
{
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return;
	return;
	//pEnv->FFI_AddDoRecord(this, hWidget);
}

FX_BOOL CPDFXFA_Document::_NotifySubmit(FX_BOOL bPrevOrPost)
{
	if (bPrevOrPost)
		return _OnBeforeNotifySumbit();
	else
		_OnAfterNotifySumbit();
	return TRUE;
}

FX_BOOL CPDFXFA_Document::_OnBeforeNotifySumbit()
{
	if (m_iDocType != DOCTYPE_DYNIMIC_XFA && m_iDocType != DOCTYPE_STATIC_XFA)
		return TRUE;
	if (m_pXFADocView == NULL)
		return TRUE;
	IXFA_WidgetHandler* pWidgetHandler = m_pXFADocView->GetWidgetHandler();
	if (pWidgetHandler == NULL)
		return TRUE;
	IXFA_WidgetAccIterator* pWidgetAccIterator = m_pXFADocView->CreateWidgetAccIterator();
	if (pWidgetAccIterator)
	{
		CXFA_EventParam Param;
		Param.m_eType = XFA_EVENT_PreSubmit;
		CXFA_WidgetAcc* pWidgetAcc = pWidgetAccIterator->MoveToNext();
		while (pWidgetAcc)
		{
			pWidgetHandler->ProcessEvent(pWidgetAcc, &Param);
			pWidgetAcc = pWidgetAccIterator->MoveToNext();
		}
		pWidgetAccIterator->Release();
	}
	pWidgetAccIterator = m_pXFADocView->CreateWidgetAccIterator();
	if (pWidgetAccIterator)
	{
		CXFA_WidgetAcc* pWidgetAcc = pWidgetAccIterator->MoveToNext();
		pWidgetAcc = pWidgetAccIterator->MoveToNext();
		while (pWidgetAcc)
		{
			int fRet = pWidgetAcc->ProcessValidate(-1);
			if (fRet == XFA_EVENTERROR_Error)
			{
				CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
				if (pEnv == NULL)
					return FALSE;
				CFX_WideString ws;
				ws.FromLocal(IDS_XFA_Validate_Input);
				CFX_ByteString bs = ws.UTF16LE_Encode();
				int len = bs.GetLength()/sizeof(unsigned short);
				pEnv->FFI_Alert((FPDF_WIDESTRING)bs.GetBuffer(len*sizeof(unsigned short)), (FPDF_WIDESTRING)L"", 0, 1);
				bs.ReleaseBuffer(len*sizeof(unsigned short));
				pWidgetAccIterator->Release();
				return FALSE;
			}
			pWidgetAcc = pWidgetAccIterator->MoveToNext();
		}
		pWidgetAccIterator->Release();
		m_pXFADocView->UpdateDocView();
	}
	
	return TRUE;

}
void CPDFXFA_Document::_OnAfterNotifySumbit()
{
	if (m_iDocType != DOCTYPE_DYNIMIC_XFA && m_iDocType != DOCTYPE_STATIC_XFA)
		return;
	if (m_pXFADocView == NULL)
		return;
	IXFA_WidgetHandler* pWidgetHandler = m_pXFADocView->GetWidgetHandler();
	if (pWidgetHandler == NULL)
		return;
	IXFA_WidgetAccIterator* pWidgetAccIterator = m_pXFADocView->CreateWidgetAccIterator();
	if (pWidgetAccIterator == NULL)
		return;
	CXFA_EventParam Param; 
	Param.m_eType =  XFA_EVENT_PostSubmit;

	CXFA_WidgetAcc* pWidgetAcc = pWidgetAccIterator->MoveToNext();
	while (pWidgetAcc)
	{
		pWidgetHandler->ProcessEvent(pWidgetAcc, &Param);
		pWidgetAcc = pWidgetAccIterator->MoveToNext();
	}
	pWidgetAccIterator->Release();
	m_pXFADocView->UpdateDocView();
}

FX_BOOL CPDFXFA_Document::SubmitData(XFA_HDOC hDoc, CXFA_Submit submit)
{
	if (!_NotifySubmit(TRUE))
		return FALSE;
	if (NULL == m_pXFADocView)
		return FALSE;
	m_pXFADocView->UpdateDocView();

	FX_BOOL ret = _SubmitData(hDoc, submit);
	_NotifySubmit(FALSE);
	return ret;
}

IFX_FileRead* CPDFXFA_Document::OpenLinkedFile(XFA_HDOC hDoc, const CFX_WideString& wsLink)
{
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return FALSE;
	CFX_ByteString bs = wsLink.UTF16LE_Encode();
	int len = bs.GetLength()/sizeof(unsigned short);
	FPDF_FILEHANDLER* pFileHandler = pEnv->FFI_OpenFile(0, (FPDF_WIDESTRING)bs.GetBuffer(len*sizeof(unsigned short)), "rb");
	bs.ReleaseBuffer(len*sizeof(unsigned short));

	if (pFileHandler == NULL)
		return NULL;
	CFPDF_FileStream* pFileRead = FX_NEW CFPDF_FileStream(pFileHandler);
	return pFileRead;
}
FX_BOOL CPDFXFA_Document::_ExportSubmitFile(FPDF_FILEHANDLER* pFileHandler, int fileType, FPDF_DWORD encodeType, FPDF_DWORD flag)
{
	if (NULL == m_pXFADocView)
		return FALSE;
	IXFA_DocHandler* pDocHandler = m_pApp->GetXFAApp()->GetDocHandler();
	CFX_ByteString content;

	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return FALSE;
	
	CFPDF_FileStream fileStream(pFileHandler);

	if (fileType == FXFA_SAVEAS_XML)
	{
		CFX_WideString ws;
		ws.FromLocal("data");
		CFX_ByteString content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
		fileStream.WriteBlock((FX_LPCSTR)content,0,content.GetLength());
		pDocHandler->SavePackage(m_pXFADoc, ws, &fileStream);
	}
	else if (fileType == FXFA_SAVEAS_XDP)
	{	
		if (flag == 0)
			flag = FXFA_CONFIG|FXFA_TEMPLATE|FXFA_LOCALESET|FXFA_DATASETS|FXFA_XMPMETA|FXFA_XFDF|FXFA_FORM;
		if (m_pPDFDoc == NULL) 
		{
			fileStream.Flush();
			return FALSE;
		}	
		CPDF_Dictionary* pRoot = m_pPDFDoc->GetRoot();
		if (pRoot == NULL) 
		{
			fileStream.Flush();
			return FALSE;
		}
		CPDF_Dictionary* pAcroForm = pRoot->GetDict("AcroForm");
		if (NULL == pAcroForm)
		{
			fileStream.Flush();
			return FALSE;
		}
		CPDF_Object* pXFA = pAcroForm->GetElement("XFA");
		if (pXFA == NULL) 
		{
			fileStream.Flush();
			return FALSE;
		}
		if (pXFA->GetType() != PDFOBJ_ARRAY)
		{
			fileStream.Flush();
			return FALSE;
		}
		CPDF_Array* pArray = pXFA->GetArray();
		if (NULL == pArray)
		{
			fileStream.Flush();
			return FALSE;
		}
		int size = pArray->GetCount();
		int iFormIndex = -1;
		int iDataSetsIndex = -1;
		for (int i=1; i<size;i+=2)
		{
			CPDF_Object* pPDFObj = pArray->GetElement(i);
			CPDF_Object* pPrePDFObj = pArray->GetElement(i-1);
			if(pPrePDFObj->GetType() != PDFOBJ_STRING)
				continue;
			if (pPDFObj->GetType() != PDFOBJ_REFERENCE)
				continue;
			CPDF_Object* pDirectObj = pPDFObj->GetDirect();
			if (pDirectObj->GetType() != PDFOBJ_STREAM)
				continue;
			if (pPrePDFObj->GetString()=="config" && !(flag & FXFA_CONFIG))
				continue;
			if (pPrePDFObj->GetString()=="template" && !(flag & FXFA_TEMPLATE))
				continue;
			if (pPrePDFObj->GetString()=="localeSet" && !(flag & FXFA_LOCALESET))
				continue;
			if (pPrePDFObj->GetString()=="datasets" && !(flag & FXFA_DATASETS))
				continue;
			if (pPrePDFObj->GetString()=="xmpmeta" && !(flag & FXFA_XMPMETA))
				continue;
			if (pPrePDFObj->GetString()=="xfdf" && !(flag & FXFA_XFDF))
				continue;
			if (pPrePDFObj->GetString()=="form" && !(flag & FXFA_FORM))
				continue;
			if (pPrePDFObj->GetString()=="form")
			{
				CFX_WideString ws;
				ws.FromLocal("form");
				pDocHandler->SavePackage(m_pXFADoc, ws, &fileStream);
			}
			else if (pPrePDFObj->GetString()=="datasets")
			{
				CFX_WideString ws;
				ws.FromLocal("datasets");
				pDocHandler->SavePackage(m_pXFADoc, ws, &fileStream);
			}
			else 
			{	
				//PDF,creator.
				//TODO:
			}
		}
	}
	return TRUE;
}

void CPDFXFA_Document::_ClearChangeMark()
{
	if (m_pSDKDoc)
		m_pSDKDoc->ClearChangeMark();
}

void CPDFXFA_Document::_ToXFAContentFlags(CFX_WideString csSrcContent, FPDF_DWORD& flag)
{
	if (csSrcContent.Find(L" config ", 0) != -1)
		flag |= FXFA_CONFIG;
	if (csSrcContent.Find(L" template ", 0) != -1)
		flag  |= FXFA_TEMPLATE;
	if (csSrcContent.Find(L" localeSet ", 0) != -1)
		flag  |= FXFA_LOCALESET;
	if (csSrcContent.Find(L" datasets ", 0) != -1)
		flag  |= FXFA_DATASETS;
	if (csSrcContent.Find(L" xmpmeta ", 0) != -1)
		flag  |= FXFA_XMPMETA;
	if (csSrcContent.Find(L" xfdf ", 0) != -1)
		flag  |= FXFA_XFDF;
	if (csSrcContent.Find(L" form ", 0) != -1)
		flag  |= FXFA_FORM;
	if (flag == 0)
		flag = FXFA_CONFIG|FXFA_TEMPLATE|FXFA_LOCALESET|FXFA_DATASETS|FXFA_XMPMETA|FXFA_XFDF|FXFA_FORM;
}
FX_BOOL CPDFXFA_Document::_MailToInfo(CFX_WideString& csURL, CFX_WideString& csToAddress, CFX_WideString& csCCAddress, CFX_WideString& csBCCAddress, CFX_WideString& csSubject, CFX_WideString& csMsg)
{
	CFX_WideString srcURL = csURL;
	srcURL.TrimLeft();
	if (0 != srcURL.Left(7).CompareNoCase(L"mailto:"))
		return FALSE;
	int pos = srcURL.Find(L'?',0);
	CFX_WideString tmp;
	if (pos == -1) {
		pos = srcURL.Find(L'@',0);
		if (pos == -1)
			return FALSE;
		else
		{
			tmp = srcURL.Right(csURL.GetLength()-7);
			tmp.TrimLeft();
			tmp.TrimRight();
		}
	} else {
		tmp = srcURL.Left(pos);
		tmp = tmp.Right(tmp.GetLength()-7);
		tmp.TrimLeft();
		tmp.TrimRight();
	}
	
	csToAddress = tmp;
	
	srcURL = srcURL.Right(srcURL.GetLength()-(pos+1));
	while (	!srcURL.IsEmpty() )
	{
		srcURL.TrimLeft();
		srcURL.TrimRight();
		pos = srcURL.Find(L'&',0);
		if (pos == -1)
			tmp = srcURL;
		else
			tmp = srcURL.Left(pos);
		
		tmp.TrimLeft();
		tmp.TrimRight();
		if (tmp.GetLength() >= 3 && 0 == tmp.Left(3).CompareNoCase(L"cc=") )
		{
			tmp = tmp.Right(tmp.GetLength()-3);
			if (!csCCAddress.IsEmpty())
				csCCAddress += L';';
			csCCAddress += tmp;
			
		} 
		else if (tmp.GetLength() >= 4 && 0 == tmp.Left(4).CompareNoCase(L"bcc="))
		{
			tmp = tmp.Right(tmp.GetLength() - 4);
			if (!csBCCAddress.IsEmpty())
				csBCCAddress += L';';
			csBCCAddress += tmp;
		}
		else if (tmp.GetLength() >= 8 && 0 ==tmp.Left(8).CompareNoCase(L"subject=") )
		{
			tmp=tmp.Right(tmp.GetLength()-8);
			csSubject += tmp;
		}
		else if(tmp.GetLength() >= 5 && 0 ==tmp.Left(5).CompareNoCase(L"body=") )
		{
			tmp=tmp.Right(tmp.GetLength() - 5);
			csMsg += tmp;
		}
		if (pos == -1)
			srcURL = L"";
		else
			srcURL = srcURL.Right(csURL.GetLength()-(pos+1));
	}
	csToAddress.Replace((FX_LPCWSTR)L",", (FX_LPCWSTR)L";");
	csCCAddress.Replace((FX_LPCWSTR)L",", (FX_LPCWSTR)L";");
	csBCCAddress.Replace((FX_LPCWSTR)L",", (FX_LPCWSTR)L";");
	return TRUE;
}

FX_BOOL CPDFXFA_Document::_SubmitData(XFA_HDOC hDoc, CXFA_Submit submit)
{
	CFX_WideStringC csURLC;
	submit.GetSubmitTarget(csURLC);
	CFX_WideString csURL = csURLC;
	CPDFDoc_Environment* pEnv = m_pSDKDoc->GetEnv();
	if (pEnv == NULL)
		return FALSE;
	if (csURL.IsEmpty()){	 
		CFX_WideString ws;
		ws.FromLocal("Submit cancelled.");
		CFX_ByteString bs = ws.UTF16LE_Encode();
		int len = bs.GetLength()/sizeof(unsigned short);
		pEnv->FFI_Alert((FPDF_WIDESTRING)bs.GetBuffer(len*sizeof(unsigned short)), (FPDF_WIDESTRING)L"", 0, 4);
		bs.ReleaseBuffer(len*sizeof(unsigned short));
		return FALSE;
	}

	FPDF_BOOL bRet = TRUE;
	FPDF_FILEHANDLER* pFileHandler = NULL;
	int fileFlag = -1;

	if (submit.GetSubmitFormat() == XFA_ATTRIBUTEENUM_Xdp)
	{
		CFX_WideStringC csContentC;
		submit.GetSubmitXDPContent(csContentC);
		CFX_WideString csContent;
		csContent = csContentC.GetPtr();
		csContent.TrimLeft();
		csContent.TrimRight();
		CFX_WideString space;
		space.FromLocal(" ");
		csContent = space + csContent + space;
		FPDF_DWORD flag = 0;
		if (submit.IsSubmitEmbedPDF())
			flag |= FXFA_PDF;
		_ToXFAContentFlags(csContent, flag);
		pFileHandler = pEnv->FFI_OpenFile(FXFA_SAVEAS_XDP, NULL, "wb");
		fileFlag = FXFA_SAVEAS_XDP;
		_ExportSubmitFile(pFileHandler, FXFA_SAVEAS_XDP, 0, flag);
	}
	else if (submit.GetSubmitFormat() == XFA_ATTRIBUTEENUM_Xml )
	{
		pFileHandler = pEnv->FFI_OpenFile(FXFA_SAVEAS_XML, NULL, "wb");
		fileFlag = FXFA_SAVEAS_XML;
		_ExportSubmitFile(pFileHandler, FXFA_SAVEAS_XML, 0);
	}
	else if (submit.GetSubmitFormat() == XFA_ATTRIBUTEENUM_Pdf )
	{
			//csfilename = csDocName;
	}
	else if (submit.GetSubmitFormat() == XFA_ATTRIBUTEENUM_Formdata )
	{
		return FALSE;
	}
	else if (submit.GetSubmitFormat() == XFA_ATTRIBUTEENUM_Urlencoded )
	{
		pFileHandler = pEnv->FFI_OpenFile(FXFA_SAVEAS_XML, NULL, "wb");
		fileFlag = FXFA_SAVEAS_XML;
		_ExportSubmitFile(pFileHandler, FXFA_SAVEAS_XML, 0);
	}
	else if (submit.GetSubmitFormat() == XFA_ATTRIBUTEENUM_Xfd )
	{
		return FALSE;
	}
	else
	{
		return FALSE;
	}
	if (pFileHandler == NULL)
		return FALSE;
	if (0 == csURL.Left(7).CompareNoCase(L"mailto:"))
	{
		CFX_WideString csToAddress; 
		CFX_WideString csCCAddress;
		CFX_WideString csBCCAddress;
		CFX_WideString csSubject;
		CFX_WideString csMsg;

		bRet = _MailToInfo(csURL, csToAddress, csCCAddress, csBCCAddress, csSubject, csMsg);
		if (FALSE == bRet)
			return FALSE;

		CFX_ByteString bsTo = CFX_WideString(csToAddress).UTF16LE_Encode();
		CFX_ByteString bsCC = CFX_WideString(csCCAddress).UTF16LE_Encode();
		CFX_ByteString bsBcc = CFX_WideString(csBCCAddress).UTF16LE_Encode();
		CFX_ByteString bsSubject = CFX_WideString(csSubject).UTF16LE_Encode();
		CFX_ByteString bsMsg = CFX_WideString(csMsg).UTF16LE_Encode();

		FPDF_WIDESTRING pTo = (FPDF_WIDESTRING)bsTo.GetBuffer(bsTo.GetLength());
		FPDF_WIDESTRING pCC = (FPDF_WIDESTRING)bsCC.GetBuffer(bsCC.GetLength());
		FPDF_WIDESTRING pBcc = (FPDF_WIDESTRING)bsBcc.GetBuffer(bsBcc.GetLength());
		FPDF_WIDESTRING pSubject = (FPDF_WIDESTRING)bsSubject.GetBuffer(bsSubject.GetLength());
		FPDF_WIDESTRING pMsg = (FPDF_WIDESTRING)bsMsg.GetBuffer(bsMsg.GetLength());

		pEnv->FFI_EmailTo(pFileHandler, pTo, pSubject, pCC, pBcc, pMsg);
		bsTo.ReleaseBuffer();
		bsCC.ReleaseBuffer();
		bsBcc.ReleaseBuffer();
		bsSubject.ReleaseBuffer();
		bsMsg.ReleaseBuffer();
	}
	else
	{
		//http¡¢ftp
		CFX_WideString ws;
		CFX_ByteString bs = csURL.UTF16LE_Encode();
		int len = bs.GetLength()/sizeof(unsigned short);
		pEnv->FFI_UploadTo(pFileHandler, fileFlag, (FPDF_WIDESTRING)bs.GetBuffer(len*sizeof(unsigned short)));
		bs.ReleaseBuffer(len*sizeof(unsigned short));
	}

	return bRet;
}

FX_BOOL	CPDFXFA_Document::SetGlobalProperty(XFA_HDOC hDoc, FX_BSTR szPropName, FXJSE_HVALUE hValue)
{
	if (hDoc != m_pXFADoc)
		return FALSE;

	if (m_pSDKDoc && m_pSDKDoc->GetEnv()->GetJSRuntime()) 
		return m_pSDKDoc->GetEnv()->GetJSRuntime()->SetHValueByName(szPropName, hValue);
	return FALSE;
}
FX_BOOL	CPDFXFA_Document::GetPDFScriptObject(XFA_HDOC hDoc, FX_BSTR utf8Name, FXJSE_HVALUE hValue)
{
	if (hDoc != m_pXFADoc)
		return FALSE;

	if (!m_pSDKDoc || !m_pSDKDoc->GetEnv()->GetJSRuntime())
		return FALSE;

	if (!m_pJSContext)
	{
		m_pSDKDoc->GetEnv()->GetJSRuntime()->SetReaderDocument(m_pSDKDoc);
		m_pJSContext =m_pSDKDoc->GetEnv()->GetJSRuntime()->NewContext();
	}

	return _GetHValueByName(utf8Name, hValue, m_pSDKDoc->GetEnv()->GetJSRuntime());

}
FX_BOOL CPDFXFA_Document::GetGlobalProperty(XFA_HDOC hDoc, FX_BSTR szPropName, FXJSE_HVALUE hValue)
{
	if (hDoc != m_pXFADoc)
		return FALSE;
	if (!m_pSDKDoc || !m_pSDKDoc->GetEnv()->GetJSRuntime())
		return FALSE;

	if (!m_pJSContext)
	{
		m_pSDKDoc->GetEnv()->GetJSRuntime()->SetReaderDocument(m_pSDKDoc);
		m_pJSContext = m_pSDKDoc->GetEnv()->GetJSRuntime()->NewContext();
	}

	return _GetHValueByName(szPropName, hValue, m_pSDKDoc->GetEnv()->GetJSRuntime());

}
FX_BOOL CPDFXFA_Document::_GetHValueByName(FX_BSTR utf8Name, FXJSE_HVALUE hValue, IFXJS_Runtime* runTime)
{
	return runTime->GetHValueByName(utf8Name, hValue);
}

