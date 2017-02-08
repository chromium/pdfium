// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fde/css/cfde_csscustomproperty.h"

CFDE_CSSCustomProperty::CFDE_CSSCustomProperty(const CFX_WideString& name,
                                               const CFX_WideString& value)
    : name_(name), value_(value) {}

CFDE_CSSCustomProperty::CFDE_CSSCustomProperty(
    const CFDE_CSSCustomProperty& prop)
    : name_(prop.name_), value_(prop.value_) {}

CFDE_CSSCustomProperty::~CFDE_CSSCustomProperty() {}
