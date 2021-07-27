// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_FXJSE_H_
#define FXJS_XFA_FXJSE_H_

#include <stdint.h>

#include "core/fxcrt/fx_string.h"
#include "v8/include/v8.h"

namespace pdfium {
namespace fxjse {

// These are strings by design. With ASLR, their addresses should be random, so
// it should be very unlikely for an object to accidentally have the same tag.
extern const char kFuncTag[];
extern const char kClassTag[];

}  // namespace fxjse
}  // namespace pdfium

class CFXJSE_FormCalcContext;
class CJS_Result;
class CJX_Object;

// C++ object which is retrieved from v8 object's slot.
class CFXJSE_HostObject {
 public:
  static CFXJSE_HostObject* FromV8(v8::Local<v8::Value> arg);
  virtual ~CFXJSE_HostObject();

  // Two subclasses.
  virtual CFXJSE_FormCalcContext* AsFormCalcContext();
  virtual CJX_Object* AsCJXObject();

  v8::Local<v8::Object> NewBoundV8Object(v8::Isolate* pIsolate,
                                         v8::Local<v8::FunctionTemplate> tmpl);

 protected:
  CFXJSE_HostObject();
};

typedef CJS_Result (*FXJSE_MethodCallback)(
    const v8::FunctionCallbackInfo<v8::Value>& info,
    const WideString& functionName);
typedef void (*FXJSE_FuncCallback)(
    CFXJSE_HostObject* pThis,
    const v8::FunctionCallbackInfo<v8::Value>& info);
typedef v8::Local<v8::Value> (*FXJSE_PropGetter)(v8::Isolate* pIsolate,
                                                 v8::Local<v8::Object> pObject,
                                                 ByteStringView szPropName);
typedef void (*FXJSE_PropSetter)(v8::Isolate* pIsolate,
                                 v8::Local<v8::Object> pObject,
                                 ByteStringView szPropName,
                                 v8::Local<v8::Value> pValue);
typedef int32_t (*FXJSE_PropTypeGetter)(v8::Isolate* pIsolate,
                                        v8::Local<v8::Object> pObject,
                                        ByteStringView szPropName,
                                        bool bQueryIn);

enum FXJSE_ClassPropTypes {
  FXJSE_ClassPropType_None,
  FXJSE_ClassPropType_Property,
  FXJSE_ClassPropType_Method
};

struct FXJSE_FUNCTION_DESCRIPTOR {
  const char* tag;  // `pdfium::fxjse::kFuncTag` always.
  const char* name;
  FXJSE_FuncCallback callbackProc;
};

struct FXJSE_CLASS_DESCRIPTOR {
  const char* tag;  // `pdfium::fxjse::kClassTag` always.
  const char* name;
  const FXJSE_FUNCTION_DESCRIPTOR* methods;
  int32_t methNum;
  FXJSE_PropTypeGetter dynPropTypeGetter;
  FXJSE_PropGetter dynPropGetter;
  FXJSE_PropSetter dynPropSetter;
  FXJSE_MethodCallback dynMethodCall;
};

extern const FXJSE_CLASS_DESCRIPTOR GlobalClassDescriptor;
extern const FXJSE_CLASS_DESCRIPTOR NormalClassDescriptor;
extern const FXJSE_CLASS_DESCRIPTOR VariablesClassDescriptor;
extern const FXJSE_CLASS_DESCRIPTOR kFormCalcFM2JSDescriptor;

void FXJSE_ThrowMessage(ByteStringView utf8Message);

#endif  // FXJS_XFA_FXJSE_H_
