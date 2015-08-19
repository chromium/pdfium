// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_CSSCACHE
#define _FDE_CSSCACHE
typedef struct _FDE_CSSCACHEITEM : public CFX_Target {
  _FDE_CSSCACHEITEM(IFDE_CSSStyleSheet* p);
  ~_FDE_CSSCACHEITEM();
  IFDE_CSSStyleSheet* pStylesheet;
  FX_DWORD dwActivity;
} FDE_CSSCACHEITEM, *FDE_LPCSSCACHEITEM;
class CFDE_CSSStyleSheetCache : public IFDE_CSSStyleSheetCache,
                                public CFX_ThreadLock,
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
  std::map<CFX_ByteString, FDE_LPCSSCACHEITEM> m_Stylesheets;
  IFX_MEMAllocator* m_pFixedStore;
  int32_t m_iMaxItems;
};
typedef struct _FDE_CSSTAGCACHE : public CFX_Target {
 public:
  _FDE_CSSTAGCACHE(_FDE_CSSTAGCACHE* parent, IFDE_CSSTagProvider* tag);
  _FDE_CSSTAGCACHE(const _FDE_CSSTAGCACHE& it);
  _FDE_CSSTAGCACHE* GetParent() const { return pParent; }
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
  _FDE_CSSTAGCACHE* pParent;
  FX_DWORD dwIDHash;
  FX_DWORD dwTagHash;
  int32_t iClassIndex;
  CFDE_DWordArray dwClassHashs;
} FDE_CSSTAGCACHE, *FDE_LPCSSTAGCACHE;
typedef CFX_ObjectStackTemplate<FDE_CSSTAGCACHE> CFDE_CSSTagStack;
class CFDE_CSSAccelerator : public IFDE_CSSAccelerator, public CFX_Target {
 public:
  virtual void OnEnterTag(IFDE_CSSTagProvider* pTag);
  virtual void OnLeaveTag(IFDE_CSSTagProvider* pTag);
  void Clear() { m_Stack.RemoveAll(); }
  FDE_LPCSSTAGCACHE GetTopElement() const { return m_Stack.GetTopElement(); }

 protected:
  CFDE_CSSTagStack m_Stack;
};
#endif
