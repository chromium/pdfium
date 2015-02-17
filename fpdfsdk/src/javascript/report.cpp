// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/report.h"

/* ---------------------- report ---------------------- */

BEGIN_JS_STATIC_CONST(CJS_Report)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_Report)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_Report)
	JS_STATIC_METHOD_ENTRY(save, 1)
	JS_STATIC_METHOD_ENTRY(writeText,1)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_Report, Report)

Report::Report(CJS_Object* pJSObject) : CJS_EmbedObj(pJSObject)
{

}

Report::~Report()
{

}

FX_BOOL Report::writeText(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Report::save(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
  // Unsafe, not supported.
  return TRUE;
}
