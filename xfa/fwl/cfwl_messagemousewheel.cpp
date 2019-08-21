// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_messagemousewheel.h"

#include <memory>

#include "third_party/base/ptr_util.h"

CFWL_MessageMouseWheel::CFWL_MessageMouseWheel(CFWL_Widget* pDstTarget,
                                               uint32_t flags,
                                               CFX_PointF pos,
                                               CFX_PointF delta)
    : CFWL_Message(CFWL_Message::Type::MouseWheel, nullptr, pDstTarget),
      m_dwFlags(flags),
      m_pos(pos),
      m_delta(delta) {}

CFWL_MessageMouseWheel::~CFWL_MessageMouseWheel() = default;
