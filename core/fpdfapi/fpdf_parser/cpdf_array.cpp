// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"

#include <set>

#include "core/fpdfapi/fpdf_parser/include/cpdf_name.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_number.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_reference.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_string.h"
#include "third_party/base/logging.h"
#include "third_party/base/stl_util.h"

CPDF_Array::CPDF_Array() {}

CPDF_Array::~CPDF_Array() {
  // Mark the object as deleted so that it will not be deleted again
  // in case of cyclic references.
  m_ObjNum = kInvalidObjNum;
  for (auto& it : m_Objects) {
    if (it)
      it->Release();
  }
}

CPDF_Object::Type CPDF_Array::GetType() const {
  return ARRAY;
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

CPDF_Object* CPDF_Array::Clone() const {
  return CloneObjectNonCyclic(false);
}

CPDF_Object* CPDF_Array::CloneNonCyclic(
    bool bDirect,
    std::set<const CPDF_Object*>* pVisited) const {
  pVisited->insert(this);
  CPDF_Array* pCopy = new CPDF_Array();
  for (size_t i = 0; i < GetCount(); i++) {
    CPDF_Object* value = m_Objects[i];
    if (!pdfium::ContainsKey(*pVisited, value))
      pCopy->m_Objects.push_back(value->CloneNonCyclic(bDirect, pVisited));
  }
  return pCopy;
}

CFX_FloatRect CPDF_Array::GetRect() {
  CFX_FloatRect rect;
  if (!IsArray() || m_Objects.size() != 4)
    return rect;

  rect.left = GetNumberAt(0);
  rect.bottom = GetNumberAt(1);
  rect.right = GetNumberAt(2);
  rect.top = GetNumberAt(3);
  return rect;
}

CFX_Matrix CPDF_Array::GetMatrix() {
  CFX_Matrix matrix;
  if (!IsArray() || m_Objects.size() != 6)
    return matrix;

  matrix.Set(GetNumberAt(0), GetNumberAt(1), GetNumberAt(2), GetNumberAt(3),
             GetNumberAt(4), GetNumberAt(5));
  return matrix;
}

CPDF_Object* CPDF_Array::GetObjectAt(size_t i) const {
  if (i >= m_Objects.size())
    return nullptr;
  return m_Objects[i];
}

CPDF_Object* CPDF_Array::GetDirectObjectAt(size_t i) const {
  if (i >= m_Objects.size())
    return nullptr;
  return m_Objects[i]->GetDirect();
}

CFX_ByteString CPDF_Array::GetStringAt(size_t i) const {
  if (i >= m_Objects.size())
    return CFX_ByteString();
  return m_Objects[i]->GetString();
}

int CPDF_Array::GetIntegerAt(size_t i) const {
  if (i >= m_Objects.size())
    return 0;
  return m_Objects[i]->GetInteger();
}

FX_FLOAT CPDF_Array::GetNumberAt(size_t i) const {
  if (i >= m_Objects.size())
    return 0;
  return m_Objects[i]->GetNumber();
}

CPDF_Dictionary* CPDF_Array::GetDictAt(size_t i) const {
  CPDF_Object* p = GetDirectObjectAt(i);
  if (!p)
    return nullptr;
  if (CPDF_Dictionary* pDict = p->AsDictionary())
    return pDict;
  if (CPDF_Stream* pStream = p->AsStream())
    return pStream->GetDict();
  return nullptr;
}

CPDF_Stream* CPDF_Array::GetStreamAt(size_t i) const {
  return ToStream(GetDirectObjectAt(i));
}

CPDF_Array* CPDF_Array::GetArrayAt(size_t i) const {
  return ToArray(GetDirectObjectAt(i));
}

void CPDF_Array::RemoveAt(size_t i, size_t nCount) {
  if (i >= m_Objects.size())
    return;

  if (nCount <= 0 || nCount > m_Objects.size() - i)
    return;

  for (size_t j = 0; j < nCount; ++j) {
    if (CPDF_Object* p = m_Objects[i + j])
      p->Release();
  }
  m_Objects.erase(m_Objects.begin() + i, m_Objects.begin() + i + nCount);
}

void CPDF_Array::SetAt(size_t i, CPDF_Object* pObj) {
  ASSERT(IsArray());
  CHECK(!pObj || pObj->GetObjNum() == 0);
  if (i >= m_Objects.size()) {
    ASSERT(false);
    return;
  }
  if (CPDF_Object* pOld = m_Objects[i])
    pOld->Release();

  m_Objects[i] = pObj;
}

void CPDF_Array::InsertAt(size_t index, CPDF_Object* pObj) {
  ASSERT(IsArray());
  CHECK(!pObj || pObj->GetObjNum() == 0);
  if (index >= m_Objects.size()) {
    // Allocate space first.
    m_Objects.resize(index + 1, nullptr);
    m_Objects[index] = pObj;
  } else {
    // Directly insert.
    m_Objects.insert(m_Objects.begin() + index, pObj);
  }
}

void CPDF_Array::Add(CPDF_Object* pObj) {
  ASSERT(IsArray());
  CHECK(!pObj || pObj->GetObjNum() == 0);
  m_Objects.push_back(pObj);
}

void CPDF_Array::AddName(const CFX_ByteString& str) {
  Add(new CPDF_Name(str));
}

void CPDF_Array::AddString(const CFX_ByteString& str) {
  Add(new CPDF_String(str, FALSE));
}

void CPDF_Array::AddInteger(int i) {
  Add(new CPDF_Number(i));
}

void CPDF_Array::AddNumber(FX_FLOAT f) {
  Add(new CPDF_Number(f));
}

void CPDF_Array::AddReference(CPDF_IndirectObjectHolder* pDoc,
                              uint32_t objnum) {
  Add(new CPDF_Reference(pDoc, objnum));
}
