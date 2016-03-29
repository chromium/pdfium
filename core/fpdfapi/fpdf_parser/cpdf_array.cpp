// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_name.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_number.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_reference.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_string.h"

CPDF_Array::CPDF_Array() {}

CPDF_Array::~CPDF_Array() {
  int size = m_Objects.GetSize();
  CPDF_Object** pList = m_Objects.GetData();
  for (int i = 0; i < size; i++) {
    if (pList[i])
      pList[i]->Release();
  }
}

CPDF_Object::Type CPDF_Array::GetType() const {
  return ARRAY;
}

CPDF_Array* CPDF_Array::GetArray() const {
  // The method should be made non-const if we want to not be const.
  // See bug #234.
  return const_cast<CPDF_Array*>(this);
}

bool CPDF_Array::IsArray() const {
  return true;
}

CPDF_Array* CPDF_Array::AsArray() {
  return this;
}

const CPDF_Array* CPDF_Array::AsArray() const {
  return this;
}

CPDF_Object* CPDF_Array::Clone(FX_BOOL bDirect) const {
  CPDF_Array* pCopy = new CPDF_Array();
  for (size_t i = 0; i < GetCount(); i++) {
    CPDF_Object* value = m_Objects.GetAt(i);
    pCopy->m_Objects.Add(value->Clone(bDirect));
  }
  return pCopy;
}

CFX_FloatRect CPDF_Array::GetRect() {
  CFX_FloatRect rect;
  if (!IsArray() || m_Objects.GetSize() != 4)
    return rect;

  rect.left = GetNumberAt(0);
  rect.bottom = GetNumberAt(1);
  rect.right = GetNumberAt(2);
  rect.top = GetNumberAt(3);
  return rect;
}

CFX_Matrix CPDF_Array::GetMatrix() {
  CFX_Matrix matrix;
  if (!IsArray() || m_Objects.GetSize() != 6)
    return matrix;

  matrix.Set(GetNumberAt(0), GetNumberAt(1), GetNumberAt(2), GetNumberAt(3),
             GetNumberAt(4), GetNumberAt(5));
  return matrix;
}

CPDF_Object* CPDF_Array::GetObjectAt(uint32_t i) const {
  if (i >= (uint32_t)m_Objects.GetSize())
    return nullptr;
  return m_Objects.GetAt(i);
}

CPDF_Object* CPDF_Array::GetDirectObjectAt(uint32_t i) const {
  if (i >= (uint32_t)m_Objects.GetSize())
    return nullptr;
  return m_Objects.GetAt(i)->GetDirect();
}

CFX_ByteString CPDF_Array::GetStringAt(uint32_t i) const {
  if (i >= (uint32_t)m_Objects.GetSize())
    return CFX_ByteString();
  return m_Objects.GetAt(i)->GetString();
}

CFX_ByteStringC CPDF_Array::GetConstStringAt(uint32_t i) const {
  if (i >= (uint32_t)m_Objects.GetSize())
    return CFX_ByteStringC();
  return m_Objects.GetAt(i)->GetConstString();
}

int CPDF_Array::GetIntegerAt(uint32_t i) const {
  if (i >= (uint32_t)m_Objects.GetSize())
    return 0;
  return m_Objects.GetAt(i)->GetInteger();
}

FX_FLOAT CPDF_Array::GetNumberAt(uint32_t i) const {
  if (i >= (uint32_t)m_Objects.GetSize())
    return 0;
  return m_Objects.GetAt(i)->GetNumber();
}

CPDF_Dictionary* CPDF_Array::GetDictAt(uint32_t i) const {
  CPDF_Object* p = GetDirectObjectAt(i);
  if (!p)
    return NULL;
  if (CPDF_Dictionary* pDict = p->AsDictionary())
    return pDict;
  if (CPDF_Stream* pStream = p->AsStream())
    return pStream->GetDict();
  return NULL;
}

CPDF_Stream* CPDF_Array::GetStreamAt(uint32_t i) const {
  return ToStream(GetDirectObjectAt(i));
}

CPDF_Array* CPDF_Array::GetArrayAt(uint32_t i) const {
  return ToArray(GetDirectObjectAt(i));
}

void CPDF_Array::RemoveAt(uint32_t i, uint32_t nCount) {
  if (i >= (uint32_t)m_Objects.GetSize())
    return;

  if (nCount <= 0 || nCount > m_Objects.GetSize() - i)
    return;

  for (uint32_t j = 0; j < nCount; ++j) {
    if (CPDF_Object* p = m_Objects.GetAt(i + j))
      p->Release();
  }
  m_Objects.RemoveAt(i, nCount);
}

void CPDF_Array::SetAt(uint32_t i,
                       CPDF_Object* pObj,
                       CPDF_IndirectObjectHolder* pObjs) {
  ASSERT(IsArray());
  ASSERT(i < (uint32_t)m_Objects.GetSize());
  if (i >= (uint32_t)m_Objects.GetSize())
    return;
  if (CPDF_Object* pOld = m_Objects.GetAt(i))
    pOld->Release();
  if (pObj->GetObjNum()) {
    ASSERT(pObjs);
    pObj = new CPDF_Reference(pObjs, pObj->GetObjNum());
  }
  m_Objects.SetAt(i, pObj);
}

void CPDF_Array::InsertAt(uint32_t index,
                          CPDF_Object* pObj,
                          CPDF_IndirectObjectHolder* pObjs) {
  if (pObj->GetObjNum()) {
    ASSERT(pObjs);
    pObj = new CPDF_Reference(pObjs, pObj->GetObjNum());
  }
  m_Objects.InsertAt(index, pObj);
}

void CPDF_Array::Add(CPDF_Object* pObj, CPDF_IndirectObjectHolder* pObjs) {
  if (pObj->GetObjNum()) {
    ASSERT(pObjs);
    pObj = new CPDF_Reference(pObjs, pObj->GetObjNum());
  }
  m_Objects.Add(pObj);
}

void CPDF_Array::AddName(const CFX_ByteString& str) {
  ASSERT(IsArray());
  Add(new CPDF_Name(str));
}

void CPDF_Array::AddString(const CFX_ByteString& str) {
  ASSERT(IsArray());
  Add(new CPDF_String(str, FALSE));
}

void CPDF_Array::AddInteger(int i) {
  ASSERT(IsArray());
  Add(new CPDF_Number(i));
}

void CPDF_Array::AddNumber(FX_FLOAT f) {
  ASSERT(IsArray());
  CPDF_Number* pNumber = new CPDF_Number(f);
  Add(pNumber);
}

void CPDF_Array::AddReference(CPDF_IndirectObjectHolder* pDoc,
                              uint32_t objnum) {
  ASSERT(IsArray());
  Add(new CPDF_Reference(pDoc, objnum));
}
