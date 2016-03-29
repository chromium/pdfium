// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"

#include "core/fpdfapi/fpdf_parser/cpdf_boolean.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_name.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_number.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_reference.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_stream.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_string.h"
#include "third_party/base/stl_util.h"

CPDF_Dictionary::CPDF_Dictionary() {}

CPDF_Dictionary::~CPDF_Dictionary() {
  for (const auto& it : m_Map)
    it.second->Release();
}

CPDF_Object::Type CPDF_Dictionary::GetType() const {
  return DICTIONARY;
}

CPDF_Dictionary* CPDF_Dictionary::GetDict() const {
  // The method should be made non-const if we want to not be const.
  // See bug #234.
  return const_cast<CPDF_Dictionary*>(this);
}

bool CPDF_Dictionary::IsDictionary() const {
  return true;
}

CPDF_Dictionary* CPDF_Dictionary::AsDictionary() {
  return this;
}

const CPDF_Dictionary* CPDF_Dictionary::AsDictionary() const {
  return this;
}

CPDF_Object* CPDF_Dictionary::Clone(FX_BOOL bDirect) const {
  CPDF_Dictionary* pCopy = new CPDF_Dictionary();
  for (const auto& it : *this)
    pCopy->m_Map.insert(std::make_pair(it.first, it.second->Clone(bDirect)));
  return pCopy;
}

CPDF_Object* CPDF_Dictionary::GetObjectBy(const CFX_ByteStringC& key) const {
  auto it = m_Map.find(key);
  if (it == m_Map.end())
    return nullptr;
  return it->second;
}
CPDF_Object* CPDF_Dictionary::GetDirectObjectBy(
    const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetObjectBy(key);
  return p ? p->GetDirect() : nullptr;
}

CFX_ByteString CPDF_Dictionary::GetStringBy(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetObjectBy(key);
  return p ? p->GetString() : CFX_ByteString();
}

CFX_ByteStringC CPDF_Dictionary::GetConstStringBy(
    const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetObjectBy(key);
  return p ? p->GetConstString() : CFX_ByteStringC();
}

CFX_WideString CPDF_Dictionary::GetUnicodeTextBy(
    const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetObjectBy(key);
  if (CPDF_Reference* pRef = ToReference(p))
    p = pRef->GetDirect();
  return p ? p->GetUnicodeText() : CFX_WideString();
}

CFX_ByteString CPDF_Dictionary::GetStringBy(const CFX_ByteStringC& key,
                                            const CFX_ByteStringC& def) const {
  CPDF_Object* p = GetObjectBy(key);
  return p ? p->GetString() : CFX_ByteString(def);
}

CFX_ByteStringC CPDF_Dictionary::GetConstStringBy(
    const CFX_ByteStringC& key,
    const CFX_ByteStringC& def) const {
  CPDF_Object* p = GetObjectBy(key);
  return p ? p->GetConstString() : CFX_ByteStringC(def);
}

int CPDF_Dictionary::GetIntegerBy(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetObjectBy(key);
  return p ? p->GetInteger() : 0;
}

int CPDF_Dictionary::GetIntegerBy(const CFX_ByteStringC& key, int def) const {
  CPDF_Object* p = GetObjectBy(key);
  return p ? p->GetInteger() : def;
}

FX_FLOAT CPDF_Dictionary::GetNumberBy(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetObjectBy(key);
  return p ? p->GetNumber() : 0;
}

FX_BOOL CPDF_Dictionary::GetBooleanBy(const CFX_ByteStringC& key,
                                      FX_BOOL bDefault) const {
  CPDF_Object* p = GetObjectBy(key);
  return ToBoolean(p) ? p->GetInteger() : bDefault;
}

CPDF_Dictionary* CPDF_Dictionary::GetDictBy(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetDirectObjectBy(key);
  if (!p)
    return nullptr;
  if (CPDF_Dictionary* pDict = p->AsDictionary())
    return pDict;
  if (CPDF_Stream* pStream = p->AsStream())
    return pStream->GetDict();
  return nullptr;
}

CPDF_Array* CPDF_Dictionary::GetArrayBy(const CFX_ByteStringC& key) const {
  return ToArray(GetDirectObjectBy(key));
}

CPDF_Stream* CPDF_Dictionary::GetStreamBy(const CFX_ByteStringC& key) const {
  return ToStream(GetDirectObjectBy(key));
}

CFX_FloatRect CPDF_Dictionary::GetRectBy(const CFX_ByteStringC& key) const {
  CFX_FloatRect rect;
  CPDF_Array* pArray = GetArrayBy(key);
  if (pArray)
    rect = pArray->GetRect();
  return rect;
}

CFX_Matrix CPDF_Dictionary::GetMatrixBy(const CFX_ByteStringC& key) const {
  CFX_Matrix matrix;
  CPDF_Array* pArray = GetArrayBy(key);
  if (pArray)
    matrix = pArray->GetMatrix();
  return matrix;
}

FX_BOOL CPDF_Dictionary::KeyExist(const CFX_ByteStringC& key) const {
  return pdfium::ContainsKey(m_Map, key);
}

bool CPDF_Dictionary::IsSignatureDict() const {
  CPDF_Object* pType = GetDirectObjectBy("Type");
  if (!pType)
    pType = GetDirectObjectBy("FT");
  return pType && pType->GetString() == "Sig";
}

void CPDF_Dictionary::SetAt(const CFX_ByteStringC& key, CPDF_Object* pObj) {
  ASSERT(IsDictionary());
  // Avoid 2 constructions of CFX_ByteString.
  CFX_ByteString key_bytestring = key;
  auto it = m_Map.find(key_bytestring);
  if (it == m_Map.end()) {
    if (pObj)
      m_Map.insert(std::make_pair(key_bytestring, pObj));
    return;
  }

  if (it->second == pObj)
    return;
  it->second->Release();

  if (pObj)
    it->second = pObj;
  else
    m_Map.erase(it);
}

void CPDF_Dictionary::RemoveAt(const CFX_ByteStringC& key) {
  auto it = m_Map.find(key);
  if (it == m_Map.end())
    return;

  it->second->Release();
  m_Map.erase(it);
}

void CPDF_Dictionary::ReplaceKey(const CFX_ByteStringC& oldkey,
                                 const CFX_ByteStringC& newkey) {
  auto old_it = m_Map.find(oldkey);
  if (old_it == m_Map.end())
    return;

  // Avoid 2 constructions of CFX_ByteString.
  CFX_ByteString newkey_bytestring = newkey;
  auto new_it = m_Map.find(newkey_bytestring);
  if (new_it == old_it)
    return;

  if (new_it != m_Map.end()) {
    new_it->second->Release();
    new_it->second = old_it->second;
  } else {
    m_Map.insert(std::make_pair(newkey_bytestring, old_it->second));
  }
  m_Map.erase(old_it);
}

void CPDF_Dictionary::SetAtInteger(const CFX_ByteStringC& key, int i) {
  SetAt(key, new CPDF_Number(i));
}

void CPDF_Dictionary::SetAtName(const CFX_ByteStringC& key,
                                const CFX_ByteString& name) {
  SetAt(key, new CPDF_Name(name));
}

void CPDF_Dictionary::SetAtString(const CFX_ByteStringC& key,
                                  const CFX_ByteString& str) {
  SetAt(key, new CPDF_String(str, FALSE));
}

void CPDF_Dictionary::SetAtReference(const CFX_ByteStringC& key,
                                     CPDF_IndirectObjectHolder* pDoc,
                                     uint32_t objnum) {
  SetAt(key, new CPDF_Reference(pDoc, objnum));
}

void CPDF_Dictionary::AddReference(const CFX_ByteStringC& key,
                                   CPDF_IndirectObjectHolder* pDoc,
                                   uint32_t objnum) {
  SetAt(key, new CPDF_Reference(pDoc, objnum));
}

void CPDF_Dictionary::SetAtNumber(const CFX_ByteStringC& key, FX_FLOAT f) {
  CPDF_Number* pNumber = new CPDF_Number(f);
  SetAt(key, pNumber);
}

void CPDF_Dictionary::SetAtBoolean(const CFX_ByteStringC& key, FX_BOOL bValue) {
  SetAt(key, new CPDF_Boolean(bValue));
}

void CPDF_Dictionary::SetAtRect(const CFX_ByteStringC& key,
                                const CFX_FloatRect& rect) {
  CPDF_Array* pArray = new CPDF_Array;
  pArray->AddNumber(rect.left);
  pArray->AddNumber(rect.bottom);
  pArray->AddNumber(rect.right);
  pArray->AddNumber(rect.top);
  SetAt(key, pArray);
}

void CPDF_Dictionary::SetAtMatrix(const CFX_ByteStringC& key,
                                  const CFX_Matrix& matrix) {
  CPDF_Array* pArray = new CPDF_Array;
  pArray->AddNumber(matrix.a);
  pArray->AddNumber(matrix.b);
  pArray->AddNumber(matrix.c);
  pArray->AddNumber(matrix.d);
  pArray->AddNumber(matrix.e);
  pArray->AddNumber(matrix.f);
  SetAt(key, pArray);
}
