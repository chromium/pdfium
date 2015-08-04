// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_IMAGEEDIT_IMP_H
#define _FXFA_FORMFILLER_IMAGEEDIT_IMP_H
class CXFA_FFImageEdit : public CXFA_FFField {
 public:
  CXFA_FFImageEdit(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFImageEdit();

  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            FX_DWORD dwStatus = 0,
                            int32_t iRotate = 0);
  virtual FX_BOOL LoadWidget();
  virtual void UnloadWidget();
  virtual FX_BOOL OnLButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual int32_t OnProcessMessage(CFWL_Message* pMessage);
  virtual FWL_ERR OnProcessEvent(CFWL_Event* pEvent);
  virtual FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = NULL);

 protected:
  virtual void SetFWLRect();
  virtual FX_BOOL UpdateFWLData();
  virtual FX_BOOL CommitData();
  IFWL_WidgetDelegate* m_pOldDelegate;
};
#endif
