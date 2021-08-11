// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_IPWL_FILLERNOTIFY_H_
#define FPDFSDK_PWL_IPWL_FILLERNOTIFY_H_

#include <utility>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/mask.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"

class IPWL_FillerNotify {
 public:
  virtual ~IPWL_FillerNotify() = default;

  // Must write to |bBottom| and |fPopupRet|.
  virtual void QueryWherePopup(
      const IPWL_SystemHandler::PerWindowData* pAttached,
      float fPopupMin,
      float fPopupMax,
      bool* bBottom,
      float* fPopupRet) = 0;

  virtual std::pair<bool, bool> OnBeforeKeyStroke(
      const IPWL_SystemHandler::PerWindowData* pAttached,
      WideString& strChange,
      const WideString& strChangeEx,
      int nSelStart,
      int nSelEnd,
      bool bKeyDown,
      Mask<FWL_EVENTFLAG> nFlag) = 0;

  virtual bool OnPopupPreOpen(
      const IPWL_SystemHandler::PerWindowData* pAttached,
      Mask<FWL_EVENTFLAG> nFlag) = 0;

  virtual bool OnPopupPostOpen(
      const IPWL_SystemHandler::PerWindowData* pAttached,
      Mask<FWL_EVENTFLAG> nFlag) = 0;
};

#endif  // FPDFSDK_PWL_IPWL_FILLERNOTIFY_H_
