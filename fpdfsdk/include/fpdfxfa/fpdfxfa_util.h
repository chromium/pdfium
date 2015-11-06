// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FPDFXFA_FPDFXFA_UTIL_H_
#define FPDFSDK_INCLUDE_FPDFXFA_FPDFXFA_UTIL_H_

#include "xfa/include/fwl/adapter/fwl_adaptertimermgr.h"

#define JS_STR_VIEWERTYPE_STANDARD L"Exchange"
#define JS_STR_LANGUANGE L"ENU"
#define JS_STR_VIEWERVARIATION L"Full"
#define JS_STR_VIEWERVERSION_XFA L"11"

class CXFA_FWLAdapterTimerMgr : public IFWL_AdapterTimerMgr {
 public:
  CXFA_FWLAdapterTimerMgr(CPDFDoc_Environment* pEnv) : m_pEnv(pEnv) {}
  virtual FWL_ERR Start(IFWL_Timer* pTimer,
                        FX_DWORD dwElapse,
                        FWL_HTIMER& hTimer,
                        FX_BOOL bImmediately = TRUE);
  virtual FWL_ERR Stop(FWL_HTIMER hTimer);

 protected:
  static void TimerProc(int32_t idEvent);

  static CFX_PtrArray ms_timerArray;
  CPDFDoc_Environment* m_pEnv;
};

class CFWL_TimerInfo {
 public:
  CFWL_TimerInfo() : pTimer(nullptr) {}
  uint32_t uIDEvent;
  IFWL_Timer* pTimer;
};

#endif  // FPDFSDK_INCLUDE_FPDFXFA_FPDFXFA_UTIL_H_
