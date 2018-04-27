// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jbig2/JBig2_HuffmanTable.h"

#include <limits>
#include <vector>

#include "core/fxcodec/jbig2/JBig2_BitStream.h"
#include "core/fxcodec/jbig2/JBig2_Context.h"
#include "core/fxcodec/jbig2/JBig2_HuffmanTable_Standard.h"
#include "core/fxcrt/fx_memory.h"
#include "third_party/base/numerics/safe_math.h"

CJBig2_HuffmanTable::CJBig2_HuffmanTable(const JBig2TableLine* pTable,
                                         uint32_t nLines,
                                         bool bHTOOB)
    : HTOOB(bHTOOB), NTEMP(nLines) {
  m_bOK = ParseFromStandardTable(pTable);
  ASSERT(m_bOK);
}

CJBig2_HuffmanTable::CJBig2_HuffmanTable(CJBig2_BitStream* pStream)
    : HTOOB(false), NTEMP(0) {
  m_bOK = ParseFromCodedBuffer(pStream);
}

CJBig2_HuffmanTable::~CJBig2_HuffmanTable() {}

bool CJBig2_HuffmanTable::ParseFromStandardTable(const JBig2TableLine* pTable) {
  CODES.resize(NTEMP);
  RANGELEN.resize(NTEMP);
  RANGELOW.resize(NTEMP);
  for (uint32_t i = 0; i < NTEMP; ++i) {
    CODES[i].codelen = pTable[i].PREFLEN;
    RANGELEN[i] = pTable[i].RANDELEN;
    RANGELOW[i] = pTable[i].RANGELOW;
  }
  return CJBig2_Context::HuffmanAssignCode(CODES.data(), NTEMP);
}

bool CJBig2_HuffmanTable::ParseFromCodedBuffer(CJBig2_BitStream* pStream) {
  unsigned char cTemp;
  if (pStream->read1Byte(&cTemp) == -1)
    return false;

  HTOOB = !!(cTemp & 0x01);
  unsigned char HTPS = ((cTemp >> 1) & 0x07) + 1;
  unsigned char HTRS = ((cTemp >> 4) & 0x07) + 1;
  uint32_t HTLOW;
  uint32_t HTHIGH;
  if (pStream->readInteger(&HTLOW) == -1 ||
      pStream->readInteger(&HTHIGH) == -1) {
    return false;
  }

  const int low = static_cast<int>(HTLOW);
  const int high = static_cast<int>(HTHIGH);
  if (low > high)
    return false;

  ExtendBuffers(false);
  pdfium::base::CheckedNumeric<int> cur_low = low;
  do {
    if ((pStream->readNBits(HTPS, &CODES[NTEMP].codelen) == -1) ||
        (pStream->readNBits(HTRS, &RANGELEN[NTEMP]) == -1) ||
        (static_cast<size_t>(RANGELEN[NTEMP]) >= 8 * sizeof(cur_low))) {
      return false;
    }
    RANGELOW[NTEMP] = cur_low.ValueOrDie();

    if (RANGELEN[NTEMP] >= 32)
      return false;

    cur_low += (1 << RANGELEN[NTEMP]);
    if (!cur_low.IsValid())
      return false;
    ExtendBuffers(true);
  } while (cur_low.ValueOrDie() < high);

  if (pStream->readNBits(HTPS, &CODES[NTEMP].codelen) == -1)
    return false;

  RANGELEN[NTEMP] = 32;
  if (low == std::numeric_limits<int>::min())
    return false;

  RANGELOW[NTEMP] = low - 1;
  ExtendBuffers(true);

  if (pStream->readNBits(HTPS, &CODES[NTEMP].codelen) == -1)
    return false;

  RANGELEN[NTEMP] = 32;
  RANGELOW[NTEMP] = high;
  ExtendBuffers(true);

  if (HTOOB) {
    if (pStream->readNBits(HTPS, &CODES[NTEMP].codelen) == -1)
      return false;

    ++NTEMP;
  }

  return CJBig2_Context::HuffmanAssignCode(CODES.data(), NTEMP);
}

void CJBig2_HuffmanTable::ExtendBuffers(bool increment) {
  if (increment)
    ++NTEMP;

  size_t size = CODES.size();
  if (NTEMP < size)
    return;

  size += 16;
  ASSERT(NTEMP < size);
  CODES.resize(size);
  RANGELEN.resize(size);
  RANGELOW.resize(size);
}
