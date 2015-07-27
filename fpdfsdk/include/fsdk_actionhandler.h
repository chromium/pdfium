// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FSDK_ACTIONHANDLER_H_
#define FPDFSDK_INCLUDE_FSDK_ACTIONHANDLER_H_

#include "../../core/include/fpdfdoc/fpdf_doc.h"
#include "../../core/include/fxcrt/fx_string.h"
#include "../../third_party/base/nonstd_unique_ptr.h"
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
	bool	DoAction_Hide(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	bool	DoAction_SubmitForm(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	bool	DoAction_ResetForm(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	bool	DoAction_ImportData(const CPDF_Action& action, CPDFSDK_Document* pDocument);
};

class CPDFSDK_MediaActionHandler
{
public:
	bool	DoAction_Rendition(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	bool	DoAction_Sound(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	bool	DoAction_Movie(const CPDF_Action& action, CPDFSDK_Document* pDocument);
};

class CPDFSDK_ActionHandler
{
public:
	CPDFSDK_ActionHandler(CPDFDoc_Environment* pEvi);

	void SetMediaActionHandler(CPDFSDK_MediaActionHandler* pHandler);

	bool		DoAction_DocOpen(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	bool		DoAction_JavaScript(const CPDF_Action& JsAction,CFX_WideString csJSName, CPDFSDK_Document* pDocument);
	bool		DoAction_Page(const CPDF_Action& action, enum CPDF_AAction::AActionType eType, CPDFSDK_Document* pDocument);
	bool		DoAction_Document(const CPDF_Action& action, enum CPDF_AAction::AActionType eType, CPDFSDK_Document* pDocument);
	bool		DoAction_BookMark(CPDF_Bookmark *pBookMark, const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument);
	bool		DoAction_Screen(const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument, CPDFSDK_Annot* pScreen);
	bool		DoAction_Link(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	bool		DoAction_Field(const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument,  CPDF_FormField* pFormField, PDFSDK_FieldAction& data);
	bool		DoAction_FieldJavaScript(const CPDF_Action& JsAction, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument, CPDF_FormField* pFormField, PDFSDK_FieldAction& data);

private:
	bool				ExecuteDocumentOpenAction(const CPDF_Action& action, CPDFSDK_Document* pDocument,  CFX_PtrList& list);
	bool				ExecuteDocumentPageAction(const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument,  CFX_PtrList& list);
	bool				ExecuteFieldAction(const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument,  CPDF_FormField* pFormField, PDFSDK_FieldAction& data, CFX_PtrList& list);
	bool				ExecuteScreenAction(const CPDF_Action& action, CPDF_AAction::AActionType type, CPDFSDK_Document* pDocument,  CPDFSDK_Annot* pScreen, CFX_PtrList& list);
	bool				ExecuteBookMark(const CPDF_Action& action, CPDFSDK_Document* pDocument,  CPDF_Bookmark* pBookmark, CFX_PtrList& list);
	bool				ExecuteLinkAction(const CPDF_Action& action, CPDFSDK_Document* pDocument,  CFX_PtrList& list);

	void				DoAction_NoJs(const CPDF_Action& action, CPDFSDK_Document* pDocument);
	void				RunDocumentPageJavaScript(CPDFSDK_Document* pDocument, CPDF_AAction::AActionType type, const CFX_WideString& script);
	void				RunDocumentOpenJavaScript(CPDFSDK_Document* pDocument, const CFX_WideString& sScriptName, const CFX_WideString& script);
	void				RunFieldJavaScript(CPDFSDK_Document* pDocument, CPDF_FormField* pFormField, CPDF_AAction::AActionType type, PDFSDK_FieldAction& data, const CFX_WideString& script);

	bool				IsValidField(CPDFSDK_Document* pDocument, CPDF_Dictionary* pFieldDict);
	bool				IsValidDocView(CPDFSDK_Document* pDocument);

	void				DoAction_GoTo(CPDFSDK_Document* pDocument,  const CPDF_Action& action);
	void				DoAction_GoToR(CPDFSDK_Document* pDocument, const CPDF_Action& action);
	void				DoAction_Launch(CPDFSDK_Document* pDocument, const CPDF_Action& action);
	void				DoAction_URI(CPDFSDK_Document* pDocument, const CPDF_Action& action);
	void				DoAction_Named(CPDFSDK_Document* pDocument, const CPDF_Action& action);
	void				DoAction_SetOCGState(CPDFSDK_Document* pDocument, const CPDF_Action& action);

    nonstd::unique_ptr<CPDFSDK_FormActionHandler> m_pFormActionHandler;
    CPDFSDK_MediaActionHandler* m_pMediaActionHandler;
};

#endif  // FPDFSDK_INCLUDE_FSDK_ACTIONHANDLER_H_
