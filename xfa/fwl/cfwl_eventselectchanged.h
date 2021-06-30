// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_EVENTSELECTCHANGED_H_
#define XFA_FWL_CFWL_EVENTSELECTCHANGED_H_

#include <stdint.h>

#include "xfa/fwl/cfwl_event.h"

class CFWL_EventSelectChanged final : public CFWL_Event {
 public:
  CFWL_EventSelectChanged(CFWL_Widget* pSrcTarget, bool bLButtonUp);
  CFWL_EventSelectChanged(CFWL_Widget* pSrcTarget,
                          int32_t iYear,
                          int32_t iMonth,
                          int32_t iDay);
  ~CFWL_EventSelectChanged() override;

  bool GetLButtonUp() const { return m_bLButtonUp; }
  int32_t GetYear() const { return m_iYear; }
  int32_t GetMonth() const { return m_iMonth; }
  int32_t GetDay() const { return m_iDay; }

 protected:
  // Used by ComboBox.
  const bool m_bLButtonUp;

  // Used by DateTimePIcker
  const int32_t m_iYear;
  const int32_t m_iMonth;
  const int32_t m_iDay;
};

#endif  // XFA_FWL_CFWL_EVENTSELECTCHANGED_H_
