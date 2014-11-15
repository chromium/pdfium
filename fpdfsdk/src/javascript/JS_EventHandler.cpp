// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_EventHandler.h"
//#include "../../include/javascript/JS_ResMgr.h"
#include "../../include/javascript/JS_Context.h"
#include "../../include/javascript/JS_Runtime.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/Document.h"
#include "../../include/javascript/Field.h"

/* ---------------------------- CJS_EventHandler ---------------------------- */

CJS_EventHandler::CJS_EventHandler(CJS_Context * pContext)	 :
	m_pJSContext(pContext),
	m_eEventType(JET_UNKNOWN),
	m_bValid(FALSE),
	m_pWideStrChange(NULL),
	m_nCommitKey(-1),
	m_bKeyDown(FALSE),
	m_bModifier(FALSE),
	m_bShift(FALSE),
	m_pISelEnd(NULL),
	m_nSelEndDu(0),
	m_pISelStart(NULL),
	m_nSelStartDu(0),
	m_bWillCommit(FALSE),
	m_pValue(NULL),
	m_bFieldFull(FALSE),
	m_pbRc(NULL),
	m_bRcDu(FALSE),
	m_pSourceDoc(NULL),
	m_pTargetBookMark(NULL),
	m_pTargetDoc(NULL),
	m_pTargetAnnot(NULL)
{
}

CJS_EventHandler::~CJS_EventHandler()
{
}

void CJS_EventHandler::OnApp_Init()
{
	Initial(JET_APP_INIT);
}

void CJS_EventHandler::OnDoc_Open(CPDFSDK_Document* pDoc, const CFX_WideString& strTargetName)
{
	Initial(JET_DOC_OPEN);

	m_pTargetDoc = pDoc;
	m_strTargetName = strTargetName;	
}

void CJS_EventHandler::OnDoc_WillPrint(CPDFSDK_Document* pDoc)
{
	Initial(JET_DOC_WILLPRINT);

	m_pTargetDoc = pDoc;	
}

void CJS_EventHandler::OnDoc_DidPrint(CPDFSDK_Document* pDoc)
{
	Initial(JET_DOC_DIDPRINT);
	
	m_pTargetDoc = pDoc;		
}

void CJS_EventHandler::OnDoc_WillSave(CPDFSDK_Document* pDoc)
{
	Initial(JET_DOC_WILLSAVE);
	m_pTargetDoc = pDoc;		
}

void CJS_EventHandler::OnDoc_DidSave(CPDFSDK_Document* pDoc)
{
	Initial(JET_DOC_DIDSAVE);	
	
	m_pTargetDoc = pDoc;	
}

void CJS_EventHandler::OnDoc_WillClose(CPDFSDK_Document* pDoc)
{
	Initial(JET_DOC_WILLCLOSE);

	m_pTargetDoc = pDoc;	
}

void CJS_EventHandler::OnPage_Open(CPDFSDK_Document* pDoc)
{
	Initial(JET_PAGE_OPEN);

	m_pTargetDoc = pDoc;
}

void CJS_EventHandler::OnPage_Close(CPDFSDK_Document* pDoc)
{
	Initial(JET_PAGE_CLOSE);
	
	m_pTargetDoc = pDoc;
}

void CJS_EventHandler::OnPage_InView(CPDFSDK_Document* pDoc)
{
	Initial(JET_PAGE_INVIEW);
	
	m_pTargetDoc = pDoc;
}

void CJS_EventHandler::OnPage_OutView(CPDFSDK_Document* pDoc)
{
	Initial(JET_PAGE_OUTVIEW);
	
	m_pTargetDoc = pDoc;
}

void CJS_EventHandler::OnField_MouseEnter(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget)
{
	Initial(JET_FIELD_MOUSEENTER);

	m_bModifier = bModifier;
	m_bShift = bShift;

	ASSERT(pTarget != NULL);
	m_strTargetName = pTarget->GetFullName();
}

void CJS_EventHandler::OnField_MouseExit(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget)
{
	Initial(JET_FIELD_MOUSEEXIT);

	m_bModifier = bModifier;
	m_bShift = bShift;
	ASSERT(pTarget != NULL);
	m_strTargetName = pTarget->GetFullName();	
}

void CJS_EventHandler::OnField_MouseDown(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget)
{
	Initial(JET_FIELD_MOUSEDOWN);
	m_eEventType = JET_FIELD_MOUSEDOWN;
	
	m_bModifier = bModifier;
	m_bShift = bShift;	
	ASSERT(pTarget != NULL);
	m_strTargetName = pTarget->GetFullName();
}

void CJS_EventHandler::OnField_MouseUp(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget)
{
	Initial(JET_FIELD_MOUSEUP);

	m_bModifier = bModifier;
	m_bShift = bShift;
	ASSERT(pTarget != NULL);
	m_strTargetName = pTarget->GetFullName();
}

void CJS_EventHandler::OnField_Focus(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget, 
									 const CFX_WideString& Value)
{
	Initial(JET_FIELD_FOCUS);

	m_bModifier = bModifier;
	m_bShift = bShift;  
	ASSERT(pTarget != NULL);
	m_strTargetName = pTarget->GetFullName();
	m_pValue = (CFX_WideString*)&Value;
}

void CJS_EventHandler::OnField_Blur(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget,
									const CFX_WideString& Value)
{
	Initial(JET_FIELD_BLUR);

	m_bModifier = bModifier;
	m_bShift = bShift;
	ASSERT(pTarget != NULL);
	m_strTargetName = pTarget->GetFullName();
	m_pValue = (CFX_WideString*)&Value;	
}

void CJS_EventHandler::OnField_Keystroke(int nCommitKey, CFX_WideString &strChange,
										 const CFX_WideString& strChangeEx, FX_BOOL KeyDown,
										 FX_BOOL bModifier, int& nSelEnd, int& nSelStart,
										 FX_BOOL bShift, CPDF_FormField* pTarget,
										 CFX_WideString& Value, FX_BOOL bWillCommit,
										  FX_BOOL bFieldFull, FX_BOOL& bRc)
{
	Initial(JET_FIELD_KEYSTROKE);
	
	m_nCommitKey = nCommitKey;
	m_pWideStrChange = &strChange;
	m_WideStrChangeEx = strChangeEx;
	m_bKeyDown = KeyDown;
	m_bModifier = bModifier;
	m_pISelEnd = &nSelEnd;
	m_pISelStart = &nSelStart;
	m_bShift = bShift;	
	ASSERT(pTarget != NULL);
	m_strTargetName = pTarget->GetFullName();
	m_pValue = &Value;
	m_bWillCommit = bWillCommit;	
	m_pbRc = &bRc;
	m_bFieldFull = bFieldFull;
}

void CJS_EventHandler::OnField_Validate(CFX_WideString& strChange, const CFX_WideString& strChangeEx,
										FX_BOOL bKeyDown, FX_BOOL bModifier, FX_BOOL bShift,
										CPDF_FormField* pTarget, CFX_WideString& Value, FX_BOOL& bRc)
{
	Initial(JET_FIELD_VALIDATE);
	
	m_pWideStrChange = &strChange;
	m_WideStrChangeEx = strChangeEx;	
	m_bKeyDown = bKeyDown;
	m_bModifier = bModifier;
	m_bShift = bShift;
	ASSERT(pTarget != NULL);
	m_strTargetName = pTarget->GetFullName();
	m_pValue = &Value;	
	m_pbRc = &bRc;	
}

void CJS_EventHandler::OnField_Calculate(CPDF_FormField* pSource, CPDF_FormField* pTarget, 
										 CFX_WideString& Value, FX_BOOL& bRc)
{
	Initial(JET_FIELD_CALCULATE);

	if (pSource)
		m_strSourceName = pSource->GetFullName();
	ASSERT(pTarget != NULL);
	m_strTargetName = pTarget->GetFullName();
    m_pValue = &Value;
	m_pbRc = &bRc;
}

void CJS_EventHandler::OnField_Format(int nCommitKey, CPDF_FormField* pTarget,
									  CFX_WideString& Value, FX_BOOL bWillCommit)
{
	Initial(JET_FIELD_FORMAT);
	
	m_nCommitKey = nCommitKey;	  
	ASSERT(pTarget != NULL);
	m_strTargetName = pTarget->GetFullName();
	m_pValue = &Value;
	m_bWillCommit = bWillCommit;
}

void CJS_EventHandler::OnScreen_Focus(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen)
{
	Initial(JET_SCREEN_FOCUS);

	m_bModifier = bModifier;
	m_bShift = bShift;
	m_pTargetAnnot = pScreen;
}

void CJS_EventHandler::OnScreen_Blur(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen)
{
	Initial(JET_SCREEN_BLUR);

	m_bModifier = bModifier;
	m_bShift = bShift;
	m_pTargetAnnot = pScreen;
}

void CJS_EventHandler::OnScreen_Open(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen)
{
	Initial(JET_SCREEN_OPEN);

	m_bModifier = bModifier;
	m_bShift = bShift;
	m_pTargetAnnot = pScreen;
}

void CJS_EventHandler::OnScreen_Close(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen)
{
	Initial(JET_SCREEN_CLOSE);

	m_bModifier = bModifier;
	m_bShift = bShift;
	m_pTargetAnnot = pScreen;
}

void CJS_EventHandler::OnScreen_MouseDown(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen)
{
	Initial(JET_SCREEN_MOUSEDOWN);

	m_bModifier = bModifier;
	m_bShift = bShift;
	m_pTargetAnnot = pScreen;
}

void CJS_EventHandler::OnScreen_MouseUp(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen)
{
	Initial(JET_SCREEN_MOUSEUP);

	m_bModifier = bModifier;
	m_bShift = bShift;
	m_pTargetAnnot = pScreen;
}

void CJS_EventHandler::OnScreen_MouseEnter(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen)
{
	Initial(JET_SCREEN_MOUSEENTER);

	m_bModifier = bModifier;
	m_bShift = bShift;
	m_pTargetAnnot = pScreen;
}

void CJS_EventHandler::OnScreen_MouseExit(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen)
{
	Initial(JET_SCREEN_MOUSEEXIT);

	m_bModifier = bModifier;
	m_bShift = bShift;
	m_pTargetAnnot = pScreen;
}

void CJS_EventHandler::OnScreen_InView(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen)
{
	Initial(JET_SCREEN_INVIEW);

	m_bModifier = bModifier;
	m_bShift = bShift;
	m_pTargetAnnot = pScreen;
}

void CJS_EventHandler::OnScreen_OutView(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen)
{
	Initial(JET_SCREEN_OUTVIEW);

	m_bModifier = bModifier;
	m_bShift = bShift;
	m_pTargetAnnot = pScreen;
}

void CJS_EventHandler::OnLink_MouseUp(CPDFSDK_Document* pTarget)
{
	Initial(JET_LINK_MOUSEUP);
	
	m_pTargetDoc = pTarget;	
}

void CJS_EventHandler::OnBookmark_MouseUp(CPDF_Bookmark* pBookMark)
{
	Initial(JET_BOOKMARK_MOUSEUP);

	m_pTargetBookMark = pBookMark;	
}

void CJS_EventHandler::OnMenu_Exec(CPDFSDK_Document* pTarget, const CFX_WideString& strTargetName)
{
	Initial(JET_MENU_EXEC);

	m_pTargetDoc = pTarget;
	m_strTargetName = strTargetName;
}

void CJS_EventHandler::OnExternal_Exec()
{
	Initial(JET_EXTERNAL_EXEC);
}

void CJS_EventHandler::OnBatchExec(CPDFSDK_Document* pTarget)
{
	Initial(JET_BATCH_EXEC);

	m_pTargetDoc = pTarget;	
}

void CJS_EventHandler::OnConsole_Exec()
{
	Initial(JET_CONSOLE_EXEC);
}


void CJS_EventHandler::Initial(JS_EVENT_T type)
{
	m_eEventType = type;

	m_strTargetName = L"";
	m_strSourceName = L"";
	m_pWideStrChange = NULL;
	m_WideStrChangeDu = L"";
	m_WideStrChangeEx = L"";
	m_nCommitKey = -1;
	m_bKeyDown = FALSE;
	m_bModifier = FALSE;
	m_bShift = FALSE;
	m_pISelEnd = NULL;
	m_nSelEndDu = 0;
	m_pISelStart = NULL;
	m_nSelStartDu = 0;
	m_bWillCommit = FALSE;
	m_pValue = NULL;
	m_bFieldFull = FALSE;
	m_pbRc = NULL;
	m_bRcDu = FALSE;

	m_pSourceDoc = NULL;
	m_pTargetBookMark = NULL;
	m_pTargetDoc = NULL;
	m_pTargetAnnot = NULL;

	m_bValid = TRUE;
}

void CJS_EventHandler::Destroy()
{
	m_bValid = FALSE;
}

FX_BOOL CJS_EventHandler::IsValid()
{
	return m_bValid;
}

CFX_WideString & CJS_EventHandler::Change()
{
	if (m_pWideStrChange != NULL)
		return *m_pWideStrChange;
	else
	{
		return m_WideStrChangeDu;
	}
}

CFX_WideString CJS_EventHandler::ChangeEx()
{
	return m_WideStrChangeEx;
}

int CJS_EventHandler::CommitKey()
{
	return m_nCommitKey;
}

FX_BOOL CJS_EventHandler::FieldFull()
{
	return m_bFieldFull;
}

FX_BOOL CJS_EventHandler::KeyDown()
{
	return m_bKeyDown;
}

FX_BOOL CJS_EventHandler::Modifier()
{
	return m_bModifier;
}

FX_LPCWSTR CJS_EventHandler::Name()
{
	switch (m_eEventType)
	{
	case JET_APP_INIT:			return L"Init";
	case JET_BATCH_EXEC:		return L"Exec";
	case JET_BOOKMARK_MOUSEUP:	return L"Mouse Up";
	case JET_CONSOLE_EXEC:		return L"Exec";
	case JET_DOC_DIDPRINT:		return L"DidPrint";
	case JET_DOC_DIDSAVE:		return L"DidSave";
	case JET_DOC_OPEN:			return L"Open";
	case JET_DOC_WILLCLOSE:		return L"WillClose";
	case JET_DOC_WILLPRINT:		return L"WillPrint";
	case JET_DOC_WILLSAVE:		return L"WillSave";
	case JET_EXTERNAL_EXEC:		return L"Exec";
	case JET_FIELD_FOCUS:		
	case JET_SCREEN_FOCUS:		return L"Focus";
	case JET_FIELD_BLUR:		
	case JET_SCREEN_BLUR:		return L"Blur";
	case JET_FIELD_MOUSEDOWN:
	case JET_SCREEN_MOUSEDOWN:	return L"Mouse Down";
	case JET_FIELD_MOUSEUP:		
	case JET_SCREEN_MOUSEUP:	return L"Mouse Up";
	case JET_FIELD_MOUSEENTER:
	case JET_SCREEN_MOUSEENTER:	return L"Mouse Enter";
	case JET_FIELD_MOUSEEXIT:
	case JET_SCREEN_MOUSEEXIT:	return L"Mouse Exit";
	case JET_FIELD_CALCULATE:	return L"Calculate";
	case JET_FIELD_FORMAT:		return L"Format";
	case JET_FIELD_KEYSTROKE:	return L"Keystroke";
	case JET_FIELD_VALIDATE:	return L"Validate";
	case JET_LINK_MOUSEUP:		return L"Mouse Up";
	case JET_MENU_EXEC:			return L"Exec";
	case JET_PAGE_OPEN:		
	case JET_SCREEN_OPEN:		return L"Open";
	case JET_PAGE_CLOSE:
	case JET_SCREEN_CLOSE:		return L"Close";
	case JET_SCREEN_INVIEW:	
	case JET_PAGE_INVIEW:		return L"InView";
	case JET_PAGE_OUTVIEW:
	case JET_SCREEN_OUTVIEW:	return L"OutView";
	default:
		return L"";
	}

	return L"";
}

FX_LPCWSTR CJS_EventHandler::Type()
{
	switch (m_eEventType)
	{
	case JET_APP_INIT:			return L"App";
	case JET_BATCH_EXEC:		return L"Batch";
	case JET_BOOKMARK_MOUSEUP:	return L"BookMark";	
	case JET_CONSOLE_EXEC:		return L"Console";
	case JET_DOC_DIDPRINT:
	case JET_DOC_DIDSAVE:
	case JET_DOC_OPEN:
	case JET_DOC_WILLCLOSE:
	case JET_DOC_WILLPRINT:
	case JET_DOC_WILLSAVE:		return L"Doc";
	case JET_EXTERNAL_EXEC:		return L"External";
	case JET_FIELD_BLUR:
	case JET_FIELD_FOCUS:
	case JET_FIELD_MOUSEDOWN:
	case JET_FIELD_MOUSEENTER:
	case JET_FIELD_MOUSEEXIT:
	case JET_FIELD_MOUSEUP:
	case JET_FIELD_CALCULATE:
	case JET_FIELD_FORMAT:
	case JET_FIELD_KEYSTROKE:
	case JET_FIELD_VALIDATE:	return L"Field";
	case JET_SCREEN_FOCUS:
	case JET_SCREEN_BLUR:
	case JET_SCREEN_OPEN:
	case JET_SCREEN_CLOSE:
	case JET_SCREEN_MOUSEDOWN:
	case JET_SCREEN_MOUSEUP:
	case JET_SCREEN_MOUSEENTER:
	case JET_SCREEN_MOUSEEXIT:
	case JET_SCREEN_INVIEW:
	case JET_SCREEN_OUTVIEW:	return L"Screen";
	case JET_LINK_MOUSEUP:		return L"Link";	
	case JET_MENU_EXEC:			return L"Menu";
	case JET_PAGE_OPEN:
	case JET_PAGE_CLOSE:
	case JET_PAGE_INVIEW:
	case JET_PAGE_OUTVIEW:return L"Page";
	default:
		return L"";
	}

	return L"";
}

FX_BOOL& CJS_EventHandler::Rc()
{
	if (m_pbRc != NULL)
		return *m_pbRc;
	else
	{	    
		return m_bRcDu;
	}
}

int & CJS_EventHandler::SelEnd()
{
	if (m_pISelEnd != NULL)
	{
		return *m_pISelEnd;
	}
	else
	{
		return m_nSelEndDu;
	}
}

int & CJS_EventHandler::SelStart()
{
	if (m_pISelStart != NULL)
		return * m_pISelStart;
	else
	{
		return m_nSelStartDu;
	}
}

FX_BOOL CJS_EventHandler::Shift()
{
	return m_bShift;
}

Field* CJS_EventHandler::Source()
{
	ASSERT(m_pJSContext != NULL);

	CJS_Runtime* pRuntime = m_pJSContext->GetJSRuntime();

	JSFXObject  pDocObj = JS_NewFxDynamicObj(*pRuntime, m_pJSContext, JS_GetObjDefnID(*pRuntime, L"Document"));
	ASSERT(pDocObj.IsEmpty() == FALSE);
	JSFXObject  pFieldObj = JS_NewFxDynamicObj(*pRuntime, m_pJSContext, JS_GetObjDefnID(*pRuntime, L"Field"));
	ASSERT(pFieldObj.IsEmpty() == FALSE);

	CJS_Document* pJSDocument = (CJS_Document*)JS_GetPrivate(pDocObj);
	ASSERT(pJSDocument != NULL);
	Document* pDocument = (Document*)pJSDocument->GetEmbedObject();
	ASSERT(pDocument != NULL);
 	if (m_pTargetDoc != NULL)
 		pDocument->AttachDoc(m_pTargetDoc);
 	else
 		pDocument->AttachDoc(m_pJSContext->GetReaderDocument());
	
	//if (m_pSourceField == NULL)
	//	return NULL;
	//CRAO_Widget *pWidget = IBCL_Widget::GetWidget(m_pSourceField);
	//CPDF_FormField* pFormField = pWidget->GetFormField();
	//ASSERT(pFormField);
	//CFX_WideString csFieldName = pFormField->GetFullName();
	CJS_Field * pJSField = (CJS_Field*)JS_GetPrivate(pFieldObj);
	ASSERT(pJSField != NULL);
	Field * pField = (Field *)pJSField->GetEmbedObject(); 
	ASSERT(pField != NULL);
	pField->AttachField(pDocument, m_strSourceName);
	return pField;	
}

Field* CJS_EventHandler::Target_Field()
{
	ASSERT(m_pJSContext != NULL);

	CJS_Runtime* pRuntime = m_pJSContext->GetJSRuntime();

	JSFXObject pDocObj = JS_NewFxDynamicObj(*pRuntime, m_pJSContext, JS_GetObjDefnID(*pRuntime, L"Document"));
	ASSERT(pDocObj.IsEmpty() == FALSE);
	JSFXObject pFieldObj = JS_NewFxDynamicObj(*pRuntime, m_pJSContext, JS_GetObjDefnID(*pRuntime, L"Field"));
	ASSERT(pFieldObj.IsEmpty() == FALSE);

	CJS_Document* pJSDocument = (CJS_Document*)JS_GetPrivate(pDocObj);
	ASSERT(pJSDocument != NULL);
	Document* pDocument = (Document*)pJSDocument->GetEmbedObject();
 	ASSERT(pDocument != NULL);
 	if (m_pTargetDoc != NULL)
 		pDocument->AttachDoc(m_pTargetDoc);
 	else
 		pDocument->AttachDoc(m_pJSContext->GetReaderDocument());
	
	CJS_Field* pJSField = (CJS_Field*)JS_GetPrivate(pFieldObj);
	ASSERT(pJSField != NULL);

	Field* pField = (Field *)pJSField->GetEmbedObject(); 
	ASSERT(pField != NULL);

	pField->AttachField(pDocument, m_strTargetName);
	return pField;	
}

CFX_WideString& CJS_EventHandler::Value()
{
	return *m_pValue;
}

FX_BOOL CJS_EventHandler::WillCommit()
{
	return m_bWillCommit;
}

CFX_WideString CJS_EventHandler::TargetName()
{
	return m_strTargetName;
}


