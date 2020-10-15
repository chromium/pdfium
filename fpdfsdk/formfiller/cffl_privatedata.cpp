// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/formfiller/cffl_privatedata.h"

#include "fpdfsdk/cpdfsdk_widget.h"
#include "third_party/base/ptr_util.h"

CFFL_PrivateData::CFFL_PrivateData(CPDFSDK_Widget* pWidget,
                                   CPDFSDK_PageView* pPageView,
                                   uint32_t nAppearanceAge,
                                   uint32_t nValueAge)
    : m_pWidget(pWidget),
      m_pPageView(pPageView),
      m_nAppearanceAge(nAppearanceAge),
      m_nValueAge(nValueAge) {}

CFFL_PrivateData::CFFL_PrivateData(const CFFL_PrivateData& that) = default;

CFFL_PrivateData::~CFFL_PrivateData() = default;

std::unique_ptr<IPWL_SystemHandler::PerWindowData> CFFL_PrivateData::Clone()
    const {
  // Private constructor.
  return pdfium::WrapUnique(new CFFL_PrivateData(*this));
}
