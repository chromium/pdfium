// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_ARRAY_H_
#define CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_ARRAY_H_

#include "core/fpdfapi/fpdf_parser/include/cpdf_indirect_object_holder.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_object.h"
#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_coordinates.h"

class CPDF_Array : public CPDF_Object {
 public:
  CPDF_Array();

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
  CPDF_Array* GetArray() const override;
  bool IsArray() const override;
  CPDF_Array* AsArray() override;
  const CPDF_Array* AsArray() const override;

  uint32_t GetCount() const { return m_Objects.GetSize(); }
  CPDF_Object* GetObjectAt(uint32_t index) const;
  CPDF_Object* GetDirectObjectAt(uint32_t index) const;
  CFX_ByteString GetStringAt(uint32_t index) const;
  CFX_ByteStringC GetConstStringAt(uint32_t index) const;
  int GetIntegerAt(uint32_t index) const;
  FX_FLOAT GetNumberAt(uint32_t index) const;
  CPDF_Dictionary* GetDictAt(uint32_t index) const;
  CPDF_Stream* GetStreamAt(uint32_t index) const;
  CPDF_Array* GetArrayAt(uint32_t index) const;
  FX_FLOAT GetFloatAt(uint32_t index) const { return GetNumberAt(index); }
  CFX_Matrix GetMatrix();
  CFX_FloatRect GetRect();

  void SetAt(uint32_t index,
             CPDF_Object* pObj,
             CPDF_IndirectObjectHolder* pObjs = nullptr);
  void InsertAt(uint32_t index,
                CPDF_Object* pObj,
                CPDF_IndirectObjectHolder* pObjs = nullptr);
  void RemoveAt(uint32_t index, uint32_t nCount = 1);

  void Add(CPDF_Object* pObj, CPDF_IndirectObjectHolder* pObjs = nullptr);
  void AddNumber(FX_FLOAT f);
  void AddInteger(int i);
  void AddString(const CFX_ByteString& str);
  void AddName(const CFX_ByteString& str);
  void AddReference(CPDF_IndirectObjectHolder* pDoc, uint32_t objnum);
  void AddReference(CPDF_IndirectObjectHolder* pDoc, CPDF_Object* obj) {
    AddReference(pDoc, obj->GetObjNum());
  }

 protected:
  ~CPDF_Array() override;

  CFX_ArrayTemplate<CPDF_Object*> m_Objects;
};

#endif  // CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_ARRAY_H_
