// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_render.h"
#include "../../../include/fpdfapi/fpdf_pageobj.h"
#include "../../../include/fxge/fx_ge.h"
#include "../fpdf_page/pageint.h"
#include "render_int.h"
struct CACHEINFO {
    FX_DWORD time;
    CPDF_Stream* pStream;
};
extern "C" {
    static int compare(const void* data1, const void* data2)
    {
        return ((CACHEINFO*)data1)->time - ((CACHEINFO*)data2)->time;
    }
};
void CPDF_Page::ClearRenderCache()
{
    if (m_pPageRender) {
        m_pPageRender->ClearAll();
    }
}
void CPDF_PageRenderCache::ClearAll()
{
    FX_POSITION pos = m_ImageCaches.GetStartPosition();
    while (pos) {
        FX_LPVOID key, value;
        m_ImageCaches.GetNextAssoc(pos, key, value);
        delete (CPDF_ImageCache*)value;
    }
    m_ImageCaches.RemoveAll();
    m_nCacheSize = 0;
    m_nTimeCount = 0;
}
void CPDF_PageRenderCache::CacheOptimization(FX_INT32 dwLimitCacheSize)
{
    if (m_nCacheSize <= (FX_DWORD)dwLimitCacheSize) {
        return;
    }
    int nCount = m_ImageCaches.GetCount();
    CACHEINFO* pCACHEINFO = (CACHEINFO*)FX_Alloc(FX_BYTE, (sizeof (CACHEINFO)) * nCount);
    FX_POSITION pos = m_ImageCaches.GetStartPosition();
    int i = 0;
    while (pos) {
        FX_LPVOID key, value;
        m_ImageCaches.GetNextAssoc(pos, key, value);
        pCACHEINFO[i].time = ((CPDF_ImageCache*)value)->GetTimeCount();
        pCACHEINFO[i++].pStream = ((CPDF_ImageCache*)value)->GetStream();
    }
    FXSYS_qsort(pCACHEINFO, nCount, sizeof (CACHEINFO), compare);
    FX_DWORD nTimeCount = m_nTimeCount;
    if (nTimeCount + 1 < nTimeCount) {
        for (i = 0; i < nCount; i ++) {
            ((CPDF_ImageCache*)(m_ImageCaches[pCACHEINFO[i].pStream]))->m_dwTimeCount = i;
        }
        m_nTimeCount = nCount;
    }
    i = 0;
    while(nCount > 15) {
        ClearImageCache(pCACHEINFO[i++].pStream);
        nCount--;
    }
    while (m_nCacheSize > (FX_DWORD)dwLimitCacheSize) {
        ClearImageCache(pCACHEINFO[i++].pStream);
    }
    FX_Free(pCACHEINFO);
}
void CPDF_PageRenderCache::ClearImageCache(CPDF_Stream* pStream)
{
    FX_LPVOID value = m_ImageCaches.GetValueAt(pStream);
    if (value == NULL)	{
        m_ImageCaches.RemoveKey(pStream);
        return;
    }
    m_nCacheSize -= ((CPDF_ImageCache*)value)->EstimateSize();
    delete (CPDF_ImageCache*)value;
    m_ImageCaches.RemoveKey(pStream);
}
FX_DWORD CPDF_PageRenderCache::EstimateSize()
{
    FX_DWORD dwSize = 0;
    FX_POSITION pos = m_ImageCaches.GetStartPosition();
    while (pos) {
        FX_LPVOID key, value;
        m_ImageCaches.GetNextAssoc(pos, key, value);
        dwSize += ((CPDF_ImageCache*)value)->EstimateSize();
    }
    m_nCacheSize = dwSize;
    return dwSize;
}
FX_DWORD CPDF_PageRenderCache::GetCachedSize(CPDF_Stream* pStream) const
{
    if (pStream == NULL) {
        return m_nCacheSize;
    }
    CPDF_ImageCache* pImageCache;
    if (!m_ImageCaches.Lookup(pStream, (FX_LPVOID&)pImageCache)) {
        return 0;
    }
    return pImageCache->EstimateSize();
}
void CPDF_PageRenderCache::GetCachedBitmap(CPDF_Stream* pStream, CFX_DIBSource*& pBitmap, CFX_DIBSource*& pMask, FX_DWORD& MatteColor,
        FX_BOOL bStdCS, FX_DWORD GroupFamily, FX_BOOL bLoadMask, CPDF_RenderStatus* pRenderStatus,
        FX_INT32 downsampleWidth, FX_INT32 downsampleHeight)
{
    CPDF_ImageCache* pImageCache;
    FX_BOOL bFind = m_ImageCaches.Lookup(pStream, (FX_LPVOID&)pImageCache);
    if (!bFind) {
        pImageCache = FX_NEW CPDF_ImageCache(m_pPage->m_pDocument, pStream);
    }
    m_nTimeCount ++;
    FX_BOOL bCached = pImageCache->GetCachedBitmap(pBitmap, pMask, MatteColor, m_pPage->m_pPageResources, bStdCS, GroupFamily, bLoadMask, pRenderStatus, downsampleWidth, downsampleHeight);
    if (!bFind) {
        m_ImageCaches.SetAt(pStream, pImageCache);
    }
    if (!bCached) {
        m_nCacheSize += pImageCache->EstimateSize();
    }
}
FX_BOOL	CPDF_PageRenderCache::StartGetCachedBitmap(CPDF_Stream* pStream, FX_BOOL bStdCS, FX_DWORD GroupFamily, FX_BOOL bLoadMask, CPDF_RenderStatus* pRenderStatus, FX_INT32 downsampleWidth, FX_INT32 downsampleHeight)
{
    m_bCurFindCache = m_ImageCaches.Lookup(pStream, (FX_LPVOID&)m_pCurImageCache);
    if (!m_bCurFindCache) {
        m_pCurImageCache = FX_NEW CPDF_ImageCache(m_pPage->m_pDocument, pStream);
    }
    int ret = m_pCurImageCache->StartGetCachedBitmap(pRenderStatus->m_pFormResource, m_pPage->m_pPageResources, bStdCS, GroupFamily, bLoadMask, pRenderStatus, downsampleWidth, downsampleHeight);
    if (ret == 2) {
        return TRUE;
    }
    m_nTimeCount ++;
    if (!m_bCurFindCache) {
        m_ImageCaches.SetAt(pStream, m_pCurImageCache);
    }
    if (!ret) {
        m_nCacheSize += m_pCurImageCache->EstimateSize();
    }
    return FALSE;
}
FX_BOOL	CPDF_PageRenderCache::Continue(IFX_Pause* pPause)
{
    int ret = m_pCurImageCache->Continue(pPause);
    if (ret == 2) {
        return TRUE;
    }
    m_nTimeCount ++;
    if (!m_bCurFindCache) {
        m_ImageCaches.SetAt(m_pCurImageCache->GetStream(), m_pCurImageCache);
    }
    if (!ret) {
        m_nCacheSize += m_pCurImageCache->EstimateSize();
    }
    return FALSE;
}
void CPDF_PageRenderCache::ResetBitmap(CPDF_Stream* pStream, const CFX_DIBitmap* pBitmap)
{
    CPDF_ImageCache* pImageCache;
    if (!m_ImageCaches.Lookup(pStream, (FX_LPVOID&)pImageCache)) {
        if (pBitmap == NULL) {
            return;
        }
        pImageCache = FX_NEW CPDF_ImageCache(m_pPage->m_pDocument, pStream);
        m_ImageCaches.SetAt(pStream, pImageCache);
    }
    int oldsize = pImageCache->EstimateSize();
    pImageCache->Reset(pBitmap);
    m_nCacheSize = pImageCache->EstimateSize() - oldsize;
}
CPDF_ImageCache::CPDF_ImageCache(CPDF_Document* pDoc, CPDF_Stream* pStream)
    : m_dwTimeCount(0)
    , m_pCurBitmap(NULL)
    , m_pCurMask(NULL)
    , m_MatteColor(0)
    , m_pRenderStatus(NULL)
    , m_pDocument(pDoc)
    , m_pStream(pStream)
    , m_pCachedBitmap(NULL)
    , m_pCachedMask(NULL)
    , m_dwCacheSize(0)
{
}
CPDF_ImageCache::~CPDF_ImageCache()
{
    if (m_pCachedBitmap) {
        delete m_pCachedBitmap;
        m_pCachedBitmap = NULL;
    }
    if (m_pCachedMask) {
        delete m_pCachedMask;
        m_pCachedMask = NULL;
    }
}
void CPDF_ImageCache::Reset(const CFX_DIBitmap* pBitmap)
{
    if (m_pCachedBitmap) {
        delete m_pCachedBitmap;
    }
    m_pCachedBitmap = NULL;
    if (pBitmap) {
        m_pCachedBitmap = pBitmap->Clone();
    }
    CalcSize();
}
void CPDF_PageRenderCache::ClearImageData()
{
    FX_POSITION pos = m_ImageCaches.GetStartPosition();
    while (pos) {
        FX_LPVOID key, value;
        m_ImageCaches.GetNextAssoc(pos, key, value);
        ((CPDF_ImageCache*)value)->ClearImageData();
    }
}
void CPDF_ImageCache::ClearImageData()
{
    if (m_pCachedBitmap && m_pCachedBitmap->GetBuffer() == NULL) {
        ((CPDF_DIBSource*)m_pCachedBitmap)->ClearImageData();
    }
}
static FX_DWORD FPDF_ImageCache_EstimateImageSize(const CFX_DIBSource* pDIB)
{
    return pDIB && pDIB->GetBuffer() ? (FX_DWORD)pDIB->GetHeight() * pDIB->GetPitch() + (FX_DWORD)pDIB->GetPaletteSize() * 4 : 0;
}
FX_BOOL CPDF_ImageCache::GetCachedBitmap(CFX_DIBSource*& pBitmap, CFX_DIBSource*& pMask, FX_DWORD& MatteColor, CPDF_Dictionary* pPageResources,
        FX_BOOL bStdCS, FX_DWORD GroupFamily, FX_BOOL bLoadMask, CPDF_RenderStatus* pRenderStatus,
        FX_INT32 downsampleWidth, FX_INT32 downsampleHeight)
{
    if (m_pCachedBitmap) {
        pBitmap = m_pCachedBitmap;
        pMask = m_pCachedMask;
        MatteColor = m_MatteColor;
        return TRUE;
    }
    if (!pRenderStatus) {
        return FALSE;
    }
    CPDF_RenderContext*pContext = pRenderStatus->GetContext();
    CPDF_PageRenderCache* pPageRenderCache = pContext->m_pPageCache;
    m_dwTimeCount = pPageRenderCache->GetTimeCount();
    CPDF_DIBSource* pSrc = FX_NEW CPDF_DIBSource;
    CPDF_DIBSource* pMaskSrc = NULL;
    if (!pSrc->Load(m_pDocument, m_pStream, &pMaskSrc, &MatteColor, pRenderStatus->m_pFormResource, pPageResources, bStdCS, GroupFamily, bLoadMask)) {
        delete pSrc;
        pBitmap = NULL;
        return FALSE;
    }
    m_MatteColor = MatteColor;
    if (pSrc->GetPitch() * pSrc->GetHeight() < FPDF_HUGE_IMAGE_SIZE) {
        m_pCachedBitmap = pSrc->Clone();
        delete pSrc;
    } else {
        m_pCachedBitmap = pSrc;
    }
    if (pMaskSrc) {
        m_pCachedMask = pMaskSrc->Clone();
        delete pMaskSrc;
    }

    pBitmap = m_pCachedBitmap;
    pMask = m_pCachedMask;
    CalcSize();
    return FALSE;
}
CFX_DIBSource* CPDF_ImageCache::DetachBitmap()
{
    CFX_DIBSource* pDIBSource = m_pCurBitmap;
    m_pCurBitmap = NULL;
    return pDIBSource;
}
CFX_DIBSource* CPDF_ImageCache::DetachMask()
{
    CFX_DIBSource* pDIBSource = m_pCurMask;
    m_pCurMask = NULL;
    return pDIBSource;
}
int	CPDF_ImageCache::StartGetCachedBitmap(CPDF_Dictionary* pFormResources, CPDF_Dictionary* pPageResources, FX_BOOL bStdCS,
        FX_DWORD GroupFamily, FX_BOOL bLoadMask, CPDF_RenderStatus* pRenderStatus,
        FX_INT32 downsampleWidth, FX_INT32 downsampleHeight)
{
    if (m_pCachedBitmap) {
        m_pCurBitmap = m_pCachedBitmap;
        m_pCurMask = m_pCachedMask;
        return 1;
    }
    if (!pRenderStatus) {
        return 0;
    }
    m_pRenderStatus = pRenderStatus;
    m_pCurBitmap = FX_NEW CPDF_DIBSource;
    int ret = ((CPDF_DIBSource*)m_pCurBitmap)->StartLoadDIBSource(m_pDocument, m_pStream, TRUE, pFormResources, pPageResources, bStdCS, GroupFamily, bLoadMask);
    if (ret == 2) {
        return ret;
    }
    if (!ret) {
        delete m_pCurBitmap;
        m_pCurBitmap = NULL;
        return 0;
    }
    ContinueGetCachedBitmap();
    return 0;
}
int CPDF_ImageCache::ContinueGetCachedBitmap()
{
    m_MatteColor = ((CPDF_DIBSource*)m_pCurBitmap)->m_MatteColor;
    m_pCurMask = ((CPDF_DIBSource*)m_pCurBitmap)->DetachMask();
    CPDF_RenderContext*pContext = m_pRenderStatus->GetContext();
    CPDF_PageRenderCache* pPageRenderCache = pContext->m_pPageCache;
    m_dwTimeCount = pPageRenderCache->GetTimeCount();
    if (m_pCurBitmap->GetPitch() * m_pCurBitmap->GetHeight() < FPDF_HUGE_IMAGE_SIZE) {
        m_pCachedBitmap = m_pCurBitmap->Clone();
        delete m_pCurBitmap;
        m_pCurBitmap = NULL;
    } else {
        m_pCachedBitmap = m_pCurBitmap;
    }
    if (m_pCurMask) {
        m_pCachedMask = m_pCurMask->Clone();
        delete m_pCurMask;
        m_pCurMask = NULL;
    }
    m_pCurBitmap = m_pCachedBitmap;
    m_pCurMask = m_pCachedMask;
    CalcSize();
    return 0;
}
int	CPDF_ImageCache::Continue(IFX_Pause* pPause)
{
    int ret = ((CPDF_DIBSource*)m_pCurBitmap)->ContinueLoadDIBSource(pPause);
    if (ret == 2) {
        return ret;
    }
    if (!ret) {
        delete m_pCurBitmap;
        m_pCurBitmap = NULL;
        return 0;
    }
    ContinueGetCachedBitmap();
    return 0;
}
void CPDF_ImageCache::CalcSize()
{
    m_dwCacheSize = FPDF_ImageCache_EstimateImageSize(m_pCachedBitmap) + FPDF_ImageCache_EstimateImageSize(m_pCachedMask);
}
void CPDF_Document::ClearRenderFont()
{
    if (m_pDocRender) {
        CFX_FontCache* pCache = m_pDocRender->GetFontCache();
        if (pCache) {
            pCache->FreeCache(FALSE);
        }
    }
}
