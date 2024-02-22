// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fgas/font/cfgas_gemodule.h"

#include "core/fxcrt/check.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"

namespace {

CFGAS_GEModule* g_module = nullptr;

}  // namespace

// static
void CFGAS_GEModule::Create() {
  DCHECK(!g_module);
  g_module = new CFGAS_GEModule();
  g_module->GetFontMgr()->EnumFonts();
}

// static
void CFGAS_GEModule::Destroy() {
  DCHECK(g_module);
  delete g_module;
  g_module = nullptr;
}

// static
CFGAS_GEModule* CFGAS_GEModule::Get() {
  DCHECK(g_module);
  return g_module;
}

CFGAS_GEModule::CFGAS_GEModule()
    : font_mgr_(std::make_unique<CFGAS_FontMgr>()) {}

CFGAS_GEModule::~CFGAS_GEModule() = default;
