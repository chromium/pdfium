// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_V8_H_
#define FXJS_CJS_V8_H_

#include <v8-util.h>
#include <v8.h>

#include <map>
#include <vector>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/widestring.h"

#ifdef PDF_ENABLE_XFA
class CXFA_Object;
#endif  // PDF_ENABLE_XFA

class CJS_V8 {
 public:
  explicit CJS_V8(v8::Isolate* pIsolate);
  virtual ~CJS_V8();

  v8::Isolate* GetIsolate() const { return m_isolate; }

  v8::Local<v8::Context> NewLocalContext();
  v8::Local<v8::Context> GetPersistentContext();

  v8::Local<v8::Value> NewNull();
  v8::Local<v8::Value> NewUndefined();
  v8::Local<v8::Array> NewArray();
  v8::Local<v8::Number> NewNumber(int number);
  v8::Local<v8::Number> NewNumber(double number);
  v8::Local<v8::Number> NewNumber(float number);
  v8::Local<v8::Boolean> NewBoolean(bool b);
  v8::Local<v8::String> NewString(const ByteStringView& str);
  v8::Local<v8::String> NewString(const WideStringView& str);
  v8::Local<v8::Date> NewDate(double d);

  int ToInt32(v8::Local<v8::Value> pValue);
  bool ToBoolean(v8::Local<v8::Value> pValue);
  double ToDouble(v8::Local<v8::Value> pValue);
  WideString ToWideString(v8::Local<v8::Value> pValue);
  ByteString ToByteString(v8::Local<v8::Value> pValue);
  v8::Local<v8::Object> ToObject(v8::Local<v8::Value> pValue);
  v8::Local<v8::Array> ToArray(v8::Local<v8::Value> pValue);

#ifdef PDF_ENABLE_XFA
  CXFA_Object* ToXFAObject(v8::Local<v8::Value> obj);
  v8::Local<v8::Value> NewXFAObject(CXFA_Object* obj,
                                    v8::Global<v8::FunctionTemplate>& tmpl);
#endif  // PDF_ENABLE_XFA

  // Arrays.
  unsigned GetArrayLength(v8::Local<v8::Array> pArray);
  v8::Local<v8::Value> GetArrayElement(v8::Local<v8::Array> pArray,
                                       unsigned index);
  unsigned PutArrayElement(v8::Local<v8::Array> pArray,
                           unsigned index,
                           v8::Local<v8::Value> pValue);

  void SetConstArray(const WideString& name, v8::Local<v8::Array> array);
  v8::Local<v8::Array> GetConstArray(const WideString& name);

  // Objects.
  std::vector<WideString> GetObjectPropertyNames(v8::Local<v8::Object> pObj);
  v8::Local<v8::Value> GetObjectProperty(v8::Local<v8::Object> pObj,
                                         const WideString& PropertyName);
  void PutObjectProperty(v8::Local<v8::Object> pObj,
                         const WideString& PropertyName,
                         v8::Local<v8::Value> pValue);

 protected:
  void SetIsolate(v8::Isolate* pIsolate) { m_isolate = pIsolate; }
  void ClearConstArray() { m_ConstArrays.clear(); }

  void ResetPersistentContext(v8::Local<v8::Context> context) {
    m_V8PersistentContext.Reset(m_isolate, context);
  }
  void ReleasePersistentContext() { m_V8PersistentContext.Reset(); }

 private:
  v8::Isolate* m_isolate;
  std::map<WideString, v8::Global<v8::Array>> m_ConstArrays;
  v8::Global<v8::Context> m_V8PersistentContext;
};

#endif  // FXJS_CJS_V8_H_
