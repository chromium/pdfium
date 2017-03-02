// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/font/cfgas_fontmgr.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxcrt/fx_stream.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/cfx_fontmgr.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/ifx_systemfontinfo.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fgas/crt/fgas_codepage.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fgas/font/fgas_fontutils.h"

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

namespace {

int32_t GetSimilarityScore(FX_FONTDESCRIPTOR const* pFont,
                           uint32_t dwFontStyles) {
  int32_t iValue = 0;
  if ((dwFontStyles & FX_FONTSTYLE_Symbolic) ==
      (pFont->dwFontStyles & FX_FONTSTYLE_Symbolic)) {
    iValue += 64;
  }
  if ((dwFontStyles & FX_FONTSTYLE_FixedPitch) ==
      (pFont->dwFontStyles & FX_FONTSTYLE_FixedPitch)) {
    iValue += 32;
  }
  if ((dwFontStyles & FX_FONTSTYLE_Serif) ==
      (pFont->dwFontStyles & FX_FONTSTYLE_Serif)) {
    iValue += 16;
  }
  if ((dwFontStyles & FX_FONTSTYLE_Script) ==
      (pFont->dwFontStyles & FX_FONTSTYLE_Script)) {
    iValue += 8;
  }
  return iValue;
}

const FX_FONTDESCRIPTOR* MatchDefaultFont(
    FX_FONTMATCHPARAMS* pParams,
    const std::deque<FX_FONTDESCRIPTOR>& fonts) {
  const FX_FONTDESCRIPTOR* pBestFont = nullptr;
  int32_t iBestSimilar = 0;
  bool bMatchStyle = (pParams->dwMatchFlags & FX_FONTMATCHPARA_MatchStyle) > 0;
  for (const auto& font : fonts) {
    if ((font.dwFontStyles & FX_FONTSTYLE_BoldItalic) ==
        FX_FONTSTYLE_BoldItalic) {
      continue;
    }
    if (pParams->pwsFamily) {
      if (FXSYS_wcsicmp(pParams->pwsFamily, font.wsFontFace))
        continue;
      if (font.uCharSet == FX_CHARSET_Symbol)
        return &font;
    }
    if (font.uCharSet == FX_CHARSET_Symbol)
      continue;
    if (pParams->wCodePage != 0xFFFF) {
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
    if (bMatchStyle) {
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

}  // namespace

std::unique_ptr<CFGAS_FontMgr> CFGAS_FontMgr::Create(
    FX_LPEnumAllFonts pEnumerator) {
  return pdfium::MakeUnique<CFGAS_FontMgr>(pEnumerator);
}

CFGAS_FontMgr::CFGAS_FontMgr(FX_LPEnumAllFonts pEnumerator)
    : m_pEnumerator(pEnumerator), m_FontFaces(100) {
  if (m_pEnumerator)
    m_pEnumerator(&m_FontFaces, nullptr, 0xFEFF);
}

CFGAS_FontMgr::~CFGAS_FontMgr() {}

CFX_RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::GetFontByCodePage(
    uint16_t wCodePage,
    uint32_t dwFontStyles,
    const FX_WCHAR* pszFontFamily) {
  uint32_t dwHash = FGAS_GetFontHashCode(wCodePage, dwFontStyles);
  auto it = m_CPFonts.find(dwHash);
  if (it != m_CPFonts.end()) {
    return it->second ? LoadFont(it->second, dwFontStyles, wCodePage) : nullptr;
  }
  const FX_FONTDESCRIPTOR* pFD =
      FindFont(pszFontFamily, dwFontStyles, true, wCodePage);
  if (!pFD)
    pFD = FindFont(nullptr, dwFontStyles, true, wCodePage);
  if (!pFD)
    pFD = FindFont(nullptr, dwFontStyles, false, wCodePage);
  if (!pFD)
    return nullptr;

  CFX_RetainPtr<CFGAS_GEFont> pFont =
      CFGAS_GEFont::LoadFont(pFD->wsFontFace, dwFontStyles, wCodePage, this);
  if (!pFont)
    return nullptr;

  m_Fonts.push_back(pFont);
  m_CPFonts[dwHash] = pFont;
  dwHash = FGAS_GetFontFamilyHash(pFD->wsFontFace, dwFontStyles, wCodePage);
  m_FamilyFonts[dwHash] = pFont;
  return LoadFont(pFont, dwFontStyles, wCodePage);
}

CFX_RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::GetFontByUnicode(
    FX_WCHAR wUnicode,
    uint32_t dwFontStyles,
    const FX_WCHAR* pszFontFamily) {
  const FGAS_FONTUSB* pRet = FGAS_GetUnicodeBitField(wUnicode);
  if (pRet->wBitField == 999)
    return nullptr;

  uint32_t dwHash =
      FGAS_GetFontFamilyHash(pszFontFamily, dwFontStyles, pRet->wBitField);
  auto it = m_UnicodeFonts.find(dwHash);
  if (it != m_UnicodeFonts.end()) {
    return it->second ? LoadFont(it->second, dwFontStyles, pRet->wCodePage)
                      : nullptr;
  }
  const FX_FONTDESCRIPTOR* pFD =
      FindFont(pszFontFamily, dwFontStyles, false, pRet->wCodePage,
               pRet->wBitField, wUnicode);
  if (!pFD && pszFontFamily) {
    pFD = FindFont(nullptr, dwFontStyles, false, pRet->wCodePage,
                   pRet->wBitField, wUnicode);
  }
  if (!pFD)
    return nullptr;

  uint16_t wCodePage = FX_GetCodePageFromCharset(pFD->uCharSet);
  const FX_WCHAR* pFontFace = pFD->wsFontFace;
  CFX_RetainPtr<CFGAS_GEFont> pFont =
      CFGAS_GEFont::LoadFont(pFontFace, dwFontStyles, wCodePage, this);
  if (!pFont)
    return nullptr;

  m_Fonts.push_back(pFont);
  m_UnicodeFonts[dwHash] = pFont;
  m_CPFonts[FGAS_GetFontHashCode(wCodePage, dwFontStyles)] = pFont;
  m_FamilyFonts[FGAS_GetFontFamilyHash(pFontFace, dwFontStyles, wCodePage)] =
      pFont;
  return LoadFont(pFont, dwFontStyles, wCodePage);
}

CFX_RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::LoadFont(
    const FX_WCHAR* pszFontFamily,
    uint32_t dwFontStyles,
    uint16_t wCodePage) {
  CFX_RetainPtr<CFGAS_GEFont> pFont;
  uint32_t dwHash =
      FGAS_GetFontFamilyHash(pszFontFamily, dwFontStyles, wCodePage);
  auto it = m_FamilyFonts.find(dwHash);
  if (it != m_FamilyFonts.end())
    return it->second ? LoadFont(it->second, dwFontStyles, wCodePage) : nullptr;

  const FX_FONTDESCRIPTOR* pFD =
      FindFont(pszFontFamily, dwFontStyles, true, wCodePage);
  if (!pFD)
    pFD = FindFont(pszFontFamily, dwFontStyles, false, wCodePage);
  if (!pFD)
    return nullptr;

  if (wCodePage == 0xFFFF)
    wCodePage = FX_GetCodePageFromCharset(pFD->uCharSet);

  pFont =
      CFGAS_GEFont::LoadFont(pFD->wsFontFace, dwFontStyles, wCodePage, this);
  if (!pFont)
    return nullptr;

  m_Fonts.push_back(pFont);
  m_FamilyFonts[dwHash] = pFont;
  dwHash = FGAS_GetFontHashCode(wCodePage, dwFontStyles);
  m_CPFonts[dwHash] = pFont;
  return LoadFont(pFont, dwFontStyles, wCodePage);
}

CFX_RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::LoadFont(
    const CFX_RetainPtr<CFGAS_GEFont>& pSrcFont,
    uint32_t dwFontStyles,
    uint16_t wCodePage) {
  if (pSrcFont->GetFontStyles() == dwFontStyles)
    return pSrcFont;

  void* buffer[3] = {pSrcFont.Get(), (void*)(uintptr_t)dwFontStyles,
                     (void*)(uintptr_t)wCodePage};
  uint32_t dwHash = FX_HashCode_GetA(
      CFX_ByteStringC((uint8_t*)buffer, sizeof(buffer)), false);
  auto it = m_DeriveFonts.find(dwHash);
  if (it != m_DeriveFonts.end() && it->second)
    return it->second;

  CFX_RetainPtr<CFGAS_GEFont> pFont = pSrcFont->Derive(dwFontStyles, wCodePage);
  if (!pFont)
    return nullptr;

  m_DeriveFonts[dwHash] = pFont;
  auto iter = std::find(m_Fonts.begin(), m_Fonts.end(), pFont);
  if (iter == m_Fonts.end())
    m_Fonts.push_back(pFont);
  return pFont;
}

void CFGAS_FontMgr::RemoveFont(
    std::map<uint32_t, CFX_RetainPtr<CFGAS_GEFont>>* pFontMap,
    const CFX_RetainPtr<CFGAS_GEFont>& pFont) {
  auto iter = pFontMap->begin();
  while (iter != pFontMap->end()) {
    auto old_iter = iter++;
    if (old_iter->second == pFont)
      pFontMap->erase(old_iter);
  }
}

void CFGAS_FontMgr::RemoveFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont) {
  RemoveFont(&m_CPFonts, pFont);
  RemoveFont(&m_FamilyFonts, pFont);
  RemoveFont(&m_UnicodeFonts, pFont);
  RemoveFont(&m_BufferFonts, pFont);
  RemoveFont(&m_StreamFonts, pFont);
  RemoveFont(&m_DeriveFonts, pFont);
  auto it = std::find(m_Fonts.begin(), m_Fonts.end(), pFont);
  if (it != m_Fonts.end())
    m_Fonts.erase(it);
}

const FX_FONTDESCRIPTOR* CFGAS_FontMgr::FindFont(const FX_WCHAR* pszFontFamily,
                                                 uint32_t dwFontStyles,
                                                 uint32_t dwMatchFlags,
                                                 uint16_t wCodePage,
                                                 uint32_t dwUSB,
                                                 FX_WCHAR wUnicode) {
  FX_FONTMATCHPARAMS params;
  FXSYS_memset(&params, 0, sizeof(params));
  params.dwUSB = dwUSB;
  params.wUnicode = wUnicode;
  params.wCodePage = wCodePage;
  params.pwsFamily = pszFontFamily;
  params.dwFontStyles = dwFontStyles;
  params.dwMatchFlags = dwMatchFlags;
  const FX_FONTDESCRIPTOR* pDesc = MatchDefaultFont(&params, m_FontFaces);
  if (pDesc)
    return pDesc;

  if (!pszFontFamily || !m_pEnumerator)
    return nullptr;

  std::deque<FX_FONTDESCRIPTOR> namedFonts;
  m_pEnumerator(&namedFonts, pszFontFamily, wUnicode);
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

uint32_t FX_GetGdiFontStyles(const LOGFONTW& lf) {
  uint32_t dwStyles = 0;
  if ((lf.lfPitchAndFamily & 0x03) == FIXED_PITCH)
    dwStyles |= FX_FONTSTYLE_FixedPitch;
  uint8_t nFamilies = lf.lfPitchAndFamily & 0xF0;
  if (nFamilies == FF_ROMAN)
    dwStyles |= FX_FONTSTYLE_Serif;
  if (nFamilies == FF_SCRIPT)
    dwStyles |= FX_FONTSTYLE_Script;
  if (lf.lfCharSet == SYMBOL_CHARSET)
    dwStyles |= FX_FONTSTYLE_Symbolic;
  return dwStyles;
}

static int32_t CALLBACK FX_GdiFontEnumProc(ENUMLOGFONTEX* lpelfe,
                                           NEWTEXTMETRICEX* lpntme,
                                           DWORD dwFontType,
                                           LPARAM lParam) {
  if (dwFontType != TRUETYPE_FONTTYPE)
    return 1;
  const LOGFONTW& lf = ((LPENUMLOGFONTEXW)lpelfe)->elfLogFont;
  if (lf.lfFaceName[0] == L'@')
    return 1;
  FX_FONTDESCRIPTOR* pFont = FX_Alloc(FX_FONTDESCRIPTOR, 1);
  FXSYS_memset(pFont, 0, sizeof(FX_FONTDESCRIPTOR));
  pFont->uCharSet = lf.lfCharSet;
  pFont->dwFontStyles = FX_GetGdiFontStyles(lf);
  FXSYS_wcsncpy(pFont->wsFontFace, (const FX_WCHAR*)lf.lfFaceName, 31);
  pFont->wsFontFace[31] = 0;
  FXSYS_memcpy(&pFont->FontSignature, &lpntme->ntmFontSig,
               sizeof(lpntme->ntmFontSig));
  reinterpret_cast<std::deque<FX_FONTDESCRIPTOR>*>(lParam)->push_back(*pFont);
  FX_Free(pFont);
  return 1;
}

static void FX_EnumGdiFonts(std::deque<FX_FONTDESCRIPTOR>* fonts,
                            const FX_WCHAR* pwsFaceName,
                            FX_WCHAR wUnicode) {
  HDC hDC = ::GetDC(nullptr);
  LOGFONTW lfFind;
  FXSYS_memset(&lfFind, 0, sizeof(lfFind));
  lfFind.lfCharSet = DEFAULT_CHARSET;
  if (pwsFaceName) {
    FXSYS_wcsncpy(lfFind.lfFaceName, pwsFaceName, 31);
    lfFind.lfFaceName[31] = 0;
  }
  EnumFontFamiliesExW(hDC, (LPLOGFONTW)&lfFind,
                      (FONTENUMPROCW)FX_GdiFontEnumProc, (LPARAM)fonts, 0);
  ::ReleaseDC(nullptr, hDC);
}

FX_LPEnumAllFonts FX_GetDefFontEnumerator() {
  return FX_EnumGdiFonts;
}

#else  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

namespace {

const FX_CHAR* g_FontFolders[] = {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_LINUX_
    "/usr/share/fonts", "/usr/share/X11/fonts/Type1",
    "/usr/share/X11/fonts/TTF", "/usr/local/share/fonts",
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
    "~/Library/Fonts", "/Library/Fonts", "/System/Library/Fonts",
#elif _FXM_PLATFORM_ == _FXM_PLATFORM_ANDROID_
    "/system/fonts",
#endif
};

struct FX_BitCodePage {
  uint16_t wBit;
  uint16_t wCodePage;
};

const FX_BitCodePage g_Bit2CodePage[] = {
    {0, 1252}, {1, 1250}, {2, 1251}, {3, 1253},  {4, 1254}, {5, 1255},
    {6, 1256}, {7, 1257}, {8, 1258}, {9, 0},     {10, 0},   {11, 0},
    {12, 0},   {13, 0},   {14, 0},   {15, 0},    {16, 874}, {17, 932},
    {18, 936}, {19, 949}, {20, 950}, {21, 1361}, {22, 0},   {23, 0},
    {24, 0},   {25, 0},   {26, 0},   {27, 0},    {28, 0},   {29, 0},
    {30, 0},   {31, 0},   {32, 0},   {33, 0},    {34, 0},   {35, 0},
    {36, 0},   {37, 0},   {38, 0},   {39, 0},    {40, 0},   {41, 0},
    {42, 0},   {43, 0},   {44, 0},   {45, 0},    {46, 0},   {47, 0},
    {48, 869}, {49, 866}, {50, 865}, {51, 864},  {52, 863}, {53, 862},
    {54, 861}, {55, 860}, {56, 857}, {57, 855},  {58, 852}, {59, 775},
    {60, 737}, {61, 708}, {62, 850}, {63, 437},
};

uint16_t FX_GetCodePageBit(uint16_t wCodePage) {
  for (size_t i = 0; i < FX_ArraySize(g_Bit2CodePage); ++i) {
    if (g_Bit2CodePage[i].wCodePage == wCodePage)
      return g_Bit2CodePage[i].wBit;
  }
  return static_cast<uint16_t>(-1);
}

uint16_t FX_GetUnicodeBit(FX_WCHAR wcUnicode) {
  const FGAS_FONTUSB* x = FGAS_GetUnicodeBitField(wcUnicode);
  return x ? x->wBitField : 999;
}

inline uint8_t GetUInt8(const uint8_t* p) {
  return p[0];
}

inline uint16_t GetUInt16(const uint8_t* p) {
  return static_cast<uint16_t>(p[0] << 8 | p[1]);
}

struct FX_BIT2CHARSET {
  uint16_t wBit;
  uint16_t wCharset;
};

const FX_BIT2CHARSET g_FX_Bit2Charset[4][16] = {
    {{1 << 0, FX_CHARSET_ANSI},
     {1 << 1, FX_CHARSET_MSWin_EasterEuropean},
     {1 << 2, FX_CHARSET_MSWin_Cyrillic},
     {1 << 3, FX_CHARSET_MSWin_Greek},
     {1 << 4, FX_CHARSET_MSWin_Turkish},
     {1 << 5, FX_CHARSET_MSWin_Hebrew},
     {1 << 6, FX_CHARSET_MSWin_Arabic},
     {1 << 7, FX_CHARSET_MSWin_Baltic},
     {1 << 8, FX_CHARSET_MSWin_Vietnamese},
     {1 << 9, FX_CHARSET_Default},
     {1 << 10, FX_CHARSET_Default},
     {1 << 11, FX_CHARSET_Default},
     {1 << 12, FX_CHARSET_Default},
     {1 << 13, FX_CHARSET_Default},
     {1 << 14, FX_CHARSET_Default},
     {1 << 15, FX_CHARSET_Default}},
    {{1 << 0, FX_CHARSET_Thai},
     {1 << 1, FX_CHARSET_ShiftJIS},
     {1 << 2, FX_CHARSET_ChineseSimplified},
     {1 << 3, FX_CHARSET_Korean},
     {1 << 4, FX_CHARSET_ChineseTriditional},
     {1 << 5, FX_CHARSET_Johab},
     {1 << 6, FX_CHARSET_Default},
     {1 << 7, FX_CHARSET_Default},
     {1 << 8, FX_CHARSET_Default},
     {1 << 9, FX_CHARSET_Default},
     {1 << 10, FX_CHARSET_Default},
     {1 << 11, FX_CHARSET_Default},
     {1 << 12, FX_CHARSET_Default},
     {1 << 13, FX_CHARSET_Default},
     {1 << 14, FX_CHARSET_OEM},
     {1 << 15, FX_CHARSET_Symbol}},
    {{1 << 0, FX_CHARSET_Default},
     {1 << 1, FX_CHARSET_Default},
     {1 << 2, FX_CHARSET_Default},
     {1 << 3, FX_CHARSET_Default},
     {1 << 4, FX_CHARSET_Default},
     {1 << 5, FX_CHARSET_Default},
     {1 << 6, FX_CHARSET_Default},
     {1 << 7, FX_CHARSET_Default},
     {1 << 8, FX_CHARSET_Default},
     {1 << 9, FX_CHARSET_Default},
     {1 << 10, FX_CHARSET_Default},
     {1 << 11, FX_CHARSET_Default},
     {1 << 12, FX_CHARSET_Default},
     {1 << 13, FX_CHARSET_Default},
     {1 << 14, FX_CHARSET_Default},
     {1 << 15, FX_CHARSET_Default}},
    {{1 << 0, FX_CHARSET_Default},
     {1 << 1, FX_CHARSET_Default},
     {1 << 2, FX_CHARSET_Default},
     {1 << 3, FX_CHARSET_Default},
     {1 << 4, FX_CHARSET_Default},
     {1 << 5, FX_CHARSET_Default},
     {1 << 6, FX_CHARSET_Default},
     {1 << 7, FX_CHARSET_Default},
     {1 << 8, FX_CHARSET_Default},
     {1 << 9, FX_CHARSET_Default},
     {1 << 10, FX_CHARSET_Default},
     {1 << 11, FX_CHARSET_Default},
     {1 << 12, FX_CHARSET_Default},
     {1 << 13, FX_CHARSET_Default},
     {1 << 14, FX_CHARSET_Default},
     {1 << 15, FX_CHARSET_US}}};

}  // namespace

CFX_FontDescriptor::CFX_FontDescriptor()
    : m_nFaceIndex(0), m_dwFontStyles(0), m_dwUsb(), m_dwCsb() {}

CFX_FontDescriptor::~CFX_FontDescriptor() {}

CFX_FontSourceEnum_File::CFX_FontSourceEnum_File() {
  for (size_t i = 0; i < FX_ArraySize(g_FontFolders); ++i)
    m_FolderPaths.push_back(g_FontFolders[i]);
}

CFX_FontSourceEnum_File::~CFX_FontSourceEnum_File() {}

CFX_ByteString CFX_FontSourceEnum_File::GetNextFile() {
  FX_FileHandle* pCurHandle =
      !m_FolderQueue.empty() ? m_FolderQueue.back().pFileHandle : nullptr;
  if (!pCurHandle) {
    if (m_FolderPaths.empty())
      return "";
    pCurHandle = FX_OpenFolder(m_FolderPaths.back().c_str());
    FX_HandleParentPath hpp;
    hpp.pFileHandle = pCurHandle;
    hpp.bsParentPath = m_FolderPaths.back();
    m_FolderQueue.push_back(hpp);
  }
  CFX_ByteString bsName;
  bool bFolder;
  CFX_ByteString bsFolderSeparator =
      CFX_ByteString::FromUnicode(CFX_WideString(FX_GetFolderSeparator()));
  while (true) {
    if (!FX_GetNextFile(pCurHandle, &bsName, &bFolder)) {
      FX_CloseFolder(pCurHandle);
      if (!m_FolderQueue.empty())
        m_FolderQueue.pop_back();
      if (m_FolderQueue.empty()) {
        if (!m_FolderPaths.empty())
          m_FolderPaths.pop_back();
        return !m_FolderPaths.empty() ? GetNextFile() : "";
      }
      pCurHandle = m_FolderQueue.back().pFileHandle;
      continue;
    }
    if (bsName == "." || bsName == "..")
      continue;
    if (bFolder) {
      FX_HandleParentPath hpp;
      hpp.bsParentPath =
          m_FolderQueue.back().bsParentPath + bsFolderSeparator + bsName;
      hpp.pFileHandle = FX_OpenFolder(hpp.bsParentPath.c_str());
      if (!hpp.pFileHandle)
        continue;
      m_FolderQueue.push_back(hpp);
      pCurHandle = hpp.pFileHandle;
      continue;
    }
    bsName = m_FolderQueue.back().bsParentPath + bsFolderSeparator + bsName;
    break;
  }
  return bsName;
}

FX_POSITION CFX_FontSourceEnum_File::GetStartPosition() {
  m_wsNext = GetNextFile().UTF8Decode();
  if (m_wsNext.GetLength() == 0)
    return (FX_POSITION)0;
  return (FX_POSITION)-1;
}

CFX_RetainPtr<IFX_FileAccess> CFX_FontSourceEnum_File::GetNext(
    FX_POSITION& pos) {
  CFX_RetainPtr<IFX_FileAccess> pAccess =
      IFX_FileAccess::CreateDefault(m_wsNext.AsStringC());
  m_wsNext = GetNextFile().UTF8Decode();
  pos = m_wsNext.GetLength() != 0 ? pAccess.Get() : nullptr;
  return pAccess;
}

std::unique_ptr<CFGAS_FontMgr> CFGAS_FontMgr::Create(
    CFX_FontSourceEnum_File* pFontEnum) {
  if (!pFontEnum)
    return nullptr;

  auto pFontMgr = pdfium::MakeUnique<CFGAS_FontMgr>(pFontEnum);
  if (!pFontMgr->EnumFonts())
    return nullptr;
  return pFontMgr;
}

CFGAS_FontMgr::CFGAS_FontMgr(CFX_FontSourceEnum_File* pFontEnum)
    : m_pFontSource(pFontEnum) {}

CFGAS_FontMgr::~CFGAS_FontMgr() {}

bool CFGAS_FontMgr::EnumFontsFromFontMapper() {
  CFX_FontMapper* pFontMapper =
      CFX_GEModule::Get()->GetFontMgr()->GetBuiltinMapper();
  if (!pFontMapper)
    return false;

  IFX_SystemFontInfo* pSystemFontInfo = pFontMapper->GetSystemFontInfo();
  if (!pSystemFontInfo)
    return false;

  pSystemFontInfo->EnumFontList(pFontMapper);
  for (int32_t i = 0; i < pFontMapper->GetFaceSize(); ++i) {
    CFX_RetainPtr<IFX_SeekableReadStream> pFontStream =
        CreateFontStream(pFontMapper, pSystemFontInfo, i);
    if (!pFontStream)
      continue;

    CFX_WideString wsFaceName =
        CFX_WideString::FromLocal(pFontMapper->GetFaceName(i).c_str());
    RegisterFaces(pFontStream, &wsFaceName);
  }
  return !m_InstalledFonts.empty();
}

bool CFGAS_FontMgr::EnumFontsFromFiles() {
  CFX_GEModule::Get()->GetFontMgr()->InitFTLibrary();
  FX_POSITION pos = m_pFontSource->GetStartPosition();
  while (pos) {
    CFX_RetainPtr<IFX_FileAccess> pFontSource = m_pFontSource->GetNext(pos);
    CFX_RetainPtr<IFX_SeekableReadStream> pFontStream =
        pFontSource->CreateFileStream(FX_FILEMODE_ReadOnly);
    if (pFontStream)
      RegisterFaces(pFontStream, nullptr);
  }
  return !m_InstalledFonts.empty();
}

bool CFGAS_FontMgr::EnumFonts() {
  return EnumFontsFromFontMapper() || EnumFontsFromFiles();
}

CFX_RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::LoadFont(
    const FX_WCHAR* pszFontFamily,
    uint32_t dwFontStyles,
    uint16_t wCodePage) {
  return GetFontByCodePage(wCodePage, dwFontStyles, pszFontFamily);
}

CFX_RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::GetFontByCodePage(
    uint16_t wCodePage,
    uint32_t dwFontStyles,
    const FX_WCHAR* pszFontFamily) {
  CFX_ByteString bsHash;
  bsHash.Format("%d, %d", wCodePage, dwFontStyles);
  bsHash += FX_UTF8Encode(CFX_WideStringC(pszFontFamily));
  uint32_t dwHash = FX_HashCode_GetA(bsHash.AsStringC(), false);
  std::vector<CFX_RetainPtr<CFGAS_GEFont>>* pFontArray = &m_Hash2Fonts[dwHash];
  if (!pFontArray->empty())
    return (*pFontArray)[0];

  std::vector<CFX_FontDescriptorInfo>* sortedFontInfos =
      m_Hash2CandidateList[dwHash].get();
  if (!sortedFontInfos) {
    auto pNewFonts = pdfium::MakeUnique<std::vector<CFX_FontDescriptorInfo>>();
    sortedFontInfos = pNewFonts.get();
    MatchFonts(sortedFontInfos, wCodePage, dwFontStyles,
               CFX_WideString(pszFontFamily), 0);
    m_Hash2CandidateList[dwHash] = std::move(pNewFonts);
  }
  if (sortedFontInfos->empty())
    return nullptr;

  CFX_FontDescriptor* pDesc = (*sortedFontInfos)[0].pFont;
  CFX_RetainPtr<CFGAS_GEFont> pFont =
      LoadFont(pDesc->m_wsFaceName, pDesc->m_nFaceIndex, nullptr);
  if (!pFont)
    return nullptr;

  pFont->SetLogicalFontStyle(dwFontStyles);
  pFontArray->push_back(pFont);
  return pFont;
}

CFX_RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::GetFontByUnicode(
    FX_WCHAR wUnicode,
    uint32_t dwFontStyles,
    const FX_WCHAR* pszFontFamily) {
  if (pdfium::ContainsKey(m_FailedUnicodesSet, wUnicode))
    return nullptr;

  const FGAS_FONTUSB* x = FGAS_GetUnicodeBitField(wUnicode);
  uint16_t wCodePage = x ? x->wCodePage : 0xFFFF;
  uint16_t wBitField = x ? x->wBitField : 0x03E7;
  CFX_ByteString bsHash;
  if (wCodePage == 0xFFFF)
    bsHash.Format("%d, %d, %d", wCodePage, wBitField, dwFontStyles);
  else
    bsHash.Format("%d, %d", wCodePage, dwFontStyles);
  bsHash += FX_UTF8Encode(CFX_WideStringC(pszFontFamily));
  uint32_t dwHash = FX_HashCode_GetA(bsHash.AsStringC(), false);
  std::vector<CFX_RetainPtr<CFGAS_GEFont>>* pFonts = &m_Hash2Fonts[dwHash];
  for (size_t i = 0; i < pFonts->size(); ++i) {
    if (VerifyUnicode((*pFonts)[i], wUnicode))
      return (*pFonts)[i];
  }
  std::vector<CFX_FontDescriptorInfo>* sortedFontInfos =
      m_Hash2CandidateList[dwHash].get();
  if (!sortedFontInfos) {
    auto pNewFonts = pdfium::MakeUnique<std::vector<CFX_FontDescriptorInfo>>();
    sortedFontInfos = pNewFonts.get();
    MatchFonts(sortedFontInfos, wCodePage, dwFontStyles,
               CFX_WideString(pszFontFamily), wUnicode);
    m_Hash2CandidateList[dwHash] = std::move(pNewFonts);
  }
  for (const auto& info : *sortedFontInfos) {
    CFX_FontDescriptor* pDesc = info.pFont;
    if (!VerifyUnicode(pDesc, wUnicode))
      continue;
    CFX_RetainPtr<CFGAS_GEFont> pFont =
        LoadFont(pDesc->m_wsFaceName, pDesc->m_nFaceIndex, nullptr);
    if (!pFont)
      continue;
    pFont->SetLogicalFontStyle(dwFontStyles);
    pFonts->push_back(pFont);
    return pFont;
  }
  if (!pszFontFamily)
    m_FailedUnicodesSet.insert(wUnicode);
  return nullptr;
}

bool CFGAS_FontMgr::VerifyUnicode(CFX_FontDescriptor* pDesc,
                                  FX_WCHAR wcUnicode) {
  CFX_RetainPtr<IFX_SeekableReadStream> pFileRead =
      CreateFontStream(pDesc->m_wsFaceName.UTF8Encode());
  if (!pFileRead)
    return false;

  FXFT_Face pFace = LoadFace(pFileRead, pDesc->m_nFaceIndex);
  FT_Error retCharmap = FXFT_Select_Charmap(pFace, FXFT_ENCODING_UNICODE);
  FT_Error retIndex = FXFT_Get_Char_Index(pFace, wcUnicode);
  if (!pFace)
    return false;

  if (FXFT_Get_Face_External_Stream(pFace))
    FXFT_Clear_Face_External_Stream(pFace);

  FXFT_Done_Face(pFace);
  return !retCharmap && retIndex;
}

bool CFGAS_FontMgr::VerifyUnicode(const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                                  FX_WCHAR wcUnicode) {
  if (!pFont)
    return false;

  FXFT_Face pFace = pFont->GetDevFont()->GetFace();
  FXFT_CharMap charmap = FXFT_Get_Face_Charmap(pFace);
  if (FXFT_Select_Charmap(pFace, FXFT_ENCODING_UNICODE) != 0)
    return false;

  if (FXFT_Get_Char_Index(pFace, wcUnicode) == 0) {
    FXFT_Set_Charmap(pFace, charmap);
    return false;
  }
  return true;
}

CFX_RetainPtr<CFGAS_GEFont> CFGAS_FontMgr::LoadFont(
    const CFX_WideString& wsFaceName,
    int32_t iFaceIndex,
    int32_t* pFaceCount) {
  CFX_FontMgr* pFontMgr = CFX_GEModule::Get()->GetFontMgr();
  CFX_FontMapper* pFontMapper = pFontMgr->GetBuiltinMapper();
  if (!pFontMapper)
    return nullptr;

  IFX_SystemFontInfo* pSystemFontInfo = pFontMapper->GetSystemFontInfo();
  if (!pSystemFontInfo)
    return nullptr;

  CFX_RetainPtr<IFX_SeekableReadStream> pFontStream =
      CreateFontStream(wsFaceName.UTF8Encode());
  if (!pFontStream)
    return nullptr;

  auto pInternalFont = pdfium::MakeUnique<CFX_Font>();
  if (!pInternalFont->LoadFile(pFontStream, iFaceIndex))
    return nullptr;

  CFX_RetainPtr<CFGAS_GEFont> pFont =
      CFGAS_GEFont::LoadFont(std::move(pInternalFont), this);
  if (!pFont)
    return nullptr;

  m_IFXFont2FileRead[pFont] = pFontStream;
  if (pFaceCount)
    *pFaceCount = pFont->GetDevFont()->GetFace()->num_faces;
  return pFont;
}

extern "C" {

unsigned long _ftStreamRead(FXFT_Stream stream,
                            unsigned long offset,
                            unsigned char* buffer,
                            unsigned long count) {
  if (count == 0)
    return 0;

  IFX_SeekableReadStream* pFile =
      static_cast<IFX_SeekableReadStream*>(stream->descriptor.pointer);
  if (!pFile->ReadBlock(buffer, offset, count))
    return 0;

  return count;
}

void _ftStreamClose(FXFT_Stream stream) {}

};  // extern "C"

FXFT_Face CFGAS_FontMgr::LoadFace(
    const CFX_RetainPtr<IFX_SeekableReadStream>& pFontStream,
    int32_t iFaceIndex) {
  if (!pFontStream)
    return nullptr;

  CFX_FontMgr* pFontMgr = CFX_GEModule::Get()->GetFontMgr();
  pFontMgr->InitFTLibrary();

  FXFT_Library library = pFontMgr->GetFTLibrary();
  if (!library)
    return nullptr;

  FXFT_Stream ftStream = FX_Alloc(FXFT_StreamRec, 1);
  FXSYS_memset(ftStream, 0, sizeof(FXFT_StreamRec));
  ftStream->base = nullptr;
  ftStream->descriptor.pointer = static_cast<void*>(pFontStream.Get());
  ftStream->pos = 0;
  ftStream->size = static_cast<unsigned long>(pFontStream->GetSize());
  ftStream->read = _ftStreamRead;
  ftStream->close = _ftStreamClose;

  FXFT_Open_Args ftArgs;
  FXSYS_memset(&ftArgs, 0, sizeof(FXFT_Open_Args));
  ftArgs.flags |= FT_OPEN_STREAM;
  ftArgs.stream = ftStream;

  FXFT_Face pFace = nullptr;
  if (FXFT_Open_Face(library, &ftArgs, iFaceIndex, &pFace)) {
    FX_Free(ftStream);
    return nullptr;
  }

  FXFT_Set_Pixel_Sizes(pFace, 0, 64);
  return pFace;
}

CFX_RetainPtr<IFX_SeekableReadStream> CFGAS_FontMgr::CreateFontStream(
    CFX_FontMapper* pFontMapper,
    IFX_SystemFontInfo* pSystemFontInfo,
    uint32_t index) {
  int iExact = 0;
  void* hFont =
      pSystemFontInfo->MapFont(0, 0, FXFONT_DEFAULT_CHARSET, 0,
                               pFontMapper->GetFaceName(index).c_str(), iExact);
  if (!hFont)
    return nullptr;

  uint32_t dwFileSize = pSystemFontInfo->GetFontData(hFont, 0, nullptr, 0);
  if (dwFileSize == 0)
    return nullptr;

  uint8_t* pBuffer = FX_Alloc(uint8_t, dwFileSize + 1);
  dwFileSize = pSystemFontInfo->GetFontData(hFont, 0, pBuffer, dwFileSize);

  return IFX_MemoryStream::Create(pBuffer, dwFileSize, true);
}

CFX_RetainPtr<IFX_SeekableReadStream> CFGAS_FontMgr::CreateFontStream(
    const CFX_ByteString& bsFaceName) {
  CFX_FontMgr* pFontMgr = CFX_GEModule::Get()->GetFontMgr();
  CFX_FontMapper* pFontMapper = pFontMgr->GetBuiltinMapper();
  if (!pFontMapper)
    return nullptr;

  IFX_SystemFontInfo* pSystemFontInfo = pFontMapper->GetSystemFontInfo();
  if (!pSystemFontInfo)
    return nullptr;

  pSystemFontInfo->EnumFontList(pFontMapper);
  for (int32_t i = 0; i < pFontMapper->GetFaceSize(); ++i) {
    if (pFontMapper->GetFaceName(i) == bsFaceName)
      return CreateFontStream(pFontMapper, pSystemFontInfo, i);
  }
  return nullptr;
}

void CFGAS_FontMgr::MatchFonts(
    std::vector<CFX_FontDescriptorInfo>* pMatchedFonts,
    uint16_t wCodePage,
    uint32_t dwFontStyles,
    const CFX_WideString& FontName,
    FX_WCHAR wcUnicode) {
  pMatchedFonts->clear();
  for (const auto& pFont : m_InstalledFonts) {
    int32_t nPenalty =
        CalcPenalty(pFont.get(), wCodePage, dwFontStyles, FontName, wcUnicode);
    if (nPenalty >= 0xffff)
      continue;
    pMatchedFonts->push_back({pFont.get(), nPenalty});
    if (pMatchedFonts->size() == 0xffff)
      break;
  }
  std::sort(pMatchedFonts->begin(), pMatchedFonts->end());
}

int32_t CFGAS_FontMgr::CalcPenalty(CFX_FontDescriptor* pInstalled,
                                   uint16_t wCodePage,
                                   uint32_t dwFontStyles,
                                   const CFX_WideString& FontName,
                                   FX_WCHAR wcUnicode) {
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
    if (30000 == nPenalty &&
        0 == IsPartName(pInstalled->m_wsFaceName, FontName)) {
      size_t i;
      for (i = 0; i < pInstalled->m_wsFamilyNames.size(); i++) {
        if (IsPartName(pInstalled->m_wsFamilyNames[i], FontName) != 0)
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
  if (dwStyleMask & FX_FONTSTYLE_Bold)
    nPenalty += 4500;
  if (dwStyleMask & FX_FONTSTYLE_FixedPitch)
    nPenalty += 10000;
  if (dwStyleMask & FX_FONTSTYLE_Italic)
    nPenalty += 10000;
  if (dwStyleMask & FX_FONTSTYLE_Serif)
    nPenalty += 500;
  if (dwStyleMask & FX_FONTSTYLE_Symbolic)
    nPenalty += 0xFFFF;
  if (nPenalty >= 0xFFFF)
    return 0xFFFF;

  uint16_t wBit = (wCodePage == 0 || wCodePage == 0xFFFF)
                      ? static_cast<uint16_t>(-1)
                      : FX_GetCodePageBit(wCodePage);
  if (wBit != static_cast<uint16_t>(-1)) {
    ASSERT(wBit < 64);
    if ((pInstalled->m_dwCsb[wBit / 32] & (1 << (wBit % 32))) == 0)
      nPenalty += 0xFFFF;
    else
      nPenalty -= 60000;
  }
  wBit = (wcUnicode == 0 || wcUnicode == 0xFFFE) ? static_cast<uint16_t>(999)
                                                 : FX_GetUnicodeBit(wcUnicode);
  if (wBit != static_cast<uint16_t>(999)) {
    ASSERT(wBit < 128);
    if ((pInstalled->m_dwUsb[wBit / 32] & (1 << (wBit % 32))) == 0)
      nPenalty += 0xFFFF;
    else
      nPenalty -= 60000;
  }
  return nPenalty;
}

void CFGAS_FontMgr::RemoveFont(const CFX_RetainPtr<CFGAS_GEFont>& pEFont) {
  if (!pEFont)
    return;

  m_IFXFont2FileRead.erase(pEFont);

  auto iter = m_Hash2Fonts.begin();
  while (iter != m_Hash2Fonts.end()) {
    auto old_iter = iter++;
    bool all_empty = true;
    for (size_t i = 0; i < old_iter->second.size(); i++) {
      if (old_iter->second[i] == pEFont)
        old_iter->second[i].Reset();
      else if (old_iter->second[i])
        all_empty = false;
    }
    if (all_empty)
      m_Hash2Fonts.erase(old_iter);
  }
}

void CFGAS_FontMgr::RegisterFace(FXFT_Face pFace,
                                 const CFX_WideString* pFaceName) {
  if ((pFace->face_flags & FT_FACE_FLAG_SCALABLE) == 0)
    return;

  auto pFont = pdfium::MakeUnique<CFX_FontDescriptor>();
  pFont->m_dwFontStyles |= FXFT_Is_Face_Bold(pFace) ? FX_FONTSTYLE_Bold : 0;
  pFont->m_dwFontStyles |= FXFT_Is_Face_Italic(pFace) ? FX_FONTSTYLE_Italic : 0;
  pFont->m_dwFontStyles |= GetFlags(pFace);

  std::vector<uint16_t> charsets = GetCharsets(pFace);
  GetUSBCSB(pFace, pFont->m_dwUsb, pFont->m_dwCsb);

  FT_ULong dwTag;
  FT_ENC_TAG(dwTag, 'n', 'a', 'm', 'e');

  std::vector<uint8_t> table;
  unsigned long nLength = 0;
  unsigned int error = FXFT_Load_Sfnt_Table(pFace, dwTag, 0, nullptr, &nLength);
  if (error == 0 && nLength != 0) {
    table.resize(nLength);
    if (FXFT_Load_Sfnt_Table(pFace, dwTag, 0, table.data(), nullptr))
      table.clear();
  }
  GetNames(table.empty() ? nullptr : table.data(), pFont->m_wsFamilyNames);
  pFont->m_wsFamilyNames.push_back(
      CFX_ByteString(pFace->family_name).UTF8Decode());
  pFont->m_wsFaceName =
      pFaceName ? *pFaceName
                : CFX_WideString::FromLocal(FXFT_Get_Postscript_Name(pFace));
  pFont->m_nFaceIndex = pFace->face_index;
  m_InstalledFonts.push_back(std::move(pFont));
}

void CFGAS_FontMgr::RegisterFaces(
    const CFX_RetainPtr<IFX_SeekableReadStream>& pFontStream,
    const CFX_WideString* pFaceName) {
  int32_t index = 0;
  int32_t num_faces = 0;
  do {
    FXFT_Face pFace = LoadFace(pFontStream, index++);
    if (!pFace)
      continue;
    // All faces keep number of faces. It can be retrieved from any one face.
    if (num_faces == 0)
      num_faces = pFace->num_faces;
    RegisterFace(pFace, pFaceName);
    if (FXFT_Get_Face_External_Stream(pFace))
      FXFT_Clear_Face_External_Stream(pFace);
    FXFT_Done_Face(pFace);
  } while (index < num_faces);
}

uint32_t CFGAS_FontMgr::GetFlags(FXFT_Face pFace) {
  uint32_t flag = 0;
  if (FT_IS_FIXED_WIDTH(pFace))
    flag |= FX_FONTSTYLE_FixedPitch;
  TT_OS2* pOS2 = (TT_OS2*)FT_Get_Sfnt_Table(pFace, ft_sfnt_os2);
  if (!pOS2)
    return flag;

  if (pOS2->ulCodePageRange1 & (1 << 31))
    flag |= FX_FONTSTYLE_Symbolic;
  if (pOS2->panose[0] == 2) {
    uint8_t uSerif = pOS2->panose[1];
    if ((uSerif > 1 && uSerif < 10) || uSerif > 13)
      flag |= FX_FONTSTYLE_Serif;
  }
  return flag;
}

void CFGAS_FontMgr::GetNames(const uint8_t* name_table,
                             std::vector<CFX_WideString>& Names) {
  if (!name_table)
    return;

  uint8_t* lpTable = (uint8_t*)name_table;
  CFX_WideString wsFamily;
  uint8_t* sp = lpTable + 2;
  uint8_t* lpNameRecord = lpTable + 6;
  uint16_t nNameCount = GetUInt16(sp);
  uint8_t* lpStr = lpTable + GetUInt16(sp + 2);
  for (uint16_t j = 0; j < nNameCount; j++) {
    uint16_t nNameID = GetUInt16(lpNameRecord + j * 12 + 6);
    if (nNameID != 1)
      continue;

    uint16_t nPlatformID = GetUInt16(lpNameRecord + j * 12 + 0);
    uint16_t nNameLength = GetUInt16(lpNameRecord + j * 12 + 8);
    uint16_t nNameOffset = GetUInt16(lpNameRecord + j * 12 + 10);
    wsFamily.clear();
    if (nPlatformID != 1) {
      for (uint16_t k = 0; k < nNameLength / 2; k++) {
        FX_WCHAR wcTemp = GetUInt16(lpStr + nNameOffset + k * 2);
        wsFamily += wcTemp;
      }
      Names.push_back(wsFamily);
      continue;
    }
    for (uint16_t k = 0; k < nNameLength; k++) {
      FX_WCHAR wcTemp = GetUInt8(lpStr + nNameOffset + k);
      wsFamily += wcTemp;
    }
    Names.push_back(wsFamily);
  }
}

std::vector<uint16_t> CFGAS_FontMgr::GetCharsets(FXFT_Face pFace) const {
  std::vector<uint16_t> charsets;
  TT_OS2* pOS2 = (TT_OS2*)FT_Get_Sfnt_Table(pFace, ft_sfnt_os2);
  if (!pOS2) {
    charsets.push_back(FX_CHARSET_Default);
    return charsets;
  }
  uint16_t a[4] = {
      pOS2->ulCodePageRange1 & 0xffff, (pOS2->ulCodePageRange1 >> 16) & 0xffff,
      pOS2->ulCodePageRange2 & 0xffff, (pOS2->ulCodePageRange2 >> 16) & 0xffff};
  for (int n = 0; n < 4; n++) {
    for (int32_t i = 0; i < 16; i++) {
      if ((a[n] & g_FX_Bit2Charset[n][i].wBit) != 0)
        charsets.push_back(g_FX_Bit2Charset[n][i].wCharset);
    }
  }
  return charsets;
}

void CFGAS_FontMgr::GetUSBCSB(FXFT_Face pFace, uint32_t* USB, uint32_t* CSB) {
  TT_OS2* pOS2 = (TT_OS2*)FT_Get_Sfnt_Table(pFace, ft_sfnt_os2);
  if (!pOS2) {
    USB[0] = 0;
    USB[1] = 0;
    USB[2] = 0;
    USB[3] = 0;
    CSB[0] = 0;
    CSB[1] = 0;
    return;
  }
  USB[0] = pOS2->ulUnicodeRange1;
  USB[1] = pOS2->ulUnicodeRange2;
  USB[2] = pOS2->ulUnicodeRange3;
  USB[3] = pOS2->ulUnicodeRange4;
  CSB[0] = pOS2->ulCodePageRange1;
  CSB[1] = pOS2->ulCodePageRange2;
}

int32_t CFGAS_FontMgr::IsPartName(const CFX_WideString& Name1,
                                  const CFX_WideString& Name2) {
  if (Name1.Find(Name2.c_str()) != -1)
    return 1;
  return 0;
}

#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
