// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/fgas/src/fgas_base.h"
#include "fx_memory.h"
#define FX_4BYTEALIGN(size) (((size) + 3) / 4 * 4)
IFX_MEMAllocator* FX_CreateAllocator(FX_ALLOCTYPE eType,
                                     size_t chunkSize,
                                     size_t blockSize) {
  switch (eType) {
#ifndef _FXEMB
    case FX_ALLOCTYPE_Dynamic:
      return new CFX_DynamicStore(chunkSize);
#endif
    case FX_ALLOCTYPE_Default:
      return new CFX_DefStore();
    case FX_ALLOCTYPE_Static:
      return new CFX_StaticStore(chunkSize);
    case FX_ALLOCTYPE_Fixed:
      return new CFX_FixedStore(blockSize, chunkSize);
    default:
      return NULL;
  }
}
CFX_StaticStore::CFX_StaticStore(size_t iDefChunkSize)
    : m_iAllocatedSize(0),
      m_iDefChunkSize(iDefChunkSize),
      m_pChunk(NULL),
      m_pLastChunk(NULL) {
  FXSYS_assert(m_iDefChunkSize != 0);
}
CFX_StaticStore::~CFX_StaticStore() {
  FX_LPSTATICSTORECHUNK pChunk, pNext;
  pChunk = m_pChunk;
  while (pChunk != NULL) {
    pNext = pChunk->pNextChunk;
    FX_Free(pChunk);
    pChunk = pNext;
  }
}
FX_LPSTATICSTORECHUNK CFX_StaticStore::AllocChunk(size_t size) {
  FXSYS_assert(size != 0);
  FX_LPSTATICSTORECHUNK pChunk = (FX_LPSTATICSTORECHUNK)FX_Alloc(
      uint8_t, sizeof(FX_STATICSTORECHUNK) + size);
  pChunk->iChunkSize = size;
  pChunk->iFreeSize = size;
  pChunk->pNextChunk = NULL;
  if (m_pLastChunk == NULL) {
    m_pChunk = pChunk;
  } else {
    m_pLastChunk->pNextChunk = pChunk;
  }
  m_pLastChunk = pChunk;
  return pChunk;
}
FX_LPSTATICSTORECHUNK CFX_StaticStore::FindChunk(size_t size) {
  FXSYS_assert(size != 0);
  if (m_pLastChunk == NULL || m_pLastChunk->iFreeSize < size) {
    return AllocChunk(std::max(m_iDefChunkSize, size));
  }
  return m_pLastChunk;
}
void* CFX_StaticStore::Alloc(size_t size) {
  size = FX_4BYTEALIGN(size);
  FXSYS_assert(size != 0);
  FX_LPSTATICSTORECHUNK pChunk = FindChunk(size);
  FXSYS_assert(pChunk != NULL && pChunk->iFreeSize >= size);
  uint8_t* p = (uint8_t*)pChunk;
  p += sizeof(FX_STATICSTORECHUNK) + pChunk->iChunkSize - pChunk->iFreeSize;
  pChunk->iFreeSize -= size;
  m_iAllocatedSize += size;
  return p;
}
size_t CFX_StaticStore::SetDefChunkSize(size_t size) {
  FXSYS_assert(size != 0);
  size_t v = m_iDefChunkSize;
  m_iDefChunkSize = size;
  return v;
}
CFX_FixedStore::CFX_FixedStore(size_t iBlockSize, size_t iBlockNumsInChunk)
    : m_iBlockSize(FX_4BYTEALIGN(iBlockSize)),
      m_iDefChunkSize(FX_4BYTEALIGN(iBlockNumsInChunk)),
      m_pChunk(NULL) {
  FXSYS_assert(m_iBlockSize != 0 && m_iDefChunkSize != 0);
}
CFX_FixedStore::~CFX_FixedStore() {
  FX_LPFIXEDSTORECHUNK pChunk, pNext;
  pChunk = m_pChunk;
  while (pChunk != NULL) {
    pNext = pChunk->pNextChunk;
    FX_Free(pChunk);
    pChunk = pNext;
  }
}
FX_LPFIXEDSTORECHUNK CFX_FixedStore::AllocChunk() {
  int32_t iTotalSize = sizeof(FX_FIXEDSTORECHUNK) + m_iDefChunkSize +
                       m_iBlockSize * m_iDefChunkSize;
  FX_LPFIXEDSTORECHUNK pChunk =
      (FX_LPFIXEDSTORECHUNK)FX_Alloc(uint8_t, iTotalSize);
  if (pChunk == NULL) {
    return NULL;
  }
  FXSYS_memset(pChunk->FirstFlag(), 0, m_iDefChunkSize);
  pChunk->pNextChunk = m_pChunk;
  pChunk->iChunkSize = m_iDefChunkSize;
  pChunk->iFreeNum = m_iDefChunkSize;
  m_pChunk = pChunk;
  return pChunk;
}
void* CFX_FixedStore::Alloc(size_t size) {
  if (size > m_iBlockSize) {
    return NULL;
  }
  FX_LPFIXEDSTORECHUNK pChunk = m_pChunk;
  while (pChunk != NULL) {
    if (pChunk->iFreeNum > 0) {
      break;
    }
    pChunk = pChunk->pNextChunk;
  }
  if (pChunk == NULL) {
    pChunk = AllocChunk();
  }
  FXSYS_assert(pChunk != NULL);
  uint8_t* pFlags = pChunk->FirstFlag();
  size_t i = 0;
  for (; i < pChunk->iChunkSize; i++)
    if (pFlags[i] == 0) {
      break;
    }
  FXSYS_assert(i < pChunk->iChunkSize);
  pFlags[i] = 1;
  pChunk->iFreeNum--;
  return pChunk->FirstBlock() + i * m_iBlockSize;
}
void CFX_FixedStore::Free(void* pBlock) {
  FXSYS_assert(pBlock != NULL);
  FX_LPFIXEDSTORECHUNK pPrior, pChunk;
  pPrior = NULL, pChunk = m_pChunk;
  uint8_t* pStart = NULL;
  uint8_t* pEnd;
  while (pChunk != NULL) {
    pStart = pChunk->FirstBlock();
    if (pBlock >= pStart) {
      pEnd = pStart + m_iBlockSize * pChunk->iChunkSize;
      if (pBlock < pEnd) {
        break;
      }
    }
    pPrior = pChunk, pChunk = pChunk->pNextChunk;
  }
  FXSYS_assert(pChunk != NULL);
  size_t iPos = ((uint8_t*)pBlock - pStart) / m_iBlockSize;
  FXSYS_assert(iPos < pChunk->iChunkSize);
  uint8_t* pFlags = pChunk->FirstFlag();
  if (pFlags[iPos] == 0) {
    return;
  }
  pFlags[iPos] = 0;
  pChunk->iFreeNum++;
  if (pChunk->iFreeNum == pChunk->iChunkSize) {
    if (pPrior == NULL) {
      m_pChunk = pChunk->pNextChunk;
    } else {
      pPrior->pNextChunk = pChunk->pNextChunk;
    }
    FX_Free(pChunk);
  }
}
size_t CFX_FixedStore::SetDefChunkSize(size_t iChunkSize) {
  FXSYS_assert(iChunkSize != 0);
  size_t v = m_iDefChunkSize;
  m_iDefChunkSize = FX_4BYTEALIGN(iChunkSize);
  return v;
}
#ifndef _FXEMB
CFX_DynamicStore::CFX_DynamicStore(size_t iDefChunkSize)
    : m_iDefChunkSize(iDefChunkSize), m_pChunk(NULL) {
  FXSYS_assert(m_iDefChunkSize != 0);
}
CFX_DynamicStore::~CFX_DynamicStore() {
  FX_LPDYNAMICSTORECHUNK pChunk, pNext;
  pChunk = m_pChunk;
  while (pChunk != NULL) {
    pNext = pChunk->pNextChunk;
    FX_Free(pChunk);
    pChunk = pNext;
  }
}
FX_LPDYNAMICSTORECHUNK CFX_DynamicStore::AllocChunk(size_t size) {
  FXSYS_assert(size != 0);
  FX_LPDYNAMICSTORECHUNK pChunk = (FX_LPDYNAMICSTORECHUNK)FX_Alloc(
      uint8_t,
      sizeof(FX_DYNAMICSTORECHUNK) + sizeof(FX_DYNAMICSTOREBLOCK) * 2 + size);
  if (pChunk == NULL) {
    return NULL;
  }
  pChunk->iChunkSize = size;
  pChunk->iFreeSize = size;
  FX_LPDYNAMICSTOREBLOCK pBlock = pChunk->FirstBlock();
  pBlock->iBlockSize = size;
  pBlock->bUsed = FALSE;
  pBlock = pBlock->NextBlock();
  pBlock->iBlockSize = 0;
  pBlock->bUsed = TRUE;
  if (m_pChunk != NULL && size >= m_iDefChunkSize) {
    FX_LPDYNAMICSTORECHUNK pLast = m_pChunk;
    while (pLast->pNextChunk != NULL) {
      pLast = pLast->pNextChunk;
    }
    pLast->pNextChunk = pChunk;
    pChunk->pNextChunk = NULL;
  } else {
    pChunk->pNextChunk = m_pChunk;
    m_pChunk = pChunk;
  }
  return pChunk;
}
void* CFX_DynamicStore::Alloc(size_t size) {
  size = FX_4BYTEALIGN(size);
  FXSYS_assert(size != 0);
  FX_LPDYNAMICSTORECHUNK pChunk = m_pChunk;
  FX_LPDYNAMICSTOREBLOCK pBlock = NULL;
  while (pChunk != NULL) {
    if (pChunk->iFreeSize >= size) {
      pBlock = pChunk->FirstBlock();
      FX_BOOL bFind = FALSE;
      while (pBlock->iBlockSize != 0) {
        if (!pBlock->bUsed && pBlock->iBlockSize >= size) {
          bFind = TRUE;
          break;
        }
        pBlock = pBlock->NextBlock();
      }
      if (bFind) {
        break;
      }
    }
    pChunk = pChunk->pNextChunk;
  }
  if (pChunk == NULL) {
    pChunk = AllocChunk(std::max(m_iDefChunkSize, size));
    pBlock = pChunk->FirstBlock();
  }
  FXSYS_assert(pChunk != NULL && pBlock != NULL);
  size_t m = size + sizeof(FX_DYNAMICSTOREBLOCK);
  pBlock->bUsed = TRUE;
  if (pBlock->iBlockSize > m) {
    size_t n = pBlock->iBlockSize;
    pBlock->iBlockSize = size;
    FX_LPDYNAMICSTOREBLOCK pNextBlock = pBlock->NextBlock();
    pNextBlock->bUsed = FALSE;
    pNextBlock->iBlockSize = n - size - sizeof(FX_DYNAMICSTOREBLOCK);
    pChunk->iFreeSize -= size + sizeof(FX_DYNAMICSTOREBLOCK);
  } else {
    pChunk->iFreeSize -= pBlock->iBlockSize;
  }
  return pBlock->Data();
}
void CFX_DynamicStore::Free(void* pBlock) {
  FXSYS_assert(pBlock != NULL);
  FX_LPDYNAMICSTORECHUNK pPriorChunk, pChunk;
  pPriorChunk = NULL, pChunk = m_pChunk;
  while (pChunk != NULL) {
    if (pBlock > pChunk &&
        pBlock <= ((uint8_t*)pChunk + sizeof(FX_DYNAMICSTORECHUNK) +
                   pChunk->iChunkSize)) {
      break;
    }
    pPriorChunk = pChunk, pChunk = pChunk->pNextChunk;
  }
  FXSYS_assert(pChunk != NULL);
  FX_LPDYNAMICSTOREBLOCK pPriorBlock, pFindBlock;
  pPriorBlock = NULL, pFindBlock = pChunk->FirstBlock();
  while (pFindBlock->iBlockSize != 0) {
    if (pBlock == (void*)pFindBlock->Data()) {
      break;
    }
    pPriorBlock = pFindBlock;
    pFindBlock = pFindBlock->NextBlock();
  }
  FXSYS_assert(pFindBlock->iBlockSize != 0 && pFindBlock->bUsed &&
               pBlock == (void*)pFindBlock->Data());
  pFindBlock->bUsed = FALSE;
  pChunk->iFreeSize += pFindBlock->iBlockSize;
  if (pPriorBlock == NULL) {
    pPriorBlock = pChunk->FirstBlock();
  } else if (pPriorBlock->bUsed) {
    pPriorBlock = pFindBlock;
  }
  pFindBlock = pPriorBlock;
  size_t sizeFree = 0;
  size_t sizeBlock = 0;
  while (pFindBlock->iBlockSize != 0 && !pFindBlock->bUsed) {
    if (pFindBlock != pPriorBlock) {
      sizeFree += sizeof(FX_DYNAMICSTOREBLOCK);
      sizeBlock += sizeof(FX_DYNAMICSTOREBLOCK);
    }
    sizeBlock += pFindBlock->iBlockSize;
    pFindBlock = pFindBlock->NextBlock();
  }
  pPriorBlock->iBlockSize = sizeBlock;
  pChunk->iFreeSize += sizeFree;
  if (pChunk->iFreeSize == pChunk->iChunkSize) {
    if (pPriorChunk == NULL) {
      m_pChunk = pChunk->pNextChunk;
    } else {
      pPriorChunk->pNextChunk = pChunk->pNextChunk;
    }
    FX_Free(pChunk);
  }
}
size_t CFX_DynamicStore::SetDefChunkSize(size_t size) {
  FXSYS_assert(size != 0);
  size_t v = m_iDefChunkSize;
  m_iDefChunkSize = size;
  return v;
}
#endif
