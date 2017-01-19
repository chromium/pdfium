// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/cfde_csstagcache.h"

#include <algorithm>

#include "core/fxcrt/fx_ext.h"
#include "xfa/fxfa/app/cxfa_csstagprovider.h"

CFDE_CSSTagCache::CFDE_CSSTagCache(CFDE_CSSTagCache* parent,
                                   CXFA_CSSTagProvider* tag)
    : pTag(tag), pParent(parent), dwIDHash(0), dwTagHash(0), iClassIndex(0) {
  static const uint32_t s_dwIDHash = FX_HashCode_GetW(L"id", true);
  static const uint32_t s_dwClassHash = FX_HashCode_GetW(L"class", true);
  dwTagHash = FX_HashCode_GetW(pTag->GetTagName().AsStringC(), true);

  for (auto it : *pTag) {
    CFX_WideString wsValue = it.first;
    CFX_WideString wsName = it.second;
    uint32_t dwNameHash = FX_HashCode_GetW(wsName.AsStringC(), true);
    if (dwNameHash == s_dwClassHash) {
      uint32_t dwHash = FX_HashCode_GetW(wsValue.AsStringC(), false);
      dwClassHashes.push_back(dwHash);
    } else if (dwNameHash == s_dwIDHash) {
      dwIDHash = FX_HashCode_GetW(wsValue.AsStringC(), false);
    }
  }
}

CFDE_CSSTagCache::CFDE_CSSTagCache(const CFDE_CSSTagCache& it)
    : pTag(it.pTag),
      pParent(it.pParent),
      dwIDHash(it.dwIDHash),
      dwTagHash(it.dwTagHash),
      iClassIndex(0) {
  std::copy(it.dwClassHashes.begin(), it.dwClassHashes.end(),
            dwClassHashes.begin());
}

CFDE_CSSTagCache::~CFDE_CSSTagCache() {}
