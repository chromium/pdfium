// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../fgas_base.h"
#include "fx_utils.h"
CFX_ThreadLock::CFX_ThreadLock()
    : m_pData(NULL)
{
}
CFX_ThreadLock::~CFX_ThreadLock()
{
}
void CFX_ThreadLock::Lock()
{
}
void CFX_ThreadLock::Unlock()
{
}
typedef struct _FX_BASEARRAYDATA : public CFX_Target {
    FX_INT32	iGrowSize;
    FX_INT32	iBlockSize;
    FX_INT32	iTotalCount;
    FX_INT32	iBlockCount;
    FX_LPBYTE	pBuffer;
} FX_BASEARRAYDATA, * FX_LPBASEARRAYDATA;
typedef FX_BASEARRAYDATA const * FX_LPCBASEARRAYDATA;
CFX_BaseArray::CFX_BaseArray(FX_INT32 iGrowSize, FX_INT32 iBlockSize)
{
    FXSYS_assert(iGrowSize > 0 && iBlockSize > 0);
    m_pData = FXTARGET_New FX_BASEARRAYDATA;
    FX_memset(m_pData, 0, sizeof(FX_BASEARRAYDATA));
    ((FX_LPBASEARRAYDATA)m_pData)->iGrowSize = iGrowSize;
    ((FX_LPBASEARRAYDATA)m_pData)->iBlockSize = iBlockSize;
}
CFX_BaseArray::~CFX_BaseArray()
{
    FX_LPBASEARRAYDATA pData = (FX_LPBASEARRAYDATA)m_pData;
    if (pData->pBuffer != NULL) {
        FX_Free(pData->pBuffer);
    }
    FXTARGET_Delete pData;
}
FX_INT32 CFX_BaseArray::GetSize() const
{
    return ((FX_LPBASEARRAYDATA)m_pData)->iBlockCount;
}
FX_INT32 CFX_BaseArray::GetBlockSize() const
{
    return ((FX_LPBASEARRAYDATA)m_pData)->iBlockSize;
}
FX_LPBYTE CFX_BaseArray::AddSpaceTo(FX_INT32 index)
{
    FXSYS_assert(index > -1);
    FX_LPBYTE &pBuffer = ((FX_LPBASEARRAYDATA)m_pData)->pBuffer;
    FX_INT32 &iTotalCount = ((FX_LPBASEARRAYDATA)m_pData)->iTotalCount;
    FX_INT32 iBlockSize = ((FX_LPBASEARRAYDATA)m_pData)->iBlockSize;
    if (index >= iTotalCount) {
        FX_INT32 iGrowSize = ((FX_LPBASEARRAYDATA)m_pData)->iGrowSize;
        iTotalCount = (index / iGrowSize + 1) * iGrowSize;
        FX_INT32 iNewSize = iTotalCount * iBlockSize;
        if (pBuffer == NULL) {
            pBuffer = (FX_LPBYTE)FX_Alloc(FX_BYTE, iNewSize);
        } else {
            pBuffer = (FX_LPBYTE)FX_Realloc(FX_BYTE, pBuffer, iNewSize);
        }
    }
    FXSYS_assert(pBuffer != NULL);
    FX_INT32 &iBlockCount = ((FX_LPBASEARRAYDATA)m_pData)->iBlockCount;
    if (index >= iBlockCount) {
        iBlockCount = index + 1;
    }
    return pBuffer + index * iBlockSize;
}
FX_LPBYTE CFX_BaseArray::GetAt(FX_INT32 index) const
{
    FXSYS_assert(index > -1 && index < ((FX_LPBASEARRAYDATA)m_pData)->iBlockCount);
    return ((FX_LPBASEARRAYDATA)m_pData)->pBuffer + index * ((FX_LPBASEARRAYDATA)m_pData)->iBlockSize;
}
FX_LPBYTE CFX_BaseArray::GetBuffer() const
{
    return ((FX_LPBASEARRAYDATA)m_pData)->pBuffer;
}
FX_INT32 CFX_BaseArray::Append(const CFX_BaseArray &src, FX_INT32 iStart, FX_INT32 iCount)
{
    FX_INT32 iBlockSize = ((FX_LPBASEARRAYDATA)m_pData)->iBlockSize;
    FXSYS_assert(iBlockSize == ((FX_LPBASEARRAYDATA)src.m_pData)->iBlockSize);
    FX_INT32 &iBlockCount = ((FX_LPBASEARRAYDATA)m_pData)->iBlockCount;
    FX_INT32 iAdded = src.GetSize();
    FXSYS_assert(iStart > -1 && iStart < iAdded);
    if (iCount < 0) {
        iCount = iAdded;
    }
    if (iStart + iCount > iAdded) {
        iCount = iAdded - iStart;
    }
    if (iCount < 1) {
        return 0;
    }
    FX_LPBYTE pDst = ((FX_LPBASEARRAYDATA)m_pData)->pBuffer + iBlockCount * iBlockSize;
    AddSpaceTo(iBlockCount + iCount - 1);
    FX_memcpy(pDst, ((FX_LPBASEARRAYDATA)src.m_pData)->pBuffer + iStart * iBlockSize, iCount * iBlockSize);
    return iCount;
}
FX_INT32 CFX_BaseArray::Copy(const CFX_BaseArray &src, FX_INT32 iStart, FX_INT32 iCount)
{
    FX_INT32 iBlockSize = ((FX_LPBASEARRAYDATA)m_pData)->iBlockSize;
    FXSYS_assert(iBlockSize == ((FX_LPBASEARRAYDATA)src.m_pData)->iBlockSize);
    FX_INT32 iCopied = src.GetSize();
    FXSYS_assert(iStart > -1 && iStart < iCopied);
    if (iCount < 0) {
        iCount = iCopied;
    }
    if (iStart + iCount > iCopied) {
        iCount = iCopied - iStart;
    }
    if (iCount < 1) {
        return 0;
    }
    RemoveAll(TRUE);
    AddSpaceTo(iCount - 1);
    FX_memcpy(((FX_LPBASEARRAYDATA)m_pData)->pBuffer, ((FX_LPBASEARRAYDATA)src.m_pData)->pBuffer + iStart * iBlockSize, iCount * iBlockSize);
    return iCount;
}
FX_INT32 CFX_BaseArray::RemoveLast(FX_INT32 iCount)
{
    FX_INT32 &iBlockCount = ((FX_LPBASEARRAYDATA)m_pData)->iBlockCount;
    if (iCount < 0 || iCount > iBlockCount) {
        iCount = iBlockCount;
        iBlockCount = 0;
    } else {
        iBlockCount -= iCount;
    }
    return iCount;
}
void CFX_BaseArray::RemoveAll(FX_BOOL bLeaveMemory)
{
    if (!bLeaveMemory) {
        FX_LPBYTE &pBuffer = ((FX_LPBASEARRAYDATA)m_pData)->pBuffer;
        if (pBuffer != NULL) {
            FX_Free(pBuffer);
            pBuffer = NULL;
        }
        ((FX_LPBASEARRAYDATA)m_pData)->iTotalCount = 0;
    }
    ((FX_LPBASEARRAYDATA)m_pData)->iBlockCount = 0;
}
CFX_BaseMassArrayImp::CFX_BaseMassArrayImp(FX_INT32 iChunkSize, FX_INT32 iBlockSize)
    : m_iChunkSize(iChunkSize)
    , m_iBlockSize(iBlockSize)
    , m_iChunkCount(0)
    , m_iBlockCount(0)
{
    FXSYS_assert(m_iChunkSize > 0 && m_iBlockSize > 0);
    m_pData = FX_NEW CFX_PtrArray;
    m_pData->SetSize(16);
}
CFX_BaseMassArrayImp::~CFX_BaseMassArrayImp()
{
    RemoveAll();
    delete m_pData;
}
FX_LPBYTE CFX_BaseMassArrayImp::AddSpaceTo(FX_INT32 index)
{
    FXSYS_assert(index > -1);
    FX_LPBYTE pChunk;
    if (index < m_iBlockCount) {
        pChunk = (FX_LPBYTE)m_pData->GetAt(index / m_iChunkSize);
    } else {
        FX_INT32 iMemSize = m_iChunkSize * m_iBlockSize;
        while (TRUE) {
            if (index < m_iChunkCount * m_iChunkSize) {
                pChunk = (FX_LPBYTE)m_pData->GetAt(index / m_iChunkSize);
                break;
            } else {
                pChunk = (FX_LPBYTE)FX_Alloc(FX_BYTE, iMemSize);
                if (m_iChunkCount < m_pData->GetSize()) {
                    m_pData->SetAt(m_iChunkCount, pChunk);
                } else {
                    m_pData->Add(pChunk);
                }
                m_iChunkCount ++;
            }
        }
    }
    FXSYS_assert(pChunk != NULL);
    m_iBlockCount = index + 1;
    return pChunk + (index % m_iChunkSize) * m_iBlockSize;
}
FX_LPBYTE CFX_BaseMassArrayImp::GetAt(FX_INT32 index) const
{
    FXSYS_assert(index > -1 && index < m_iBlockCount);
    FX_LPBYTE pChunk = (FX_LPBYTE)m_pData->GetAt(index / m_iChunkSize);
    FXSYS_assert(pChunk != NULL);
    return pChunk + (index % m_iChunkSize) * m_iBlockSize;
}
FX_INT32 CFX_BaseMassArrayImp::Append(const CFX_BaseMassArrayImp &src, FX_INT32 iStart, FX_INT32 iCount)
{
    FXSYS_assert(m_iBlockSize == src.m_iBlockSize);
    FX_INT32 iAdded = src.m_iBlockCount;
    FXSYS_assert(iStart > -1 && iStart < iAdded);
    if (iCount < 0) {
        iCount = iAdded;
    }
    if (iStart + iCount > iAdded) {
        iCount = iAdded - iStart;
    }
    if (iCount < 1) {
        return m_iBlockCount;
    }
    FX_INT32 iBlockCount = m_iBlockCount;
    FX_INT32 iTotal = m_iBlockCount + iCount;
    AddSpaceTo(iTotal - 1);
    Append(iBlockCount, src, iStart, iCount);
    return m_iBlockCount;
}
FX_INT32 CFX_BaseMassArrayImp::Copy(const CFX_BaseMassArrayImp &src, FX_INT32 iStart, FX_INT32 iCount)
{
    FXSYS_assert(m_iBlockSize == src.m_iBlockSize);
    FX_INT32 iCopied = src.m_iBlockCount;
    FXSYS_assert(iStart > -1);
    if (iStart >= iCopied) {
        return 0;
    }
    RemoveAll(TRUE);
    if (iCount < 0) {
        iCount = iCopied;
    }
    if (iStart + iCount > iCopied) {
        iCount = iCopied - iStart;
    }
    if (iCount < 1) {
        return 0;
    }
    if (m_iBlockCount < iCount) {
        AddSpaceTo(iCount - 1);
    }
    Append(0, src, iStart, iCount);
    return m_iBlockCount;
}
void CFX_BaseMassArrayImp::Append(FX_INT32 iDstStart, const CFX_BaseMassArrayImp &src, FX_INT32 iSrcStart, FX_INT32 iSrcCount)
{
    FXSYS_assert(iDstStart > -1 && m_iBlockSize == src.m_iBlockSize);
    FX_INT32 iSrcTotal = src.m_iBlockCount;
    FXSYS_assert(iSrcTotal > 0 && m_iBlockCount >= iDstStart + iSrcCount);
    FXSYS_assert(iSrcStart > -1 && iSrcStart < iSrcTotal && iSrcCount > 0 && iSrcStart + iSrcCount <= iSrcTotal);
    FX_INT32 iDstChunkIndex = iDstStart / m_iChunkSize;
    FX_INT32 iSrcChunkIndex = iSrcStart / src.m_iChunkSize;
    FX_LPBYTE pDstChunk = (FX_LPBYTE)GetAt(iDstStart);
    FX_LPBYTE pSrcChunk = (FX_LPBYTE)src.GetAt(iSrcStart);
    FX_INT32 iDstChunkSize = m_iChunkSize - (iDstStart % m_iChunkSize);
    FX_INT32 iSrcChunkSize = src.m_iChunkSize - (iSrcStart % src.m_iChunkSize);
    FX_INT32 iCopySize = FX_MIN(iSrcCount, FX_MIN(iSrcChunkSize, iDstChunkSize));
    FX_INT32 iCopyBytes = iCopySize * m_iBlockSize;
    while (iSrcCount > 0) {
        FXSYS_assert(pDstChunk != NULL && pSrcChunk != NULL);
        FXSYS_memcpy(pDstChunk, pSrcChunk, iCopyBytes);
        iSrcCount -= iCopySize;
        iSrcChunkSize -= iCopySize;
        if (iSrcChunkSize < 1) {
            iSrcChunkSize = src.m_iChunkSize;
            iSrcChunkIndex ++;
            pSrcChunk = (FX_LPBYTE)src.m_pData->GetAt(iSrcChunkIndex);
        } else {
            pSrcChunk += iCopyBytes;
        }
        iDstChunkSize -= iCopySize;
        if (iDstChunkSize < 1) {
            iDstChunkSize = m_iChunkSize;
            iDstChunkIndex ++;
            pDstChunk = (FX_LPBYTE)m_pData->GetAt(iDstChunkIndex);
        } else {
            pDstChunk += iCopyBytes;
        }
        iCopySize = FX_MIN(iSrcCount, FX_MIN(iSrcChunkSize, iDstChunkSize));
        iCopyBytes = iCopySize * m_iBlockSize;
    }
}
FX_INT32 CFX_BaseMassArrayImp::RemoveLast(FX_INT32 iCount)
{
    if (iCount < 0 || iCount >= m_iBlockCount) {
        m_iBlockCount = 0;
    } else {
        m_iBlockCount -= iCount;
    }
    return m_iBlockCount;
}
void CFX_BaseMassArrayImp::RemoveAll(FX_BOOL bLeaveMemory)
{
    if (bLeaveMemory) {
        m_iBlockCount = 0;
        return;
    }
    for (FX_INT32 i = 0; i < m_iChunkCount; i ++) {
        FX_LPVOID p = m_pData->GetAt(i);
        if (p == NULL) {
            continue;
        }
        FX_Free(p);
    }
    m_pData->RemoveAll();
    m_iChunkCount = 0;
    m_iBlockCount = 0;
}
CFX_BaseMassArray::CFX_BaseMassArray(FX_INT32 iChunkSize, FX_INT32 iBlockSize)
{
    m_pData = FXTARGET_New CFX_BaseMassArrayImp(iChunkSize, iBlockSize);
}
CFX_BaseMassArray::~CFX_BaseMassArray()
{
    FXTARGET_Delete (CFX_BaseMassArrayImp*)m_pData;
}
FX_INT32 CFX_BaseMassArray::GetSize() const
{
    return ((CFX_BaseMassArrayImp*)m_pData)->m_iBlockCount;
}
FX_LPBYTE CFX_BaseMassArray::AddSpaceTo(FX_INT32 index)
{
    return ((CFX_BaseMassArrayImp*)m_pData)->AddSpaceTo(index);
}
FX_LPBYTE CFX_BaseMassArray::GetAt(FX_INT32 index) const
{
    return ((CFX_BaseMassArrayImp*)m_pData)->GetAt(index);
}
FX_INT32 CFX_BaseMassArray::Append(const CFX_BaseMassArray &src, FX_INT32 iStart, FX_INT32 iCount)
{
    return ((CFX_BaseMassArrayImp*)m_pData)->Append(*(CFX_BaseMassArrayImp*)src.m_pData, iStart, iCount);
}
FX_INT32 CFX_BaseMassArray::Copy(const CFX_BaseMassArray &src, FX_INT32 iStart, FX_INT32 iCount)
{
    return ((CFX_BaseMassArrayImp*)m_pData)->Copy(*(CFX_BaseMassArrayImp*)src.m_pData, iStart, iCount);
}
FX_INT32 CFX_BaseMassArray::RemoveLast(FX_INT32 iCount)
{
    return ((CFX_BaseMassArrayImp*)m_pData)->RemoveLast(iCount);
}
void CFX_BaseMassArray::RemoveAll(FX_BOOL bLeaveMemory)
{
    ((CFX_BaseMassArrayImp*)m_pData)->RemoveAll(bLeaveMemory);
}
typedef struct _FX_BASEDISCRETEARRAYDATA : public CFX_Object {
    FX_INT32		iBlockSize;
    FX_INT32		iChunkSize;
    FX_INT32		iChunkCount;
    CFX_PtrArray	ChunkBuffer;
} FX_BASEDISCRETEARRAYDATA, * FX_LPBASEDISCRETEARRAYDATA;
typedef FX_BASEDISCRETEARRAYDATA const * FX_LPCBASEDISCRETEARRAYDATA;
CFX_BaseDiscreteArray::CFX_BaseDiscreteArray(FX_INT32 iChunkSize, FX_INT32 iBlockSize)
{
    FXSYS_assert(iChunkSize > 0 && iBlockSize > 0);
    FX_LPBASEDISCRETEARRAYDATA pData;
    m_pData = pData = FX_NEW FX_BASEDISCRETEARRAYDATA;
    pData->ChunkBuffer.SetSize(16);
    pData->iChunkCount = 0;
    pData->iChunkSize = iChunkSize;
    pData->iBlockSize = iBlockSize;
}
CFX_BaseDiscreteArray::~CFX_BaseDiscreteArray()
{
    RemoveAll();
    delete (FX_LPBASEDISCRETEARRAYDATA)m_pData;
}
FX_LPBYTE CFX_BaseDiscreteArray::AddSpaceTo(FX_INT32 index)
{
    FXSYS_assert(index > -1);
    FX_LPBASEDISCRETEARRAYDATA pData = (FX_LPBASEDISCRETEARRAYDATA)m_pData;
    FX_INT32 &iChunkCount = pData->iChunkCount;
    FX_INT32 iChunkSize = pData->iChunkSize;
    FX_LPBYTE pChunk = NULL;
    FX_INT32 iChunk = index / iChunkSize;
    if (iChunk < iChunkCount) {
        pChunk = (FX_LPBYTE)pData->ChunkBuffer.GetAt(iChunk);
    }
    if (pChunk == NULL) {
        FX_INT32 iMemSize = iChunkSize * pData->iBlockSize;
        pChunk = (FX_LPBYTE)FX_Alloc(FX_BYTE, iMemSize);
        FXSYS_memset(pChunk, 0, iMemSize);
        pData->ChunkBuffer.SetAtGrow(iChunk, pChunk);
        if (iChunkCount <= iChunk) {
            iChunkCount = iChunk + 1;
        }
    }
    return pChunk + (index % iChunkSize) * pData->iBlockSize;
}
FX_LPBYTE CFX_BaseDiscreteArray::GetAt(FX_INT32 index) const
{
    FXSYS_assert(index > -1);
    FX_LPBASEDISCRETEARRAYDATA pData = (FX_LPBASEDISCRETEARRAYDATA)m_pData;
    FX_INT32 iChunkSize = pData->iChunkSize;
    FX_INT32 iChunk = index / iChunkSize;
    if (iChunk >= pData->iChunkCount) {
        return NULL;
    }
    FX_LPBYTE pChunk = (FX_LPBYTE)pData->ChunkBuffer.GetAt(iChunk);
    if (pChunk == NULL) {
        return NULL;
    }
    return pChunk + (index % iChunkSize) * pData->iBlockSize;
}
void CFX_BaseDiscreteArray::RemoveAll()
{
    FX_LPBASEDISCRETEARRAYDATA pData = (FX_LPBASEDISCRETEARRAYDATA)m_pData;
    CFX_PtrArray &ChunkBuffer = pData->ChunkBuffer;
    FX_INT32 &iChunkCount = pData->iChunkCount;
    for (FX_INT32 i = 0; i < iChunkCount; i++) {
        FX_LPVOID p = ChunkBuffer.GetAt(i);
        if (p == NULL) {
            continue;
        }
        FX_Free(p);
    }
    ChunkBuffer.RemoveAll();
    iChunkCount = 0;
}
CFX_BaseStack::CFX_BaseStack(FX_INT32 iChunkSize, FX_INT32 iBlockSize)
{
    m_pData = FXTARGET_New CFX_BaseMassArrayImp(iChunkSize, iBlockSize);
}
CFX_BaseStack::~CFX_BaseStack()
{
    FXTARGET_Delete (CFX_BaseMassArrayImp*)m_pData;
}
FX_LPBYTE CFX_BaseStack::Push()
{
    return ((CFX_BaseMassArrayImp*)m_pData)->AddSpace();
}
void CFX_BaseStack::Pop()
{
    FX_INT32 &iBlockCount = ((CFX_BaseMassArrayImp*)m_pData)->m_iBlockCount;
    if (iBlockCount < 1) {
        return;
    }
    iBlockCount --;
}
FX_LPBYTE CFX_BaseStack::GetTopElement() const
{
    FX_INT32 iSize = ((CFX_BaseMassArrayImp*)m_pData)->m_iBlockCount;
    if (iSize < 1) {
        return NULL;
    }
    return ((CFX_BaseMassArrayImp*)m_pData)->GetAt(iSize - 1);
}
FX_INT32 CFX_BaseStack::GetSize() const
{
    return ((CFX_BaseMassArrayImp*)m_pData)->m_iBlockCount;
}
FX_LPBYTE CFX_BaseStack::GetAt(FX_INT32 index) const
{
    return ((CFX_BaseMassArrayImp*)m_pData)->GetAt(index);
}
void CFX_BaseStack::RemoveAll(FX_BOOL bLeaveMemory )
{
    ((CFX_BaseMassArrayImp*)m_pData)->RemoveAll(bLeaveMemory);
}
