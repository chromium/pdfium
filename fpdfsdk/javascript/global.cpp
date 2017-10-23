// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/global.h"

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_extension.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_GlobalData.h"
#include "fpdfsdk/javascript/JS_KeyValue.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_event_context.h"
#include "fpdfsdk/javascript/resource.h"

#define IMPLEMENT_SPECIAL_JS_CLASS(js_class_name, class_alternate, class_name) \
  IMPLEMENT_JS_CLASS_BASE_PART(js_class_name, class_name)                      \
  IMPLEMENT_JS_CLASS_CONST_PART(js_class_name, class_name)                     \
  IMPLEMENT_JS_CLASS_PART(js_class_name, class_alternate, class_name)          \
  void js_class_name::queryprop_static(                                        \
      v8::Local<v8::String> property,                                          \
      const v8::PropertyCallbackInfo<v8::Integer>& info) {                     \
    JSSpecialPropQuery<class_alternate>(#class_name, property, info);          \
  }                                                                            \
  void js_class_name::getprop_static(                                          \
      v8::Local<v8::String> property,                                          \
      const v8::PropertyCallbackInfo<v8::Value>& info) {                       \
    JSSpecialPropGet<class_alternate>(#class_name, property, info);            \
  }                                                                            \
  void js_class_name::putprop_static(                                          \
      v8::Local<v8::String> property, v8::Local<v8::Value> value,              \
      const v8::PropertyCallbackInfo<v8::Value>& info) {                       \
    JSSpecialPropPut<class_alternate>(#class_name, property, value, info);     \
  }                                                                            \
  void js_class_name::delprop_static(                                          \
      v8::Local<v8::String> property,                                          \
      const v8::PropertyCallbackInfo<v8::Boolean>& info) {                     \
    JSSpecialPropDel<class_alternate>(#class_name, property, info);            \
  }                                                                            \
  void js_class_name::DefineAllProperties(CFXJS_Engine* pEngine) {             \
    pEngine->DefineObjAllProperties(                                           \
        g_nObjDefnID, js_class_name::queryprop_static,                         \
        js_class_name::getprop_static, js_class_name::putprop_static,          \
        js_class_name::delprop_static);                                        \
  }                                                                            \
  void js_class_name::DefineJSObjects(CFXJS_Engine* pEngine,                   \
                                      FXJSOBJTYPE eObjType) {                  \
    g_nObjDefnID = pEngine->DefineObj(js_class_name::g_pClassName, eObjType,   \
                                      JSConstructor, JSDestructor);            \
    DefineConsts(pEngine);                                                     \
    DefineProps(pEngine);                                                      \
    DefineMethods(pEngine);                                                    \
    DefineAllProperties(pEngine);                                              \
  }

namespace {

template <class Alt>
void JSSpecialPropQuery(const char*,
                        v8::Local<v8::String> property,
                        const v8::PropertyCallbackInfo<v8::Integer>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  v8::String::Utf8Value utf8_value(property);
  WideString propname =
      WideString::FromUTF8(ByteStringView(*utf8_value, utf8_value.length()));
  bool bRet = pObj->QueryProperty(propname.c_str());
  info.GetReturnValue().Set(bRet ? 4 : 0);
}

template <class Alt>
void JSSpecialPropGet(const char* class_name,
                      v8::Local<v8::String> property,
                      const v8::PropertyCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  v8::String::Utf8Value utf8_value(property);
  WideString propname =
      WideString::FromUTF8(ByteStringView(*utf8_value, utf8_value.length()));

  CJS_Value value(pRuntime);
  if (!pObj->GetProperty(pRuntime, propname.c_str(), &value)) {
    pRuntime->Error(JSFormatErrorString(class_name, "GetProperty", L""));
    return;
  }
  info.GetReturnValue().Set(value.ToV8Value(pRuntime));
}

template <class Alt>
void JSSpecialPropPut(const char* class_name,
                      v8::Local<v8::String> property,
                      v8::Local<v8::Value> value,
                      const v8::PropertyCallbackInfo<v8::Value>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  v8::String::Utf8Value utf8_value(property);
  WideString propname =
      WideString::FromUTF8(ByteStringView(*utf8_value, utf8_value.length()));
  CJS_Value prop_value(pRuntime, value);
  if (!pObj->SetProperty(pRuntime, propname.c_str(), prop_value)) {
    pRuntime->Error(JSFormatErrorString(class_name, "PutProperty", L""));
  }
}

template <class Alt>
void JSSpecialPropDel(const char* class_name,
                      v8::Local<v8::String> property,
                      const v8::PropertyCallbackInfo<v8::Boolean>& info) {
  CJS_Runtime* pRuntime =
      CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());
  if (!pRuntime)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(info.Holder()));
  if (!pJSObj)
    return;

  Alt* pObj = reinterpret_cast<Alt*>(pJSObj->GetEmbedObject());
  v8::String::Utf8Value utf8_value(property);
  WideString propname =
      WideString::FromUTF8(ByteStringView(*utf8_value, utf8_value.length()));
  if (!pObj->DelProperty(pRuntime, propname.c_str())) {
    ByteString cbName;
    cbName.Format("%s.%s", class_name, "DelProperty");
    // Probably a missing call to JSFX_Error().
  }
}

struct JSGlobalData {
  JSGlobalData();
  ~JSGlobalData();

  JS_GlobalDataType nType;
  double dData;
  bool bData;
  ByteString sData;
  v8::Global<v8::Object> pData;
  bool bPersistent;
  bool bDeleted;
};

class JSGlobalAlternate : public CJS_EmbedObj {
 public:
  explicit JSGlobalAlternate(CJS_Object* pJSObject);
  ~JSGlobalAlternate() override;

  bool setPersistent(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError);
  bool QueryProperty(const wchar_t* propname);
  bool GetProperty(CJS_Runtime* pRuntime,
                   const wchar_t* propname,
                   CJS_Value* vp);
  bool SetProperty(CJS_Runtime* pRuntime,
                   const wchar_t* propname,
                   const CJS_Value& vp);
  bool DelProperty(CJS_Runtime* pRuntime, const wchar_t* propname);
  void Initial(CPDFSDK_FormFillEnvironment* pFormFillEnv);

 private:
  void UpdateGlobalPersistentVariables();
  void CommitGlobalPersisitentVariables(CJS_Runtime* pRuntime);
  void DestroyGlobalPersisitentVariables();
  bool SetGlobalVariables(const ByteString& propname,
                          JS_GlobalDataType nType,
                          double dData,
                          bool bData,
                          const ByteString& sData,
                          v8::Local<v8::Object> pData,
                          bool bDefaultPersistent);
  void ObjectToArray(CJS_Runtime* pRuntime,
                     v8::Local<v8::Object> pObj,
                     CJS_GlobalVariableArray& array);
  void PutObjectProperty(v8::Local<v8::Object> obj, CJS_KeyValue* pData);

  std::map<ByteString, std::unique_ptr<JSGlobalData>> m_MapGlobal;
  WideString m_sFilePath;
  CJS_GlobalData* m_pGlobalData;
  CPDFSDK_FormFillEnvironment::ObservedPtr m_pFormFillEnv;
};

}  // namespace

JSConstSpec CJS_Global::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Global::PropertySpecs[] = {{0, 0, 0}};

JSMethodSpec CJS_Global::MethodSpecs[] = {
    {"setPersistent", setPersistent_static},
    {0, 0}};

IMPLEMENT_SPECIAL_JS_CLASS(CJS_Global, JSGlobalAlternate, global);

// static
void CJS_Global::setPersistent_static(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  JSMethod<JSGlobalAlternate, &JSGlobalAlternate::setPersistent>(
      "setPersistent", "global", info);
}

void CJS_Global::InitInstance(IJS_Runtime* pIRuntime) {
  CJS_Runtime* pRuntime = static_cast<CJS_Runtime*>(pIRuntime);
  JSGlobalAlternate* pGlobal =
      static_cast<JSGlobalAlternate*>(GetEmbedObject());
  pGlobal->Initial(pRuntime->GetFormFillEnv());
}

JSGlobalData::JSGlobalData()
    : nType(JS_GlobalDataType::NUMBER),
      dData(0),
      bData(false),
      sData(""),
      bPersistent(false),
      bDeleted(false) {}

JSGlobalData::~JSGlobalData() {
  pData.Reset();
}

JSGlobalAlternate::JSGlobalAlternate(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject), m_pFormFillEnv(nullptr) {}

JSGlobalAlternate::~JSGlobalAlternate() {
  DestroyGlobalPersisitentVariables();
  m_pGlobalData->Release();
}

void JSGlobalAlternate::Initial(CPDFSDK_FormFillEnvironment* pFormFillEnv) {
  m_pFormFillEnv.Reset(pFormFillEnv);
  m_pGlobalData = CJS_GlobalData::GetRetainedInstance(pFormFillEnv);
  UpdateGlobalPersistentVariables();
}

bool JSGlobalAlternate::QueryProperty(const wchar_t* propname) {
  return WideString(propname) != L"setPersistent";
}

bool JSGlobalAlternate::DelProperty(CJS_Runtime* pRuntime,
                                    const wchar_t* propname) {
  auto it = m_MapGlobal.find(ByteString::FromUnicode(propname));
  if (it == m_MapGlobal.end())
    return false;

  it->second->bDeleted = true;
  return true;
}

bool JSGlobalAlternate::GetProperty(CJS_Runtime* pRuntime,
                                    const wchar_t* propname,
                                    CJS_Value* vp) {
  auto it = m_MapGlobal.find(ByteString::FromUnicode(propname));
  if (it == m_MapGlobal.end()) {
    vp->SetNull(pRuntime);
    return true;
  }

  JSGlobalData* pData = it->second.get();
  if (pData->bDeleted) {
    vp->SetNull(pRuntime);
    return true;
  }

  switch (pData->nType) {
    case JS_GlobalDataType::NUMBER:
      vp->Set(pRuntime, pData->dData);
      return true;
    case JS_GlobalDataType::BOOLEAN:
      vp->Set(pRuntime, pData->bData);
      return true;
    case JS_GlobalDataType::STRING:
      vp->Set(pRuntime, pData->sData);
      return true;
    case JS_GlobalDataType::OBJECT: {
      vp->Set(pRuntime,
              v8::Local<v8::Object>::New(pRuntime->GetIsolate(), pData->pData));
      return true;
    }
    case JS_GlobalDataType::NULLOBJ:
      vp->SetNull(pRuntime);
      return true;
    default:
      break;
  }
  return false;
}

bool JSGlobalAlternate::SetProperty(CJS_Runtime* pRuntime,
                                    const wchar_t* propname,
                                    const CJS_Value& vp) {
  ByteString sPropName = ByteString::FromUnicode(propname);
  switch (vp.GetType()) {
    case CJS_Value::VT_number:
      return SetGlobalVariables(sPropName, JS_GlobalDataType::NUMBER,
                                vp.ToDouble(pRuntime), false, "",
                                v8::Local<v8::Object>(), false);
    case CJS_Value::VT_boolean:
      return SetGlobalVariables(sPropName, JS_GlobalDataType::BOOLEAN, 0,
                                vp.ToBool(pRuntime), "",
                                v8::Local<v8::Object>(), false);
    case CJS_Value::VT_string:
      return SetGlobalVariables(sPropName, JS_GlobalDataType::STRING, 0, false,
                                vp.ToByteString(pRuntime),
                                v8::Local<v8::Object>(), false);
    case CJS_Value::VT_object:
      return SetGlobalVariables(sPropName, JS_GlobalDataType::OBJECT, 0, false,
                                "", vp.ToV8Object(pRuntime), false);
    case CJS_Value::VT_null:
      return SetGlobalVariables(sPropName, JS_GlobalDataType::NULLOBJ, 0, false,
                                "", v8::Local<v8::Object>(), false);
    case CJS_Value::VT_undefined:
      DelProperty(pRuntime, propname);
      return true;
    default:
      break;
  }
  return false;
}

bool JSGlobalAlternate::setPersistent(CJS_Runtime* pRuntime,
                                      const std::vector<CJS_Value>& params,
                                      CJS_Value& vRet,
                                      WideString& sError) {
  if (params.size() != 2) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return false;
  }
  auto it = m_MapGlobal.find(params[0].ToByteString(pRuntime));
  if (it == m_MapGlobal.end() || it->second->bDeleted) {
    sError = JSGetStringFromID(IDS_STRING_JSNOGLOBAL);
    return false;
  }
  it->second->bPersistent = params[1].ToBool(pRuntime);
  return true;
}

void JSGlobalAlternate::UpdateGlobalPersistentVariables() {
  CJS_Runtime* pRuntime =
      static_cast<CJS_Runtime*>(CFXJS_Engine::CurrentEngineFromIsolate(
          m_pJSObject->ToV8Object()->GetIsolate()));

  for (int i = 0, sz = m_pGlobalData->GetSize(); i < sz; i++) {
    CJS_GlobalData_Element* pData = m_pGlobalData->GetAt(i);
    switch (pData->data.nType) {
      case JS_GlobalDataType::NUMBER:
        SetGlobalVariables(pData->data.sKey, JS_GlobalDataType::NUMBER,
                           pData->data.dData, false, "",
                           v8::Local<v8::Object>(), pData->bPersistent == 1);
        pRuntime->PutObjectProperty(m_pJSObject->ToV8Object(),
                                    pData->data.sKey.UTF8Decode(),
                                    pRuntime->NewNumber(pData->data.dData));
        break;
      case JS_GlobalDataType::BOOLEAN:
        SetGlobalVariables(pData->data.sKey, JS_GlobalDataType::BOOLEAN, 0,
                           pData->data.bData == 1, "", v8::Local<v8::Object>(),
                           pData->bPersistent == 1);
        pRuntime->PutObjectProperty(
            m_pJSObject->ToV8Object(), pData->data.sKey.UTF8Decode(),
            pRuntime->NewBoolean(pData->data.bData == 1));
        break;
      case JS_GlobalDataType::STRING:
        SetGlobalVariables(pData->data.sKey, JS_GlobalDataType::STRING, 0,
                           false, pData->data.sData, v8::Local<v8::Object>(),
                           pData->bPersistent == 1);
        pRuntime->PutObjectProperty(
            m_pJSObject->ToV8Object(), pData->data.sKey.UTF8Decode(),
            pRuntime->NewString(pData->data.sData.UTF8Decode().AsStringView()));
        break;
      case JS_GlobalDataType::OBJECT: {
        v8::Local<v8::Object> pObj = pRuntime->NewFxDynamicObj(-1);
        if (!pObj.IsEmpty()) {
          PutObjectProperty(pObj, &pData->data);
          SetGlobalVariables(pData->data.sKey, JS_GlobalDataType::OBJECT, 0,
                             false, "", pObj, pData->bPersistent == 1);
          pRuntime->PutObjectProperty(m_pJSObject->ToV8Object(),
                                      pData->data.sKey.UTF8Decode(), pObj);
        }
      } break;
      case JS_GlobalDataType::NULLOBJ:
        SetGlobalVariables(pData->data.sKey, JS_GlobalDataType::NULLOBJ, 0,
                           false, "", v8::Local<v8::Object>(),
                           pData->bPersistent == 1);
        pRuntime->PutObjectProperty(m_pJSObject->ToV8Object(),
                                    pData->data.sKey.UTF8Decode(),
                                    pRuntime->NewNull());
        break;
    }
  }
}

void JSGlobalAlternate::CommitGlobalPersisitentVariables(
    CJS_Runtime* pRuntime) {
  for (const auto& iter : m_MapGlobal) {
    ByteString name = iter.first;
    JSGlobalData* pData = iter.second.get();
    if (pData->bDeleted) {
      m_pGlobalData->DeleteGlobalVariable(name);
      continue;
    }
    switch (pData->nType) {
      case JS_GlobalDataType::NUMBER:
        m_pGlobalData->SetGlobalVariableNumber(name, pData->dData);
        m_pGlobalData->SetGlobalVariablePersistent(name, pData->bPersistent);
        break;
      case JS_GlobalDataType::BOOLEAN:
        m_pGlobalData->SetGlobalVariableBoolean(name, pData->bData);
        m_pGlobalData->SetGlobalVariablePersistent(name, pData->bPersistent);
        break;
      case JS_GlobalDataType::STRING:
        m_pGlobalData->SetGlobalVariableString(name, pData->sData);
        m_pGlobalData->SetGlobalVariablePersistent(name, pData->bPersistent);
        break;
      case JS_GlobalDataType::OBJECT: {
        CJS_GlobalVariableArray array;
        v8::Local<v8::Object> obj = v8::Local<v8::Object>::New(
            GetJSObject()->GetIsolate(), pData->pData);
        ObjectToArray(pRuntime, obj, array);
        m_pGlobalData->SetGlobalVariableObject(name, array);
        m_pGlobalData->SetGlobalVariablePersistent(name, pData->bPersistent);
      } break;
      case JS_GlobalDataType::NULLOBJ:
        m_pGlobalData->SetGlobalVariableNull(name);
        m_pGlobalData->SetGlobalVariablePersistent(name, pData->bPersistent);
        break;
    }
  }
}

void JSGlobalAlternate::ObjectToArray(CJS_Runtime* pRuntime,
                                      v8::Local<v8::Object> pObj,
                                      CJS_GlobalVariableArray& array) {
  std::vector<WideString> pKeyList = pRuntime->GetObjectPropertyNames(pObj);
  for (const auto& ws : pKeyList) {
    ByteString sKey = ws.UTF8Encode();
    v8::Local<v8::Value> v = pRuntime->GetObjectProperty(pObj, ws);
    switch (CJS_Value::GetValueType(v)) {
      case CJS_Value::VT_number: {
        CJS_KeyValue* pObjElement = new CJS_KeyValue;
        pObjElement->nType = JS_GlobalDataType::NUMBER;
        pObjElement->sKey = sKey;
        pObjElement->dData = pRuntime->ToDouble(v);
        array.Add(pObjElement);
      } break;
      case CJS_Value::VT_boolean: {
        CJS_KeyValue* pObjElement = new CJS_KeyValue;
        pObjElement->nType = JS_GlobalDataType::BOOLEAN;
        pObjElement->sKey = sKey;
        pObjElement->dData = pRuntime->ToBoolean(v);
        array.Add(pObjElement);
      } break;
      case CJS_Value::VT_string: {
        ByteString sValue = CJS_Value(pRuntime, v).ToByteString(pRuntime);
        CJS_KeyValue* pObjElement = new CJS_KeyValue;
        pObjElement->nType = JS_GlobalDataType::STRING;
        pObjElement->sKey = sKey;
        pObjElement->sData = sValue;
        array.Add(pObjElement);
      } break;
      case CJS_Value::VT_object: {
        CJS_KeyValue* pObjElement = new CJS_KeyValue;
        pObjElement->nType = JS_GlobalDataType::OBJECT;
        pObjElement->sKey = sKey;
        ObjectToArray(pRuntime, pRuntime->ToObject(v), pObjElement->objData);
        array.Add(pObjElement);
      } break;
      case CJS_Value::VT_null: {
        CJS_KeyValue* pObjElement = new CJS_KeyValue;
        pObjElement->nType = JS_GlobalDataType::NULLOBJ;
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
  CJS_Runtime* pRuntime = CJS_Runtime::CurrentRuntimeFromIsolate(
      m_pJSObject->ToV8Object()->GetIsolate());

  for (int i = 0, sz = pData->objData.Count(); i < sz; i++) {
    CJS_KeyValue* pObjData = pData->objData.GetAt(i);
    switch (pObjData->nType) {
      case JS_GlobalDataType::NUMBER:
        pRuntime->PutObjectProperty(pObj, pObjData->sKey.UTF8Decode(),
                                    pRuntime->NewNumber(pObjData->dData));
        break;
      case JS_GlobalDataType::BOOLEAN:
        pRuntime->PutObjectProperty(pObj, pObjData->sKey.UTF8Decode(),
                                    pRuntime->NewBoolean(pObjData->bData == 1));
        break;
      case JS_GlobalDataType::STRING:
        pRuntime->PutObjectProperty(
            pObj, pObjData->sKey.UTF8Decode(),
            pRuntime->NewString(pObjData->sData.UTF8Decode().AsStringView()));
        break;
      case JS_GlobalDataType::OBJECT: {
        v8::Local<v8::Object> pNewObj = pRuntime->NewFxDynamicObj(-1);
        if (!pNewObj.IsEmpty()) {
          PutObjectProperty(pNewObj, pObjData);
          pRuntime->PutObjectProperty(pObj, pObjData->sKey.UTF8Decode(),
                                      pNewObj);
        }
      } break;
      case JS_GlobalDataType::NULLOBJ:
        pRuntime->PutObjectProperty(pObj, pObjData->sKey.UTF8Decode(),
                                    pRuntime->NewNull());
        break;
    }
  }
}

void JSGlobalAlternate::DestroyGlobalPersisitentVariables() {
  m_MapGlobal.clear();
}

bool JSGlobalAlternate::SetGlobalVariables(const ByteString& propname,
                                           JS_GlobalDataType nType,
                                           double dData,
                                           bool bData,
                                           const ByteString& sData,
                                           v8::Local<v8::Object> pData,
                                           bool bDefaultPersistent) {
  if (propname.IsEmpty())
    return false;

  auto it = m_MapGlobal.find(propname);
  if (it != m_MapGlobal.end()) {
    JSGlobalData* pTemp = it->second.get();
    if (pTemp->bDeleted || pTemp->nType != nType) {
      pTemp->dData = 0;
      pTemp->bData = 0;
      pTemp->sData = "";
      pTemp->nType = nType;
    }
    pTemp->bDeleted = false;
    switch (nType) {
      case JS_GlobalDataType::NUMBER:
        pTemp->dData = dData;
        break;
      case JS_GlobalDataType::BOOLEAN:
        pTemp->bData = bData;
        break;
      case JS_GlobalDataType::STRING:
        pTemp->sData = sData;
        break;
      case JS_GlobalDataType::OBJECT:
        pTemp->pData.Reset(pData->GetIsolate(), pData);
        break;
      case JS_GlobalDataType::NULLOBJ:
        break;
      default:
        return false;
    }
    return true;
  }

  auto pNewData = pdfium::MakeUnique<JSGlobalData>();
  switch (nType) {
    case JS_GlobalDataType::NUMBER:
      pNewData->nType = JS_GlobalDataType::NUMBER;
      pNewData->dData = dData;
      pNewData->bPersistent = bDefaultPersistent;
      break;
    case JS_GlobalDataType::BOOLEAN:
      pNewData->nType = JS_GlobalDataType::BOOLEAN;
      pNewData->bData = bData;
      pNewData->bPersistent = bDefaultPersistent;
      break;
    case JS_GlobalDataType::STRING:
      pNewData->nType = JS_GlobalDataType::STRING;
      pNewData->sData = sData;
      pNewData->bPersistent = bDefaultPersistent;
      break;
    case JS_GlobalDataType::OBJECT:
      pNewData->nType = JS_GlobalDataType::OBJECT;
      pNewData->pData.Reset(pData->GetIsolate(), pData);
      pNewData->bPersistent = bDefaultPersistent;
      break;
    case JS_GlobalDataType::NULLOBJ:
      pNewData->nType = JS_GlobalDataType::NULLOBJ;
      pNewData->bPersistent = bDefaultPersistent;
      break;
    default:
      return false;
  }
  m_MapGlobal[propname] = std::move(pNewData);
  return true;
}
