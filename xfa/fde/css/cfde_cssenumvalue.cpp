// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_cssenumvalue.h"

CFDE_CSSEnumValue::CFDE_CSSEnumValue(FDE_CSSPropertyValue value)
    : CFDE_CSSValue(FDE_CSSPrimitiveType::Enum), value_(value) {}

CFDE_CSSEnumValue::~CFDE_CSSEnumValue() {}
