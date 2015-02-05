// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
// #include "../../include/javascript/JS_MsgBox.h"
// #include "../../include/javascript/JS_ResMgr.h"
#include "../../include/javascript/JS_Context.h"

JS_TIMER_MAPARRAY& GetTimeMap()
{
  // Leak the timer array at shutdown.
  static auto* timeMap = new JS_TIMER_MAPARRAY;
  return *timeMap;
}

int FXJS_MsgBox(CPDFDoc_Environment* pApp, CPDFSDK_PageView* pPageView, FX_LPCWSTR swMsg, FX_LPCWSTR swTitle, FX_UINT nType, FX_UINT nIcon)
{
	int nRet = 0;

	if(pApp)
	{
		CPDFSDK_Document* pDoc = pApp->GetCurrentDoc();
		if(pDoc)
			pDoc->KillFocusAnnot();
		nRet = pApp->JS_appAlert(swMsg, swTitle, nType, nIcon);
	}

	return nRet;
}

CPDFSDK_PageView* FXJS_GetPageView(IFXJS_Context* cc)
{
	if (CJS_Context* pContext = (CJS_Context *)cc)
	{
		if (pContext->GetReaderDocument())
			return NULL;
	}
	return NULL;
}

/* ---------------------------------  CJS_EmbedObj --------------------------------- */

CJS_EmbedObj::CJS_EmbedObj(CJS_Object* pJSObject) : 
	m_pJSObject(pJSObject)
{
}

CJS_EmbedObj::~CJS_EmbedObj()
{
	m_pJSObject = NULL;

}

CPDFSDK_PageView* CJS_EmbedObj::JSGetPageView(IFXJS_Context* cc)
{
	return FXJS_GetPageView(cc);
}

int CJS_EmbedObj::MsgBox(CPDFDoc_Environment* pApp, CPDFSDK_PageView* pPageView,FX_LPCWSTR swMsg,FX_LPCWSTR swTitle,FX_UINT nType,FX_UINT nIcon)
{
	return FXJS_MsgBox(pApp, pPageView, swMsg, swTitle, nType, nIcon);
}

void CJS_EmbedObj::Alert(CJS_Context* pContext, FX_LPCWSTR swMsg)
{
	CJS_Object::Alert(pContext, swMsg);
}

CJS_Timer* CJS_EmbedObj::BeginTimer(CPDFDoc_Environment * pApp,FX_UINT nElapse)
{
	CJS_Timer* pTimer = new CJS_Timer(this,pApp);
	pTimer->SetJSTimer(nElapse);
	
	return pTimer;
}

void CJS_EmbedObj::EndTimer(CJS_Timer* pTimer)
{
	ASSERT(pTimer != NULL);
	pTimer->KillJSTimer();
	delete pTimer;
}

/* ---------------------------------  CJS_Object --------------------------------- */
void  FreeObject(const v8::WeakCallbackData<v8::Object, CJS_Object>& data)
{
	CJS_Object* pJSObj  = data.GetParameter();
	if(pJSObj)
	{
		pJSObj->ExitInstance();
		delete pJSObj;
	}
	v8::Local<v8::Object> obj = data.GetValue();
	JS_FreePrivate(obj);
}

CJS_Object::CJS_Object(JSFXObject pObject) :m_pEmbedObj(NULL)
{
	v8::Local<v8::Context> context = pObject->CreationContext();
	m_pIsolate = context->GetIsolate();
	m_pObject.Reset(m_pIsolate, pObject);
};

CJS_Object::~CJS_Object(void)
{
	delete m_pEmbedObj;
	m_pEmbedObj = NULL;

	m_pObject.Reset();
};

void	CJS_Object::MakeWeak()
{
	m_pObject.SetWeak(this, FreeObject);
}

CPDFSDK_PageView* CJS_Object::JSGetPageView(IFXJS_Context* cc)
{
	return FXJS_GetPageView(cc);
}

int CJS_Object::MsgBox(CPDFDoc_Environment* pApp, CPDFSDK_PageView* pPageView, FX_LPCWSTR swMsg, FX_LPCWSTR swTitle, FX_UINT nType, FX_UINT nIcon)
{
	return FXJS_MsgBox(pApp, pPageView, swMsg, swTitle, nType, nIcon);
}

void CJS_Object::Alert(CJS_Context* pContext, FX_LPCWSTR swMsg)
{
	ASSERT(pContext != NULL);

	if (pContext->IsMsgBoxEnabled())
	{
		CPDFDoc_Environment* pApp = pContext->GetReaderApp();
		if(pApp)
			pApp->JS_appAlert(swMsg, NULL, 0, 3);
	}
}


