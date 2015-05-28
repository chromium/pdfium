// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FSDK_ACTIONHANDLER_H_
#define FPDFSDK_INCLUDE_FSDK_ACTIONHANDLER_H_

#include "../../core/include/fpdfdoc/fpdf_doc.h"
#include "../../core/include/fxcrt/fx_string.h"
#include "fsdk_baseform.h"

class CFX_PtrList;
class CPDFDoc_Environment;
class CPDFSDK_Annot;
class CPDFSDK_Document;
class CPDF_Bookmark;
class CPDF_Dictionary;
class IFXJS_Runtime;

class CPDFSDK_FormActionHandler
{
public:
	FX_BOOL	DoAction_Hide(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	FX_BOOL	DoAction_SubmitForm(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	FX_BOOL	DoAction_ResetForm(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	FX_BOOL	DoAction_ImportData(const CPDF_Action& action, CPDFSDK_Document* pDocument);
};

class CPDFSDK_MediaActionHandler
{
public:
	FX_BOOL	DoAction_Rendition(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	FX_BOOL	DoAction_Sound(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	FX_BOOL	DoAction_Movie(const CPDF_Action& action, CPDFSDK_Document* pDocument);
};

class  CPDFSDK_ActionHandler /*: public CReader_ActionHandler*/
{
public:
	CPDFSDK_ActionHandler(CPDFDoc_Environment* pEvi);
	virtual ~CPDFSDK_ActionHandler();
	
	virtual void		Destroy();
	virtual FX_BOOL		DoAction_DocOpen(const CPDF_Action& action, CPDFSDK_Document* pDocument/*, CPDFSDK_DocView *pDocView*/);
	virtual FX_BOOL		DoAction_JavaScript(const CPDF_Action& JsAction,CFX_WideString csJSName, CPDFSDK_Document* pDocument/*, CReader_DocView *pDocView*/);
	virtual FX_BOOL		DoAction_Page(const CPDF_Action& action, enum CPDF_AAction::AActionType eType, CPDFSDK_Document* pDocument/*, CReader_DocView *pDocView*/);
	virtual FX_BOOL		DoAction_Document(const CPDF_Action& action, enum CPDF_AAction::AActionType eType, CPDFSDK_Document* pDocument/*, CReader_DocView *pDocView*/);
	virtual FX_BOOL		DoAction_BookMark(CPDF_Bookmark *pBookMark, const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument/*, CReader_DocView *pDocView*/);	
	virtual FX_BOOL		DoAction_Screen(const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument,/* CReader_DocView *pDocView,*/ CPDFSDK_Annot* pScreen);
	virtual FX_BOOL		DoAction_Link(const CPDF_Action& action, CPDFSDK_Document* pDocument/*, CReader_DocView *pDocView*/);
	virtual FX_BOOL		DoAction_Field(const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument, /*CReader_DocView *pDocView,*/ CPDF_FormField* pFormField, PDFSDK_FieldAction& data);
	virtual FX_BOOL		DoAction_FieldJavaScript(const CPDF_Action& JsAction, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument, CPDF_FormField* pFormField, PDFSDK_FieldAction& data);
public:
	void				SetFormActionHandler(CPDFSDK_FormActionHandler* pHandler);
	void				SetMediaActionHandler(CPDFSDK_MediaActionHandler* pHandler);
	
private:
	FX_BOOL				ExecuteDocumentOpenAction(const CPDF_Action& action, CPDFSDK_Document* pDocument, /*CReader_DocView *pDocView,*/ CFX_PtrList& list);
	FX_BOOL				ExecuteDocumentPageAction(const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument, /*CReader_DocView *pDocView,*/ CFX_PtrList& list);
	FX_BOOL				ExecuteFieldAction(const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument, /*CReader_DocView* pDocView,*/ CPDF_FormField* pFormField, PDFSDK_FieldAction& data, CFX_PtrList& list);
	FX_BOOL				ExecuteScreenAction(const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument, /*CReader_DocView* pDocView,*/ CPDFSDK_Annot* pScreen, CFX_PtrList& list);
	FX_BOOL				ExecuteBookMark(const CPDF_Action& action, CPDFSDK_Document* pDocument, /*CReader_DocView* pDocView,*/ CPDF_Bookmark* pBookmark, CFX_PtrList& list);
	FX_BOOL				ExecuteLinkAction(const CPDF_Action& action, CPDFSDK_Document* pDocument, /*CReader_DocView* pDocView,*/ CFX_PtrList& list);
	
	void				DoAction_NoJs(const CPDF_Action& action, CPDFSDK_Document* pDocument/*, CReader_DocView* pDocView*/);
	void				RunDocumentPageJavaScript(CPDFSDK_Document* pDocument, CPDF_AAction::AActionType type, const CFX_WideString& script);
	void				RunDocumentOpenJavaScript(CPDFSDK_Document* pDocument, const CFX_WideString& sScriptName, const CFX_WideString& script);
	void				RunFieldJavaScript(CPDFSDK_Document* pDocument, CPDF_FormField* pFormField, CPDF_AAction::AActionType type, PDFSDK_FieldAction& data, const CFX_WideString& script);
	
private:
	FX_BOOL				IsValidField(CPDFSDK_Document* pDocument, CPDF_Dictionary* pFieldDict);
	FX_BOOL				IsValidDocView(CPDFSDK_Document* pDocument/*, CReader_DocView* pDocView*/);
	
	void				DoAction_GoTo(CPDFSDK_Document* pDocument, /*CReader_DocView *pDocView,*/ const CPDF_Action& action);
	void				DoAction_GoToR(CPDFSDK_Document* pDocument, const CPDF_Action& action);
	void				DoAction_Launch(CPDFSDK_Document* pDocument, const CPDF_Action& action);
	void				DoAction_URI(CPDFSDK_Document* pDocument, const CPDF_Action& action);
	void				DoAction_Named(CPDFSDK_Document* pDocument, const CPDF_Action& action);
	void				DoAction_SetOCGState(CPDFSDK_Document* pDocument, /*CReader_DocView* pDocView,*/ const CPDF_Action& action);
	
private:
	CPDFSDK_FormActionHandler*		m_pFormActionHandler;
	CPDFSDK_MediaActionHandler*		m_pMediaActionHandler;
};

#endif  // FPDFSDK_INCLUDE_FSDK_ACTIONHANDLER_H_
