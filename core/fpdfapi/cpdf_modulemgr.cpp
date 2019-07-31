// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/cpdf_modulemgr.h"

#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fxcodec/fx_codec.h"

namespace {

bool g_bModuleMgrCreated = false;

}  // namespace

// static
void CPDF_ModuleMgr::Create() {
  ASSERT(!g_bModuleMgrCreated);
  fxcodec::ModuleMgr::Create();
  CPDF_PageModule::Create();
  CPDF_FontGlobals::GetInstance()->LoadEmbeddedMaps();
  g_bModuleMgrCreated = true;
}

// static
void CPDF_ModuleMgr::Destroy() {
  ASSERT(g_bModuleMgrCreated);
  CPDF_PageModule::Destroy();
  fxcodec::ModuleMgr::Destroy();
  g_bModuleMgrCreated = false;
}
