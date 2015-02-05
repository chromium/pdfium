// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDFSDK_MGR_H
#define _FPDFSDK_MGR_H

#include "fsdk_common.h"
#include "fsdk_define.h"
#include "fx_systemhandler.h"
#include "fsdk_baseannot.h"
#include "fsdk_baseform.h"
#include "fpdfformfill.h"
#include "fsdk_annothandler.h"
#include "fsdk_actionhandler.h"

//cross platform keycode and events define.
#include "fpdf_fwlevent.h"


class CPDFSDK_Document;
class CPDFSDK_PageView;
class CPDFSDK_Annot;
class CFFL_IFormFiller;
class CPDFSDK_Widget;
class IFX_SystemHandler;
class CPDFSDK_ActionHandler;
class CJS_RuntimeFactory;

#include "javascript/IJavaScript.h"

class CPDFDoc_Environment FX_FINAL
{
public:
	CPDFDoc_Environment(CPDF_Document * pDoc);
	~CPDFDoc_Environment();

	int RegAppHandle(FPDF_FORMFILLINFO* pFFinfo);//{ m_pInfo  = pFFinfo; return TRUE;}

	void FFI_Invalidate(FPDF_PAGE page, double left, double top, double right, double bottom)
	{
		if (m_pInfo && m_pInfo->FFI_Invalidate)
		{
			m_pInfo->FFI_Invalidate(m_pInfo, page, left, top, right, bottom);
		}
	}
	void FFI_OutputSelectedRect(FPDF_PAGE page, double left, double top, double right, double bottom)
	{
		if (m_pInfo && m_pInfo->FFI_OutputSelectedRect)
		{
			m_pInfo->FFI_OutputSelectedRect(m_pInfo, page, left, top, right, bottom);
		}
	}

	void FFI_SetCursor(int nCursorType)
	{
		if (m_pInfo && m_pInfo->FFI_SetCursor)
		{
			m_pInfo->FFI_SetCursor(m_pInfo, nCursorType);
		}
	}

	int  FFI_SetTimer(int uElapse, TimerCallback lpTimerFunc)
	{
		if (m_pInfo && m_pInfo->FFI_SetTimer)
		{
			return m_pInfo->FFI_SetTimer(m_pInfo, uElapse, lpTimerFunc);
		}
		return -1;
	}

	void FFI_KillTimer(int nTimerID)
	{
		if (m_pInfo && m_pInfo->FFI_KillTimer)
		{
			m_pInfo->FFI_KillTimer(m_pInfo, nTimerID);
		}
	}
	FX_SYSTEMTIME FFI_GetLocalTime()
	{
		FX_SYSTEMTIME fxtime;
		if(m_pInfo && m_pInfo->FFI_GetLocalTime)
		{
			FPDF_SYSTEMTIME systime = m_pInfo->FFI_GetLocalTime(m_pInfo);
			fxtime.wDay = systime.wDay;
			fxtime.wDayOfWeek = systime.wDayOfWeek;
			fxtime.wHour = systime.wHour;
			fxtime.wMilliseconds = systime.wMilliseconds;
			fxtime.wMinute = systime.wMinute;
			fxtime.wMonth = systime.wMonth;
			fxtime.wSecond = systime.wSecond;
			fxtime.wYear = systime.wYear;
		}
		return fxtime;
	}

	void FFI_OnChange()
	{
		if(m_pInfo && m_pInfo->FFI_OnChange)
		{
			m_pInfo->FFI_OnChange(m_pInfo);
		}
	}

	FX_BOOL	FFI_IsSHIFTKeyDown(FX_DWORD nFlag)
	{

		return (nFlag & FWL_EVENTFLAG_ShiftKey) != 0;
	}
	FX_BOOL	FFI_IsCTRLKeyDown(FX_DWORD nFlag)
	{

		return (nFlag & FWL_EVENTFLAG_ControlKey) != 0;
	}
	FX_BOOL	FFI_IsALTKeyDown(FX_DWORD nFlag)
	{

		return (nFlag & FWL_EVENTFLAG_AltKey) != 0;
	}
	FX_BOOL	FFI_IsINSERTKeyDown(FX_DWORD nFlag)
	{
		return FALSE;
	}

	int JS_appAlert(FX_LPCWSTR Msg, FX_LPCWSTR Title, FX_UINT Type, FX_UINT Icon)
	{
		if(m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->app_alert)
		{
			CFX_ByteString bsMsg = CFX_WideString(Msg).UTF16LE_Encode();
			CFX_ByteString bsTitle = CFX_WideString(Title).UTF16LE_Encode();
			FPDF_WIDESTRING pMsg = (FPDF_WIDESTRING)bsMsg.GetBuffer(bsMsg.GetLength());
			FPDF_WIDESTRING pTitle = (FPDF_WIDESTRING)bsTitle.GetBuffer(bsTitle.GetLength());
			int ret = m_pInfo->m_pJsPlatform->app_alert(m_pInfo->m_pJsPlatform, pMsg, pTitle, Type, Icon);
			bsMsg.ReleaseBuffer();
			bsTitle.ReleaseBuffer();
			return ret;
		}
		return -1;
	}

	int JS_appResponse(FX_LPCWSTR Question, FX_LPCWSTR Title, FX_LPCWSTR Default, FX_LPCWSTR cLabel, FPDF_BOOL bPassword, void* response, int length)
	{
		if (m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->app_response)
		{
			CFX_ByteString bsQuestion = CFX_WideString(Question).UTF16LE_Encode();
			CFX_ByteString bsTitle = CFX_WideString(Title).UTF16LE_Encode();
			CFX_ByteString bsDefault = CFX_WideString(Default).UTF16LE_Encode();
			CFX_ByteString bsLabel = CFX_WideString(cLabel).UTF16LE_Encode();
			FPDF_WIDESTRING pQuestion = (FPDF_WIDESTRING)bsQuestion.GetBuffer(bsQuestion.GetLength());
			FPDF_WIDESTRING pTitle = (FPDF_WIDESTRING)bsTitle.GetBuffer(bsTitle.GetLength());
			FPDF_WIDESTRING pDefault = (FPDF_WIDESTRING)bsDefault.GetBuffer(bsDefault.GetLength());
			FPDF_WIDESTRING pLabel = (FPDF_WIDESTRING)bsLabel.GetBuffer(bsLabel.GetLength());
			int ret = m_pInfo->m_pJsPlatform->app_response(m_pInfo->m_pJsPlatform, pQuestion, pTitle,
				pDefault, pLabel, bPassword, response, length);
			bsQuestion.ReleaseBuffer();
			bsTitle.ReleaseBuffer();
			bsDefault.ReleaseBuffer();
			bsLabel.ReleaseBuffer();
			return ret;
		}
		return -1;
	}

	void JS_appBeep(int nType)
	{
		if(m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->app_beep)
		{
			m_pInfo->m_pJsPlatform->app_beep(m_pInfo->m_pJsPlatform, nType);
		}
	}

	CFX_WideString JS_fieldBrowse()
	{
		if (m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->Field_browse)
		{
			int nRequiredLen = m_pInfo->m_pJsPlatform->Field_browse(m_pInfo->m_pJsPlatform, NULL, 0);
			if (nRequiredLen <= 0)
				return L"";

			char* pbuff = new char[nRequiredLen];
			if (!pbuff)
				return L"";

			memset(pbuff, 0, nRequiredLen);
			int nActualLen = m_pInfo->m_pJsPlatform->Field_browse(m_pInfo->m_pJsPlatform, pbuff, nRequiredLen);
			if (nActualLen <= 0 || nActualLen > nRequiredLen)
			{
				delete[] pbuff;
				return L"";
			}
			CFX_ByteString bsRet = CFX_ByteString(pbuff, nActualLen);
			CFX_WideString wsRet = CFX_WideString::FromLocal(bsRet);
			delete[] pbuff;
			return wsRet;
		}
		return L"";
	}

	CFX_WideString JS_docGetFilePath()
	{
		if (m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->Doc_getFilePath)
		{
			int nRequiredLen = m_pInfo->m_pJsPlatform->Doc_getFilePath(m_pInfo->m_pJsPlatform, NULL, 0);
			if (nRequiredLen <= 0)
				return L"";

			char* pbuff = new char[nRequiredLen];
			if (!pbuff)
				return L"";

			memset(pbuff, 0, nRequiredLen);
			int nActualLen = m_pInfo->m_pJsPlatform->Doc_getFilePath(m_pInfo->m_pJsPlatform, pbuff, nRequiredLen);
			if (nActualLen <= 0 || nActualLen > nRequiredLen)
			{
				delete[] pbuff;
				return L"";
			}
			CFX_ByteString bsRet = CFX_ByteString(pbuff, nActualLen);
			CFX_WideString wsRet = CFX_WideString::FromLocal(bsRet);
			delete[] pbuff;
			return wsRet;
		}
		return L"";
	}

	void JS_docSubmitForm(void* formData, int length, FX_LPCWSTR URL)
	{
		if(m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->Doc_submitForm)
		{
			CFX_ByteString bsDestination = CFX_WideString(URL).UTF16LE_Encode();
			FPDF_WIDESTRING pDestination = (FPDF_WIDESTRING)bsDestination.GetBuffer(bsDestination.GetLength());
			m_pInfo->m_pJsPlatform->Doc_submitForm(m_pInfo->m_pJsPlatform, formData, length, pDestination);
			bsDestination.ReleaseBuffer();
		}
	}

	void JS_docmailForm(void* mailData, int length, FPDF_BOOL bUI,FX_LPCWSTR To, FX_LPCWSTR Subject, FX_LPCWSTR CC, FX_LPCWSTR BCC, FX_LPCWSTR Msg)
	{
		if(m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->Doc_mail)
		{
			CFX_ByteString bsTo = CFX_WideString(To).UTF16LE_Encode();
			CFX_ByteString bsCC = CFX_WideString(Subject).UTF16LE_Encode();
			CFX_ByteString bsBcc = CFX_WideString(BCC).UTF16LE_Encode();
			CFX_ByteString bsSubject = CFX_WideString(Subject).UTF16LE_Encode();
			CFX_ByteString bsMsg = CFX_WideString(Msg).UTF16LE_Encode();
			FPDF_WIDESTRING pTo = (FPDF_WIDESTRING)bsTo.GetBuffer(bsTo.GetLength());
			FPDF_WIDESTRING pCC = (FPDF_WIDESTRING)bsCC.GetBuffer(bsCC.GetLength());
			FPDF_WIDESTRING pBcc = (FPDF_WIDESTRING)bsBcc.GetBuffer(bsBcc.GetLength());
			FPDF_WIDESTRING pSubject = (FPDF_WIDESTRING)bsSubject.GetBuffer(bsSubject.GetLength());
			FPDF_WIDESTRING pMsg = (FPDF_WIDESTRING)bsMsg.GetBuffer(bsMsg.GetLength());
			m_pInfo->m_pJsPlatform->Doc_mail(m_pInfo->m_pJsPlatform, mailData, length, bUI, pTo, pSubject,
				pCC, pBcc, pMsg);
			bsTo.ReleaseBuffer();
			bsCC.ReleaseBuffer();
			bsBcc.ReleaseBuffer();
			bsSubject.ReleaseBuffer();
			bsMsg.ReleaseBuffer();
		}
	}

	void JS_docprint(FPDF_BOOL bUI , int nStart, int nEnd, FPDF_BOOL bSilent ,FPDF_BOOL bShrinkToFit,FPDF_BOOL bPrintAsImage ,FPDF_BOOL bReverse ,FPDF_BOOL bAnnotations)
	{
		if(m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->Doc_print)
		{
			m_pInfo->m_pJsPlatform->Doc_print(m_pInfo->m_pJsPlatform, bUI, nStart, nEnd, bSilent, bShrinkToFit, bPrintAsImage, bReverse, bAnnotations);
		}
	}
	void JS_docgotoPage(int nPageNum)
	{
		if(m_pInfo && m_pInfo->m_pJsPlatform && m_pInfo->m_pJsPlatform->Doc_gotoPage)
		{
			m_pInfo->m_pJsPlatform->Doc_gotoPage(m_pInfo->m_pJsPlatform, nPageNum);
		}
	}

	FPDF_PAGE	FFI_GetPage(FPDF_DOCUMENT document,int nPageIndex)
	{
		if(m_pInfo && m_pInfo->FFI_GetPage)
		{
			return m_pInfo->FFI_GetPage(m_pInfo, document, nPageIndex);
		}
		return NULL;
	}

	FPDF_PAGE FFI_GetCurrentPage(FPDF_DOCUMENT document)
	{
		if(m_pInfo && m_pInfo->FFI_GetCurrentPage)
		{
			return m_pInfo->FFI_GetCurrentPage(m_pInfo, document);
		}
		return NULL;
	}

	int 	FFI_GetRotation(FPDF_PAGE page)
	{
		if(m_pInfo && m_pInfo->FFI_GetRotation)
		{
			return m_pInfo->FFI_GetRotation(m_pInfo, page);
		}
		return 0;
	}
	void	FFI_ExecuteNamedAction(FX_LPCSTR namedAction)
	{
		if(m_pInfo && m_pInfo->FFI_ExecuteNamedAction)
		{
			m_pInfo->FFI_ExecuteNamedAction(m_pInfo, namedAction);
		}
	}
	void	FFI_OnSetFieldInputFocus(void* field,FPDF_WIDESTRING focusText, FPDF_DWORD nTextLen, FX_BOOL bFocus)
	{
		if(m_pInfo && m_pInfo->FFI_SetTextFieldFocus)
		{
			m_pInfo->FFI_SetTextFieldFocus(m_pInfo, focusText, nTextLen, bFocus);
		}
	}

	void	FFI_DoURIAction(FX_LPCSTR bsURI)
	{
		if(m_pInfo && m_pInfo->FFI_DoURIAction)
		{
			m_pInfo->FFI_DoURIAction(m_pInfo, bsURI);
		}
	}

	void	FFI_DoGoToAction(int nPageIndex, int zoomMode, float* fPosArray, int sizeOfArray)
	{
		if(m_pInfo && m_pInfo->FFI_DoGoToAction)
		{
			m_pInfo->FFI_DoGoToAction(m_pInfo, nPageIndex, zoomMode, fPosArray, sizeOfArray);
		}
	}

public:
	FX_BOOL				IsJSInitiated();

public:
	void				SetCurrentDoc(CPDFSDK_Document* pFXDoc) {m_pSDKDoc = pFXDoc;}
	CPDFSDK_Document*	GetCurrentDoc();
	CPDF_Document*		GetPDFDocument() {return m_pPDFDoc;}
// 	CPDFSDK_Document*   GetDocument(int nIndex);
// 	int					CountDocuments() {return m_docMap.GetCount();}

	CPDFSDK_Document*		OpenDocument(CFX_WideString &fileName){return NULL;}
	CPDFSDK_Document*		OpenMemPDFDoc(CPDF_Document* pNewDoc, CFX_WideString &fileName){return NULL;}


	CFX_ByteString		GetAppName() {return "";}

	CFFL_IFormFiller*	GetIFormFiller();
	IFX_SystemHandler*	GetSysHandler() {return m_pSysHandler;}

public:
	CPDFSDK_AnnotHandlerMgr* GetAnnotHandlerMgr();
	IFXJS_Runtime*	GetJSRuntime();
	CPDFSDK_ActionHandler* GetActionHander();
private:
	CPDFSDK_AnnotHandlerMgr* m_pAnnotHandlerMgr;
	CPDFSDK_ActionHandler*	m_pActionHandler;
	IFXJS_Runtime*	m_pJSRuntime;
public:
	FPDF_FORMFILLINFO* GetFormFillInfo() {return m_pInfo;}
private:
	FPDF_FORMFILLINFO*	m_pInfo;
//	CFX_MapPtrTemplate<CPDF_Document*, CPDFSDK_Document*> m_docMap;
	CPDFSDK_Document* m_pSDKDoc;
	CPDF_Document* m_pPDFDoc;

	CFFL_IFormFiller* m_pIFormFiller;
	IFX_SystemHandler* m_pSysHandler;

public:
	CJS_RuntimeFactory*  m_pJSRuntimeFactory;
};



// class CFX_App
// {
// public:
// 	CFX_App():m_pCurDoc(NULL) {}
// 	void SetAt(CPDF_Document* pPDFDoc, CPDFSDK_Document* pFXDoc);
// 	CPDFSDK_Document* GetAt(CPDF_Document* pPDFDoc);
// public:
// 	void SetCurrentDocument(CPDFSDK_Document* pFXDoc) {m_pCurDoc = pFXDoc;}
// 	CPDFSDK_Document* GetCurrentDocument() {return m_pCurDoc;}
// private:
// 	CFX_MapPtrTemplate<CPDF_Document*, CPDFSDK_Document*> m_docArray;
// 	CPDFSDK_Document* m_pCurDoc;
// };
class CPDFSDK_InterForm;
class CPDFSDK_Document
{
public:
	CPDFSDK_Document(CPDF_Document* pDoc, CPDFDoc_Environment* pEnv);
	~CPDFSDK_Document();
public:
	CPDFSDK_InterForm*		GetInterForm() ;
	CPDF_Document*			GetDocument() {return m_pDoc;}

public:
	void					InitPageView();
	void					AddPageView(CPDF_Page* pPDFPage, CPDFSDK_PageView* pPageView);
	CPDFSDK_PageView*		GetPageView(CPDF_Page* pPDFPage, FX_BOOL ReNew = TRUE);
	CPDFSDK_PageView*		GetPageView(int nIndex);
	CPDFSDK_PageView*		GetCurrentView();
	void					ReMovePageView(CPDF_Page* pPDFPage);
	void					UpdateAllViews(CPDFSDK_PageView* pSender, CPDFSDK_Annot* pAnnot);

	CPDFSDK_Annot*			GetFocusAnnot();//{return NULL;}

	IFXJS_Runtime *			GetJsRuntime();

	FX_BOOL					SetFocusAnnot(CPDFSDK_Annot* pAnnot, FX_UINT nFlag = 0);//{return FALSE;}
	FX_BOOL					KillFocusAnnot(FX_UINT nFlag = 0);

	FX_BOOL					ExtractPages(const CFX_WordArray &arrExtraPages, CPDF_Document* pDstDoc);
	FX_BOOL					InsertPages(int nInsertAt, const CPDF_Document* pSrcDoc, const CFX_WordArray &arrSrcPages);
	FX_BOOL					DeletePages(int nStart, int nCount);
	FX_BOOL					ReplacePages(int nPage, const CPDF_Document* pSrcDoc, const CFX_WordArray &arrSrcPages);

	void					OnCloseDocument();

	int						GetPageCount() {return m_pDoc->GetPageCount();}
	FX_BOOL					GetPermissions(int nFlag);
	FX_BOOL					GetChangeMark() {return m_bChangeMask;}
	void					SetChangeMark() {m_bChangeMask = TRUE;}
	void					ClearChangeMark() {m_bChangeMask= FALSE;}
//	FX_BOOL					GetChangeMark(){return FALSE;}//IsAnnotModified()||IsFormModified() || IsWidgetModified()|| m_nChangeMark>0 ;}
//	void                    ClearChangeMark(){}
	CFX_WideString			GetPath() ;
	CPDF_Page*				GetPage(int nIndex);
	CPDFDoc_Environment *	GetEnv() {return m_pEnv; }
	void				    ProcJavascriptFun();
	FX_BOOL					ProcOpenAction();
	CPDF_OCContext*			GetOCContext();
private:
	//CFX_ArrayTemplate<CPDFSDK_PageView*> m_pageArray;
	CFX_MapPtrTemplate<CPDF_Page*, CPDFSDK_PageView*> m_pageMap;
	CPDF_Document*			m_pDoc;

	CPDFSDK_InterForm*		m_pInterForm;
	CPDFSDK_Annot*			m_pFocusAnnot;
	CPDFDoc_Environment *	m_pEnv;
	CPDF_OCContext *		m_pOccontent;
	FX_BOOL					m_bChangeMask;
};
class CPDFSDK_PageView FX_FINAL
{
public:
	CPDFSDK_PageView(CPDFSDK_Document* pSDKDoc,CPDF_Page* page);
	~CPDFSDK_PageView();
	void PageView_OnDraw(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,CPDF_RenderOptions* pOptions) ;
	CPDF_Annot*						GetPDFAnnotAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
	CPDFSDK_Annot*					GetFXAnnotAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
	CPDF_Annot*						GetPDFWidgetAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
	CPDFSDK_Annot*					GetFXWidgetAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
	CPDFSDK_Annot*					GetFocusAnnot() ;
	void							SetFocusAnnot(CPDFSDK_Annot* pSDKAnnot,FX_UINT nFlag = 0) {m_pSDKDoc->SetFocusAnnot(pSDKAnnot, nFlag);}
	FX_BOOL							KillFocusAnnot(FX_UINT nFlag = 0) {return m_pSDKDoc->KillFocusAnnot(nFlag);}
	FX_BOOL							Annot_HasAppearance(CPDF_Annot* pAnnot);

	CPDFSDK_Annot*					AddAnnot(CPDF_Dictionary * pDict);
	CPDFSDK_Annot*					AddAnnot(FX_LPCSTR lpSubType,CPDF_Dictionary * pDict);
	CPDFSDK_Annot*					AddAnnot(CPDF_Annot * pPDFAnnot);
        FX_BOOL						DeleteAnnot(CPDFSDK_Annot* pAnnot);
	int								CountAnnots();
	CPDFSDK_Annot*					GetAnnot(int nIndex);
	CPDFSDK_Annot*				    GetAnnotByDict(CPDF_Dictionary * pDict);
	CPDF_Page*						GetPDFPage(){return m_page;}
	CPDF_Document*					GetPDFDocument();
	CPDFSDK_Document*				GetSDKDocument() {return m_pSDKDoc;}
	FX_BOOL					OnLButtonDown(const CPDF_Point & point, FX_UINT nFlag);
	FX_BOOL					OnLButtonUp(const CPDF_Point & point, FX_UINT nFlag);
	FX_BOOL					OnChar(int nChar, FX_UINT nFlag);
	FX_BOOL					OnKeyDown(int nKeyCode, int nFlag);
	FX_BOOL					OnKeyUp(int nKeyCode, int nFlag);

	FX_BOOL					OnMouseMove(const CPDF_Point & point, int nFlag);
	FX_BOOL					OnMouseWheel(double deltaX, double deltaY,const CPDF_Point& point, int nFlag);
	FX_BOOL					IsValidAnnot(FX_LPVOID p);
	void					GetCurrentMatrix(CPDF_Matrix& matrix) {matrix = m_curMatrix;}
	void					UpdateRects(CFX_RectArray& rects);
	void							UpdateView(CPDFSDK_Annot* pAnnot);
	CFX_PtrArray*					GetAnnotList(){ return &m_fxAnnotArray; }

	int						GetPageIndex();
	void							LoadFXAnnots();

        void SetValid(FX_BOOL bValid) {m_bValid = bValid;}
        FX_BOOL IsValid() {return m_bValid;}
        void SetLock(FX_BOOL bLocked) {m_bLocked= bLocked;}
        FX_BOOL IsLocked() {return m_bLocked;}
        void TakeOverPage() {m_bTakeOverPage = TRUE;}
private:
	void PageView_OnHighlightFormFields(CFX_RenderDevice* pDevice, CPDFSDK_Widget* pWidget);
        CPDF_Matrix m_curMatrix;
	CPDF_Page* m_page;
	CPDF_AnnotList* m_pAnnotList;
	//CPDFSDK_Annot* m_pFocusAnnot;
	CFX_PtrArray  m_fxAnnotArray;
	CPDFSDK_Document* m_pSDKDoc;
	CPDFSDK_Widget* m_CaptureWidget;
	FX_BOOL m_bEnterWidget;
	FX_BOOL m_bExitWidget;
	FX_BOOL m_bOnWidget;
	FX_BOOL m_bValid;
        FX_BOOL m_bLocked;
        FX_BOOL m_bTakeOverPage;
};


template<class TYPE>
class CGW_ArrayTemplate : public CFX_ArrayTemplate<TYPE>
{
public:
	CGW_ArrayTemplate(){}
	virtual ~CGW_ArrayTemplate(){}

	typedef int (*LP_COMPARE)(TYPE p1, TYPE p2);

	void Sort(LP_COMPARE pCompare, FX_BOOL bAscent = TRUE)
	{
		int nSize = this->GetSize();
		QuickSort(0, nSize -1, bAscent, pCompare);
	}

private:
	void QuickSort(FX_UINT nStartPos, FX_UINT nStopPos, FX_BOOL bAscend, LP_COMPARE pCompare)
	{
		if (nStartPos >= nStopPos) return;

		if ((nStopPos - nStartPos) == 1)
		{
			TYPE Value1 = this->GetAt(nStartPos);
			TYPE Value2 = this->GetAt(nStopPos);

			int iGreate = (*pCompare)(Value1, Value2);
			if ((bAscend && iGreate > 0) || (!bAscend && iGreate < 0))
			{
				this->SetAt(nStartPos, Value2);
				this->SetAt(nStopPos, Value1);
			}
			return;
		}

		FX_UINT m = nStartPos + (nStopPos - nStartPos) / 2;
		FX_UINT i = nStartPos;

		TYPE Value = this->GetAt(m);

		while (i < m)
		{
			TYPE temp = this->GetAt(i);

			int iGreate = (*pCompare)(temp, Value);
			if ((bAscend && iGreate > 0) || (!bAscend && iGreate < 0))
			{
				this->InsertAt(m+1, temp);
				this->RemoveAt(i);
				m--;
			}
			else
			{
				i++;
			}
		}

		FX_UINT j = nStopPos;

		while (j > m)
		{
			TYPE temp = this->GetAt(j);

			int iGreate = (*pCompare)(temp, Value);
			if ((bAscend && iGreate < 0) || (!bAscend && iGreate > 0))
			{
				this->RemoveAt(j);
				this->InsertAt(m, temp);
				m++;
			}
			else
			{
				j--;
			}
		}

		if (nStartPos < m) QuickSort(nStartPos, m, bAscend, pCompare);
		if (nStopPos > m) QuickSort(m, nStopPos, bAscend, pCompare);
	}
};


#endif //_FPDFSDK_MGR_H
