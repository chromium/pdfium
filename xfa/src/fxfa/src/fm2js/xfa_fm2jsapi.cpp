// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa_fm2js.h"
#ifdef __cplusplus
extern "C" {
#endif
int32_t XFA_FM2JS_Translate(const CFX_WideStringC& wsFormcalc,
                            CFX_WideTextBuf& wsJavascript,
                            CFX_WideString& wsError) {
  if (wsFormcalc.IsEmpty()) {
    wsJavascript.Clear();
    wsError.Empty();
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
XFA_HFM2JSCONTEXT XFA_FM2JS_ContextCreate() {
  return (XFA_HFM2JSCONTEXT)CXFA_FM2JSContext::Create();
}
void XFA_FM2JS_ContextInitialize(XFA_HFM2JSCONTEXT hFM2JSContext,
                                 FXJSE_HRUNTIME hScriptRuntime,
                                 FXJSE_HCONTEXT hScriptContext,
                                 CXFA_Document* pDocument) {
  CXFA_FM2JSContext* pContext =
      reinterpret_cast<CXFA_FM2JSContext*>(hFM2JSContext);
  pContext->Initialize(hScriptRuntime, hScriptContext, pDocument);
}
void XFA_FM2JS_GlobalPropertyGetter(XFA_HFM2JSCONTEXT hFM2JSContext,
                                    FXJSE_HVALUE hValue) {
  CXFA_FM2JSContext* pContext =
      reinterpret_cast<CXFA_FM2JSContext*>(hFM2JSContext);
  pContext->GlobalPropertyGetter(hValue);
}
void XFA_FM2JS_ContextRelease(XFA_HFM2JSCONTEXT hFM2JSContext) {
  CXFA_FM2JSContext* pContext =
      reinterpret_cast<CXFA_FM2JSContext*>(hFM2JSContext);
  pContext->Release();
}
#ifdef __cplusplus
}
#endif
