// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/test_support.h"

#include "core/fxge/cfx_gemodule.h"

void InitializePDFTestEnvironment() {
  CFX_GEModule::Create(nullptr);
}

void DestroyPDFTestEnvironment() {
  CFX_GEModule::Destroy();
}
