// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_FONTCACHE_H_
#define CORE_FXGE_CFX_FONTCACHE_H_

#include <map>
#include <memory>

#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_glyphcache.h"
#include "core/fxge/fx_freetype.h"

class CFX_Font;

class CFX_FontCache {
 public:
  CFX_FontCache();
  ~CFX_FontCache();

  RetainPtr<CFX_GlyphCache> GetGlyphCache(const CFX_Font* pFont);
#ifdef _SKIA_SUPPORT_
  CFX_TypeFace* GetDeviceCache(const CFX_Font* pFont);
#endif

 private:
  std::map<CFX_Face*, ObservedPtr<CFX_GlyphCache>> m_GlyphCacheMap;
  std::map<CFX_Face*, ObservedPtr<CFX_GlyphCache>> m_ExtGlyphCacheMap;
};

#endif  // CORE_FXGE_CFX_FONTCACHE_H_
