// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_CJS_BORDER_H_
#define FPDFSDK_JAVASCRIPT_CJS_BORDER_H_

#include "fpdfsdk/javascript/JS_Define.h"

class CJS_Border : public CJS_Object {
 public:
  explicit CJS_Border(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Border() override {}

  DECLARE_JS_CLASS_BASE_PART();
  DECLARE_JS_CLASS_CONST_PART();
};

#endif  // FPDFSDK_JAVASCRIPT_CJS_BORDER_H_
