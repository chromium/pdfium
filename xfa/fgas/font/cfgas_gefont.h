// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_FONT_CFGAS_GEFONT_H_
#define XFA_FGAS_FONT_CFGAS_GEFONT_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/fx_codepage_forward.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/maybe_owned.h"
#include "core/fxcrt/retain_ptr.h"

class CFX_Font;
class CFX_UnicodeEncodingEx;
class CPDF_Document;
class CPDF_Font;

class CFGAS_GEFont final : public Retainable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  static RetainPtr<CFGAS_GEFont> LoadFont(const wchar_t* pszFontFamily,
                                          uint32_t dwFontStyles,
                                          FX_CodePage wCodePage);
  static RetainPtr<CFGAS_GEFont> LoadFont(RetainPtr<CPDF_Font> font);
  static RetainPtr<CFGAS_GEFont> LoadFont(std::unique_ptr<CFX_Font> font);
  static RetainPtr<CFGAS_GEFont> LoadStockFont(CPDF_Document* doc,
                                               const ByteString& font_family);

  uint32_t GetFontStyles() const;
  std::optional<uint16_t> GetCharWidth(wchar_t wUnicode);
  int32_t GetGlyphIndex(wchar_t wUnicode);
  int32_t GetAscent() const;
  int32_t GetDescent() const;
  std::optional<FX_RECT> GetCharBBox(wchar_t wUnicode);

  RetainPtr<CFGAS_GEFont> GetSubstFont(int32_t iGlyphIndex);
  CFX_Font* GetDevFont() const { return font_.Get(); }

  void SetLogicalFontStyle(uint32_t dwLogFontStyle) {
    log_font_style_ = dwLogFontStyle;
  }

 private:
  CFGAS_GEFont();
  ~CFGAS_GEFont() override;

#if BUILDFLAG(IS_WIN)
  bool LoadFontInternal(const wchar_t* pszFontFamily,
                        uint32_t dwFontStyles,
                        FX_CodePage wCodePage);
#endif
  bool LoadFontInternal(std::unique_ptr<CFX_Font> pInternalFont);
  bool LoadFontInternal(RetainPtr<CPDF_Font> pPDFFont);
  bool InitFont();
  std::pair<int32_t, RetainPtr<CFGAS_GEFont>> GetGlyphIndexAndFont(
      wchar_t wUnicode,
      bool bRecursive);
  WideString GetFamilyName() const;

  std::optional<uint32_t> log_font_style_;
  RetainPtr<CPDF_Font> pdffont_;  // Must come before |font_|.
  MaybeOwned<CFX_Font> font_;     // Must come before |font_encoding_|.
  std::unique_ptr<CFX_UnicodeEncodingEx> font_encoding_;
  std::map<wchar_t, std::optional<uint16_t>> char_width_map_;
  std::map<wchar_t, FX_RECT> bbox_map_;
  std::vector<RetainPtr<CFGAS_GEFont>> subst_fonts_;
  std::map<wchar_t, RetainPtr<CFGAS_GEFont>> font_mapper_;
};

#endif  // XFA_FGAS_FONT_CFGAS_GEFONT_H_
