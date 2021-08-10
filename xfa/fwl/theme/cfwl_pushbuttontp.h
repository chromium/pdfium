// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_PUSHBUTTONTP_H_
#define XFA_FWL_THEME_CFWL_PUSHBUTTONTP_H_

#include <memory>

#include "fxjs/gc/heap.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

class CFWL_PushButtonTP final : public CFWL_WidgetTP {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_PushButtonTP() override;

  // CFWL_WidgetTP:
  void DrawBackground(const CFWL_ThemeBackground& pParams) override;

 private:
  struct PBThemeData {
    FX_ARGB clrBorder[5];
    FX_ARGB clrStart[5];
    FX_ARGB clrEnd[5];
    FX_ARGB clrFill[5];
  };

  CFWL_PushButtonTP();

  int32_t GetColorID(Mask<CFWL_PartState> dwStates) const;
  void SetThemeData();

  std::unique_ptr<PBThemeData> m_pThemeData;
};

#endif  // XFA_FWL_THEME_CFWL_PUSHBUTTONTP_H_
