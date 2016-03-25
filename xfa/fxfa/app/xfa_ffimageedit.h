// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FFIMAGEEDIT_H_
#define XFA_FXFA_APP_XFA_FFIMAGEEDIT_H_

#include "xfa/fxfa/app/xfa_fffield.h"

class CXFA_FFImageEdit : public CXFA_FFField {
 public:
  CXFA_FFImageEdit(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFImageEdit();

  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            uint32_t dwStatus = 0,
                            int32_t iRotate = 0);
  virtual FX_BOOL LoadWidget();
  virtual void UnloadWidget();
  virtual FX_BOOL OnLButtonDown(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy);
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

#endif  // XFA_FXFA_APP_XFA_FFIMAGEEDIT_H_
