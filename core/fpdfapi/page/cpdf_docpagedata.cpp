// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_docpagedata.h"

#include <algorithm>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fdrm/fx_crypt.h"
#include "core/fpdfapi/font/cpdf_type1font.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_iccprofile.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_pagemodule.h"
#include "core/fpdfapi/page/cpdf_pattern.h"
#include "core/fpdfapi/page/cpdf_shadingpattern.h"
#include "core/fpdfapi/page/cpdf_tilingpattern.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/cfx_unicodeencoding.h"
#include "core/fxge/fx_font.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

void InsertWidthArrayImpl(std::vector<int> widths, CPDF_Array* pWidthArray) {
  size_t i;
  for (i = 1; i < widths.size(); i++) {
    if (widths[i] != widths[0])
      break;
  }
  if (i == widths.size()) {
    int first = pWidthArray->GetIntegerAt(pWidthArray->size() - 1);
    pWidthArray->AddNew<CPDF_Number>(first + static_cast<int>(widths.size()) -
                                     1);
    pWidthArray->AddNew<CPDF_Number>(widths[0]);
    return;
  }
  CPDF_Array* pWidthArray1 = pWidthArray->AddNew<CPDF_Array>();
  for (int w : widths)
    pWidthArray1->AddNew<CPDF_Number>(w);
}

#if defined(OS_WIN)
void InsertWidthArray(HDC hDC, int start, int end, CPDF_Array* pWidthArray) {
  std::vector<int> widths(end - start + 1);
  GetCharWidth(hDC, start, end, widths.data());
  InsertWidthArrayImpl(std::move(widths), pWidthArray);
}

ByteString FPDF_GetPSNameFromTT(HDC hDC) {
  ByteString result;
  DWORD size = ::GetFontData(hDC, 'eman', 0, nullptr, 0);
  if (size != GDI_ERROR) {
    LPBYTE buffer = FX_Alloc(BYTE, size);
    ::GetFontData(hDC, 'eman', 0, buffer, size);
    result = GetNameFromTT({buffer, size}, 6);
    FX_Free(buffer);
  }
  return result;
}
#endif  // defined(OS_WIN)

void InsertWidthArray1(CFX_Font* pFont,
                       CFX_UnicodeEncoding* pEncoding,
                       wchar_t start,
                       wchar_t end,
                       CPDF_Array* pWidthArray) {
  std::vector<int> widths(end - start + 1);
  for (size_t i = 0; i < widths.size(); ++i) {
    int glyph_index = pEncoding->GlyphFromCharCode(start + i);
    widths[i] = pFont->GetGlyphWidth(glyph_index);
  }
  InsertWidthArrayImpl(std::move(widths), pWidthArray);
}

int CalculateFlags(bool bold,
                   bool italic,
                   bool fixedPitch,
                   bool serif,
                   bool script,
                   bool symbolic) {
  int flags = 0;
  if (bold)
    flags |= FXFONT_BOLD;
  if (italic)
    flags |= FXFONT_ITALIC;
  if (fixedPitch)
    flags |= FXFONT_FIXED_PITCH;
  if (serif)
    flags |= FXFONT_SERIF;
  if (script)
    flags |= FXFONT_SCRIPT;
  if (symbolic)
    flags |= FXFONT_SYMBOLIC;
  else
    flags |= FXFONT_NONSYMBOLIC;
  return flags;
}

void ProcessNonbCJK(CPDF_Dictionary* pBaseDict,
                    bool bold,
                    bool italic,
                    ByteString basefont,
                    RetainPtr<CPDF_Array> pWidths) {
  if (bold && italic)
    basefont += ",BoldItalic";
  else if (bold)
    basefont += ",Bold";
  else if (italic)
    basefont += ",Italic";
  pBaseDict->SetNewFor<CPDF_Name>("Subtype", "TrueType");
  pBaseDict->SetNewFor<CPDF_Name>("BaseFont", basefont);
  pBaseDict->SetNewFor<CPDF_Number>("FirstChar", 32);
  pBaseDict->SetNewFor<CPDF_Number>("LastChar", 255);
  pBaseDict->SetFor("Widths", pWidths);
}

RetainPtr<CPDF_Dictionary> CalculateFontDesc(CPDF_Document* pDoc,
                                             ByteString basefont,
                                             int flags,
                                             int italicangle,
                                             int ascend,
                                             int descend,
                                             RetainPtr<CPDF_Array> bbox,
                                             int32_t stemV) {
  auto pFontDesc = pDoc->New<CPDF_Dictionary>();
  pFontDesc->SetNewFor<CPDF_Name>("Type", "FontDescriptor");
  pFontDesc->SetNewFor<CPDF_Name>("FontName", basefont);
  pFontDesc->SetNewFor<CPDF_Number>("Flags", flags);
  pFontDesc->SetFor("FontBBox", bbox);
  pFontDesc->SetNewFor<CPDF_Number>("ItalicAngle", italicangle);
  pFontDesc->SetNewFor<CPDF_Number>("Ascent", ascend);
  pFontDesc->SetNewFor<CPDF_Number>("Descent", descend);
  pFontDesc->SetNewFor<CPDF_Number>("StemV", stemV);
  return pFontDesc;
}

}  // namespace

// static
CPDF_DocPageData* CPDF_DocPageData::FromDocument(const CPDF_Document* pDoc) {
  return static_cast<CPDF_DocPageData*>(pDoc->GetPageData());
}

CPDF_DocPageData::CPDF_DocPageData() = default;

CPDF_DocPageData::~CPDF_DocPageData() {
  for (auto& it : m_FontMap) {
    if (it.second)
      it.second->WillBeDestroyed();
  }
}

void CPDF_DocPageData::ClearStockFont() {
  CPDF_PageModule::GetInstance()->ClearStockFont(GetDocument());
}

RetainPtr<CPDF_Font> CPDF_DocPageData::GetFont(CPDF_Dictionary* pFontDict) {
  if (!pFontDict)
    return nullptr;

  auto it = m_FontMap.find(pFontDict);
  if (it != m_FontMap.end() && it->second)
    return pdfium::WrapRetain(it->second.Get());

  RetainPtr<CPDF_Font> pFont =
      CPDF_Font::Create(GetDocument(), pFontDict, this);
  if (!pFont)
    return nullptr;

  m_FontMap[pFontDict].Reset(pFont.Get());
  return pFont;
}

RetainPtr<CPDF_Font> CPDF_DocPageData::GetStandardFont(
    const ByteString& fontName,
    const CPDF_FontEncoding* pEncoding) {
  if (fontName.IsEmpty())
    return nullptr;

  for (auto& it : m_FontMap) {
    CPDF_Font* pFont = it.second.Get();
    if (!pFont)
      continue;
    if (pFont->GetBaseFont() != fontName)
      continue;
    if (pFont->IsEmbedded())
      continue;
    if (!pFont->IsType1Font())
      continue;
    if (pFont->GetFontDict()->KeyExist("Widths"))
      continue;

    CPDF_Type1Font* pT1Font = pFont->AsType1Font();
    if (pEncoding && !pT1Font->GetEncoding()->IsIdentical(pEncoding))
      continue;

    return pdfium::WrapRetain(pFont);
  }

  CPDF_Dictionary* pDict = GetDocument()->NewIndirect<CPDF_Dictionary>();
  pDict->SetNewFor<CPDF_Name>("Type", "Font");
  pDict->SetNewFor<CPDF_Name>("Subtype", "Type1");
  pDict->SetNewFor<CPDF_Name>("BaseFont", fontName);
  if (pEncoding) {
    pDict->SetFor("Encoding",
                  pEncoding->Realize(GetDocument()->GetByteStringPool()));
  }

  // Note: NULL FormFactoryIface OK since known Type1 font from above.
  RetainPtr<CPDF_Font> pFont = CPDF_Font::Create(GetDocument(), pDict, nullptr);
  if (!pFont)
    return nullptr;

  m_FontMap[pDict].Reset(pFont.Get());
  return pFont;
}

RetainPtr<CPDF_ColorSpace> CPDF_DocPageData::GetColorSpace(
    const CPDF_Object* pCSObj,
    const CPDF_Dictionary* pResources) {
  std::set<const CPDF_Object*> visited;
  return GetColorSpaceGuarded(pCSObj, pResources, &visited);
}

RetainPtr<CPDF_ColorSpace> CPDF_DocPageData::GetColorSpaceGuarded(
    const CPDF_Object* pCSObj,
    const CPDF_Dictionary* pResources,
    std::set<const CPDF_Object*>* pVisited) {
  std::set<const CPDF_Object*> visitedLocal;
  return GetColorSpaceInternal(pCSObj, pResources, pVisited, &visitedLocal);
}

RetainPtr<CPDF_ColorSpace> CPDF_DocPageData::GetColorSpaceInternal(
    const CPDF_Object* pCSObj,
    const CPDF_Dictionary* pResources,
    std::set<const CPDF_Object*>* pVisited,
    std::set<const CPDF_Object*>* pVisitedInternal) {
  if (!pCSObj)
    return nullptr;

  if (pdfium::ContainsKey(*pVisitedInternal, pCSObj))
    return nullptr;

  pdfium::ScopedSetInsertion<const CPDF_Object*> insertion(pVisitedInternal,
                                                           pCSObj);

  if (pCSObj->IsName()) {
    ByteString name = pCSObj->GetString();
    RetainPtr<CPDF_ColorSpace> pCS = CPDF_ColorSpace::ColorspaceFromName(name);
    if (!pCS && pResources) {
      const CPDF_Dictionary* pList = pResources->GetDictFor("ColorSpace");
      if (pList) {
        return GetColorSpaceInternal(pList->GetDirectObjectFor(name), nullptr,
                                     pVisited, pVisitedInternal);
      }
    }
    if (!pCS || !pResources)
      return pCS;

    const CPDF_Dictionary* pColorSpaces = pResources->GetDictFor("ColorSpace");
    if (!pColorSpaces)
      return pCS;

    const CPDF_Object* pDefaultCS = nullptr;
    switch (pCS->GetFamily()) {
      case PDFCS_DEVICERGB:
        pDefaultCS = pColorSpaces->GetDirectObjectFor("DefaultRGB");
        break;
      case PDFCS_DEVICEGRAY:
        pDefaultCS = pColorSpaces->GetDirectObjectFor("DefaultGray");
        break;
      case PDFCS_DEVICECMYK:
        pDefaultCS = pColorSpaces->GetDirectObjectFor("DefaultCMYK");
        break;
    }
    if (!pDefaultCS)
      return pCS;

    return GetColorSpaceInternal(pDefaultCS, nullptr, pVisited,
                                 pVisitedInternal);
  }

  const CPDF_Array* pArray = pCSObj->AsArray();
  if (!pArray || pArray->IsEmpty())
    return nullptr;

  if (pArray->size() == 1) {
    return GetColorSpaceInternal(pArray->GetDirectObjectAt(0), pResources,
                                 pVisited, pVisitedInternal);
  }

  auto it = m_ColorSpaceMap.find(pCSObj);
  if (it != m_ColorSpaceMap.end() && it->second)
    return pdfium::WrapRetain(it->second.Get());

  RetainPtr<CPDF_ColorSpace> pCS =
      CPDF_ColorSpace::Load(GetDocument(), pArray, pVisited);
  if (!pCS)
    return nullptr;

  m_ColorSpaceMap[pCSObj].Reset(pCS.Get());
  return pCS;
}

RetainPtr<CPDF_Pattern> CPDF_DocPageData::GetPattern(CPDF_Object* pPatternObj,
                                                     bool bShading,
                                                     const CFX_Matrix& matrix) {
  if (!pPatternObj)
    return nullptr;

  auto it = m_PatternMap.find(pPatternObj);
  if (it != m_PatternMap.end() && it->second)
    return pdfium::WrapRetain(it->second.Get());

  RetainPtr<CPDF_Pattern> pPattern;
  if (bShading) {
    pPattern = pdfium::MakeRetain<CPDF_ShadingPattern>(
        GetDocument(), pPatternObj, true, matrix);
  } else {
    CPDF_Dictionary* pDict = pPatternObj->GetDict();
    if (!pDict)
      return nullptr;

    int type = pDict->GetIntegerFor("PatternType");
    if (type == CPDF_Pattern::kTiling) {
      pPattern = pdfium::MakeRetain<CPDF_TilingPattern>(GetDocument(),
                                                        pPatternObj, matrix);
    } else if (type == CPDF_Pattern::kShading) {
      pPattern = pdfium::MakeRetain<CPDF_ShadingPattern>(
          GetDocument(), pPatternObj, false, matrix);
    } else {
      return nullptr;
    }
  }

  m_PatternMap[pPatternObj].Reset(pPattern.Get());
  return pPattern;
}

RetainPtr<CPDF_Image> CPDF_DocPageData::GetImage(uint32_t dwStreamObjNum) {
  ASSERT(dwStreamObjNum);
  auto it = m_ImageMap.find(dwStreamObjNum);
  if (it != m_ImageMap.end())
    return it->second;

  auto pImage = pdfium::MakeRetain<CPDF_Image>(GetDocument(), dwStreamObjNum);
  m_ImageMap[dwStreamObjNum] = pImage;
  return pImage;
}

void CPDF_DocPageData::MaybePurgeImage(uint32_t dwStreamObjNum) {
  ASSERT(dwStreamObjNum);
  auto it = m_ImageMap.find(dwStreamObjNum);
  if (it != m_ImageMap.end() && it->second->HasOneRef())
    m_ImageMap.erase(it);
}

RetainPtr<CPDF_IccProfile> CPDF_DocPageData::GetIccProfile(
    const CPDF_Stream* pProfileStream) {
  if (!pProfileStream)
    return nullptr;

  auto it = m_IccProfileMap.find(pProfileStream);
  if (it != m_IccProfileMap.end() && it->second)
    return pdfium::WrapRetain(it->second.Get());

  auto pAccessor = pdfium::MakeRetain<CPDF_StreamAcc>(pProfileStream);
  pAccessor->LoadAllDataFiltered();

  uint8_t digest[20];
  CRYPT_SHA1Generate(pAccessor->GetData(), pAccessor->GetSize(), digest);

  ByteString bsDigest(digest, 20);
  auto hash_it = m_HashProfileMap.find(bsDigest);
  if (hash_it != m_HashProfileMap.end()) {
    auto it_copied_stream = m_IccProfileMap.find(hash_it->second.Get());
    if (it_copied_stream != m_IccProfileMap.end() && it_copied_stream->second)
      return pdfium::WrapRetain(it_copied_stream->second.Get());
  }
  auto pProfile =
      pdfium::MakeRetain<CPDF_IccProfile>(pProfileStream, pAccessor->GetSpan());
  m_IccProfileMap[pProfileStream].Reset(pProfile.Get());
  m_HashProfileMap[bsDigest].Reset(pProfileStream);
  return pProfile;
}

RetainPtr<CPDF_StreamAcc> CPDF_DocPageData::GetFontFileStreamAcc(
    const CPDF_Stream* pFontStream) {
  ASSERT(pFontStream);
  auto it = m_FontFileMap.find(pFontStream);
  if (it != m_FontFileMap.end())
    return it->second;

  const CPDF_Dictionary* pFontDict = pFontStream->GetDict();
  int32_t len1 = pFontDict->GetIntegerFor("Length1");
  int32_t len2 = pFontDict->GetIntegerFor("Length2");
  int32_t len3 = pFontDict->GetIntegerFor("Length3");
  uint32_t org_size = 0;
  if (len1 >= 0 && len2 >= 0 && len3 >= 0) {
    FX_SAFE_UINT32 safe_org_size = len1;
    safe_org_size += len2;
    safe_org_size += len3;
    org_size = safe_org_size.ValueOrDefault(0);
  }

  auto pFontAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pFontStream);
  pFontAcc->LoadAllDataFilteredWithEstimatedSize(org_size);
  m_FontFileMap[pFontStream] = pFontAcc;
  return pFontAcc;
}

void CPDF_DocPageData::MaybePurgeFontFileStreamAcc(
    const CPDF_Stream* pFontStream) {
  if (!pFontStream)
    return;

  auto it = m_FontFileMap.find(pFontStream);
  if (it != m_FontFileMap.end() && it->second->HasOneRef())
    m_FontFileMap.erase(it);
}

std::unique_ptr<CPDF_Font::FormIface> CPDF_DocPageData::CreateForm(
    CPDF_Document* pDocument,
    CPDF_Dictionary* pPageResources,
    CPDF_Stream* pFormStream) {
  return pdfium::MakeUnique<CPDF_Form>(pDocument, pPageResources, pFormStream);
}

RetainPtr<CPDF_Font> CPDF_DocPageData::AddStandardFont(
    const char* font,
    const CPDF_FontEncoding* pEncoding) {
  ByteString name(font);
  if (!CFX_FontMapper::GetStandardFontName(&name))
    return nullptr;
  return GetStandardFont(name, pEncoding);
}

RetainPtr<CPDF_Font> CPDF_DocPageData::AddFont(std::unique_ptr<CFX_Font> pFont,
                                               int charset) {
  if (!pFont)
    return nullptr;

  const bool bCJK = FX_CharSetIsCJK(charset);
  ByteString basefont = pFont->GetFamilyName();
  basefont.Replace(" ", "");
  int flags =
      CalculateFlags(pFont->IsBold(), pFont->IsItalic(), pFont->IsFixedWidth(),
                     false, false, charset == FX_CHARSET_Symbol);

  CPDF_Dictionary* pBaseDict = GetDocument()->NewIndirect<CPDF_Dictionary>();
  pBaseDict->SetNewFor<CPDF_Name>("Type", "Font");
  auto pEncoding = pdfium::MakeUnique<CFX_UnicodeEncoding>(pFont.get());
  CPDF_Dictionary* pFontDict = pBaseDict;
  if (!bCJK) {
    auto pWidths = pdfium::MakeRetain<CPDF_Array>();
    for (int charcode = 32; charcode < 128; charcode++) {
      int glyph_index = pEncoding->GlyphFromCharCode(charcode);
      int char_width = pFont->GetGlyphWidth(glyph_index);
      pWidths->AddNew<CPDF_Number>(char_width);
    }
    if (charset == FX_CHARSET_ANSI || charset == FX_CHARSET_Default ||
        charset == FX_CHARSET_Symbol) {
      pBaseDict->SetNewFor<CPDF_Name>("Encoding", "WinAnsiEncoding");
      for (int charcode = 128; charcode <= 255; charcode++) {
        int glyph_index = pEncoding->GlyphFromCharCode(charcode);
        int char_width = pFont->GetGlyphWidth(glyph_index);
        pWidths->AddNew<CPDF_Number>(char_width);
      }
    } else {
      size_t i = CalculateEncodingDict(charset, pBaseDict);
      if (i < FX_ArraySize(g_FX_CharsetUnicodes)) {
        const uint16_t* pUnicodes = g_FX_CharsetUnicodes[i].m_pUnicodes;
        for (int j = 0; j < 128; j++) {
          int glyph_index = pEncoding->GlyphFromCharCode(pUnicodes[j]);
          int char_width = pFont->GetGlyphWidth(glyph_index);
          pWidths->AddNew<CPDF_Number>(char_width);
        }
      }
    }
    ProcessNonbCJK(pBaseDict, pFont->IsBold(), pFont->IsItalic(), basefont,
                   std::move(pWidths));
  } else {
    pFontDict = ProcessbCJK(
        pBaseDict, charset, basefont,
        [&pFont, &pEncoding](wchar_t start, wchar_t end, CPDF_Array* widthArr) {
          InsertWidthArray1(pFont.get(), pEncoding.get(), start, end, widthArr);
        });
  }
  int italicangle =
      pFont->GetSubstFont() ? pFont->GetSubstFont()->m_ItalicAngle : 0;
  FX_RECT bbox;
  pFont->GetBBox(&bbox);
  auto pBBox = pdfium::MakeRetain<CPDF_Array>();
  pBBox->AddNew<CPDF_Number>(bbox.left);
  pBBox->AddNew<CPDF_Number>(bbox.bottom);
  pBBox->AddNew<CPDF_Number>(bbox.right);
  pBBox->AddNew<CPDF_Number>(bbox.top);
  int32_t nStemV = 0;
  if (pFont->GetSubstFont()) {
    nStemV = pFont->GetSubstFont()->m_Weight / 5;
  } else {
    static const char stem_chars[] = {'i', 'I', '!', '1'};
    const size_t count = FX_ArraySize(stem_chars);
    uint32_t glyph = pEncoding->GlyphFromCharCode(stem_chars[0]);
    nStemV = pFont->GetGlyphWidth(glyph);
    for (size_t i = 1; i < count; i++) {
      glyph = pEncoding->GlyphFromCharCode(stem_chars[i]);
      int width = pFont->GetGlyphWidth(glyph);
      if (width > 0 && width < nStemV)
        nStemV = width;
    }
  }
  CPDF_Dictionary* pFontDesc =
      ToDictionary(GetDocument()->AddIndirectObject(CalculateFontDesc(
          GetDocument(), basefont, flags, italicangle, pFont->GetAscent(),
          pFont->GetDescent(), std::move(pBBox), nStemV)));
  pFontDict->SetNewFor<CPDF_Reference>("FontDescriptor", GetDocument(),
                                       pFontDesc->GetObjNum());
  return GetFont(pBaseDict);
}

#if defined(OS_WIN)
RetainPtr<CPDF_Font> CPDF_DocPageData::AddWindowsFont(LOGFONTA* pLogFont) {
  pLogFont->lfHeight = -1000;
  pLogFont->lfWidth = 0;
  HGDIOBJ hFont = CreateFontIndirectA(pLogFont);
  HDC hDC = CreateCompatibleDC(nullptr);
  hFont = SelectObject(hDC, hFont);
  int tm_size = GetOutlineTextMetrics(hDC, 0, nullptr);
  if (tm_size == 0) {
    hFont = SelectObject(hDC, hFont);
    DeleteObject(hFont);
    DeleteDC(hDC);
    return nullptr;
  }

  LPBYTE tm_buf = FX_Alloc(BYTE, tm_size);
  OUTLINETEXTMETRIC* ptm = reinterpret_cast<OUTLINETEXTMETRIC*>(tm_buf);
  GetOutlineTextMetrics(hDC, tm_size, ptm);
  int flags = CalculateFlags(false, pLogFont->lfItalic != 0,
                             (pLogFont->lfPitchAndFamily & 3) == FIXED_PITCH,
                             (pLogFont->lfPitchAndFamily & 0xf8) == FF_ROMAN,
                             (pLogFont->lfPitchAndFamily & 0xf8) == FF_SCRIPT,
                             pLogFont->lfCharSet == FX_CHARSET_Symbol);

  const bool bCJK = FX_CharSetIsCJK(pLogFont->lfCharSet);
  ByteString basefont;
  if (bCJK)
    basefont = FPDF_GetPSNameFromTT(hDC);

  if (basefont.IsEmpty())
    basefont = pLogFont->lfFaceName;

  int italicangle = ptm->otmItalicAngle / 10;
  int ascend = ptm->otmrcFontBox.top;
  int descend = ptm->otmrcFontBox.bottom;
  int capheight = ptm->otmsCapEmHeight;
  int bbox[4] = {ptm->otmrcFontBox.left, ptm->otmrcFontBox.bottom,
                 ptm->otmrcFontBox.right, ptm->otmrcFontBox.top};
  FX_Free(tm_buf);
  basefont.Replace(" ", "");
  CPDF_Dictionary* pBaseDict = GetDocument()->NewIndirect<CPDF_Dictionary>();
  pBaseDict->SetNewFor<CPDF_Name>("Type", "Font");
  CPDF_Dictionary* pFontDict = pBaseDict;
  if (!bCJK) {
    if (pLogFont->lfCharSet == FX_CHARSET_ANSI ||
        pLogFont->lfCharSet == FX_CHARSET_Default ||
        pLogFont->lfCharSet == FX_CHARSET_Symbol) {
      pBaseDict->SetNewFor<CPDF_Name>("Encoding", "WinAnsiEncoding");
    } else {
      CalculateEncodingDict(pLogFont->lfCharSet, pBaseDict);
    }
    int char_widths[224];
    GetCharWidth(hDC, 32, 255, char_widths);
    auto pWidths = pdfium::MakeRetain<CPDF_Array>();
    for (size_t i = 0; i < 224; i++)
      pWidths->AddNew<CPDF_Number>(char_widths[i]);
    ProcessNonbCJK(pBaseDict, pLogFont->lfWeight > FW_MEDIUM,
                   pLogFont->lfItalic != 0, basefont, std::move(pWidths));
  } else {
    pFontDict =
        ProcessbCJK(pBaseDict, pLogFont->lfCharSet, basefont,
                    [&hDC](wchar_t start, wchar_t end, CPDF_Array* widthArr) {
                      InsertWidthArray(hDC, start, end, widthArr);
                    });
  }
  auto pBBox = pdfium::MakeRetain<CPDF_Array>();
  for (int i = 0; i < 4; i++)
    pBBox->AddNew<CPDF_Number>(bbox[i]);
  RetainPtr<CPDF_Dictionary> pFontDesc =
      CalculateFontDesc(GetDocument(), basefont, flags, italicangle, ascend,
                        descend, std::move(pBBox), pLogFont->lfWeight / 5);
  pFontDesc->SetNewFor<CPDF_Number>("CapHeight", capheight);
  pFontDict->SetFor("FontDescriptor",
                    GetDocument()
                        ->AddIndirectObject(std::move(pFontDesc))
                        ->MakeReference(GetDocument()));
  hFont = SelectObject(hDC, hFont);
  DeleteObject(hFont);
  DeleteDC(hDC);
  return GetFont(pBaseDict);
}
#endif  //  defined(OS_WIN)

size_t CPDF_DocPageData::CalculateEncodingDict(int charset,
                                               CPDF_Dictionary* pBaseDict) {
  size_t i;
  for (i = 0; i < FX_ArraySize(g_FX_CharsetUnicodes); ++i) {
    if (g_FX_CharsetUnicodes[i].m_Charset == charset)
      break;
  }
  if (i == FX_ArraySize(g_FX_CharsetUnicodes))
    return i;

  CPDF_Dictionary* pEncodingDict =
      GetDocument()->NewIndirect<CPDF_Dictionary>();
  pEncodingDict->SetNewFor<CPDF_Name>("BaseEncoding", "WinAnsiEncoding");

  CPDF_Array* pArray = pEncodingDict->SetNewFor<CPDF_Array>("Differences");
  pArray->AddNew<CPDF_Number>(128);

  const uint16_t* pUnicodes = g_FX_CharsetUnicodes[i].m_pUnicodes;
  for (int j = 0; j < 128; j++) {
    ByteString name = PDF_AdobeNameFromUnicode(pUnicodes[j]);
    pArray->AddNew<CPDF_Name>(name.IsEmpty() ? ".notdef" : name);
  }
  pBaseDict->SetNewFor<CPDF_Reference>("Encoding", GetDocument(),
                                       pEncodingDict->GetObjNum());
  return i;
}

CPDF_Dictionary* CPDF_DocPageData::ProcessbCJK(
    CPDF_Dictionary* pBaseDict,
    int charset,
    ByteString basefont,
    std::function<void(wchar_t, wchar_t, CPDF_Array*)> Insert) {
  CPDF_Dictionary* pFontDict = GetDocument()->NewIndirect<CPDF_Dictionary>();
  ByteString cmap;
  ByteString ordering;
  int supplement = 0;
  CPDF_Array* pWidthArray = pFontDict->SetNewFor<CPDF_Array>("W");
  switch (charset) {
    case FX_CHARSET_ChineseTraditional:
      cmap = "ETenms-B5-H";
      ordering = "CNS1";
      supplement = 4;
      pWidthArray->AddNew<CPDF_Number>(1);
      Insert(0x20, 0x7e, pWidthArray);
      break;
    case FX_CHARSET_ChineseSimplified:
      cmap = "GBK-EUC-H";
      ordering = "GB1";
      supplement = 2;
      pWidthArray->AddNew<CPDF_Number>(7716);
      Insert(0x20, 0x20, pWidthArray);
      pWidthArray->AddNew<CPDF_Number>(814);
      Insert(0x21, 0x7e, pWidthArray);
      break;
    case FX_CHARSET_Hangul:
      cmap = "KSCms-UHC-H";
      ordering = "Korea1";
      supplement = 2;
      pWidthArray->AddNew<CPDF_Number>(1);
      Insert(0x20, 0x7e, pWidthArray);
      break;
    case FX_CHARSET_ShiftJIS:
      cmap = "90ms-RKSJ-H";
      ordering = "Japan1";
      supplement = 5;
      pWidthArray->AddNew<CPDF_Number>(231);
      Insert(0x20, 0x7d, pWidthArray);
      pWidthArray->AddNew<CPDF_Number>(326);
      Insert(0xa0, 0xa0, pWidthArray);
      pWidthArray->AddNew<CPDF_Number>(327);
      Insert(0xa1, 0xdf, pWidthArray);
      pWidthArray->AddNew<CPDF_Number>(631);
      Insert(0x7e, 0x7e, pWidthArray);
      break;
  }
  pBaseDict->SetNewFor<CPDF_Name>("Subtype", "Type0");
  pBaseDict->SetNewFor<CPDF_Name>("BaseFont", basefont);
  pBaseDict->SetNewFor<CPDF_Name>("Encoding", cmap);
  pFontDict->SetNewFor<CPDF_Name>("Type", "Font");
  pFontDict->SetNewFor<CPDF_Name>("Subtype", "CIDFontType2");
  pFontDict->SetNewFor<CPDF_Name>("BaseFont", basefont);

  CPDF_Dictionary* pCIDSysInfo =
      pFontDict->SetNewFor<CPDF_Dictionary>("CIDSystemInfo");
  pCIDSysInfo->SetNewFor<CPDF_String>("Registry", "Adobe", false);
  pCIDSysInfo->SetNewFor<CPDF_String>("Ordering", ordering, false);
  pCIDSysInfo->SetNewFor<CPDF_Number>("Supplement", supplement);

  CPDF_Array* pArray = pBaseDict->SetNewFor<CPDF_Array>("DescendantFonts");
  pArray->AddNew<CPDF_Reference>(GetDocument(), pFontDict->GetObjNum());
  return pFontDict;
}
