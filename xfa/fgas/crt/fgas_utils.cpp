// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/crt/fgas_utils.h"

#include <algorithm>

#include "core/fxcrt/fx_basic.h"

struct FX_BASEDISCRETEARRAYDATA {
  int32_t iBlockSize;
  int32_t iChunkSize;
  int32_t iChunkCount;
  CFX_ArrayTemplate<uint8_t*> ChunkBuffer;
};

CFX_BaseDiscreteArray::CFX_BaseDiscreteArray(int32_t iChunkSize,
                                             int32_t iBlockSize) {
  ASSERT(iChunkSize > 0 && iBlockSize > 0);
  FX_BASEDISCRETEARRAYDATA* pData = new FX_BASEDISCRETEARRAYDATA;
  m_pData = pData;
  pData->ChunkBuffer.SetSize(16);
  pData->iChunkCount = 0;
  pData->iChunkSize = iChunkSize;
  pData->iBlockSize = iBlockSize;
}
CFX_BaseDiscreteArray::~CFX_BaseDiscreteArray() {
  RemoveAll();
  delete static_cast<FX_BASEDISCRETEARRAYDATA*>(m_pData);
}
uint8_t* CFX_BaseDiscreteArray::AddSpaceTo(int32_t index) {
  ASSERT(index > -1);
  FX_BASEDISCRETEARRAYDATA* pData = (FX_BASEDISCRETEARRAYDATA*)m_pData;
  int32_t& iChunkCount = pData->iChunkCount;
  int32_t iChunkSize = pData->iChunkSize;
  uint8_t* pChunk = nullptr;
  int32_t iChunk = index / iChunkSize;
  if (iChunk < iChunkCount) {
    pChunk = pData->ChunkBuffer.GetAt(iChunk);
  }
  if (!pChunk) {
    pChunk = FX_Alloc2D(uint8_t, iChunkSize, pData->iBlockSize);
    FXSYS_memset(pChunk, 0, iChunkSize * pData->iBlockSize);
    pData->ChunkBuffer.SetAtGrow(iChunk, pChunk);
    if (iChunkCount <= iChunk) {
      iChunkCount = iChunk + 1;
    }
  }
  return pChunk + (index % iChunkSize) * pData->iBlockSize;
}
uint8_t* CFX_BaseDiscreteArray::GetAt(int32_t index) const {
  ASSERT(index >= 0);
  FX_BASEDISCRETEARRAYDATA* pData = (FX_BASEDISCRETEARRAYDATA*)m_pData;
  int32_t iChunkSize = pData->iChunkSize;
  int32_t iChunk = index / iChunkSize;
  if (iChunk >= pData->iChunkCount)
    return nullptr;

  uint8_t* pChunk = pData->ChunkBuffer.GetAt(iChunk);
  if (!pChunk)
    return nullptr;

  return pChunk + (index % iChunkSize) * pData->iBlockSize;
}
void CFX_BaseDiscreteArray::RemoveAll() {
  FX_BASEDISCRETEARRAYDATA* pData = (FX_BASEDISCRETEARRAYDATA*)m_pData;
  CFX_ArrayTemplate<uint8_t*>& ChunkBuffer = pData->ChunkBuffer;
  int32_t& iChunkCount = pData->iChunkCount;
  for (int32_t i = 0; i < iChunkCount; i++)
    FX_Free(ChunkBuffer.GetAt(i));

  ChunkBuffer.RemoveAll();
  iChunkCount = 0;
}
