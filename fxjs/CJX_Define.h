// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_DEFINE_H_
#define FXJS_CJX_DEFINE_H_

#include "fxjs/cfxjse_arguments.h"

template <class C, void (C::*M)(CFXJSE_Arguments* args)>
void JSMethod(C* node, CFXJSE_Arguments* args) {
  (node->*M)(args);
}

#define JS_METHOD(method_name, class_name)                                     \
  static void method_name##_static(CJX_Object* node, CFXJSE_Arguments* args) { \
    JSMethod<class_name, &class_name::method_name>(                            \
        static_cast<class_name*>(node), args);                                 \
  }                                                                            \
  void method_name(CFXJSE_Arguments* pArguments)

#endif  // FXJS_CJX_DEFINE_H_
