// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_INDIRECT_OBJECT_HOLDER_H_
#define CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_INDIRECT_OBJECT_HOLDER_H_

#include <map>
#include <memory>

#include "core/fxcrt/include/fx_system.h"

class CPDF_Object;

class CPDF_IndirectObjectHolder {
 public:
  using const_iterator =
      std::map<uint32_t, std::unique_ptr<CPDF_Object>>::const_iterator;

  CPDF_IndirectObjectHolder();
  virtual ~CPDF_IndirectObjectHolder();

  CPDF_Object* GetIndirectObject(uint32_t objnum) const;
  CPDF_Object* GetOrParseIndirectObject(uint32_t objnum);
  void ReleaseIndirectObject(uint32_t objnum);

  // Take ownership of |pObj|.
  uint32_t AddIndirectObject(CPDF_Object* pObj);
  bool ReplaceIndirectObjectIfHigherGeneration(uint32_t objnum,
                                               CPDF_Object* pObj);

  uint32_t GetLastObjNum() const { return m_LastObjNum; }
  void SetLastObjNum(uint32_t objnum) { m_LastObjNum = objnum; }

  const_iterator begin() const { return m_IndirectObjs.begin(); }
  const_iterator end() const { return m_IndirectObjs.end(); }

 protected:
  virtual CPDF_Object* ParseIndirectObject(uint32_t objnum);

 private:
  uint32_t m_LastObjNum;
  std::map<uint32_t, std::unique_ptr<CPDF_Object>> m_IndirectObjs;
};

#endif  // CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_INDIRECT_OBJECT_HOLDER_H_
