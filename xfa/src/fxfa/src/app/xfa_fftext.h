// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FXFA_SRC_APP_XFA_FFTEXT_H_
#define XFA_SRC_FXFA_SRC_APP_XFA_FFTEXT_H_

#include "xfa/src/fxfa/src/app/xfa_ffdraw.h"

class CXFA_TextLayout;

class CXFA_FFText : public CXFA_FFDraw {
 public:
  CXFA_FFText(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFText();
  virtual FX_BOOL OnLButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnLButtonUp(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnMouseMove(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_DWORD OnHitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            FX_DWORD dwStatus = 0,
                            int32_t iRotate = 0);
  virtual FX_BOOL IsLoaded();
  virtual FX_BOOL PerformLayout();

 private:
  virtual const FX_WCHAR* GetLinkURLAtPoint(FX_FLOAT fx, FX_FLOAT fy);
  void FWLToClient(FX_FLOAT& fx, FX_FLOAT& fy);
};

#endif  // XFA_SRC_FXFA_SRC_APP_XFA_FFTEXT_H_
