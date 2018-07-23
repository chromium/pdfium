// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/fxjse.h"

CFXJSE_HostObject::CFXJSE_HostObject() = default;

CFXJSE_HostObject::~CFXJSE_HostObject() = default;

CFXJSE_FormCalcContext* CFXJSE_HostObject::AsFormCalcContext() {
  return nullptr;
}

CXFA_Object* CFXJSE_HostObject::AsCXFAObject() {
  return nullptr;
}
