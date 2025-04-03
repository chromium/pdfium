// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_eventselectchanged.h"

namespace pdfium {

CFWL_EventSelectChanged::CFWL_EventSelectChanged(CFWL_Widget* pSrcTarget,
                                                 bool bLButtonUp)
    : CFWL_Event(CFWL_Event::Type::SelectChanged, pSrcTarget),
      lbutton_up_(bLButtonUp),
      year_(-1),
      month_(-1),
      day_(-1) {}

CFWL_EventSelectChanged::CFWL_EventSelectChanged(CFWL_Widget* pSrcTarget,
                                                 int32_t iYear,
                                                 int32_t iMonth,
                                                 int32_t iDay)
    : CFWL_Event(CFWL_Event::Type::SelectChanged, pSrcTarget),
      lbutton_up_(false),
      year_(iYear),
      month_(iMonth),
      day_(iDay) {}

CFWL_EventSelectChanged::~CFWL_EventSelectChanged() = default;

}  // namespace pdfium
