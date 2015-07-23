// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
//#include "../../include/javascript/JS_ResMgr.h"
#include "../../include/javascript/JS_Context.h"
#include "../../include/javascript/JS_EventHandler.h"
#include "../../include/javascript/JS_Runtime.h"
#include "../../include/javascript/resource.h"

/* -------------------------- CJS_Context -------------------------- */

CJS_Context::CJS_Context(CJS_Runtime* pRuntime) :
	m_pRuntime(pRuntime),
	m_bBusy(false),
	m_bMsgBoxEnable(true)
{
	m_pEventHandler = new CJS_EventHandler(this);
}

CJS_Context::~CJS_Context(void)
{
    delete m_pEventHandler;
}

CPDFSDK_Document* CJS_Context::GetReaderDocument()
{
	ASSERT(m_pRuntime != NULL);

	return m_pRuntime->GetReaderDocument();
}

CPDFDoc_Environment* CJS_Context::GetReaderApp()
{
	ASSERT(m_pRuntime != NULL);

	return m_pRuntime->GetReaderApp();
}

bool CJS_Context::DoJob(int nMode, const CFX_WideString& script, CFX_WideString& info)
{
	if (m_bBusy)
	{
		info = JSGetStringFromID(this, IDS_STRING_JSBUSY);
		return false;
	}

	m_bBusy = true;

	ASSERT(m_pRuntime != NULL);
	ASSERT(m_pEventHandler != NULL);
	ASSERT(m_pEventHandler->IsValid());

	if (!m_pRuntime->AddEventToLoop(m_pEventHandler->TargetName(), m_pEventHandler->EventType()))
	{
		info = JSGetStringFromID(this, IDS_STRING_JSEVENT);
		return false;
	}

	FXJSErr error ={NULL,NULL, 0};
	int nRet = 0;

	if (script.GetLength() > 0)
	{
		if (nMode == 0)
		{
			nRet = JS_Execute(*m_pRuntime, this, script.c_str(), script.GetLength(), &error);
		}
		else
		{
			nRet = JS_Parse(*m_pRuntime, this, script.c_str(), script.GetLength(), &error);
		}
	}

	if (nRet < 0)
	{
		CFX_WideString sLine;
		sLine.Format(L"[ Line: %05d { %s } ] : %s",error.linnum-1,error.srcline,error.message);

//			TRACE(L"/* -------------- JS Error -------------- */\n");
//			TRACE(sLine);
//			TRACE(L"\n");
		//CFX_ByteString sTemp = CFX_ByteString::FromUnicode(error.message);
		info += sLine;
	}
	else
	{
		info = JSGetStringFromID(this, IDS_STRING_RUN);
	}

	m_pRuntime->RemoveEventInLoop(m_pEventHandler->TargetName(), m_pEventHandler->EventType());

	m_pEventHandler->Destroy();
	m_bBusy = false;

	return nRet >= 0;
}

bool CJS_Context::RunScript(const CFX_WideString& script, CFX_WideString& info)
{
	v8::Isolate::Scope isolate_scope(m_pRuntime->GetIsolate());
	v8::HandleScope handle_scope(m_pRuntime->GetIsolate());
	v8::Local<v8::Context> context = m_pRuntime->NewJSContext();
	v8::Context::Scope context_scope(context);

	return DoJob(0, script, info);
}

bool CJS_Context::Compile(const CFX_WideString& script, CFX_WideString& info)
{
	v8::Isolate::Scope isolate_scope(m_pRuntime->GetIsolate());
	v8::HandleScope handle_scope(m_pRuntime->GetIsolate());
	v8::Local<v8::Context> context = m_pRuntime->NewJSContext();
	v8::Context::Scope context_scope(context);

	return DoJob(1, script, info);
}

void CJS_Context::OnApp_Init()
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnApp_Init();
}

void CJS_Context::OnDoc_Open(CPDFSDK_Document* pDoc, const CFX_WideString &strTargetName)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnDoc_Open(pDoc,strTargetName);
}

void CJS_Context::OnDoc_WillPrint(CPDFSDK_Document* pDoc)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnDoc_WillPrint(pDoc);
}

void CJS_Context::OnDoc_DidPrint(CPDFSDK_Document* pDoc)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnDoc_DidPrint(pDoc);
}

void CJS_Context::OnDoc_WillSave(CPDFSDK_Document* pDoc)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnDoc_WillSave(pDoc);
}

void CJS_Context::OnDoc_DidSave(CPDFSDK_Document* pDoc)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnDoc_DidSave(pDoc);
}

void CJS_Context::OnDoc_WillClose(CPDFSDK_Document* pDoc)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnDoc_WillClose(pDoc);
}

void CJS_Context::OnPage_Open(CPDFSDK_Document* pTarget)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnPage_Open(pTarget);
}

void CJS_Context::OnPage_Close(CPDFSDK_Document* pTarget)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnPage_Close(pTarget);
}

void CJS_Context::OnPage_InView(CPDFSDK_Document* pTarget)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnPage_InView(pTarget);
}

void CJS_Context::OnPage_OutView(CPDFSDK_Document* pTarget)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnPage_OutView(pTarget);
}

void CJS_Context::OnField_MouseDown(bool bModifier, bool bShift, CPDF_FormField *pTarget)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnField_MouseDown(bModifier, bShift, pTarget);
}

void CJS_Context::OnField_MouseEnter(bool bModifier, bool bShift, CPDF_FormField *pTarget)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnField_MouseEnter(bModifier, bShift, pTarget);
}

void CJS_Context::OnField_MouseExit(bool bModifier, bool bShift, CPDF_FormField *pTarget)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnField_MouseExit(bModifier, bShift, pTarget);
}

void CJS_Context::OnField_MouseUp(bool bModifier, bool bShift, CPDF_FormField *pTarget)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnField_MouseUp(bModifier, bShift, pTarget);
}

void CJS_Context::OnField_Focus(bool bModifier, bool bShift, CPDF_FormField* pTarget, const CFX_WideString& Value)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnField_Focus(bModifier, bShift, pTarget, Value);
}

void CJS_Context::OnField_Blur(bool bModifier, bool bShift, CPDF_FormField* pTarget, const CFX_WideString& Value)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnField_Blur(bModifier, bShift, pTarget, Value);
}

void CJS_Context::OnField_Calculate(CPDF_FormField* pSource, CPDF_FormField* pTarget, CFX_WideString& Value, bool& bRc)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnField_Calculate(pSource, pTarget, Value, bRc);
}

void CJS_Context::OnField_Format(CPDF_FormField* pTarget, CFX_WideString& Value, bool bWillCommit)
{
    m_pEventHandler->OnField_Format(pTarget, Value, bWillCommit);
}


void CJS_Context::OnField_Keystroke(CFX_WideString& strChange, const CFX_WideString& strChangeEx,
									bool bKeyDown, bool bModifier, int &nSelEnd,int &nSelStart,
									bool bShift, CPDF_FormField* pTarget, CFX_WideString& Value,
									bool bWillCommit, bool bFieldFull, bool& bRc)
{
    m_pEventHandler->OnField_Keystroke(
        strChange, strChangeEx, bKeyDown, bModifier, nSelEnd, nSelStart,
        bShift, pTarget, Value, bWillCommit, bFieldFull, bRc);
}

void CJS_Context::OnField_Validate(CFX_WideString& strChange,const CFX_WideString& strChangeEx,
								   bool bKeyDown, bool bModifier, bool bShift, CPDF_FormField* pTarget,
								   CFX_WideString& Value, bool& bRc)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnField_Validate(strChange, strChangeEx, bKeyDown, bModifier, bShift, pTarget, Value, bRc);
}

void CJS_Context::OnScreen_Focus(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnScreen_Focus(bModifier, bShift, pScreen);
}

void CJS_Context::OnScreen_Blur(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnScreen_Blur(bModifier, bShift, pScreen);
}

void CJS_Context::OnScreen_Open(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnScreen_Open(bModifier, bShift, pScreen);
}

void CJS_Context::OnScreen_Close(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnScreen_Close(bModifier, bShift, pScreen);
}

void CJS_Context::OnScreen_MouseDown(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnScreen_MouseDown(bModifier, bShift, pScreen);
}

void CJS_Context::OnScreen_MouseUp(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnScreen_MouseUp(bModifier, bShift, pScreen);
}

void CJS_Context::OnScreen_MouseEnter(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnScreen_MouseEnter(bModifier, bShift, pScreen);
}

void CJS_Context::OnScreen_MouseExit(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnScreen_MouseExit(bModifier, bShift, pScreen);
}

void CJS_Context::OnScreen_InView(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnScreen_InView(bModifier, bShift, pScreen);
}

void CJS_Context::OnScreen_OutView(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnScreen_OutView(bModifier, bShift, pScreen);
}

void CJS_Context::OnBookmark_MouseUp(CPDF_Bookmark* pBookMark)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnBookmark_MouseUp(pBookMark);
}

void CJS_Context::OnLink_MouseUp(CPDFSDK_Document* pTarget)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnLink_MouseUp(pTarget);
}

void CJS_Context::OnConsole_Exec()
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnConsole_Exec();
}

void CJS_Context::OnExternal_Exec()
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnExternal_Exec();
}

void CJS_Context::OnBatchExec(CPDFSDK_Document* pTarget)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnBatchExec(pTarget);
}

void CJS_Context::OnMenu_Exec(CPDFSDK_Document* pTarget,const CFX_WideString& strTargetName)
{
	ASSERT(m_pEventHandler != NULL);
	m_pEventHandler->OnMenu_Exec(pTarget, strTargetName);
}

