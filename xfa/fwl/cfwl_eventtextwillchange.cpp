// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_eventtextwillchange.h"

namespace pdfium {

CFWL_EventTextWillChange::CFWL_EventTextWillChange(
    CFWL_Widget* pSrcTarget,
    const WideString& change_text,
    const WideString& previous_text,
    size_t selection_start,
    size_t selection_end)
    : CFWL_Event(CFWL_Event::Type::TextWillChange, pSrcTarget),
      change_text_(change_text),
      previous_text_(previous_text),
      selection_start_(selection_start),
      selection_end_(selection_end) {}

CFWL_EventTextWillChange::~CFWL_EventTextWillChange() = default;

}  // namespace pdfium
