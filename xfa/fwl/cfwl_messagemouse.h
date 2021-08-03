// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_MESSAGEMOUSE_H_
#define XFA_FWL_CFWL_MESSAGEMOUSE_H_

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fwl/cfwl_message.h"

enum class FWL_MouseCommand : uint8_t {
  LeftButtonDown,
  LeftButtonUp,
  LeftButtonDblClk,
  RightButtonDown,
  RightButtonUp,
  RightButtonDblClk,
  Move,
  Enter,
  Leave,
  Hover
};

class CFWL_MessageMouse final : public CFWL_Message {
 public:
  CFWL_MessageMouse(CFWL_Widget* pDstTarget, FWL_MouseCommand cmd);
  CFWL_MessageMouse(CFWL_Widget* pDstTarget,
                    FWL_MouseCommand cmd,
                    FWL_KeyFlagMask flags,
                    CFX_PointF pos);
  ~CFWL_MessageMouse() override;

  const FWL_MouseCommand m_dwCmd;
  FWL_KeyFlagMask m_dwFlags = 0;
  CFX_PointF m_pos;
};

#endif  // XFA_FWL_CFWL_MESSAGEMOUSE_H_
