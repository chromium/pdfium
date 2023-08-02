// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_context.h"

#include <utility>

#include "fxjs/cfxjs_engine.h"
#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_class.h"
#include "fxjs/xfa/cfxjse_isolatetracker.h"
#include "fxjs/xfa/cfxjse_runtimedata.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/memory/ptr_util.h"
#include "v8/include/v8-exception.h"
#include "v8/include/v8-function.h"
#include "v8/include/v8-message.h"
#include "v8/include/v8-script.h"
#include "xfa/fxfa/parser/cxfa_thisproxy.h"

namespace {

const char szCompatibleModeScript[] =
    "(function(global, list) {\n"
    "  'use strict';\n"
    "  var objname;\n"
    "  for (objname in list) {\n"
    "    var globalobj = global[objname];\n"
    "    if (globalobj) {\n"
    "      list[objname].forEach(function(name) {\n"
    "        if (!globalobj[name]) {\n"
    "          Object.defineProperty(globalobj, name, {\n"
    "            writable: true,\n"
    "            enumerable: false,\n"
    "            value: (function(obj) {\n"
    "              if (arguments.length === 0) {\n"
    "                throw new TypeError('missing argument 0 when calling "
    "                    function ' + objname + '.' + name);\n"
    "              }\n"
    "              return globalobj.prototype[name].apply(obj, "
    "                  Array.prototype.slice.call(arguments, 1));\n"
    "            })\n"
    "          });\n"
    "        }\n"
    "      });\n"
    "    }\n"
    "  }\n"
    "}(this, {String: ['substr', 'toUpperCase']}));";

const char szConsoleScript[] =
    "console.show = function() {};\n"
    "\n"
    "console.println = function(...args) {\n"
    "  this.log(...args);\n"
    "};";

// Only address matters, values are for humans debuging here.  Keep these
// wchar_t to prevent the compiler from doing something clever, like
// aligning them on a byte boundary to save space, which would make them
// incompatible for use as V8 aligned pointers.
const wchar_t kFXJSEHostObjectTag[] = L"FXJSE Host Object";
const wchar_t kFXJSEProxyObjectTag[] = L"FXJSE Proxy Object";

v8::Local<v8::Object> CreateReturnValue(v8::Isolate* pIsolate,
                                        v8::TryCatch* trycatch) {
  v8::Local<v8::Object> hReturnValue = v8::Object::New(pIsolate);
  if (!trycatch->HasCaught())
    return hReturnValue;

  v8::Local<v8::Message> hMessage = trycatch->Message();
  if (hMessage.IsEmpty())
    return hReturnValue;

  v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
  v8::Local<v8::Value> hException = trycatch->Exception();
  if (hException->IsObject()) {
    v8::Local<v8::String> hNameStr = fxv8::NewStringHelper(pIsolate, "name");
    v8::Local<v8::Value> hValue =
        hException.As<v8::Object>()->Get(context, hNameStr).ToLocalChecked();
    if (hValue->IsString() || hValue->IsStringObject()) {
      hReturnValue->Set(context, 0, hValue).FromJust();
    } else {
      v8::Local<v8::String> hErrorStr =
          fxv8::NewStringHelper(pIsolate, "Error");
      hReturnValue->Set(context, 0, hErrorStr).FromJust();
    }
    v8::Local<v8::String> hMessageStr =
        fxv8::NewStringHelper(pIsolate, "message");
    hValue =
        hException.As<v8::Object>()->Get(context, hMessageStr).ToLocalChecked();
    if (hValue->IsString() || hValue->IsStringObject())
      hReturnValue->Set(context, 1, hValue).FromJust();
    else
      hReturnValue->Set(context, 1, hMessage->Get()).FromJust();
  } else {
    v8::Local<v8::String> hErrorStr = fxv8::NewStringHelper(pIsolate, "Error");
    hReturnValue->Set(context, 0, hErrorStr).FromJust();
    hReturnValue->Set(context, 1, hMessage->Get()).FromJust();
  }
  hReturnValue->Set(context, 2, hException).FromJust();
  int line = hMessage->GetLineNumber(context).FromMaybe(0);
  hReturnValue->Set(context, 3, v8::Integer::New(pIsolate, line)).FromJust();
  v8::Local<v8::String> source =
      hMessage->GetSourceLine(context).FromMaybe(v8::Local<v8::String>());
  hReturnValue->Set(context, 4, source).FromJust();
  int column = hMessage->GetStartColumn(context).FromMaybe(0);
  hReturnValue->Set(context, 5, v8::Integer::New(pIsolate, column)).FromJust();
  column = hMessage->GetEndColumn(context).FromMaybe(0);
  hReturnValue->Set(context, 6, v8::Integer::New(pIsolate, column)).FromJust();
  return hReturnValue;
}

void FXJSE_UpdateProxyBinding(v8::Local<v8::Object> hObject) {
  DCHECK(!hObject.IsEmpty());
  DCHECK_EQ(hObject->InternalFieldCount(), 2);
  hObject->SetAlignedPointerInInternalField(
      0, const_cast<wchar_t*>(kFXJSEProxyObjectTag));
  hObject->SetAlignedPointerInInternalField(1, nullptr);
}

}  // namespace

void FXJSE_UpdateObjectBinding(v8::Local<v8::Object> hObject,
                               CFXJSE_HostObject* pNewBinding) {
  DCHECK(!hObject.IsEmpty());
  DCHECK_EQ(hObject->InternalFieldCount(), 2);
  hObject->SetAlignedPointerInInternalField(
      0, const_cast<wchar_t*>(kFXJSEHostObjectTag));
  hObject->SetAlignedPointerInInternalField(1, pNewBinding);
}

void FXJSE_ClearObjectBinding(v8::Local<v8::Object> hObject) {
  DCHECK(!hObject.IsEmpty());
  DCHECK_EQ(hObject->InternalFieldCount(), 2);
  hObject->SetAlignedPointerInInternalField(0, nullptr);
  hObject->SetAlignedPointerInInternalField(1, nullptr);
}

CFXJSE_HostObject* FXJSE_RetrieveObjectBinding(v8::Local<v8::Value> hValue) {
  if (!fxv8::IsObject(hValue))
    return nullptr;

  v8::Local<v8::Object> hObject = hValue.As<v8::Object>();
  if (hObject->InternalFieldCount() != 2 ||
      hObject->GetAlignedPointerFromInternalField(0) == kFXJSEProxyObjectTag) {
    v8::Local<v8::Value> hProtoObject = hObject->GetPrototype();
    if (!fxv8::IsObject(hProtoObject))
      return nullptr;

    hObject = hProtoObject.As<v8::Object>();
    if (hObject->InternalFieldCount() != 2)
      return nullptr;
  }
  if (hObject->GetAlignedPointerFromInternalField(0) != kFXJSEHostObjectTag)
    return nullptr;

  return static_cast<CFXJSE_HostObject*>(
      hObject->GetAlignedPointerFromInternalField(1));
}

// static
std::unique_ptr<CFXJSE_Context> CFXJSE_Context::Create(
    v8::Isolate* pIsolate,
    const FXJSE_CLASS_DESCRIPTOR* pGlobalClass,
    CFXJSE_HostObject* pGlobalObject,
    CXFA_ThisProxy* pProxy) {
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);

  // Private constructor.
  auto pContext = pdfium::WrapUnique(new CFXJSE_Context(pIsolate, pProxy));
  v8::Local<v8::ObjectTemplate> hObjectTemplate;
  if (pGlobalClass) {
    CFXJSE_Class* pGlobalClassObj =
        CFXJSE_Class::Create(pContext.get(), pGlobalClass, true);
    hObjectTemplate =
        pGlobalClassObj->GetTemplate(pIsolate)->InstanceTemplate();
  } else {
    hObjectTemplate = v8::ObjectTemplate::New(pIsolate);
    hObjectTemplate->SetInternalFieldCount(2);
  }
  hObjectTemplate->Set(v8::Symbol::GetToStringTag(pIsolate),
                       fxv8::NewStringHelper(pIsolate, "global"));

  v8::Local<v8::Context> hNewContext =
      v8::Context::New(pIsolate, nullptr, hObjectTemplate);
  v8::Local<v8::Object> pThisProxy = hNewContext->Global();
  FXJSE_UpdateProxyBinding(pThisProxy);

  v8::Local<v8::Object> pThis = pThisProxy->GetPrototype().As<v8::Object>();
  FXJSE_UpdateObjectBinding(pThis, pGlobalObject);

  v8::Local<v8::Context> hRootContext = v8::Local<v8::Context>::New(
      pIsolate, CFXJSE_RuntimeData::Get(pIsolate)->GetRootContext());
  hNewContext->SetSecurityToken(hRootContext->GetSecurityToken());
  pContext->m_hContext.Reset(pIsolate, hNewContext);
  return pContext;
}

CFXJSE_Context::CFXJSE_Context(v8::Isolate* pIsolate, CXFA_ThisProxy* pProxy)
    : m_pIsolate(pIsolate), m_pProxy(pProxy) {}

CFXJSE_Context::~CFXJSE_Context() = default;

v8::Local<v8::Object> CFXJSE_Context::GetGlobalObject() {
  v8::Isolate::Scope isolate_scope(GetIsolate());
  v8::EscapableHandleScope handle_scope(GetIsolate());
  v8::Local<v8::Context> hContext =
      v8::Local<v8::Context>::New(GetIsolate(), m_hContext);
  v8::Local<v8::Object> result =
      hContext->Global()->GetPrototype().As<v8::Object>();
  return handle_scope.Escape(result);
}

v8::Local<v8::Context> CFXJSE_Context::GetContext() {
  return v8::Local<v8::Context>::New(GetIsolate(), m_hContext);
}

void CFXJSE_Context::AddClass(std::unique_ptr<CFXJSE_Class> pClass) {
  m_rgClasses.push_back(std::move(pClass));
}

CFXJSE_Class* CFXJSE_Context::GetClassByName(ByteStringView szName) const {
  auto pClass =
      std::find_if(m_rgClasses.begin(), m_rgClasses.end(),
                   [szName](const std::unique_ptr<CFXJSE_Class>& item) {
                     return item->IsName(szName);
                   });
  return pClass != m_rgClasses.end() ? pClass->get() : nullptr;
}

void CFXJSE_Context::EnableCompatibleMode() {
  ExecuteScript(szCompatibleModeScript, nullptr, v8::Local<v8::Object>());
  ExecuteScript(szConsoleScript, nullptr, v8::Local<v8::Object>());
}

bool CFXJSE_Context::ExecuteScript(ByteStringView bsScript,
                                   CFXJSE_Value* pRetValue,
                                   v8::Local<v8::Object> hNewThis) {
  CFXJSE_ScopeUtil_IsolateHandleContext scope(this);
  v8::Local<v8::Context> hContext = GetIsolate()->GetCurrentContext();
  v8::TryCatch trycatch(GetIsolate());
  v8::Local<v8::String> hScriptString =
      fxv8::NewStringHelper(GetIsolate(), bsScript);
  if (hNewThis.IsEmpty()) {
    v8::Local<v8::Script> hScript;
    if (v8::Script::Compile(hContext, hScriptString).ToLocal(&hScript)) {
      CHECK(!trycatch.HasCaught());
      v8::Local<v8::Value> hValue;
      if (hScript->Run(hContext).ToLocal(&hValue)) {
        CHECK(!trycatch.HasCaught());
        if (pRetValue)
          pRetValue->ForceSetValue(GetIsolate(), hValue);
        return true;
      }
    }
    if (pRetValue) {
      pRetValue->ForceSetValue(GetIsolate(),
                               CreateReturnValue(GetIsolate(), &trycatch));
    }
    return false;
  }

  v8::Local<v8::String> hEval = fxv8::NewStringHelper(
      GetIsolate(), "(function () { return eval(arguments[0]); })");
  v8::Local<v8::Script> hWrapper =
      v8::Script::Compile(hContext, hEval).ToLocalChecked();
  v8::Local<v8::Value> hWrapperValue;
  if (hWrapper->Run(hContext).ToLocal(&hWrapperValue)) {
    CHECK(!trycatch.HasCaught());
    CHECK(hWrapperValue->IsFunction());
    v8::Local<v8::Function> hWrapperFn = hWrapperValue.As<v8::Function>();
    v8::Local<v8::Value> rgArgs[] = {hScriptString};
    v8::Local<v8::Value> hValue;
    if (hWrapperFn->Call(hContext, hNewThis, 1, rgArgs).ToLocal(&hValue)) {
      DCHECK(!trycatch.HasCaught());
      if (pRetValue)
        pRetValue->ForceSetValue(GetIsolate(), hValue);
      return true;
    }
  }

#ifndef NDEBUG
  v8::String::Utf8Value error(GetIsolate(), trycatch.Exception());
  fprintf(stderr, "JS Error: %s\n", *error);

  v8::Local<v8::Message> message = trycatch.Message();
  if (!message.IsEmpty()) {
    v8::Local<v8::Context> context(GetIsolate()->GetCurrentContext());
    int linenum = message->GetLineNumber(context).FromJust();
    v8::String::Utf8Value sourceline(
        GetIsolate(), message->GetSourceLine(context).ToLocalChecked());
    fprintf(stderr, "Line %d: %s\n", linenum, *sourceline);
  }
#endif  // NDEBUG

  if (pRetValue) {
    pRetValue->ForceSetValue(GetIsolate(),
                             CreateReturnValue(GetIsolate(), &trycatch));
  }
  return false;
}
