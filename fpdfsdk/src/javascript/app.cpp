// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/app.h"
#include "../../include/javascript/JS_EventHandler.h"
#include "../../include/javascript/resource.h"
#include "../../include/javascript/JS_Context.h"
#include "../../include/javascript/JS_Runtime.h"
#include "../../include/javascript/Document.h"


static v8::Isolate* GetIsolate(IFXJS_Context* cc)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);

	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	return pRuntime->GetIsolate();
}

/* ---------------------------- TimerObj ---------------------------- */

BEGIN_JS_STATIC_CONST(CJS_TimerObj)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_TimerObj)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_TimerObj)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_TimerObj, TimerObj)

TimerObj::TimerObj(CJS_Object* pJSObject)
: CJS_EmbedObj(pJSObject),
m_pTimer(NULL)
{

}

TimerObj::~TimerObj()
{
}

void TimerObj::SetTimer(CJS_Timer* pTimer)
{
	m_pTimer = pTimer;
}

CJS_Timer* TimerObj::GetTimer() const
{
	return m_pTimer;
}

#define JS_STR_VIEWERTYPE_READER		L"Reader"
#define JS_STR_VIEWERTYPE_STANDARD		L"Exchange"
#define JS_STR_VIEWERVARIATION			L"Full"
#define JS_STR_PLATFORM					L"WIN"
#define JS_STR_LANGUANGE				L"ENU"
#define JS_STR_VIEWERVERSION			8
#define JS_NUM_FORMSVERSION				7

#define JS_FILEPATH_MAXLEN				2000

/* ---------------------------- app ---------------------------- */

BEGIN_JS_STATIC_CONST(CJS_App)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_App)
	JS_STATIC_PROP_ENTRY(activeDocs)
	JS_STATIC_PROP_ENTRY(calculate)
	JS_STATIC_PROP_ENTRY(formsVersion)
	JS_STATIC_PROP_ENTRY(fs)
	JS_STATIC_PROP_ENTRY(fullscreen)
	JS_STATIC_PROP_ENTRY(language)
	JS_STATIC_PROP_ENTRY(media)
	JS_STATIC_PROP_ENTRY(platform)
	JS_STATIC_PROP_ENTRY(runtimeHighlight)
	JS_STATIC_PROP_ENTRY(viewerType)
	JS_STATIC_PROP_ENTRY(viewerVariation)
	JS_STATIC_PROP_ENTRY(viewerVersion)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_App)
	JS_STATIC_METHOD_ENTRY(alert)
	JS_STATIC_METHOD_ENTRY(beep)
	JS_STATIC_METHOD_ENTRY(browseForDoc)
	JS_STATIC_METHOD_ENTRY(clearInterval)
	JS_STATIC_METHOD_ENTRY(clearTimeOut)
	JS_STATIC_METHOD_ENTRY(execDialog)
	JS_STATIC_METHOD_ENTRY(execMenuItem)
	JS_STATIC_METHOD_ENTRY(findComponent)
	JS_STATIC_METHOD_ENTRY(goBack)
	JS_STATIC_METHOD_ENTRY(goForward)
	JS_STATIC_METHOD_ENTRY(launchURL)
	JS_STATIC_METHOD_ENTRY(mailMsg)
	JS_STATIC_METHOD_ENTRY(newFDF)
	JS_STATIC_METHOD_ENTRY(newDoc)
	JS_STATIC_METHOD_ENTRY(openDoc)
	JS_STATIC_METHOD_ENTRY(openFDF)
	JS_STATIC_METHOD_ENTRY(popUpMenuEx)
	JS_STATIC_METHOD_ENTRY(popUpMenu)
	JS_STATIC_METHOD_ENTRY(response)
	JS_STATIC_METHOD_ENTRY(setInterval)
	JS_STATIC_METHOD_ENTRY(setTimeOut)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_App,app)

app::app(CJS_Object * pJSObject) : CJS_EmbedObj(pJSObject) ,
	m_bCalculate(true),
	m_bRuntimeHighLight(false)
//	m_pMenuHead(NULL)
{
}

app::~app(void)
{
	for (int i=0,sz=m_aTimer.GetSize(); i<sz; i++)
		delete m_aTimer[i];

	m_aTimer.RemoveAll();
}

FX_BOOL app::activeDocs(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (vp.IsGetting())
	{

		CJS_Context* pContext = (CJS_Context *)cc;
		ASSERT(pContext != NULL);

		CPDFDoc_Environment* pApp = pContext->GetReaderApp();
		ASSERT(pApp != NULL);

		CJS_Runtime* pRuntime = pContext->GetJSRuntime();
		ASSERT(pRuntime != NULL);

		CPDFSDK_Document* pCurDoc = pContext->GetReaderDocument();

		CJS_Array aDocs(pRuntime->GetIsolate());
//		int iNumDocs = pApp->CountDocuments();

// 		for(int iIndex = 0; iIndex<iNumDocs; iIndex++)
// 		{
			CPDFSDK_Document* pDoc = pApp->GetCurrentDoc();
			if (pDoc)
			{
				CJS_Document * pJSDocument = NULL;

				if (pDoc == pCurDoc)
				{
					JSFXObject pObj = JS_GetThisObj(*pRuntime);

					if (JS_GetObjDefnID(pObj) == JS_GetObjDefnID(*pRuntime, L"Document"))
					{
						pJSDocument = (CJS_Document*)JS_GetPrivate(pRuntime->GetIsolate(),pObj);
					}
				}
				else
				{
					JSFXObject pObj = JS_NewFxDynamicObj(*pRuntime, pContext, JS_GetObjDefnID(*pRuntime,L"Document"));
					pJSDocument = (CJS_Document*)JS_GetPrivate(pRuntime->GetIsolate(),pObj);
					ASSERT(pJSDocument != NULL);


					//			pDocument->AttachDoc(pDoc);
				}

				aDocs.SetElement(0,CJS_Value(pRuntime->GetIsolate(),pJSDocument));
			}
	//		}

		if (aDocs.GetLength() > 0)
			vp << aDocs;
		else
			vp.SetNull();
		return TRUE;
	}
	return FALSE;
}

FX_BOOL app::calculate(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (vp.IsSetting())
	{
		bool bVP;
		vp >> bVP;
		m_bCalculate = (FX_BOOL)bVP;

		CJS_Context* pContext = (CJS_Context*)cc;
		ASSERT(pContext != NULL);

		CPDFDoc_Environment* pApp = pContext->GetReaderApp();
		ASSERT(pApp != NULL);

		CJS_Runtime* pRuntime = pContext->GetJSRuntime();
		ASSERT(pRuntime != NULL);

		CJS_Array aDocs(pRuntime->GetIsolate());
		if (CPDFSDK_Document* pDoc = pApp->GetCurrentDoc())
		{
			CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pDoc->GetInterForm();
			ASSERT(pInterForm != NULL);
			pInterForm->EnableCalculate((FX_BOOL)m_bCalculate);
		}
	}
	else
	{
		vp << (bool)m_bCalculate;
	}

	return TRUE;
}

FX_BOOL app::formsVersion(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (vp.IsGetting())
	{
		vp << JS_NUM_FORMSVERSION;
		return TRUE;
	}

	return FALSE;
}

FX_BOOL app::viewerType(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (vp.IsGetting())
	{
		vp << L"unknown";
		return TRUE;
	}

	return FALSE;
}

FX_BOOL app::viewerVariation(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (vp.IsGetting())
	{
		vp << JS_STR_VIEWERVARIATION;
		return TRUE;
	}

	return FALSE;
}

FX_BOOL app::viewerVersion(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (vp.IsGetting())
	{
		vp << JS_STR_VIEWERVERSION;
		return TRUE;
	}

	return FALSE;
}

FX_BOOL app::platform(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (vp.IsGetting())
	{
		vp << JS_STR_PLATFORM;
		return TRUE;
	}

	return FALSE;
}

FX_BOOL app::language(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (vp.IsGetting())
	{
		vp << JS_STR_LANGUANGE;
		return TRUE;
	}

	return FALSE;
}

//creates a new fdf object that contains no data
//comment: need reader support
//note:
//CFDF_Document * CPDFDoc_Environment::NewFDF();
FX_BOOL app::newFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return TRUE;
}
//opens a specified pdf document and returns its document object
//comment:need reader support
//note: as defined in js reference, the proto of this function's fourth parmeters, how old an fdf document while do not show it.
//CFDF_Document * CPDFDoc_Environment::OpenFDF(string strPath,bool bUserConv);

FX_BOOL app::openFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return TRUE;
}

FX_BOOL app::alert(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	int iSize = params.size();
	if (iSize < 1)
		return FALSE;

	CFX_WideString swMsg = L"";
	CFX_WideString swTitle = L"";
	int iIcon = 0;
	int iType = 0;

	v8::Isolate* isolate = GetIsolate(cc);

	if (iSize == 1)
	{
		if (params[0].GetType() == VT_object)
		{
			JSObject pObj = params[0].ToV8Object();
			{
				v8::Local<v8::Value> pValue = JS_GetObjectElement(isolate, pObj, L"cMsg");
				swMsg = CJS_Value(isolate, pValue, VT_unknown).ToCFXWideString();

				pValue = JS_GetObjectElement(isolate, pObj, L"cTitle");
				swTitle = CJS_Value(isolate, pValue, VT_unknown).ToCFXWideString();

				pValue = JS_GetObjectElement(isolate, pObj, L"nIcon");
				iIcon = CJS_Value(isolate, pValue, VT_unknown).ToInt();

				pValue = JS_GetObjectElement(isolate, pObj, L"nType");
				iType = CJS_Value(isolate, pValue, VT_unknown).ToInt();
			}

			if (swMsg == L"")
			{
				CJS_Array carray(isolate);
				if (params[0].ConvertToArray(carray))
				{
					int iLenth = carray.GetLength();
					CJS_Value* pValue = new CJS_Value(isolate);
//					if (iLenth == 1)
//						pValue = new CJS_Value(isolate);
//					else if (iLenth > 1)
//						pValue = new CJS_Value[iLenth];

					for(int i = 0; i < iLenth; i++)
					{
						carray.GetElement(i, *pValue);
						swMsg += (*pValue).ToCFXWideString();
						if (i < iLenth - 1)
							swMsg += L",  ";
					}

					if(pValue) delete pValue;
				}
			}

			if (swTitle == L"")
				swTitle = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSALERT);
		}
		else if (params[0].GetType() == VT_boolean)
		{
			FX_BOOL bGet = params[0].ToBool();
			if (bGet)
				swMsg = L"true";
			else
				swMsg = L"false";

			swTitle = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSALERT);
		}
		else
		{
			swMsg = params[0].ToCFXWideString();
			swTitle = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSALERT);
		}
	}
	else
	{
		if (params[0].GetType() == VT_boolean)
		{
			FX_BOOL bGet = params[0].ToBool();
			if (bGet)
				swMsg = L"true";
			else
				swMsg = L"false";
		}
		else
		{
			swMsg = params[0].ToCFXWideString();
		}
		swTitle = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSALERT);

		for(int i = 1;i<iSize;i++)
		{
			if (i == 1)
				iIcon = params[i].ToInt();
			if (i == 2)
				iType = params[i].ToInt();
			if (i == 3)
				swTitle = params[i].ToCFXWideString();
		}
	}


	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);
	pRuntime->BeginBlock();
	vRet = MsgBox(pRuntime->GetReaderApp(), JSGetPageView(cc), swMsg.c_str(), swTitle.c_str(), iType, iIcon);
	pRuntime->EndBlock();

	return TRUE;
}


FX_BOOL app::beep(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	if (params.size() == 1)
	{
		CJS_Context* pContext = (CJS_Context*)cc;
		CJS_Runtime* pRuntime = pContext->GetJSRuntime();
		CPDFDoc_Environment * pEnv = pRuntime->GetReaderApp();
		pEnv->JS_appBeep(params[0].ToInt());
		return TRUE;
	}

	sError = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSPARAMERROR);
	return FALSE;
}

FX_BOOL app::findComponent(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return TRUE;
}

FX_BOOL app::popUpMenuEx(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return FALSE;
}

FX_BOOL app::fs(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	return FALSE;
}

FX_BOOL app::setInterval(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	if (params.size() > 2 || params.size() == 0)
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	CFX_WideString script = params.size() > 0 ?  params[0].ToCFXWideString() : L"";
	if (script.IsEmpty())
	{
		sError = JSGetStringFromID(pContext, IDS_STRING_JSAFNUMBER_KEYSTROKE);
		return TRUE;
	}

	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	FX_DWORD dwInterval = params.size() > 1 ? params[1].ToInt() : 1000;

	CPDFDoc_Environment* pApp = pRuntime->GetReaderApp();
	ASSERT(pApp);
	CJS_Timer* pTimer = new CJS_Timer(this, pApp);
	m_aTimer.Add(pTimer);

	pTimer->SetType(0);
	pTimer->SetRuntime(pRuntime);
	pTimer->SetJScript(script);
	pTimer->SetTimeOut(0);
//	pTimer->SetStartTime(GetTickCount());
	pTimer->SetJSTimer(dwInterval);

	JSFXObject pRetObj = JS_NewFxDynamicObj(*pRuntime, pContext, JS_GetObjDefnID(*pRuntime, L"TimerObj"));

	CJS_TimerObj* pJS_TimerObj = (CJS_TimerObj*)JS_GetPrivate(pRuntime->GetIsolate(),pRetObj);
	ASSERT(pJS_TimerObj != NULL);

	TimerObj* pTimerObj = (TimerObj*)pJS_TimerObj->GetEmbedObject();
	ASSERT(pTimerObj != NULL);

	pTimerObj->SetTimer(pTimer);

	vRet = pRetObj;

	return TRUE;
}

FX_BOOL app::setTimeOut(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	if (params.size() > 2 || params.size() == 0)
	{
		sError = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	CFX_WideString script = params.size() > 0 ? params[0].ToCFXWideString() : L"";
	if (script.IsEmpty())
	{
		sError = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSAFNUMBER_KEYSTROKE);
		return TRUE;
	}

	FX_DWORD dwTimeOut = params.size() > 1 ? params[1].ToInt() : 1000;

	CPDFDoc_Environment* pApp = pRuntime->GetReaderApp();
	ASSERT(pApp);

	CJS_Timer* pTimer = new CJS_Timer(this, pApp);
	m_aTimer.Add(pTimer);

	pTimer->SetType(1);
	pTimer->SetRuntime(pRuntime);
	pTimer->SetJScript(script);
	pTimer->SetTimeOut(dwTimeOut);
	pTimer->SetJSTimer(dwTimeOut);

	JSFXObject pRetObj = JS_NewFxDynamicObj(*pRuntime, pContext, JS_GetObjDefnID(*pRuntime, L"TimerObj"));

	CJS_TimerObj* pJS_TimerObj = (CJS_TimerObj*)JS_GetPrivate(pRuntime->GetIsolate(),pRetObj);
	ASSERT(pJS_TimerObj != NULL);

	TimerObj* pTimerObj = (TimerObj*)pJS_TimerObj->GetEmbedObject();
	ASSERT(pTimerObj != NULL);

	pTimerObj->SetTimer(pTimer);

	vRet = pRetObj;

	return TRUE;
}

FX_BOOL app::clearTimeOut(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	if (params.size() != 1)
	{
		sError = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	if (params[0].GetType() == VT_fxobject)
	{
		JSFXObject pObj = params[0].ToV8Object();
		{
			if (JS_GetObjDefnID(pObj) == JS_GetObjDefnID(*pRuntime, L"TimerObj"))
			{
				if (CJS_Object* pJSObj = params[0].ToCJSObject())
				{
					if (TimerObj* pTimerObj = (TimerObj*)pJSObj->GetEmbedObject())
					{
						if (CJS_Timer* pTimer = pTimerObj->GetTimer())
						{
							pTimer->KillJSTimer();

							for (int i=0,sz=m_aTimer.GetSize(); i<sz; i++)
							{
								if (m_aTimer[i] == pTimer)
								{
									m_aTimer.RemoveAt(i);
									break;
								}
							}

							delete pTimer;
							pTimerObj->SetTimer(NULL);
						}
					}
				}
			}
		}
	}

	return TRUE;
}

FX_BOOL app::clearInterval(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	if (params.size() != 1)
	{
		sError = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	if (params[0].GetType() == VT_fxobject)
	{
		JSFXObject pObj = params[0].ToV8Object();
		{
			if (JS_GetObjDefnID(pObj) == JS_GetObjDefnID(*pRuntime, L"TimerObj"))
			{
				if (CJS_Object* pJSObj = params[0].ToCJSObject())
				{
					if (TimerObj* pTimerObj = (TimerObj*)pJSObj->GetEmbedObject())
					{
						if (CJS_Timer* pTimer = pTimerObj->GetTimer())
						{
							pTimer->KillJSTimer();

							for (int i=0,sz=m_aTimer.GetSize(); i<sz; i++)
							{
								if (m_aTimer[i] == pTimer)
								{
									m_aTimer.RemoveAt(i);
									break;
								}
							}

							delete pTimer;
							pTimerObj->SetTimer(NULL);
						}
					}
				}
			}
		}
	}

	return TRUE;
}

FX_BOOL app::execMenuItem(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return FALSE;
}

void app::TimerProc(CJS_Timer* pTimer)
{
	ASSERT(pTimer != NULL);

	switch (pTimer->GetType())
	{
	case 0: //interval
		RunJsScript(pTimer->GetRuntime(), pTimer->GetJScript());
		break;
	case 1:
		if (pTimer->GetTimeOut() > 0)
		{
			RunJsScript(pTimer->GetRuntime(), pTimer->GetJScript());
			pTimer->KillJSTimer();
		}
		break;
	}

}

void app::RunJsScript(CJS_Runtime* pRuntime,const CFX_WideString& wsScript)
{
	ASSERT(pRuntime != NULL);

	if (!pRuntime->IsBlocking())
	{
		IFXJS_Context* pContext = pRuntime->NewContext();
		ASSERT(pContext != NULL);
		pContext->OnExternal_Exec();
		CFX_WideString wtInfo;
		pContext->RunScript(wsScript,wtInfo);
		pRuntime->ReleaseContext(pContext);
	}
}

FX_BOOL app::goBack(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
  // Not supported.
  return TRUE;
}

FX_BOOL app::goForward(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
  // Not supported.
  return TRUE;
}

FX_BOOL app::mailMsg(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	v8::Isolate* isolate = GetIsolate(cc);

	FX_BOOL bUI = TRUE;
	CFX_WideString cTo = L"";
	CFX_WideString cCc = L"";
	CFX_WideString cBcc = L"";
	CFX_WideString cSubject = L"";
	CFX_WideString cMsg = L"";

	if (params.size() < 1)
		return FALSE;

	if (params[0].GetType() == VT_object)
	{
		JSObject pObj = params[0].ToV8Object();

		v8::Local<v8::Value> pValue = JS_GetObjectElement(isolate, pObj, L"bUI");
		bUI = CJS_Value(isolate, pValue, GET_VALUE_TYPE(pValue)).ToBool();

		pValue = JS_GetObjectElement(isolate, pObj, L"cTo");
		cTo = CJS_Value(isolate, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

		pValue = JS_GetObjectElement(isolate, pObj, L"cCc");
		cCc = CJS_Value(isolate, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

		pValue = JS_GetObjectElement(isolate, pObj, L"cBcc");
		cBcc = CJS_Value(isolate, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

		pValue = JS_GetObjectElement(isolate, pObj, L"cSubject");
		cSubject = CJS_Value(isolate, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

		pValue = JS_GetObjectElement(isolate, pObj, L"cMsg");
		cMsg = CJS_Value(isolate, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();
	} else {
		if (params.size() < 2)
			return FALSE;

		bUI = params[0].ToBool();
		cTo = params[1].ToCFXWideString();

		if (params.size() >= 3)
			cCc = params[2].ToCFXWideString();
		if (params.size() >= 4)
			cBcc = params[3].ToCFXWideString();
		if (params.size() >= 5)
			cSubject = params[4].ToCFXWideString();
		if (params.size() >= 6)
			cMsg = params[5].ToCFXWideString();
	}

	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	CPDFDoc_Environment* pApp = pContext->GetReaderApp();
	ASSERT(pApp != NULL);

	pRuntime->BeginBlock();
	pApp->JS_docmailForm(NULL, 0, bUI, cTo.c_str(), cSubject.c_str(), cCc.c_str(), cBcc.c_str(), cMsg.c_str());
	pRuntime->EndBlock();

	return FALSE;
}

FX_BOOL app::launchURL(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL app::runtimeHighlight(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	if (vp.IsSetting())
	{
		vp>>m_bRuntimeHighLight;
	}
	else
	{
		vp<<m_bRuntimeHighLight;
	}

	return TRUE;
}

FX_BOOL app::fullscreen(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	return FALSE;
}

FX_BOOL app::popUpMenu(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return FALSE;
}


FX_BOOL app::browseForDoc(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
  // Unsafe, not supported.
  return TRUE;
}

CFX_WideString app::SysPathToPDFPath(const CFX_WideString& sOldPath)
{
	CFX_WideString sRet = L"/";

	for (int i=0,sz=sOldPath.GetLength(); i<sz; i++)
	{
		wchar_t c = sOldPath.GetAt(i);
		if (c == L':')
		{
		}
		else
		{
			if (c == L'\\')
			{
				sRet += L"/";
			}
			else
			{
				sRet += c;
			}
		}
	}

	return sRet;
}

FX_BOOL app::newDoc(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return FALSE;
}

FX_BOOL app::openDoc(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return FALSE;
}

FX_BOOL app::response(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	CFX_WideString swQuestion = L"";
	CFX_WideString swLabel = L"";
	CFX_WideString swTitle = L"PDF";
	CFX_WideString swDefault = L"";
	bool bPassWord = false;

	v8::Isolate* isolate = GetIsolate(cc);

	int iLength = params.size();
	if (iLength > 0 && params[0].GetType() == VT_object)
	{
		JSObject pObj = params[0].ToV8Object();
		v8::Local<v8::Value> pValue = JS_GetObjectElement(isolate,pObj,L"cQuestion");
		swQuestion = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue)).ToCFXWideString();

		pValue = JS_GetObjectElement(isolate,pObj,L"cTitle");
		swTitle = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue)).ToCFXWideString();

		pValue = JS_GetObjectElement(isolate,pObj,L"cDefault");
		swDefault = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue)).ToCFXWideString();

		pValue = JS_GetObjectElement(isolate,pObj,L"cLabel");
		swLabel = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue)).ToCFXWideString();

		pValue = JS_GetObjectElement(isolate,pObj,L"bPassword");
		bPassWord = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue)).ToBool();
	}
	else
	{
		switch(iLength)
		{
		case 5:
			swLabel = params[4].ToCFXWideString();
			// FALLTHROUGH
		case 4:
			bPassWord = params[3].ToBool();
			// FALLTHROUGH
		case 3:
			swDefault = params[2].ToCFXWideString();
			// FALLTHROUGH
		case 2:
			swTitle = params[1].ToCFXWideString();
			// FALLTHROUGH
		case 1:
			swQuestion = params[0].ToCFXWideString();
			// FALLTHROUGH
		default:
			break;
		}
	}

	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);

	CPDFDoc_Environment* pApp = pContext->GetReaderApp();
	ASSERT(pApp != NULL);

	const int MAX_INPUT_BYTES = 2048;
	char* pBuff = new char[MAX_INPUT_BYTES + 2];
	if (!pBuff)
		return FALSE;

	memset(pBuff, 0, MAX_INPUT_BYTES + 2);
	int nLengthBytes = pApp->JS_appResponse(swQuestion.c_str(), swTitle.c_str(), swDefault.c_str(),
                                            swLabel.c_str(), bPassWord, pBuff, MAX_INPUT_BYTES);
	if (nLengthBytes <= 0)
	{
		vRet.SetNull();
		delete[] pBuff;
		return FALSE;
	}
	if (nLengthBytes > MAX_INPUT_BYTES)
		nLengthBytes = MAX_INPUT_BYTES;

	vRet = CFX_WideString::FromUTF16LE((unsigned short*)pBuff, nLengthBytes / sizeof(unsigned short)).c_str();
	delete[] pBuff;
	return TRUE;
}

FX_BOOL app::media(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError)
{
	return FALSE;
}

FX_BOOL app::execDialog(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError)
{
	return TRUE;
}
