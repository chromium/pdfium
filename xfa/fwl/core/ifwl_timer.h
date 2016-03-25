// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_TIMER_H_
#define XFA_FWL_CORE_IFWL_TIMER_H_

#include "core/fxcrt/include/fx_system.h"

typedef struct FWL_HTIMER_ { void* pData; } * FWL_HTIMER;

class IFWL_Timer {
 public:
  virtual ~IFWL_Timer() {}
  virtual int32_t Run(FWL_HTIMER hTimer) = 0;
};
FWL_HTIMER FWL_StartTimer(IFWL_Timer* pTimer,
                          uint32_t dwElapse,
                          FX_BOOL bImmediately = TRUE);
int32_t FWL_StopTimer(FWL_HTIMER hTimer);

#endif  // XFA_FWL_CORE_IFWL_TIMER_H_
