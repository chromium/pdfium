// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_messagekey.h"

CFWL_MessageKey::CFWL_MessageKey(CFWL_Widget* pDstTarget,
                                 KeyCommand cmd,
                                 FWL_KeyFlagMask flags,
                                 XFA_FWL_VKEYCODE keycode)
    : CFWL_Message(CFWL_Message::Type::kKey, nullptr, pDstTarget),
      m_dwCmd(cmd),
      m_dwFlags(flags),
      m_dwKeyCode(keycode) {}

CFWL_MessageKey::~CFWL_MessageKey() = default;
