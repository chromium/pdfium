// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_basic_data.h"

#include "core/fxcrt/fx_memory.h"
#include "xfa/fxfa/fxfa_basic.h"

const XFA_AttributeValueInfo g_XFAEnumData[] = {
#undef VALUE____
#define VALUE____(a, b, c) {a, XFA_AttributeValue::c, b},
#include "xfa/fxfa/parser/attribute_values.inc"
#undef VALUE____
};

const size_t g_szXFAEnumCount = FX_ArraySize(g_XFAEnumData);
