// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXJSE_VALUE_H_
#define XFA_FXJSE_VALUE_H_

#include "xfa/fxjse/scope_inline.h"

V8_INLINE static double FXJSE_ftod(FX_FLOAT fNumber) {
  if (sizeof(FX_FLOAT) != 4) {
    ASSERT(FALSE);
    return fNumber;
  }

  uint32_t nFloatBits = (uint32_t&)fNumber;
  uint8_t nExponent = (uint8_t)(nFloatBits >> 16 >> 7);
  if (nExponent == 0 || nExponent == 255)
    return fNumber;

  int8_t nErrExp = nExponent - 127 - 23;
  if (nErrExp >= 0)
    return fNumber;

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

class CFXJSE_Value {
 public:
  CFXJSE_Value(v8::Isolate* pIsolate) : m_pIsolate(pIsolate) {}

  V8_INLINE FX_BOOL IsUndefined() const {
    if (m_hValue.IsEmpty()) {
      return FALSE;
    }
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return hValue->IsUndefined();
  }
  V8_INLINE FX_BOOL IsNull() const {
    if (m_hValue.IsEmpty()) {
      return FALSE;
    }
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return hValue->IsNull();
  }
  V8_INLINE FX_BOOL IsBoolean() const {
    if (m_hValue.IsEmpty()) {
      return FALSE;
    }
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return hValue->IsBoolean();
  }
  V8_INLINE FX_BOOL IsString() const {
    if (m_hValue.IsEmpty()) {
      return FALSE;
    }
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return hValue->IsString();
  }
  V8_INLINE FX_BOOL IsNumber() const {
    if (m_hValue.IsEmpty()) {
      return FALSE;
    }
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return hValue->IsNumber();
  }
  V8_INLINE FX_BOOL IsInteger() const {
    if (m_hValue.IsEmpty()) {
      return FALSE;
    }
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return hValue->IsInt32();
  }
  V8_INLINE FX_BOOL IsObject() const {
    if (m_hValue.IsEmpty()) {
      return FALSE;
    }
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return hValue->IsObject();
  }
  V8_INLINE FX_BOOL IsArray() const {
    if (m_hValue.IsEmpty()) {
      return FALSE;
    }
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return hValue->IsArray();
  }
  V8_INLINE FX_BOOL IsFunction() const {
    if (m_hValue.IsEmpty()) {
      return FALSE;
    }
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return hValue->IsFunction();
  }
  V8_INLINE FX_BOOL IsDate() const {
    if (m_hValue.IsEmpty()) {
      return FALSE;
    }
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return hValue->IsDate();
  }

  V8_INLINE FX_BOOL ToBoolean() const {
    ASSERT(!m_hValue.IsEmpty());
    CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return static_cast<FX_BOOL>(hValue->BooleanValue());
  }
  V8_INLINE FX_FLOAT ToFloat() const {
    ASSERT(!m_hValue.IsEmpty());
    CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return static_cast<FX_FLOAT>(hValue->NumberValue());
  }
  V8_INLINE double ToDouble() const {
    ASSERT(!m_hValue.IsEmpty());
    CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return static_cast<double>(hValue->NumberValue());
  }
  V8_INLINE int32_t ToInteger() const {
    ASSERT(!m_hValue.IsEmpty());
    CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    return static_cast<int32_t>(hValue->NumberValue());
  }
  V8_INLINE CFX_ByteString ToString() const {
    ASSERT(!m_hValue.IsEmpty());
    CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Local<v8::Value>::New(m_pIsolate, m_hValue);
    v8::Local<v8::String> hString = hValue->ToString();
    v8::String::Utf8Value hStringVal(hString);
    return CFX_ByteString(*hStringVal);
  }
  V8_INLINE CFX_WideString ToWideString() const {
    return CFX_WideString::FromUTF8(ToString().AsStringC());
  }
  CFXJSE_HostObject* ToHostObject(CFXJSE_Class* lpClass) const;

  V8_INLINE void SetUndefined() {
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue = v8::Undefined(m_pIsolate);
    m_hValue.Reset(m_pIsolate, hValue);
  }
  V8_INLINE void SetNull() {
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue = v8::Null(m_pIsolate);
    m_hValue.Reset(m_pIsolate, hValue);
  }
  V8_INLINE void SetBoolean(FX_BOOL bBoolean) {
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue =
        v8::Boolean::New(m_pIsolate, bBoolean != FALSE);
    m_hValue.Reset(m_pIsolate, hValue);
  }
  V8_INLINE void SetInteger(int32_t nInteger) {
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue = v8::Integer::New(m_pIsolate, nInteger);
    m_hValue.Reset(m_pIsolate, hValue);
  }
  V8_INLINE void SetDouble(double dDouble) {
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue = v8::Number::New(m_pIsolate, dDouble);
    m_hValue.Reset(m_pIsolate, hValue);
  }
  V8_INLINE void SetString(const CFX_ByteStringC& szString) {
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> hValue = v8::String::NewFromUtf8(
        m_pIsolate, reinterpret_cast<const char*>(szString.raw_str()),
        v8::String::kNormalString, szString.GetLength());
    m_hValue.Reset(m_pIsolate, hValue);
  }
  V8_INLINE void SetFloat(FX_FLOAT fFloat) {
    CFXJSE_ScopeUtil_IsolateHandle scope(m_pIsolate);
    v8::Local<v8::Value> pValue =
        v8::Number::New(m_pIsolate, FXJSE_ftod(fFloat));
    m_hValue.Reset(m_pIsolate, pValue);
  }
  V8_INLINE void SetJSObject() {
    CFXJSE_ScopeUtil_IsolateHandleRootContext scope(m_pIsolate);
    v8::Local<v8::Value> hValue = v8::Object::New(m_pIsolate);
    m_hValue.Reset(m_pIsolate, hValue);
  }

  void SetObject(CFXJSE_HostObject* lpObject, CFXJSE_Class* pClass);
  void SetHostObject(CFXJSE_HostObject* lpObject, CFXJSE_Class* lpClass);
  void SetArray(uint32_t uValueCount, CFXJSE_Value** rgValues);
  void SetDate(double dDouble);

  FX_BOOL GetObjectProperty(const CFX_ByteStringC& szPropName,
                            CFXJSE_Value* lpPropValue);
  FX_BOOL SetObjectProperty(const CFX_ByteStringC& szPropName,
                            CFXJSE_Value* lpPropValue);
  FX_BOOL GetObjectPropertyByIdx(uint32_t uPropIdx, CFXJSE_Value* lpPropValue);
  FX_BOOL SetObjectProperty(uint32_t uPropIdx, CFXJSE_Value* lpPropValue);
  FX_BOOL DeleteObjectProperty(const CFX_ByteStringC& szPropName);
  FX_BOOL HasObjectOwnProperty(const CFX_ByteStringC& szPropName,
                               FX_BOOL bUseTypeGetter);
  FX_BOOL SetObjectOwnProperty(const CFX_ByteStringC& szPropName,
                               CFXJSE_Value* lpPropValue);
  FX_BOOL SetFunctionBind(CFXJSE_Value* lpOldFunction, CFXJSE_Value* lpNewThis);
  FX_BOOL Call(CFXJSE_Value* lpReceiver,
               CFXJSE_Value* lpRetValue,
               uint32_t nArgCount,
               CFXJSE_Value** lpArgs);

  V8_INLINE v8::Isolate* GetIsolate() const { return m_pIsolate; }
  V8_INLINE const v8::Global<v8::Value>& DirectGetValue() const {
    return m_hValue;
  }
  V8_INLINE void ForceSetValue(v8::Local<v8::Value> hValue) {
    m_hValue.Reset(m_pIsolate, hValue);
  }
  V8_INLINE void Assign(const CFXJSE_Value* lpValue) {
    ASSERT(lpValue);
    if (lpValue) {
      m_hValue.Reset(m_pIsolate, lpValue->m_hValue);
    } else {
      m_hValue.Reset();
    }
  }

 private:
  friend class CFXJSE_Class;
  friend class CFXJSE_Context;

  CFXJSE_Value();
  CFXJSE_Value(const CFXJSE_Value&);
  CFXJSE_Value& operator=(const CFXJSE_Value&);

  v8::Isolate* m_pIsolate;
  v8::Global<v8::Value> m_hValue;
};

#endif  // XFA_FXJSE_VALUE_H_
