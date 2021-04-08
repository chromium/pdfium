// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_SBBUTTON_H_
#define FPDFSDK_PWL_CPWL_SBBUTTON_H_

#include <memory>

#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"

enum PWL_SBBUTTON_TYPE { PSBT_MIN, PSBT_MAX, PSBT_POS };

class CPWL_SBButton final : public CPWL_Wnd {
 public:
  CPWL_SBButton(
      const CreateParams& cp,
      std::unique_ptr<IPWL_SystemHandler::PerWindowData> pAttachedData,
      PWL_SBBUTTON_TYPE eButtonType);
  ~CPWL_SBButton() override;

  // CPWL_Wnd
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          const CFX_Matrix& mtUser2Device) override;
  bool OnLButtonDown(uint32_t nFlag, const CFX_PointF& point) override;
  bool OnLButtonUp(uint32_t nFlag, const CFX_PointF& point) override;
  bool OnMouseMove(uint32_t nFlag, const CFX_PointF& point) override;

 private:
  PWL_SBBUTTON_TYPE m_eSBButtonType;
  bool m_bMouseDown = false;
};

#endif  // FPDFSDK_PWL_CPWL_SBBUTTON_H_
