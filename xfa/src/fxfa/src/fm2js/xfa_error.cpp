// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa_fm2js.h"
static const FX_WCHAR* gs_lpStrErrorMsgInfo[] = {
    L"unsupported char '%c'",         L"bad suffix on number",
    L"invalidate char '%c'",          L"expected identifier instead of '%s'",
    L"expected '%s' instead of '%s'", L"expected 'endif' instead of '%s'",
    L"unexpected expression '%s'",    L"expected operator '%s' instead of '%s'",
};
const FX_WCHAR* XFA_FM_ErrorMsg(XFA_FM_ERRMSG msg) {
  if (msg < FMERR_MAXIMUM) {
    return gs_lpStrErrorMsgInfo[msg];
  } else {
    return L"";
  }
}
