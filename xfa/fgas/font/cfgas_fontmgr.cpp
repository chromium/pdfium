// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/font/cfgas_fontmgr.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_font.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/fx_font.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/cxx17_backports.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "third_party/base/span.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/font/fgas_fontutils.h"

namespace {

bool VerifyUnicode(const RetainPtr<CFGAS_GEFont>& pFont, wchar_t wcUnicode) {
  RetainPtr<CFX_Face> pFace = pFont->GetDevFont()->GetFace();
  if (!pFace)
    return false;

  FXFT_FaceRec* pFaceRec = pFace->GetRec();
  FT_CharMap charmap = FXFT_Get_Face_Charmap(pFaceRec);
  if (FXFT_Select_Charmap(pFaceRec, FT_ENCODING_UNICODE) != 0)
    return false;

  if (FT_Get_Char_Index(pFaceRec, wcUnicode) == 0) {
    FT_Set_Charmap(pFaceRec, charmap);
    return false;
  }
  return true;
}

uint32_t ShortFormHash(FX_CodePage wCodePage,
                       uint32_t dwFontStyles,
                       WideStringView wsFontFamily) {
  ByteString bsHash = ByteString::Format("%d, %d", wCodePage, dwFontStyles);
  bsHash += FX_UTF8Encode(wsFontFamily);
  return FX_HashCode_GetA(bsHash.AsStringView());
}

uint32_t LongFormHash(FX_CodePage wCodePage,
                      uint16_t wBitField,
                      uint32_t dwFontStyles,
                      WideStringView wsFontFamily) {
  ByteString bsHash =
      ByteString::Format("%d, %d, %d", wCodePage, wBitField, dwFontStyles);
  bsHash += FX_UTF8Encode(wsFontFamily);
  return FX_HashCode_GetA(bsHash.AsStringView());
}

}  // namespace

#if BUILDFLAG(IS_WIN)

namespace {

struct FX_FONTMATCHPARAMS {
  const wchar_t* pwsFamily;
  uint32_t dwFontStyles;
  uint32_t dwUSB;
  bool matchParagraphStyle;
  wchar_t wUnicode;
  FX_CodePage wCodePage;
};

int32_t GetSimilarityScore(FX_FONTDESCRIPTOR const* pFont,
                           uint32_t dwFontStyles) {
  int32_t iValue = 0;
  if (FontStyleIsSymbolic(dwFontStyles) ==
      FontStyleIsSymbolic(pFont->dwFontStyles)) {
    iValue += 64;
  }
  if (FontStyleIsFixedPitch(dwFontStyles) ==
      FontStyleIsFixedPitch(pFont->dwFontStyles)) {
    iValue += 32;
  }
  if (FontStyleIsSerif(dwFontStyles) == FontStyleIsSerif(pFont->dwFontStyles))
    iValue += 16;
  if (FontStyleIsScript(dwFontStyles) == FontStyleIsScript(pFont->dwFontStyles))
    iValue += 8;
  return iValue;
}

const FX_FONTDESCRIPTOR* MatchDefaultFont(
    FX_FONTMATCHPARAMS* pParams,
    const std::deque<FX_FONTDESCRIPTOR>& fonts) {
  const FX_FONTDESCRIPTOR* pBestFont = nullptr;
  int32_t iBestSimilar = 0;
  for (const auto& font : fonts) {
    if (FontStyleIsForceBold(font.dwFontStyles) &&
        FontStyleIsItalic(font.dwFontStyles)) {
      continue;
    }

    if (pParams->pwsFamily) {
      if (FXSYS_wcsicmp(pParams->pwsFamily, font.wsFontFace))
        continue;
      if (font.uCharSet == FX_Charset::kSymbol)
        return &font;
    }
    if (font.uCharSet == FX_Charset::kSymbol)
      continue;
    if (pParams->wCodePage != FX_CodePage::kFailure) {
      if (FX_GetCodePageFromCharset(font.uCharSet) != pParams->wCodePage)
        continue;
    } else {
      if (pParams->dwUSB < 128) {
        uint32_t dwByte = pParams->dwUSB / 32;
        uint32_t dwUSB = 1 << (pParams->dwUSB % 32);
        if ((font.FontSignature.fsUsb[dwByte] & dwUSB) == 0)
          continue;
      }
    }
    if (pParams->matchParagraphStyle) {
      if ((font.dwFontStyles & 0x0F) == (pParams->dwFontStyles & 0x0F))
        return &font;
      continue;
    }
    if (pParams->pwsFamily) {
      if (FXSYS_wcsicmp(pParams->pwsFamily, font.wsFontFace) == 0)
        return &font;
    }
    int32_t iSimilarValue = GetSimilarityScore(&font, pParams->dwFontStyles);
    if (iBestSimilar < iSimilarValue) {
      iBestSimilar = iSimilarValue;
      pBestFont = &font;
    }
  }
  return iBestSimilar < 1 ? nullptr : pBestFont;
}

uint32_t GetGdiFontStyles(const LOGFONTW& lf) {
  uint32_t dwStyles = 0;
  if ((lf.lfPitchAndFamily & 0x03) == FIXED_PITCH)
    dwStyles |= FXFONT_FIXED_PITCH;
  uint8_t nFamilies = lf.lfPitchAndFamily & 0xF0;
  if (nFamilies == FF_ROMAN)
    dwStyles |= FXFONT_SERIF;
  if (nFamilies == FF_SCRIPT)
    dwStyles |= FXFONT_SCRIPT;
  if (lf.lfCharSet == SYMBOL_CHARSET)
    dwStyles |= FXFONT_SYMBOLIC;
  return dwStyles;
}

int32_t CALLBACK GdiFontEnumProc(ENUMLOGFONTEX* lpelfe,
                                 NEWTEXTMETRICEX* lpntme,
                                 DWORD dwFontType,
                                 LPARAM lParam) {
  if (dwFontType != TRUETYPE_FONTTYPE)
    return 1;
  const LOGFONTW& lf = ((LPENUMLOGFONTEXW)lpelfe)->elfLogFont;
  if (lf.lfFaceName[0] == L'@')
    return 1;
  FX_FONTDESCRIPTOR font;
  memset(&font, 0, sizeof(FX_FONTDESCRIPTOR));
  font.uCharSet = FX_GetCharsetFromInt(lf.lfCharSet);
  font.dwFontStyles = GetGdiFontStyles(lf);
  FXSYS_wcsncpy(font.wsFontFace, (const wchar_t*)lf.lfFaceName, 31);
  font.wsFontFace[31] = 0;
  memcpy(&font.FontSignature, &lpntme->ntmFontSig, sizeof(lpntme->ntmFontSig));
  reinterpret_cast<std::deque<FX_FONTDESCRIPTOR>*>(lParam)->push_back(font);
  return 1;
}

std::deque<FX_FONTDESCRIPTOR> EnumGdiFonts(const wchar_t* pwsFaceName,
                                           wchar_t wUnicode) {
  std::deque<FX_FONTDESCRIPTOR> fonts;
  LOGFONTW lfFind;
  memset(&lfFind, 0, sizeof(lfFind));
  lfFind.lfCharSet = DEFAULT_CHARSET;
  if (pwsFaceName) {
    FXSYS_wcsncpy(lfFind.lfFaceName, pwsFaceName, 31);
    lfFind.lfFaceName[31] = 0;
  }
  HDC hDC = ::GetDC(nullptr);
  EnumFontFamiliesExW(hDC, (LPLOGFONTW)&lfFind, (FONTENUMPROCW)GdiFontEnumProc,
                      (LPARAM)&fonts, 0);
  ::ReleaseDC(nullptr, hDC);
  return fonts;
}

}  // namespace

CFGAS_FontMgr::CFGAS_FontMgr() : m_FontFaces(EnumGdiFonts(nullptr, 0xFEFF)) {}

CFGAS_FontMgr::~CFGAS_FontMgr() = default;

bool CFGAS_FontMgr::EnumFonts() {
  return true;
}

RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::GetFontByUnicodeImpl(
    wchar_t wUnicode,
    uint32_t dwFontStyles,
    const wchar_t* pszFontFamily,
    uint32_t dwHash,
    FX_CodePage wCodePage,
    uint16_t wBitField) {
  const FX_FONTDESCRIPTOR* pFD = FindFont(pszFontFamily, dwFontStyles, false,
                                          wCodePage, wBitField, wUnicode);
  if (!pFD && pszFontFamily) {
    pFD =
        FindFont(nullptr, dwFontStyles, false, wCodePage, wBitField, wUnicode);
  }
  if (!pFD)
    return nullptr;

  FX_CodePage newCodePage = FX_GetCodePageFromCharset(pFD->uCharSet);
  RetainPtr<CFGAS_GEFont> pFont =
      CFGAS_GEFont::LoadFont(pFD->wsFontFace, dwFontStyles, newCodePage);
  if (!pFont)
    return nullptr;

  pFont->SetLogicalFontStyle(dwFontStyles);
  if (!VerifyUnicode(pFont, wUnicode)) {
    m_FailedUnicodesSet.insert(wUnicode);
    return nullptr;
  }

  m_Hash2Fonts[dwHash].push_back(pFont);
  return pFont;
}

const FX_FONTDESCRIPTOR* CFGAS_FontMgr::FindFont(const wchar_t* pszFontFamily,
                                                 uint32_t dwFontStyles,
                                                 bool matchParagraphStyle,
                                                 FX_CodePage wCodePage,
                                                 uint32_t dwUSB,
                                                 wchar_t wUnicode) {
  FX_FONTMATCHPARAMS params;
  memset(&params, 0, sizeof(params));
  params.dwUSB = dwUSB;
  params.wUnicode = wUnicode;
  params.wCodePage = wCodePage;
  params.pwsFamily = pszFontFamily;
  params.dwFontStyles = dwFontStyles;
  params.matchParagraphStyle = matchParagraphStyle;

  const FX_FONTDESCRIPTOR* pDesc = MatchDefaultFont(&params, m_FontFaces);
  if (pDesc)
    return pDesc;

  if (!pszFontFamily)
    return nullptr;

  // Use a named object to store the returned value of EnumGdiFonts() instead
  // of using a temporary object. This can prevent use-after-free issues since
  // pDesc may point to one of std::deque object's elements.
  std::deque<FX_FONTDESCRIPTOR> namedFonts =
      EnumGdiFonts(pszFontFamily, wUnicode);
  params.pwsFamily = nullptr;
  pDesc = MatchDefaultFont(&params, namedFonts);
  if (!pDesc)
    return nullptr;

  auto it = std::find(m_FontFaces.rbegin(), m_FontFaces.rend(), *pDesc);
  if (it != m_FontFaces.rend())
    return &*it;

  m_FontFaces.push_back(*pDesc);
  return &m_FontFaces.back();
}

#else  // BUILDFLAG(IS_WIN)

namespace {

const FX_CodePage kCodePages[] = {FX_CodePage::kMSWin_WesternEuropean,
                                  FX_CodePage::kMSWin_EasternEuropean,
                                  FX_CodePage::kMSWin_Cyrillic,
                                  FX_CodePage::kMSWin_Greek,
                                  FX_CodePage::kMSWin_Turkish,
                                  FX_CodePage::kMSWin_Hebrew,
                                  FX_CodePage::kMSWin_Arabic,
                                  FX_CodePage::kMSWin_Baltic,
                                  FX_CodePage::kMSWin_Vietnamese,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kMSDOS_Thai,
                                  FX_CodePage::kShiftJIS,
                                  FX_CodePage::kChineseSimplified,
                                  FX_CodePage::kHangul,
                                  FX_CodePage::kChineseTraditional,
                                  FX_CodePage::kJohab,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kDefANSI,
                                  FX_CodePage::kMSDOS_Greek2,
                                  FX_CodePage::kMSDOS_Russian,
                                  FX_CodePage::kMSDOS_Norwegian,
                                  FX_CodePage::kMSDOS_Arabic,
                                  FX_CodePage::kMSDOS_FrenchCanadian,
                                  FX_CodePage::kMSDOS_Hebrew,
                                  FX_CodePage::kMSDOS_Icelandic,
                                  FX_CodePage::kMSDOS_Portuguese,
                                  FX_CodePage::kMSDOS_Turkish,
                                  FX_CodePage::kMSDOS_Cyrillic,
                                  FX_CodePage::kMSDOS_EasternEuropean,
                                  FX_CodePage::kMSDOS_Baltic,
                                  FX_CodePage::kMSDOS_Greek1,
                                  FX_CodePage::kArabic_ASMO708,
                                  FX_CodePage::kMSDOS_WesternEuropean,
                                  FX_CodePage::kMSDOS_US};

uint16_t FX_GetCodePageBit(FX_CodePage wCodePage) {
  for (size_t i = 0; i < pdfium::size(kCodePages); ++i) {
    if (kCodePages[i] == wCodePage)
      return static_cast<uint16_t>(i);
  }
  return static_cast<uint16_t>(-1);
}

uint16_t FX_GetUnicodeBit(wchar_t wcUnicode) {
  const FGAS_FONTUSB* x = FGAS_GetUnicodeBitField(wcUnicode);
  return x ? x->wBitField : FGAS_FONTUSB::kNoBitField;
}

uint16_t ReadUInt16FromSpanAtOffset(pdfium::span<const uint8_t> data,
                                    size_t offset) {
  const uint8_t* p = &data[offset];
  return FXSYS_UINT16_GET_MSBFIRST(p);
}

extern "C" {

unsigned long ftStreamRead(FXFT_StreamRec* stream,
                           unsigned long offset,
                           unsigned char* buffer,
                           unsigned long count) {
  if (count == 0)
    return 0;

  IFX_SeekableReadStream* pFile =
      static_cast<IFX_SeekableReadStream*>(stream->descriptor.pointer);
  if (!pFile->ReadBlockAtOffset(buffer, offset, count))
    return 0;

  return count;
}

void ftStreamClose(FXFT_StreamRec* stream) {}

}  // extern "C"

std::vector<WideString> GetNames(pdfium::span<const uint8_t> name_table) {
  std::vector<WideString> results;
  if (name_table.empty())
    return results;

  uint16_t nNameCount = ReadUInt16FromSpanAtOffset(name_table, 2);
  pdfium::span<const uint8_t> str =
      name_table.subspan(ReadUInt16FromSpanAtOffset(name_table, 4));
  pdfium::span<const uint8_t> name_record = name_table.subspan(6);
  for (uint16_t i = 0; i < nNameCount; ++i) {
    uint16_t nNameID = ReadUInt16FromSpanAtOffset(name_table, i * 12 + 6);
    if (nNameID != 1)
      continue;

    uint16_t nPlatformID = ReadUInt16FromSpanAtOffset(name_record, i * 12);
    uint16_t nNameLength = ReadUInt16FromSpanAtOffset(name_record, i * 12 + 8);
    uint16_t nNameOffset = ReadUInt16FromSpanAtOffset(name_record, i * 12 + 10);
    if (nPlatformID != 1) {
      WideString wsFamily;
      for (uint16_t j = 0; j < nNameLength / 2; ++j) {
        wchar_t wcTemp = ReadUInt16FromSpanAtOffset(str, nNameOffset + j * 2);
        wsFamily += wcTemp;
      }
      results.push_back(wsFamily);
      continue;
    }

    WideString wsFamily;
    for (uint16_t j = 0; j < nNameLength; ++j) {
      wchar_t wcTemp = str[nNameOffset + j];
      wsFamily += wcTemp;
    }
    results.push_back(wsFamily);
  }
  return results;
}

void GetUSBCSB(FXFT_FaceRec* pFace, uint32_t* USB, uint32_t* CSB) {
  TT_OS2* pOS2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(pFace, ft_sfnt_os2));
  if (!pOS2) {
    USB[0] = 0;
    USB[1] = 0;
    USB[2] = 0;
    USB[3] = 0;
    CSB[0] = 0;
    CSB[1] = 0;
    return;
  }
  USB[0] = static_cast<uint32_t>(pOS2->ulUnicodeRange1);
  USB[1] = static_cast<uint32_t>(pOS2->ulUnicodeRange2);
  USB[2] = static_cast<uint32_t>(pOS2->ulUnicodeRange3);
  USB[3] = static_cast<uint32_t>(pOS2->ulUnicodeRange4);
  CSB[0] = static_cast<uint32_t>(pOS2->ulCodePageRange1);
  CSB[1] = static_cast<uint32_t>(pOS2->ulCodePageRange2);
}

uint32_t GetFlags(FXFT_FaceRec* pFace) {
  uint32_t flags = 0;
  if (FXFT_Is_Face_Bold(pFace))
    flags |= FXFONT_FORCE_BOLD;
  if (FXFT_Is_Face_Italic(pFace))
    flags |= FXFONT_ITALIC;
  if (FT_IS_FIXED_WIDTH(pFace))
    flags |= FXFONT_FIXED_PITCH;

  TT_OS2* pOS2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(pFace, ft_sfnt_os2));
  if (!pOS2)
    return flags;

  if (pOS2->ulCodePageRange1 & (1 << 31))
    flags |= FXFONT_SYMBOLIC;
  if (pOS2->panose[0] == 2) {
    uint8_t uSerif = pOS2->panose[1];
    if ((uSerif > 1 && uSerif < 10) || uSerif > 13)
      flags |= FXFONT_SERIF;
  }
  return flags;
}

RetainPtr<IFX_SeekableReadStream> CreateFontStream(CFX_FontMapper* pFontMapper,
                                                   size_t index) {
  size_t dwFileSize = 0;
  std::unique_ptr<uint8_t, FxFreeDeleter> pBuffer =
      pFontMapper->RawBytesForIndex(index, &dwFileSize);
  if (!pBuffer)
    return nullptr;

  return pdfium::MakeRetain<CFX_MemoryStream>(std::move(pBuffer), dwFileSize);
}

RetainPtr<IFX_SeekableReadStream> CreateFontStream(
    const ByteString& bsFaceName) {
  CFX_FontMgr* pFontMgr = CFX_GEModule::Get()->GetFontMgr();
  CFX_FontMapper* pFontMapper = pFontMgr->GetBuiltinMapper();
  pFontMapper->LoadInstalledFonts();

  for (size_t i = 0; i < pFontMapper->GetFaceSize(); ++i) {
    if (pFontMapper->GetFaceName(i) == bsFaceName)
      return CreateFontStream(pFontMapper, i);
  }
  return nullptr;
}

RetainPtr<CFX_Face> LoadFace(
    const RetainPtr<IFX_SeekableReadStream>& pFontStream,
    int32_t iFaceIndex) {
  if (!pFontStream)
    return nullptr;

  CFX_FontMgr* pFontMgr = CFX_GEModule::Get()->GetFontMgr();
  FXFT_LibraryRec* library = pFontMgr->GetFTLibrary();
  if (!library)
    return nullptr;

  // TODO(palmer): This memory will be freed with |ft_free| (which is |free|).
  // Ultimately, we want to change this to:
  //   FXFT_Stream ftStream = FX_Alloc(FXFT_StreamRec, 1);
  // https://bugs.chromium.org/p/pdfium/issues/detail?id=690
  FXFT_StreamRec* ftStream =
      static_cast<FXFT_StreamRec*>(ft_scalloc(sizeof(FXFT_StreamRec), 1));
  memset(ftStream, 0, sizeof(FXFT_StreamRec));
  ftStream->base = nullptr;
  ftStream->descriptor.pointer = static_cast<void*>(pFontStream.Get());
  ftStream->pos = 0;
  ftStream->size = static_cast<unsigned long>(pFontStream->GetSize());
  ftStream->read = ftStreamRead;
  ftStream->close = ftStreamClose;

  FT_Open_Args ftArgs;
  memset(&ftArgs, 0, sizeof(FT_Open_Args));
  ftArgs.flags |= FT_OPEN_STREAM;
  ftArgs.stream = ftStream;

  RetainPtr<CFX_Face> pFace = CFX_Face::Open(library, &ftArgs, iFaceIndex);
  if (!pFace) {
    ft_sfree(ftStream);
    return nullptr;
  }
  FT_Set_Pixel_Sizes(pFace->GetRec(), 0, 64);
  return pFace;
}

bool VerifyUnicodeForFontDescriptor(CFGAS_FontDescriptor* pDesc,
                                    wchar_t wcUnicode) {
  RetainPtr<IFX_SeekableReadStream> pFileRead =
      CreateFontStream(pDesc->m_wsFaceName.ToUTF8());
  if (!pFileRead)
    return false;

  RetainPtr<CFX_Face> pFace = LoadFace(pFileRead, pDesc->m_nFaceIndex);
  if (!pFace)
    return false;

  FT_Error retCharmap =
      FXFT_Select_Charmap(pFace->GetRec(), FT_ENCODING_UNICODE);
  FT_Error retIndex = FT_Get_Char_Index(pFace->GetRec(), wcUnicode);

  if (FXFT_Get_Face_External_Stream(pFace->GetRec()))
    FXFT_Clear_Face_External_Stream(pFace->GetRec());

  return !retCharmap && retIndex;
}

bool IsPartName(const WideString& name1, const WideString& name2) {
  return name1.Contains(name2.AsStringView());
}

int32_t CalcPenalty(CFGAS_FontDescriptor* pInstalled,
                    FX_CodePage wCodePage,
                    uint32_t dwFontStyles,
                    const WideString& FontName,
                    wchar_t wcUnicode) {
  int32_t nPenalty = 30000;
  if (FontName.GetLength() != 0) {
    if (FontName != pInstalled->m_wsFaceName) {
      size_t i;
      for (i = 0; i < pInstalled->m_wsFamilyNames.size(); ++i) {
        if (pInstalled->m_wsFamilyNames[i] == FontName)
          break;
      }
      if (i == pInstalled->m_wsFamilyNames.size())
        nPenalty += 0xFFFF;
      else
        nPenalty -= 28000;
    } else {
      nPenalty -= 30000;
    }
    if (nPenalty == 30000 && !IsPartName(pInstalled->m_wsFaceName, FontName)) {
      size_t i;
      for (i = 0; i < pInstalled->m_wsFamilyNames.size(); i++) {
        if (IsPartName(pInstalled->m_wsFamilyNames[i], FontName))
          break;
      }
      if (i == pInstalled->m_wsFamilyNames.size())
        nPenalty += 0xFFFF;
      else
        nPenalty -= 26000;
    } else {
      nPenalty -= 27000;
    }
  }
  uint32_t dwStyleMask = pInstalled->m_dwFontStyles ^ dwFontStyles;
  if (FontStyleIsForceBold(dwStyleMask))
    nPenalty += 4500;
  if (FontStyleIsFixedPitch(dwStyleMask))
    nPenalty += 10000;
  if (FontStyleIsItalic(dwStyleMask))
    nPenalty += 10000;
  if (FontStyleIsSerif(dwStyleMask))
    nPenalty += 500;
  if (FontStyleIsSymbolic(dwStyleMask))
    nPenalty += 0xFFFF;
  if (nPenalty >= 0xFFFF)
    return 0xFFFF;

  uint16_t wBit =
      (wCodePage == FX_CodePage::kDefANSI || wCodePage == FX_CodePage::kFailure)
          ? static_cast<uint16_t>(-1)
          : FX_GetCodePageBit(wCodePage);
  if (wBit != static_cast<uint16_t>(-1)) {
    DCHECK(wBit < 64);
    if ((pInstalled->m_dwCsb[wBit / 32] & (1 << (wBit % 32))) == 0)
      nPenalty += 0xFFFF;
    else
      nPenalty -= 60000;
  }
  wBit = (wcUnicode == 0 || wcUnicode == 0xFFFE) ? FGAS_FONTUSB::kNoBitField
                                                 : FX_GetUnicodeBit(wcUnicode);
  if (wBit != FGAS_FONTUSB::kNoBitField) {
    DCHECK(wBit < 128);
    if ((pInstalled->m_dwUsb[wBit / 32] & (1 << (wBit % 32))) == 0)
      nPenalty += 0xFFFF;
    else
      nPenalty -= 60000;
  }
  return nPenalty;
}

}  // namespace

CFGAS_FontDescriptor::CFGAS_FontDescriptor() = default;

CFGAS_FontDescriptor::~CFGAS_FontDescriptor() = default;

CFGAS_FontMgr::CFGAS_FontMgr() = default;

CFGAS_FontMgr::~CFGAS_FontMgr() = default;

bool CFGAS_FontMgr::EnumFontsFromFontMapper() {
  CFX_FontMapper* pFontMapper =
      CFX_GEModule::Get()->GetFontMgr()->GetBuiltinMapper();
  pFontMapper->LoadInstalledFonts();

  for (size_t i = 0; i < pFontMapper->GetFaceSize(); ++i) {
    RetainPtr<IFX_SeekableReadStream> pFontStream =
        CreateFontStream(pFontMapper, i);
    if (!pFontStream)
      continue;

    WideString wsFaceName =
        WideString::FromDefANSI(pFontMapper->GetFaceName(i).AsStringView());
    RegisterFaces(pFontStream, &wsFaceName);
  }

  return !m_InstalledFonts.empty();
}

bool CFGAS_FontMgr::EnumFonts() {
  return EnumFontsFromFontMapper();
}

RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::GetFontByUnicodeImpl(
    wchar_t wUnicode,
    uint32_t dwFontStyles,
    const wchar_t* pszFontFamily,
    uint32_t dwHash,
    FX_CodePage wCodePage,
    uint16_t /* wBitField*/) {
  if (!pdfium::Contains(m_Hash2CandidateList, dwHash)) {
    m_Hash2CandidateList[dwHash] =
        MatchFonts(wCodePage, dwFontStyles, pszFontFamily, wUnicode);
  }
  for (const auto& info : m_Hash2CandidateList[dwHash]) {
    CFGAS_FontDescriptor* pDesc = info.pFont;
    if (!VerifyUnicodeForFontDescriptor(pDesc, wUnicode))
      continue;
    RetainPtr<CFGAS_GEFont> pFont =
        LoadFontInternal(pDesc->m_wsFaceName, pDesc->m_nFaceIndex);
    if (!pFont)
      continue;
    pFont->SetLogicalFontStyle(dwFontStyles);
    m_Hash2Fonts[dwHash].push_back(pFont);
    return pFont;
  }
  if (!pszFontFamily)
    m_FailedUnicodesSet.insert(wUnicode);
  return nullptr;
}

RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::LoadFontInternal(
    const WideString& wsFaceName,
    int32_t iFaceIndex) {
  RetainPtr<IFX_SeekableReadStream> pFontStream =
      CreateFontStream(wsFaceName.ToUTF8());
  if (!pFontStream)
    return nullptr;

  auto pInternalFont = std::make_unique<CFX_Font>();
  if (!pInternalFont->LoadFile(std::move(pFontStream), iFaceIndex))
    return nullptr;

  return CFGAS_GEFont::LoadFont(std::move(pInternalFont));
}

std::vector<CFGAS_FontDescriptorInfo> CFGAS_FontMgr::MatchFonts(
    FX_CodePage wCodePage,
    uint32_t dwFontStyles,
    const WideString& FontName,
    wchar_t wcUnicode) {
  std::vector<CFGAS_FontDescriptorInfo> matched_fonts;
  for (const auto& pFont : m_InstalledFonts) {
    int32_t nPenalty =
        CalcPenalty(pFont.get(), wCodePage, dwFontStyles, FontName, wcUnicode);
    if (nPenalty >= 0xffff)
      continue;
    matched_fonts.push_back({pFont.get(), nPenalty});
    if (matched_fonts.size() == 0xffff)
      break;
  }
  std::sort(matched_fonts.begin(), matched_fonts.end());
  return matched_fonts;
}

void CFGAS_FontMgr::RegisterFace(RetainPtr<CFX_Face> pFace,
                                 const WideString* pFaceName) {
  if ((pFace->GetRec()->face_flags & FT_FACE_FLAG_SCALABLE) == 0)
    return;

  auto pFont = std::make_unique<CFGAS_FontDescriptor>();
  pFont->m_dwFontStyles |= GetFlags(pFace->GetRec());

  GetUSBCSB(pFace->GetRec(), pFont->m_dwUsb, pFont->m_dwCsb);

  FT_ULong dwTag;
  FT_ENC_TAG(dwTag, 'n', 'a', 'm', 'e');

  std::vector<uint8_t, FxAllocAllocator<uint8_t>> table;
  unsigned long nLength = 0;
  unsigned int error =
      FT_Load_Sfnt_Table(pFace->GetRec(), dwTag, 0, nullptr, &nLength);
  if (error == 0 && nLength != 0) {
    table.resize(nLength);
    if (FT_Load_Sfnt_Table(pFace->GetRec(), dwTag, 0, table.data(), nullptr))
      table.clear();
  }
  pFont->m_wsFamilyNames = GetNames(table);
  pFont->m_wsFamilyNames.push_back(
      WideString::FromUTF8(pFace->GetRec()->family_name));
  pFont->m_wsFaceName =
      pFaceName
          ? *pFaceName
          : WideString::FromDefANSI(FT_Get_Postscript_Name(pFace->GetRec()));
  pFont->m_nFaceIndex =
      pdfium::base::checked_cast<int32_t>(pFace->GetRec()->face_index);
  m_InstalledFonts.push_back(std::move(pFont));
}

void CFGAS_FontMgr::RegisterFaces(
    const RetainPtr<IFX_SeekableReadStream>& pFontStream,
    const WideString* pFaceName) {
  int32_t index = 0;
  int32_t num_faces = 0;
  do {
    RetainPtr<CFX_Face> pFace = LoadFace(pFontStream, index++);
    if (!pFace)
      continue;
    // All faces keep number of faces. It can be retrieved from any one face.
    if (num_faces == 0) {
      num_faces =
          pdfium::base::checked_cast<int32_t>(pFace->GetRec()->num_faces);
    }
    RegisterFace(pFace, pFaceName);
    if (FXFT_Get_Face_External_Stream(pFace->GetRec()))
      FXFT_Clear_Face_External_Stream(pFace->GetRec());
  } while (index < num_faces);
}

#endif  // BUILDFLAG(IS_WIN)

RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::GetFontByCodePage(
    FX_CodePage wCodePage,
    uint32_t dwFontStyles,
    const wchar_t* pszFontFamily) {
  uint32_t dwHash = ShortFormHash(wCodePage, dwFontStyles, pszFontFamily);
  auto* pFontVector = &m_Hash2Fonts[dwHash];
  if (!pFontVector->empty()) {
    for (auto iter = pFontVector->begin(); iter != pFontVector->end(); ++iter) {
      if (*iter != nullptr)
        return *iter;
    }
    return nullptr;
  }

#if BUILDFLAG(IS_WIN)
  const FX_FONTDESCRIPTOR* pFD =
      FindFont(pszFontFamily, dwFontStyles, true, wCodePage,
               FGAS_FONTUSB::kNoBitField, 0);
  if (!pFD) {
    pFD = FindFont(nullptr, dwFontStyles, true, wCodePage,
                   FGAS_FONTUSB::kNoBitField, 0);
  }
  if (!pFD) {
    pFD = FindFont(nullptr, dwFontStyles, false, wCodePage,
                   FGAS_FONTUSB::kNoBitField, 0);
  }
  if (!pFD)
    return nullptr;

  RetainPtr<CFGAS_GEFont> pFont =
      CFGAS_GEFont::LoadFont(pFD->wsFontFace, dwFontStyles, wCodePage);
#else   // BUILDFLAG(IS_WIN)
  if (!pdfium::Contains(m_Hash2CandidateList, dwHash)) {
    m_Hash2CandidateList[dwHash] =
        MatchFonts(wCodePage, dwFontStyles, WideString(pszFontFamily), 0);
  }
  if (m_Hash2CandidateList[dwHash].empty())
    return nullptr;

  CFGAS_FontDescriptor* pDesc = m_Hash2CandidateList[dwHash].front().pFont;
  RetainPtr<CFGAS_GEFont> pFont =
      LoadFontInternal(pDesc->m_wsFaceName, pDesc->m_nFaceIndex);
#endif  // BUILDFLAG(IS_WIN)

  if (!pFont)
    return nullptr;

  pFont->SetLogicalFontStyle(dwFontStyles);
  pFontVector->push_back(pFont);
  return pFont;
}

RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::GetFontByUnicode(
    wchar_t wUnicode,
    uint32_t dwFontStyles,
    const wchar_t* pszFontFamily) {
  if (pdfium::Contains(m_FailedUnicodesSet, wUnicode))
    return nullptr;

  const FGAS_FONTUSB* x = FGAS_GetUnicodeBitField(wUnicode);
  FX_CodePage wCodePage = x ? x->wCodePage : FX_CodePage::kFailure;
  uint16_t wBitField = x ? x->wBitField : FGAS_FONTUSB::kNoBitField;
  uint32_t dwHash =
      wCodePage == FX_CodePage::kFailure
          ? LongFormHash(wCodePage, wBitField, dwFontStyles, pszFontFamily)
          : ShortFormHash(wCodePage, dwFontStyles, pszFontFamily);
  for (auto& pFont : m_Hash2Fonts[dwHash]) {
    if (VerifyUnicode(pFont, wUnicode))
      return pFont;
  }
  return GetFontByUnicodeImpl(wUnicode, dwFontStyles, pszFontFamily, dwHash,
                              wCodePage, wBitField);
}

RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::LoadFont(const wchar_t* pszFontFamily,
                                                uint32_t dwFontStyles,
                                                FX_CodePage wCodePage) {
#if BUILDFLAG(IS_WIN)
  uint32_t dwHash = ShortFormHash(wCodePage, dwFontStyles, pszFontFamily);
  std::vector<RetainPtr<CFGAS_GEFont>>* pFontArray = &m_Hash2Fonts[dwHash];
  if (!pFontArray->empty())
    return pFontArray->front();

  const FX_FONTDESCRIPTOR* pFD =
      FindFont(pszFontFamily, dwFontStyles, true, wCodePage,
               FGAS_FONTUSB::kNoBitField, 0);
  if (!pFD) {
    pFD = FindFont(pszFontFamily, dwFontStyles, false, wCodePage,
                   FGAS_FONTUSB::kNoBitField, 0);
  }
  if (!pFD)
    return nullptr;

  RetainPtr<CFGAS_GEFont> pFont =
      CFGAS_GEFont::LoadFont(pFD->wsFontFace, dwFontStyles, wCodePage);
  if (!pFont)
    return nullptr;

  pFont->SetLogicalFontStyle(dwFontStyles);
  pFontArray->push_back(pFont);
  return pFont;
#else   // BUILDFLAG(IS_WIN)
  return GetFontByCodePage(wCodePage, dwFontStyles, pszFontFamily);
#endif  // BUILDFLAG(IS_WIN)
}
