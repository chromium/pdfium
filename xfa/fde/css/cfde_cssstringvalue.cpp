// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssstringvalue.h"

CFDE_CSSStringValue::CFDE_CSSStringValue(const CFX_WideString& value)
    : CFDE_CSSValue(FDE_CSSPrimitiveType::String), value_(value) {}

CFDE_CSSStringValue::~CFDE_CSSStringValue() {}
