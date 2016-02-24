// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/app/xfa_ffapp.h"
#include "xfa/src/fxfa/src/app/xfa_ffdoc.h"
#include "xfa/src/fxfa/src/app/xfa_ffpageview.h"
#include "xfa/src/fxfa/src/app/xfa_ffsubform.h"
#include "xfa/src/fxfa/src/app/xfa_ffwidget.h"

CXFA_FFSubForm::CXFA_FFSubForm(CXFA_FFPageView* pPageView,
                               CXFA_WidgetAcc* pDataAcc)
    : CXFA_FFWidget(pPageView, pDataAcc) {}
CXFA_FFSubForm::~CXFA_FFSubForm() {}
