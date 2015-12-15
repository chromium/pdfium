// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxge/fx_ge.h"

#include "text_int.h"

static CFX_GEModule* g_pGEModule = NULL;
CFX_GEModule::CFX_GEModule(const char** pUserFontPaths) {
  m_pFontCache = NULL;
  m_pFontMgr = NULL;
  m_FTLibrary = NULL;
  m_pCodecModule = NULL;
  m_pPlatformData = NULL;
  m_pUserFontPaths = pUserFontPaths;
}
CFX_GEModule::~CFX_GEModule() {
  delete m_pFontCache;
  m_pFontCache = NULL;
  delete m_pFontMgr;
  m_pFontMgr = NULL;
  DestroyPlatform();
}
CFX_GEModule* CFX_GEModule::Get() {
  return g_pGEModule;
}
void CFX_GEModule::Create(const char** userFontPaths) {
  g_pGEModule = new CFX_GEModule(userFontPaths);
  g_pGEModule->m_pFontMgr = new CFX_FontMgr;
  g_pGEModule->InitPlatform();
  g_pGEModule->SetTextGamma(2.2f);
}
void CFX_GEModule::Use(CFX_GEModule* pModule) {
  g_pGEModule = pModule;
}
void CFX_GEModule::Destroy() {
  delete g_pGEModule;
  g_pGEModule = NULL;
}
CFX_FontCache* CFX_GEModule::GetFontCache() {
  if (!m_pFontCache) {
    m_pFontCache = new CFX_FontCache();
  }
  return m_pFontCache;
}
void CFX_GEModule::SetTextGamma(FX_FLOAT gammaValue) {
  gammaValue /= 2.2f;
  int i = 0;
  while (i < 256) {
    m_GammaValue[i] =
        (uint8_t)(FXSYS_pow((FX_FLOAT)i / 255, gammaValue) * 255.0f + 0.5f);
    i++;
  }
}
const uint8_t* CFX_GEModule::GetTextGammaTable() {
  return m_GammaValue;
}
