// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pagemodule.h"

#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "third_party/base/check.h"

namespace {

CPDF_PageModule* g_PageModule = nullptr;

}  // namespace

// static
void CPDF_PageModule::Create() {
  DCHECK(!g_PageModule);
  g_PageModule = new CPDF_PageModule();
}

// static
void CPDF_PageModule::Destroy() {
  DCHECK(g_PageModule);
  delete g_PageModule;
  g_PageModule = nullptr;
}

// static
CPDF_PageModule* CPDF_PageModule::GetInstance() {
  DCHECK(g_PageModule);
  return g_PageModule;
}

CPDF_PageModule::CPDF_PageModule() {
  CPDF_ColorSpace::InitializeGlobals();
  CPDF_FontGlobals::Create();
  CPDF_FontGlobals::GetInstance()->LoadEmbeddedMaps();
}

CPDF_PageModule::~CPDF_PageModule() {
  CPDF_FontGlobals::Destroy();
  CPDF_ColorSpace::DestroyGlobals();
}

void CPDF_PageModule::ClearStockFont(CPDF_Document* pDoc) {
  CPDF_FontGlobals::GetInstance()->Clear(pDoc);
}
