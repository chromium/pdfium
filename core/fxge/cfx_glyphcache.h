// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_GLYPHCACHE_H_
#define CORE_FXGE_CFX_GLYPHCACHE_H_

#include <map>
#include <memory>
#include <tuple>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_face.h"

#if defined _SKIA_SUPPORT_ || _SKIA_SUPPORT_PATHS_
#include "core/fxge/fx_font.h"
#include "third_party/skia/include/core/SkTypeface.h"
#endif

class CFX_Font;
class CFX_GlyphBitmap;
class CFX_Matrix;
class CFX_PathData;

class CFX_GlyphCache : public Retainable, public Observable {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  ~CFX_GlyphCache() override;

  const CFX_GlyphBitmap* LoadGlyphBitmap(const CFX_Font* pFont,
                                         uint32_t glyph_index,
                                         bool bFontStyle,
                                         const CFX_Matrix& matrix,
                                         uint32_t dest_width,
                                         int anti_alias,
                                         int* pTextFlags);
  const CFX_PathData* LoadGlyphPath(const CFX_Font* pFont,
                                    uint32_t glyph_index,
                                    uint32_t dest_width);

  RetainPtr<CFX_Face> GetFace() { return m_Face; }
  FXFT_FaceRec* GetFaceRec() { return m_Face ? m_Face->GetRec() : nullptr; }

#if defined _SKIA_SUPPORT_ || _SKIA_SUPPORT_PATHS_
  CFX_TypeFace* GetDeviceCache(const CFX_Font* pFont);
#endif

 private:
  explicit CFX_GlyphCache(RetainPtr<CFX_Face> face);

  using SizeGlyphCache = std::map<uint32_t, std::unique_ptr<CFX_GlyphBitmap>>;
  // <glyph_index, width, weight, angle, vertical>
  using PathMapKey = std::tuple<uint32_t, uint32_t, int, int, bool>;

  std::unique_ptr<CFX_GlyphBitmap> RenderGlyph(const CFX_Font* pFont,
                                               uint32_t glyph_index,
                                               bool bFontStyle,
                                               const CFX_Matrix& matrix,
                                               uint32_t dest_width,
                                               int anti_alias);
  std::unique_ptr<CFX_GlyphBitmap> RenderGlyph_Nativetext(
      const CFX_Font* pFont,
      uint32_t glyph_index,
      const CFX_Matrix& matrix,
      uint32_t dest_width,
      int anti_alias);
  CFX_GlyphBitmap* LookUpGlyphBitmap(const CFX_Font* pFont,
                                     const CFX_Matrix& matrix,
                                     const ByteString& FaceGlyphsKey,
                                     uint32_t glyph_index,
                                     bool bFontStyle,
                                     uint32_t dest_width,
                                     int anti_alias);
  void InitPlatform();
  void DestroyPlatform();

  RetainPtr<CFX_Face> const m_Face;
  std::map<ByteString, SizeGlyphCache> m_SizeMap;
  std::map<PathMapKey, std::unique_ptr<CFX_PathData>> m_PathMap;
#if defined _SKIA_SUPPORT_ || _SKIA_SUPPORT_PATHS_
  sk_sp<SkTypeface> m_pTypeface;
#endif
};

#endif  //  CORE_FXGE_CFX_GLYPHCACHE_H_
