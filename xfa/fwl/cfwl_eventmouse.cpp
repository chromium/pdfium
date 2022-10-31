// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_eventmouse.h"

CFWL_EventMouse::CFWL_EventMouse(CFWL_Widget* pSrcTarget,
                                 CFWL_Widget* pDstTarget,
                                 CFWL_MessageMouse::MouseCommand cmd)
    : CFWL_Event(CFWL_Event::Type::Mouse, pSrcTarget, pDstTarget),
      m_dwCmd(cmd) {}

CFWL_EventMouse::~CFWL_EventMouse() = default;
