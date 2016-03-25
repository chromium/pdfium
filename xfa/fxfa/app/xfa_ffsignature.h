// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FFSIGNATURE_H_
#define XFA_FXFA_APP_XFA_FFSIGNATURE_H_

#include "xfa/fxfa/app/xfa_fffield.h"

class CXFA_FFSignature final : public CXFA_FFField {
 public:
  CXFA_FFSignature(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFSignature();

  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            uint32_t dwStatus = 0,
                            int32_t iRotate = 0);
  virtual FX_BOOL LoadWidget();
  virtual FX_BOOL OnMouseEnter();
  virtual FX_BOOL OnMouseExit();
  virtual FX_BOOL OnLButtonDown(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnLButtonUp(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnLButtonDblClk(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnMouseMove(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnMouseWheel(uint32_t dwFlags,
                               int16_t zDelta,
                               FX_FLOAT fx,
                               FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDown(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnRButtonUp(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDblClk(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);

  virtual FX_BOOL OnKeyDown(uint32_t dwKeyCode, uint32_t dwFlags);
  virtual FX_BOOL OnKeyUp(uint32_t dwKeyCode, uint32_t dwFlags);
  virtual FX_BOOL OnChar(uint32_t dwChar, uint32_t dwFlags);
  virtual uint32_t OnHitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnSetCursor(FX_FLOAT fx, FX_FLOAT fy);
};

#endif  // XFA_FXFA_APP_XFA_FFSIGNATURE_H_
