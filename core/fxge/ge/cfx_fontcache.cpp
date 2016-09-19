// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/include/cfx_fontcache.h"

#include "core/fxge/include/cfx_facecache.h"
#include "core/fxge/include/fx_font.h"
#include "core/fxge/include/fx_freetype.h"

CFX_FontCache::CFX_FontCache() {}

CFX_FontCache::~CFX_FontCache() {
  FreeCache(TRUE);
}

CFX_FaceCache* CFX_FontCache::GetCachedFace(CFX_Font* pFont) {
  FXFT_Face face = pFont->GetFace();
  const bool bExternal = !face;
  CFX_FTCacheMap& map = bExternal ? m_ExtFaceMap : m_FTFaceMap;
  auto it = map.find(face);
  if (it != map.end()) {
    CFX_CountedFaceCache* counted_face_cache = it->second;
    counted_face_cache->m_nCount++;
    return counted_face_cache->m_Obj;
  }

  CFX_FaceCache* face_cache = new CFX_FaceCache(bExternal ? nullptr : face);
  CFX_CountedFaceCache* counted_face_cache = new CFX_CountedFaceCache;
  counted_face_cache->m_nCount = 2;
  counted_face_cache->m_Obj = face_cache;
  map[face] = counted_face_cache;
  return face_cache;
}

#ifdef _SKIA_SUPPORT_
CFX_TypeFace* CFX_FontCache::GetDeviceCache(CFX_Font* pFont) {
  return GetCachedFace(pFont)->GetDeviceCache(pFont);
}
#endif

void CFX_FontCache::ReleaseCachedFace(CFX_Font* pFont) {
  FXFT_Face face = pFont->GetFace();
  const bool bExternal = !face;
  CFX_FTCacheMap& map = bExternal ? m_ExtFaceMap : m_FTFaceMap;

  auto it = map.find(face);
  if (it == map.end())
    return;

  CFX_CountedFaceCache* counted_face_cache = it->second;
  if (counted_face_cache->m_nCount > 1) {
    counted_face_cache->m_nCount--;
  }
}

void CFX_FontCache::FreeCache(FX_BOOL bRelease) {
  for (auto it = m_FTFaceMap.begin(); it != m_FTFaceMap.end();) {
    auto curr_it = it++;
    CFX_CountedFaceCache* cache = curr_it->second;
    if (bRelease || cache->m_nCount < 2) {
      delete cache->m_Obj;
      delete cache;
      m_FTFaceMap.erase(curr_it);
    }
  }

  for (auto it = m_ExtFaceMap.begin(); it != m_ExtFaceMap.end();) {
    auto curr_it = it++;
    CFX_CountedFaceCache* cache = curr_it->second;
    if (bRelease || cache->m_nCount < 2) {
      delete cache->m_Obj;
      delete cache;
      m_ExtFaceMap.erase(curr_it);
    }
  }
}
