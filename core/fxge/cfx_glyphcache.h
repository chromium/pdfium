// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_GLYPHCACHE_H_
#define CORE_FXGE_CFX_GLYPHCACHE_H_

#include <map>
#include <memory>
#include <tuple>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_face.h"

#ifdef _SKIA_SUPPORT_
#include "core/fxge/fx_font.h"
#include "third_party/skia/include/core/SkTypeface.h"  // nogncheck
#endif

class CFX_Font;
class CFX_GlyphBitmap;
class CFX_Matrix;
class CFX_Path;
struct CFX_TextRenderOptions;

class CFX_GlyphCache final : public Retainable, public Observable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;
  ~CFX_GlyphCache() override;

  const CFX_GlyphBitmap* LoadGlyphBitmap(const CFX_Font* pFont,
                                         uint32_t glyph_index,
                                         bool bFontStyle,
                                         const CFX_Matrix& matrix,
                                         int dest_width,
                                         int anti_alias,
                                         CFX_TextRenderOptions* text_options);
  const CFX_Path* LoadGlyphPath(const CFX_Font* pFont,
                                uint32_t glyph_index,
                                int dest_width);

  RetainPtr<CFX_Face> GetFace() { return m_Face; }
  FXFT_FaceRec* GetFaceRec() { return m_Face ? m_Face->GetRec() : nullptr; }

#ifdef _SKIA_SUPPORT_
  CFX_TypeFace* GetDeviceCache(const CFX_Font* pFont);
#endif

 private:
  explicit CFX_GlyphCache(RetainPtr<CFX_Face> face);

  using SizeGlyphCache = std::map<uint32_t, std::unique_ptr<CFX_GlyphBitmap>>;
  // <glyph_index, width, weight, angle, vertical>
  using PathMapKey = std::tuple<uint32_t, int, int, int, bool>;

  std::unique_ptr<CFX_GlyphBitmap> RenderGlyph(const CFX_Font* pFont,
                                               uint32_t glyph_index,
                                               bool bFontStyle,
                                               const CFX_Matrix& matrix,
                                               int dest_width,
                                               int anti_alias);
  std::unique_ptr<CFX_GlyphBitmap> RenderGlyph_Nativetext(
      const CFX_Font* pFont,
      uint32_t glyph_index,
      const CFX_Matrix& matrix,
      int dest_width,
      int anti_alias);
  CFX_GlyphBitmap* LookUpGlyphBitmap(const CFX_Font* pFont,
                                     const CFX_Matrix& matrix,
                                     const ByteString& FaceGlyphsKey,
                                     uint32_t glyph_index,
                                     bool bFontStyle,
                                     int dest_width,
                                     int anti_alias);
  void InitPlatform();
  void DestroyPlatform();

  RetainPtr<CFX_Face> const m_Face;
  std::map<ByteString, SizeGlyphCache> m_SizeMap;
  std::map<PathMapKey, std::unique_ptr<CFX_Path>> m_PathMap;
#ifdef _SKIA_SUPPORT_
  sk_sp<SkTypeface> m_pTypeface;
#endif
};

#endif  //  CORE_FXGE_CFX_GLYPHCACHE_H_
