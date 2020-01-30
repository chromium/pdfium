// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_NAMETREE_H_
#define CORE_FPDFDOC_CPDF_NAMETREE_H_

#include <memory>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"

class CPDF_Array;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Object;

class CPDF_NameTree {
 public:
  explicit CPDF_NameTree(CPDF_Dictionary* pRoot);
  CPDF_NameTree(CPDF_Document* pDoc, const ByteString& category);
  ~CPDF_NameTree();

  bool AddValueAndName(RetainPtr<CPDF_Object> pObj, const WideString& name);
  bool DeleteValueAndName(int nIndex);

  CPDF_Object* LookupValueAndName(int nIndex, WideString* csName) const;
  CPDF_Object* LookupValue(const WideString& csName) const;
  CPDF_Array* LookupNamedDest(CPDF_Document* pDoc, const WideString& sName);

  size_t GetCount() const;
  CPDF_Dictionary* GetRootForTest() const { return m_pRoot.Get(); }

 private:
  RetainPtr<CPDF_Dictionary> m_pRoot;
};

#endif  // CORE_FPDFDOC_CPDF_NAMETREE_H_
