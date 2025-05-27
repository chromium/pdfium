// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_SIMPLEFONT_H_
#define CORE_FPDFAPI_FONT_CPDF_SIMPLEFONT_H_

#include <stdint.h>

#include <array>
#include <vector>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/font/cpdf_fontencoding.h"
#include "core/fxcrt/fx_string.h"

class CPDF_SimpleFont : public CPDF_Font {
 public:
  ~CPDF_SimpleFont() override;

  // CPDF_Font
  int GetCharWidthF(uint32_t charcode) override;
  FX_RECT GetCharBBox(uint32_t charcode) override;
  int GlyphFromCharCode(uint32_t charcode, bool* pVertGlyph) override;
  bool IsUnicodeCompatible() const override;
  WideString UnicodeFromCharCode(uint32_t charcode) const override;
  uint32_t CharCodeFromUnicode(wchar_t Unicode) const override;

  const CPDF_FontEncoding* GetEncoding() const { return &encoding_; }

  bool HasFontWidths() const override;

  static constexpr char kNotDef[] = ".notdef";
  static constexpr char kSpace[] = "space";

 protected:
  static constexpr size_t kInternalTableSize = 256;

  CPDF_SimpleFont(CPDF_Document* document,
                  RetainPtr<CPDF_Dictionary> font_dict);

  virtual void LoadGlyphMap() = 0;

  bool LoadCommon();
  void LoadSubstFont();
  void LoadCharMetrics(int charcode);
  void LoadCharWidths(const CPDF_Dictionary* font_desc);
  void LoadDifferences(const CPDF_Dictionary* encoding);
  void LoadPDFEncoding(bool bEmbedded, bool bTrueType);

  CPDF_FontEncoding encoding_{FontEncoding::kBuiltin};
  FontEncoding base_encoding_ = FontEncoding::kBuiltin;
  bool use_font_width_ = false;
  std::vector<ByteString> char_names_;
  std::array<uint16_t, kInternalTableSize> glyph_index_;
  std::array<uint16_t, kInternalTableSize> char_width_;
  std::array<FX_RECT, kInternalTableSize> char_bbox_;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_SIMPLEFONT_H_
