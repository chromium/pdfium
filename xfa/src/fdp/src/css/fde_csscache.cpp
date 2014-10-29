// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "fde_csscache.h"
_FDE_CSSCACHEITEM::_FDE_CSSCACHEITEM(IFDE_CSSStyleSheet *p)
    : pStylesheet(p)
    , dwActivity(0)
{
    FXSYS_assert(pStylesheet);
    pStylesheet->AddRef();
}
_FDE_CSSCACHEITEM::~_FDE_CSSCACHEITEM()
{
    pStylesheet->Release();
}
IFDE_CSSStyleSheetCache* IFDE_CSSStyleSheetCache::Create()
{
    return FDE_New CFDE_CSSStyleSheetCache;
}
CFDE_CSSStyleSheetCache::CFDE_CSSStyleSheetCache()
    : m_pFixedStore(NULL)
    , m_iMaxItems(5)
{
}
CFDE_CSSStyleSheetCache::~CFDE_CSSStyleSheetCache()
{
    FX_POSITION pos = m_Stylesheets.GetStartPosition();
    if (pos != NULL) {
        CFX_ByteString szKey;
        FX_LPVOID pValue;
        while (pos != NULL) {
            m_Stylesheets.GetNextAssoc(pos, szKey, pValue);
            FDE_DeleteWith(FDE_CSSCACHEITEM, m_pFixedStore, (FDE_LPCSSCACHEITEM)pValue);
        }
        m_Stylesheets.RemoveAll();
    }
    if (m_pFixedStore != NULL) {
        m_pFixedStore->Release();
    }
}
void CFDE_CSSStyleSheetCache::AddStyleSheet(FX_BSTR szKey, IFDE_CSSStyleSheet *pStyleSheet)
{
    FXSYS_assert(pStyleSheet != NULL);
    if (m_pFixedStore == NULL) {
        m_pFixedStore = FX_CreateAllocator(FX_ALLOCTYPE_Fixed, FX_MAX(10, m_iMaxItems), sizeof(FDE_CSSCACHEITEM));
        FXSYS_assert(m_pFixedStore != NULL);
    }
    FX_LPVOID pValue = NULL;
    if (m_Stylesheets.Lookup(szKey, pValue)) {
        FDE_LPCSSCACHEITEM pItem = (FDE_LPCSSCACHEITEM)pValue;
        if (pItem->pStylesheet != pStyleSheet) {
            pItem->pStylesheet->Release();
            pItem->pStylesheet = pStyleSheet;
            pItem->pStylesheet->AddRef();
            pItem->dwActivity = 0;
        }
    } else {
        while (m_Stylesheets.GetCount() >= m_iMaxItems) {
            RemoveLowestActivityItem();
        }
        FDE_LPCSSCACHEITEM pItem = FDE_NewWith(m_pFixedStore) FDE_CSSCACHEITEM(pStyleSheet);
        FXSYS_assert(pItem != NULL);
        m_Stylesheets.SetAt(szKey, pItem);
    }
}
IFDE_CSSStyleSheet* CFDE_CSSStyleSheetCache::GetStyleSheet(FX_BSTR szKey) const
{
    FX_LPVOID pValue = NULL;
    if (m_Stylesheets.Lookup(szKey, pValue)) {
        FDE_LPCSSCACHEITEM pItem = (FDE_LPCSSCACHEITEM)pValue;
        pItem->dwActivity++;
        pItem->pStylesheet->AddRef();
        return pItem->pStylesheet;
    }
    return NULL;
}
void CFDE_CSSStyleSheetCache::RemoveStyleSheet(FX_BSTR szKey)
{
    FX_LPVOID pValue = NULL;
    if (!m_Stylesheets.Lookup(szKey, pValue)) {
        return;
    }
    FDE_DeleteWith(FDE_CSSCACHEITEM, m_pFixedStore, (FDE_LPCSSCACHEITEM)pValue);
    m_Stylesheets.RemoveKey(szKey);
}
void CFDE_CSSStyleSheetCache::RemoveLowestActivityItem()
{
    FX_POSITION pos = m_Stylesheets.GetStartPosition();
    CFX_ByteString szKey;
    FX_LPVOID pValue;
    FDE_LPCSSCACHEITEM pItem = NULL;
    CFX_ByteString szItem;
    while (pos != NULL) {
        m_Stylesheets.GetNextAssoc(pos, szKey, pValue);
        switch (szKey.GetID()) {
            case FXBSTR_ID('#', 'U', 'S', 'E'):
            case FXBSTR_ID('#', 'A', 'G', 'E'):
                continue;
        }
        FDE_LPCSSCACHEITEM p = (FDE_LPCSSCACHEITEM)pValue;
        if (pItem == NULL || pItem->dwActivity > p->dwActivity) {
            szItem = szKey;
            pItem = p;
        }
    }
    if (pItem != NULL) {
        FDE_DeleteWith(FDE_CSSCACHEITEM, m_pFixedStore, pItem);
        m_Stylesheets.RemoveKey(szItem);
    }
}
_FDE_CSSTAGCACHE::_FDE_CSSTAGCACHE(_FDE_CSSTAGCACHE *parent, IFDE_CSSTagProvider *tag)
    : pTag(tag)
    , pParent(parent)
    , dwIDHash(0)
    , dwTagHash(0)
    , iClassIndex(0)
    , dwClassHashs(1)
{
    FXSYS_assert(pTag != NULL);
    CFX_WideStringC wsValue, wsName = pTag->GetTagName();
    dwTagHash = FX_HashCode_String_GetW(wsName.GetPtr(), wsName.GetLength(), TRUE);
    FX_POSITION pos = pTag->GetFirstAttribute();
    while (pos != NULL) {
        pTag->GetNextAttribute(pos, wsName, wsValue);
        FX_DWORD dwNameHash = FX_HashCode_String_GetW(wsName.GetPtr(), wsName.GetLength(), TRUE);
        static const FX_DWORD s_dwIDHash = FX_HashCode_String_GetW((FX_LPCWSTR)L"id", 2, TRUE);
        static const FX_DWORD s_dwClassHash = FX_HashCode_String_GetW((FX_LPCWSTR)L"class", 5, TRUE);
        if (dwNameHash == s_dwClassHash) {
            FX_DWORD dwHash = FX_HashCode_String_GetW(wsValue.GetPtr(), wsValue.GetLength());
            dwClassHashs.Add(dwHash);
        } else if (dwNameHash == s_dwIDHash) {
            dwIDHash = FX_HashCode_String_GetW(wsValue.GetPtr(), wsValue.GetLength());
        }
    }
}
_FDE_CSSTAGCACHE::_FDE_CSSTAGCACHE(const _FDE_CSSTAGCACHE &it)
    : pTag(it.pTag)
    , pParent(it.pParent)
    , dwIDHash(it.dwIDHash)
    , dwTagHash(it.dwTagHash)
    , iClassIndex(0)
    , dwClassHashs(1)
{
    if (it.dwClassHashs.GetSize() > 0) {
        dwClassHashs.Copy(it.dwClassHashs);
    }
}
void CFDE_CSSAccelerator::OnEnterTag(IFDE_CSSTagProvider *pTag)
{
    FDE_CSSTAGCACHE *pTop = GetTopElement();
    FDE_CSSTAGCACHE item(pTop, pTag);
    m_Stack.Push(item);
}
void CFDE_CSSAccelerator::OnLeaveTag(IFDE_CSSTagProvider *pTag)
{
    FDE_CSSTAGCACHE *pItem = m_Stack.GetTopElement();
    FXSYS_assert(pItem && pItem->GetTag() == pTag);
    m_Stack.Pop();
}
