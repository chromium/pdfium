// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_fontglobals.h"

#include "core/fpdfapi/cmaps/CNS1/cmaps_cns1.h"
#include "core/fpdfapi/cmaps/GB1/cmaps_gb1.h"
#include "core/fpdfapi/cmaps/Japan1/cmaps_japan1.h"
#include "core/fpdfapi/cmaps/Korea1/cmaps_korea1.h"
#include "core/fpdfapi/font/cfx_stockfontarray.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

CPDF_FontGlobals* g_FontGlobals = nullptr;

}  // namespace

// static
void CPDF_FontGlobals::Create() {
  ASSERT(!g_FontGlobals);
  g_FontGlobals = new CPDF_FontGlobals();
}

// static
void CPDF_FontGlobals::Destroy() {
  ASSERT(g_FontGlobals);
  delete g_FontGlobals;
  g_FontGlobals = nullptr;
}

// static
CPDF_FontGlobals* CPDF_FontGlobals::GetInstance() {
  ASSERT(g_FontGlobals);
  return g_FontGlobals;
}

CPDF_FontGlobals::CPDF_FontGlobals() {
  memset(m_EmbeddedCharsets, 0, sizeof(m_EmbeddedCharsets));
  memset(m_EmbeddedToUnicodes, 0, sizeof(m_EmbeddedToUnicodes));
}

CPDF_FontGlobals::~CPDF_FontGlobals() = default;

void CPDF_FontGlobals::LoadEmbeddedMaps() {
  LoadEmbeddedGB1CMaps();
  LoadEmbeddedCNS1CMaps();
  LoadEmbeddedJapan1CMaps();
  LoadEmbeddedKorea1CMaps();
}

RetainPtr<CPDF_Font> CPDF_FontGlobals::Find(
    CPDF_Document* pDoc,
    CFX_FontMapper::StandardFont index) {
  auto it = m_StockMap.find(pDoc);
  if (it == m_StockMap.end() || !it->second)
    return nullptr;

  return it->second->GetFont(index);
}

void CPDF_FontGlobals::Set(CPDF_Document* pDoc,
                           CFX_FontMapper::StandardFont index,
                           const RetainPtr<CPDF_Font>& pFont) {
  if (!pdfium::ContainsKey(m_StockMap, pDoc))
    m_StockMap[pDoc] = pdfium::MakeUnique<CFX_StockFontArray>();
  m_StockMap[pDoc]->SetFont(index, pFont);
}

void CPDF_FontGlobals::Clear(CPDF_Document* pDoc) {
  m_StockMap.erase(pDoc);
}

void CPDF_FontGlobals::LoadEmbeddedGB1CMaps() {
  SetEmbeddedCharset(CIDSET_GB1, pdfium::make_span(g_FXCMAP_GB1_cmaps,
                                                   g_FXCMAP_GB1_cmaps_size));
  SetEmbeddedToUnicode(CIDSET_GB1, g_FXCMAP_GB1CID2Unicode_5);
}

void CPDF_FontGlobals::LoadEmbeddedCNS1CMaps() {
  SetEmbeddedCharset(CIDSET_CNS1, pdfium::make_span(g_FXCMAP_CNS1_cmaps,
                                                    g_FXCMAP_CNS1_cmaps_size));
  SetEmbeddedToUnicode(CIDSET_CNS1, g_FXCMAP_CNS1CID2Unicode_5);
}

void CPDF_FontGlobals::LoadEmbeddedJapan1CMaps() {
  SetEmbeddedCharset(
      CIDSET_JAPAN1,
      pdfium::make_span(g_FXCMAP_Japan1_cmaps, g_FXCMAP_Japan1_cmaps_size));
  SetEmbeddedToUnicode(CIDSET_JAPAN1, g_FXCMAP_Japan1CID2Unicode_4);
}

void CPDF_FontGlobals::LoadEmbeddedKorea1CMaps() {
  SetEmbeddedCharset(
      CIDSET_KOREA1,
      pdfium::make_span(g_FXCMAP_Korea1_cmaps, g_FXCMAP_Korea1_cmaps_size));
  SetEmbeddedToUnicode(CIDSET_KOREA1, g_FXCMAP_Korea1CID2Unicode_2);
}
