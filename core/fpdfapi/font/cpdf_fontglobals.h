// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_FONTGLOBALS_H_
#define CORE_FPDFAPI_FONT_CPDF_FONTGLOBALS_H_

#include <array>
#include <functional>
#include <map>
#include <memory>

#include "core/fpdfapi/cmaps/fpdf_cmaps.h"
#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fxcrt/raw_span.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxge/cfx_fontmapper.h"

class CFX_StockFontArray;
class CPDF_Font;

class CPDF_FontGlobals {
 public:
  // Per-process singleton which must be managed by callers.
  static void Create();
  static void Destroy();
  static CPDF_FontGlobals* GetInstance();

  // Caller must load the maps before using font globals.
  void LoadEmbeddedMaps();

  void Clear(CPDF_Document* doc);
  RetainPtr<CPDF_Font> Find(CPDF_Document* doc,
                            CFX_FontMapper::StandardFont index);
  void Set(CPDF_Document* doc,
           CFX_FontMapper::StandardFont index,
           RetainPtr<CPDF_Font> font);

  void SetEmbeddedCharset(CIDSet idx, pdfium::span<const fxcmap::CMap> map) {
    embedded_charsets_[idx] = map;
  }
  pdfium::span<const fxcmap::CMap> GetEmbeddedCharset(CIDSet idx) const {
    return embedded_charsets_[idx];
  }
  void SetEmbeddedToUnicode(CIDSet idx, pdfium::span<const uint16_t> map) {
    embedded_to_unicodes_[idx] = map;
  }
  pdfium::span<const uint16_t> GetEmbeddedToUnicode(CIDSet idx) {
    return embedded_to_unicodes_[idx];
  }

  RetainPtr<const CPDF_CMap> GetPredefinedCMap(const ByteString& name);
  CPDF_CID2UnicodeMap* GetCID2UnicodeMap(CIDSet charset);

 private:
  CPDF_FontGlobals();
  ~CPDF_FontGlobals();

  void LoadEmbeddedGB1CMaps();
  void LoadEmbeddedCNS1CMaps();
  void LoadEmbeddedJapan1CMaps();
  void LoadEmbeddedKorea1CMaps();

  std::map<ByteString, RetainPtr<const CPDF_CMap>> cmaps_;
  std::array<std::unique_ptr<CPDF_CID2UnicodeMap>, CIDSET_NUM_SETS>
      cid2unicode_maps_;
  std::array<pdfium::raw_span<const fxcmap::CMap>, CIDSET_NUM_SETS>
      embedded_charsets_;
  std::array<pdfium::raw_span<const uint16_t>, CIDSET_NUM_SETS>
      embedded_to_unicodes_;
  std::map<UnownedPtr<CPDF_Document>,
           std::unique_ptr<CFX_StockFontArray>,
           std::less<>>
      stock_map_;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_FONTGLOBALS_H_
