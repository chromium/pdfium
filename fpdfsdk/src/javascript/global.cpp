// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_Context.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_EventHandler.h"
#include "../../include/javascript/JS_GlobalData.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/global.h"
#include "../../include/javascript/resource.h"

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

const unsigned int JSCONST_nStringHash =
    CHash<'s', 't', 'r', 'i', 'n', 'g'>::value;
const unsigned int JSCONST_nNumberHash =
    CHash<'n', 'u', 'm', 'b', 'e', 'r'>::value;
const unsigned int JSCONST_nBoolHash =
    CHash<'b', 'o', 'o', 'l', 'e', 'a', 'n'>::value;
const unsigned int JSCONST_nDateHash = CHash<'d', 'a', 't', 'e'>::value;
const unsigned int JSCONST_nObjectHash =
    CHash<'o', 'b', 'j', 'e', 'c', 't'>::value;
const unsigned int JSCONST_nFXobjHash = CHash<'f', 'x', 'o', 'b', 'j'>::value;
const unsigned int JSCONST_nNullHash = CHash<'n', 'u', 'l', 'l'>::value;
const unsigned int JSCONST_nUndefHash =
    CHash<'u', 'n', 'd', 'e', 'f', 'i', 'n', 'e', 'd'>::value;

#ifdef _DEBUG
class HashVerify {
 public:
  HashVerify();
} g_hashVerify;

HashVerify::HashVerify() {
  ASSERT(JSCONST_nStringHash ==
         JS_CalcHash(VALUE_NAME_STRING, wcslen(VALUE_NAME_STRING)));
  ASSERT(JSCONST_nNumberHash ==
         JS_CalcHash(VALUE_NAME_NUMBER, wcslen(VALUE_NAME_NUMBER)));
  ASSERT(JSCONST_nBoolHash ==
         JS_CalcHash(VALUE_NAME_BOOLEAN, wcslen(VALUE_NAME_BOOLEAN)));
  ASSERT(JSCONST_nDateHash ==
         JS_CalcHash(VALUE_NAME_DATE, wcslen(VALUE_NAME_DATE)));
  ASSERT(JSCONST_nObjectHash ==
         JS_CalcHash(VALUE_NAME_OBJECT, wcslen(VALUE_NAME_OBJECT)));
  ASSERT(JSCONST_nFXobjHash ==
         JS_CalcHash(VALUE_NAME_FXOBJ, wcslen(VALUE_NAME_FXOBJ)));
  ASSERT(JSCONST_nNullHash ==
         JS_CalcHash(VALUE_NAME_NULL, wcslen(VALUE_NAME_NULL)));
  ASSERT(JSCONST_nUndefHash ==
         JS_CalcHash(VALUE_NAME_UNDEFINED, wcslen(VALUE_NAME_UNDEFINED)));
}
#endif

BEGIN_JS_STATIC_CONST(CJS_Global)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_Global)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_Global)
JS_STATIC_METHOD_ENTRY(setPersistent)
END_JS_STATIC_METHOD()

IMPLEMENT_SPECIAL_JS_CLASS(CJS_Global, JSGlobalAlternate, global);

FX_BOOL CJS_Global::InitInstance(IFXJS_Context* cc) {
  CJS_Context* pContext = (CJS_Context*)cc;
  ASSERT(pContext != NULL);

  JSGlobalAlternate* pGlobal = (JSGlobalAlternate*)GetEmbedObject();
  ASSERT(pGlobal != NULL);

  pGlobal->Initial(pContext->GetReaderApp());

  return TRUE;
};

JSGlobalAlternate::JSGlobalAlternate(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject), m_pApp(NULL) {
}

JSGlobalAlternate::~JSGlobalAlternate() {
  DestroyGlobalPersisitentVariables();
  m_pApp->GetRuntimeFactory()->ReleaseGlobalData();
}

void JSGlobalAlternate::Initial(CPDFDoc_Environment* pApp) {
  m_pApp = pApp;
  m_pGlobalData = pApp->GetRuntimeFactory()->NewGlobalData(pApp);
  UpdateGlobalPersistentVariables();
}

FX_BOOL JSGlobalAlternate::QueryProperty(const FX_WCHAR* propname) {
  return CFX_WideString(propname) != L"setPersistent";
}

FX_BOOL JSGlobalAlternate::DelProperty(IFXJS_Context* cc,
                                       const FX_WCHAR* propname,
                                       CFX_WideString& sError) {
  auto it = m_mapGlobal.find(CFX_ByteString::FromUnicode(propname));
  if (it == m_mapGlobal.end())
    return FALSE;

  it->second->bDeleted = TRUE;
  return TRUE;
}

FX_BOOL JSGlobalAlternate::DoProperty(IFXJS_Context* cc,
                                      const FX_WCHAR* propname,
                                      CJS_PropValue& vp,
                                      CFX_WideString& sError) {
  if (vp.IsSetting()) {
    CFX_ByteString sPropName = CFX_ByteString::FromUnicode(propname);
    switch (vp.GetType()) {
      case VT_number: {
        double dData;
        vp >> dData;
        return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_NUMBER, dData,
                                  false, "", v8::Local<v8::Object>(), FALSE);
      }
      case VT_boolean: {
        bool bData;
        vp >> bData;
        return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_BOOLEAN, 0,
                                  bData, "", v8::Local<v8::Object>(), FALSE);
      }
      case VT_string: {
        CFX_ByteString sData;
        vp >> sData;
        return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_STRING, 0,
                                  false, sData, v8::Local<v8::Object>(), FALSE);
      }
      case VT_object: {
        JSObject pData;
        vp >> pData;
        return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_OBJECT, 0,
                                  false, "", pData, FALSE);
      }
      case VT_null: {
        return SetGlobalVariables(sPropName, JS_GLOBALDATA_TYPE_NULL, 0, false,
                                  "", v8::Local<v8::Object>(), FALSE);
      }
      case VT_undefined: {
        DelProperty(cc, propname, sError);
        return TRUE;
      }
      default:
        break;
    }
  } else {
    auto it = m_mapGlobal.find(CFX_ByteString::FromUnicode(propname));
    if (it == m_mapGlobal.end()) {
      vp.SetNull();
      return TRUE;
    }
    JSGlobalData* pData = it->second;
    if (pData->bDeleted) {
      vp.SetNull();
      return TRUE;
    }
    switch (pData->nType) {
      case JS_GLOBALDATA_TYPE_NUMBER:
        vp << pData->dData;
        return TRUE;
      case JS_GLOBALDATA_TYPE_BOOLEAN:
        vp << pData->bData;
        return TRUE;
      case JS_GLOBALDATA_TYPE_STRING:
        vp << pData->sData;
        return TRUE;
      case JS_GLOBALDATA_TYPE_OBJECT: {
        v8::Local<v8::Object> obj =
            v8::Local<v8::Object>::New(vp.GetIsolate(), pData->pData);
        vp << obj;
        return TRUE;
      }
      case JS_GLOBALDATA_TYPE_NULL:
        vp.SetNull();
        return TRUE;
      default:
        break;
    }
  }
  return FALSE;
}

FX_BOOL JSGlobalAlternate::setPersistent(IFXJS_Context* cc,
                                         const CJS_Parameters& params,
                                         CJS_Value& vRet,
                                         CFX_WideString& sError) {
  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  if (params.size() != 2) {
    sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  auto it = m_mapGlobal.find(params[0].ToCFXByteString());
  if (it != m_mapGlobal.end()) {
    JSGlobalData* pData = it->second;
    if (!pData->bDeleted) {
      pData->bPersistent = params[1].ToBool();
      return TRUE;
    }
  }

  sError = JSGetStringFromID(pContext, IDS_STRING_JSNOGLOBAL);
  return FALSE;
}

void JSGlobalAlternate::UpdateGlobalPersistentVariables() {
  ASSERT(m_pGlobalData != NULL);

  for (int i = 0, sz = m_pGlobalData->GetSize(); i < sz; i++) {
    CJS_GlobalData_Element* pData = m_pGlobalData->GetAt(i);
    ASSERT(pData != NULL);

    switch (pData->data.nType) {
      case JS_GLOBALDATA_TYPE_NUMBER:
        SetGlobalVariables(pData->data.sKey, JS_GLOBALDATA_TYPE_NUMBER,
                           pData->data.dData, false, "",
                           v8::Local<v8::Object>(), pData->bPersistent == 1);
        JS_PutObjectNumber(NULL, (JSFXObject)(*m_pJSObject),
                           pData->data.sKey.UTF8Decode().c_str(),
                           pData->data.dData);
        break;
      case JS_GLOBALDATA_TYPE_BOOLEAN:
        SetGlobalVariables(pData->data.sKey, JS_GLOBALDATA_TYPE_BOOLEAN, 0,
                           (bool)(pData->data.bData == 1), "",
                           v8::Local<v8::Object>(), pData->bPersistent == 1);
        JS_PutObjectBoolean(NULL, (JSFXObject)(*m_pJSObject),
                            pData->data.sKey.UTF8Decode().c_str(),
                            (bool)(pData->data.bData == 1));
        break;
      case JS_GLOBALDATA_TYPE_STRING:
        SetGlobalVariables(pData->data.sKey, JS_GLOBALDATA_TYPE_STRING, 0,
                           false, pData->data.sData, v8::Local<v8::Object>(),
                           pData->bPersistent == 1);
        JS_PutObjectString(NULL, (JSFXObject)(*m_pJSObject),
                           pData->data.sKey.UTF8Decode().c_str(),
                           pData->data.sData.UTF8Decode().c_str());
        break;
      case JS_GLOBALDATA_TYPE_OBJECT: {
        IJS_Runtime* pRuntime = JS_GetRuntime((JSFXObject)(*m_pJSObject));
        v8::Local<v8::Object> pObj = JS_NewFxDynamicObj(pRuntime, NULL, -1);

        PutObjectProperty(pObj, &pData->data);

        SetGlobalVariables(pData->data.sKey, JS_GLOBALDATA_TYPE_OBJECT, 0,
                           false, "", (JSObject)pObj, pData->bPersistent == 1);
        JS_PutObjectObject(NULL, (JSFXObject)(*m_pJSObject),
                           pData->data.sKey.UTF8Decode().c_str(),
                           (JSObject)pObj);
      } break;
      case JS_GLOBALDATA_TYPE_NULL:
        SetGlobalVariables(pData->data.sKey, JS_GLOBALDATA_TYPE_NULL, 0, false,
                           "", v8::Local<v8::Object>(),
                           pData->bPersistent == 1);
        JS_PutObjectNull(NULL, (JSFXObject)(*m_pJSObject),
                         pData->data.sKey.UTF8Decode().c_str());
        break;
    }
  }
}

void JSGlobalAlternate::CommitGlobalPersisitentVariables() {
  ASSERT(m_pGlobalData);
  for (auto it = m_mapGlobal.begin(); it != m_mapGlobal.end(); ++it) {
    CFX_ByteString name = it->first;
    JSGlobalData* pData = it->second;
    if (pData->bDeleted) {
      m_pGlobalData->DeleteGlobalVariable(name);
    } else {
      switch (pData->nType) {
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
          // if (pData->pData)
          {
            CJS_GlobalVariableArray array;
            v8::Local<v8::Object> obj = v8::Local<v8::Object>::New(
                GetJSObject()->GetIsolate(), pData->pData);
            ObjectToArray(obj, array);
            m_pGlobalData->SetGlobalVariableObject(name, array);
            m_pGlobalData->SetGlobalVariablePersistent(name,
                                                       pData->bPersistent);
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

void JSGlobalAlternate::ObjectToArray(v8::Local<v8::Object> pObj,
                                      CJS_GlobalVariableArray& array) {
  v8::Local<v8::Context> context = pObj->CreationContext();
  v8::Isolate* isolate = context->GetIsolate();
  v8::Local<v8::Array> pKeyList = JS_GetObjectElementNames(isolate, pObj);
  int nObjElements = pKeyList->Length();

  for (int i = 0; i < nObjElements; i++) {
    CFX_WideString ws =
        JS_ToString(isolate, JS_GetArrayElement(isolate, pKeyList, i));
    CFX_ByteString sKey = ws.UTF8Encode();

    v8::Local<v8::Value> v = JS_GetObjectElement(isolate, pObj, ws.c_str());
    FXJSVALUETYPE vt = GET_VALUE_TYPE(v);
    switch (vt) {
      case VT_number: {
        CJS_KeyValue* pObjElement = new CJS_KeyValue;
        pObjElement->nType = JS_GLOBALDATA_TYPE_NUMBER;
        pObjElement->sKey = sKey;
        pObjElement->dData = JS_ToNumber(isolate, v);
        array.Add(pObjElement);
      } break;
      case VT_boolean: {
        CJS_KeyValue* pObjElement = new CJS_KeyValue;
        pObjElement->nType = JS_GLOBALDATA_TYPE_BOOLEAN;
        pObjElement->sKey = sKey;
        pObjElement->dData = JS_ToBoolean(isolate, v);
        array.Add(pObjElement);
      } break;
      case VT_string: {
        CFX_ByteString sValue =
            CJS_Value(isolate, v, VT_string).ToCFXByteString();
        CJS_KeyValue* pObjElement = new CJS_KeyValue;
        pObjElement->nType = JS_GLOBALDATA_TYPE_STRING;
        pObjElement->sKey = sKey;
        pObjElement->sData = sValue;
        array.Add(pObjElement);
      } break;
      case VT_object: {
        CJS_KeyValue* pObjElement = new CJS_KeyValue;
        pObjElement->nType = JS_GLOBALDATA_TYPE_OBJECT;
        pObjElement->sKey = sKey;
        ObjectToArray(JS_ToObject(isolate, v), pObjElement->objData);
        array.Add(pObjElement);
      } break;
      case VT_null: {
        CJS_KeyValue* pObjElement = new CJS_KeyValue;
        pObjElement->nType = JS_GLOBALDATA_TYPE_NULL;
        pObjElement->sKey = sKey;
        array.Add(pObjElement);
      } break;
      default:
        break;
    }
  }
}

void JSGlobalAlternate::PutObjectProperty(v8::Local<v8::Object> pObj,
                                          CJS_KeyValue* pData) {
  ASSERT(pData != NULL);

  for (int i = 0, sz = pData->objData.Count(); i < sz; i++) {
    CJS_KeyValue* pObjData = pData->objData.GetAt(i);
    ASSERT(pObjData != NULL);

    switch (pObjData->nType) {
      case JS_GLOBALDATA_TYPE_NUMBER:
        JS_PutObjectNumber(NULL, (JSObject)pObj,
                           pObjData->sKey.UTF8Decode().c_str(),
                           pObjData->dData);
        break;
      case JS_GLOBALDATA_TYPE_BOOLEAN:
        JS_PutObjectBoolean(NULL, (JSObject)pObj,
                            pObjData->sKey.UTF8Decode().c_str(),
                            (bool)(pObjData->bData == 1));
        break;
      case JS_GLOBALDATA_TYPE_STRING:
        JS_PutObjectString(NULL, (JSObject)pObj,
                           pObjData->sKey.UTF8Decode().c_str(),
                           pObjData->sData.UTF8Decode().c_str());
        break;
      case JS_GLOBALDATA_TYPE_OBJECT: {
        IJS_Runtime* pRuntime = JS_GetRuntime((JSFXObject)(*m_pJSObject));
        v8::Local<v8::Object> pNewObj = JS_NewFxDynamicObj(pRuntime, NULL, -1);
        PutObjectProperty(pNewObj, pObjData);
        JS_PutObjectObject(NULL, (JSObject)pObj,
                           pObjData->sKey.UTF8Decode().c_str(),
                           (JSObject)pNewObj);
      } break;
      case JS_GLOBALDATA_TYPE_NULL:
        JS_PutObjectNull(NULL, (JSObject)pObj,
                         pObjData->sKey.UTF8Decode().c_str());
        break;
    }
  }
}

void JSGlobalAlternate::DestroyGlobalPersisitentVariables() {
  for (const auto& pair : m_mapGlobal) {
    delete pair.second;
  }
  m_mapGlobal.clear();
}

FX_BOOL JSGlobalAlternate::SetGlobalVariables(const FX_CHAR* propname,
                                              int nType,
                                              double dData,
                                              bool bData,
                                              const CFX_ByteString& sData,
                                              JSObject pData,
                                              bool bDefaultPersistent) {
  if (!propname)
    return FALSE;

  auto it = m_mapGlobal.find(propname);
  if (it != m_mapGlobal.end()) {
    JSGlobalData* pTemp = it->second;
    if (pTemp->bDeleted || pTemp->nType != nType) {
      pTemp->dData = 0;
      pTemp->bData = 0;
      pTemp->sData = "";
      pTemp->nType = nType;
    }

    pTemp->bDeleted = FALSE;
    switch (nType) {
      case JS_GLOBALDATA_TYPE_NUMBER: {
        pTemp->dData = dData;
      } break;
      case JS_GLOBALDATA_TYPE_BOOLEAN: {
        pTemp->bData = bData;
      } break;
      case JS_GLOBALDATA_TYPE_STRING: {
        pTemp->sData = sData;
      } break;
      case JS_GLOBALDATA_TYPE_OBJECT: {
        pTemp->pData.Reset(JS_GetRuntime(pData), pData);
      } break;
      case JS_GLOBALDATA_TYPE_NULL:
        break;
      default:
        return FALSE;
    }
    return TRUE;
  }

  JSGlobalData* pNewData = NULL;

  switch (nType) {
    case JS_GLOBALDATA_TYPE_NUMBER: {
      pNewData = new JSGlobalData;
      pNewData->nType = JS_GLOBALDATA_TYPE_NUMBER;
      pNewData->dData = dData;
      pNewData->bPersistent = bDefaultPersistent;
    } break;
    case JS_GLOBALDATA_TYPE_BOOLEAN: {
      pNewData = new JSGlobalData;
      pNewData->nType = JS_GLOBALDATA_TYPE_BOOLEAN;
      pNewData->bData = bData;
      pNewData->bPersistent = bDefaultPersistent;
    } break;
    case JS_GLOBALDATA_TYPE_STRING: {
      pNewData = new JSGlobalData;
      pNewData->nType = JS_GLOBALDATA_TYPE_STRING;
      pNewData->sData = sData;
      pNewData->bPersistent = bDefaultPersistent;
    } break;
    case JS_GLOBALDATA_TYPE_OBJECT: {
      pNewData = new JSGlobalData;
      pNewData->nType = JS_GLOBALDATA_TYPE_OBJECT;
      pNewData->pData.Reset(JS_GetRuntime(pData), pData);
      pNewData->bPersistent = bDefaultPersistent;
    } break;
    case JS_GLOBALDATA_TYPE_NULL: {
      pNewData = new JSGlobalData;
      pNewData->nType = JS_GLOBALDATA_TYPE_NULL;
      pNewData->bPersistent = bDefaultPersistent;
    } break;
    default:
      return FALSE;
  }

  m_mapGlobal[propname] = pNewData;
  return TRUE;
}

FXJSVALUETYPE GET_VALUE_TYPE(v8::Local<v8::Value> p) {
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
