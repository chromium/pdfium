// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_FONT_CFGAS_GEFONT_H_
#define XFA_FGAS_FONT_CFGAS_GEFONT_H_

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/fx_codepage_forward.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/maybe_owned.h"
#include "core/fxcrt/retain_ptr.h"
#include "third_party/base/optional.h"

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
  static RetainPtr<CFGAS_GEFont> LoadFont(const RetainPtr<CPDF_Font>& pFont);
  static RetainPtr<CFGAS_GEFont> LoadFont(std::unique_ptr<CFX_Font> pFont);
  static RetainPtr<CFGAS_GEFont> LoadStockFont(CPDF_Document* pDoc,
                                               const ByteString& font_family);

  uint32_t GetFontStyles() const;
  Optional<uint16_t> GetCharWidth(wchar_t wUnicode);
  int32_t GetGlyphIndex(wchar_t wUnicode);
  int32_t GetAscent() const;
  int32_t GetDescent() const;
  Optional<FX_RECT> GetCharBBox(wchar_t wUnicode);
  Optional<FX_RECT> GetBBox();

  RetainPtr<CFGAS_GEFont> GetSubstFont(int32_t iGlyphIndex);
  CFX_Font* GetDevFont() const { return m_pFont.Get(); }

  void SetLogicalFontStyle(uint32_t dwLogFontStyle) {
    m_dwLogFontStyle = dwLogFontStyle;
  }

 private:
  CFGAS_GEFont();
  ~CFGAS_GEFont() override;

#if defined(OS_WIN)
  bool LoadFontInternal(const wchar_t* pszFontFamily,
                        uint32_t dwFontStyles,
                        FX_CodePage wCodePage);
  bool LoadFontInternal(const uint8_t* pBuffer, int32_t length);
#endif
  bool LoadFontInternal(std::unique_ptr<CFX_Font> pInternalFont);
  bool LoadFontInternal(const RetainPtr<CPDF_Font>& pPDFFont);
  bool InitFont();
  std::pair<int32_t, RetainPtr<CFGAS_GEFont>> GetGlyphIndexAndFont(
      wchar_t wUnicode,
      bool bRecursive);
  WideString GetFamilyName() const;

  Optional<uint32_t> m_dwLogFontStyle;
  RetainPtr<CPDF_Font> m_pPDFFont;  // Must come before |m_pFont|.
  MaybeOwned<CFX_Font> m_pFont;     // Must come before |m_pFontEncoding|.
  std::unique_ptr<CFX_UnicodeEncodingEx> m_pFontEncoding;
  std::map<wchar_t, Optional<uint16_t>> m_CharWidthMap;
  std::map<wchar_t, FX_RECT> m_BBoxMap;
  std::vector<RetainPtr<CFGAS_GEFont>> m_SubstFonts;
  std::map<wchar_t, RetainPtr<CFGAS_GEFont>> m_FontMapper;
};

#endif  // XFA_FGAS_FONT_CFGAS_GEFONT_H_
