// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDFXFA_DOC_H_
#define _FPDFXFA_DOC_H_

class CPDFXFA_App;
class CPDFXFA_Document;
class CPDFXFA_Page;
class CPDFSDK_Document;
class CPDFDoc_Environment;
class IFXJS_Runtime;
class IFXJS_Context;

class CPDFXFA_Document : public IXFA_DocProvider, public CFX_Object
{
public:
	CPDFXFA_Document(CPDF_Document* pPDFDoc, CPDFXFA_App* pProvider);
	~CPDFXFA_Document();

	FX_BOOL				LoadXFADoc();
	void				CloseXFADoc();
	CPDFXFA_App*		GetApp() {return m_pApp;}
	CPDF_Document*		GetPDFDoc() { return m_pPDFDoc; }
	XFA_HDOC			GetXFADoc() { return m_pXFADoc; }
	IXFA_DocView*	    GetXFADocView() { return m_pXFADocView; }

	int					GetPageCount();
	CPDFXFA_Page*		GetPage(int page_index);
	CPDFXFA_Page*		GetPage(IXFA_PageView* pPage);
	void				RemovePage(CPDFXFA_Page* page);
	int					GetDocType(){ return m_iDocType; }

	CPDFSDK_Document*	GetSDKDocument(CPDFDoc_Environment* pFormFillEnv);
	void				ReleaseSDKDoc();

	void				FXRect2PDFRect(const CFX_RectF& fxRectF, CPDF_Rect& pdfRect);

public:
	virtual void		SetChangeMark(XFA_HDOC hDoc);
	virtual FX_BOOL		GetChangeMark(XFA_HDOC hDoc);
	//used in dynamic xfa, dwFlags refer to XFA_INVALIDATE_XXX macros.
	virtual void		InvalidateRect(IXFA_PageView* pPageView, const CFX_RectF& rt, FX_DWORD dwFlags = 0);
	//used in static xfa, dwFlags refer to XFA_INVALIDATE_XXX macros.
	virtual void		InvalidateRect(XFA_HWIDGET hWidget, FX_DWORD dwFlags = 0);
	//show or hide caret
	virtual void		DisplayCaret(XFA_HWIDGET hWidget, FX_BOOL bVisible, const CFX_RectF* pRtAnchor);
	//dwPos: (0:bottom 1:top)
	virtual FX_BOOL		GetPopupPos(XFA_HWIDGET hWidget, FX_FLOAT fMinPopup, FX_FLOAT fMaxPopup, 
							const CFX_RectF &rtAnchor, CFX_RectF &rtPopup);
	virtual FX_BOOL		PopupMenu(XFA_HWIDGET hWidget, CFX_PointF ptPopup, const CFX_RectF* pRectExclude = NULL);

	//dwFlags XFA_PAGEVIEWEVENT_Added, XFA_PAGEVIEWEVENT_Removing
	virtual void		PageViewEvent(IXFA_PageView* pPageView, FX_DWORD dwFlags);
	//dwEvent refer to XFA_WIDGETEVENT_XXX
	virtual void		WidgetEvent(XFA_HWIDGET hWidget, CXFA_WidgetAcc* pWidgetData, FX_DWORD dwEvent, FX_LPVOID pParam = NULL, FX_LPVOID pAdditional = NULL);
	
	//return true if render it.
	virtual FX_BOOL		RenderCustomWidget(XFA_HWIDGET hWidget, CFX_Graphics* pGS, CFX_Matrix* pMatrix, const CFX_RectF& rtUI){return FALSE;}

	//host method
	virtual	FX_INT32	CountPages(XFA_HDOC hDoc);
	virtual	FX_INT32	GetCurrentPage(XFA_HDOC hDoc);
	virtual void		SetCurrentPage(XFA_HDOC hDoc, FX_INT32 iCurPage);
	virtual FX_BOOL		IsCalculationsEnabled(XFA_HDOC hDoc);
	virtual void		SetCalculationsEnabled(XFA_HDOC hDoc, FX_BOOL bEnabled);
	virtual void		GetTitle(XFA_HDOC hDoc, CFX_WideString &wsTitle);
	virtual void		SetTitle(XFA_HDOC hDoc, FX_WSTR wsTitle);
	virtual void		ExportData(XFA_HDOC hDoc, FX_WSTR wsFilePath, FX_BOOL bXDP = TRUE);
	virtual void		ImportData(XFA_HDOC hDoc, FX_WSTR wsFilePath);
	virtual void		GotoURL(XFA_HDOC hDoc, FX_WSTR bsURL, FX_BOOL bAppend = TRUE);
	virtual FX_BOOL		IsValidationsEnabled(XFA_HDOC hDoc);
	virtual void		SetValidationsEnabled(XFA_HDOC hDoc, FX_BOOL bEnabled);
	virtual void		SetFocusWidget(XFA_HDOC hDoc, XFA_HWIDGET hWidget);
	virtual void		Print(XFA_HDOC hDoc, FX_INT32 nStartPage, FX_INT32 nEndPage, FX_DWORD dwOptions);

	//LayoutPseudo method
	virtual FX_INT32			AbsPageCountInBatch(XFA_HDOC hDoc){return 0;}
	virtual FX_INT32			AbsPageInBatch(XFA_HDOC hDoc, XFA_HWIDGET hWidget){return 0;}
	virtual FX_INT32			SheetCountInBatch(XFA_HDOC hDoc){return 0;}
	virtual FX_INT32			SheetInBatch(XFA_HDOC hDoc, XFA_HWIDGET hWidget){return 0;}

	//SignaturePseudoModel method
	//TODO:
	virtual FX_INT32			Verify(XFA_HDOC hDoc, CXFA_Node* pSigNode, FX_BOOL bUsed = TRUE/*, SecurityHandler* pHandler, SignatureInfo &info*/) {return 0;}
	virtual FX_BOOL				Sign(XFA_HDOC hDoc, CXFA_NodeList* pNodeList, FX_WSTR wsExpression, FX_WSTR wsXMLIdent, FX_WSTR wsValue = FX_WSTRC(L"open"), FX_BOOL bUsed = TRUE/*, SecurityHandler* pHandler = NULL, SignatureInfo &info*/) {return 0;}
	virtual CXFA_NodeList*		Enumerate(XFA_HDOC hDoc) {return 0;}
	virtual FX_BOOL				Clear(XFA_HDOC hDoc, CXFA_Node* pSigNode, FX_BOOL bCleared = TRUE) {return 0;}

	//Get document path
	virtual void		GetURL(XFA_HDOC hDoc, CFX_WideString &wsDocURL);
	virtual FX_ARGB		GetHighlightColor(XFA_HDOC hDoc);
	virtual void		AddDoRecord(XFA_HWIDGET hWidget);
	/** 
	 *Submit data to email, http, ftp.	
	 * @param[in] hDoc The document handler.
	 * @param[in] eFormat Determines the format in which the data will be submitted. XFA_ATTRIBUTEENUM_Xdp, XFA_ATTRIBUTEENUM_Xml...
	 * @param[in] wsTarget The URL to which the data will be submitted.
	 * @param[in] eEncoding The encoding of text content.
	 * @param[in] pXDPContent Controls what subset of the data is submitted, used only when the format property is xdp.
	 * @param[in] bEmbedPDF, specifies whether PDF is embedded in the submitted content or not.
	 */
	virtual FX_BOOL		SubmitData(XFA_HDOC hDoc, CXFA_Submit submit);

	virtual FX_BOOL		CheckWord(XFA_HDOC hDoc, FX_BSTR sWord){return FALSE;}
	virtual FX_BOOL		GetSuggestWords(XFA_HDOC hDoc, FX_BSTR sWord, CFX_ByteStringArray& sSuggest){return FALSE;}

	//Get PDF javascript object, set the object to hValue.
	virtual FX_BOOL		GetPDFScriptObject(XFA_HDOC hDoc, FX_BSTR utf8Name, FXJSE_HVALUE hValue);

	virtual FX_BOOL		GetGlobalProperty(XFA_HDOC hDoc, FX_BSTR szPropName, FXJSE_HVALUE hValue);
	virtual FX_BOOL		SetGlobalProperty(XFA_HDOC hDoc, FX_BSTR szPropName, FXJSE_HVALUE hValue);
	virtual CPDF_Document*  OpenPDF(XFA_HDOC hDoc, IFX_FileRead* pFile, FX_BOOL bTakeOverFile){return NULL;}

	virtual IFX_FileRead*	OpenLinkedFile(XFA_HDOC hDoc, const CFX_WideString& wsLink);

	FX_BOOL		_GetHValueByName(FX_BSTR utf8Name, FXJSE_HVALUE hValue, IFXJS_Runtime* runTime);
	FX_BOOL		_OnBeforeNotifySumbit();
	void		_OnAfterNotifySumbit();
	FX_BOOL		_NotifySubmit(FX_BOOL bPrevOrPost);
	FX_BOOL		_SubmitData(XFA_HDOC hDoc, CXFA_Submit submit);
	FX_BOOL		_MailToInfo(CFX_WideString& csURL, CFX_WideString& csToAddress, CFX_WideString& csCCAddress, CFX_WideString& csBCCAddress, CFX_WideString& csSubject, CFX_WideString& csMsg);
	FX_BOOL		_ExportSubmitFile(FPDF_FILEHANDLER* ppFileHandler, int fileType, FPDF_DWORD encodeType, FPDF_DWORD flag = 0x01111111);
	void		_ToXFAContentFlags(CFX_WideString csSrcContent, FPDF_DWORD& flag);
	void		_ClearChangeMark();

private:
	CPDF_Document* m_pPDFDoc;
	XFA_HDOC  m_pXFADoc;
	IXFA_DocView* m_pXFADocView;
	CFX_ArrayTemplate<CPDFXFA_Page*> m_XFAPageList;

	CPDFSDK_Document* m_pSDKDoc;
	CPDFXFA_App* m_pApp;

	CFX_MapByteStringToPtr	m_XfaGlobalProperty;

	CFX_MapByteStringToPtr	m_ValueMap;

	IFXJS_Context*			m_pJSContext;

	int			m_iDocType;
};

#endif 
