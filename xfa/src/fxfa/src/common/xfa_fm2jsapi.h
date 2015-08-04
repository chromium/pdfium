// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_FM2JS_API_H
#define _XFA_FM2JS_API_H
#define FOXIT_XFA_FM2JS_FORMCALC_RUNTIME "foxit_xfa_formcalc_runtime"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct XFA_HFM2JSCONTEXT_ { void** pData; } * XFA_HFM2JSCONTEXT;
int32_t XFA_FM2JS_Translate(const CFX_WideStringC& wsFormcalc,
                            CFX_WideTextBuf& wsJavascript,
                            CFX_WideString& wsError);
XFA_HFM2JSCONTEXT XFA_FM2JS_ContextCreate();
void XFA_FM2JS_ContextInitialize(XFA_HFM2JSCONTEXT hFM2JSContext,
                                 FXJSE_HRUNTIME hScriptRuntime,
                                 FXJSE_HCONTEXT hScriptContext,
                                 CXFA_Document* pDocument);
void XFA_FM2JS_GlobalPropertyGetter(XFA_HFM2JSCONTEXT hFM2JSContext,
                                    FXJSE_HVALUE hValue);
void XFA_FM2JS_ContextRelease(XFA_HFM2JSCONTEXT hFM2JSContext);
#ifdef __cplusplus
}
#endif
#endif
