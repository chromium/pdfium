// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_fontmanager.h"

#include <utility>

#include "core/fxcrt/fx_codepage.h"
#include "xfa/fgas/font/cfgas_gefont.h"

namespace {

CFWL_FontManager* g_FontManager = nullptr;

}  // namespace

// static
CFWL_FontManager* CFWL_FontManager::GetInstance() {
  if (!g_FontManager)
    g_FontManager = new CFWL_FontManager;
  return g_FontManager;
}

// static
void CFWL_FontManager::DestroyInstance() {
  delete g_FontManager;
  g_FontManager = nullptr;
}

CFWL_FontManager::CFWL_FontManager() = default;

CFWL_FontManager::~CFWL_FontManager() = default;

RetainPtr<CFGAS_GEFont> CFWL_FontManager::GetFWLFont() {
  if (!m_pFWLFont) {
    m_pFWLFont = CFGAS_GEFont::LoadFont(L"Helvetica", 0, FX_CodePage::kDefANSI);
  }
  return m_pFWLFont;
}
