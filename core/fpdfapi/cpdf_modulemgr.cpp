// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/cpdf_modulemgr.h"

#include "core/fpdfapi/page/cpdf_pagemodule.h"

namespace {

bool g_bModuleMgrCreated = false;

}  // namespace

// static
void CPDF_ModuleMgr::Create() {
  ASSERT(!g_bModuleMgrCreated);
  CPDF_PageModule::Create();
  g_bModuleMgrCreated = true;
}

// static
void CPDF_ModuleMgr::Destroy() {
  ASSERT(g_bModuleMgrCreated);
  CPDF_PageModule::Destroy();
  g_bModuleMgrCreated = false;
}
