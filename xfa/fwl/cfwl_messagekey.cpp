// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_messagekey.h"

#include <memory>

#include "third_party/base/ptr_util.h"

CFWL_MessageKey::CFWL_MessageKey(CFWL_Widget* pDstTarget,
                                 FWL_KeyCommand cmd,
                                 uint32_t flags,
                                 uint32_t keycode)
    : CFWL_Message(CFWL_Message::Type::Key, nullptr, pDstTarget),
      m_dwCmd(cmd),
      m_dwFlags(flags),
      m_dwKeyCode(keycode) {}

CFWL_MessageKey::~CFWL_MessageKey() = default;
