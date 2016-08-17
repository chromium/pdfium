// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/include/cfx_autofontcache.h"

#include "core/fxge/include/cfx_fontcache.h"
#include "core/fxge/include/fx_font.h"

CFX_AutoFontCache::CFX_AutoFontCache(CFX_FontCache* pFontCache, CFX_Font* pFont)
    : m_pFontCache(pFontCache), m_pFont(pFont) {}

CFX_AutoFontCache::~CFX_AutoFontCache() {
  m_pFontCache->ReleaseCachedFace(m_pFont);
}
