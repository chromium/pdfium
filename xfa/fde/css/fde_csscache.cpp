// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_csscache.h"

#include <algorithm>

#include "core/fxcrt/include/fx_ext.h"

FDE_CSSCacheItem::FDE_CSSCacheItem(IFDE_CSSStyleSheet* p)
    : pStylesheet(p), dwActivity(0) {
  FXSYS_assert(pStylesheet);
  pStylesheet->AddRef();
}
FDE_CSSCacheItem::~FDE_CSSCacheItem() {
  pStylesheet->Release();
}
IFDE_CSSStyleSheetCache* IFDE_CSSStyleSheetCache::Create() {
  return new CFDE_CSSStyleSheetCache;
}

CFDE_CSSStyleSheetCache::CFDE_CSSStyleSheetCache()
    : m_pFixedStore(NULL), m_iMaxItems(5) {}

CFDE_CSSStyleSheetCache::~CFDE_CSSStyleSheetCache() {
  for (const auto& pair : m_Stylesheets) {
    FXTARGET_DeleteWith(FDE_CSSCacheItem, m_pFixedStore, pair.second);
  }
  m_Stylesheets.clear();
  if (m_pFixedStore) {
    m_pFixedStore->Release();
  }
}
void CFDE_CSSStyleSheetCache::AddStyleSheet(const CFX_ByteStringC& szKey,
                                            IFDE_CSSStyleSheet* pStyleSheet) {
  FXSYS_assert(pStyleSheet != NULL);
  if (m_pFixedStore == NULL) {
    m_pFixedStore =
        FX_CreateAllocator(FX_ALLOCTYPE_Fixed, std::max(10, m_iMaxItems),
                           sizeof(FDE_CSSCacheItem));
    FXSYS_assert(m_pFixedStore != NULL);
  }
  auto it = m_Stylesheets.find(szKey);
  if (it != m_Stylesheets.end()) {
    FDE_CSSCacheItem* pItem = it->second;
    if (pItem->pStylesheet != pStyleSheet) {
      pItem->pStylesheet->Release();
      pItem->pStylesheet = pStyleSheet;
      pItem->pStylesheet->AddRef();
      pItem->dwActivity = 0;
    }
  } else {
    while (static_cast<int32_t>(m_Stylesheets.size()) >= m_iMaxItems) {
      RemoveLowestActivityItem();
    }
    m_Stylesheets[szKey] =
        FXTARGET_NewWith(m_pFixedStore) FDE_CSSCacheItem(pStyleSheet);
  }
}
IFDE_CSSStyleSheet* CFDE_CSSStyleSheetCache::GetStyleSheet(
    const CFX_ByteStringC& szKey) const {
  auto it = m_Stylesheets.find(szKey);
  if (it == m_Stylesheets.end()) {
    return nullptr;
  }
  FDE_CSSCacheItem* pItem = it->second;
  pItem->dwActivity++;
  pItem->pStylesheet->AddRef();
  return pItem->pStylesheet;
}
void CFDE_CSSStyleSheetCache::RemoveStyleSheet(const CFX_ByteStringC& szKey) {
  auto it = m_Stylesheets.find(szKey);
  if (it == m_Stylesheets.end()) {
    return;
  }
  FXTARGET_DeleteWith(FDE_CSSCacheItem, m_pFixedStore, it->second);
  m_Stylesheets.erase(it);
}
void CFDE_CSSStyleSheetCache::RemoveLowestActivityItem() {
  auto found = m_Stylesheets.end();
  for (auto it = m_Stylesheets.begin(); it != m_Stylesheets.end(); ++it) {
    switch (it->first.GetID()) {
      case FXBSTR_ID('#', 'U', 'S', 'E'):
      case FXBSTR_ID('#', 'A', 'G', 'E'):
        continue;
    }
    if (found == m_Stylesheets.end() ||
        it->second->dwActivity > found->second->dwActivity) {
      found = it;
    }
  }
  if (found != m_Stylesheets.end()) {
    FXTARGET_DeleteWith(FDE_CSSCacheItem, m_pFixedStore, found->second);
    m_Stylesheets.erase(found);
  }
}
FDE_CSSTagCache::FDE_CSSTagCache(FDE_CSSTagCache* parent,
                                 IFDE_CSSTagProvider* tag)
    : pTag(tag),
      pParent(parent),
      dwIDHash(0),
      dwTagHash(0),
      iClassIndex(0),
      dwClassHashs(1) {
  FXSYS_assert(pTag != NULL);
  CFX_WideStringC wsValue, wsName = pTag->GetTagName();
  dwTagHash =
      FX_HashCode_String_GetW(wsName.raw_str(), wsName.GetLength(), TRUE);
  FX_POSITION pos = pTag->GetFirstAttribute();
  while (pos != NULL) {
    pTag->GetNextAttribute(pos, wsName, wsValue);
    uint32_t dwNameHash =
        FX_HashCode_String_GetW(wsName.raw_str(), wsName.GetLength(), TRUE);
    static const uint32_t s_dwIDHash = FX_HashCode_String_GetW(L"id", 2, TRUE);
    static const uint32_t s_dwClassHash =
        FX_HashCode_String_GetW(L"class", 5, TRUE);
    if (dwNameHash == s_dwClassHash) {
      uint32_t dwHash =
          FX_HashCode_String_GetW(wsValue.raw_str(), wsValue.GetLength());
      dwClassHashs.Add(dwHash);
    } else if (dwNameHash == s_dwIDHash) {
      dwIDHash =
          FX_HashCode_String_GetW(wsValue.raw_str(), wsValue.GetLength());
    }
  }
}
FDE_CSSTagCache::FDE_CSSTagCache(const FDE_CSSTagCache& it)
    : pTag(it.pTag),
      pParent(it.pParent),
      dwIDHash(it.dwIDHash),
      dwTagHash(it.dwTagHash),
      iClassIndex(0),
      dwClassHashs(1) {
  if (it.dwClassHashs.GetSize() > 0) {
    dwClassHashs.Copy(it.dwClassHashs);
  }
}
void CFDE_CSSAccelerator::OnEnterTag(IFDE_CSSTagProvider* pTag) {
  FDE_CSSTagCache* pTop = GetTopElement();
  FDE_CSSTagCache item(pTop, pTag);
  m_Stack.Push(item);
}
void CFDE_CSSAccelerator::OnLeaveTag(IFDE_CSSTagProvider* pTag) {
  FXSYS_assert(m_Stack.GetTopElement());
  FXSYS_assert(m_Stack.GetTopElement()->GetTag() == pTag);
  m_Stack.Pop();
}
