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
  CFDE_CSSTagCache(const CFDE_CSSTagCache& it);
  ~CFDE_CSSTagCache();

  CFDE_CSSTagCache* GetParent() const { return pParent; }
  CXFA_CSSTagProvider* GetTag() const { return pTag; }
  uint32_t HashID() const { return dwIDHash; }
  uint32_t HashTag() const { return dwTagHash; }
  int32_t CountHashClass() const {
    return pdfium::CollectionSize<int32_t>(dwClassHashes);
  }
  void SetClassIndex(int32_t index) { iClassIndex = index; }
  uint32_t HashClass() const {
    return iClassIndex < pdfium::CollectionSize<int32_t>(dwClassHashes)
               ? dwClassHashes[iClassIndex]
               : 0;
  }

 private:
  CXFA_CSSTagProvider* pTag;
  CFDE_CSSTagCache* pParent;
  uint32_t dwIDHash;
  uint32_t dwTagHash;
  int32_t iClassIndex;
  std::vector<uint32_t> dwClassHashes;
};

#endif  // XFA_FDE_CSS_CFDE_CSSTAGCACHE_H_
