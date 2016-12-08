// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_MESSAGEMOUSEWHEEL_H_
#define XFA_FWL_CFWL_MESSAGEMOUSEWHEEL_H_

#include <memory>

#include "xfa/fwl/cfwl_message.h"

class CFWL_MessageMouseWheel : public CFWL_Message {
 public:
  CFWL_MessageMouseWheel(CFWL_Widget* pSrcTarget, CFWL_Widget* pDstTarget);
  ~CFWL_MessageMouseWheel() override;

  // CFWL_Message
  std::unique_ptr<CFWL_Message> Clone() override;

  FX_FLOAT m_fx;
  FX_FLOAT m_fy;
  FX_FLOAT m_fDeltaX;
  FX_FLOAT m_fDeltaY;
  uint32_t m_dwFlags;
};

#endif  // XFA_FWL_CFWL_MESSAGEMOUSEWHEEL_H_
