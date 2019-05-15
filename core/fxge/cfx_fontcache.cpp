// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_fontcache.h"

#include <memory>
#include <utility>

#include "core/fxge/cfx_facecache.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/fx_freetype.h"
#include "third_party/base/ptr_util.h"

CFX_FontCache::CFX_FontCache() = default;

CFX_FontCache::~CFX_FontCache() = default;

RetainPtr<CFX_FaceCache> CFX_FontCache::GetFaceCache(const CFX_Font* pFont) {
  FXFT_Face face = pFont->GetFace();
  const bool bExternal = !face;
  auto& map = bExternal ? m_ExtFaceMap : m_FTFaceMap;
  auto it = map.find(face);
  if (it != map.end() && it->second)
    return pdfium::WrapRetain(it->second.Get());

  auto new_cache = pdfium::MakeRetain<CFX_FaceCache>(face);
  map[face].Reset(new_cache.Get());
  return new_cache;
}

#ifdef _SKIA_SUPPORT_
CFX_TypeFace* CFX_FontCache::GetDeviceCache(const CFX_Font* pFont) {
  return GetFaceCache(pFont)->GetDeviceCache(pFont);
}
#endif
