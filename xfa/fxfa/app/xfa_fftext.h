// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FFTEXT_H_
#define XFA_FXFA_APP_XFA_FFTEXT_H_

#include "xfa/fxfa/app/xfa_ffdraw.h"

class CXFA_FFText : public CXFA_FFDraw {
 public:
  CXFA_FFText(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFText();
  virtual FX_BOOL OnLButtonDown(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnLButtonUp(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnMouseMove(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual uint32_t OnHitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            uint32_t dwStatus = 0,
                            int32_t iRotate = 0);
  virtual FX_BOOL IsLoaded();
  virtual FX_BOOL PerformLayout();

 private:
  virtual const FX_WCHAR* GetLinkURLAtPoint(FX_FLOAT fx, FX_FLOAT fy);
  void FWLToClient(FX_FLOAT& fx, FX_FLOAT& fy);
};

#endif  // XFA_FXFA_APP_XFA_FFTEXT_H_
