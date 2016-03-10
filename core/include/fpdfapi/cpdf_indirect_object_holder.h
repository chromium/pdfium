// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_CPDF_INDIRECT_OBJECT_HOLDER_H_
#define CORE_INCLUDE_FPDFAPI_CPDF_INDIRECT_OBJECT_HOLDER_H_

#include <map>

#include "core/include/fxcrt/fx_system.h"

class CPDF_Object;
class CPDF_Parser;

class CPDF_IndirectObjectHolder {
 public:
  using iterator = std::map<FX_DWORD, CPDF_Object*>::iterator;
  using const_iterator = std::map<FX_DWORD, CPDF_Object*>::const_iterator;

  explicit CPDF_IndirectObjectHolder(CPDF_Parser* pParser);
  ~CPDF_IndirectObjectHolder();

  CPDF_Object* GetIndirectObject(FX_DWORD objnum);
  FX_DWORD AddIndirectObject(CPDF_Object* pObj);
  void ReleaseIndirectObject(FX_DWORD objnum);

  // Takes ownership of |pObj|.
  FX_BOOL InsertIndirectObject(FX_DWORD objnum, CPDF_Object* pObj);

  FX_DWORD GetLastObjNum() const { return m_LastObjNum; }
  iterator begin() { return m_IndirectObjs.begin(); }
  const_iterator begin() const { return m_IndirectObjs.begin(); }
  iterator end() { return m_IndirectObjs.end(); }
  const_iterator end() const { return m_IndirectObjs.end(); }

 protected:
  CPDF_Parser* m_pParser;
  FX_DWORD m_LastObjNum;
  std::map<FX_DWORD, CPDF_Object*> m_IndirectObjs;
};

#endif  // CORE_INCLUDE_FPDFAPI_CPDF_INDIRECT_OBJECT_HOLDER_H_
