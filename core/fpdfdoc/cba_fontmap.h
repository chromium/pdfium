// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CBA_FONTMAP_H_
#define CORE_FPDFDOC_CBA_FONTMAP_H_

#include <memory>
#include <vector>

#include "core/fpdfdoc/ipvt_fontmap.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Dictionary;
class CPDF_Document;

class CBA_FontMap final : public IPVT_FontMap {
 public:
  static int32_t GetNativeCharset();

  CBA_FontMap(CPDF_Document* pDocument, CPDF_Dictionary* pAnnotDict);
  ~CBA_FontMap() override;

  // IPVT_FontMap
  RetainPtr<CPDF_Font> GetPDFFont(int32_t nFontIndex) override;
  ByteString GetPDFFontAlias(int32_t nFontIndex) override;
  int32_t GetWordFontIndex(uint16_t word,
                           int32_t nCharset,
                           int32_t nFontIndex) override;
  int32_t CharCodeFromUnicode(int32_t nFontIndex, uint16_t word) override;
  int32_t CharSetFromUnicode(uint16_t word, int32_t nOldCharset) override;

  void Reset();
  void SetAPType(const ByteString& sAPType);

 private:
  struct Data {
    Data();
    ~Data();

    RetainPtr<CPDF_Font> pFont;
    int32_t nCharset;
    ByteString sFontName;
  };

  struct Native {
    int32_t nCharset;
    ByteString sFontName;
  };

  void Initialize();
  RetainPtr<CPDF_Font> FindFontSameCharset(ByteString* sFontAlias,
                                           int32_t nCharset);
  RetainPtr<CPDF_Font> FindResFontSameCharset(const CPDF_Dictionary* pResDict,
                                              ByteString* sFontAlias,
                                              int32_t nCharset);
  RetainPtr<CPDF_Font> GetAnnotDefaultFont(ByteString* sAlias);
  void AddFontToAnnotDict(const RetainPtr<CPDF_Font>& pFont,
                          const ByteString& sAlias);

  bool KnowWord(int32_t nFontIndex, uint16_t word);

  void Clear();
  int32_t GetFontIndex(const ByteString& sFontName,
                       int32_t nCharset,
                       bool bFind);
  int32_t AddFontData(const RetainPtr<CPDF_Font>& pFont,
                      const ByteString& sFontAlias,
                      int32_t nCharset);

  ByteString EncodeFontAlias(const ByteString& sFontName, int32_t nCharset);
  ByteString EncodeFontAlias(const ByteString& sFontName);

  int32_t FindFont(const ByteString& sFontName, int32_t nCharset);
  ByteString GetNativeFontName(int32_t nCharset);
  ByteString GetCachedNativeFontName(int32_t nCharset);
  bool IsStandardFont(const ByteString& sFontName);
  RetainPtr<CPDF_Font> AddFontToDocument(ByteString sFontName,
                                         uint8_t nCharset);
  RetainPtr<CPDF_Font> AddStandardFont(ByteString sFontName);
  RetainPtr<CPDF_Font> AddSystemFont(ByteString sFontName, uint8_t nCharset);

  std::vector<std::unique_ptr<Data>> m_Data;
  std::vector<std::unique_ptr<Native>> m_NativeFont;
  UnownedPtr<CPDF_Document> const m_pDocument;
  RetainPtr<CPDF_Dictionary> const m_pAnnotDict;
  RetainPtr<CPDF_Font> m_pDefaultFont;
  ByteString m_sDefaultFontName;
  ByteString m_sAPType = "N";
};

#endif  // CORE_FPDFDOC_CBA_FONTMAP_H_
