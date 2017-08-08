// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/css/cfx_csscustomproperty.h"

CFX_CSSCustomProperty::CFX_CSSCustomProperty(const CFX_WideString& name,
                                             const CFX_WideString& value)
    : name_(name), value_(value) {}

CFX_CSSCustomProperty::CFX_CSSCustomProperty(const CFX_CSSCustomProperty& prop)
    : name_(prop.name_), value_(prop.value_) {}

CFX_CSSCustomProperty::~CFX_CSSCustomProperty() {}
