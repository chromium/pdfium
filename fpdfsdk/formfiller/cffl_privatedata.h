// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FORMFILLER_CFFL_PRIVATEDATA_H_
#define FPDFSDK_FORMFILLER_CFFL_PRIVATEDATA_H_

#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"

class CPDFSDK_PageView;
class CPDFSDK_Widget;

class CFFL_PrivateData final : public IPWL_SystemHandler::PerWindowData {
 public:
  CFFL_PrivateData(CPDFSDK_Widget* pWidget,
                   CPDFSDK_PageView* pPageView,
                   uint32_t nAppearanceAge,
                   uint32_t nValueAge);
  CFFL_PrivateData& operator=(const CFFL_PrivateData& that) = delete;
  ~CFFL_PrivateData() override;

  // IPWL_SystemHandler::PerWindowData:
  std::unique_ptr<IPWL_SystemHandler::PerWindowData> Clone() const override;

  CPDFSDK_Widget* GetWidget() const { return m_pWidget.Get(); }
  CPDFSDK_PageView* GetPageView() const { return m_pPageView; }
  bool AppearanceAgeEquals(uint32_t age) const {
    return age == m_nAppearanceAge;
  }
  bool ValueAgeEquals(uint32_t age) const { return age == m_nValueAge; }

 private:
  CFFL_PrivateData(const CFFL_PrivateData& that);

  ObservedPtr<CPDFSDK_Widget> m_pWidget;
  UnownedPtr<CPDFSDK_PageView> const m_pPageView;
  const uint32_t m_nAppearanceAge;
  const uint32_t m_nValueAge;
};

#endif  // FPDFSDK_FORMFILLER_CFFL_PRIVATEDATA_H_
