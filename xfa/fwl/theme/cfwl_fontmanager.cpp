// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_fontmanager.h"

#include <utility>

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

RetainPtr<CFGAS_GEFont> CFWL_FontManager::FindFont(WideStringView wsFontFamily,
                                                   uint32_t dwFontStyles,
                                                   FX_CodePage wCodePage) {
  for (const auto& pData : m_FontsArray) {
    if (pData->Equal(wsFontFamily, dwFontStyles, wCodePage))
      return pData->GetFont();
  }
  auto pFontData = std::make_unique<FontData>();
  if (!pFontData->LoadFont(wsFontFamily, dwFontStyles, wCodePage))
    return nullptr;

  m_FontsArray.push_back(std::move(pFontData));
  return m_FontsArray.back()->GetFont();
}

CFWL_FontManager::FontData::FontData() = default;

CFWL_FontManager::FontData::~FontData() = default;

bool CFWL_FontManager::FontData::Equal(WideStringView wsFontFamily,
                                       uint32_t dwFontStyles,
                                       FX_CodePage wCodePage) {
  return m_wsFamily == wsFontFamily && m_dwStyles == dwFontStyles &&
         m_dwCodePage == wCodePage;
}

bool CFWL_FontManager::FontData::LoadFont(WideStringView wsFontFamily,
                                          uint32_t dwFontStyles,
                                          FX_CodePage dwCodePage) {
  m_wsFamily = wsFontFamily;
  m_dwStyles = dwFontStyles;
  m_dwCodePage = dwCodePage;

  // TODO(tsepez): check usage of c_str() below.
  m_pFont = CFGAS_GEFont::LoadFont(wsFontFamily.unterminated_c_str(),
                                   dwFontStyles, dwCodePage);
  return !!m_pFont;
}

RetainPtr<CFGAS_GEFont> CFWL_FontManager::FontData::GetFont() const {
  return m_pFont;
}
