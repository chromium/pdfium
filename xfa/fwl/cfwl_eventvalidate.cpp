// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_eventvalidate.h"

CFWL_EventValidate::CFWL_EventValidate(CFWL_Widget* pSrcTarget,
                                       const WideString& wsInsert)
    : CFWL_Event(CFWL_Event::Type::Validate, pSrcTarget),
      m_wsInsert(wsInsert) {}

CFWL_EventValidate::~CFWL_EventValidate() = default;
