// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cpdf_fontglobals.h"

#include <utility>

#include "core/fpdfapi/cmaps/CNS1/cmaps_cns1.h"
#include "core/fpdfapi/cmaps/GB1/cmaps_gb1.h"
#include "core/fpdfapi/cmaps/Japan1/cmaps_japan1.h"
#include "core/fpdfapi/cmaps/Korea1/cmaps_korea1.h"
#include "core/fpdfapi/font/cfx_stockfontarray.h"
#include "core/fpdfapi/font/cpdf_cid2unicodemap.h"
#include "core/fpdfapi/font/cpdf_cmap.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/contains.h"

namespace {

CPDF_FontGlobals* g_FontGlobals = nullptr;

RetainPtr<const CPDF_CMap> LoadPredefinedCMap(ByteStringView name) {
  if (!name.IsEmpty() && name[0] == '/') {
    name = name.Last(name.GetLength() - 1);
  }
  return pdfium::MakeRetain<CPDF_CMap>(name);
}

}  // namespace

// static
void CPDF_FontGlobals::Create() {
  DCHECK(!g_FontGlobals);
  g_FontGlobals = new CPDF_FontGlobals();
}

// static
void CPDF_FontGlobals::Destroy() {
  DCHECK(g_FontGlobals);
  delete g_FontGlobals;
  g_FontGlobals = nullptr;
}

// static
CPDF_FontGlobals* CPDF_FontGlobals::GetInstance() {
  DCHECK(g_FontGlobals);
  return g_FontGlobals;
}

CPDF_FontGlobals::CPDF_FontGlobals() = default;

CPDF_FontGlobals::~CPDF_FontGlobals() = default;

void CPDF_FontGlobals::LoadEmbeddedMaps() {
  LoadEmbeddedGB1CMaps();
  LoadEmbeddedCNS1CMaps();
  LoadEmbeddedJapan1CMaps();
  LoadEmbeddedKorea1CMaps();
}

RetainPtr<CPDF_Font> CPDF_FontGlobals::Find(
    CPDF_Document* doc,
    CFX_FontMapper::StandardFont index) {
  auto it = stock_map_.find(doc);
  if (it == stock_map_.end() || !it->second) {
    return nullptr;
  }

  return it->second->GetFont(index);
}

void CPDF_FontGlobals::Set(CPDF_Document* doc,
                           CFX_FontMapper::StandardFont index,
                           RetainPtr<CPDF_Font> font) {
  UnownedPtr<CPDF_Document> pKey(doc);
  if (!pdfium::Contains(stock_map_, pKey)) {
    stock_map_[pKey] = std::make_unique<CFX_StockFontArray>();
  }
  stock_map_[pKey]->SetFont(index, std::move(font));
}

void CPDF_FontGlobals::Clear(CPDF_Document* doc) {
  // Avoid constructing smart-pointer key as erase() doesn't invoke
  // transparent lookup in the same way find() does.
  auto it = stock_map_.find(doc);
  if (it != stock_map_.end()) {
    stock_map_.erase(it);
  }
}

void CPDF_FontGlobals::LoadEmbeddedGB1CMaps() {
  SetEmbeddedCharset(CIDSET_GB1, fxcmap::kGB1_cmaps);
  SetEmbeddedToUnicode(CIDSET_GB1, fxcmap::kGB1CID2Unicode_5);
}

void CPDF_FontGlobals::LoadEmbeddedCNS1CMaps() {
  SetEmbeddedCharset(CIDSET_CNS1, fxcmap::kCNS1_cmaps);
  SetEmbeddedToUnicode(CIDSET_CNS1, fxcmap::kCNS1CID2Unicode_5);
}

void CPDF_FontGlobals::LoadEmbeddedJapan1CMaps() {
  SetEmbeddedCharset(CIDSET_JAPAN1, fxcmap::kJapan1_cmaps);
  SetEmbeddedToUnicode(CIDSET_JAPAN1, fxcmap::kJapan1CID2Unicode_4);
}

void CPDF_FontGlobals::LoadEmbeddedKorea1CMaps() {
  SetEmbeddedCharset(CIDSET_KOREA1, fxcmap::kKorea1_cmaps);
  SetEmbeddedToUnicode(CIDSET_KOREA1, fxcmap::kKorea1CID2Unicode_2);
}

RetainPtr<const CPDF_CMap> CPDF_FontGlobals::GetPredefinedCMap(
    const ByteString& name) {
  auto it = cmaps_.find(name);
  if (it != cmaps_.end()) {
    return it->second;
  }

  RetainPtr<const CPDF_CMap> pCMap = LoadPredefinedCMap(name.AsStringView());
  if (!name.IsEmpty()) {
    cmaps_[name] = pCMap;
  }

  return pCMap;
}

CPDF_CID2UnicodeMap* CPDF_FontGlobals::GetCID2UnicodeMap(CIDSet charset) {
  if (!cid2unicode_maps_[charset]) {
    cid2unicode_maps_[charset] = std::make_unique<CPDF_CID2UnicodeMap>(charset);
  }
  return cid2unicode_maps_[charset].get();
}
