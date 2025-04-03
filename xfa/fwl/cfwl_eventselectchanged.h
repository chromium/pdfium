// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_EVENTSELECTCHANGED_H_
#define XFA_FWL_CFWL_EVENTSELECTCHANGED_H_

#include <stdint.h>

#include "xfa/fwl/cfwl_event.h"

namespace pdfium {

class CFWL_EventSelectChanged final : public CFWL_Event {
 public:
  CFWL_EventSelectChanged(CFWL_Widget* pSrcTarget, bool bLButtonUp);
  CFWL_EventSelectChanged(CFWL_Widget* pSrcTarget,
                          int32_t iYear,
                          int32_t iMonth,
                          int32_t iDay);
  ~CFWL_EventSelectChanged() override;

  bool GetLButtonUp() const { return lbutton_up_; }
  int32_t GetYear() const { return year_; }
  int32_t GetMonth() const { return month_; }
  int32_t GetDay() const { return day_; }

 protected:
  // Used by ComboBox.
  const bool lbutton_up_;

  // Used by DateTimePIcker
  const int32_t year_;
  const int32_t month_;
  const int32_t day_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_EventSelectChanged;

#endif  // XFA_FWL_CFWL_EVENTSELECTCHANGED_H_
