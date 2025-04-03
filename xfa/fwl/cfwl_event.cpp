// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_event.h"

namespace pdfium {

CFWL_Event::CFWL_Event(CFWL_Event::Type type) : type_(type) {}

CFWL_Event::CFWL_Event(Type type, CFWL_Widget* pSrcTarget)
    : type_(type), src_target_(pSrcTarget) {}

CFWL_Event::CFWL_Event(Type type,
                       CFWL_Widget* pSrcTarget,
                       CFWL_Widget* pDstTarget)
    : type_(type), src_target_(pSrcTarget), dst_target_(pDstTarget) {}

CFWL_Event::~CFWL_Event() = default;

}  // namespace pdfium
