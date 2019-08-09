// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cba_fontmap.h"

#include <memory>
#include <utility>

#include "constants/annotation_common.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/font/cpdf_fontencoding.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fpdfdoc/cpdf_defaultappearance.h"
#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fpdfdoc/ipvt_fontmap.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_substfont.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

bool FindNativeTrueTypeFont(ByteString sFontFaceName) {
  CFX_FontMgr* pFontMgr = CFX_GEModule::Get()->GetFontMgr();
  if (!pFontMgr)
    return false;

  CFX_FontMapper* pFontMapper = pFontMgr->GetBuiltinMapper();
  pFontMapper->LoadInstalledFonts();

  for (const auto& font : pFontMapper->m_InstalledTTFonts) {
    if (font.Compare(sFontFaceName.AsStringView()))
      return true;
  }
  for (const auto& fontPair : pFontMapper->m_LocalizedTTFonts) {
    if (fontPair.first.Compare(sFontFaceName.AsStringView()))
      return true;
  }
  return false;
}

RetainPtr<CPDF_Font> AddNativeTrueTypeFontToPDF(CPDF_Document* pDoc,
                                                ByteString sFontFaceName,
                                                uint8_t nCharset) {
  if (!pDoc)
    return nullptr;

  auto pFXFont = pdfium::MakeUnique<CFX_Font>();
  pFXFont->LoadSubst(sFontFaceName, true, 0, 0, 0,
                     FX_GetCodePageFromCharset(nCharset), false);

  auto* pDocPageData = CPDF_DocPageData::FromDocument(pDoc);
  return pDocPageData->AddFont(std::move(pFXFont), nCharset);
}

}  // namespace

CBA_FontMap::Data::Data() = default;

CBA_FontMap::Data::~Data() = default;

CBA_FontMap::CBA_FontMap(CPDF_Document* pDocument, CPDF_Dictionary* pAnnotDict)
    : m_pDocument(pDocument), m_pAnnotDict(pAnnotDict) {
  Initialize();
}

CBA_FontMap::~CBA_FontMap() {
  Empty();
}

RetainPtr<CPDF_Font> CBA_FontMap::GetPDFFont(int32_t nFontIndex) {
  if (pdfium::IndexInBounds(m_Data, nFontIndex))
    return m_Data[nFontIndex]->pFont;
  return nullptr;
}

ByteString CBA_FontMap::GetPDFFontAlias(int32_t nFontIndex) {
  if (pdfium::IndexInBounds(m_Data, nFontIndex))
    return m_Data[nFontIndex]->sFontName;
  return ByteString();
}

int32_t CBA_FontMap::GetWordFontIndex(uint16_t word,
                                      int32_t nCharset,
                                      int32_t nFontIndex) {
  if (nFontIndex > 0) {
    if (KnowWord(nFontIndex, word))
      return nFontIndex;
  } else {
    if (!m_Data.empty()) {
      const Data* pData = m_Data.front().get();
      if (nCharset == FX_CHARSET_Default ||
          pData->nCharset == FX_CHARSET_Symbol || nCharset == pData->nCharset) {
        if (KnowWord(0, word))
          return 0;
      }
    }
  }

  int32_t nNewFontIndex =
      GetFontIndex(GetCachedNativeFontName(nCharset), nCharset, true);
  if (nNewFontIndex >= 0) {
    if (KnowWord(nNewFontIndex, word))
      return nNewFontIndex;
  }
  nNewFontIndex = GetFontIndex(CFX_Font::kUniversalDefaultFontName,
                               FX_CHARSET_Default, false);
  if (nNewFontIndex >= 0) {
    if (KnowWord(nNewFontIndex, word))
      return nNewFontIndex;
  }
  return -1;
}

int32_t CBA_FontMap::CharCodeFromUnicode(int32_t nFontIndex, uint16_t word) {
  if (!pdfium::IndexInBounds(m_Data, nFontIndex))
    return -1;

  Data* pData = m_Data[nFontIndex].get();
  if (!pData->pFont)
    return -1;

  if (pData->pFont->IsUnicodeCompatible())
    return pData->pFont->CharCodeFromUnicode(word);

  return word < 0xFF ? word : -1;
}

int32_t CBA_FontMap::CharSetFromUnicode(uint16_t word, int32_t nOldCharset) {
  // to avoid CJK Font to show ASCII
  if (word < 0x7F)
    return FX_CHARSET_ANSI;

  // follow the old charset
  if (nOldCharset != FX_CHARSET_Default)
    return nOldCharset;

  return CFX_Font::GetCharSetFromUnicode(word);
}

int32_t CBA_FontMap::GetNativeCharset() {
  return FX_GetCharsetFromCodePage(FXSYS_GetACP());
}

void CBA_FontMap::Reset() {
  Empty();
  m_pDefaultFont = nullptr;
  m_sDefaultFontName.clear();
}

void CBA_FontMap::SetDefaultFont(const RetainPtr<CPDF_Font>& pFont,
                                 const ByteString& sFontName) {
  ASSERT(pFont);

  if (m_pDefaultFont)
    return;

  m_pDefaultFont = pFont;
  m_sDefaultFontName = sFontName;

  const CFX_SubstFont* pSubstFont = m_pDefaultFont->GetSubstFont();
  int32_t nCharset = pSubstFont ? pSubstFont->m_Charset : FX_CHARSET_Default;
  AddFontData(m_pDefaultFont, m_sDefaultFontName, nCharset);
}

void CBA_FontMap::SetAPType(const ByteString& sAPType) {
  m_sAPType = sAPType;

  Reset();
  Initialize();
}

void CBA_FontMap::Initialize() {
  int32_t nCharset = FX_CHARSET_Default;

  if (!m_pDefaultFont) {
    m_pDefaultFont = GetAnnotDefaultFont(&m_sDefaultFontName);
    if (m_pDefaultFont) {
      if (const CFX_SubstFont* pSubstFont = m_pDefaultFont->GetSubstFont()) {
        nCharset = pSubstFont->m_Charset;
      } else {
        if (m_sDefaultFontName == "Wingdings" ||
            m_sDefaultFontName == "Wingdings2" ||
            m_sDefaultFontName == "Wingdings3" ||
            m_sDefaultFontName == "Webdings")
          nCharset = FX_CHARSET_Symbol;
        else
          nCharset = FX_CHARSET_ANSI;
      }
      AddFontData(m_pDefaultFont, m_sDefaultFontName, nCharset);
      AddFontToAnnotDict(m_pDefaultFont, m_sDefaultFontName);
    }
  }

  if (nCharset != FX_CHARSET_ANSI)
    GetFontIndex(CFX_Font::kDefaultAnsiFontName, FX_CHARSET_ANSI, false);
}

RetainPtr<CPDF_Font> CBA_FontMap::FindFontSameCharset(ByteString* sFontAlias,
                                                      int32_t nCharset) {
  if (m_pAnnotDict->GetStringFor(pdfium::annotation::kSubtype) != "Widget")
    return nullptr;

  const CPDF_Dictionary* pRootDict = m_pDocument->GetRoot();
  if (!pRootDict)
    return nullptr;

  const CPDF_Dictionary* pAcroFormDict = pRootDict->GetDictFor("AcroForm");
  if (!pAcroFormDict)
    return nullptr;

  const CPDF_Dictionary* pDRDict = pAcroFormDict->GetDictFor("DR");
  if (!pDRDict)
    return nullptr;

  return FindResFontSameCharset(pDRDict, sFontAlias, nCharset);
}

RetainPtr<CPDF_Font> CBA_FontMap::FindResFontSameCharset(
    const CPDF_Dictionary* pResDict,
    ByteString* sFontAlias,
    int32_t nCharset) {
  if (!pResDict)
    return nullptr;

  const CPDF_Dictionary* pFonts = pResDict->GetDictFor("Font");
  if (!pFonts)
    return nullptr;

  RetainPtr<CPDF_Font> pFind;
  CPDF_DictionaryLocker locker(pFonts);
  for (const auto& it : locker) {
    const ByteString& csKey = it.first;
    if (!it.second)
      continue;

    CPDF_Dictionary* pElement = ToDictionary(it.second->GetDirect());
    if (!pElement || pElement->GetStringFor("Type") != "Font")
      continue;

    auto* pData = CPDF_DocPageData::FromDocument(m_pDocument.Get());
    RetainPtr<CPDF_Font> pFont = pData->GetFont(pElement);
    if (!pFont)
      continue;

    const CFX_SubstFont* pSubst = pFont->GetSubstFont();
    if (!pSubst)
      continue;

    if (pSubst->m_Charset == nCharset) {
      *sFontAlias = csKey;
      pFind = std::move(pFont);
    }
  }
  return pFind;
}

RetainPtr<CPDF_Font> CBA_FontMap::GetAnnotDefaultFont(ByteString* sAlias) {
  CPDF_Dictionary* pAcroFormDict = nullptr;
  const bool bWidget =
      (m_pAnnotDict->GetStringFor(pdfium::annotation::kSubtype) == "Widget");
  if (bWidget) {
    CPDF_Dictionary* pRootDict = m_pDocument->GetRoot();
    if (pRootDict)
      pAcroFormDict = pRootDict->GetDictFor("AcroForm");
  }

  ByteString sDA;
  const CPDF_Object* pObj = FPDF_GetFieldAttr(m_pAnnotDict.Get(), "DA");
  if (pObj)
    sDA = pObj->GetString();

  if (bWidget) {
    if (sDA.IsEmpty()) {
      pObj = FPDF_GetFieldAttr(pAcroFormDict, "DA");
      sDA = pObj ? pObj->GetString() : ByteString();
    }
  }
  if (sDA.IsEmpty())
    return nullptr;

  CPDF_DefaultAppearance appearance(sDA);
  float font_size;
  Optional<ByteString> font = appearance.GetFont(&font_size);
  *sAlias = font.value_or(ByteString());

  CPDF_Dictionary* pFontDict = nullptr;
  if (CPDF_Dictionary* pAPDict =
          m_pAnnotDict->GetDictFor(pdfium::annotation::kAP)) {
    if (CPDF_Dictionary* pNormalDict = pAPDict->GetDictFor("N")) {
      if (CPDF_Dictionary* pNormalResDict =
              pNormalDict->GetDictFor("Resources")) {
        if (CPDF_Dictionary* pResFontDict = pNormalResDict->GetDictFor("Font"))
          pFontDict = pResFontDict->GetDictFor(*sAlias);
      }
    }
  }
  if (bWidget && !pFontDict && pAcroFormDict) {
    if (CPDF_Dictionary* pDRDict = pAcroFormDict->GetDictFor("DR")) {
      if (CPDF_Dictionary* pDRFontDict = pDRDict->GetDictFor("Font"))
        pFontDict = pDRFontDict->GetDictFor(*sAlias);
    }
  }
  if (!pFontDict)
    return nullptr;

  return CPDF_DocPageData::FromDocument(m_pDocument.Get())->GetFont(pFontDict);
}

void CBA_FontMap::AddFontToAnnotDict(const RetainPtr<CPDF_Font>& pFont,
                                     const ByteString& sAlias) {
  if (!pFont)
    return;

  CPDF_Dictionary* pAPDict = m_pAnnotDict->GetDictFor(pdfium::annotation::kAP);
  if (!pAPDict)
    pAPDict = m_pAnnotDict->SetNewFor<CPDF_Dictionary>(pdfium::annotation::kAP);

  // to avoid checkbox and radiobutton
  if (ToDictionary(pAPDict->GetObjectFor(m_sAPType)))
    return;

  CPDF_Stream* pStream = pAPDict->GetStreamFor(m_sAPType);
  if (!pStream) {
    pStream = m_pDocument->NewIndirect<CPDF_Stream>();
    pAPDict->SetNewFor<CPDF_Reference>(m_sAPType, m_pDocument.Get(),
                                       pStream->GetObjNum());
  }

  CPDF_Dictionary* pStreamDict = pStream->GetDict();
  if (!pStreamDict) {
    auto pOwnedDict = m_pDocument->New<CPDF_Dictionary>();
    pStreamDict = pOwnedDict.Get();
    pStream->InitStream({}, std::move(pOwnedDict));
  }

  CPDF_Dictionary* pStreamResList = pStreamDict->GetDictFor("Resources");
  if (!pStreamResList)
    pStreamResList = pStreamDict->SetNewFor<CPDF_Dictionary>("Resources");
  CPDF_Dictionary* pStreamResFontList = pStreamResList->GetDictFor("Font");
  if (!pStreamResFontList) {
    pStreamResFontList = m_pDocument->NewIndirect<CPDF_Dictionary>();
    pStreamResList->SetNewFor<CPDF_Reference>("Font", m_pDocument.Get(),
                                              pStreamResFontList->GetObjNum());
  }
  if (!pStreamResFontList->KeyExist(sAlias)) {
    CPDF_Dictionary* pFontDict = pFont->GetFontDict();
    RetainPtr<CPDF_Object> pObject =
        pFontDict->IsInline() ? pFontDict->Clone()
                              : pFontDict->MakeReference(m_pDocument.Get());
    pStreamResFontList->SetFor(sAlias, std::move(pObject));
  }
}

bool CBA_FontMap::KnowWord(int32_t nFontIndex, uint16_t word) {
  return pdfium::IndexInBounds(m_Data, nFontIndex) &&
         CharCodeFromUnicode(nFontIndex, word) >= 0;
}

void CBA_FontMap::Empty() {
  m_Data.clear();
  m_NativeFont.clear();
}

int32_t CBA_FontMap::GetFontIndex(const ByteString& sFontName,
                                  int32_t nCharset,
                                  bool bFind) {
  int32_t nFontIndex = FindFont(EncodeFontAlias(sFontName, nCharset), nCharset);
  if (nFontIndex >= 0)
    return nFontIndex;

  ByteString sAlias;
  RetainPtr<CPDF_Font> pFont =
      bFind ? FindFontSameCharset(&sAlias, nCharset) : nullptr;
  if (!pFont) {
    ByteString sTemp = sFontName;
    pFont = AddFontToDocument(m_pDocument.Get(), sTemp, nCharset);
    sAlias = EncodeFontAlias(sTemp, nCharset);
  }
  AddFontToAnnotDict(pFont, sAlias);
  return AddFontData(pFont, sAlias, nCharset);
}

int32_t CBA_FontMap::AddFontData(const RetainPtr<CPDF_Font>& pFont,
                                 const ByteString& sFontAlias,
                                 int32_t nCharset) {
  auto pNewData = pdfium::MakeUnique<Data>();
  pNewData->pFont = pFont;
  pNewData->sFontName = sFontAlias;
  pNewData->nCharset = nCharset;
  m_Data.push_back(std::move(pNewData));
  return pdfium::CollectionSize<int32_t>(m_Data) - 1;
}

ByteString CBA_FontMap::EncodeFontAlias(const ByteString& sFontName,
                                        int32_t nCharset) {
  return EncodeFontAlias(sFontName) + ByteString::Format("_%02X", nCharset);
}

ByteString CBA_FontMap::EncodeFontAlias(const ByteString& sFontName) {
  ByteString sRet = sFontName;
  sRet.Remove(' ');
  return sRet;
}

int32_t CBA_FontMap::FindFont(const ByteString& sFontName, int32_t nCharset) {
  int32_t i = 0;
  for (const auto& pData : m_Data) {
    if ((nCharset == FX_CHARSET_Default || nCharset == pData->nCharset) &&
        (sFontName.IsEmpty() || pData->sFontName == sFontName)) {
      return i;
    }
    ++i;
  }
  return -1;
}

ByteString CBA_FontMap::GetNativeFontName(int32_t nCharset) {
  if (nCharset == FX_CHARSET_Default)
    nCharset = GetNativeCharset();

  ByteString sFontName = CFX_Font::GetDefaultFontNameByCharset(nCharset);
  if (!FindNativeTrueTypeFont(sFontName))
    return ByteString();

  return sFontName;
}

ByteString CBA_FontMap::GetCachedNativeFontName(int32_t nCharset) {
  for (const auto& pData : m_NativeFont) {
    if (pData && pData->nCharset == nCharset)
      return pData->sFontName;
  }

  ByteString sNew = GetNativeFontName(nCharset);
  if (sNew.IsEmpty())
    return ByteString();

  auto pNewData = pdfium::MakeUnique<Native>();
  pNewData->nCharset = nCharset;
  pNewData->sFontName = sNew;
  m_NativeFont.push_back(std::move(pNewData));
  return sNew;
}

RetainPtr<CPDF_Font> CBA_FontMap::AddFontToDocument(CPDF_Document* pDoc,
                                                    ByteString& sFontName,
                                                    uint8_t nCharset) {
  if (IsStandardFont(sFontName))
    return AddStandardFont(pDoc, sFontName);

  return AddSystemFont(pDoc, sFontName, nCharset);
}

bool CBA_FontMap::IsStandardFont(const ByteString& sFontName) {
  static const char* const kStandardFontNames[] = {"Courier",
                                                   "Courier-Bold",
                                                   "Courier-BoldOblique",
                                                   "Courier-Oblique",
                                                   "Helvetica",
                                                   "Helvetica-Bold",
                                                   "Helvetica-BoldOblique",
                                                   "Helvetica-Oblique",
                                                   "Times-Roman",
                                                   "Times-Bold",
                                                   "Times-Italic",
                                                   "Times-BoldItalic",
                                                   "Symbol",
                                                   "ZapfDingbats"};
  for (const char* name : kStandardFontNames) {
    if (sFontName == name)
      return true;
  }
  return false;
}

RetainPtr<CPDF_Font> CBA_FontMap::AddStandardFont(CPDF_Document* pDoc,
                                                  ByteString& sFontName) {
  if (!pDoc)
    return nullptr;

  auto* pPageData = CPDF_DocPageData::FromDocument(pDoc);
  if (sFontName == "ZapfDingbats")
    return pPageData->AddStandardFont(sFontName.c_str(), nullptr);

  static const CPDF_FontEncoding fe(PDFFONT_ENCODING_WINANSI);
  return pPageData->AddStandardFont(sFontName.c_str(), &fe);
}

RetainPtr<CPDF_Font> CBA_FontMap::AddSystemFont(CPDF_Document* pDoc,
                                                ByteString& sFontName,
                                                uint8_t nCharset) {
  if (!pDoc)
    return nullptr;

  if (sFontName.IsEmpty())
    sFontName = GetNativeFontName(nCharset);

  if (nCharset == FX_CHARSET_Default)
    nCharset = GetNativeCharset();

  return AddNativeTrueTypeFontToPDF(pDoc, sFontName, nCharset);
}
