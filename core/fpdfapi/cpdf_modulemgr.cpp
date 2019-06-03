// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/cpdf_modulemgr.h"

#include "core/fpdfapi/cmaps/CNS1/cmaps_cns1.h"
#include "core/fpdfapi/cmaps/GB1/cmaps_gb1.h"
#include "core/fpdfapi/cmaps/Japan1/cmaps_japan1.h"
#include "core/fpdfapi/cmaps/Korea1/cmaps_korea1.h"
#include "core/fpdfapi/font/cpdf_fontglobals.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fxcodec/fx_codec.h"
#include "third_party/base/ptr_util.h"

namespace {

CPDF_ModuleMgr* g_pDefaultMgr = nullptr;

}  // namespace

// static
void CPDF_ModuleMgr::Create() {
  ASSERT(!g_pDefaultMgr);
  g_pDefaultMgr = new CPDF_ModuleMgr;
  CCodec_ModuleMgr::Create();
  CPDF_PageModule::Create();
  g_pDefaultMgr->LoadEmbeddedMaps();
}

// static
void CPDF_ModuleMgr::Destroy() {
  ASSERT(g_pDefaultMgr);
  CPDF_PageModule::Destroy();
  CCodec_ModuleMgr::Destroy();
  delete g_pDefaultMgr;
  g_pDefaultMgr = nullptr;
}

// static
CPDF_ModuleMgr* CPDF_ModuleMgr::Get() {
  ASSERT(g_pDefaultMgr);
  return g_pDefaultMgr;
}

CPDF_ModuleMgr::CPDF_ModuleMgr() = default;

CPDF_ModuleMgr::~CPDF_ModuleMgr() = default;

void CPDF_ModuleMgr::LoadEmbeddedMaps() {
  LoadEmbeddedGB1CMaps();
  LoadEmbeddedCNS1CMaps();
  LoadEmbeddedJapan1CMaps();
  LoadEmbeddedKorea1CMaps();
}

void CPDF_ModuleMgr::LoadEmbeddedGB1CMaps() {
  auto* pFontGlobals = CPDF_FontGlobals::GetInstance();
  pFontGlobals->SetEmbeddedCharset(
      CIDSET_GB1,
      pdfium::make_span(g_FXCMAP_GB1_cmaps, g_FXCMAP_GB1_cmaps_size));
  pFontGlobals->SetEmbeddedToUnicode(CIDSET_GB1, g_FXCMAP_GB1CID2Unicode_5);
}

void CPDF_ModuleMgr::LoadEmbeddedCNS1CMaps() {
  auto* pFontGlobals = CPDF_FontGlobals::GetInstance();
  pFontGlobals->SetEmbeddedCharset(
      CIDSET_CNS1,
      pdfium::make_span(g_FXCMAP_CNS1_cmaps, g_FXCMAP_CNS1_cmaps_size));
  pFontGlobals->SetEmbeddedToUnicode(CIDSET_CNS1, g_FXCMAP_CNS1CID2Unicode_5);
}

void CPDF_ModuleMgr::LoadEmbeddedJapan1CMaps() {
  auto* pFontGlobals = CPDF_FontGlobals::GetInstance();
  pFontGlobals->SetEmbeddedCharset(
      CIDSET_JAPAN1,
      pdfium::make_span(g_FXCMAP_Japan1_cmaps, g_FXCMAP_Japan1_cmaps_size));
  pFontGlobals->SetEmbeddedToUnicode(CIDSET_JAPAN1,
                                     g_FXCMAP_Japan1CID2Unicode_4);
}

void CPDF_ModuleMgr::LoadEmbeddedKorea1CMaps() {
  auto* pFontGlobals = CPDF_FontGlobals::GetInstance();
  pFontGlobals->SetEmbeddedCharset(
      CIDSET_KOREA1,
      pdfium::make_span(g_FXCMAP_Korea1_cmaps, g_FXCMAP_Korea1_cmaps_size));
  pFontGlobals->SetEmbeddedToUnicode(CIDSET_KOREA1,
                                     g_FXCMAP_Korea1CID2Unicode_2);
}
