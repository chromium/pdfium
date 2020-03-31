// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_array.h"

#include <set>
#include <utility>

#include "core/fpdfapi/parser/cpdf_boolean.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/fx_stream.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CPDF_Array::CPDF_Array() = default;

CPDF_Array::CPDF_Array(const WeakPtr<ByteStringPool>& pPool) : m_pPool(pPool) {}

CPDF_Array::~CPDF_Array() {
  // Break cycles for cyclic references.
  m_ObjNum = kInvalidObjNum;
  for (auto& it : m_Objects) {
    if (it && it->GetObjNum() == kInvalidObjNum)
      it.Leak();
  }
}

CPDF_Object::Type CPDF_Array::GetType() const {
  return kArray;
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

RetainPtr<CPDF_Object> CPDF_Array::Clone() const {
  return CloneObjectNonCyclic(false);
}

RetainPtr<CPDF_Object> CPDF_Array::CloneNonCyclic(
    bool bDirect,
    std::set<const CPDF_Object*>* pVisited) const {
  pVisited->insert(this);
  auto pCopy = pdfium::MakeRetain<CPDF_Array>();
  for (const auto& pValue : m_Objects) {
    if (!pdfium::ContainsKey(*pVisited, pValue.Get())) {
      std::set<const CPDF_Object*> visited(*pVisited);
      if (auto obj = pValue->CloneNonCyclic(bDirect, &visited))
        pCopy->m_Objects.push_back(std::move(obj));
    }
  }
  return pCopy;
}

CFX_FloatRect CPDF_Array::GetRect() const {
  CFX_FloatRect rect;
  if (m_Objects.size() != 4)
    return rect;

  rect.left = GetNumberAt(0);
  rect.bottom = GetNumberAt(1);
  rect.right = GetNumberAt(2);
  rect.top = GetNumberAt(3);
  return rect;
}

CFX_Matrix CPDF_Array::GetMatrix() const {
  if (m_Objects.size() != 6)
    return CFX_Matrix();

  return CFX_Matrix(GetNumberAt(0), GetNumberAt(1), GetNumberAt(2),
                    GetNumberAt(3), GetNumberAt(4), GetNumberAt(5));
}

CPDF_Object* CPDF_Array::GetObjectAt(size_t index) {
  if (index >= m_Objects.size())
    return nullptr;
  return m_Objects[index].Get();
}

const CPDF_Object* CPDF_Array::GetObjectAt(size_t index) const {
  if (index >= m_Objects.size())
    return nullptr;
  return m_Objects[index].Get();
}

CPDF_Object* CPDF_Array::GetDirectObjectAt(size_t index) {
  if (index >= m_Objects.size())
    return nullptr;
  return m_Objects[index]->GetDirect();
}

const CPDF_Object* CPDF_Array::GetDirectObjectAt(size_t index) const {
  if (index >= m_Objects.size())
    return nullptr;
  return m_Objects[index]->GetDirect();
}

ByteString CPDF_Array::GetStringAt(size_t index) const {
  if (index >= m_Objects.size())
    return ByteString();
  return m_Objects[index]->GetString();
}

WideString CPDF_Array::GetUnicodeTextAt(size_t index) const {
  if (index >= m_Objects.size())
    return WideString();
  return m_Objects[index]->GetUnicodeText();
}

bool CPDF_Array::GetBooleanAt(size_t index, bool bDefault) const {
  if (index >= m_Objects.size())
    return bDefault;
  const CPDF_Object* p = m_Objects[index].Get();
  return ToBoolean(p) ? p->GetInteger() != 0 : bDefault;
}

int CPDF_Array::GetIntegerAt(size_t index) const {
  if (index >= m_Objects.size())
    return 0;
  return m_Objects[index]->GetInteger();
}

float CPDF_Array::GetNumberAt(size_t index) const {
  if (index >= m_Objects.size())
    return 0;
  return m_Objects[index]->GetNumber();
}

CPDF_Dictionary* CPDF_Array::GetDictAt(size_t index) {
  CPDF_Object* p = GetDirectObjectAt(index);
  if (!p)
    return nullptr;
  if (CPDF_Dictionary* pDict = p->AsDictionary())
    return pDict;
  if (CPDF_Stream* pStream = p->AsStream())
    return pStream->GetDict();
  return nullptr;
}

const CPDF_Dictionary* CPDF_Array::GetDictAt(size_t index) const {
  const CPDF_Object* p = GetDirectObjectAt(index);
  if (!p)
    return nullptr;
  if (const CPDF_Dictionary* pDict = p->AsDictionary())
    return pDict;
  if (const CPDF_Stream* pStream = p->AsStream())
    return pStream->GetDict();
  return nullptr;
}

CPDF_Stream* CPDF_Array::GetStreamAt(size_t index) {
  return ToStream(GetDirectObjectAt(index));
}

const CPDF_Stream* CPDF_Array::GetStreamAt(size_t index) const {
  return ToStream(GetDirectObjectAt(index));
}

CPDF_Array* CPDF_Array::GetArrayAt(size_t index) {
  return ToArray(GetDirectObjectAt(index));
}

const CPDF_Array* CPDF_Array::GetArrayAt(size_t index) const {
  return ToArray(GetDirectObjectAt(index));
}

void CPDF_Array::Clear() {
  CHECK(!IsLocked());
  m_Objects.clear();
}

void CPDF_Array::RemoveAt(size_t index) {
  CHECK(!IsLocked());
  if (index < m_Objects.size())
    m_Objects.erase(m_Objects.begin() + index);
}

void CPDF_Array::ConvertToIndirectObjectAt(size_t index,
                                           CPDF_IndirectObjectHolder* pHolder) {
  CHECK(!IsLocked());
  if (index >= m_Objects.size())
    return;

  if (!m_Objects[index] || m_Objects[index]->IsReference())
    return;

  CPDF_Object* pNew = pHolder->AddIndirectObject(std::move(m_Objects[index]));
  m_Objects[index] = pNew->MakeReference(pHolder);
}

CPDF_Object* CPDF_Array::SetAt(size_t index, RetainPtr<CPDF_Object> pObj) {
  CHECK(!IsLocked());
  ASSERT(!pObj || pObj->IsInline());
  if (index >= m_Objects.size()) {
    NOTREACHED();
    return nullptr;
  }
  CPDF_Object* pRet = pObj.Get();
  m_Objects[index] = std::move(pObj);
  return pRet;
}

CPDF_Object* CPDF_Array::InsertAt(size_t index, RetainPtr<CPDF_Object> pObj) {
  CHECK(!IsLocked());
  CHECK(!pObj || pObj->IsInline());
  CPDF_Object* pRet = pObj.Get();
  if (index >= m_Objects.size()) {
    // Allocate space first.
    m_Objects.resize(index + 1);
    m_Objects[index] = std::move(pObj);
  } else {
    // Directly insert.
    m_Objects.insert(m_Objects.begin() + index, std::move(pObj));
  }
  return pRet;
}

CPDF_Object* CPDF_Array::Append(RetainPtr<CPDF_Object> pObj) {
  CHECK(!IsLocked());
  CHECK(!pObj || pObj->IsInline());
  CPDF_Object* pRet = pObj.Get();
  m_Objects.push_back(std::move(pObj));
  return pRet;
}

bool CPDF_Array::WriteTo(IFX_ArchiveStream* archive,
                         const CPDF_Encryptor* encryptor) const {
  if (!archive->WriteString("["))
    return false;

  for (size_t i = 0; i < size(); ++i) {
    if (!GetObjectAt(i)->WriteTo(archive, encryptor))
      return false;
  }
  return archive->WriteString("]");
}

CPDF_ArrayLocker::CPDF_ArrayLocker(const CPDF_Array* pArray)
    : m_pArray(pArray) {
  m_pArray->m_LockCount++;
}

CPDF_ArrayLocker::~CPDF_ArrayLocker() {
  m_pArray->m_LockCount--;
}
