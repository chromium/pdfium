// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "font_int.h"

#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_page.h"
#include "core/include/fpdfapi/fpdf_resource.h"
#include "core/include/fxcrt/fx_ext.h"
#include "core/include/fxge/fx_freetype.h"
#include "core/include/fxge/fx_ge.h"
#include "core/src/fpdfapi/fpdf_cmaps/cmap_int.h"

namespace {

const FX_CHAR* const g_CharsetNames[CIDSET_NUM_SETS] =
    {nullptr, "GB1", "CNS1", "Japan1", "Korea1", "UCS"};

const int g_CharsetCPs[CIDSET_NUM_SETS] = {0, 936, 950, 932, 949, 1200};

class CPDF_PredefinedCMap {
 public:
  const FX_CHAR* m_pName;
  CIDSet m_Charset;
  int m_Coding;
  CPDF_CMap::CodingScheme m_CodingScheme;
  FX_DWORD m_LeadingSegCount;
  uint8_t m_LeadingSegs[4];
};

const CPDF_PredefinedCMap g_PredefinedCMaps[] = {
    {"GB-EUC",
     CIDSET_GB1,
     CIDCODING_GB,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0xa1, 0xfe}},
    {"GBpc-EUC",
     CIDSET_GB1,
     CIDCODING_GB,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0xa1, 0xfc}},
    {"GBK-EUC",
     CIDSET_GB1,
     CIDCODING_GB,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0x81, 0xfe}},
    {"GBKp-EUC",
     CIDSET_GB1,
     CIDCODING_GB,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0x81, 0xfe}},
    {"GBK2K-EUC",
     CIDSET_GB1,
     CIDCODING_GB,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0x81, 0xfe}},
    {"GBK2K",
     CIDSET_GB1,
     CIDCODING_GB,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0x81, 0xfe}},
    {"UniGB-UCS2", CIDSET_GB1, CIDCODING_UCS2, CPDF_CMap::TwoBytes},
    {"UniGB-UTF16", CIDSET_GB1, CIDCODING_UTF16, CPDF_CMap::TwoBytes},
    {"B5pc",
     CIDSET_CNS1,
     CIDCODING_BIG5,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0xa1, 0xfc}},
    {"HKscs-B5",
     CIDSET_CNS1,
     CIDCODING_BIG5,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0x88, 0xfe}},
    {"ETen-B5",
     CIDSET_CNS1,
     CIDCODING_BIG5,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0xa1, 0xfe}},
    {"ETenms-B5",
     CIDSET_CNS1,
     CIDCODING_BIG5,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0xa1, 0xfe}},
    {"UniCNS-UCS2", CIDSET_CNS1, CIDCODING_UCS2, CPDF_CMap::TwoBytes},
    {"UniCNS-UTF16", CIDSET_CNS1, CIDCODING_UTF16, CPDF_CMap::TwoBytes},
    {"83pv-RKSJ",
     CIDSET_JAPAN1,
     CIDCODING_JIS,
     CPDF_CMap::MixedTwoBytes,
     2,
     {0x81, 0x9f, 0xe0, 0xfc}},
    {"90ms-RKSJ",
     CIDSET_JAPAN1,
     CIDCODING_JIS,
     CPDF_CMap::MixedTwoBytes,
     2,
     {0x81, 0x9f, 0xe0, 0xfc}},
    {"90msp-RKSJ",
     CIDSET_JAPAN1,
     CIDCODING_JIS,
     CPDF_CMap::MixedTwoBytes,
     2,
     {0x81, 0x9f, 0xe0, 0xfc}},
    {"90pv-RKSJ",
     CIDSET_JAPAN1,
     CIDCODING_JIS,
     CPDF_CMap::MixedTwoBytes,
     2,
     {0x81, 0x9f, 0xe0, 0xfc}},
    {"Add-RKSJ",
     CIDSET_JAPAN1,
     CIDCODING_JIS,
     CPDF_CMap::MixedTwoBytes,
     2,
     {0x81, 0x9f, 0xe0, 0xfc}},
    {"EUC",
     CIDSET_JAPAN1,
     CIDCODING_JIS,
     CPDF_CMap::MixedTwoBytes,
     2,
     {0x8e, 0x8e, 0xa1, 0xfe}},
    {"H", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::TwoBytes, 1, {0x21, 0x7e}},
    {"V", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::TwoBytes, 1, {0x21, 0x7e}},
    {"Ext-RKSJ",
     CIDSET_JAPAN1,
     CIDCODING_JIS,
     CPDF_CMap::MixedTwoBytes,
     2,
     {0x81, 0x9f, 0xe0, 0xfc}},
    {"UniJIS-UCS2", CIDSET_JAPAN1, CIDCODING_UCS2, CPDF_CMap::TwoBytes},
    {"UniJIS-UCS2-HW", CIDSET_JAPAN1, CIDCODING_UCS2, CPDF_CMap::TwoBytes},
    {"UniJIS-UTF16", CIDSET_JAPAN1, CIDCODING_UTF16, CPDF_CMap::TwoBytes},
    {"KSC-EUC",
     CIDSET_KOREA1,
     CIDCODING_KOREA,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0xa1, 0xfe}},
    {"KSCms-UHC",
     CIDSET_KOREA1,
     CIDCODING_KOREA,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0x81, 0xfe}},
    {"KSCms-UHC-HW",
     CIDSET_KOREA1,
     CIDCODING_KOREA,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0x81, 0xfe}},
    {"KSCpc-EUC",
     CIDSET_KOREA1,
     CIDCODING_KOREA,
     CPDF_CMap::MixedTwoBytes,
     1,
     {0xa1, 0xfd}},
    {"UniKS-UCS2", CIDSET_KOREA1, CIDCODING_UCS2, CPDF_CMap::TwoBytes},
    {"UniKS-UTF16", CIDSET_KOREA1, CIDCODING_UTF16, CPDF_CMap::TwoBytes},
};

CIDSet CIDSetFromSizeT(size_t index) {
  if (index >= CIDSET_NUM_SETS) {
    NOTREACHED();
    return CIDSET_UNKNOWN;
  }
  return static_cast<CIDSet>(index);
}

CIDSet CharsetFromOrdering(const CFX_ByteString& ordering) {
  for (size_t charset = 1; charset < FX_ArraySize(g_CharsetNames); ++charset) {
    if (ordering == CFX_ByteStringC(g_CharsetNames[charset]))
      return CIDSetFromSizeT(charset);
  }
  return CIDSET_UNKNOWN;
}

CFX_ByteString CMap_GetString(const CFX_ByteStringC& word) {
  return word.Mid(1, word.GetLength() - 2);
}

int CompareDWORD(const void* data1, const void* data2) {
  return (*(FX_DWORD*)data1) - (*(FX_DWORD*)data2);
}

int CompareCID(const void* key, const void* element) {
  if ((*(FX_DWORD*)key) < (*(FX_DWORD*)element)) {
    return -1;
  }
  if ((*(FX_DWORD*)key) >
      (*(FX_DWORD*)element) + ((FX_DWORD*)element)[1] / 65536) {
    return 1;
  }
  return 0;
}

int CheckCodeRange(uint8_t* codes,
                   int size,
                   CMap_CodeRange* pRanges,
                   int nRanges) {
  int iSeg = nRanges - 1;
  while (iSeg >= 0) {
    if (pRanges[iSeg].m_CharSize < size) {
      --iSeg;
      continue;
    }
    int iChar = 0;
    while (iChar < size) {
      if (codes[iChar] < pRanges[iSeg].m_Lower[iChar] ||
          codes[iChar] > pRanges[iSeg].m_Upper[iChar]) {
        break;
      }
      ++iChar;
    }
    if (iChar == pRanges[iSeg].m_CharSize)
      return 2;

    if (iChar)
      return (size == pRanges[iSeg].m_CharSize) ? 2 : 1;
    iSeg--;
  }
  return 0;
}

int GetCharSizeImpl(FX_DWORD charcode,
                    CMap_CodeRange* pRanges,
                    int iRangesSize) {
  if (!iRangesSize)
    return 1;

  uint8_t codes[4];
  codes[0] = codes[1] = 0x00;
  codes[2] = (uint8_t)(charcode >> 8 & 0xFF);
  codes[3] = (uint8_t)charcode;
  int offset = 0;
  int size = 4;
  for (int i = 0; i < 4; ++i) {
    int iSeg = iRangesSize - 1;
    while (iSeg >= 0) {
      if (pRanges[iSeg].m_CharSize < size) {
        --iSeg;
        continue;
      }
      int iChar = 0;
      while (iChar < size) {
        if (codes[offset + iChar] < pRanges[iSeg].m_Lower[iChar] ||
            codes[offset + iChar] > pRanges[iSeg].m_Upper[iChar]) {
          break;
        }
        ++iChar;
      }
      if (iChar == pRanges[iSeg].m_CharSize)
        return size;
      --iSeg;
    }
    --size;
    ++offset;
  }
  return 1;
}

bool IsValidEmbeddedCharcodeFromUnicodeCharset(CIDSet charset) {
  switch (charset) {
    case CIDSET_GB1:
    case CIDSET_CNS1:
    case CIDSET_JAPAN1:
    case CIDSET_KOREA1:
      return true;

    default:
      return false;
  }
}

#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
FX_DWORD EmbeddedCharcodeFromUnicode(const FXCMAP_CMap* pEmbedMap,
                                     CIDSet charset,
                                     FX_WCHAR unicode) {
  if (!IsValidEmbeddedCharcodeFromUnicodeCharset(charset))
    return 0;

  CPDF_FontGlobals* pFontGlobals =
      CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals();
  const FX_WORD* pCodes = pFontGlobals->m_EmbeddedToUnicodes[charset].m_pMap;
  if (!pCodes)
    return 0;

  int nCodes = pFontGlobals->m_EmbeddedToUnicodes[charset].m_Count;
  for (int i = 0; i < nCodes; ++i) {
    if (pCodes[i] == unicode) {
      FX_DWORD CharCode = FPDFAPI_CharCodeFromCID(pEmbedMap, i);
      if (CharCode != 0) {
        return CharCode;
      }
    }
  }
  return 0;
}
#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_

FX_WCHAR EmbeddedUnicodeFromCharcode(const FXCMAP_CMap* pEmbedMap,
                                     CIDSet charset,
                                     FX_DWORD charcode) {
  if (!IsValidEmbeddedCharcodeFromUnicodeCharset(charset))
    return 0;

  FX_WORD cid = FPDFAPI_CIDFromCharCode(pEmbedMap, charcode);
  if (cid == 0)
    return 0;

  CPDF_FontGlobals* pFontGlobals =
      CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals();
  const FX_WORD* pCodes = pFontGlobals->m_EmbeddedToUnicodes[charset].m_pMap;
  if (!pCodes)
    return 0;

  if (cid < pFontGlobals->m_EmbeddedToUnicodes[charset].m_Count)
    return pCodes[cid];
  return 0;
}

void FT_UseCIDCharmap(FXFT_Face face, int coding) {
  int encoding;
  switch (coding) {
    case CIDCODING_GB:
      encoding = FXFT_ENCODING_GB2312;
      break;
    case CIDCODING_BIG5:
      encoding = FXFT_ENCODING_BIG5;
      break;
    case CIDCODING_JIS:
      encoding = FXFT_ENCODING_SJIS;
      break;
    case CIDCODING_KOREA:
      encoding = FXFT_ENCODING_JOHAB;
      break;
    default:
      encoding = FXFT_ENCODING_UNICODE;
  }
  int err = FXFT_Select_Charmap(face, encoding);
  if (err) {
    err = FXFT_Select_Charmap(face, FXFT_ENCODING_UNICODE);
  }
  if (err && FXFT_Get_Face_Charmaps(face)) {
    FXFT_Set_Charmap(face, *FXFT_Get_Face_Charmaps(face));
  }
}

const struct CIDTransform {
  FX_WORD CID;
  uint8_t a, b, c, d, e, f;
} g_Japan1_VertCIDs[] = {
    {97, 129, 0, 0, 127, 55, 0},
    {7887, 127, 0, 0, 127, 76, 89},
    {7888, 127, 0, 0, 127, 79, 94},
    {7889, 0, 129, 127, 0, 17, 127},
    {7890, 0, 129, 127, 0, 17, 127},
    {7891, 0, 129, 127, 0, 17, 127},
    {7892, 0, 129, 127, 0, 17, 127},
    {7893, 0, 129, 127, 0, 17, 127},
    {7894, 0, 129, 127, 0, 17, 127},
    {7895, 0, 129, 127, 0, 17, 127},
    {7896, 0, 129, 127, 0, 17, 127},
    {7897, 0, 129, 127, 0, 17, 127},
    {7898, 0, 129, 127, 0, 17, 127},
    {7899, 0, 129, 127, 0, 17, 104},
    {7900, 0, 129, 127, 0, 17, 127},
    {7901, 0, 129, 127, 0, 17, 104},
    {7902, 0, 129, 127, 0, 17, 127},
    {7903, 0, 129, 127, 0, 17, 127},
    {7904, 0, 129, 127, 0, 17, 127},
    {7905, 0, 129, 127, 0, 17, 114},
    {7906, 0, 129, 127, 0, 17, 127},
    {7907, 0, 129, 127, 0, 17, 127},
    {7908, 0, 129, 127, 0, 17, 127},
    {7909, 0, 129, 127, 0, 17, 127},
    {7910, 0, 129, 127, 0, 17, 127},
    {7911, 0, 129, 127, 0, 17, 127},
    {7912, 0, 129, 127, 0, 17, 127},
    {7913, 0, 129, 127, 0, 17, 127},
    {7914, 0, 129, 127, 0, 17, 127},
    {7915, 0, 129, 127, 0, 17, 114},
    {7916, 0, 129, 127, 0, 17, 127},
    {7917, 0, 129, 127, 0, 17, 127},
    {7918, 127, 0, 0, 127, 18, 25},
    {7919, 127, 0, 0, 127, 18, 25},
    {7920, 127, 0, 0, 127, 18, 25},
    {7921, 127, 0, 0, 127, 18, 25},
    {7922, 127, 0, 0, 127, 18, 25},
    {7923, 127, 0, 0, 127, 18, 25},
    {7924, 127, 0, 0, 127, 18, 25},
    {7925, 127, 0, 0, 127, 18, 25},
    {7926, 127, 0, 0, 127, 18, 25},
    {7927, 127, 0, 0, 127, 18, 25},
    {7928, 127, 0, 0, 127, 18, 25},
    {7929, 127, 0, 0, 127, 18, 25},
    {7930, 127, 0, 0, 127, 18, 25},
    {7931, 127, 0, 0, 127, 18, 25},
    {7932, 127, 0, 0, 127, 18, 25},
    {7933, 127, 0, 0, 127, 18, 25},
    {7934, 127, 0, 0, 127, 18, 25},
    {7935, 127, 0, 0, 127, 18, 25},
    {7936, 127, 0, 0, 127, 18, 25},
    {7937, 127, 0, 0, 127, 18, 25},
    {7938, 127, 0, 0, 127, 18, 25},
    {7939, 127, 0, 0, 127, 18, 25},
    {8720, 0, 129, 127, 0, 19, 102},
    {8721, 0, 129, 127, 0, 13, 127},
    {8722, 0, 129, 127, 0, 19, 108},
    {8723, 0, 129, 127, 0, 19, 102},
    {8724, 0, 129, 127, 0, 19, 102},
    {8725, 0, 129, 127, 0, 19, 102},
    {8726, 0, 129, 127, 0, 19, 102},
    {8727, 0, 129, 127, 0, 19, 102},
    {8728, 0, 129, 127, 0, 19, 114},
    {8729, 0, 129, 127, 0, 19, 114},
    {8730, 0, 129, 127, 0, 38, 108},
    {8731, 0, 129, 127, 0, 13, 108},
    {8732, 0, 129, 127, 0, 19, 108},
    {8733, 0, 129, 127, 0, 19, 108},
    {8734, 0, 129, 127, 0, 19, 108},
    {8735, 0, 129, 127, 0, 19, 108},
    {8736, 0, 129, 127, 0, 19, 102},
    {8737, 0, 129, 127, 0, 19, 102},
    {8738, 0, 129, 127, 0, 19, 102},
    {8739, 0, 129, 127, 0, 19, 102},
    {8740, 0, 129, 127, 0, 19, 102},
    {8741, 0, 129, 127, 0, 19, 102},
    {8742, 0, 129, 127, 0, 19, 102},
    {8743, 0, 129, 127, 0, 19, 102},
    {8744, 0, 129, 127, 0, 19, 102},
    {8745, 0, 129, 127, 0, 19, 102},
    {8746, 0, 129, 127, 0, 19, 114},
    {8747, 0, 129, 127, 0, 19, 114},
    {8748, 0, 129, 127, 0, 19, 102},
    {8749, 0, 129, 127, 0, 19, 102},
    {8750, 0, 129, 127, 0, 19, 102},
    {8751, 0, 129, 127, 0, 19, 102},
    {8752, 0, 129, 127, 0, 19, 102},
    {8753, 0, 129, 127, 0, 19, 102},
    {8754, 0, 129, 127, 0, 19, 102},
    {8755, 0, 129, 127, 0, 19, 102},
    {8756, 0, 129, 127, 0, 19, 102},
    {8757, 0, 129, 127, 0, 19, 102},
    {8758, 0, 129, 127, 0, 19, 102},
    {8759, 0, 129, 127, 0, 19, 102},
    {8760, 0, 129, 127, 0, 19, 102},
    {8761, 0, 129, 127, 0, 19, 102},
    {8762, 0, 129, 127, 0, 19, 102},
    {8763, 0, 129, 127, 0, 19, 102},
    {8764, 0, 129, 127, 0, 19, 102},
    {8765, 0, 129, 127, 0, 19, 102},
    {8766, 0, 129, 127, 0, 19, 102},
    {8767, 0, 129, 127, 0, 19, 102},
    {8768, 0, 129, 127, 0, 19, 102},
    {8769, 0, 129, 127, 0, 19, 102},
    {8770, 0, 129, 127, 0, 19, 102},
    {8771, 0, 129, 127, 0, 19, 102},
    {8772, 0, 129, 127, 0, 19, 102},
    {8773, 0, 129, 127, 0, 19, 102},
    {8774, 0, 129, 127, 0, 19, 102},
    {8775, 0, 129, 127, 0, 19, 102},
    {8776, 0, 129, 127, 0, 19, 102},
    {8777, 0, 129, 127, 0, 19, 102},
    {8778, 0, 129, 127, 0, 19, 102},
    {8779, 0, 129, 127, 0, 19, 114},
    {8780, 0, 129, 127, 0, 19, 108},
    {8781, 0, 129, 127, 0, 19, 114},
    {8782, 0, 129, 127, 0, 13, 114},
    {8783, 0, 129, 127, 0, 19, 108},
    {8784, 0, 129, 127, 0, 13, 114},
    {8785, 0, 129, 127, 0, 19, 108},
    {8786, 0, 129, 127, 0, 19, 108},
    {8787, 0, 129, 127, 0, 19, 108},
    {8788, 0, 129, 127, 0, 19, 108},
    {8789, 0, 129, 127, 0, 19, 108},
    {8790, 0, 129, 127, 0, 19, 108},
    {8791, 0, 129, 127, 0, 19, 108},
    {8792, 0, 129, 127, 0, 19, 108},
    {8793, 0, 129, 127, 0, 19, 108},
    {8794, 0, 129, 127, 0, 19, 108},
    {8795, 0, 129, 127, 0, 19, 108},
    {8796, 0, 129, 127, 0, 19, 108},
    {8797, 0, 129, 127, 0, 19, 108},
    {8798, 0, 129, 127, 0, 19, 108},
    {8799, 0, 129, 127, 0, 19, 108},
    {8800, 0, 129, 127, 0, 19, 108},
    {8801, 0, 129, 127, 0, 19, 108},
    {8802, 0, 129, 127, 0, 19, 108},
    {8803, 0, 129, 127, 0, 19, 108},
    {8804, 0, 129, 127, 0, 19, 108},
    {8805, 0, 129, 127, 0, 19, 108},
    {8806, 0, 129, 127, 0, 19, 108},
    {8807, 0, 129, 127, 0, 19, 108},
    {8808, 0, 129, 127, 0, 19, 108},
    {8809, 0, 129, 127, 0, 19, 108},
    {8810, 0, 129, 127, 0, 19, 108},
    {8811, 0, 129, 127, 0, 19, 114},
    {8812, 0, 129, 127, 0, 19, 102},
    {8813, 0, 129, 127, 0, 19, 114},
    {8814, 0, 129, 127, 0, 76, 102},
    {8815, 0, 129, 127, 0, 13, 121},
    {8816, 0, 129, 127, 0, 19, 114},
    {8817, 0, 129, 127, 0, 19, 127},
    {8818, 0, 129, 127, 0, 19, 114},
    {8819, 0, 129, 127, 0, 218, 108},
};

int CompareCIDTransform(const void* key, const void* element) {
  FX_WORD CID = *static_cast<const FX_WORD*>(key);
  return CID - static_cast<const struct CIDTransform*>(element)->CID;
}

}  // namespace

CPDF_CMapManager::CPDF_CMapManager() {
  m_bPrompted = FALSE;
  FXSYS_memset(m_CID2UnicodeMaps, 0, sizeof m_CID2UnicodeMaps);
}
CPDF_CMapManager::~CPDF_CMapManager() {
  for (const auto& pair : m_CMaps) {
    delete pair.second;
  }
  m_CMaps.clear();
  for (size_t i = 0; i < FX_ArraySize(m_CID2UnicodeMaps); ++i) {
    delete m_CID2UnicodeMaps[i];
  }
}
CPDF_CMap* CPDF_CMapManager::GetPredefinedCMap(const CFX_ByteString& name,
                                               FX_BOOL bPromptCJK) {
  auto it = m_CMaps.find(name);
  if (it != m_CMaps.end()) {
    return it->second;
  }
  CPDF_CMap* pCMap = LoadPredefinedCMap(name, bPromptCJK);
  if (!name.IsEmpty()) {
    m_CMaps[name] = pCMap;
  }
  return pCMap;
}
CPDF_CMap* CPDF_CMapManager::LoadPredefinedCMap(const CFX_ByteString& name,
                                                FX_BOOL bPromptCJK) {
  CPDF_CMap* pCMap = new CPDF_CMap;
  const FX_CHAR* pname = name;
  if (*pname == '/') {
    pname++;
  }
  pCMap->LoadPredefined(this, pname, bPromptCJK);
  return pCMap;
}

void CPDF_CMapManager::ReloadAll() {
  for (const auto& pair : m_CMaps) {
    CPDF_CMap* pCMap = pair.second;
    pCMap->LoadPredefined(this, pair.first, FALSE);
  }
  for (size_t i = 0; i < FX_ArraySize(m_CID2UnicodeMaps); ++i) {
    if (CPDF_CID2UnicodeMap* pMap = m_CID2UnicodeMaps[i]) {
      pMap->Load(this, CIDSetFromSizeT(i), FALSE);
    }
  }
}
CPDF_CID2UnicodeMap* CPDF_CMapManager::GetCID2UnicodeMap(CIDSet charset,
                                                         FX_BOOL bPromptCJK) {
  if (!m_CID2UnicodeMaps[charset])
    m_CID2UnicodeMaps[charset] = LoadCID2UnicodeMap(charset, bPromptCJK);
  return m_CID2UnicodeMaps[charset];
}
CPDF_CID2UnicodeMap* CPDF_CMapManager::LoadCID2UnicodeMap(CIDSet charset,
                                                          FX_BOOL bPromptCJK) {
  CPDF_CID2UnicodeMap* pMap = new CPDF_CID2UnicodeMap();
  if (!pMap->Initialize()) {
    delete pMap;
    return NULL;
  }
  pMap->Load(this, charset, bPromptCJK);
  return pMap;
}
CPDF_CMapParser::CPDF_CMapParser() {
  m_pCMap = NULL;
  m_Status = 0;
  m_CodeSeq = 0;
}
FX_BOOL CPDF_CMapParser::Initialize(CPDF_CMap* pCMap) {
  m_pCMap = pCMap;
  m_Status = 0;
  m_CodeSeq = 0;
  m_AddMaps.EstimateSize(0, 10240);
  return TRUE;
}

void CPDF_CMapParser::ParseWord(const CFX_ByteStringC& word) {
  if (word.IsEmpty()) {
    return;
  }
  if (word == "begincidchar") {
    m_Status = 1;
    m_CodeSeq = 0;
  } else if (word == "begincidrange") {
    m_Status = 2;
    m_CodeSeq = 0;
  } else if (word == "endcidrange" || word == "endcidchar") {
    m_Status = 0;
  } else if (word == "/WMode") {
    m_Status = 6;
  } else if (word == "/Registry") {
    m_Status = 3;
  } else if (word == "/Ordering") {
    m_Status = 4;
  } else if (word == "/Supplement") {
    m_Status = 5;
  } else if (word == "begincodespacerange") {
    m_Status = 7;
    m_CodeSeq = 0;
  } else if (word == "usecmap") {
  } else if (m_Status == 1 || m_Status == 2) {
    m_CodePoints[m_CodeSeq] = CMap_GetCode(word);
    m_CodeSeq++;
    FX_DWORD StartCode, EndCode;
    FX_WORD StartCID;
    if (m_Status == 1) {
      if (m_CodeSeq < 2) {
        return;
      }
      EndCode = StartCode = m_CodePoints[0];
      StartCID = (FX_WORD)m_CodePoints[1];
    } else {
      if (m_CodeSeq < 3) {
        return;
      }
      StartCode = m_CodePoints[0];
      EndCode = m_CodePoints[1];
      StartCID = (FX_WORD)m_CodePoints[2];
    }
    if (EndCode < 0x10000) {
      for (FX_DWORD code = StartCode; code <= EndCode; code++) {
        m_pCMap->m_pMapping[code] = (FX_WORD)(StartCID + code - StartCode);
      }
    } else {
      FX_DWORD buf[2];
      buf[0] = StartCode;
      buf[1] = ((EndCode - StartCode) << 16) + StartCID;
      m_AddMaps.AppendBlock(buf, sizeof buf);
    }
    m_CodeSeq = 0;
  } else if (m_Status == 3) {
    CMap_GetString(word);
    m_Status = 0;
  } else if (m_Status == 4) {
    m_pCMap->m_Charset = CharsetFromOrdering(CMap_GetString(word));
    m_Status = 0;
  } else if (m_Status == 5) {
    CMap_GetCode(word);
    m_Status = 0;
  } else if (m_Status == 6) {
    m_pCMap->m_bVertical = CMap_GetCode(word);
    m_Status = 0;
  } else if (m_Status == 7) {
    if (word == "endcodespacerange") {
      int nSegs = m_CodeRanges.GetSize();
      if (nSegs > 1) {
        m_pCMap->m_CodingScheme = CPDF_CMap::MixedFourBytes;
        m_pCMap->m_nCodeRanges = nSegs;
        m_pCMap->m_pLeadingBytes =
            FX_Alloc2D(uint8_t, nSegs, sizeof(CMap_CodeRange));
        FXSYS_memcpy(m_pCMap->m_pLeadingBytes, m_CodeRanges.GetData(),
                     nSegs * sizeof(CMap_CodeRange));
      } else if (nSegs == 1) {
        m_pCMap->m_CodingScheme = (m_CodeRanges[0].m_CharSize == 2)
                                      ? CPDF_CMap::TwoBytes
                                      : CPDF_CMap::OneByte;
      }
      m_Status = 0;
    } else {
      if (word.GetLength() == 0 || word.GetAt(0) != '<') {
        return;
      }
      if (m_CodeSeq % 2) {
        CMap_CodeRange range;
        if (CMap_GetCodeRange(range, m_LastWord, word)) {
          m_CodeRanges.Add(range);
        }
      }
      m_CodeSeq++;
    }
  }
  m_LastWord = word;
}

// Static.
FX_DWORD CPDF_CMapParser::CMap_GetCode(const CFX_ByteStringC& word) {
  int num = 0;
  if (word.GetAt(0) == '<') {
    for (int i = 1; i < word.GetLength() && std::isxdigit(word.GetAt(i)); ++i)
      num = num * 16 + FXSYS_toHexDigit(word.GetAt(i));
    return num;
  }

  for (int i = 0; i < word.GetLength() && std::isdigit(word.GetAt(i)); ++i)
    num = num * 10 + FXSYS_toDecimalDigit(word.GetAt(i));
  return num;
}

// Static.
bool CPDF_CMapParser::CMap_GetCodeRange(CMap_CodeRange& range,
                                        const CFX_ByteStringC& first,
                                        const CFX_ByteStringC& second) {
  if (first.GetLength() == 0 || first.GetAt(0) != '<')
    return false;

  int i;
  for (i = 1; i < first.GetLength(); ++i) {
    if (first.GetAt(i) == '>') {
      break;
    }
  }
  range.m_CharSize = (i - 1) / 2;
  if (range.m_CharSize > 4)
    return false;

  for (i = 0; i < range.m_CharSize; ++i) {
    uint8_t digit1 = first.GetAt(i * 2 + 1);
    uint8_t digit2 = first.GetAt(i * 2 + 2);
    range.m_Lower[i] = FXSYS_toHexDigit(digit1) * 16 + FXSYS_toHexDigit(digit2);
  }

  FX_DWORD size = second.GetLength();
  for (i = 0; i < range.m_CharSize; ++i) {
    uint8_t digit1 = ((FX_DWORD)i * 2 + 1 < size)
                         ? second.GetAt((FX_STRSIZE)i * 2 + 1)
                         : '0';
    uint8_t digit2 = ((FX_DWORD)i * 2 + 2 < size)
                         ? second.GetAt((FX_STRSIZE)i * 2 + 2)
                         : '0';
    range.m_Upper[i] = FXSYS_toHexDigit(digit1) * 16 + FXSYS_toHexDigit(digit2);
  }
  return true;
}

CPDF_CMap::CPDF_CMap() {
  m_Charset = CIDSET_UNKNOWN;
  m_Coding = CIDCODING_UNKNOWN;
  m_CodingScheme = TwoBytes;
  m_bVertical = 0;
  m_bLoaded = FALSE;
  m_pMapping = NULL;
  m_pLeadingBytes = NULL;
  m_pAddMapping = NULL;
  m_pEmbedMap = NULL;
  m_pUseMap = NULL;
  m_nCodeRanges = 0;
}
CPDF_CMap::~CPDF_CMap() {
  FX_Free(m_pMapping);
  FX_Free(m_pAddMapping);
  FX_Free(m_pLeadingBytes);
  delete m_pUseMap;
}
void CPDF_CMap::Release() {
  if (m_PredefinedCMap.IsEmpty()) {
    delete this;
  }
}

FX_BOOL CPDF_CMap::LoadPredefined(CPDF_CMapManager* pMgr,
                                  const FX_CHAR* pName,
                                  FX_BOOL bPromptCJK) {
  m_PredefinedCMap = pName;
  if (m_PredefinedCMap == "Identity-H" || m_PredefinedCMap == "Identity-V") {
    m_Coding = CIDCODING_CID;
    m_bVertical = pName[9] == 'V';
    m_bLoaded = TRUE;
    return TRUE;
  }
  CFX_ByteString cmapid = m_PredefinedCMap;
  m_bVertical = cmapid.Right(1) == "V";
  if (cmapid.GetLength() > 2) {
    cmapid = cmapid.Left(cmapid.GetLength() - 2);
  }
  const CPDF_PredefinedCMap* map = nullptr;
  for (size_t i = 0; i < FX_ArraySize(g_PredefinedCMaps); ++i) {
    if (cmapid == CFX_ByteStringC(g_PredefinedCMaps[i].m_pName)) {
      map = &g_PredefinedCMaps[i];
      break;
    }
  }
  if (!map)
    return FALSE;

  m_Charset = map->m_Charset;
  m_Coding = map->m_Coding;
  m_CodingScheme = map->m_CodingScheme;
  if (m_CodingScheme == MixedTwoBytes) {
    m_pLeadingBytes = FX_Alloc(uint8_t, 256);
    for (FX_DWORD i = 0; i < map->m_LeadingSegCount; ++i) {
      const uint8_t* segs = map->m_LeadingSegs;
      for (int b = segs[i * 2]; b <= segs[i * 2 + 1]; ++b) {
        m_pLeadingBytes[b] = 1;
      }
    }
  }
  FPDFAPI_FindEmbeddedCMap(pName, m_Charset, m_Coding, m_pEmbedMap);
  if (m_pEmbedMap) {
    m_bLoaded = TRUE;
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CPDF_CMap::LoadEmbedded(const uint8_t* pData, FX_DWORD size) {
  m_pMapping = FX_Alloc(FX_WORD, 65536);
  CPDF_CMapParser parser;
  parser.Initialize(this);
  CPDF_SimpleParser syntax(pData, size);
  while (1) {
    CFX_ByteStringC word = syntax.GetWord();
    if (word.IsEmpty()) {
      break;
    }
    parser.ParseWord(word);
  }
  if (m_CodingScheme == MixedFourBytes && parser.m_AddMaps.GetSize()) {
    m_pAddMapping = FX_Alloc(uint8_t, parser.m_AddMaps.GetSize() + 4);
    *(FX_DWORD*)m_pAddMapping = parser.m_AddMaps.GetSize() / 8;
    FXSYS_memcpy(m_pAddMapping + 4, parser.m_AddMaps.GetBuffer(),
                 parser.m_AddMaps.GetSize());
    FXSYS_qsort(m_pAddMapping + 4, parser.m_AddMaps.GetSize() / 8, 8,
                CompareDWORD);
  }
  return TRUE;
}

FX_WORD CPDF_CMap::CIDFromCharCode(FX_DWORD charcode) const {
  if (m_Coding == CIDCODING_CID) {
    return (FX_WORD)charcode;
  }
  if (m_pEmbedMap) {
    return FPDFAPI_CIDFromCharCode(m_pEmbedMap, charcode);
  }
  if (!m_pMapping) {
    return (FX_WORD)charcode;
  }
  if (charcode >> 16) {
    if (m_pAddMapping) {
      void* found = FXSYS_bsearch(&charcode, m_pAddMapping + 4,
                                  *(FX_DWORD*)m_pAddMapping, 8, CompareCID);
      if (!found) {
        if (m_pUseMap) {
          return m_pUseMap->CIDFromCharCode(charcode);
        }
        return 0;
      }
      return (FX_WORD)(((FX_DWORD*)found)[1] % 65536 + charcode -
                       *(FX_DWORD*)found);
    }
    if (m_pUseMap)
      return m_pUseMap->CIDFromCharCode(charcode);
    return 0;
  }
  FX_DWORD CID = m_pMapping[charcode];
  if (!CID && m_pUseMap)
    return m_pUseMap->CIDFromCharCode(charcode);
  return (FX_WORD)CID;
}

FX_DWORD CPDF_CMap::GetNextChar(const FX_CHAR* pString,
                                int nStrLen,
                                int& offset) const {
  switch (m_CodingScheme) {
    case OneByte:
      return ((uint8_t*)pString)[offset++];
    case TwoBytes:
      offset += 2;
      return ((uint8_t*)pString)[offset - 2] * 256 +
             ((uint8_t*)pString)[offset - 1];
    case MixedTwoBytes: {
      uint8_t byte1 = ((uint8_t*)pString)[offset++];
      if (!m_pLeadingBytes[byte1]) {
        return byte1;
      }
      uint8_t byte2 = ((uint8_t*)pString)[offset++];
      return byte1 * 256 + byte2;
    }
    case MixedFourBytes: {
      uint8_t codes[4];
      int char_size = 1;
      codes[0] = ((uint8_t*)pString)[offset++];
      CMap_CodeRange* pRanges = (CMap_CodeRange*)m_pLeadingBytes;
      while (1) {
        int ret = CheckCodeRange(codes, char_size, pRanges, m_nCodeRanges);
        if (ret == 0) {
          return 0;
        }
        if (ret == 2) {
          FX_DWORD charcode = 0;
          for (int i = 0; i < char_size; i++) {
            charcode = (charcode << 8) + codes[i];
          }
          return charcode;
        }
        if (char_size == 4 || offset == nStrLen) {
          return 0;
        }
        codes[char_size++] = ((uint8_t*)pString)[offset++];
      }
      break;
    }
  }
  return 0;
}
int CPDF_CMap::GetCharSize(FX_DWORD charcode) const {
  switch (m_CodingScheme) {
    case OneByte:
      return 1;
    case TwoBytes:
      return 2;
    case MixedTwoBytes:
    case MixedFourBytes:
      if (charcode < 0x100) {
        return 1;
      }
      if (charcode < 0x10000) {
        return 2;
      }
      if (charcode < 0x1000000) {
        return 3;
      }
      return 4;
  }
  return 1;
}
int CPDF_CMap::CountChar(const FX_CHAR* pString, int size) const {
  switch (m_CodingScheme) {
    case OneByte:
      return size;
    case TwoBytes:
      return (size + 1) / 2;
    case MixedTwoBytes: {
      int count = 0;
      for (int i = 0; i < size; i++) {
        count++;
        if (m_pLeadingBytes[((uint8_t*)pString)[i]]) {
          i++;
        }
      }
      return count;
    }
    case MixedFourBytes: {
      int count = 0, offset = 0;
      while (offset < size) {
        GetNextChar(pString, size, offset);
        count++;
      }
      return count;
    }
  }
  return size;
}

int CPDF_CMap::AppendChar(FX_CHAR* str, FX_DWORD charcode) const {
  switch (m_CodingScheme) {
    case OneByte:
      str[0] = (uint8_t)charcode;
      return 1;
    case TwoBytes:
      str[0] = (uint8_t)(charcode / 256);
      str[1] = (uint8_t)(charcode % 256);
      return 2;
    case MixedTwoBytes:
    case MixedFourBytes:
      if (charcode < 0x100) {
        CMap_CodeRange* pRanges = (CMap_CodeRange*)m_pLeadingBytes;
        int iSize = GetCharSizeImpl(charcode, pRanges, m_nCodeRanges);
        if (iSize == 0) {
          iSize = 1;
        }
        if (iSize > 1) {
          FXSYS_memset(str, 0, sizeof(uint8_t) * iSize);
        }
        str[iSize - 1] = (uint8_t)charcode;
        return iSize;
      }
      if (charcode < 0x10000) {
        str[0] = (uint8_t)(charcode >> 8);
        str[1] = (uint8_t)charcode;
        return 2;
      }
      if (charcode < 0x1000000) {
        str[0] = (uint8_t)(charcode >> 16);
        str[1] = (uint8_t)(charcode >> 8);
        str[2] = (uint8_t)charcode;
        return 3;
      }
      str[0] = (uint8_t)(charcode >> 24);
      str[1] = (uint8_t)(charcode >> 16);
      str[2] = (uint8_t)(charcode >> 8);
      str[3] = (uint8_t)charcode;
      return 4;
  }
  return 0;
}
CPDF_CID2UnicodeMap::CPDF_CID2UnicodeMap() {
  m_EmbeddedCount = 0;
}
CPDF_CID2UnicodeMap::~CPDF_CID2UnicodeMap() {}
FX_BOOL CPDF_CID2UnicodeMap::Initialize() {
  return TRUE;
}
FX_BOOL CPDF_CID2UnicodeMap::IsLoaded() {
  return m_EmbeddedCount != 0;
}
FX_WCHAR CPDF_CID2UnicodeMap::UnicodeFromCID(FX_WORD CID) {
  if (m_Charset == CIDSET_UNICODE) {
    return CID;
  }
  if (CID < m_EmbeddedCount) {
    return m_pEmbeddedMap[CID];
  }
  return 0;
}

void CPDF_CID2UnicodeMap::Load(CPDF_CMapManager* pMgr,
                               CIDSet charset,
                               FX_BOOL bPromptCJK) {
  m_Charset = charset;
  FPDFAPI_LoadCID2UnicodeMap(charset, m_pEmbeddedMap, m_EmbeddedCount);
}

#include "ttgsubtable.h"
CPDF_CIDFont::CPDF_CIDFont() : CPDF_Font(PDFFONT_CIDFONT) {
  m_pCMap = NULL;
  m_pAllocatedCMap = NULL;
  m_pCID2UnicodeMap = NULL;
  m_pAnsiWidths = NULL;
  m_pCIDToGIDMap = NULL;
  m_bCIDIsGID = FALSE;
  m_bAdobeCourierStd = FALSE;
  m_pTTGSUBTable = NULL;
  FXSYS_memset(m_CharBBox, 0xff, 256 * sizeof(FX_SMALL_RECT));
}
CPDF_CIDFont::~CPDF_CIDFont() {
  if (m_pAnsiWidths) {
    FX_Free(m_pAnsiWidths);
  }
  delete m_pAllocatedCMap;
  delete m_pCIDToGIDMap;
  delete m_pTTGSUBTable;
}
FX_WORD CPDF_CIDFont::CIDFromCharCode(FX_DWORD charcode) const {
  if (!m_pCMap) {
    return (FX_WORD)charcode;
  }
  return m_pCMap->CIDFromCharCode(charcode);
}
FX_BOOL CPDF_CIDFont::IsVertWriting() const {
  return m_pCMap ? m_pCMap->IsVertWriting() : FALSE;
}

FX_WCHAR CPDF_CIDFont::_UnicodeFromCharCode(FX_DWORD charcode) const {
  switch (m_pCMap->m_Coding) {
    case CIDCODING_UCS2:
    case CIDCODING_UTF16:
      return (FX_WCHAR)charcode;
    case CIDCODING_CID:
      if (!m_pCID2UnicodeMap || !m_pCID2UnicodeMap->IsLoaded()) {
        return 0;
      }
      return m_pCID2UnicodeMap->UnicodeFromCID((FX_WORD)charcode);
  }
  if (!m_pCMap->IsLoaded() || !m_pCID2UnicodeMap ||
      !m_pCID2UnicodeMap->IsLoaded()) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    FX_WCHAR unicode;
    int charsize = 1;
    if (charcode > 255) {
      charcode = (charcode % 256) * 256 + (charcode / 256);
      charsize = 2;
    }
    int ret = FXSYS_MultiByteToWideChar(g_CharsetCPs[m_pCMap->m_Coding], 0,
                                        (const FX_CHAR*)&charcode, charsize,
                                        &unicode, 1);
    if (ret != 1) {
      return 0;
    }
    return unicode;
#endif
    if (m_pCMap->m_pEmbedMap) {
      return EmbeddedUnicodeFromCharcode(m_pCMap->m_pEmbedMap,
                                         m_pCMap->m_Charset, charcode);
    }
    return 0;
  }
  return m_pCID2UnicodeMap->UnicodeFromCID(CIDFromCharCode(charcode));
}
FX_DWORD CPDF_CIDFont::_CharCodeFromUnicode(FX_WCHAR unicode) const {
  switch (m_pCMap->m_Coding) {
    case CIDCODING_UNKNOWN:
      return 0;
    case CIDCODING_UCS2:
    case CIDCODING_UTF16:
      return unicode;
    case CIDCODING_CID: {
      if (!m_pCID2UnicodeMap || !m_pCID2UnicodeMap->IsLoaded()) {
        return 0;
      }
      FX_DWORD CID = 0;
      while (CID < 65536) {
        FX_WCHAR this_unicode = m_pCID2UnicodeMap->UnicodeFromCID((FX_WORD)CID);
        if (this_unicode == unicode) {
          return CID;
        }
        CID++;
      }
      break;
    }
  }

  if (unicode < 0x80) {
    return static_cast<FX_DWORD>(unicode);
  }
  if (m_pCMap->m_Coding == CIDCODING_CID) {
    return 0;
  }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  uint8_t buffer[32];
  int ret =
      FXSYS_WideCharToMultiByte(g_CharsetCPs[m_pCMap->m_Coding], 0, &unicode, 1,
                                (char*)buffer, 4, NULL, NULL);
  if (ret == 1) {
    return buffer[0];
  }
  if (ret == 2) {
    return buffer[0] * 256 + buffer[1];
  }
#else
  if (m_pCMap->m_pEmbedMap) {
    return EmbeddedCharcodeFromUnicode(m_pCMap->m_pEmbedMap, m_pCMap->m_Charset,
                                       unicode);
  }
#endif
  return 0;
}

FX_BOOL CPDF_CIDFont::_Load() {
  if (m_pFontDict->GetString("Subtype") == "TrueType") {
    return LoadGB2312();
  }
  CPDF_Array* pFonts = m_pFontDict->GetArray("DescendantFonts");
  if (!pFonts) {
    return FALSE;
  }
  if (pFonts->GetCount() != 1) {
    return FALSE;
  }
  CPDF_Dictionary* pCIDFontDict = pFonts->GetDict(0);
  if (!pCIDFontDict) {
    return FALSE;
  }
  m_BaseFont = pCIDFontDict->GetString("BaseFont");
  if ((m_BaseFont.Compare("CourierStd") == 0 ||
       m_BaseFont.Compare("CourierStd-Bold") == 0 ||
       m_BaseFont.Compare("CourierStd-BoldOblique") == 0 ||
       m_BaseFont.Compare("CourierStd-Oblique") == 0) &&
      !IsEmbedded()) {
    m_bAdobeCourierStd = TRUE;
  }
  CPDF_Dictionary* pFontDesc = pCIDFontDict->GetDict("FontDescriptor");
  if (pFontDesc) {
    LoadFontDescriptor(pFontDesc);
  }
  CPDF_Object* pEncoding = m_pFontDict->GetElementValue("Encoding");
  if (!pEncoding) {
    return FALSE;
  }
  CFX_ByteString subtype = pCIDFontDict->GetString("Subtype");
  m_bType1 = (subtype == "CIDFontType0");

  if (pEncoding->IsName()) {
    CFX_ByteString cmap = pEncoding->GetString();
    m_pCMap =
        CPDF_ModuleMgr::Get()
            ->GetPageModule()
            ->GetFontGlobals()
            ->m_CMapManager.GetPredefinedCMap(cmap, m_pFontFile && m_bType1);
  } else if (CPDF_Stream* pStream = pEncoding->AsStream()) {
    m_pAllocatedCMap = m_pCMap = new CPDF_CMap;
    CPDF_StreamAcc acc;
    acc.LoadAllData(pStream, FALSE);
    m_pCMap->LoadEmbedded(acc.GetData(), acc.GetSize());
  } else {
    return FALSE;
  }
  if (!m_pCMap) {
    return FALSE;
  }
  m_Charset = m_pCMap->m_Charset;
  if (m_Charset == CIDSET_UNKNOWN) {
    CPDF_Dictionary* pCIDInfo = pCIDFontDict->GetDict("CIDSystemInfo");
    if (pCIDInfo) {
      m_Charset = CharsetFromOrdering(pCIDInfo->GetString("Ordering"));
    }
  }
  if (m_Charset != CIDSET_UNKNOWN)
    m_pCID2UnicodeMap =
        CPDF_ModuleMgr::Get()
            ->GetPageModule()
            ->GetFontGlobals()
            ->m_CMapManager.GetCID2UnicodeMap(
                m_Charset,
                !m_pFontFile && (m_pCMap->m_Coding == CIDCODING_CID ||
                                 pCIDFontDict->KeyExist("W")));
  if (m_Font.GetFace()) {
    if (m_bType1) {
      FXFT_Select_Charmap(m_Font.GetFace(), FXFT_ENCODING_UNICODE);
    } else {
      FT_UseCIDCharmap(m_Font.GetFace(), m_pCMap->m_Coding);
    }
  }
  m_DefaultWidth = pCIDFontDict->GetInteger("DW", 1000);
  CPDF_Array* pWidthArray = pCIDFontDict->GetArray("W");
  if (pWidthArray) {
    LoadMetricsArray(pWidthArray, m_WidthList, 1);
  }
  if (!IsEmbedded()) {
    LoadSubstFont();
  }
  if (1) {
    if (m_pFontFile || (GetSubstFont()->m_SubstFlags & FXFONT_SUBST_EXACT)) {
      CPDF_Object* pmap = pCIDFontDict->GetElementValue("CIDToGIDMap");
      if (pmap) {
        if (CPDF_Stream* pStream = pmap->AsStream()) {
          m_pCIDToGIDMap = new CPDF_StreamAcc;
          m_pCIDToGIDMap->LoadAllData(pStream, FALSE);
        } else if (pmap->GetString() == "Identity") {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
          if (m_pFontFile) {
            m_bCIDIsGID = TRUE;
          }
#else
          m_bCIDIsGID = TRUE;
#endif
        }
      }
    }
  }
  CheckFontMetrics();
  if (IsVertWriting()) {
    pWidthArray = pCIDFontDict->GetArray("W2");
    if (pWidthArray) {
      LoadMetricsArray(pWidthArray, m_VertMetrics, 3);
    }
    CPDF_Array* pDefaultArray = pCIDFontDict->GetArray("DW2");
    if (pDefaultArray) {
      m_DefaultVY = pDefaultArray->GetInteger(0);
      m_DefaultW1 = pDefaultArray->GetInteger(1);
    } else {
      m_DefaultVY = 880;
      m_DefaultW1 = -1000;
    }
  }
  return TRUE;
}

void CPDF_CIDFont::GetCharBBox(FX_DWORD charcode, FX_RECT& rect, int level) {
  if (charcode < 256 && m_CharBBox[charcode].Right != -1) {
    rect.bottom = m_CharBBox[charcode].Bottom;
    rect.left = m_CharBBox[charcode].Left;
    rect.right = m_CharBBox[charcode].Right;
    rect.top = m_CharBBox[charcode].Top;
    return;
  }
  FX_BOOL bVert = FALSE;
  int glyph_index = GlyphFromCharCode(charcode, &bVert);
  FXFT_Face face = m_Font.GetFace();
  if (face) {
    rect.left = rect.bottom = rect.right = rect.top = 0;
    if (FXFT_Is_Face_Tricky(face)) {
      int err = FXFT_Load_Glyph(face, glyph_index,
                                FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
      if (!err) {
        FXFT_BBox cbox;
        FXFT_Glyph glyph;
        err = FXFT_Get_Glyph(((FXFT_Face)face)->glyph, &glyph);
        if (!err) {
          FXFT_Glyph_Get_CBox(glyph, FXFT_GLYPH_BBOX_PIXELS, &cbox);
          int pixel_size_x = ((FXFT_Face)face)->size->metrics.x_ppem;
          int pixel_size_y = ((FXFT_Face)face)->size->metrics.y_ppem;
          if (pixel_size_x == 0 || pixel_size_y == 0) {
            rect.left = cbox.xMin;
            rect.right = cbox.xMax;
            rect.top = cbox.yMax;
            rect.bottom = cbox.yMin;
          } else {
            rect.left = cbox.xMin * 1000 / pixel_size_x;
            rect.right = cbox.xMax * 1000 / pixel_size_x;
            rect.top = cbox.yMax * 1000 / pixel_size_y;
            rect.bottom = cbox.yMin * 1000 / pixel_size_y;
          }
          if (rect.top > FXFT_Get_Face_Ascender(face)) {
            rect.top = FXFT_Get_Face_Ascender(face);
          }
          if (rect.bottom < FXFT_Get_Face_Descender(face)) {
            rect.bottom = FXFT_Get_Face_Descender(face);
          }
          FXFT_Done_Glyph(glyph);
        }
      }
    } else {
      int err = FXFT_Load_Glyph(face, glyph_index, FXFT_LOAD_NO_SCALE);
      if (err == 0) {
        rect.left = TT2PDF(FXFT_Get_Glyph_HoriBearingX(face), face);
        rect.right = TT2PDF(
            FXFT_Get_Glyph_HoriBearingX(face) + FXFT_Get_Glyph_Width(face),
            face);
        rect.top = TT2PDF(FXFT_Get_Glyph_HoriBearingY(face), face);
        rect.top += rect.top / 64;
        rect.bottom = TT2PDF(
            FXFT_Get_Glyph_HoriBearingY(face) - FXFT_Get_Glyph_Height(face),
            face);
      }
    }
  } else {
    rect = FX_RECT(0, 0, 0, 0);
  }
  if (!m_pFontFile && m_Charset == CIDSET_JAPAN1) {
    FX_WORD CID = CIDFromCharCode(charcode);
    const uint8_t* pTransform = GetCIDTransform(CID);
    if (pTransform && !bVert) {
      CFX_Matrix matrix(CIDTransformToFloat(pTransform[0]),
                        CIDTransformToFloat(pTransform[1]),
                        CIDTransformToFloat(pTransform[2]),
                        CIDTransformToFloat(pTransform[3]),
                        CIDTransformToFloat(pTransform[4]) * 1000,
                        CIDTransformToFloat(pTransform[5]) * 1000);
      CFX_FloatRect rect_f(rect);
      rect_f.Transform(&matrix);
      rect = rect_f.GetOutterRect();
    }
  }
  if (charcode < 256) {
    m_CharBBox[charcode].Bottom = (short)rect.bottom;
    m_CharBBox[charcode].Left = (short)rect.left;
    m_CharBBox[charcode].Right = (short)rect.right;
    m_CharBBox[charcode].Top = (short)rect.top;
  }
}
int CPDF_CIDFont::GetCharWidthF(FX_DWORD charcode, int level) {
  if (m_pAnsiWidths && charcode < 0x80) {
    return m_pAnsiWidths[charcode];
  }
  FX_WORD cid = CIDFromCharCode(charcode);
  int size = m_WidthList.GetSize();
  FX_DWORD* list = m_WidthList.GetData();
  for (int i = 0; i < size; i += 3) {
    if (cid >= list[i] && cid <= list[i + 1]) {
      return (int)list[i + 2];
    }
  }
  return m_DefaultWidth;
}
short CPDF_CIDFont::GetVertWidth(FX_WORD CID) const {
  FX_DWORD vertsize = m_VertMetrics.GetSize() / 5;
  if (vertsize == 0) {
    return m_DefaultW1;
  }
  const FX_DWORD* pTable = m_VertMetrics.GetData();
  for (FX_DWORD i = 0; i < vertsize; i++)
    if (pTable[i * 5] <= CID && pTable[i * 5 + 1] >= CID) {
      return (short)(int)pTable[i * 5 + 2];
    }
  return m_DefaultW1;
}
void CPDF_CIDFont::GetVertOrigin(FX_WORD CID, short& vx, short& vy) const {
  FX_DWORD vertsize = m_VertMetrics.GetSize() / 5;
  if (vertsize) {
    const FX_DWORD* pTable = m_VertMetrics.GetData();
    for (FX_DWORD i = 0; i < vertsize; i++)
      if (pTable[i * 5] <= CID && pTable[i * 5 + 1] >= CID) {
        vx = (short)(int)pTable[i * 5 + 3];
        vy = (short)(int)pTable[i * 5 + 4];
        return;
      }
  }
  FX_DWORD dwWidth = m_DefaultWidth;
  int size = m_WidthList.GetSize();
  const FX_DWORD* list = m_WidthList.GetData();
  for (int i = 0; i < size; i += 3) {
    if (CID >= list[i] && CID <= list[i + 1]) {
      dwWidth = (FX_WORD)list[i + 2];
      break;
    }
  }
  vx = (short)dwWidth / 2;
  vy = (short)m_DefaultVY;
}
int CPDF_CIDFont::GetGlyphIndex(FX_DWORD unicode, FX_BOOL* pVertGlyph) {
  if (pVertGlyph) {
    *pVertGlyph = FALSE;
  }
  FXFT_Face face = m_Font.GetFace();
  int index = FXFT_Get_Char_Index(face, unicode);
  if (unicode == 0x2502) {
    return index;
  }
  if (index && IsVertWriting()) {
    if (m_pTTGSUBTable) {
      uint32_t vindex = 0;
      m_pTTGSUBTable->GetVerticalGlyph(index, &vindex);
      if (vindex) {
        index = vindex;
        if (pVertGlyph) {
          *pVertGlyph = TRUE;
        }
      }
      return index;
    }
    if (!m_Font.GetSubData()) {
      unsigned long length = 0;
      int error = FXFT_Load_Sfnt_Table(face, FT_MAKE_TAG('G', 'S', 'U', 'B'), 0,
                                       NULL, &length);
      if (!error) {
        m_Font.SetSubData(FX_Alloc(uint8_t, length));
      }
    }
    int error = FXFT_Load_Sfnt_Table(face, FT_MAKE_TAG('G', 'S', 'U', 'B'), 0,
                                     m_Font.GetSubData(), NULL);
    if (!error && m_Font.GetSubData()) {
      m_pTTGSUBTable = new CFX_CTTGSUBTable;
      m_pTTGSUBTable->LoadGSUBTable((FT_Bytes)m_Font.GetSubData());
      uint32_t vindex = 0;
      m_pTTGSUBTable->GetVerticalGlyph(index, &vindex);
      if (vindex) {
        index = vindex;
        if (pVertGlyph) {
          *pVertGlyph = TRUE;
        }
      }
    }
    return index;
  }
  if (pVertGlyph) {
    *pVertGlyph = FALSE;
  }
  return index;
}
int CPDF_CIDFont::GlyphFromCharCode(FX_DWORD charcode, FX_BOOL* pVertGlyph) {
  if (pVertGlyph) {
    *pVertGlyph = FALSE;
  }
  if (!m_pFontFile && !m_pCIDToGIDMap) {
    FX_WORD cid = CIDFromCharCode(charcode);
    FX_WCHAR unicode = 0;
    if (m_bCIDIsGID) {
#if _FXM_PLATFORM_ != _FXM_PLATFORM_APPLE_
      return cid;
#else
      if (m_Flags & PDFFONT_SYMBOLIC) {
        return cid;
      }
      CFX_WideString uni_str = UnicodeFromCharCode(charcode);
      if (uni_str.IsEmpty()) {
        return cid;
      }
      unicode = uni_str.GetAt(0);
#endif
    } else {
      if (cid && m_pCID2UnicodeMap && m_pCID2UnicodeMap->IsLoaded()) {
        unicode = m_pCID2UnicodeMap->UnicodeFromCID(cid);
      }
      if (unicode == 0) {
        unicode = _UnicodeFromCharCode(charcode);
      }
      if (unicode == 0 && !(m_Flags & PDFFONT_SYMBOLIC)) {
        unicode = UnicodeFromCharCode(charcode).GetAt(0);
      }
    }
    FXFT_Face face = m_Font.GetFace();
    if (unicode == 0) {
      if (!m_bAdobeCourierStd) {
        return charcode == 0 ? -1 : (int)charcode;
      }
      charcode += 31;
      int index = 0, iBaseEncoding;
      FX_BOOL bMSUnicode = FT_UseTTCharmap(face, 3, 1);
      FX_BOOL bMacRoman = FALSE;
      if (!bMSUnicode) {
        bMacRoman = FT_UseTTCharmap(face, 1, 0);
      }
      iBaseEncoding = PDFFONT_ENCODING_STANDARD;
      if (bMSUnicode) {
        iBaseEncoding = PDFFONT_ENCODING_WINANSI;
      } else if (bMacRoman) {
        iBaseEncoding = PDFFONT_ENCODING_MACROMAN;
      }
      const FX_CHAR* name = GetAdobeCharName(iBaseEncoding, NULL, charcode);
      if (!name) {
        return charcode == 0 ? -1 : (int)charcode;
      }
      FX_WORD unicode = PDF_UnicodeFromAdobeName(name);
      if (unicode) {
        if (bMSUnicode) {
          index = FXFT_Get_Char_Index(face, unicode);
        } else if (bMacRoman) {
          FX_DWORD maccode =
              FT_CharCodeFromUnicode(FXFT_ENCODING_APPLE_ROMAN, unicode);
          index = !maccode ? FXFT_Get_Name_Index(face, (char*)name)
                           : FXFT_Get_Char_Index(face, maccode);
        } else {
          return FXFT_Get_Char_Index(face, unicode);
        }
      } else {
        return charcode == 0 ? -1 : (int)charcode;
      }
      if (index == 0 || index == 0xffff) {
        return charcode == 0 ? -1 : (int)charcode;
      }
      return index;
    }
    if (m_Charset == CIDSET_JAPAN1) {
      if (unicode == '\\') {
        unicode = '/';
      }
#if _FXM_PLATFORM_ != _FXM_PLATFORM_APPLE_
      else if (unicode == 0xa5) {
        unicode = 0x5c;
      }
#endif
    }
    if (!face)
      return unicode;

    int err = FXFT_Select_Charmap(face, FXFT_ENCODING_UNICODE);
    if (err != 0) {
      int i;
      for (i = 0; i < FXFT_Get_Face_CharmapCount(face); i++) {
        FX_DWORD ret = FT_CharCodeFromUnicode(
            FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmaps(face)[i]),
            (FX_WCHAR)charcode);
        if (ret == 0) {
          continue;
        }
        FXFT_Set_Charmap(face, FXFT_Get_Face_Charmaps(face)[i]);
        unicode = (FX_WCHAR)ret;
        break;
      }
      if (i == FXFT_Get_Face_CharmapCount(face) && i) {
        FXFT_Set_Charmap(face, FXFT_Get_Face_Charmaps(face)[0]);
        unicode = (FX_WCHAR)charcode;
      }
    }
    if (FXFT_Get_Face_Charmap(face)) {
      int index = GetGlyphIndex(unicode, pVertGlyph);
      if (index == 0)
        return -1;
      return index;
    }
    return unicode;
  }
  if (!m_Font.GetFace())
    return -1;

  FX_WORD cid = CIDFromCharCode(charcode);
  if (m_bType1) {
    if (!m_pCIDToGIDMap) {
      return cid;
    }
  } else {
    if (!m_pCIDToGIDMap) {
      if (m_pFontFile && !m_pCMap->m_pMapping)
        return cid;
      if (m_pCMap->m_Coding == CIDCODING_UNKNOWN ||
          !FXFT_Get_Face_Charmap(m_Font.GetFace())) {
        return cid;
      }
      if (FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmap(m_Font.GetFace())) ==
          FXFT_ENCODING_UNICODE) {
        CFX_WideString unicode_str = UnicodeFromCharCode(charcode);
        if (unicode_str.IsEmpty()) {
          return -1;
        }
        charcode = unicode_str.GetAt(0);
      }
      return GetGlyphIndex(charcode, pVertGlyph);
    }
  }
  FX_DWORD byte_pos = cid * 2;
  if (byte_pos + 2 > m_pCIDToGIDMap->GetSize())
    return -1;

  const uint8_t* pdata = m_pCIDToGIDMap->GetData() + byte_pos;
  return pdata[0] * 256 + pdata[1];
}
FX_DWORD CPDF_CIDFont::GetNextChar(const FX_CHAR* pString,
                                   int nStrLen,
                                   int& offset) const {
  return m_pCMap->GetNextChar(pString, nStrLen, offset);
}
int CPDF_CIDFont::GetCharSize(FX_DWORD charcode) const {
  return m_pCMap->GetCharSize(charcode);
}
int CPDF_CIDFont::CountChar(const FX_CHAR* pString, int size) const {
  return m_pCMap->CountChar(pString, size);
}
int CPDF_CIDFont::AppendChar(FX_CHAR* str, FX_DWORD charcode) const {
  return m_pCMap->AppendChar(str, charcode);
}
FX_BOOL CPDF_CIDFont::IsUnicodeCompatible() const {
  if (!m_pCMap->IsLoaded() || !m_pCID2UnicodeMap ||
      !m_pCID2UnicodeMap->IsLoaded()) {
    return m_pCMap->m_Coding != CIDCODING_UNKNOWN;
  }
  return TRUE;
}
FX_BOOL CPDF_CIDFont::IsFontStyleFromCharCode(FX_DWORD charcode) const {
  return TRUE;
}
void CPDF_CIDFont::LoadSubstFont() {
  m_Font.LoadSubst(m_BaseFont, !m_bType1, m_Flags, m_StemV * 5, m_ItalicAngle,
                   g_CharsetCPs[m_Charset], IsVertWriting());
}
void CPDF_CIDFont::LoadMetricsArray(CPDF_Array* pArray,
                                    CFX_DWordArray& result,
                                    int nElements) {
  int width_status = 0;
  int iCurElement = 0;
  int first_code = 0, last_code;
  FX_DWORD count = pArray->GetCount();
  for (FX_DWORD i = 0; i < count; i++) {
    CPDF_Object* pObj = pArray->GetElementValue(i);
    if (!pObj)
      continue;

    if (CPDF_Array* pArray = pObj->AsArray()) {
      if (width_status != 1)
        return;

      FX_DWORD count = pArray->GetCount();
      for (FX_DWORD j = 0; j < count; j += nElements) {
        result.Add(first_code);
        result.Add(first_code);
        for (int k = 0; k < nElements; k++) {
          result.Add(pArray->GetInteger(j + k));
        }
        first_code++;
      }
      width_status = 0;
    } else {
      if (width_status == 0) {
        first_code = pObj->GetInteger();
        width_status = 1;
      } else if (width_status == 1) {
        last_code = pObj->GetInteger();
        width_status = 2;
        iCurElement = 0;
      } else {
        if (!iCurElement) {
          result.Add(first_code);
          result.Add(last_code);
        }
        result.Add(pObj->GetInteger());
        iCurElement++;
        if (iCurElement == nElements) {
          width_status = 0;
        }
      }
    }
  }
}

// static
FX_FLOAT CPDF_CIDFont::CIDTransformToFloat(uint8_t ch) {
  if (ch < 128) {
    return ch * 1.0f / 127;
  }
  return (-255 + ch) * 1.0f / 127;
}

FX_BOOL CPDF_CIDFont::LoadGB2312() {
  m_BaseFont = m_pFontDict->GetString("BaseFont");
  CPDF_Dictionary* pFontDesc = m_pFontDict->GetDict("FontDescriptor");
  if (pFontDesc) {
    LoadFontDescriptor(pFontDesc);
  }
  m_Charset = CIDSET_GB1;
  m_bType1 = FALSE;
  m_pCMap = CPDF_ModuleMgr::Get()
                ->GetPageModule()
                ->GetFontGlobals()
                ->m_CMapManager.GetPredefinedCMap("GBK-EUC-H", FALSE);
  m_pCID2UnicodeMap = CPDF_ModuleMgr::Get()
                          ->GetPageModule()
                          ->GetFontGlobals()
                          ->m_CMapManager.GetCID2UnicodeMap(m_Charset, FALSE);
  if (!IsEmbedded()) {
    LoadSubstFont();
  }
  CheckFontMetrics();
  m_DefaultWidth = 1000;
  m_pAnsiWidths = FX_Alloc(FX_WORD, 128);
  for (int i = 32; i < 127; i++) {
    m_pAnsiWidths[i] = 500;
  }
  return TRUE;
}

const uint8_t* CPDF_CIDFont::GetCIDTransform(FX_WORD CID) const {
  if (m_Charset != CIDSET_JAPAN1 || m_pFontFile)
    return nullptr;

  const struct CIDTransform* found = (const struct CIDTransform*)FXSYS_bsearch(
      &CID, g_Japan1_VertCIDs, FX_ArraySize(g_Japan1_VertCIDs),
      sizeof(g_Japan1_VertCIDs[0]), CompareCIDTransform);
  return found ? &found->a : nullptr;
}
