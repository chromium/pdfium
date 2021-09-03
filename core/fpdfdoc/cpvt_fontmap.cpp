// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpvt_fontmap.h"

#include <utility>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "core/fxcrt/fx_codepage.h"
#include "third_party/base/check.h"
#include "third_party/base/notreached.h"

CPVT_FontMap::CPVT_FontMap(CPDF_Document* pDoc,
                           CPDF_Dictionary* pResDict,
                           const RetainPtr<CPDF_Font>& pDefFont,
                           const ByteString& sDefFontAlias)
    : m_pDocument(pDoc),
      m_pResDict(pResDict),
      m_pDefFont(pDefFont),
      m_sDefFontAlias(sDefFontAlias) {}

CPVT_FontMap::~CPVT_FontMap() = default;

void CPVT_FontMap::SetupAnnotSysPDFFont() {
  if (!m_pDocument || !m_pResDict)
    return;

  CPDF_Dictionary* pFormDict = m_pDocument->GetRoot()->GetDictFor("AcroForm");
  RetainPtr<CPDF_Font> pPDFFont =
      CPDF_InteractiveForm::AddNativeInteractiveFormFont(
          pFormDict, m_pDocument.Get(), &m_sSysFontAlias);
  if (!pPDFFont)
    return;

  CPDF_Dictionary* pFontList = m_pResDict->GetDictFor("Font");
  if (ValidateFontResourceDict(pFontList) &&
      !pFontList->KeyExist(m_sSysFontAlias)) {
    pFontList->SetNewFor<CPDF_Reference>(m_sSysFontAlias, m_pDocument.Get(),
                                         pPDFFont->GetFontDict()->GetObjNum());
  }
  m_pSysFont = std::move(pPDFFont);
}

RetainPtr<CPDF_Font> CPVT_FontMap::GetPDFFont(int32_t nFontIndex) {
  switch (nFontIndex) {
    case 0:
      return m_pDefFont;
    case 1:
      if (!m_pSysFont)
        SetupAnnotSysPDFFont();
      return m_pSysFont;
    default:
      return nullptr;
  }
}

ByteString CPVT_FontMap::GetPDFFontAlias(int32_t nFontIndex) {
  switch (nFontIndex) {
    case 0:
      return m_sDefFontAlias;
    case 1:
      if (!m_pSysFont)
        SetupAnnotSysPDFFont();
      return m_sSysFontAlias;
    default:
      return ByteString();
  }
}

int32_t CPVT_FontMap::GetWordFontIndex(uint16_t word,
                                       FX_Charset charset,
                                       int32_t nFontIndex) {
  NOTREACHED();
  return 0;
}

int32_t CPVT_FontMap::CharCodeFromUnicode(int32_t nFontIndex, uint16_t word) {
  NOTREACHED();
  return 0;
}

FX_Charset CPVT_FontMap::CharSetFromUnicode(uint16_t word,
                                            FX_Charset nOldCharset) {
  NOTREACHED();
  return FX_Charset::kANSI;
}
