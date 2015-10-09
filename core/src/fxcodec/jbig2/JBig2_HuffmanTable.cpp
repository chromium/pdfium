// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_HuffmanTable.h"

#include <string.h>

#include <vector>

#include "../../../include/fxcrt/fx_memory.h"
#include "JBig2_BitStream.h"
#include "JBig2_Define.h"

CJBig2_HuffmanTable::CJBig2_HuffmanTable(const JBig2TableLine* pTable,
                                         int nLines,
                                         FX_BOOL bHTOOB) {
  init();
  m_bOK = parseFromStandardTable(pTable, nLines, bHTOOB);
}

CJBig2_HuffmanTable::CJBig2_HuffmanTable(CJBig2_BitStream* pStream) {
  init();
  m_bOK = parseFromCodedBuffer(pStream);
}

CJBig2_HuffmanTable::~CJBig2_HuffmanTable() {
  FX_Free(CODES);
  FX_Free(PREFLEN);
  FX_Free(RANGELEN);
  FX_Free(RANGELOW);
}
void CJBig2_HuffmanTable::init() {
  HTOOB = FALSE;
  NTEMP = 0;
  CODES = nullptr;
  PREFLEN = nullptr;
  RANGELEN = nullptr;
  RANGELOW = nullptr;
}
int CJBig2_HuffmanTable::parseFromStandardTable(const JBig2TableLine* pTable,
                                                int nLines,
                                                FX_BOOL bHTOOB) {
  HTOOB = bHTOOB;
  NTEMP = nLines;
  CODES = FX_Alloc(int, NTEMP);
  PREFLEN = FX_Alloc(int, NTEMP);
  RANGELEN = FX_Alloc(int, NTEMP);
  RANGELOW = FX_Alloc(int, NTEMP);
  int LENMAX = 0;
  for (FX_DWORD i = 0; i < NTEMP; ++i) {
    PREFLEN[i] = pTable[i].PREFLEN;
    RANGELEN[i] = pTable[i].RANDELEN;
    RANGELOW[i] = pTable[i].RANGELOW;
    if (PREFLEN[i] > LENMAX) {
      LENMAX = PREFLEN[i];
    }
  }
  int* LENCOUNT = FX_Alloc(int, LENMAX + 1);
  JBIG2_memset(LENCOUNT, 0, sizeof(int) * (LENMAX + 1));
  int* FIRSTCODE = FX_Alloc(int, LENMAX + 1);
  for (FX_DWORD i = 0; i < NTEMP; ++i)
    ++LENCOUNT[PREFLEN[i]];

  int CURLEN = 1;
  FIRSTCODE[0] = 0;
  LENCOUNT[0] = 0;
  while (CURLEN <= LENMAX) {
    FIRSTCODE[CURLEN] = (FIRSTCODE[CURLEN - 1] + LENCOUNT[CURLEN - 1]) << 1;
    int CURCODE = FIRSTCODE[CURLEN];
    FX_DWORD CURTEMP = 0;
    while (CURTEMP < NTEMP) {
      if (PREFLEN[CURTEMP] == CURLEN) {
        CODES[CURTEMP] = CURCODE;
        CURCODE = CURCODE + 1;
      }
      CURTEMP = CURTEMP + 1;
    }
    CURLEN = CURLEN + 1;
  }
  FX_Free(LENCOUNT);
  FX_Free(FIRSTCODE);
  return 1;
}

#define HT_CHECK_MEMORY_ADJUST                   \
  if (NTEMP >= nSize) {                          \
    nSize += 16;                                 \
    PREFLEN = FX_Realloc(int, PREFLEN, nSize);   \
    RANGELEN = FX_Realloc(int, RANGELEN, nSize); \
    RANGELOW = FX_Realloc(int, RANGELOW, nSize); \
  }
int CJBig2_HuffmanTable::parseFromCodedBuffer(CJBig2_BitStream* pStream) {
  unsigned char cTemp;
  if (pStream->read1Byte(&cTemp) == -1)
    return FALSE;

  HTOOB = cTemp & 0x01;
  unsigned char HTPS = ((cTemp >> 1) & 0x07) + 1;
  unsigned char HTRS = ((cTemp >> 4) & 0x07) + 1;
  FX_DWORD HTLOW;
  FX_DWORD HTHIGH;
  if (pStream->readInteger(&HTLOW) == -1 ||
      pStream->readInteger(&HTHIGH) == -1 || HTLOW > HTHIGH) {
    return FALSE;
  }

  FX_DWORD nSize = 16;
  PREFLEN = FX_Alloc(int, nSize);
  RANGELEN = FX_Alloc(int, nSize);
  RANGELOW = FX_Alloc(int, nSize);
  FX_DWORD CURRANGELOW = HTLOW;
  NTEMP = 0;
  do {
    HT_CHECK_MEMORY_ADJUST
    if ((pStream->readNBits(HTPS, &PREFLEN[NTEMP]) == -1) ||
        (pStream->readNBits(HTRS, &RANGELEN[NTEMP]) == -1)) {
      return FALSE;
    }
    RANGELOW[NTEMP] = CURRANGELOW;
    CURRANGELOW = CURRANGELOW + (1 << RANGELEN[NTEMP]);
    NTEMP = NTEMP + 1;
  } while (CURRANGELOW < HTHIGH);
  HT_CHECK_MEMORY_ADJUST
  if (pStream->readNBits(HTPS, &PREFLEN[NTEMP]) == -1)
    return FALSE;

  RANGELEN[NTEMP] = 32;
  RANGELOW[NTEMP] = HTLOW - 1;
  ++NTEMP;
  HT_CHECK_MEMORY_ADJUST
  if (pStream->readNBits(HTPS, &PREFLEN[NTEMP]) == -1)
    return FALSE;

  RANGELEN[NTEMP] = 32;
  RANGELOW[NTEMP] = HTHIGH;
  NTEMP = NTEMP + 1;
  if (HTOOB) {
    HT_CHECK_MEMORY_ADJUST
    if (pStream->readNBits(HTPS, &PREFLEN[NTEMP]) == -1)
      return FALSE;

    ++NTEMP;
  }
  CODES = FX_Alloc(int, NTEMP);
  int LENMAX = 0;
  for (FX_DWORD i = 0; i < NTEMP; ++i)
    LENMAX = std::max(PREFLEN[i], LENMAX);

  std::vector<int> LENCOUNT(LENMAX + 1);
  for (FX_DWORD i = 0; i < NTEMP; ++i)
    LENCOUNT[PREFLEN[i]]++;
  LENCOUNT[0] = 0;

  std::vector<int> FIRSTCODE(LENMAX + 1);
  FIRSTCODE[0] = 0;
  for (int i = 1; i <= LENMAX; ++i) {
    FIRSTCODE[i] = (FIRSTCODE[i - 1] + LENCOUNT[i - 1]) << 1;
    int CURCODE = FIRSTCODE[i];
    for (FX_DWORD j = 0; j < NTEMP; ++j) {
      if (PREFLEN[j] == i)
        CODES[j] = CURCODE++;
    }
  }
  return TRUE;
}
