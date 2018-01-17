// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_v8.h"

#ifdef PDF_ENABLE_XFA
#include "fxjs/cfxjse_context.h"
#include "xfa/fxfa/parser/cxfa_object.h"
#endif  // PDF_ENABLE_XFA

CJS_V8::CJS_V8(v8::Isolate* isolate) : m_isolate(isolate) {}

CJS_V8::~CJS_V8() {
  m_V8PersistentContext.Reset();
}

v8::Local<v8::Value> CJS_V8::GetObjectProperty(
    v8::Local<v8::Object> pObj,
    const WideString& wsPropertyName) {
  if (pObj.IsEmpty())
    return v8::Local<v8::Value>();
  v8::Local<v8::Value> val;
  if (!pObj->Get(m_isolate->GetCurrentContext(),
                 NewString(wsPropertyName.AsStringView()))
           .ToLocal(&val))
    return v8::Local<v8::Value>();
  return val;
}

std::vector<WideString> CJS_V8::GetObjectPropertyNames(
    v8::Local<v8::Object> pObj) {
  if (pObj.IsEmpty())
    return std::vector<WideString>();

  v8::Local<v8::Array> val;
  v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
  if (!pObj->GetPropertyNames(context).ToLocal(&val))
    return std::vector<WideString>();

  std::vector<WideString> result;
  for (uint32_t i = 0; i < val->Length(); ++i) {
    result.push_back(ToWideString(val->Get(context, i).ToLocalChecked()));
  }

  return result;
}

void CJS_V8::PutObjectProperty(v8::Local<v8::Object> pObj,
                               const WideString& wsPropertyName,
                               v8::Local<v8::Value> pPut) {
  if (pObj.IsEmpty())
    return;
  pObj->Set(m_isolate->GetCurrentContext(),
            NewString(wsPropertyName.AsStringView()), pPut)
      .FromJust();
}

v8::Local<v8::Array> CJS_V8::NewArray() {
  return v8::Array::New(m_isolate);
}

unsigned CJS_V8::PutArrayElement(v8::Local<v8::Array> pArray,
                                 unsigned index,
                                 v8::Local<v8::Value> pValue) {
  if (pArray.IsEmpty())
    return 0;
  if (pArray->Set(m_isolate->GetCurrentContext(), index, pValue).IsNothing())
    return 0;
  return 1;
}

v8::Local<v8::Value> CJS_V8::GetArrayElement(v8::Local<v8::Array> pArray,
                                             unsigned index) {
  if (pArray.IsEmpty())
    return v8::Local<v8::Value>();
  v8::Local<v8::Value> val;
  if (!pArray->Get(m_isolate->GetCurrentContext(), index).ToLocal(&val))
    return v8::Local<v8::Value>();
  return val;
}

unsigned CJS_V8::GetArrayLength(v8::Local<v8::Array> pArray) {
  if (pArray.IsEmpty())
    return 0;
  return pArray->Length();
}

v8::Local<v8::Context> CJS_V8::NewLocalContext() {
  return v8::Local<v8::Context>::New(m_isolate, m_V8PersistentContext);
}

v8::Local<v8::Context> CJS_V8::GetPersistentContext() {
  return m_V8PersistentContext.Get(m_isolate);
}

v8::Local<v8::Number> CJS_V8::NewNumber(int number) {
  return v8::Int32::New(m_isolate, number);
}

v8::Local<v8::Number> CJS_V8::NewNumber(double number) {
  return v8::Number::New(m_isolate, number);
}

v8::Local<v8::Number> CJS_V8::NewNumber(float number) {
  return v8::Number::New(m_isolate, (float)number);
}

v8::Local<v8::Boolean> CJS_V8::NewBoolean(bool b) {
  return v8::Boolean::New(m_isolate, b);
}

v8::Local<v8::String> CJS_V8::NewString(const ByteStringView& str) {
  v8::Isolate* pIsolate = m_isolate ? m_isolate : v8::Isolate::GetCurrent();
  return v8::String::NewFromUtf8(pIsolate, str.unterminated_c_str(),
                                 v8::NewStringType::kNormal, str.GetLength())
      .ToLocalChecked();
}

v8::Local<v8::String> CJS_V8::NewString(const WideStringView& str) {
  // Conversion from pdfium's wchar_t wide-strings to v8's uint16_t
  // wide-strings isn't handled by v8, so use UTF8 as a common
  // intermediate format.
  return NewString(FX_UTF8Encode(str).AsStringView());
}

v8::Local<v8::Value> CJS_V8::NewNull() {
  return v8::Null(m_isolate);
}

v8::Local<v8::Value> CJS_V8::NewUndefined() {
  return v8::Undefined(m_isolate);
}

v8::Local<v8::Date> CJS_V8::NewDate(double d) {
  return v8::Date::New(m_isolate->GetCurrentContext(), d)
      .ToLocalChecked()
      .As<v8::Date>();
}

int CJS_V8::ToInt32(v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return 0;
  v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
  v8::MaybeLocal<v8::Int32> maybe_int32 = pValue->ToInt32(context);
  if (maybe_int32.IsEmpty())
    return 0;
  return maybe_int32.ToLocalChecked()->Value();
}

bool CJS_V8::ToBoolean(v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return false;
  v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
  v8::MaybeLocal<v8::Boolean> maybe_boolean = pValue->ToBoolean(context);
  if (maybe_boolean.IsEmpty())
    return false;
  return maybe_boolean.ToLocalChecked()->Value();
}

double CJS_V8::ToDouble(v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return 0.0;
  v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
  v8::MaybeLocal<v8::Number> maybe_number = pValue->ToNumber(context);
  if (maybe_number.IsEmpty())
    return 0.0;
  return maybe_number.ToLocalChecked()->Value();
}

WideString CJS_V8::ToWideString(v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return WideString();
  v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
  v8::MaybeLocal<v8::String> maybe_string = pValue->ToString(context);
  if (maybe_string.IsEmpty())
    return WideString();
  v8::String::Utf8Value s(m_isolate, maybe_string.ToLocalChecked());
  return WideString::FromUTF8(ByteStringView(*s, s.length()));
}

ByteString CJS_V8::ToByteString(v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty())
    return ByteString();
  v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
  v8::MaybeLocal<v8::String> maybe_string = pValue->ToString(context);
  if (maybe_string.IsEmpty())
    return ByteString();
  v8::String::Utf8Value s(m_isolate, maybe_string.ToLocalChecked());
  return ByteString(*s);
}

v8::Local<v8::Object> CJS_V8::ToObject(v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty() || !pValue->IsObject())
    return v8::Local<v8::Object>();
  v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
  return pValue->ToObject(context).ToLocalChecked();
}

v8::Local<v8::Array> CJS_V8::ToArray(v8::Local<v8::Value> pValue) {
  if (pValue.IsEmpty() || !pValue->IsArray())
    return v8::Local<v8::Array>();
  v8::Local<v8::Context> context = m_isolate->GetCurrentContext();
  return v8::Local<v8::Array>::Cast(pValue->ToObject(context).ToLocalChecked());
}

void CJS_V8::SetConstArray(const WideString& name, v8::Local<v8::Array> array) {
  m_ConstArrays[name] = v8::Global<v8::Array>(GetIsolate(), array);
}

v8::Local<v8::Array> CJS_V8::GetConstArray(const WideString& name) {
  return v8::Local<v8::Array>::New(GetIsolate(), m_ConstArrays[name]);
}

#ifdef PDF_ENABLE_XFA
CXFA_Object* CJS_V8::ToXFAObject(v8::Local<v8::Value> obj) {
  ASSERT(!obj.IsEmpty());

  if (!obj->IsObject())
    return nullptr;

  CFXJSE_HostObject* hostObj =
      FXJSE_RetrieveObjectBinding(obj.As<v8::Object>(), nullptr);
  if (!hostObj || hostObj->type() != CFXJSE_HostObject::kXFA)
    return nullptr;
  return static_cast<CXFA_Object*>(hostObj);
}

v8::Local<v8::Value> CJS_V8::NewXFAObject(
    CXFA_Object* obj,
    v8::Global<v8::FunctionTemplate>& tmpl) {
  v8::EscapableHandleScope scope(m_isolate);
  v8::Local<v8::FunctionTemplate> klass =
      v8::Local<v8::FunctionTemplate>::New(m_isolate, tmpl);
  v8::Local<v8::Object> object = klass->InstanceTemplate()->NewInstance();
  FXJSE_UpdateObjectBinding(object, obj);
  return scope.Escape(object);
}
#endif  // PDF_ENABLE_XFA
