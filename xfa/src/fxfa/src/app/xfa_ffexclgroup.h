// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_EXCLGROUP_IMP_H
#define _FXFA_FORMFILLER_EXCLGROUP_IMP_H
class CXFA_FFExclGroup : public CXFA_FFWidget {
 public:
  CXFA_FFExclGroup(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFExclGroup();

  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            FX_DWORD dwStatus = 0,
                            int32_t iRotate = 0);

 protected:
};
#endif
