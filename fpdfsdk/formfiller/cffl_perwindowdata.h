// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FORMFILLER_CFFL_PERWINDOWDATA_H_
#define FPDFSDK_FORMFILLER_CFFL_PERWINDOWDATA_H_

#include <memory>

#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/pwl/ipwl_fillernotify.h"

class CFFL_FormField;
class CPDFSDK_PageView;
class CPDFSDK_Widget;

class CFFL_PerWindowData final : public IPWL_FillerNotify::PerWindowData {
 public:
  CFFL_PerWindowData(CPDFSDK_Widget* pWidget,
                     const CPDFSDK_PageView* pPageView,
                     uint32_t nAppearanceAge,
                     uint32_t nValueAge);
  CFFL_PerWindowData& operator=(const CFFL_PerWindowData& that) = delete;
  ~CFFL_PerWindowData() override;

  // IPWL_FillerNotify::PerWindowData:
  std::unique_ptr<IPWL_FillerNotify::PerWindowData> Clone() const override;

  CPDFSDK_Widget* GetWidget() const { return widget_.Get(); }
  const CPDFSDK_PageView* GetPageView() const { return page_view_; }
  bool AppearanceAgeEquals(uint32_t age) const {
    return age == appearance_age_;
  }
  uint32_t GetValueAge() const { return value_age_; }

  void SetFormField(CFFL_FormField* pFormField) { form_field_ = pFormField; }
  CFFL_FormField* GetFormField() { return form_field_; }

 private:
  CFFL_PerWindowData(const CFFL_PerWindowData& that);

  ObservedPtr<CPDFSDK_Widget> widget_;
  UnownedPtr<const CPDFSDK_PageView> const page_view_;
  UnownedPtr<CFFL_FormField> form_field_;
  const uint32_t appearance_age_;
  const uint32_t value_age_;
};

#endif  // FPDFSDK_FORMFILLER_CFFL_PERWINDOWDATA_H_
