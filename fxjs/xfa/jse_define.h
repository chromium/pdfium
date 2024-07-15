// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_JSE_DEFINE_H_
#define FXJS_XFA_JSE_DEFINE_H_

#include "core/fxcrt/span.h"
#include "fxjs/cjs_result.h"

class CFXJSE_Engine;

#define JSE_METHOD(method_name)                                      \
  static CJS_Result method_name##_static(                            \
      CJX_Object* node, CFXJSE_Engine* runtime,                      \
      pdfium::span<v8::Local<v8::Value>> params) {                   \
    if (!node->DynamicTypeIs(static_type__))                         \
      return CJS_Result::Failure(JSMessage::kBadObjectError);        \
    return static_cast<Type__*>(node)->method_name(runtime, params); \
  }                                                                  \
  CJS_Result method_name(CFXJSE_Engine* runtime,                     \
                         pdfium::span<v8::Local<v8::Value>> params)

#define JSE_PROP(prop_name)                                                 \
  static void prop_name##_static(v8::Isolate* pIsolate, CJX_Object* node,   \
                                 v8::Local<v8::Value>* value, bool setting, \
                                 XFA_Attribute attribute) {                 \
    if (node->DynamicTypeIs(static_type__))                                 \
      static_cast<Type__*>(node)->prop_name(pIsolate, value, setting,       \
                                            attribute);                     \
  }                                                                         \
  void prop_name(v8::Isolate* pIsolate, v8::Local<v8::Value>* pValue,       \
                 bool bSetting, XFA_Attribute eAttribute)

#endif  // FXJS_XFA_JSE_DEFINE_H_
