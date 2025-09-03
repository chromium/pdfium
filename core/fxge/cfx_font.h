// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_FONT_H_
#define CORE_FXGE_CFX_FONT_H_

#include <stdint.h>

#include <memory>
#include <optional>

#include "build/build_config.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_codepage_forward.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr_exclusion.h"
#include "core/fxge/cfx_face.h"
#include "core/fxge/freetype/fx_freetype.h"

#if defined(PDF_USE_SKIA)
#include "core/fxge/fx_font.h"
#endif

class CFX_GlyphBitmap;
class CFX_GlyphCache;
class CFX_Path;
class CFX_SubstFont;
class IFX_SeekableReadStream;
struct CFX_TextRenderOptions;

class CFX_Font {
 public:
  // This struct should be the same as FPDF_CharsetFontMap.
  struct CharsetFontMap {
    int charset;           // Character Set Enum value, see FX_Charset::kXXX.
    const char* fontname;  // Name of default font to use with that charset.
  };

  enum class FontType {
    kUnknown,
    kCIDTrueType,
  };

  // Used when the font name is empty.
  static const char kUntitledFontName[];

  static const char kDefaultAnsiFontName[];
  static const char kUniversalDefaultFontName[];

  static pdfium::span<const CharsetFontMap> GetDefaultTTFMapSpan();
  static ByteString GetDefaultFontNameByCharset(FX_Charset nCharset);
  static FX_Charset GetCharSetFromUnicode(uint16_t word);

  CFX_Font();
  ~CFX_Font();

  void LoadSubst(const ByteString& face_name,
                 bool bTrueType,
                 uint32_t flags,
                 int weight,
                 int italic_angle,
                 FX_CodePage code_page,
                 bool bVertical);

  bool LoadEmbedded(pdfium::span<const uint8_t> src_span,
                    bool force_vertical,
                    uint64_t object_tag);
  RetainPtr<CFX_Face> GetFace() const { return face_; }
  bool HasFaceRec() const { return face_ && face_->GetRec(); }
  CFX_SubstFont* GetSubstFont() const { return subst_font_.get(); }
  int GetSubstFontItalicAngle() const;

#if defined(PDF_ENABLE_XFA)
  bool LoadFile(RetainPtr<IFX_SeekableReadStream> pFile, int nFaceIndex);

#if !BUILDFLAG(IS_WIN)
  void SetFace(RetainPtr<CFX_Face> face);
  void SetFontSpan(pdfium::span<uint8_t> pSpan) { font_data_ = pSpan; }
  void SetSubstFont(std::unique_ptr<CFX_SubstFont> subst);
#endif  // !BUILDFLAG(IS_WIN)
#endif  // defined(PDF_ENABLE_XFA)

  const CFX_GlyphBitmap* LoadGlyphBitmap(
      uint32_t glyph_index,
      bool bFontStyle,
      const CFX_Matrix& matrix,
      int dest_width,
      int anti_alias,
      CFX_TextRenderOptions* text_options) const;
  const CFX_Path* LoadGlyphPath(uint32_t glyph_index, int dest_width) const;
  int GetGlyphWidth(uint32_t glyph_index) const;
  int GetGlyphWidth(uint32_t glyph_index, int dest_width, int weight) const;
  int GetAscent() const;
  int GetDescent() const;
  std::optional<FX_RECT> GetGlyphBBox(uint32_t glyph_index);
  bool IsItalic() const;
  bool IsBold() const;
  bool IsFixedWidth() const;
  bool IsVertical() const { return vertical_; }
  ByteString GetPsName() const;
  ByteString GetFamilyName() const;
  ByteString GetBaseFontName() const;
  bool IsTTFont() const;

  // Raw bounding box.
  std::optional<FX_RECT> GetRawBBox() const;

  // Bounding box adjusted for font units.
  std::optional<FX_RECT> GetBBox() const;

  FontType GetFontType() const { return font_type_; }
  void SetFontType(FontType type) { font_type_ = type; }
  uint64_t GetObjectTag() const { return object_tag_; }
  pdfium::raw_span<uint8_t> GetFontSpan() const { return font_data_; }
  std::unique_ptr<CFX_Path> LoadGlyphPathImpl(uint32_t glyph_index,
                                              int dest_width) const;
  int GetGlyphWidthImpl(uint32_t glyph_index, int dest_width, int weight) const;

#if defined(PDF_USE_SKIA)
  CFX_TypeFace* GetDeviceCache() const;
  bool IsSubstFontBold() const;
#endif

#if BUILDFLAG(IS_APPLE)
  void* GetPlatformFont() const { return platform_font_; }
  void SetPlatformFont(void* font) { platform_font_ = font; }
#endif

 private:
  RetainPtr<CFX_GlyphCache> GetOrCreateGlyphCache() const;
  void ClearGlyphCache();
#if BUILDFLAG(IS_APPLE)
  void ReleasePlatformResource();
#endif
  ByteString GetFamilyNameOrUntitled() const;

#if defined(PDF_ENABLE_XFA)
  // |owned_file_| must outlive |owned_stream_rec_|.
  RetainPtr<IFX_SeekableReadStream> owned_file_;
  std::unique_ptr<FXFT_StreamRec> owned_stream_rec_;  // Must outlive |face_|.
#endif

  mutable RetainPtr<CFX_Face> face_;
  mutable RetainPtr<CFX_GlyphCache> glyph_cache_;
  std::unique_ptr<CFX_SubstFont> subst_font_;
  DataVector<uint8_t> font_data_allocation_;
  pdfium::raw_span<uint8_t> font_data_;
  FontType font_type_ = FontType::kUnknown;
  uint64_t object_tag_ = 0;
  bool vertical_ = false;
#if BUILDFLAG(IS_APPLE)
  UNOWNED_PTR_EXCLUSION void* platform_font_ = nullptr;
#endif
};

#endif  // CORE_FXGE_CFX_FONT_H_
