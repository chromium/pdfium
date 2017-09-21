// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FSDK_PAUSEADAPTER_H_
#define FPDFSDK_FSDK_PAUSEADAPTER_H_

#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/ifx_pauseindicator.h"
#include "core/fxcrt/unowned_ptr.h"
#include "public/fpdf_progressive.h"

class IFSDK_PAUSE_Adapter : public IFX_PauseIndicator {
 public:
  explicit IFSDK_PAUSE_Adapter(IFSDK_PAUSE* IPause);
  ~IFSDK_PAUSE_Adapter() override;

  bool NeedToPauseNow() override;

 private:
  UnownedPtr<IFSDK_PAUSE> const m_IPause;
};

#endif  // FPDFSDK_FSDK_PAUSEADAPTER_H_
