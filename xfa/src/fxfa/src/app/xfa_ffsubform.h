// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FXFA_SRC_APP_XFA_FFSUBFORM_H_
#define XFA_SRC_FXFA_SRC_APP_XFA_FFSUBFORM_H_

#include "xfa/src/fxfa/src/app/xfa_ffpageview.h"
#include "xfa/src/fxfa/src/app/xfa_ffwidget.h"

class CXFA_FFSubForm : public CXFA_FFWidget {
 public:
  CXFA_FFSubForm(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFSubForm();
};

#endif  // XFA_SRC_FXFA_SRC_APP_XFA_FFSUBFORM_H_
