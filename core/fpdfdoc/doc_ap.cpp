// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_font/include/cpdf_font.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_simple_parser.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"
#include "core/fpdfdoc/cpvt_provider.h"
#include "core/fpdfdoc/doc_utils.h"
#include "core/fpdfdoc/pdf_vt.h"
#include "core/include/fpdfdoc/fpdf_doc.h"
#include "core/include/fpdfdoc/fpdf_vt.h"

CPVT_Provider::CPVT_Provider(IPVT_FontMap* pFontMap) : m_pFontMap(pFontMap) {
  ASSERT(m_pFontMap);
}
CPVT_Provider::~CPVT_Provider() {}
int32_t CPVT_Provider::GetCharWidth(int32_t nFontIndex,
                                    uint16_t word,
                                    int32_t nWordStyle) {
  if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex)) {
    uint32_t charcode = pPDFFont->CharCodeFromUnicode(word);
    if (charcode != CPDF_Font::kInvalidCharCode) {
      return pPDFFont->GetCharWidthF(charcode);
    }
  }
  return 0;
}
int32_t CPVT_Provider::GetTypeAscent(int32_t nFontIndex) {
  if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex)) {
    return pPDFFont->GetTypeAscent();
  }
  return 0;
}
int32_t CPVT_Provider::GetTypeDescent(int32_t nFontIndex) {
  if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex)) {
    return pPDFFont->GetTypeDescent();
  }
  return 0;
}
int32_t CPVT_Provider::GetWordFontIndex(uint16_t word,
                                        int32_t charset,
                                        int32_t nFontIndex) {
  if (CPDF_Font* pDefFont = m_pFontMap->GetPDFFont(0)) {
    if (pDefFont->CharCodeFromUnicode(word) != CPDF_Font::kInvalidCharCode) {
      return 0;
    }
  }
  if (CPDF_Font* pSysFont = m_pFontMap->GetPDFFont(1)) {
    if (pSysFont->CharCodeFromUnicode(word) != CPDF_Font::kInvalidCharCode) {
      return 1;
    }
  }
  return -1;
}
FX_BOOL CPVT_Provider::IsLatinWord(uint16_t word) {
  if ((word >= 0x61 && word <= 0x7A) || (word >= 0x41 && word <= 0x5A) ||
      word == 0x2D || word == 0x27) {
    return TRUE;
  }
  return FALSE;
}
int32_t CPVT_Provider::GetDefaultFontIndex() {
  return 0;
}

