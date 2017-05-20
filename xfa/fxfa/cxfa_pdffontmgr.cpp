// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_pdffontmgr.h"

#include <algorithm>

#include "core/fpdfapi/font/cpdf_font.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fxfa/cxfa_ffapp.h"

namespace {

// The 5 names per entry are: PsName, Normal, Bold, Italic, BoldItalic.
const char* const g_XFAPDFFontName[][5] = {
    {"Adobe PI Std", "AdobePIStd", "AdobePIStd", "AdobePIStd", "AdobePIStd"},
    {"Myriad Pro Light", "MyriadPro-Light", "MyriadPro-Semibold",
     "MyriadPro-LightIt", "MyriadPro-SemiboldIt"},
};

}  // namespace

CXFA_PDFFontMgr::CXFA_PDFFontMgr(CXFA_FFDoc* pDoc) : m_pDoc(pDoc) {}

CXFA_PDFFontMgr::~CXFA_PDFFontMgr() {}

CFX_RetainPtr<CFGAS_GEFont> CXFA_PDFFontMgr::FindFont(
    const CFX_ByteString& strPsName,
    bool bBold,
    bool bItalic,
    CPDF_Font** pDstPDFFont,
    bool bStrictMatch) {
  CPDF_Document* pDoc = m_pDoc->GetPDFDoc();
  if (!pDoc)
    return nullptr;

  CPDF_Dictionary* pFontSetDict =
      pDoc->GetRoot()->GetDictFor("AcroForm")->GetDictFor("DR");
  if (!pFontSetDict)
    return nullptr;

  pFontSetDict = pFontSetDict->GetDictFor("Font");
  if (!pFontSetDict)
    return nullptr;

  CFX_ByteString name = strPsName;
  name.Remove(' ');
  CFGAS_FontMgr* pFDEFontMgr = m_pDoc->GetApp()->GetFDEFontMgr();
  for (const auto& it : *pFontSetDict) {
    const CFX_ByteString& key = it.first;
    CPDF_Object* pObj = it.second.get();
    if (!PsNameMatchDRFontName(name.AsStringC(), bBold, bItalic, key,
                               bStrictMatch)) {
      continue;
    }
    CPDF_Dictionary* pFontDict = ToDictionary(pObj->GetDirect());
    if (!pFontDict || pFontDict->GetStringFor("Type") != "Font") {
      return nullptr;
    }
    CPDF_Font* pPDFFont = pDoc->LoadFont(pFontDict);
    if (!pPDFFont) {
      return nullptr;
    }
    if (!pPDFFont->IsEmbedded()) {
      *pDstPDFFont = pPDFFont;
      return nullptr;
    }
    return CFGAS_GEFont::LoadFont(pPDFFont->GetFont(), pFDEFontMgr);
  }
  return nullptr;
}

CFX_RetainPtr<CFGAS_GEFont> CXFA_PDFFontMgr::GetFont(
    const CFX_WideStringC& wsFontFamily,
    uint32_t dwFontStyles,
    CPDF_Font** pPDFFont,
    bool bStrictMatch) {
  uint32_t dwHashCode = FX_HashCode_GetW(wsFontFamily, false);
  CFX_ByteString strKey;
  strKey.Format("%u%u", dwHashCode, dwFontStyles);
  auto it = m_FontMap.find(strKey);
  if (it != m_FontMap.end())
    return it->second;
  CFX_ByteString bsPsName =
      CFX_ByteString::FromUnicode(CFX_WideString(wsFontFamily));
  bool bBold = (dwFontStyles & FX_FONTSTYLE_Bold) == FX_FONTSTYLE_Bold;
  bool bItalic = (dwFontStyles & FX_FONTSTYLE_Italic) == FX_FONTSTYLE_Italic;
  CFX_ByteString strFontName = PsNameToFontName(bsPsName, bBold, bItalic);
  CFX_RetainPtr<CFGAS_GEFont> pFont =
      FindFont(strFontName, bBold, bItalic, pPDFFont, bStrictMatch);
  if (pFont)
    m_FontMap[strKey] = pFont;
  return pFont;
}

CFX_ByteString CXFA_PDFFontMgr::PsNameToFontName(
    const CFX_ByteString& strPsName,
    bool bBold,
    bool bItalic) {
  for (size_t i = 0; i < FX_ArraySize(g_XFAPDFFontName); ++i) {
    if (strPsName == g_XFAPDFFontName[i][0]) {
      size_t index = 1;
      if (bBold)
        ++index;
      if (bItalic)
        index += 2;
      return g_XFAPDFFontName[i][index];
    }
  }
  return strPsName;
}

bool CXFA_PDFFontMgr::PsNameMatchDRFontName(const CFX_ByteStringC& bsPsName,
                                            bool bBold,
                                            bool bItalic,
                                            const CFX_ByteString& bsDRFontName,
                                            bool bStrictMatch) {
  CFX_ByteString bsDRName = bsDRFontName;
  bsDRName.Remove('-');
  int32_t iPsLen = bsPsName.GetLength();
  int32_t nIndex = bsDRName.Find(bsPsName);
  if (nIndex != -1 && !bStrictMatch)
    return true;

  if (nIndex != 0)
    return false;

  int32_t iDifferLength = bsDRName.GetLength() - iPsLen;
  if (iDifferLength > 1 || (bBold || bItalic)) {
    int32_t iBoldIndex = bsDRName.Find("Bold");
    bool bBoldFont = iBoldIndex > 0;
    if (bBold != bBoldFont)
      return false;

    if (bBoldFont) {
      iDifferLength =
          std::min(iDifferLength - 4, bsDRName.GetLength() - iBoldIndex - 4);
    }
    bool bItalicFont = true;
    if (bsDRName.Find("Italic") > 0) {
      iDifferLength -= 6;
    } else if (bsDRName.Find("It") > 0) {
      iDifferLength -= 2;
    } else if (bsDRName.Find("Oblique") > 0) {
      iDifferLength -= 7;
    } else {
      bItalicFont = false;
    }
    if (bItalic != bItalicFont)
      return false;

    if (iDifferLength > 1) {
      CFX_ByteString bsDRTailer = bsDRName.Right(iDifferLength);
      if (bsDRTailer == "MT" || bsDRTailer == "PSMT" ||
          bsDRTailer == "Regular" || bsDRTailer == "Reg") {
        return true;
      }
      if (bBoldFont || bItalicFont)
        return false;

      bool bMatch = false;
      switch (bsPsName.GetAt(iPsLen - 1)) {
        case 'L': {
          if (bsDRName.Right(5) == "Light") {
            bMatch = true;
          }
        } break;
        case 'R': {
          if (bsDRName.Right(7) == "Regular" || bsDRName.Right(3) == "Reg") {
            bMatch = true;
          }
        } break;
        case 'M': {
          if (bsDRName.Right(5) == "Medium") {
            bMatch = true;
          }
        } break;
        default:
          break;
      }
      return bMatch;
    }
  }
  return true;
}

bool CXFA_PDFFontMgr::GetCharWidth(const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                                   wchar_t wUnicode,
                                   bool bCharCode,
                                   int32_t* pWidth) {
  if (wUnicode != 0x20 || bCharCode)
    return false;

  auto it = m_FDE2PDFFont.find(pFont);
  if (it == m_FDE2PDFFont.end())
    return false;

  CPDF_Font* pPDFFont = it->second;
  *pWidth = pPDFFont->GetCharWidthF(pPDFFont->CharCodeFromUnicode(wUnicode));
  return true;
}

void CXFA_PDFFontMgr::SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                              CPDF_Font* pPDFFont) {
  m_FDE2PDFFont[pFont] = pPDFFont;
}
