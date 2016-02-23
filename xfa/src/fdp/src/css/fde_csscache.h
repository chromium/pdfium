// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FDP_SRC_CSS_FDE_CSSCACHE_H_
#define XFA_SRC_FDP_SRC_CSS_FDE_CSSCACHE_H_

#include "xfa/src/fdp/include/fde_css.h"
#include "xfa/src/fgas/include/fx_mem.h"

struct FDE_CSSCACHEITEM : public CFX_Target {
  FDE_CSSCACHEITEM(IFDE_CSSStyleSheet* p);
  ~FDE_CSSCACHEITEM();

  IFDE_CSSStyleSheet* pStylesheet;
  FX_DWORD dwActivity;
};

class CFDE_CSSStyleSheetCache : public IFDE_CSSStyleSheetCache,
                                public CFX_Target {
 public:
  CFDE_CSSStyleSheetCache();
  ~CFDE_CSSStyleSheetCache();
  virtual void Release() { delete this; }

  virtual void SetMaxItems(int32_t iMaxCount = 5) {
    FXSYS_assert(iMaxCount >= 3);
    m_iMaxItems = iMaxCount;
  }

  virtual void AddStyleSheet(const CFX_ByteStringC& szKey,
                             IFDE_CSSStyleSheet* pStyleSheet);
  virtual IFDE_CSSStyleSheet* GetStyleSheet(const CFX_ByteStringC& szKey) const;
  virtual void RemoveStyleSheet(const CFX_ByteStringC& szKey);

 protected:
  void RemoveLowestActivityItem();
  std::map<CFX_ByteString, FDE_CSSCACHEITEM*> m_Stylesheets;
  IFX_MEMAllocator* m_pFixedStore;
  int32_t m_iMaxItems;
};

struct FDE_CSSTAGCACHE : public CFX_Target {
 public:
  FDE_CSSTAGCACHE(FDE_CSSTAGCACHE* parent, IFDE_CSSTagProvider* tag);
  FDE_CSSTAGCACHE(const FDE_CSSTAGCACHE& it);
  FDE_CSSTAGCACHE* GetParent() const { return pParent; }
  IFDE_CSSTagProvider* GetTag() const { return pTag; }
  FX_DWORD HashID() const { return dwIDHash; }
  FX_DWORD HashTag() const { return dwTagHash; }
  int32_t CountHashClass() const { return dwClassHashs.GetSize(); }
  void SetClassIndex(int32_t index) { iClassIndex = index; }
  FX_DWORD HashClass() const {
    return iClassIndex < dwClassHashs.GetSize()
               ? dwClassHashs.GetAt(iClassIndex)
               : 0;
  }

 protected:
  IFDE_CSSTagProvider* pTag;
  FDE_CSSTAGCACHE* pParent;
  FX_DWORD dwIDHash;
  FX_DWORD dwTagHash;
  int32_t iClassIndex;
  CFDE_DWordArray dwClassHashs;
};
typedef CFX_ObjectStackTemplate<FDE_CSSTAGCACHE> CFDE_CSSTagStack;

class CFDE_CSSAccelerator : public IFDE_CSSAccelerator, public CFX_Target {
 public:
  virtual void OnEnterTag(IFDE_CSSTagProvider* pTag);
  virtual void OnLeaveTag(IFDE_CSSTagProvider* pTag);
  void Clear() { m_Stack.RemoveAll(); }
  FDE_CSSTAGCACHE* GetTopElement() const { return m_Stack.GetTopElement(); }

 protected:
  CFDE_CSSTagStack m_Stack;
};

#endif  // XFA_SRC_FDP_SRC_CSS_FDE_CSSCACHE_H_
