// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_FM2JS_H
#define _XFA_FM2JS_H
#include "../../../foxitlib.h"
#include "../common/xfa_utils.h"
#include "../common/xfa_object.h"
#include "../common/xfa_document.h"
#include "../common/xfa_parser.h"
#include "../common/xfa_script.h"
#include "../common/xfa_docdata.h"
#include "../common/xfa_doclayout.h"
#include "../common/xfa_debug.h"
#include "../common/xfa_localemgr.h"
#include "../common/xfa_fm2jsapi.h"
#define  RUNTIMEFUNCTIONRETURNVALUE (FX_WSTRC(L"foxit_xfa_formcalc_runtime_func_return_value"))
#define  EXCLAMATION_IN_IDENTIFIER	(FX_WSTRC(L"foxit_xfa_formcalc__exclamation__"))
typedef CFX_ArrayTemplate<CFX_WideStringC> CFX_WideStringCArray;
#include "xfa_error.h"
#include "xfa_lexer.h"
#include "xfa_simpleexpression.h"
#include "xfa_expression.h"
#include "xfa_fmparse.h"
#include "xfa_program.h"
#include "xfa_fm2jscontext.h"
#endif
