// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_PRINTPARAMSOBJ_H_
#define FXJS_CJS_PRINTPARAMSOBJ_H_

#include "fxjs/JS_Define.h"

class PrintParamsObj : public CJS_EmbedObj {
 public:
  explicit PrintParamsObj(CJS_Object* pJSObject);
  ~PrintParamsObj() override {}

 public:
  bool bUI;
  int nStart;
  int nEnd;
  bool bSilent;
  bool bShrinkToFit;
  bool bPrintAsImage;
  bool bReverse;
  bool bAnnotations;
};

class CJS_PrintParamsObj : public CJS_Object {
 public:
  static int GetObjDefnID();
  static void DefineJSObjects(CFXJS_Engine* pEngine);

  explicit CJS_PrintParamsObj(v8::Local<v8::Object> pObject)
      : CJS_Object(pObject) {}
  ~CJS_PrintParamsObj() override {}

 private:
  static int ObjDefnID;
};

#endif  // FXJS_CJS_PRINTPARAMSOBJ_H_
