// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_MESSAGEKEY_H_
#define XFA_FWL_CFWL_MESSAGEKEY_H_

#include <memory>

#include "xfa/fwl/cfwl_message.h"

enum class FWL_KeyCommand { KeyDown, KeyUp, Char };

class CFWL_MessageKey final : public CFWL_Message {
 public:
  CFWL_MessageKey(CFWL_Widget* pDstTarget,
                  FWL_KeyCommand cmd,
                  uint32_t flags,
                  uint32_t keycode);
  ~CFWL_MessageKey() override;

  const FWL_KeyCommand m_dwCmd;
  const uint32_t m_dwFlags;
  const uint32_t m_dwKeyCode;
};

#endif  // XFA_FWL_CFWL_MESSAGEKEY_H_
