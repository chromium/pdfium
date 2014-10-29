// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fsdk_define.h"
#include "../../include/fpdfformfill.h"
#include "../../include/fsdk_mgr.h"
#include "../../include/fpdfxfa/fpdfxfa_doc.h"
#include "../../include/fpdfxfa/fpdfxfa_util.h"
#include "../../include/jsapi/fxjs_v8.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/fpdfxfa/fpdfxfa_app.h"
CPDFXFA_App* CPDFXFA_App::m_pApp = NULL;

CPDFXFA_App* FPDFXFA_GetApp()
{
	if (!CPDFXFA_App::m_pApp)
		CPDFXFA_App::m_pApp = FX_NEW CPDFXFA_App();

	return CPDFXFA_App::m_pApp;
}

void FPDFXFA_ReleaseApp()
{
	if (CPDFXFA_App::m_pApp)
		delete CPDFXFA_App::m_pApp;
	CPDFXFA_App::m_pApp = NULL;
}

CJS_RuntimeFactory* g_GetJSRuntimeFactory()
{
	static CJS_RuntimeFactory g_JSRuntimeFactory;
	return &g_JSRuntimeFactory;
}

CPDFXFA_App::CPDFXFA_App() : 
	m_pXFAApp(NULL), 
	m_pFontMgr(NULL),
	m_hJSERuntime(NULL),
	//m_pJSRuntime(NULL),
	//m_pEnv(NULL),
	m_csAppType(JS_STR_VIEWERTYPE_STANDARD)
{
	m_pJSRuntimeFactory = NULL;
	m_pJSRuntimeFactory = g_GetJSRuntimeFactory();
	m_pJSRuntimeFactory->AddRef();
	m_pEnvList.RemoveAll();	
	m_bInitRuntime = FALSE;
}
//IFXJS_Runtime* CPDFXFA_App::GetJSRuntime()
//{
//	FXSYS_assert(m_pJSRuntimeFactory);
//	if(!m_pJSRuntime)
//		m_pJSRuntime = m_pJSRuntimeFactory->NewJSRuntime(this);
//	return m_pJSRuntime;
//}

CPDFXFA_App::~CPDFXFA_App()
{
	if (m_pFontMgr)
	{
		m_pFontMgr->Release();
		m_pFontMgr = NULL;
	}

	if (m_pXFAApp)
	{
		m_pXFAApp->Release();
		m_pXFAApp = NULL;
	}

	//if (m_pJSRuntime && m_pJSRuntimeFactory)
	//	m_pJSRuntimeFactory->DeleteJSRuntime(m_pJSRuntime);
	m_pJSRuntimeFactory->Release();


	if (m_hJSERuntime)
	{
		FXJSE_Runtime_Release(m_hJSERuntime);
		m_hJSERuntime = NULL;
	}

	FXJSE_Finalize();

	BC_Library_Destory();
}

FX_BOOL CPDFXFA_App::Initialize()
{
	BC_Library_Init();

	FXJSE_Initialize();
	m_hJSERuntime = FXJSE_Runtime_Create();

	if (!m_hJSERuntime) 
		return FALSE;

	//m_pJSRuntime = m_pJSRuntimeFactory->NewJSRuntime(this);
	
	m_pXFAApp = IXFA_App::Create(this);
	if (!m_pXFAApp)
		return FALSE;

	m_pFontMgr = XFA_GetDefaultFontMgr();
	if (!m_pFontMgr)
		return FALSE;

	m_pXFAApp->SetDefaultFontMgr(m_pFontMgr);

	return TRUE;
}

FX_BOOL CPDFXFA_App::AddFormFillEnv(CPDFDoc_Environment* pEnv)
{
	if (!pEnv) return FALSE;

	m_pEnvList.Add(pEnv);
	return TRUE;
}

FX_BOOL CPDFXFA_App::RemoveFormFillEnv(CPDFDoc_Environment* pEnv)
{
	if (!pEnv) return FALSE;

	int nFind = m_pEnvList.Find(pEnv);
	if (nFind != -1) {
		m_pEnvList.RemoveAt(nFind);
		return TRUE;
	}

	return FALSE;
}
void CPDFXFA_App::ReleaseRuntime() 
{
	v8::Persistent<v8::Context> context;
	JS_ReleaseRuntime((IJS_Runtime*)m_hJSERuntime, context);
}

void CPDFXFA_App::GetAppType(CFX_WideString &wsAppType)
{
	wsAppType = m_csAppType;
}

void CPDFXFA_App::GetAppName(CFX_WideString& wsName)
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
	{
		wsName = pEnv->FFI_GetAppName();
	}
}

void CPDFXFA_App::SetAppType(FX_WSTR wsAppType)
{
	m_csAppType = wsAppType;
}

void CPDFXFA_App::GetLanguage(CFX_WideString &wsLanguage)
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
	{
		wsLanguage = pEnv->FFI_GetLanguage();
	}
}

void CPDFXFA_App::GetPlatform(CFX_WideString &wsPlatform)
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
	{
		wsPlatform = pEnv->FFI_GetPlatform();
	}
}

void CPDFXFA_App::GetVariation(CFX_WideString &wsVariation)
{
	wsVariation = JS_STR_VIEWERVARIATION;
}

void CPDFXFA_App::GetVersion(CFX_WideString &wsVersion)
{
	wsVersion = JS_STR_VIEWERVERSION_XFA;
}

void CPDFXFA_App::Beep(FX_DWORD dwType)
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
	{
		pEnv->JS_appBeep(dwType);
	}
}

FX_INT32 CPDFXFA_App::MsgBox(FX_WSTR wsMessage, FX_WSTR wsTitle, FX_DWORD dwIconType, FX_DWORD dwButtonType)
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (!pEnv) 
		return -1;

	FX_DWORD iconType = 0;
	int iButtonType = 0;
	switch (dwIconType)
	{
	case XFA_MBICON_Error:
		iconType |= 0;
		break;
	case XFA_MBICON_Warning:
		iconType |= 1;
		break;
	case XFA_MBICON_Question:
		iconType |= 2;
		break;
	case XFA_MBICON_Status:
		iconType |= 3;
		break;
	}
	switch (dwButtonType)
	{
	case  XFA_MB_OK:
		iButtonType |= 0;
		break;
	case XFA_MB_OKCancel:
		iButtonType |= 1;
		break;
	case XFA_MB_YesNo:
		iButtonType |= 2;
		break;
	case XFA_MB_YesNoCancel:
		iButtonType |= 3;
		break;
	}
	FX_INT32 iRet = pEnv->JS_appAlert(wsMessage.GetPtr(), wsTitle.GetPtr(), iButtonType, iconType);
	switch (iRet)
	{
	case 1:
		return XFA_IDOK;
	case 2:
		return XFA_IDCancel;
	case 3:
		return XFA_IDNo;
	case 4:
		return XFA_IDYes;
	}
	return XFA_IDYes;
}

void CPDFXFA_App::Response(CFX_WideString &wsAnswer, FX_WSTR wsQuestion, FX_WSTR wsTitle, FX_WSTR wsDefaultAnswer, FX_BOOL bMark)
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
	{
		int nLength = 2048;
		char* pBuff = new char[nLength];
		nLength = pEnv->JS_appResponse(wsQuestion.GetPtr(), wsTitle.GetPtr(), wsDefaultAnswer.GetPtr(), NULL, bMark, pBuff, nLength);
		if(nLength > 0)
		{
			nLength = nLength>2046?2046:nLength;
			pBuff[nLength] = 0;
			pBuff[nLength+1] = 0;
			wsAnswer = CFX_WideString::FromUTF16LE((unsigned short*)pBuff, nLength);
		}
		delete[] pBuff;
	}
}

FX_INT32 CPDFXFA_App::GetCurDocumentInBatch()
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
	{
		return pEnv->FFI_GetCurDocument();
	}
	return 0;
}

FX_INT32 CPDFXFA_App::GetDocumentCountInBatch()
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
	{
		return pEnv->FFI_GetDocumentCount();
	}

	return 0;
}

IFX_FileRead* CPDFXFA_App::DownloadURL(FX_WSTR wsURL)
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
	{
		return pEnv->FFI_DownloadFromURL(wsURL.GetPtr());
	}
	return NULL;
}

FX_BOOL CPDFXFA_App::PostRequestURL(FX_WSTR wsURL, FX_WSTR wsData, FX_WSTR wsContentType, 
	FX_WSTR wsEncode, FX_WSTR wsHeader, CFX_WideString &wsResponse)
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
	{
		wsResponse = pEnv->FFI_PostRequestURL(wsURL.GetPtr(), wsData.GetPtr(), wsContentType.GetPtr(), wsEncode.GetPtr(), wsHeader.GetPtr());
		return TRUE;
	}
	return FALSE;
}

FX_BOOL CPDFXFA_App::PutRequestURL(FX_WSTR wsURL, FX_WSTR wsData, FX_WSTR wsEncode)
{
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
	{
		return pEnv->FFI_PutRequestURL(wsURL.GetPtr(), wsData.GetPtr(), wsEncode.GetPtr());
	}
	return FALSE;
}

void CPDFXFA_App::LoadString(FX_INT32 iStringID, CFX_WideString &wsString)
{
	switch (iStringID)
	{
	case XFA_IDS_ValidateFailed:
		wsString = L"%s validate failed";
		return;
	case XFA_IDS_CalcOverride:
		wsString = L"Calculate Override";
		return;
	case XFA_IDS_ModifyField:
		wsString = L"Are you sure you want to modify this field?";
		return;
	case XFA_IDS_NotModifyField:
		wsString = L"You are not allowed to modify this field.";
		return;
	case XFA_IDS_AppName:
		wsString = L"Foxit";
		return;
	case XFA_IDS_ImageFilter:
		wsString = L"Image Files(*.bmp;*.jpg;*.png;*.gif;*.tif)|*.bmp;*.jpg;*.png;*.gif;*.tif|All Files(*.*)|*.*||";
		return;
	case XFA_IDS_UNKNOW_CATCHED:
		wsString = L"unknown error is catched!";
		return;
	case XFA_IDS_Unable_TO_SET:
		wsString = L"Unable to set ";
		return;
	case XFA_IDS_VALUE_EXCALMATORY:
		wsString = L" value!";
		return;
	case XFA_IDS_INVALID_ENUM_VALUE:
		wsString = L"Invalid enumerated value: ";
		return;
	case XFA_IDS_UNSUPPORT_METHOD:
		wsString = L"unsupport %s method.";
		return;
	case XFA_IDS_UNSUPPORT_PROP:
		wsString = L"unsupport %s property.";
		return;
	case XFA_IDS_INVAlID_PROP_SET:
		wsString = L"Invalid property set operation;";
		return;
	case XFA_IDS_NOT_DEFAUL_VALUE:
		wsString = L" doesn't have a default property";
		return;
	case XFA_IDS_UNABLE_SET_LANGUAGE:
		wsString = L"Unable to set language value!";
		return;
	case XFA_IDS_UNABLE_SET_NUMPAGES:
		wsString = L"Unable to set numPages value!";
		return;
	case XFA_IDS_UNABLE_SET_PLATFORM:
		wsString = L"Unable to set platform value!";
		return;
	case XFA_IDS_UNABLE_SET_VALIDATIONENABLE:
		wsString = L"Unable to set validationsEnabled value!";
		return;
	case XFA_IDS_UNABLE_SET_VARIATION:
		wsString = L"Unable to set variation value!";
		return;
	case XFA_IDS_UNABLE_SET_VERSION:
		wsString = L"Unable to set version value!";
		return;
	case XFA_IDS_UNABLE_SET_READY:
		wsString = L"Unable to set ready value!";
		return;
	case XFA_IDS_NUMBER_OF_OCCUR:
		wsString = L"The element [%s] has violated its allowable number of occurrences";
		return;
	case XFA_IDS_UNABLE_SET_CLASS_NAME:
		wsString = L"Unable to set className value!";
		return;
	case XFA_IDS_UNABLE_SET_LENGTH_VALUE:
		wsString = L"Unable to set length value!";
		return;
	case XFA_IDS_UNSUPPORT_CHAR:
		wsString = L"unsupported char '%c'";
		return;
	case XFA_IDS_BAD_SUFFIX:
		wsString = L"bad suffix on number";
		return;
	case XFA_IDS_EXPECTED_IDENT:
		wsString = L"expected identifier instead of '%s'";
		return;
	case XFA_IDS_EXPECTED_STRING:
		wsString = L"expected '%s' instead of '%s'";
		return;
	case XFA_IDS_INVALIDATE_CHAR:
		wsString = L"invalidate char '%c'";
		return;
	case XFA_IDS_REDEFINITION:
		wsString = L"'%s' redefinition ";
		return;
	case XFA_IDS_INVALIDATE_TOKEN:
		wsString = L"invalidate token '%s'";
		return;
	case XFA_IDS_INVALIDATE_EXPRESSION:
		wsString = L"invalidate expression '%s'";
		return;
	case XFA_IDS_UNDEFINE_IDENTIFIER:
		wsString = L"undefined identifier '%s'";
		return;
	case XFA_IDS_INVALIDATE_LEFTVALUE:
		wsString = L"invalidate left-value '%s'";
		return;
	case XFA_IDS_COMPILER_ERROR:
		wsString = L"compiler error";
		return;
	case XFA_IDS_CANNOT_MODIFY_VALUE:
		wsString = L"can't modify the '%s' value";
		return;
	case XFA_IDS_ERROR_PARAMETERS:
		wsString = L"function '%s' has not %d parameters";
		return;
	case XFA_IDS_EXPECT_ENDIF:
		wsString = L"expected 'endif' instead of '%s'";
		return;
	case XFA_IDS_UNEXPECTED_EXPRESSION:
		wsString = L"unexpected expression '%s'";
		return;
	case XFA_IDS_CONDITION_IS_NULL:
		wsString = L"condition is null";
		return;
	case XFA_IDS_ILLEGALBREAK:
		wsString = L"illegal break";
		return;
	case XFA_IDS_ILLEGALCONTINUE:
		wsString = L"illegal continue";
		return;
	case XFA_IDS_EXPECTED_OPERATOR:
		wsString = L"expected operator '%s' instead of '%s'";
		return;
	case XFA_IDS_DIVIDE_ZERO:
		wsString = L"divide by zero";
		return;
	case XFA_IDS_CANNOT_COVERT_OBJECT:
		wsString = L"%s.%s can not covert to object";
		return;
	case XFA_IDS_NOT_FOUND_CONTAINER:
		wsString = L"can not found container '%s'";
		return;
	case XFA_IDS_NOT_FOUND_PROPERTY:
		wsString = L"can not found property '%s'";
		return;
	case XFA_IDS_NOT_FOUND_METHOD:
		wsString = L"can not found method '%s'";
		return;
	case XFA_IDS_NOT_FOUND_CONST:
		wsString = L"can not found const '%s'";
		return;
	case XFA_IDS_NOT_ASSIGN_OBJECT:
		wsString = L"can not direct assign value to object";
		return;
	case XFA_IDS_IVALIDATE_INSTRUCTION:
		wsString = L"invalidate instruction";
		return;
	case XFA_IDS_EXPECT_NUMBER:
		wsString = L"expected number instead of '%s'";
		return;
	case XFA_IDS_VALIDATE_OUT_ARRAY:
		wsString = L"validate access index '%s' out of array";
		return;
	case XFA_IDS_CANNOT_ASSIGN_IDENT:
		wsString = L"can not assign to %s";
		return;
	case XFA_IDS_NOT_FOUNT_FUNCTION:
		wsString = L"can not found '%s' function";
		return;
	case XFA_IDS_NOT_ARRAY:
		wsString = L"'%s' doesn't an array";
		return;
	case XFA_IDS_OUT_ARRAY:
		wsString = L"out of range of '%s' array";
		return;
	case XFA_IDS_NOT_SUPPORT_CALC:
		wsString = L"'%s' operator can not support array calculate";
		return;
	case XFA_IDS_ARGUMENT_NOT_ARRAY:
		wsString = L"'%s' function's %d argument can not be array";
		return;
	case XFA_IDS_ARGUMENT_EXPECT_CONTAINER:
		wsString = L"'%s' argument expected a container";
		return;
	case XFA_IDS_ACCESS_PROPERTY_IN_NOT_OBJECT:
		wsString = L"an attempt was made to reference property '%s' of a non-object in SOM expression %s";
		return;
	case XFA_IDS_FUNCTION_IS_BUILDIN:
		wsString = L"function '%s' is buildin";
		return;
	case XFA_IDS_ERROR_MSG:
		wsString = L"%s : %s";
		return;
	case XFA_IDS_INDEX_OUT_OF_BOUNDS:
		wsString = L"Index value is out of bounds";
		return;
	case XFA_IDS_INCORRECT_NUMBER_OF_METHOD:
		wsString = L"Incorrect number of parameters calling method '%s'";
		return;
	case XFA_IDS_ARGUMENT_MISMATCH:
		wsString = L"Argument mismatch in property or function argument";
		return;
	case XFA_IDS_INVALID_ENUMERATE:
		wsString = L"Invalid enumerated value: %s";
		return;
	case XFA_IDS_INVALID_APPEND:
		wsString = L"Invalid append operation: %s cannot have a child element of %s";
		return;
	case XFA_IDS_SOM_EXPECTED_LIST:
		wsString = L"SOM expression returned list when single result was expected";
		return;
	case XFA_IDS_NOT_HAVE_PROPERTY:
		wsString = L"'%s' doesn't have property '%s'";
		return;
	case XFA_IDS_INVALID_NODE_TYPE:
		wsString = L"Invalid node type : '%s'";
		return;
	case XFA_IDS_VIOLATE_BOUNDARY:
		wsString = L"The element [%s] has violated its allowable number of occurrences";
		return;
	case XFA_IDS_SERVER_DENY:
		wsString = L"Server does not permit";
		return;
	/*case XFA_IDS_StringWeekDay_Sun:
		wsString = L"?¨¹¨¨?";
		return;
	case XFA_IDS_StringWeekDay_Mon:
		wsString = L"?¨¹¨°?";
		return;
	case XFA_IDS_StringWeekDay_Tue:
		wsString = L"?¨¹?t";
		return;
	case XFA_IDS_StringWeekDay_Wed:
		wsString = L"?¨¹¨¨y";
		return;
	case XFA_IDS_StringWeekDay_Thu:
		wsString = L"?¨¹??";
		return;
	case XFA_IDS_StringWeekDay_Fri:
		wsString = L"?¨¹??";
		return;
	case XFA_IDS_StringWeekDay_Sat:
		wsString = L"?¨¹¨¢¨´";
		return;
	case XFA_IDS_StringMonth_Jan:
		wsString = L"1??";
		return;
	case XFA_IDS_StringMonth_Feb:
		wsString = L"2??";
		return;
	case XFA_IDS_StringMonth_March:
		wsString = L"3??";
		return;
	case XFA_IDS_StringMonth_April:
		wsString = L"4??";
		return;
	case XFA_IDS_StringMonth_May:
		wsString = L"5??";
		return;
	case XFA_IDS_StringMonth_June:
		wsString = L"6??";
		return;
	case XFA_IDS_StringMonth_July:
		wsString = L"7??";
		return;
	case XFA_IDS_StringMonth_Aug:
		wsString = L"8??";
		return;
	case XFA_IDS_StringMonth_Sept:
		wsString = L"9??";
		return;
	case XFA_IDS_StringMonth_Oct:
		wsString = L"10??";
		return;
	case XFA_IDS_StringMonth_Nov:
		wsString = L"11??";
		return;
	case XFA_IDS_StringMonth_Dec:
		wsString = L"12??";
		return;
	case XFA_IDS_String_Today:
		wsString = L"??¨¬¨¬";
		return;*/
	case XFA_IDS_ValidateLimit:
		wsString = FX_WSTRC(L"Message limit exceeded. Remaining %d validation errors not reported.");
		return;
	case XFA_IDS_ValidateNullWarning:
		wsString = FX_WSTRC(L"%s cannot be left blank. To ignore validations for %s, click Ignore.");
		return;
	case XFA_IDS_ValidateNullError:
		wsString = FX_WSTRC(L"%s cannot be left blank.");
		return;
	case XFA_IDS_ValidateWarning:
		wsString = FX_WSTRC(L"The value you entered for %s is invalid. To ignore validations for %s, click Ignore.");
		return;
	case XFA_IDS_ValidateError:
		wsString = FX_WSTRC(L"The value you entered for %s is invalid.");
		return;
	}

}

FX_BOOL CPDFXFA_App::ShowFileDialog(FX_WSTR wsTitle, FX_WSTR wsFilter, CFX_WideStringArray &wsPathArr, FX_BOOL bOpen)
{
	//if (m_pEnv)
	//{
	//	return m_pEnv->FFI_ShowFileDialog(wsTitle.GetPtr(), wsFilter.GetPtr(), wsPathArr, bOpen);
	//}
	return FALSE;
}

IFWL_AdapterTimerMgr* CPDFXFA_App::GetTimerMgr()
{
	CXFA_FWLAdapterTimerMgr* pAdapter = NULL;
	CPDFDoc_Environment* pEnv = m_pEnvList.GetAt(0);
	if (pEnv)
		pAdapter = FX_NEW CXFA_FWLAdapterTimerMgr(pEnv);
	return pAdapter;
}
