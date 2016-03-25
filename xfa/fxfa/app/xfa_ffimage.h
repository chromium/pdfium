// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FFIMAGE_H_
#define XFA_FXFA_APP_XFA_FFIMAGE_H_

#include "xfa/fxfa/app/xfa_ffdraw.h"

class CXFA_FFImage : public CXFA_FFDraw {
 public:
  CXFA_FFImage(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFImage();
  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            uint32_t dwStatus = 0,
                            int32_t iRotate = 0);
  virtual FX_BOOL IsLoaded();
  virtual FX_BOOL LoadWidget();
  virtual void UnloadWidget();
};

#endif  // XFA_FXFA_APP_XFA_FFIMAGE_H_
