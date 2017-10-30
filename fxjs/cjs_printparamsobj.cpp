// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_printparamsobj.h"

int CJS_PrintParamsObj::ObjDefnID = -1;

// static
int CJS_PrintParamsObj::GetObjDefnID() {
  return ObjDefnID;
}

// static
void CJS_PrintParamsObj::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID =
      pEngine->DefineObj("PrintParamsObj", FXJSOBJTYPE_DYNAMIC,
                         JSConstructor<CJS_PrintParamsObj, PrintParamsObj>,
                         JSDestructor<CJS_PrintParamsObj>);
}

PrintParamsObj::PrintParamsObj(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject) {
  bUI = true;
  nStart = 0;
  nEnd = 0;
  bSilent = false;
  bShrinkToFit = false;
  bPrintAsImage = false;
  bReverse = false;
  bAnnotations = true;
}
