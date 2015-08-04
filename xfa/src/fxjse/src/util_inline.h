// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJSE_UTIL_INLINE_H_
#define FXJSE_UTIL_INLINE_H_
static V8_INLINE v8::Local<v8::Object> FXJSE_GetGlobalObjectFromContext(
    const v8::Local<v8::Context>& hContext) {
  return hContext->Global()->GetPrototype().As<v8::Object>();
}
static V8_INLINE void FXJSE_UpdateObjectBinding(v8::Local<v8::Object>& hObject,
                                                void* lpNewBinding) {
  ASSERT(!hObject.IsEmpty());
  ASSERT(hObject->InternalFieldCount() > 0);
  hObject->SetAlignedPointerInInternalField(0, lpNewBinding);
}
static V8_INLINE void* FXJSE_RetrieveObjectBinding(
    const v8::Local<v8::Object>& hJSObject,
    CFXJSE_Class* lpClass = NULL) {
  ASSERT(!hJSObject.IsEmpty());
  if (!hJSObject->IsObject()) {
    return NULL;
  }
  v8::Local<v8::Object> hObject = hJSObject;
  if (hObject->InternalFieldCount() == 0) {
    v8::Local<v8::Value> hProtoObject = hObject->GetPrototype();
    if (hProtoObject.IsEmpty() || !hProtoObject->IsObject()) {
      return NULL;
    }
    hObject = hProtoObject.As<v8::Object>();
    if (hObject->InternalFieldCount() == 0) {
      return NULL;
    }
  }
  if (lpClass) {
    v8::Local<v8::FunctionTemplate> hClass =
        v8::Local<v8::FunctionTemplate>::New(
            lpClass->GetContext()->GetRuntime(), lpClass->GetTemplate());
    if (!hClass->HasInstance(hObject)) {
      return NULL;
    }
  }
  return hObject->GetAlignedPointerFromInternalField(0);
}
#endif
