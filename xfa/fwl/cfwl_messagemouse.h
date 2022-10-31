// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_MESSAGEMOUSE_H_
#define XFA_FWL_CFWL_MESSAGEMOUSE_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/mask.h"
#include "xfa/fwl/cfwl_message.h"
#include "xfa/fwl/fwl_widgetdef.h"

class CFWL_MessageMouse final : public CFWL_Message {
 public:
  enum class MouseCommand : uint8_t {
    kLeftButtonDown,
    kLeftButtonUp,
    kLeftButtonDblClk,
    kRightButtonDown,
    kRightButtonUp,
    kRightButtonDblClk,
    kMove,
    kEnter,
    kLeave,
    kHover
  };

  CFWL_MessageMouse(CFWL_Widget* pDstTarget,
                    MouseCommand cmd,
                    Mask<XFA_FWL_KeyFlag> flags,
                    CFX_PointF pos);
  ~CFWL_MessageMouse() override;

  const MouseCommand m_dwCmd;
  Mask<XFA_FWL_KeyFlag> m_dwFlags;
  CFX_PointF m_pos;
};

#endif  // XFA_FWL_CFWL_MESSAGEMOUSE_H_
