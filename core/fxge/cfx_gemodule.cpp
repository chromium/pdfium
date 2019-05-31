// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_gemodule.h"

#include "core/fxge/cfx_folderfontinfo.h"
#include "core/fxge/cfx_fontcache.h"
#include "core/fxge/cfx_fontmgr.h"
#include "third_party/base/ptr_util.h"

namespace {

CFX_GEModule* g_pGEModule = nullptr;

}  // namespace

CFX_GEModule::CFX_GEModule(const char** pUserFontPaths)
    : m_pPlatform(PlatformIface::Create()),
      m_pFontMgr(pdfium::MakeUnique<CFX_FontMgr>()),
      m_pFontCache(pdfium::MakeUnique<CFX_FontCache>()),
      m_pUserFontPaths(pUserFontPaths) {}

CFX_GEModule::~CFX_GEModule() = default;

// static
void CFX_GEModule::Create(const char** pUserFontPaths) {
  ASSERT(!g_pGEModule);
  g_pGEModule = new CFX_GEModule(pUserFontPaths);
  g_pGEModule->m_pPlatform->Init();
}

// static
void CFX_GEModule::Destroy() {
  ASSERT(g_pGEModule);
  delete g_pGEModule;
  g_pGEModule = nullptr;
}

// static
CFX_GEModule* CFX_GEModule::Get() {
  ASSERT(g_pGEModule);
  return g_pGEModule;
}
