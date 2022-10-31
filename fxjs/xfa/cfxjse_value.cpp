// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_value.h"

#include <math.h>

#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_class.h"
#include "fxjs/xfa/cfxjse_context.h"
#include "fxjs/xfa/cfxjse_isolatetracker.h"
#include "third_party/base/check.h"
#include "v8/include/v8-container.h"
#include "v8/include/v8-exception.h"
#include "v8/include/v8-function.h"
#include "v8/include/v8-primitive.h"
#include "v8/include/v8-script.h"

namespace {

double ftod(float fNumber) {
  static_assert(sizeof(float) == 4, "float of incorrect size");

  uint32_t nFloatBits = (uint32_t&)fNumber;
  uint8_t nExponent = (uint8_t)(nFloatBits >> 23);
  if (nExponent == 0 || nExponent == 255)
    return fNumber;

  int8_t nErrExp = nExponent - 150;
  if (nErrExp >= 0)
    return fNumber;

  double dwError = pow(2.0, nErrExp);
  double dwErrorHalf = dwError / 2;
  double dNumber = fNumber;
  double dNumberAbs = fabs(fNumber);
  double dNumberAbsMin = dNumberAbs - dwErrorHalf;
  double dNumberAbsMax = dNumberAbs + dwErrorHalf;
  int32_t iErrPos = 0;
  if (floor(dNumberAbsMin) == floor(dNumberAbsMax)) {
    dNumberAbsMin = fmod(dNumberAbsMin, 1.0);
    dNumberAbsMax = fmod(dNumberAbsMax, 1.0);
    int32_t iErrPosMin = 1;
    int32_t iErrPosMax = 38;
    do {
      int32_t iMid = (iErrPosMin + iErrPosMax) / 2;
      double dPow = pow(10.0, iMid);
      if (floor(dNumberAbsMin * dPow) == floor(dNumberAbsMax * dPow)) {
        iErrPosMin = iMid + 1;
      } else {
        iErrPosMax = iMid;
      }
    } while (iErrPosMin < iErrPosMax);
    iErrPos = iErrPosMax;
  }
  double dPow = pow(10.0, iErrPos);
  return fNumber < 0 ? ceil(dNumber * dPow - 0.5) / dPow
                     : floor(dNumber * dPow + 0.5) / dPow;
}

}  // namespace

void FXJSE_ThrowMessage(v8::Isolate* pIsolate, ByteStringView utf8Message) {
  DCHECK(pIsolate);
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::String> hMessage = fxv8::NewStringHelper(pIsolate, utf8Message);
  v8::Local<v8::Value> hError = v8::Exception::Error(hMessage);
  pIsolate->ThrowException(hError);
}

CFXJSE_Value::CFXJSE_Value() = default;

CFXJSE_Value::CFXJSE_Value(v8::Isolate* pIsolate, v8::Local<v8::Value> value) {
  ForceSetValue(pIsolate, value);
}

CFXJSE_Value::~CFXJSE_Value() = default;

CFXJSE_HostObject* CFXJSE_Value::ToHostObject(v8::Isolate* pIsolate) const {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  return CFXJSE_HostObject::FromV8(
      v8::Local<v8::Value>::New(pIsolate, m_hValue));
}

void CFXJSE_Value::SetHostObject(v8::Isolate* pIsolate,
                                 CFXJSE_HostObject* pObject,
                                 CFXJSE_Class* pClass) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  m_hValue.Reset(pIsolate, pObject->NewBoundV8Object(
                               pIsolate, pClass->GetTemplate(pIsolate)));
}

void CFXJSE_Value::SetArray(
    v8::Isolate* pIsolate,
    const std::vector<std::unique_ptr<CFXJSE_Value>>& values) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  std::vector<v8::Local<v8::Value>> local_values;
  local_values.reserve(values.size());
  for (auto& v : values) {
    if (v->IsEmpty())
      local_values.push_back(fxv8::NewUndefinedHelper(pIsolate));
    else
      local_values.push_back(v->GetValue(pIsolate));
  }
  v8::Local<v8::Array> hArrayObject =
      v8::Array::New(pIsolate, local_values.data(), local_values.size());
  m_hValue.Reset(pIsolate, hArrayObject);
}

void CFXJSE_Value::SetFloat(v8::Isolate* pIsolate, float fFloat) {
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  m_hValue.Reset(pIsolate, fxv8::NewNumberHelper(pIsolate, ftod(fFloat)));
}

bool CFXJSE_Value::SetObjectProperty(v8::Isolate* pIsolate,
                                     ByteStringView szPropName,
                                     CFXJSE_Value* pPropValue) {
  if (pPropValue->IsEmpty())
    return false;

  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::Value> hObject = GetValue(pIsolate);
  if (!hObject->IsObject())
    return false;

  return fxv8::ReentrantPutObjectPropertyHelper(
      pIsolate, hObject.As<v8::Object>(), szPropName,
      pPropValue->GetValue(pIsolate));
}

bool CFXJSE_Value::GetObjectProperty(v8::Isolate* pIsolate,
                                     ByteStringView szPropName,
                                     CFXJSE_Value* pPropValue) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::Value> hObject = GetValue(pIsolate);
  if (!hObject->IsObject())
    return false;

  pPropValue->ForceSetValue(
      pIsolate, fxv8::ReentrantGetObjectPropertyHelper(
                    pIsolate, hObject.As<v8::Object>(), szPropName));
  return true;
}

bool CFXJSE_Value::GetObjectPropertyByIdx(v8::Isolate* pIsolate,
                                          uint32_t uPropIdx,
                                          CFXJSE_Value* pPropValue) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::Value> hObject = GetValue(pIsolate);
  if (!hObject->IsArray())
    return false;

  pPropValue->ForceSetValue(pIsolate,
                            fxv8::ReentrantGetArrayElementHelper(
                                pIsolate, hObject.As<v8::Array>(), uPropIdx));
  return true;
}

void CFXJSE_Value::DeleteObjectProperty(v8::Isolate* pIsolate,
                                        ByteStringView szPropName) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::Value> hObject = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  if (hObject->IsObject()) {
    fxv8::ReentrantDeleteObjectPropertyHelper(
        pIsolate, hObject.As<v8::Object>(), szPropName);
  }
}

bool CFXJSE_Value::SetObjectOwnProperty(v8::Isolate* pIsolate,
                                        ByteStringView szPropName,
                                        CFXJSE_Value* pPropValue) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::Value> hObject = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  if (!hObject->IsObject())
    return false;

  v8::Local<v8::Value> pValue =
      v8::Local<v8::Value>::New(pIsolate, pPropValue->m_hValue);
  return fxv8::ReentrantSetObjectOwnPropertyHelper(
      pIsolate, hObject.As<v8::Object>(), szPropName, pValue);
}

v8::Local<v8::Function> CFXJSE_Value::NewBoundFunction(
    v8::Isolate* pIsolate,
    v8::Local<v8::Function> hOldFunction,
    v8::Local<v8::Object> hNewThis) {
  DCHECK(!hOldFunction.IsEmpty());
  DCHECK(!hNewThis.IsEmpty());

  CFXJSE_ScopeUtil_RootContext scope(pIsolate);
  v8::Local<v8::Value> rgArgs[2];
  rgArgs[0] = hOldFunction;
  rgArgs[1] = hNewThis;
  v8::Local<v8::String> hBinderFuncSource = fxv8::NewStringHelper(
      pIsolate, "(function (fn, obj) { return fn.bind(obj); })");
  v8::Local<v8::Context> hContext = pIsolate->GetCurrentContext();
  v8::Local<v8::Function> hBinderFunc =
      v8::Script::Compile(hContext, hBinderFuncSource)
          .ToLocalChecked()
          ->Run(hContext)
          .ToLocalChecked()
          .As<v8::Function>();
  v8::Local<v8::Value> hBoundFunction =
      hBinderFunc->Call(hContext, hContext->Global(), 2, rgArgs)
          .ToLocalChecked();
  if (!fxv8::IsFunction(hBoundFunction))
    return v8::Local<v8::Function>();

  return hBoundFunction.As<v8::Function>();
}

v8::Local<v8::Value> CFXJSE_Value::GetValue(v8::Isolate* pIsolate) const {
  return v8::Local<v8::Value>::New(pIsolate, m_hValue);
}

bool CFXJSE_Value::IsEmpty() const {
  return m_hValue.IsEmpty();
}

bool CFXJSE_Value::IsUndefined(v8::Isolate* pIsolate) const {
  if (IsEmpty())
    return false;

  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::Value> hValue = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  return hValue->IsUndefined();
}

bool CFXJSE_Value::IsNull(v8::Isolate* pIsolate) const {
  if (IsEmpty())
    return false;

  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::Value> hValue = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  return hValue->IsNull();
}

bool CFXJSE_Value::IsBoolean(v8::Isolate* pIsolate) const {
  if (IsEmpty())
    return false;

  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::Value> hValue = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  return hValue->IsBoolean();
}

bool CFXJSE_Value::IsString(v8::Isolate* pIsolate) const {
  if (IsEmpty())
    return false;

  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::Value> hValue = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  return hValue->IsString();
}

bool CFXJSE_Value::IsNumber(v8::Isolate* pIsolate) const {
  if (IsEmpty())
    return false;

  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::Value> hValue = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  return hValue->IsNumber();
}

bool CFXJSE_Value::IsInteger(v8::Isolate* pIsolate) const {
  if (IsEmpty())
    return false;

  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::Value> hValue = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  return hValue->IsInt32();
}

bool CFXJSE_Value::IsObject(v8::Isolate* pIsolate) const {
  if (IsEmpty())
    return false;

  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::Value> hValue = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  return hValue->IsObject();
}

bool CFXJSE_Value::IsArray(v8::Isolate* pIsolate) const {
  if (IsEmpty())
    return false;

  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::Value> hValue = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  return hValue->IsArray();
}

bool CFXJSE_Value::IsFunction(v8::Isolate* pIsolate) const {
  if (IsEmpty())
    return false;

  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  v8::Local<v8::Value> hValue = v8::Local<v8::Value>::New(pIsolate, m_hValue);
  return hValue->IsFunction();
}

bool CFXJSE_Value::ToBoolean(v8::Isolate* pIsolate) const {
  DCHECK(!IsEmpty());
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  return fxv8::ReentrantToBooleanHelper(
      pIsolate, v8::Local<v8::Value>::New(pIsolate, m_hValue));
}

float CFXJSE_Value::ToFloat(v8::Isolate* pIsolate) const {
  return static_cast<float>(ToDouble(pIsolate));
}

double CFXJSE_Value::ToDouble(v8::Isolate* pIsolate) const {
  DCHECK(!IsEmpty());
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  return fxv8::ReentrantToDoubleHelper(
      pIsolate, v8::Local<v8::Value>::New(pIsolate, m_hValue));
}

int32_t CFXJSE_Value::ToInteger(v8::Isolate* pIsolate) const {
  DCHECK(!IsEmpty());
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  return fxv8::ReentrantToInt32Helper(
      pIsolate, v8::Local<v8::Value>::New(pIsolate, m_hValue));
}

ByteString CFXJSE_Value::ToString(v8::Isolate* pIsolate) const {
  DCHECK(!IsEmpty());
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  return fxv8::ReentrantToByteStringHelper(
      pIsolate, v8::Local<v8::Value>::New(pIsolate, m_hValue));
}

void CFXJSE_Value::SetUndefined(v8::Isolate* pIsolate) {
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  m_hValue.Reset(pIsolate, fxv8::NewUndefinedHelper(pIsolate));
}

void CFXJSE_Value::SetNull(v8::Isolate* pIsolate) {
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  m_hValue.Reset(pIsolate, fxv8::NewNullHelper(pIsolate));
}

void CFXJSE_Value::SetBoolean(v8::Isolate* pIsolate, bool bBoolean) {
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  m_hValue.Reset(pIsolate, fxv8::NewBooleanHelper(pIsolate, bBoolean));
}

void CFXJSE_Value::SetInteger(v8::Isolate* pIsolate, int32_t nInteger) {
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  m_hValue.Reset(pIsolate, fxv8::NewNumberHelper(pIsolate, nInteger));
}

void CFXJSE_Value::SetDouble(v8::Isolate* pIsolate, double dDouble) {
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  m_hValue.Reset(pIsolate, fxv8::NewNumberHelper(pIsolate, dDouble));
}

void CFXJSE_Value::SetString(v8::Isolate* pIsolate, ByteStringView szString) {
  CFXJSE_ScopeUtil_IsolateHandle scope(pIsolate);
  m_hValue.Reset(pIsolate, fxv8::NewStringHelper(pIsolate, szString));
}
