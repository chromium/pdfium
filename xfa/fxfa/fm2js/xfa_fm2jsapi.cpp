// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/xfa_fm2jsapi.h"

#include "core/fxcrt/include/fx_basic.h"
#include "xfa/fxfa/fm2js/xfa_fm2jscontext.h"
#include "xfa/fxfa/fm2js/xfa_program.h"
#include "xfa/fxfa/parser/xfa_document.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t XFA_FM2JS_Translate(const CFX_WideStringC& wsFormcalc,
                            CFX_WideTextBuf& wsJavascript,
                            CFX_WideString& wsError) {
  if (wsFormcalc.IsEmpty()) {
    wsJavascript.Clear();
    wsError.clear();
    return 0;
  }
  int32_t status = 0;
  CXFA_FMProgram program;
  status = program.Init(wsFormcalc);
  if (status) {
    wsError = program.GetError().message;
    return status;
  }
  status = program.ParseProgram();
  if (status) {
    wsError = program.GetError().message;
    return status;
  }
  program.TranslateProgram(wsJavascript);
  return 0;
}

XFA_HFM2JSCONTEXT XFA_FM2JS_ContextCreate(v8::Isolate* pScriptIsolate,
                                          CFXJSE_Context* pScriptContext,
                                          CXFA_Document* pDocument) {
  return reinterpret_cast<XFA_HFM2JSCONTEXT>(
      new CXFA_FM2JSContext(pScriptIsolate, pScriptContext, pDocument));
}

void XFA_FM2JS_GlobalPropertyGetter(XFA_HFM2JSCONTEXT hFM2JSContext,
                                    CFXJSE_Value* pValue) {
  CXFA_FM2JSContext* pContext =
      reinterpret_cast<CXFA_FM2JSContext*>(hFM2JSContext);
  pContext->GlobalPropertyGetter(pValue);
}

void XFA_FM2JS_ContextRelease(XFA_HFM2JSCONTEXT hFM2JSContext) {
  delete reinterpret_cast<CXFA_FM2JSContext*>(hFM2JSContext);
}
#ifdef __cplusplus
}
#endif
