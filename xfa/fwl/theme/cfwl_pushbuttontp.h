// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_THEME_CFWL_PUSHBUTTONTP_H_
#define XFA_FWL_THEME_CFWL_PUSHBUTTONTP_H_

#include <array>
#include <memory>

#include "fxjs/gc/heap.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

namespace pdfium {

class CFWL_PushButtonTP final : public CFWL_WidgetTP {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_PushButtonTP() override;

  // CFWL_WidgetTP:
  void DrawBackground(const CFWL_ThemeBackground& pParams) override;

 private:
  struct PBThemeData {
    std::array<FX_ARGB, 5> clrBorder;
    std::array<FX_ARGB, 5> clrStart;
    std::array<FX_ARGB, 5> clrEnd;
    std::array<FX_ARGB, 5> clrFill;
  };

  CFWL_PushButtonTP();

  int32_t GetColorID(Mask<CFWL_PartState> dwStates) const;
  void SetThemeData();

  std::unique_ptr<PBThemeData> m_pThemeData;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_PushButtonTP;

#endif  // XFA_FWL_THEME_CFWL_PUSHBUTTONTP_H_
