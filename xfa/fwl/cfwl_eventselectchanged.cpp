// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_eventselectchanged.h"

namespace pdfium {

CFWL_EventSelectChanged::CFWL_EventSelectChanged(CFWL_Widget* pSrcTarget,
                                                 bool bLButtonUp)
    : CFWL_Event(CFWL_Event::Type::SelectChanged, pSrcTarget),
      m_bLButtonUp(bLButtonUp),
      m_iYear(-1),
      m_iMonth(-1),
      m_iDay(-1) {}

CFWL_EventSelectChanged::CFWL_EventSelectChanged(CFWL_Widget* pSrcTarget,
                                                 int32_t iYear,
                                                 int32_t iMonth,
                                                 int32_t iDay)
    : CFWL_Event(CFWL_Event::Type::SelectChanged, pSrcTarget),
      m_bLButtonUp(false),
      m_iYear(iYear),
      m_iMonth(iMonth),
      m_iDay(iDay) {}

CFWL_EventSelectChanged::~CFWL_EventSelectChanged() = default;

}  // namespace pdfium
