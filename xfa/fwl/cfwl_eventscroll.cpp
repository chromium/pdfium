// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_eventscroll.h"

CFWL_EventScroll::CFWL_EventScroll(CFWL_Widget* pSrcTarget,
                                   Code code,
                                   float pos)
    : CFWL_Event(CFWL_Event::Type::Scroll, pSrcTarget),
      m_iScrollCode(code),
      m_fPos(pos) {}

CFWL_EventScroll::~CFWL_EventScroll() = default;
