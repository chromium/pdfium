// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FFEXCLGROUP_H_
#define XFA_FXFA_APP_XFA_FFEXCLGROUP_H_

#include "xfa/include/fxfa/xfa_ffpageview.h"
#include "xfa/include/fxfa/xfa_ffwidget.h"

class CXFA_FFExclGroup : public CXFA_FFWidget {
 public:
  CXFA_FFExclGroup(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFExclGroup();

  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            uint32_t dwStatus = 0,
                            int32_t iRotate = 0);
};

#endif  // XFA_FXFA_APP_XFA_FFEXCLGROUP_H_
