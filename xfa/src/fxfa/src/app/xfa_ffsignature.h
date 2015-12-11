// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_SIGNATURE_IMP_H
#define _FXFA_FORMFILLER_SIGNATURE_IMP_H
class CXFA_FFSignature final : public CXFA_FFField {
 public:
  CXFA_FFSignature(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFSignature();

  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            FX_DWORD dwStatus = 0,
                            int32_t iRotate = 0);
  virtual FX_BOOL LoadWidget();
  virtual FX_BOOL OnMouseEnter();
  virtual FX_BOOL OnMouseExit();
  virtual FX_BOOL OnLButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnLButtonUp(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnLButtonDblClk(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnMouseMove(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnMouseWheel(FX_DWORD dwFlags,
                               int16_t zDelta,
                               FX_FLOAT fx,
                               FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnRButtonUp(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDblClk(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);

  virtual FX_BOOL OnKeyDown(FX_DWORD dwKeyCode, FX_DWORD dwFlags);
  virtual FX_BOOL OnKeyUp(FX_DWORD dwKeyCode, FX_DWORD dwFlags);
  virtual FX_BOOL OnChar(FX_DWORD dwChar, FX_DWORD dwFlags);
  virtual FX_DWORD OnHitTest(FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnSetCursor(FX_FLOAT fx, FX_FLOAT fy);

 protected:
};
#endif
