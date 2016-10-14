// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_INDIRECT_OBJECT_HOLDER_H_
#define CORE_FPDFAPI_PARSER_CPDF_INDIRECT_OBJECT_HOLDER_H_

#include <map>
#include <memory>

#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fxcrt/cfx_string_pool_template.h"
#include "core/fxcrt/cfx_weak_ptr.h"
#include "core/fxcrt/fx_system.h"

class CPDF_Array;
class CPDF_Dictionary;
class CPDF_Stream;

class CPDF_IndirectObjectHolder {
 public:
  using const_iterator =
      std::map<uint32_t, std::unique_ptr<CPDF_Object>>::const_iterator;

  CPDF_IndirectObjectHolder();
  virtual ~CPDF_IndirectObjectHolder();

  CPDF_Object* GetIndirectObject(uint32_t objnum) const;
  CPDF_Object* GetOrParseIndirectObject(uint32_t objnum);
  void DeleteIndirectObject(uint32_t objnum);

  // Take ownership of |pObj|, returns unowned pointer to it.
  CPDF_Object* AddIndirectObject(UniqueObject pObj);

  // Adds and owns a new object, returns unowned pointer to it.
  CPDF_Array* AddIndirectArray();
  CPDF_Dictionary* AddIndirectDictionary();
  CPDF_Dictionary* AddIndirectDictionary(
      const CFX_WeakPtr<CFX_ByteStringPool>& pPool);
  CPDF_Stream* AddIndirectStream();
  CPDF_Stream* AddIndirectStream(uint8_t* pData,
                                 uint32_t size,
                                 CPDF_Dictionary* pDict);

  bool ReplaceIndirectObjectIfHigherGeneration(uint32_t objnum,
                                               UniqueObject pObj);

  uint32_t GetLastObjNum() const { return m_LastObjNum; }
  void SetLastObjNum(uint32_t objnum) { m_LastObjNum = objnum; }

  const_iterator begin() const { return m_IndirectObjs.begin(); }
  const_iterator end() const { return m_IndirectObjs.end(); }

 protected:
  virtual CPDF_Object* ParseIndirectObject(uint32_t objnum);

 private:
  uint32_t m_LastObjNum;

  // Ordinary deleter, not Release().
  std::map<uint32_t, std::unique_ptr<CPDF_Object>> m_IndirectObjs;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_INDIRECT_OBJECT_HOLDER_H_
