// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_FONTGLOBALS_H_
#define CORE_FPDFAPI_FONT_CPDF_FONTGLOBALS_H_

#include <map>
#include <memory>

#include "core/fpdfapi/cmaps/cmap_int.h"
#include "core/fpdfapi/font/cpdf_cmapmanager.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_fontmapper.h"
#include "third_party/base/span.h"

class CFX_StockFontArray;

class CPDF_FontGlobals {
 public:
  // Per-process singleton which must be managed by callers.
  static void Create();
  static void Destroy();
  static CPDF_FontGlobals* GetInstance();

  void Clear(CPDF_Document* pDoc);
  RetainPtr<CPDF_Font> Find(CPDF_Document* pDoc,
                            CFX_FontMapper::StandardFont index);
  void Set(CPDF_Document* pDoc,
           CFX_FontMapper::StandardFont index,
           const RetainPtr<CPDF_Font>& pFont);

  void SetEmbeddedCharset(size_t idx, pdfium::span<const FXCMAP_CMap> map) {
    m_EmbeddedCharsets[idx] = map;
  }
  pdfium::span<const FXCMAP_CMap> GetEmbeddedCharset(size_t idx) const {
    return m_EmbeddedCharsets[idx];
  }
  void SetEmbeddedToUnicode(size_t idx, pdfium::span<const uint16_t> map) {
    m_EmbeddedToUnicodes[idx] = map;
  }
  pdfium::span<const uint16_t> GetEmbeddedToUnicode(size_t idx) {
    return m_EmbeddedToUnicodes[idx];
  }

  CPDF_CMapManager* GetCMapManager() { return &m_CMapManager; }

 private:
  CPDF_FontGlobals();
  ~CPDF_FontGlobals();

  CPDF_CMapManager m_CMapManager;
  pdfium::span<const FXCMAP_CMap> m_EmbeddedCharsets[CIDSET_NUM_SETS];
  pdfium::span<const uint16_t> m_EmbeddedToUnicodes[CIDSET_NUM_SETS];
  std::map<CPDF_Document*, std::unique_ptr<CFX_StockFontArray>> m_StockMap;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_FONTGLOBALS_H_
