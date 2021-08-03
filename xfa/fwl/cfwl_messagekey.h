// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_MESSAGEKEY_H_
#define XFA_FWL_CFWL_MESSAGEKEY_H_

#include "xfa/fwl/cfwl_message.h"
#include "xfa/fwl/fwl_widgetdef.h"

class CFWL_MessageKey final : public CFWL_Message {
 public:
  enum class KeyCommand : uint8_t { kKeyDown, kKeyUp, kChar };

  CFWL_MessageKey(CFWL_Widget* pDstTarget,
                  KeyCommand subtype,
                  FWL_KeyFlagMask flags,
                  XFA_FWL_VKEYCODE keycode);
  ~CFWL_MessageKey() override;

  const KeyCommand m_dwCmd;
  const FWL_KeyFlagMask m_dwFlags;
  const XFA_FWL_VKEYCODE m_dwKeyCode;
};

#endif  // XFA_FWL_CFWL_MESSAGEKEY_H_
