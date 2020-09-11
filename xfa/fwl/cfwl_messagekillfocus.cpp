// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_messagekillfocus.h"

CFWL_MessageKillFocus::CFWL_MessageKillFocus(CFWL_Widget* pSrcTarget)
    : CFWL_MessageKillFocus(pSrcTarget, nullptr) {}

CFWL_MessageKillFocus::CFWL_MessageKillFocus(CFWL_Widget* pSrcTarget,
                                             CFWL_Widget* pDstTarget)
    : CFWL_Message(CFWL_Message::Type::kKillFocus, pSrcTarget, pDstTarget) {}

CFWL_MessageKillFocus::~CFWL_MessageKillFocus() = default;
