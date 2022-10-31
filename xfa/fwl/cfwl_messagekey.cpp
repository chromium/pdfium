// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_messagekey.h"

CFWL_MessageKey::CFWL_MessageKey(CFWL_Widget* pDstTarget,
                                 KeyCommand cmd,
                                 Mask<XFA_FWL_KeyFlag> flags,
                                 uint32_t dwKeyCodeOrChar)
    : CFWL_Message(CFWL_Message::Type::kKey, pDstTarget),
      m_dwCmd(cmd),
      m_dwFlags(flags),
      m_dwKeyCodeOrChar(dwKeyCodeOrChar) {}

CFWL_MessageKey::~CFWL_MessageKey() = default;
