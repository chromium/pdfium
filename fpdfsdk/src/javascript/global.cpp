// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/JS_GlobalData.h"
#include "../../include/javascript/global.h"
#include "../../include/javascript/JS_EventHandler.h"
#include "../../include/javascript/JS_Context.h"

/* ---------------------------- global ---------------------------- */

// Helper class for compile-time calculation of hash values in order to
// avoid having global object initializers.
template <unsigned ACC, wchar_t... Ns>
struct CHash;

// Only needed to hash single-character strings.
template <wchar_t N>
struct CHash<N> {
  static const unsigned value = N;
};

template <unsigned ACC, wchar_t N>
struct CHash<ACC, N> {
  static const unsigned value = (ACC * 1313LLU + N) & 0xFFFFFFFF;
};

template <unsigned ACC, wchar_t N, wchar_t... Ns>
struct CHash<ACC, N, Ns...> {
  static const unsigned value = CHash<CHash<ACC, N>::value, Ns...>::value;
};

extern const unsigned int JSCONST_nStringHash =
  CHash<'s','t','r','i','n','g'>::value;
extern const unsigned int JSCONST_nNumberHash =
  CHash<'n','u','m','b','e','r'>::value;
extern const unsigned int JSCONST_nBoolHash =
  CHash<'b','o','o','l','e','a','n'>::value;
extern const unsigned int JSCONST_nDateHash =
  CHash<'d','a','t','e'>::value;
extern const unsigned int JSCONST_nObjectHash =
  CHash<'o','b','j','e','c','t'>::value;
extern const unsigned int JSCONST_nFXobjHash =
  CHash<'f','x','o','b','j'>::value;
extern const unsigned int JSCONST_nNullHash =
  CHash<'n','u','l','l'>::value;
extern const unsigned int JSCONST_nUndefHash =
  CHash<'u','n','d','e','f','i','n','e','d'>::value;

#ifdef _DEBUG
class HashVerify
{
public:
  HashVerify();
} g_hashVerify;

HashVerify::HashVerify()
{
  ASSERT(JSCONST_nStringHash ==
    JS_CalcHash(VALUE_NAME_STRING,wcslen(VALUE_NAME_STRING)));
  ASSERT(JSCONST_nNumberHash ==
    JS_CalcHash(VALUE_NAME_NUMBER,wcslen(VALUE_NAME_NUMBER)));
  ASSERT(JSCONST_nBoolHash ==
    JS_CalcHash(VALUE_NAME_BOOLEAN,wcslen(VALUE_NAME_BOOLEAN)));
  ASSERT(JSCONST_nDateHash ==
    JS_CalcHash(VALUE_NAME_DATE,wcslen(VALUE_NAME_DATE)));
  ASSERT(JSCONST_nObjectHash ==
    JS_CalcHash(VALUE_NAME_OBJECT,wcslen(VALUE_NAME_OBJECT)));
  ASSERT(JSCONST_nFXobjHash ==
    JS_CalcHash(VALUE_NAME_FXOBJ,wcslen(VALUE_NAME_FXOBJ)));
  ASSERT(JSCONST_nNullHash ==
    JS_CalcHash(VALUE_NAME_NULL,wcslen(VALUE_NAME_NULL)));
  ASSERT(JSCONST_nUndefHash ==
    JS_CalcHash(VALUE_NAME_UNDEFINED,wcslen(VALUE_NAME_UNDEFINED)));
}
#endif


BEGIN_JS_STATIC_CONST(CJS_Global)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_Global)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_Global)
	JS_STATIC_METHOD_ENTRY(setPersistent, 2)
END_JS_STATIC_METHOD()

IMPLEMENT_SPECIAL_JS_CLASS(CJS_Global, global_alternate, global);

FX_BOOL	CJS_Global::InitInstance(IFXJS_Context* cc)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);

	global_alternate* pGlobal = (global_alternate*)GetEmbedObject();
	ASSERT(pGlobal != NULL);
	
	pGlobal->Initial(pContext->GetReaderApp());
	
	return TRUE;
};

global_alternate::global_alternate(CJS_Object* pJSObject)
	: CJS_EmbedObj(pJSObject),
	m_pApp(NULL)
{
}

global_alternate::~global_alternate(void)
{
	ASSERT(m_pApp != NULL);

//	CommitGlobalPersisitentVariables();
	DestroyGlobalPersisitentVariables();

	CJS_RuntimeFactory* pFactory = m_pApp->m_pJSRuntimeFactory;
	ASSERT(pFactory);

	pFactory->ReleaseGlobalData();
}
  
void global_alternate::Initial(CPDFDoc_Environment* pApp)
{
	m_pApp = pApp;

	CJS_RuntimeFactory* pFactory = pApp->m_pJSRuntimeFactory;
	ASSERT(pFactory);
	m_pGlobalData = pFactory->NewGlobalData(pApp);
	UpdateGlobalPersistentVariables();
}

FX_BOOL	global_alternate::QueryProperty(FX_LPCWSTR propname)
{
	return CFX_WideString(propname) != L"setPersistent";
}

FX_BOOL	global_alternate::DelProperty(IFXJS_Context* cc, FX_LPCWSTR propname, JS_ErrorString& sError)
{
	js_global_data* pData = NULL;
	CFX_ByteString sPropName = CFX_ByteString::FromUnicode(propname);

	if (m_mapGlobal.Lookup(sPropName, (FX_LPVOID&)pData))
	{
		pData->bDeleted = TRUE;
		return TRUE;
	}

	return FALSE;
}

FX_BOOL global_alternate::DoProperty(IFXJS_Context* cc, FX_LPCWSTR propname, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (vp.IsSetting())
	{
		CFX_ByteString sPropName = CFX_ByteString::FromUnicode(propname);
		switch (vp.GetType())
		{
		case VT_number:
			{
				double dData;
				vp >> dData;
				return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_NUMBER, dData, false, "", v8::Handle<v8::Object>(), FALSE);
			}
		case VT_boolean:
			{
				bool bData;
				vp >> bData;
				return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_BOOLEAN, 0, (bool)vp, "", v8::Handle<v8::Object>(), FALSE);
			}
		case VT_string:
			{
				CFX_ByteString sData;
				vp >> sData;
				return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_STRING, 0, false, sData, v8::Handle<v8::Object>(), FALSE);
			}
		case VT_object:
			{
				JSObject pData = (JSObject)vp;
				return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_OBJECT, 0, false, "", pData, FALSE);
// 				else
// 				{
// 					if (vp.IsArrayObject())
// 					{
// 						CJS_Array array;
// 						vp.ConvertToArray(array);
// 						return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_OBJECT, 0, false, "", 
// 							(Dobject*)(Darray*)array, FALSE);
// 					}
// 					else
// 						return FALSE;
// 				}
			}
		case VT_null:
			{
				return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_NULL, 0, false, "", v8::Handle<v8::Object>(), FALSE);
			}
		case VT_undefined:
			{
				DelProperty(cc, propname, sError);
				return TRUE;
			}
		default:
			return FALSE;
		}
	}
	else
	{
		js_global_data* pData = NULL;
		CFX_ByteString sPropName = CFX_ByteString::FromUnicode(propname);

		if (m_mapGlobal.Lookup(sPropName, (FX_LPVOID&)pData))
		{
			if (pData)
			{
				if (!pData->bDeleted)
				{
					switch (pData->nType)
					{
					case JS_GLOBALDATA_TYPE_NUMBER:
						vp << pData->dData;
						break;
					case JS_GLOBALDATA_TYPE_BOOLEAN:
						vp << pData->bData;
						break;
					case JS_GLOBALDATA_TYPE_STRING:
						vp << pData->sData;
						break;
					case JS_GLOBALDATA_TYPE_OBJECT:
						{
							v8::Handle<v8::Object> obj = v8::Local<v8::Object>::New(vp.GetIsolate(),pData->pData);
							vp << obj;
							break;
						}
					case JS_GLOBALDATA_TYPE_NULL:
						vp.SetNull();
						break;
					default:
						return FALSE;
					}
					return TRUE;
				}
				else
				{
					return TRUE;
				}
			}
			else
			{
				vp.SetNull();
				return TRUE;
			}
		}
		else
		{
			vp.SetNull();
			return TRUE;
		}
	}

	return FALSE;
}

FX_BOOL global_alternate::setPersistent(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	if (params.size() != 2)
	{
		//sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);	
		return FALSE;
	}

	CFX_ByteString sName = params[0];

	js_global_data* pData = NULL;
	if (m_mapGlobal.Lookup(sName, (FX_LPVOID&)pData))
	{
		if (pData && !pData->bDeleted)
		{
			pData->bPersistent = (bool)params[1];
			return TRUE;
		}
	}

	//sError = JSGetStringFromID(IDS_JSPARAM_INCORRECT);	
	return FALSE;
}

void global_alternate::UpdateGlobalPersistentVariables()
{
	ASSERT(m_pGlobalData != NULL);

	for (int i=0,sz=m_pGlobalData->GetSize(); i<sz; i++)
	{
		CJS_GlobalData_Element* pData = m_pGlobalData->GetAt(i);
		ASSERT(pData != NULL);

		switch (pData->data.nType)
		{
		case JS_GLOBALDATA_TYPE_NUMBER:
			this->SetGlobalVariables(pData->data.sKey, JS_GLOBALDATA_TYPE_NUMBER, pData->data.dData, false, "", v8::Handle<v8::Object>(), pData->bPersistent == 1);
			JS_PutObjectNumber(NULL,(JSFXObject)(*m_pJSObject),
				pData->data.sKey.UTF8Decode(), pData->data.dData);
			break;
		case JS_GLOBALDATA_TYPE_BOOLEAN:
			this->SetGlobalVariables(pData->data.sKey, JS_GLOBALDATA_TYPE_BOOLEAN, 0, (bool)(pData->data.bData == 1), "", v8::Handle<v8::Object>(), pData->bPersistent == 1);
			JS_PutObjectBoolean(NULL,(JSFXObject)(*m_pJSObject),
				pData->data.sKey.UTF8Decode(), (bool)(pData->data.bData == 1));
			break;
		case JS_GLOBALDATA_TYPE_STRING:
			this->SetGlobalVariables(pData->data.sKey, JS_GLOBALDATA_TYPE_STRING, 0, false, pData->data.sData, v8::Handle<v8::Object>(), pData->bPersistent == 1);
			JS_PutObjectString(NULL,(JSFXObject)(*m_pJSObject),
				pData->data.sKey.UTF8Decode(), 
				pData->data.sData.UTF8Decode());
			break;
		case JS_GLOBALDATA_TYPE_OBJECT:
			{
				IJS_Runtime* pRuntime = JS_GetRuntime((JSFXObject)(*m_pJSObject));
				v8::Handle<v8::Object> pObj = JS_NewFxDynamicObj(pRuntime, NULL, -1);

				PutObjectProperty(pObj, &pData->data);

				this->SetGlobalVariables(pData->data.sKey, JS_GLOBALDATA_TYPE_OBJECT, 0, false, "", 
					(JSObject)pObj, pData->bPersistent == 1);
				JS_PutObjectObject(NULL,(JSFXObject)(*m_pJSObject),
					pData->data.sKey.UTF8Decode(), (JSObject)pObj);
			}
			break;
		case JS_GLOBALDATA_TYPE_NULL:
			this->SetGlobalVariables(pData->data.sKey, JS_GLOBALDATA_TYPE_NULL, 0, false, "", v8::Handle<v8::Object>(), pData->bPersistent == 1);
			JS_PutObjectNull(NULL,(JSFXObject)(*m_pJSObject),
				pData->data.sKey.UTF8Decode());
			break;
		}
	}
}

void global_alternate::CommitGlobalPersisitentVariables()
{
	ASSERT(m_pGlobalData != NULL);

	FX_POSITION	 pos = m_mapGlobal.GetStartPosition();
	while (pos)
	{
		CFX_ByteString name; 
		js_global_data* pData = NULL;
		m_mapGlobal.GetNextAssoc(pos, name, (FX_LPVOID&)pData);
		
		if (pData)
		{
			if (pData->bDeleted)
			{
				m_pGlobalData->DeleteGlobalVariable(name);
			}
			else
			{
				switch (pData->nType)
				{
				case JS_GLOBALDATA_TYPE_NUMBER:
					m_pGlobalData->SetGlobalVariableNumber(name, pData->dData);
					m_pGlobalData->SetGlobalVariablePersistent(name, pData->bPersistent);
					break;
				case JS_GLOBALDATA_TYPE_BOOLEAN:
					m_pGlobalData->SetGlobalVariableBoolean(name, pData->bData);
					m_pGlobalData->SetGlobalVariablePersistent(name, pData->bPersistent);
					break;
				case JS_GLOBALDATA_TYPE_STRING:
					m_pGlobalData->SetGlobalVariableString(name, pData->sData);
					m_pGlobalData->SetGlobalVariablePersistent(name, pData->bPersistent);
					break;
				case JS_GLOBALDATA_TYPE_OBJECT:
					//if (pData->pData)
					{
						CJS_GlobalVariableArray array;
						v8::Handle<v8::Object> obj = v8::Local<v8::Object>::New(GetJSObject()->GetIsolate(),pData->pData);
						ObjectToArray(obj, array);
						m_pGlobalData->SetGlobalVariableObject(name, array);
						m_pGlobalData->SetGlobalVariablePersistent(name, pData->bPersistent);
					}
					break;
				case JS_GLOBALDATA_TYPE_NULL:
					m_pGlobalData->SetGlobalVariableNull(name);
					m_pGlobalData->SetGlobalVariablePersistent(name, pData->bPersistent);
					break;
				}
			}
		}
	}
}

void global_alternate::ObjectToArray(v8::Handle<v8::Object> pObj, CJS_GlobalVariableArray& array)
{
	v8::Handle<v8::Array> pKeyList = JS_GetObjectElementNames(pObj);
	int	nObjElements = pKeyList->Length();

	v8::Local<v8::Context> context = pObj->CreationContext();
	v8::Isolate* isolate = context->GetIsolate();

	for (int i=0; i<nObjElements; i++)
	{
		
		CFX_WideString ws = JS_ToString(JS_GetArrayElemnet(pKeyList, i));
		CFX_ByteString sKey = ws.UTF8Encode();

		v8::Handle<v8::Value> v = JS_GetObjectElement(isolate, pObj, ws.c_str());
		FXJSVALUETYPE vt = GET_VALUE_TYPE(v);
		switch (vt)
		{
		case VT_number:
			{
				CJS_KeyValue* pObjElement = new CJS_KeyValue;
				pObjElement->nType = JS_GLOBALDATA_TYPE_NUMBER;
				pObjElement->sKey = sKey;
				pObjElement->dData = JS_ToNumber(v);
				array.Add(pObjElement);
			}
			break;
		case VT_boolean:
			{
				CJS_KeyValue* pObjElement = new CJS_KeyValue;
				pObjElement->nType = JS_GLOBALDATA_TYPE_BOOLEAN;
				pObjElement->sKey = sKey;
				pObjElement->dData = JS_ToBoolean(v);
				array.Add(pObjElement);
			}
			break;
		case VT_string:
			{
				CFX_ByteString sValue = CJS_Value(isolate, v, VT_string);
				CJS_KeyValue* pObjElement = new CJS_KeyValue;
				pObjElement->nType = JS_GLOBALDATA_TYPE_STRING;
				pObjElement->sKey = sKey;
				pObjElement->sData = sValue;
				array.Add(pObjElement);
			}
			break;
		case VT_object:
			{
				CJS_KeyValue* pObjElement = new CJS_KeyValue;
				pObjElement->nType = JS_GLOBALDATA_TYPE_OBJECT;
				pObjElement->sKey = sKey;
				ObjectToArray(JS_ToObject(v), pObjElement->objData);
				array.Add(pObjElement);
			}
			break;
		case VT_null:
			{
				CJS_KeyValue* pObjElement = new CJS_KeyValue;
				pObjElement->nType = JS_GLOBALDATA_TYPE_NULL;
				pObjElement->sKey = sKey;
				array.Add(pObjElement);
			}
			break;
		default:
			break;
		}
	}
}

void global_alternate::PutObjectProperty(v8::Handle<v8::Object> pObj, CJS_KeyValue* pData)
{
	ASSERT(pData != NULL);

	for (int i=0,sz=pData->objData.Count(); i<sz; i++)
	{
		CJS_KeyValue* pObjData = pData->objData.GetAt(i);
		ASSERT(pObjData != NULL);

		switch (pObjData->nType)
		{
		case JS_GLOBALDATA_TYPE_NUMBER:
			JS_PutObjectNumber(NULL,(JSObject)pObj, pObjData->sKey.UTF8Decode(), pObjData->dData);
			break;
		case JS_GLOBALDATA_TYPE_BOOLEAN:
			JS_PutObjectBoolean(NULL,(JSObject)pObj, pObjData->sKey.UTF8Decode(), (bool)(pObjData->bData == 1));
			break;
		case JS_GLOBALDATA_TYPE_STRING:
			JS_PutObjectString(NULL,(JSObject)pObj, pObjData->sKey.UTF8Decode(), pObjData->sData.UTF8Decode());
			break;
		case JS_GLOBALDATA_TYPE_OBJECT:
			{
				IJS_Runtime* pRuntime = JS_GetRuntime((JSFXObject)(*m_pJSObject));
				v8::Handle<v8::Object> pNewObj = JS_NewFxDynamicObj(pRuntime, NULL, -1);
				PutObjectProperty(pNewObj, pObjData);
				JS_PutObjectObject(NULL, (JSObject)pObj, pObjData->sKey.UTF8Decode(), (JSObject)pNewObj);
			}
			break;
		case JS_GLOBALDATA_TYPE_NULL:
			JS_PutObjectNull(NULL,(JSObject)pObj, pObjData->sKey.UTF8Decode());
			break;
		}
	}
}

void global_alternate::DestroyGlobalPersisitentVariables()
{
	FX_POSITION	 pos = m_mapGlobal.GetStartPosition();
	while (pos)
	{
		CFX_ByteString name; 
		js_global_data* pData = NULL;
		m_mapGlobal.GetNextAssoc(pos, name, (FX_LPVOID&)pData);
		delete pData;
	}

	m_mapGlobal.RemoveAll();
}


FX_BOOL global_alternate::SetGlobalVariables(FX_LPCSTR propname, int nType, 
				double dData, bool bData, const CFX_ByteString& sData, JSObject pData, bool bDefaultPersistent)
{
	if (propname == NULL) return FALSE;

	js_global_data* pTemp = NULL;
	m_mapGlobal.Lookup(propname, (FX_LPVOID&)pTemp);

	if (pTemp)
	{
		if (pTemp->bDeleted || pTemp->nType != nType)
		{
			pTemp->dData = 0;
			pTemp->bData = 0;
			pTemp->sData = "";
			pTemp->nType = nType;
		}

		pTemp->bDeleted = FALSE;

		switch (nType)
		{
		case JS_GLOBALDATA_TYPE_NUMBER:
			{
				pTemp->dData = dData;
			}
			break;
		case JS_GLOBALDATA_TYPE_BOOLEAN:
			{
				pTemp->bData = bData;
			}
			break;
		case JS_GLOBALDATA_TYPE_STRING:
			{
				pTemp->sData = sData;
			}
			break;
		case JS_GLOBALDATA_TYPE_OBJECT:
			{
				pTemp->pData.Reset(JS_GetRuntime(pData), pData);
			}
			break;
		case JS_GLOBALDATA_TYPE_NULL:
			break;
		default:
			return FALSE;
		}	

		return TRUE;
	}

	js_global_data* pNewData = NULL;

	switch (nType)
	{
	case JS_GLOBALDATA_TYPE_NUMBER:
		{
			pNewData = new js_global_data;
			pNewData->nType = JS_GLOBALDATA_TYPE_NUMBER;
			pNewData->dData = dData;
			pNewData->bPersistent = bDefaultPersistent;
		}
		break;
	case JS_GLOBALDATA_TYPE_BOOLEAN:
		{
			pNewData = new js_global_data;
			pNewData->nType = JS_GLOBALDATA_TYPE_BOOLEAN;
			pNewData->bData = bData;
			pNewData->bPersistent = bDefaultPersistent;
		}
		break;
	case JS_GLOBALDATA_TYPE_STRING:
		{
			pNewData = new js_global_data;
			pNewData->nType = JS_GLOBALDATA_TYPE_STRING;
			pNewData->sData = sData;
			pNewData->bPersistent = bDefaultPersistent;
		}
		break;
	case JS_GLOBALDATA_TYPE_OBJECT:
		{
			pNewData = new js_global_data;
			pNewData->nType = JS_GLOBALDATA_TYPE_OBJECT;
			pNewData->pData.Reset(JS_GetRuntime(pData), pData);
			pNewData->bPersistent = bDefaultPersistent;
		}
		break;
	case JS_GLOBALDATA_TYPE_NULL:
		{
			pNewData = new js_global_data;
			pNewData->nType = JS_GLOBALDATA_TYPE_NULL;
			pNewData->bPersistent = bDefaultPersistent;
		}
		break;
	default:
		return FALSE;
	}	

	m_mapGlobal.SetAt(propname, (FX_LPVOID)pNewData);

	return TRUE;
}

FXJSVALUETYPE GET_VALUE_TYPE(v8::Handle<v8::Value> p)
{
  const unsigned int nHash = JS_CalcHash(JS_GetTypeof(p));

  if (nHash == JSCONST_nUndefHash)
    return VT_undefined;
  if (nHash == JSCONST_nNullHash)
    return VT_null;
  if (nHash == JSCONST_nStringHash)
    return VT_string;
  if (nHash == JSCONST_nNumberHash)
    return VT_number;
  if (nHash == JSCONST_nBoolHash)
    return VT_boolean;
  if (nHash == JSCONST_nDateHash)
    return VT_date;
  if (nHash == JSCONST_nObjectHash)
    return VT_object;
  if (nHash == JSCONST_nFXobjHash)
    return VT_fxobject;

  return VT_unknown;
}

