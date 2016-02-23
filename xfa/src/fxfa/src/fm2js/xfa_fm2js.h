// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FXFA_SRC_FM2JS_XFA_FM2JS_H_
#define XFA_SRC_FXFA_SRC_FM2JS_XFA_FM2JS_H_

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_docdata.h"
#include "xfa/src/fxfa/src/common/xfa_doclayout.h"
#include "xfa/src/fxfa/src/common/xfa_document.h"
#include "xfa/src/fxfa/src/common/xfa_fm2jsapi.h"
#include "xfa/src/fxfa/src/common/xfa_localemgr.h"
#include "xfa/src/fxfa/src/common/xfa_object.h"
#include "xfa/src/fxfa/src/common/xfa_parser.h"
#include "xfa/src/fxfa/src/common/xfa_script.h"
#include "xfa/src/fxfa/src/common/xfa_utils.h"
#include "xfa/src/fxfa/src/fm2js/xfa_error.h"
#include "xfa/src/fxfa/src/fm2js/xfa_expression.h"
#include "xfa/src/fxfa/src/fm2js/xfa_fm2jscontext.h"
#include "xfa/src/fxfa/src/fm2js/xfa_fmparse.h"
#include "xfa/src/fxfa/src/fm2js/xfa_lexer.h"
#include "xfa/src/fxfa/src/fm2js/xfa_program.h"
#include "xfa/src/fxfa/src/fm2js/xfa_simpleexpression.h"

#define RUNTIMEFUNCTIONRETURNVALUE \
  (FX_WSTRC(L"foxit_xfa_formcalc_runtime_func_return_value"))
#define EXCLAMATION_IN_IDENTIFIER \
  (FX_WSTRC(L"foxit_xfa_formcalc__exclamation__"))

#endif  // XFA_SRC_FXFA_SRC_FM2JS_XFA_FM2JS_H_
