// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "fxv8.h"
#include "value.h"
#include "class.h"
#include <math.h>
#include "util_inline.h"
FX_BOOL FXJSE_Value_IsUndefined(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue && lpValue->IsUndefined();
}
FX_BOOL FXJSE_Value_IsNull(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue && lpValue->IsNull();
}
FX_BOOL FXJSE_Value_IsBoolean(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue && lpValue->IsBoolean();
}
FX_BOOL FXJSE_Value_IsUTF8String(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue && lpValue->IsString();
}
FX_BOOL FXJSE_Value_IsNumber(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue && lpValue->IsNumber();
}
FX_BOOL FXJSE_Value_IsInteger(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue && lpValue->IsInteger();
}
FX_BOOL FXJSE_Value_IsObject(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue && lpValue->IsObject();
}
FX_BOOL FXJSE_Value_IsArray(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue && lpValue->IsArray();
}
FX_BOOL FXJSE_Value_IsFunction(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue && lpValue->IsFunction();
}
FX_BOOL FXJSE_Value_IsDate(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue && lpValue->IsDate();
}
FX_BOOL FXJSE_Value_ToBoolean(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->ToBoolean();
}
FX_FLOAT FXJSE_Value_ToFloat(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->ToFloat();
}
FXJSE_DOUBLE FXJSE_Value_ToDouble(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->ToDouble();
}
void FXJSE_Value_ToUTF8String(FXJSE_HVALUE hValue,
                              CFX_ByteString& szStrOutput) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->ToString(szStrOutput);
}
int32_t FXJSE_Value_ToInteger(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->ToInteger();
}
void* FXJSE_Value_ToObject(FXJSE_HVALUE hValue, FXJSE_HCLASS hClass) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  CFXJSE_Class* lpClass = reinterpret_cast<CFXJSE_Class*>(hClass);
  ASSERT(lpValue);
  return lpValue->ToObject(lpClass);
}
void FXJSE_Value_SetUndefined(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->SetUndefined();
}
void FXJSE_Value_SetNull(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->SetNull();
}
void FXJSE_Value_SetBoolean(FXJSE_HVALUE hValue, FX_BOOL bBoolean) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->SetBoolean(bBoolean);
}
void FXJSE_Value_SetUTF8String(FXJSE_HVALUE hValue,
                               const CFX_ByteStringC& szString) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->SetString(szString);
}
void FXJSE_Value_SetInteger(FXJSE_HVALUE hValue, int32_t nInteger) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->SetInteger(nInteger);
}
void FXJSE_Value_SetFloat(FXJSE_HVALUE hValue, FX_FLOAT fFloat) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->SetFloat(fFloat);
}
void FXJSE_Value_SetDouble(FXJSE_HVALUE hValue, FXJSE_DOUBLE dDouble) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->SetDouble(dDouble);
}
void FXJSE_Value_SetObject(FXJSE_HVALUE hValue,
                           void* lpObject,
                           FXJSE_HCLASS hClass) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  CFXJSE_Class* lpClass = reinterpret_cast<CFXJSE_Class*>(hClass);
  if (lpClass == NULL) {
    ASSERT(lpObject == NULL);
    lpValue->SetJSObject();
  } else if (lpClass != NULL) {
    lpValue->SetHostObject(lpObject, lpClass);
  }
}
void FXJSE_Value_SetArray(FXJSE_HVALUE hValue,
                          uint32_t uValueCount,
                          FXJSE_HVALUE* rgValues) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue->SetArray(uValueCount,
                           reinterpret_cast<CFXJSE_Value**>(rgValues));
}
void FXJSE_Value_SetDate(FXJSE_HVALUE hValue, FXJSE_DOUBLE dDouble) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  return lpValue->SetDate(dDouble);
}
void FXJSE_Value_Set(FXJSE_HVALUE hValue, FXJSE_HVALUE hOriginalValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  CFXJSE_Value* lpOriginalValue =
      reinterpret_cast<CFXJSE_Value*>(hOriginalValue);
  ASSERT(lpValue && lpOriginalValue);
  return lpValue->Assign(lpOriginalValue);
}
FX_BOOL FXJSE_Value_GetObjectProp(FXJSE_HVALUE hValue,
                                  const CFX_ByteStringC& szPropName,
                                  FXJSE_HVALUE hPropValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  CFXJSE_Value* lpPropValue = reinterpret_cast<CFXJSE_Value*>(hPropValue);
  ASSERT(lpValue && lpPropValue);
  return lpValue->GetObjectProperty(szPropName, lpPropValue);
}
FX_BOOL FXJSE_Value_SetObjectProp(FXJSE_HVALUE hValue,
                                  const CFX_ByteStringC& szPropName,
                                  FXJSE_HVALUE hPropValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  CFXJSE_Value* lpPropValue = reinterpret_cast<CFXJSE_Value*>(hPropValue);
  ASSERT(lpValue && lpPropValue);
  return lpValue->SetObjectProperty(szPropName, lpPropValue);
}
FX_BOOL FXJSE_Value_GetObjectPropByIdx(FXJSE_HVALUE hValue,
                                       uint32_t uPropIdx,
                                       FXJSE_HVALUE hPropValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  CFXJSE_Value* lpPropValue = reinterpret_cast<CFXJSE_Value*>(hPropValue);
  ASSERT(lpValue && lpPropValue);
  return lpValue->GetObjectProperty(uPropIdx, lpPropValue);
}
FX_BOOL FXJSE_Value_SetObjectPropByIdx(FXJSE_HVALUE hValue,
                                       uint32_t uPropIdx,
                                       FXJSE_HVALUE hPropValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  CFXJSE_Value* lpPropValue = reinterpret_cast<CFXJSE_Value*>(hPropValue);
  ASSERT(lpValue && lpPropValue);
  return lpValue->SetObjectProperty(uPropIdx, lpPropValue);
}
FX_BOOL FXJSE_Value_DeleteObjectProp(FXJSE_HVALUE hValue,
                                     const CFX_ByteStringC& szPropName) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->DeleteObjectProperty(szPropName);
}
FX_BOOL FXJSE_Value_ObjectHasOwnProp(FXJSE_HVALUE hValue,
                                     const CFX_ByteStringC& szPropName,
                                     FX_BOOL bUseTypeGetter) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return lpValue->HasObjectOwnProperty(szPropName, bUseTypeGetter);
}
FX_BOOL FXJSE_Value_SetObjectOwnProp(FXJSE_HVALUE hValue,
                                     const CFX_ByteStringC& szPropName,
                                     FXJSE_HVALUE hPropValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  CFXJSE_Value* lpPropValue = reinterpret_cast<CFXJSE_Value*>(hPropValue);
  ASSERT(lpValue && lpPropValue);
  return lpValue->SetObjectOwnProperty(szPropName, lpPropValue);
}
FX_BOOL FXJSE_Value_SetFunctionBind(FXJSE_HVALUE hValue,
                                    FXJSE_HVALUE hOldFunction,
                                    FXJSE_HVALUE hNewThis) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  CFXJSE_Value* lpOldFunction = reinterpret_cast<CFXJSE_Value*>(hOldFunction);
  CFXJSE_Value* lpNewThis = reinterpret_cast<CFXJSE_Value*>(hNewThis);
  ASSERT(lpValue && lpOldFunction && lpNewThis);
  return lpValue->SetFunctionBind(lpOldFunction, lpNewThis);
}
FX_BOOL FXJSE_Value_CallFunction(FXJSE_HVALUE hFunction,
                                 FXJSE_HVALUE hThis,
                                 FXJSE_HVALUE hRetValue,
                                 uint32_t nArgCount,
                                 FXJSE_HVALUE* lpArgs) {
  CFXJSE_Value* lpFunction = reinterpret_cast<CFXJSE_Value*>(hFunction);
  CFXJSE_Value* lpThis = reinterpret_cast<CFXJSE_Value*>(hThis);
  CFXJSE_Value* lpRetValue = reinterpret_cast<CFXJSE_Value*>(hRetValue);
  ASSERT(lpFunction);
  return lpFunction->Call(lpThis, lpRetValue, nArgCount, lpArgs);
}
FXJSE_HVALUE FXJSE_Value_Create(FXJSE_HRUNTIME hRuntime) {
  CFXJSE_Value* lpValue =
      CFXJSE_Value::Create(reinterpret_cast<v8::Isolate*>(hRuntime));
  ASSERT(lpValue);
  return reinterpret_cast<FXJSE_HVALUE>(lpValue);
}
void FXJSE_Value_Release(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  if (lpValue) {
    delete lpValue;
  }
}
FXJSE_HRUNTIME FXJSE_Value_GetRuntime(FXJSE_HVALUE hValue) {
  CFXJSE_Value* lpValue = reinterpret_cast<CFXJSE_Value*>(hValue);
  ASSERT(lpValue);
  return reinterpret_cast<FXJSE_HRUNTIME>(lpValue->GetIsolate());
}
void FXJSE_ThrowMessage(const CFX_ByteStringC& utf8Name,
                        const CFX_ByteStringC& utf8Message) {
  v8::Isolate* pIsolate = v8::Isolate::GetCurrent();
  ASSERT(pIsolate);
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(pIsolate);
  v8::Local<v8::String> hMessage = v8::String::NewFromUtf8(
      pIsolate, utf8Message.GetCStr(), v8::String::kNormalString,
      utf8Message.GetLength());
  v8::Local<v8::Value> hError;
  if (utf8Name == "RangeError") {
    hError = v8::Exception::RangeError(hMessage);
  } else if (utf8Name == "ReferenceError") {
    hError = v8::Exception::ReferenceError(hMessage);
  } else if (utf8Name == "SyntaxError") {
    hError = v8::Exception::SyntaxError(hMessage);
  } else if (utf8Name == "TypeError") {
    hError = v8::Exception::TypeError(hMessage);
  } else {
    hError = v8::Exception::Error(hMessage);
    if (utf8Name != "Error" && !utf8Name.IsEmpty()) {
      hError.As<v8::Object>()->Set(
          v8::String::NewFromUtf8(pIsolate, "name"),
          v8::String::NewFromUtf8(pIsolate, utf8Name.GetCStr(),
                                  v8::String::kNormalString,
                                  utf8Name.GetLength()));
    }
  }
  pIsolate->ThrowException(hError);
}
CFXJSE_Value* CFXJSE_Value::Create(v8::Isolate* pIsolate) {
  return new CFXJSE_Value(pIsolate);
}
void* CFXJSE_Value::ToObject(CFXJSE_Class* lpClass) const {
  ASSERT(!m_hValue.IsEmpty());
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> hValue = v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
  ASSERT(!hValue.IsEmpty());
  if (!hValue->IsObject()) {
    return NULL;
  }
  return FXJSE_RetrieveObjectBinding(hValue.As<v8::Object>(), lpClass);
}
V8_INLINE static double FXJSE_ftod(FX_FLOAT fNumber) {
  if (sizeof(FX_FLOAT) != 4) {
    ASSERT(FALSE);
    return fNumber;
  }
  uint32_t nFloatBits = (uint32_t&)fNumber;
  uint8_t nExponent = (uint8_t)(nFloatBits >> 16 >> 7);
  if (nExponent == 0 || nExponent == 255) {
    return fNumber;
  }
  int8_t nErrExp = nExponent - 127 - 23;
  if (nErrExp >= 0) {
    return fNumber;
  }
  double dwError = pow(2.0, nErrExp), dwErrorHalf = dwError / 2;
  double dNumber = fNumber, dNumberAbs = fabs(fNumber);
  double dNumberAbsMin = dNumberAbs - dwErrorHalf,
         dNumberAbsMax = dNumberAbs + dwErrorHalf;
  int32_t iErrPos = 0;
  if (floor(dNumberAbsMin) == floor(dNumberAbsMax)) {
    dNumberAbsMin = fmod(dNumberAbsMin, 1.0);
    dNumberAbsMax = fmod(dNumberAbsMax, 1.0);
    int32_t iErrPosMin = 1, iErrPosMax = 38;
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
void CFXJSE_Value::SetFloat(FX_FLOAT fFloat) {
  CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
  v8::Local<v8::Value> hValue = v8::Number::New(m_pIsolate, FXJSE_ftod(fFloat));
  m_hValue.Reset(m_pIsolate, hValue);
}
void CFXJSE_Value::SetHostObject(void* lpObject, CFXJSE_Class* lpClass) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  ASSERT(lpClass);
  v8::Local<v8::FunctionTemplate> hClass =
      v8::Local<v8::FunctionTemplate>::New(m_pIsolate, lpClass->m_hTemplate);
  v8::Local<v8::Object> hObject = hClass->InstanceTemplate()->NewInstance();
  FXJSE_UpdateObjectBinding(hObject, lpObject);
  m_hValue.Reset(m_pIsolate, hObject);
}
void CFXJSE_Value::SetArray(uint32_t uValueCount, CFXJSE_Value** rgValues) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Array> hArrayObject = v8::Array::New(m_pIsolate, uValueCount);
  if (rgValues) {
    for (uint32_t i = 0; i < uValueCount; i++) {
      if (rgValues[i]) {
        hArrayObject->Set(i, v8::Local<v8::Value>::New(
                                 m_pIsolate, rgValues[i]->DirectGetValue()));
      }
    }
  }
  m_hValue.Reset(m_pIsolate, hArrayObject);
}
void CFXJSE_Value::SetDate(FXJSE_DOUBLE dDouble) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> hDate = v8::Date::New(m_pIsolate, dDouble);
  m_hValue.Reset(m_pIsolate, hDate);
}
FX_BOOL CFXJSE_Value::SetObjectProperty(const CFX_ByteStringC& szPropName,
                                        CFXJSE_Value* lpPropValue) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> hObject =
      v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
  if (!hObject->IsObject()) {
    return FALSE;
  }
  v8::Local<v8::Value> hPropValue =
      v8::Local<v8::Value>::New(m_pIsolate, lpPropValue->DirectGetValue());
  return (FX_BOOL)hObject.As<v8::Object>()->Set(
      v8::String::NewFromUtf8(m_pIsolate, szPropName.GetCStr(),
                              v8::String::kNormalString,
                              szPropName.GetLength()),
      hPropValue);
}
FX_BOOL CFXJSE_Value::GetObjectProperty(const CFX_ByteStringC& szPropName,
                                        CFXJSE_Value* lpPropValue) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> hObject =
      v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
  if (!hObject->IsObject()) {
    return FALSE;
  }
  v8::Local<v8::Value> hPropValue =
      hObject.As<v8::Object>()->Get(v8::String::NewFromUtf8(
          m_pIsolate, szPropName.GetCStr(), v8::String::kNormalString,
          szPropName.GetLength()));
  lpPropValue->ForceSetValue(hPropValue);
  return TRUE;
}
FX_BOOL CFXJSE_Value::SetObjectProperty(uint32_t uPropIdx,
                                        CFXJSE_Value* lpPropValue) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> hObject =
      v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
  if (!hObject->IsObject()) {
    return FALSE;
  }
  v8::Local<v8::Value> hPropValue =
      v8::Local<v8::Value>::New(m_pIsolate, lpPropValue->DirectGetValue());
  return (FX_BOOL)hObject.As<v8::Object>()->Set(uPropIdx, hPropValue);
}
FX_BOOL CFXJSE_Value::GetObjectProperty(uint32_t uPropIdx,
                                        CFXJSE_Value* lpPropValue) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> hObject =
      v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
  if (!hObject->IsObject()) {
    return FALSE;
  }
  v8::Local<v8::Value> hPropValue = hObject.As<v8::Object>()->Get(uPropIdx);
  lpPropValue->ForceSetValue(hPropValue);
  return TRUE;
}
FX_BOOL CFXJSE_Value::DeleteObjectProperty(const CFX_ByteStringC& szPropName) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> hObject =
      v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
  if (!hObject->IsObject()) {
    return FALSE;
  }
  hObject.As<v8::Object>()->Delete(v8::String::NewFromUtf8(
      m_pIsolate, szPropName.GetCStr(), v8::String::kNormalString,
      szPropName.GetLength()));
  return TRUE;
}
FX_BOOL CFXJSE_Value::HasObjectOwnProperty(const CFX_ByteStringC& szPropName,
                                           FX_BOOL bUseTypeGetter) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> hObject =
      v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
  if (!hObject->IsObject()) {
    return FALSE;
  }
  v8::Local<v8::String> hKey = v8::String::NewFromUtf8(
      m_pIsolate, szPropName.GetCStr(), v8::String::kNormalString,
      szPropName.GetLength());
  return hObject.As<v8::Object>()->HasRealNamedProperty(hKey) ||
         (bUseTypeGetter && hObject.As<v8::Object>()->HasOwnProperty(hKey));
}
FX_BOOL CFXJSE_Value::SetObjectOwnProperty(const CFX_ByteStringC& szPropName,
                                           CFXJSE_Value* lpPropValue) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> hObject =
      v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
  if (!hObject->IsObject()) {
    return FALSE;
  }
  v8::Local<v8::Value> hValue =
      v8::Local<v8::Value>::New(m_pIsolate, lpPropValue->m_hValue);
  return hObject.As<v8::Object>()->ForceSet(
      v8::String::NewFromUtf8(m_pIsolate, szPropName.GetCStr(),
                              v8::String::kNormalString,
                              szPropName.GetLength()),
      hValue);
}
FX_BOOL CFXJSE_Value::SetFunctionBind(CFXJSE_Value* lpOldFunction,
                                      CFXJSE_Value* lpNewThis) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> rgArgs[2];
  v8::Local<v8::Value> hOldFunction =
      v8::Local<v8::Value>::New(m_pIsolate, lpOldFunction->DirectGetValue());
  if (hOldFunction.IsEmpty() || !hOldFunction->IsFunction()) {
    return FALSE;
  }
  rgArgs[0] = hOldFunction;
  v8::Local<v8::Value> hNewThis =
      v8::Local<v8::Value>::New(m_pIsolate, lpNewThis->DirectGetValue());
  if (hNewThis.IsEmpty()) {
    return FALSE;
  }
  rgArgs[1] = hNewThis;
  v8::Local<v8::String> hBinderFuncSource =
      v8::String::NewFromUtf8(m_pIsolate,
                              "(function (oldfunction, newthis) { return "
                              "oldfunction.bind(newthis); })");
  v8::Local<v8::Function> hBinderFunc =
      v8::Script::Compile(hBinderFuncSource)->Run().As<v8::Function>();
  v8::Local<v8::Value> hBoundFunction =
      hBinderFunc->Call(m_pIsolate->GetCurrentContext()->Global(), 2, rgArgs);
  if (hBoundFunction.IsEmpty() || !hBoundFunction->IsFunction()) {
    return FALSE;
  }
  m_hValue.Reset(m_pIsolate, hBoundFunction);
  return TRUE;
}
#define FXJSE_INVALID_PTR ((void*)(intptr_t)-1)
FX_BOOL CFXJSE_Value::Call(CFXJSE_Value* lpReceiver,
                           CFXJSE_Value* lpRetValue,
                           uint32_t nArgCount,
                           FXJSE_HVALUE* lpArgs) {
  CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
  v8::Local<v8::Value> hFunctionValue =
      v8::Local<v8::Value>::New(m_pIsolate, DirectGetValue());
  v8::Local<v8::Object> hFunctionObject =
      !hFunctionValue.IsEmpty() && hFunctionValue->IsObject()
          ? hFunctionValue.As<v8::Object>()
          : v8::Local<v8::Object>();
  v8::TryCatch trycatch;
  if (hFunctionObject.IsEmpty() || !hFunctionObject->IsCallable()) {
    if (lpRetValue) {
      lpRetValue->ForceSetValue(FXJSE_CreateReturnValue(m_pIsolate, trycatch));
    }
    return FALSE;
  }
  v8::Local<v8::Value> hReturnValue;
  v8::Local<v8::Value>* lpLocalArgs = NULL;
  if (nArgCount) {
    lpLocalArgs = FX_Alloc(v8::Local<v8::Value>, nArgCount);
    for (uint32_t i = 0; i < nArgCount; i++) {
      new (lpLocalArgs + i) v8::Local<v8::Value>;
      CFXJSE_Value* lpArg = (CFXJSE_Value*)lpArgs[i];
      if (lpArg) {
        lpLocalArgs[i] =
            v8::Local<v8::Value>::New(m_pIsolate, lpArg->DirectGetValue());
      }
      if (lpLocalArgs[i].IsEmpty()) {
        lpLocalArgs[i] = v8::Undefined(m_pIsolate);
      }
    }
  }
  FX_BOOL bRetValue = TRUE;
  if (lpReceiver == FXJSE_INVALID_PTR) {
    hReturnValue = hFunctionObject->CallAsConstructor(nArgCount, lpLocalArgs);
  } else {
    v8::Local<v8::Value> hReceiver;
    if (lpReceiver) {
      hReceiver =
          v8::Local<v8::Value>::New(m_pIsolate, lpReceiver->DirectGetValue());
    }
    if (hReceiver.IsEmpty() || !hReceiver->IsObject()) {
      hReceiver = v8::Object::New(m_pIsolate);
    }
    hReturnValue =
        hFunctionObject->CallAsFunction(hReceiver, nArgCount, lpLocalArgs);
  }
  if (trycatch.HasCaught()) {
    hReturnValue = FXJSE_CreateReturnValue(m_pIsolate, trycatch);
    bRetValue = FALSE;
  }
  if (lpRetValue) {
    lpRetValue->ForceSetValue(hReturnValue);
  }
  if (lpLocalArgs) {
    for (uint32_t i = 0; i < nArgCount; i++) {
      lpLocalArgs[i].~Local();
    }
    FX_Free(lpLocalArgs);
  }
  return bRetValue;
}
