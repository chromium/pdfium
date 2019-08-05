// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CFX_SYSTEMHANDLER_H_
#define FPDFSDK_CFX_SYSTEMHANDLER_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"

class CFFL_FormFiller;
class CPDFSDK_FormFillEnvironment;
class CPDFSDK_Widget;

class CFX_SystemHandler final : public IPWL_SystemHandler {
 public:
  explicit CFX_SystemHandler(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  ~CFX_SystemHandler() override;

  void InvalidateRect(CPDFSDK_Widget* widget,
                      const CFX_FloatRect& rect) override;
  void OutputSelectedRect(CFFL_FormFiller* pFormFiller,
                          const CFX_FloatRect& rect) override;
  bool IsSelectionImplemented() const override;
  void SetCursor(int32_t nCursorType) override;
  int32_t SetTimer(int32_t uElapse, TimerCallback lpTimerFunc) override;
  void KillTimer(int32_t nID) override;

 private:
  UnownedPtr<CPDFSDK_FormFillEnvironment> const m_pFormFillEnv;
};

#endif  // FPDFSDK_CFX_SYSTEMHANDLER_H_
