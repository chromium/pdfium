// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "cmap_int.h"

#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_resource.h"
#include "core/src/fpdfapi/fpdf_font/font_int.h"

void FPDFAPI_FindEmbeddedCMap(const char* name,
                              int charset,
                              int coding,
                              const FXCMAP_CMap*& pMap) {
  pMap = NULL;
  CPDF_FontGlobals* pFontGlobals =
      CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals();
  const FXCMAP_CMap* pCMaps =
      pFontGlobals->m_EmbeddedCharsets[charset].m_pMapList;
  int nCMaps = pFontGlobals->m_EmbeddedCharsets[charset].m_Count;
  for (int i = 0; i < nCMaps; i++) {
    if (FXSYS_strcmp(name, pCMaps[i].m_Name)) {
      continue;
    }
    pMap = &pCMaps[i];
    break;
  }
}
extern "C" {
static int compareWord(const void* p1, const void* p2) {
  return (*(FX_WORD*)p1) - (*(FX_WORD*)p2);
}
};
extern "C" {
static int compareWordRange(const void* key, const void* element) {
  if (*(FX_WORD*)key < *(FX_WORD*)element) {
    return -1;
  }
  if (*(FX_WORD*)key > ((FX_WORD*)element)[1]) {
    return 1;
  }
  return 0;
}
};
extern "C" {
static int compareDWordRange(const void* p1, const void* p2) {
  FX_DWORD key = *(FX_DWORD*)p1;
  FX_WORD hiword = (FX_WORD)(key >> 16);
  FX_WORD* element = (FX_WORD*)p2;
  if (hiword < element[0]) {
    return -1;
  }
  if (hiword > element[0]) {
    return 1;
  }
  FX_WORD loword = (FX_WORD)key;
  if (loword < element[1]) {
    return -1;
  }
  if (loword > element[2]) {
    return 1;
  }
  return 0;
}
};
extern "C" {
static int compareDWordSingle(const void* p1, const void* p2) {
  FX_DWORD key = *(FX_DWORD*)p1;
  FX_DWORD value = ((*(FX_WORD*)p2) << 16) | ((FX_WORD*)p2)[1];
  if (key < value) {
    return -1;
  }
  if (key > value) {
    return 1;
  }
  return 0;
}
};
FX_WORD FPDFAPI_CIDFromCharCode(const FXCMAP_CMap* pMap, FX_DWORD charcode) {
  if (charcode >> 16) {
    while (1) {
      if (pMap->m_DWordMapType == FXCMAP_CMap::Range) {
        FX_WORD* found =
            (FX_WORD*)FXSYS_bsearch(&charcode, pMap->m_pDWordMap,
                                    pMap->m_DWordCount, 8, compareDWordRange);
        if (found) {
          return found[3] + (FX_WORD)charcode - found[1];
        }
      } else if (pMap->m_DWordMapType == FXCMAP_CMap::Single) {
        FX_WORD* found =
            (FX_WORD*)FXSYS_bsearch(&charcode, pMap->m_pDWordMap,
                                    pMap->m_DWordCount, 6, compareDWordSingle);
        if (found) {
          return found[2];
        }
      }
      if (pMap->m_UseOffset == 0) {
        return 0;
      }
      pMap = pMap + pMap->m_UseOffset;
    }
    return 0;
  }
  FX_WORD code = (FX_WORD)charcode;
  while (1) {
    if (!pMap->m_pWordMap) {
      return 0;
    }
    if (pMap->m_WordMapType == FXCMAP_CMap::Single) {
      FX_WORD* found = (FX_WORD*)FXSYS_bsearch(
          &code, pMap->m_pWordMap, pMap->m_WordCount, 4, compareWord);
      if (found) {
        return found[1];
      }
    } else if (pMap->m_WordMapType == FXCMAP_CMap::Range) {
      FX_WORD* found = (FX_WORD*)FXSYS_bsearch(
          &code, pMap->m_pWordMap, pMap->m_WordCount, 6, compareWordRange);
      if (found) {
        return found[2] + code - found[0];
      }
    }
    if (pMap->m_UseOffset == 0) {
      return 0;
    }
    pMap = pMap + pMap->m_UseOffset;
  }
  return 0;
}
FX_DWORD FPDFAPI_CharCodeFromCID(const FXCMAP_CMap* pMap, FX_WORD cid) {
  while (1) {
    if (pMap->m_WordMapType == FXCMAP_CMap::Single) {
      const FX_WORD* pCur = pMap->m_pWordMap;
      const FX_WORD* pEnd = pMap->m_pWordMap + pMap->m_WordCount * 2;
      while (pCur < pEnd) {
        if (pCur[1] == cid) {
          return pCur[0];
        }
        pCur += 2;
      }
    } else if (pMap->m_WordMapType == FXCMAP_CMap::Range) {
      const FX_WORD* pCur = pMap->m_pWordMap;
      const FX_WORD* pEnd = pMap->m_pWordMap + pMap->m_WordCount * 3;
      while (pCur < pEnd) {
        if (cid >= pCur[2] && cid <= pCur[2] + pCur[1] - pCur[0]) {
          return pCur[0] + cid - pCur[2];
        }
        pCur += 3;
      }
    }
    if (pMap->m_UseOffset == 0) {
      return 0;
    }
    pMap = pMap + pMap->m_UseOffset;
  }
  while (1) {
    if (pMap->m_DWordMapType == FXCMAP_CMap::Range) {
      const FX_WORD* pCur = pMap->m_pDWordMap;
      const FX_WORD* pEnd = pMap->m_pDWordMap + pMap->m_DWordCount * 4;
      while (pCur < pEnd) {
        if (cid >= pCur[3] && cid <= pCur[3] + pCur[2] - pCur[1]) {
          return (((FX_DWORD)pCur[0] << 16) | pCur[1]) + cid - pCur[3];
        }
        pCur += 4;
      }
    } else if (pMap->m_DWordMapType == FXCMAP_CMap::Single) {
      const FX_WORD* pCur = pMap->m_pDWordMap;
      const FX_WORD* pEnd = pMap->m_pDWordMap + pMap->m_DWordCount * 3;
      while (pCur < pEnd) {
        if (pCur[2] == cid) {
          return ((FX_DWORD)pCur[0] << 16) | pCur[1];
        }
        pCur += 3;
      }
    }
    if (pMap->m_UseOffset == 0) {
      return 0;
    }
    pMap = pMap + pMap->m_UseOffset;
  }
  return 0;
}

void FPDFAPI_LoadCID2UnicodeMap(CIDSet charset,
                                const FX_WORD*& pMap,
                                FX_DWORD& count) {
  CPDF_FontGlobals* pFontGlobals =
      CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals();
  pMap = pFontGlobals->m_EmbeddedToUnicodes[charset].m_pMap;
  count = pFontGlobals->m_EmbeddedToUnicodes[charset].m_Count;
}
