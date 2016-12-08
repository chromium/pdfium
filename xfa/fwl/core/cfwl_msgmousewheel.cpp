// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_msgmousewheel.h"

#include <memory>

#include "third_party/base/ptr_util.h"

CFWL_MsgMouseWheel::CFWL_MsgMouseWheel(CFWL_Widget* pSrcTarget,
                                       CFWL_Widget* pDstTarget)
    : CFWL_Message(CFWL_Message::Type::MouseWheel, pSrcTarget, pDstTarget) {}

CFWL_MsgMouseWheel::~CFWL_MsgMouseWheel() {}

std::unique_ptr<CFWL_Message> CFWL_MsgMouseWheel::Clone() {
  return pdfium::MakeUnique<CFWL_MsgMouseWheel>(*this);
}
