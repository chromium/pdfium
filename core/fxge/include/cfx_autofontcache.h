// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_INCLUDE_CFX_AUTOFONTCACHE_H_
#define CORE_FXGE_INCLUDE_CFX_AUTOFONTCACHE_H_

#include "core/fxge/include/cfx_fontcache.h"
#include "core/fxge/include/fx_font.h"

class CFX_AutoFontCache {
 public:
  CFX_AutoFontCache(CFX_FontCache* pFontCache, CFX_Font* pFont);
  ~CFX_AutoFontCache();

 private:
  CFX_FontCache* m_pFontCache;
  CFX_Font* m_pFont;
};

#endif  // CORE_FXGE_INCLUDE_CFX_AUTOFONTCACHE_H_
