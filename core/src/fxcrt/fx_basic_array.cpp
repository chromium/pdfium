// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcrt/fx_basic.h"
#include "../../../third_party/base/numerics/safe_math.h"

CFX_BasicArray::CFX_BasicArray(int unit_size)
    : m_pData(NULL)
    , m_nSize(0)
    , m_nMaxSize(0)
{
    if (unit_size < 0 || unit_size > (1 << 28)) {
        m_nUnitSize = 4;
    } else {
        m_nUnitSize = unit_size;
    }
}
CFX_BasicArray::~CFX_BasicArray()
{
    FX_Free(m_pData);
}
FX_BOOL CFX_BasicArray::SetSize(int nNewSize)
{
    if (nNewSize <= 0) {
        FX_Free(m_pData);
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
        return 0 == nNewSize;
    }

    if (m_pData == NULL) {
        pdfium::base::CheckedNumeric<int> totalSize = nNewSize;
        totalSize *= m_nUnitSize;
        if (!totalSize.IsValid()) {
            m_nSize = m_nMaxSize = 0;
            return FALSE;
        }
        m_pData = FX_Alloc(FX_BYTE, totalSize.ValueOrDie());
        if (!m_pData) {
            m_nSize = m_nMaxSize = 0;
            return FALSE;
        }
        m_nSize = m_nMaxSize = nNewSize;
    } else if (nNewSize <= m_nMaxSize) {
        if (nNewSize > m_nSize) {
            FXSYS_memset32(m_pData + m_nSize * m_nUnitSize, 0, (nNewSize - m_nSize) * m_nUnitSize);
        }
        m_nSize = nNewSize;
    } else {
        int nNewMax = nNewSize < m_nMaxSize ? m_nMaxSize : nNewSize;
        pdfium::base::CheckedNumeric<int> totalSize = nNewMax;
        totalSize *= m_nUnitSize;
        if (!totalSize.IsValid() || nNewMax < m_nSize) {
            return FALSE;
        }
        FX_LPBYTE pNewData = FX_Realloc(FX_BYTE, m_pData, totalSize.ValueOrDie());
        if (pNewData == NULL) {
            return FALSE;
        }
        FXSYS_memset32(pNewData + m_nSize * m_nUnitSize, 0, (nNewMax - m_nSize) * m_nUnitSize);
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }
    return TRUE;
}
FX_BOOL CFX_BasicArray::Append(const CFX_BasicArray& src)
{
    int nOldSize = m_nSize;
    pdfium::base::CheckedNumeric<int> newSize = m_nSize;
    newSize += src.m_nSize;
    if (m_nUnitSize != src.m_nUnitSize || !newSize.IsValid() || !SetSize(newSize.ValueOrDie())) {
        return FALSE;
    }

    FXSYS_memcpy32(m_pData + nOldSize * m_nUnitSize, src.m_pData, src.m_nSize * m_nUnitSize);
    return TRUE;
}
FX_BOOL CFX_BasicArray::Copy(const CFX_BasicArray& src)
{
    if (!SetSize(src.m_nSize)) {
        return FALSE;
    }
    FXSYS_memcpy32(m_pData, src.m_pData, src.m_nSize * m_nUnitSize);
    return TRUE;
}
FX_LPBYTE CFX_BasicArray::InsertSpaceAt(int nIndex, int nCount)
{
    if (nIndex < 0 || nCount <= 0) {
        return NULL;
    }
    if (nIndex >= m_nSize) {
        if (!SetSize(nIndex + nCount)) {
            return NULL;
        }
    } else {
        int nOldSize = m_nSize;
        if (!SetSize(m_nSize + nCount)) {
            return NULL;
        }
        FXSYS_memmove32(m_pData + (nIndex + nCount)*m_nUnitSize, m_pData + nIndex * m_nUnitSize,
                        (nOldSize - nIndex) * m_nUnitSize);
        FXSYS_memset32(m_pData + nIndex * m_nUnitSize, 0, nCount * m_nUnitSize);
    }
    return m_pData + nIndex * m_nUnitSize;
}
FX_BOOL CFX_BasicArray::RemoveAt(int nIndex, int nCount)
{
    if (nIndex < 0 || nCount <= 0 || m_nSize < nIndex + nCount) {
        return FALSE;
    }
    int nMoveCount = m_nSize - (nIndex + nCount);
    if (nMoveCount) {
        FXSYS_memmove32(m_pData + nIndex * m_nUnitSize, m_pData + (nIndex + nCount) * m_nUnitSize, nMoveCount * m_nUnitSize);
    }
    m_nSize -= nCount;
    return TRUE;
}
FX_BOOL CFX_BasicArray::InsertAt(int nStartIndex, const CFX_BasicArray* pNewArray)
{
    if (pNewArray == NULL) {
        return FALSE;
    }
    if (pNewArray->m_nSize == 0) {
        return TRUE;
    }
    if (!InsertSpaceAt(nStartIndex, pNewArray->m_nSize)) {
        return FALSE;
    }
    FXSYS_memcpy32(m_pData + nStartIndex * m_nUnitSize, pNewArray->m_pData, pNewArray->m_nSize * m_nUnitSize);
    return TRUE;
}
const void* CFX_BasicArray::GetDataPtr(int index) const
{
    if (index < 0 || index >= m_nSize || m_pData == NULL) {
        return NULL;
    }
    return m_pData + index * m_nUnitSize;
}
CFX_BaseSegmentedArray::CFX_BaseSegmentedArray(int unit_size, int segment_units, int index_size)
    : m_UnitSize(unit_size)
    , m_SegmentSize(segment_units)
    , m_IndexSize(index_size)
    , m_IndexDepth(0)
    , m_DataSize(0)
    , m_pIndex(NULL)
{
}
void CFX_BaseSegmentedArray::SetUnitSize(int unit_size, int segment_units, int index_size)
{
    ASSERT(m_DataSize == 0);
    m_UnitSize = unit_size;
    m_SegmentSize = segment_units;
    m_IndexSize = index_size;
}
CFX_BaseSegmentedArray::~CFX_BaseSegmentedArray()
{
    RemoveAll();
}
static void _ClearIndex(int level, int size, void** pIndex)
{
    if (level == 0) {
        FX_Free(pIndex);
        return;
    }
    for (int i = 0; i < size; i++) {
        if (pIndex[i] == NULL) {
            continue;
        }
        _ClearIndex(level - 1, size, (void**)pIndex[i]);
    }
    FX_Free(pIndex);
}
void CFX_BaseSegmentedArray::RemoveAll()
{
    if (m_pIndex == NULL) {
        return;
    }
    _ClearIndex(m_IndexDepth, m_IndexSize, (void**)m_pIndex);
    m_pIndex = NULL;
    m_IndexDepth = 0;
    m_DataSize = 0;
}
void* CFX_BaseSegmentedArray::Add()
{
    if (m_DataSize % m_SegmentSize) {
        return GetAt(m_DataSize ++);
    }
    void* pSegment = FX_Alloc(FX_BYTE, m_UnitSize * m_SegmentSize);
    if (!pSegment) {
        return NULL;
    }
    if (m_pIndex == NULL) {
        m_pIndex = pSegment;
        m_DataSize ++;
        return pSegment;
    }
    if (m_IndexDepth == 0) {
        void** pIndex = (void**)FX_Alloc(void*, m_IndexSize);
        if (pIndex == NULL) {
            FX_Free(pSegment);
            return NULL;
        }
        pIndex[0] = m_pIndex;
        pIndex[1] = pSegment;
        m_pIndex = pIndex;
        m_DataSize ++;
        m_IndexDepth ++;
        return pSegment;
    }
    int seg_index = m_DataSize / m_SegmentSize;
    if (seg_index % m_IndexSize) {
        void** pIndex = GetIndex(seg_index);
        pIndex[seg_index % m_IndexSize] = pSegment;
        m_DataSize ++;
        return pSegment;
    }
    int tree_size = 1;
    int i;
    for (i = 0; i < m_IndexDepth; i ++) {
        tree_size *= m_IndexSize;
    }
    if (m_DataSize == tree_size * m_SegmentSize) {
        void** pIndex = (void**)FX_Alloc(void*, m_IndexSize);
        if (pIndex == NULL) {
            FX_Free(pSegment);
            return NULL;
        }
        pIndex[0] = m_pIndex;
        m_pIndex = pIndex;
        m_IndexDepth ++;
    } else {
        tree_size /= m_IndexSize;
    }
    void** pSpot = (void**)m_pIndex;
    for (i = 1; i < m_IndexDepth; i ++) {
        if (pSpot[seg_index / tree_size] == NULL) {
            pSpot[seg_index / tree_size] = (void*)FX_Alloc(void*, m_IndexSize);
            if (pSpot[seg_index / tree_size] == NULL) {
                break;
            }
        }
        pSpot = (void**)pSpot[seg_index / tree_size];
        seg_index = seg_index % tree_size;
        tree_size /= m_IndexSize;
    }
    if (i < m_IndexDepth) {
        FX_Free(pSegment);
        RemoveAll();
        return NULL;
    }
    pSpot[seg_index % m_IndexSize] = pSegment;
    m_DataSize ++;
    return pSegment;
}
void** CFX_BaseSegmentedArray::GetIndex(int seg_index) const
{
    ASSERT(m_IndexDepth != 0);
    if (m_IndexDepth == 1) {
        return (void**)m_pIndex;
    } else if (m_IndexDepth == 2) {
        return (void**)((void**)m_pIndex)[seg_index / m_IndexSize];
    }
    int tree_size = 1;
    int i;
    for (i = 1; i < m_IndexDepth; i ++) {
        tree_size *= m_IndexSize;
    }
    void** pSpot = (void**)m_pIndex;
    for (i = 1; i < m_IndexDepth; i ++) {
        pSpot = (void**)pSpot[seg_index / tree_size];
        seg_index = seg_index % tree_size;
        tree_size /= m_IndexSize;
    }
    return pSpot;
}
void* CFX_BaseSegmentedArray::IterateSegment(FX_LPCBYTE pSegment, int count, FX_BOOL (*callback)(void* param, void* pData), void* param) const
{
    for (int i = 0; i < count; i ++) {
        if (!callback(param, (void*)(pSegment + i * m_UnitSize))) {
            return (void*)(pSegment + i * m_UnitSize);
        }
    }
    return NULL;
}
void* CFX_BaseSegmentedArray::IterateIndex(int level, int& start, void** pIndex, FX_BOOL (*callback)(void* param, void* pData), void* param) const
{
    if (level == 0) {
        int count = m_DataSize - start;
        if (count > m_SegmentSize) {
            count = m_SegmentSize;
        }
        start += count;
        return IterateSegment((FX_LPCBYTE)pIndex, count, callback, param);
    }
    for (int i = 0; i < m_IndexSize; i ++) {
        if (pIndex[i] == NULL) {
            continue;
        }
        void* p = IterateIndex(level - 1, start, (void**)pIndex[i], callback, param);
        if (p) {
            return p;
        }
    }
    return NULL;
}
void* CFX_BaseSegmentedArray::Iterate(FX_BOOL (*callback)(void* param, void* pData), void* param) const
{
    if (m_pIndex == NULL) {
        return NULL;
    }
    int start = 0;
    return IterateIndex(m_IndexDepth, start, (void**)m_pIndex, callback, param);
}
void* CFX_BaseSegmentedArray::GetAt(int index) const
{
    if (index < 0 || index >= m_DataSize) {
        return NULL;
    }
    if (m_IndexDepth == 0) {
        return (FX_LPBYTE)m_pIndex + m_UnitSize * index;
    }
    int seg_index = index / m_SegmentSize;
    return (FX_LPBYTE)GetIndex(seg_index)[seg_index % m_IndexSize] + (index % m_SegmentSize) * m_UnitSize;
}
void CFX_BaseSegmentedArray::Delete(int index, int count)
{
    if(index < 0 || count < 1 || index + count > m_DataSize) {
        return;
    }
    int i;
    for (i = index; i < m_DataSize - count; i ++) {
        FX_BYTE* pSrc = (FX_BYTE*)GetAt(i + count);
        FX_BYTE* pDest = (FX_BYTE*)GetAt(i);
        for (int j = 0; j < m_UnitSize; j ++) {
            pDest[j] = pSrc[j];
        }
    }
    int new_segs = (m_DataSize - count + m_SegmentSize - 1) / m_SegmentSize;
    int old_segs = (m_DataSize + m_SegmentSize - 1) / m_SegmentSize;
    if (new_segs < old_segs) {
        if(m_IndexDepth) {
            for (i = new_segs; i < old_segs; i ++) {
                void** pIndex = GetIndex(i);
                FX_Free(pIndex[i % m_IndexSize]);
                pIndex[i % m_IndexSize] = NULL;
            }
        } else {
            FX_Free(m_pIndex);
            m_pIndex = NULL;
        }
    }
    m_DataSize -= count;
}
