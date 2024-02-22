// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_VALUE_H_
#define FXJS_XFA_CFXJSE_VALUE_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/unowned_ptr.h"
#include "v8/include/v8-forward.h"
#include "v8/include/v8-persistent-handle.h"

class CFXJSE_Class;
class CFXJSE_HostObject;

class CFXJSE_Value {
 public:
  CFXJSE_Value();
  CFXJSE_Value(v8::Isolate* pIsolate, v8::Local<v8::Value> value);
  ~CFXJSE_Value();

  bool IsEmpty() const;
  bool IsUndefined(v8::Isolate* pIsolate) const;
  bool IsNull(v8::Isolate* pIsolate) const;
  bool IsBoolean(v8::Isolate* pIsolate) const;
  bool IsString(v8::Isolate* pIsolate) const;
  bool IsNumber(v8::Isolate* pIsolate) const;
  bool IsInteger(v8::Isolate* pIsolate) const;
  bool IsObject(v8::Isolate* pIsolate) const;
  bool IsArray(v8::Isolate* pIsolate) const;
  bool IsFunction(v8::Isolate* pIsolate) const;
  bool ToBoolean(v8::Isolate* pIsolate) const;
  float ToFloat(v8::Isolate* pIsolate) const;
  double ToDouble(v8::Isolate* pIsolate) const;
  int32_t ToInteger(v8::Isolate* pIsolate) const;
  ByteString ToString(v8::Isolate* pIsolate) const;
  WideString ToWideString(v8::Isolate* pIsolate) const {
    return WideString::FromUTF8(ToString(pIsolate).AsStringView());
  }
  CFXJSE_HostObject* ToHostObject(v8::Isolate* pIsolate) const;

  void SetUndefined(v8::Isolate* pIsolate);
  void SetNull(v8::Isolate* pIsolate);
  void SetBoolean(v8::Isolate* pIsolate, bool bBoolean);
  void SetInteger(v8::Isolate* pIsolate, int32_t nInteger);
  void SetDouble(v8::Isolate* pIsolate, double dDouble);
  void SetString(v8::Isolate* pIsolate, ByteStringView szString);
  void SetFloat(v8::Isolate* pIsolate, float fFloat);

  void SetHostObject(v8::Isolate* pIsolate,
                     CFXJSE_HostObject* pObject,
                     CFXJSE_Class* pClass);

  void SetArray(v8::Isolate* pIsolate,
                const std::vector<std::unique_ptr<CFXJSE_Value>>& values);

  bool GetObjectProperty(v8::Isolate* pIsolate,
                         ByteStringView szPropName,
                         CFXJSE_Value* pPropValue);
  bool SetObjectProperty(v8::Isolate* pIsolate,
                         ByteStringView szPropName,
                         CFXJSE_Value* pPropValue);
  bool GetObjectPropertyByIdx(v8::Isolate* pIsolate,
                              uint32_t uPropIdx,
                              CFXJSE_Value* pPropValue);
  void DeleteObjectProperty(v8::Isolate* pIsolate, ByteStringView szPropName);
  bool SetObjectOwnProperty(v8::Isolate* pIsolate,
                            ByteStringView szPropName,
                            CFXJSE_Value* pPropValue);

  // Return empty local on error.
  static v8::Local<v8::Function> NewBoundFunction(
      v8::Isolate* pIsolate,
      v8::Local<v8::Function> hOldFunction,
      v8::Local<v8::Object> lpNewThis);

  v8::Local<v8::Value> GetValue(v8::Isolate* pIsolate) const;
  const v8::Global<v8::Value>& DirectGetValue() const { return m_hValue; }
  void ForceSetValue(v8::Isolate* pIsolate, v8::Local<v8::Value> hValue) {
    m_hValue.Reset(pIsolate, hValue);
  }

 private:
  CFXJSE_Value(const CFXJSE_Value&) = delete;
  CFXJSE_Value& operator=(const CFXJSE_Value&) = delete;

  v8::Global<v8::Value> m_hValue;
};

#endif  // FXJS_XFA_CFXJSE_VALUE_H_
