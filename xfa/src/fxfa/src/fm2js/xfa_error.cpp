// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa_fm2js.h"
static FX_LPCWSTR gs_lpStrErrorMsgInfo[] = {
    (FX_LPCWSTR)(L"unsupported char '%c'"),
    (FX_LPCWSTR)(L"bad suffix on number"),
    (FX_LPCWSTR)(L"invalidate char '%c'"),
    (FX_LPCWSTR)(L"expected identifier instead of '%s'"),
    (FX_LPCWSTR)(L"expected '%s' instead of '%s'"),
    (FX_LPCWSTR)(L"expected 'endif' instead of '%s'"),
    (FX_LPCWSTR)(L"unexpected expression '%s'"),
    (FX_LPCWSTR)(L"expected operator '%s' instead of '%s'"),
};
FX_LPCWSTR XFA_FM_ErrorMsg(XFA_FM_ERRMSG msg)
{
    if(msg < FMERR_MAXIMUM) {
        return gs_lpStrErrorMsgInfo[msg];
    } else {
        return (FX_LPCWSTR)(L"");
    }
}
