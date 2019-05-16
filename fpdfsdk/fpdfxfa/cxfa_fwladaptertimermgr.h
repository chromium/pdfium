// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FPDFXFA_CXFA_FWLADAPTERTIMERMGR_H_
#define FPDFSDK_FPDFXFA_CXFA_FWLADAPTERTIMERMGR_H_

#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "xfa/fwl/cfwl_timerinfo.h"
#include "xfa/fwl/ifwl_adaptertimermgr.h"

class CXFA_FWLAdapterTimerMgr final : public IFWL_AdapterTimerMgr {
 public:
  explicit CXFA_FWLAdapterTimerMgr(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  ~CXFA_FWLAdapterTimerMgr() override;

  CFWL_TimerInfo* Start(CFWL_Timer* pTimer,
                        uint32_t dwElapse,
                        bool bImmediately) override;
  void Stop(CFWL_TimerInfo* pTimerInfo) override;

 private:
  UnownedPtr<CPDFSDK_FormFillEnvironment> const m_pFormFillEnv;
};

#endif  // FPDFSDK_FPDFXFA_CXFA_FWLADAPTERTIMERMGR_H_
