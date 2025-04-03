// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_messagekey.h"

namespace pdfium {

CFWL_MessageKey::CFWL_MessageKey(CFWL_Widget* pDstTarget,
                                 KeyCommand cmd,
                                 Mask<XFA_FWL_KeyFlag> flags,
                                 uint32_t dwKeyCodeOrChar)
    : CFWL_Message(CFWL_Message::Type::kKey, pDstTarget),
      cmd_(cmd),
      flags_(flags),
      key_code_or_char_(dwKeyCodeOrChar) {}

CFWL_MessageKey::~CFWL_MessageKey() = default;

}  // namespace pdfium
