// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_MESSAGEMOUSE_H_
#define XFA_FWL_CFWL_MESSAGEMOUSE_H_

#include <type_traits>

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

enum FWL_KeyFlag : uint8_t {
  FWL_KEYFLAG_Ctrl = 1 << 0,
  FWL_KEYFLAG_Alt = 1 << 1,
  FWL_KEYFLAG_Shift = 1 << 2,
  FWL_KEYFLAG_Command = 1 << 3,
  FWL_KEYFLAG_LButton = 1 << 4,
  FWL_KEYFLAG_RButton = 1 << 5,
  FWL_KEYFLAG_MButton = 1 << 6
};
using FWL_KeyFlagMask = std::underlying_type<FWL_KeyFlag>::type;

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
