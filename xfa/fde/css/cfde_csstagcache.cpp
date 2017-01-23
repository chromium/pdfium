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
    : pTag(tag), pParent(parent), dwTagHash(0) {
  dwTagHash = FX_HashCode_GetW(pTag->GetTagName().AsStringC(), true);
}

CFDE_CSSTagCache::~CFDE_CSSTagCache() {}
