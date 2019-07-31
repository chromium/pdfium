// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/cpdf_modulemgr.h"

#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fxcodec/fx_codec.h"
#include "third_party/base/ptr_util.h"

namespace {

CPDF_ModuleMgr* g_pDefaultMgr = nullptr;

}  // namespace

// static
void CPDF_ModuleMgr::Create() {
  ASSERT(!g_pDefaultMgr);
  g_pDefaultMgr = new CPDF_ModuleMgr;
  fxcodec::ModuleMgr::Create();
  CPDF_PageModule::Create();
  CPDF_FontGlobals::GetInstance()->LoadEmbeddedMaps();
}

// static
void CPDF_ModuleMgr::Destroy() {
  ASSERT(g_pDefaultMgr);
  CPDF_PageModule::Destroy();
  fxcodec::ModuleMgr::Destroy();
  delete g_pDefaultMgr;
  g_pDefaultMgr = nullptr;
}

// static
CPDF_ModuleMgr* CPDF_ModuleMgr::Get() {
  ASSERT(g_pDefaultMgr);
  return g_pDefaultMgr;
}

CPDF_ModuleMgr::CPDF_ModuleMgr() = default;

CPDF_ModuleMgr::~CPDF_ModuleMgr() = default;
