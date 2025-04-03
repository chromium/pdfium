// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_MESSAGEKEY_H_
#define XFA_FWL_CFWL_MESSAGEKEY_H_

#include "core/fxcrt/mask.h"
#include "xfa/fwl/cfwl_message.h"
#include "xfa/fwl/fwl_widgetdef.h"

namespace pdfium {

class CFWL_MessageKey final : public CFWL_Message {
 public:
  enum class KeyCommand : uint8_t { kKeyDown, kChar };

  CFWL_MessageKey(CFWL_Widget* pDstTarget,
                  KeyCommand subtype,
                  Mask<XFA_FWL_KeyFlag> flags,
                  uint32_t dwKeyCodeOrChar);
  ~CFWL_MessageKey() override;

  const KeyCommand cmd_;
  const Mask<XFA_FWL_KeyFlag> flags_;
  const uint32_t key_code_or_char_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_MessageKey;

#endif  // XFA_FWL_CFWL_MESSAGEKEY_H_
