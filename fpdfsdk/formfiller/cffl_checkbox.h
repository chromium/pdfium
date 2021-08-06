// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FORMFILLER_CFFL_CHECKBOX_H_
#define FPDFSDK_FORMFILLER_CFFL_CHECKBOX_H_

#include <memory>

#include "fpdfsdk/formfiller/cffl_button.h"

class CPWL_CheckBox;

class CFFL_CheckBox final : public CFFL_Button {
 public:
  CFFL_CheckBox(CPDFSDK_FormFillEnvironment* pApp, CPDFSDK_Widget* pWidget);
  ~CFFL_CheckBox() override;

  // CFFL_Button:
  std::unique_ptr<CPWL_Wnd> NewPWLWindow(
      const CPWL_Wnd::CreateParams& cp,
      std::unique_ptr<IPWL_SystemHandler::PerWindowData> pAttachedData)
      override;
  bool OnKeyDown(FWL_VKEYCODE nKeyCode, FWL_EventFlagMask nFlags) override;
  bool OnChar(CPDFSDK_Annot* pAnnot,
              uint32_t nChar,
              FWL_EventFlagMask nFlags) override;
  bool OnLButtonUp(CPDFSDK_PageView* pPageView,
                   CPDFSDK_Annot* pAnnot,
                   FWL_EventFlagMask nFlags,
                   const CFX_PointF& point) override;
  bool IsDataChanged(const CPDFSDK_PageView* pPageView) override;
  void SaveData(const CPDFSDK_PageView* pPageView) override;

 private:
  CPWL_CheckBox* GetCheckBox(const CPDFSDK_PageView* pPageView) const;
  CPWL_CheckBox* CreateOrUpdateCheckBox(const CPDFSDK_PageView* pPageView);
};

#endif  // FPDFSDK_FORMFILLER_CFFL_CHECKBOX_H_
