// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSTAGCACHE_H_
#define XFA_FDE_CSS_CFDE_CSSTAGCACHE_H_

#include <vector>

#include "core/fxcrt/fx_system.h"
#include "third_party/base/stl_util.h"

class CXFA_CSSTagProvider;

class CFDE_CSSTagCache {
 public:
  CFDE_CSSTagCache(CFDE_CSSTagCache* parent, CXFA_CSSTagProvider* tag);
  ~CFDE_CSSTagCache();

  CFDE_CSSTagCache* GetParent() const { return pParent; }
  CXFA_CSSTagProvider* GetTag() const { return pTag; }
  uint32_t HashTag() const { return dwTagHash; }

 private:
  CXFA_CSSTagProvider* pTag;
  CFDE_CSSTagCache* pParent;
  uint32_t dwTagHash;
};

#endif  // XFA_FDE_CSS_CFDE_CSSTAGCACHE_H_
