// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSCACHE_H_
#define XFA_FDE_CSS_FDE_CSSCACHE_H_

#include <map>

#include "xfa/fde/css/fde_css.h"

class FDE_CSSTagCache {
 public:
  FDE_CSSTagCache(FDE_CSSTagCache* parent, CXFA_CSSTagProvider* tag);
  FDE_CSSTagCache(const FDE_CSSTagCache& it);
  ~FDE_CSSTagCache();

  FDE_CSSTagCache* GetParent() const { return pParent; }
  CXFA_CSSTagProvider* GetTag() const { return pTag; }
  uint32_t HashID() const { return dwIDHash; }
  uint32_t HashTag() const { return dwTagHash; }
  int32_t CountHashClass() const { return dwClassHashs.GetSize(); }
  void SetClassIndex(int32_t index) { iClassIndex = index; }
  uint32_t HashClass() const {
    return iClassIndex < dwClassHashs.GetSize()
               ? dwClassHashs.GetAt(iClassIndex)
               : 0;
  }

 private:
  CXFA_CSSTagProvider* pTag;
  FDE_CSSTagCache* pParent;
  uint32_t dwIDHash;
  uint32_t dwTagHash;
  int32_t iClassIndex;
  CFX_BaseArrayTemplate<uint32_t> dwClassHashs;
};

class CFDE_CSSAccelerator {
 public:
  CFDE_CSSAccelerator();
  ~CFDE_CSSAccelerator();

  void OnEnterTag(CXFA_CSSTagProvider* pTag);
  void OnLeaveTag(CXFA_CSSTagProvider* pTag);

  void Clear() { m_Stack.RemoveAll(false); }

  FDE_CSSTagCache* GetTopElement() const { return m_Stack.GetTopElement(); }

 private:
  CFX_ObjectStackTemplate<FDE_CSSTagCache> m_Stack;
};

#endif  // XFA_FDE_CSS_FDE_CSSCACHE_H_
