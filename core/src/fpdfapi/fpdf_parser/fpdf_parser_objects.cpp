// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfapi/fpdf_objects.h"

#include <algorithm>

#include "core/include/fpdfapi/fpdf_parser.h"
#include "core/include/fxcrt/fx_string.h"
#include "third_party/base/stl_util.h"

namespace {

const FX_DWORD kBlockSize = 1024;

}  // namespace

// static
int CPDF_Object::s_nCurRefDepth = 0;

void CPDF_Object::Release() {
  if (m_ObjNum) {
    return;
  }
  Destroy();
}
void CPDF_Object::Destroy() {
  switch (m_Type) {
    case PDFOBJ_STRING:
      delete AsString();
      break;
    case PDFOBJ_NAME:
      delete AsName();
      break;
    case PDFOBJ_ARRAY:
      delete AsArray();
      break;
    case PDFOBJ_DICTIONARY:
      delete AsDictionary();
      break;
    case PDFOBJ_STREAM:
      delete AsStream();
      break;
    default:
      delete this;
  }
}
CFX_ByteString CPDF_Object::GetString() const {
  switch (m_Type) {
    case PDFOBJ_BOOLEAN:
      return AsBoolean()->m_bValue ? "true" : "false";
    case PDFOBJ_NUMBER:
      return AsNumber()->GetString();
    case PDFOBJ_STRING:
      return AsString()->m_String;
    case PDFOBJ_NAME:
      return AsName()->m_Name;
    case PDFOBJ_REFERENCE: {
      const CPDF_Reference* pRef = AsReference();
      if (!pRef->m_pObjList)
        break;

      CPDF_Object* pObj =
          pRef->m_pObjList->GetIndirectObject(pRef->GetRefObjNum(), nullptr);
      return pObj ? pObj->GetString() : CFX_ByteString();
    }
  }
  return CFX_ByteString();
}
CFX_ByteStringC CPDF_Object::GetConstString() const {
  switch (m_Type) {
    case PDFOBJ_STRING: {
      CFX_ByteString str = AsString()->m_String;
      return CFX_ByteStringC((const uint8_t*)str, str.GetLength());
    }
    case PDFOBJ_NAME: {
      CFX_ByteString name = AsName()->m_Name;
      return CFX_ByteStringC((const uint8_t*)name, name.GetLength());
    }
    case PDFOBJ_REFERENCE: {
      const CPDF_Reference* pRef = AsReference();
      if (!pRef->m_pObjList)
        break;

      CPDF_Object* pObj =
          pRef->m_pObjList->GetIndirectObject(pRef->GetRefObjNum(), nullptr);
      return pObj ? pObj->GetConstString() : CFX_ByteStringC();
    }
  }
  return CFX_ByteStringC();
}

FX_FLOAT CPDF_Object::GetNumber() const {
  switch (m_Type) {
    case PDFOBJ_NUMBER:
      return AsNumber()->GetNumber();
    case PDFOBJ_REFERENCE: {
      const CPDF_Reference* pRef = AsReference();
      if (!pRef->m_pObjList)
        break;

      CPDF_Object* pObj =
          pRef->m_pObjList->GetIndirectObject(pRef->GetRefObjNum(), nullptr);
      return pObj ? pObj->GetNumber() : 0;
    }
  }
  return 0;
}

FX_FLOAT CPDF_Object::GetNumber16() const {
  return GetNumber();
}

int CPDF_Object::GetInteger() const {
  CFX_AutoRestorer<int> restorer(&s_nCurRefDepth);
  if (++s_nCurRefDepth > kObjectRefMaxDepth)
    return 0;

  switch (m_Type) {
    case PDFOBJ_BOOLEAN:
      return AsBoolean()->m_bValue;
    case PDFOBJ_NUMBER:
      return AsNumber()->GetInteger();
    case PDFOBJ_REFERENCE: {
      const CPDF_Reference* pRef = AsReference();
      PARSE_CONTEXT context;
      FXSYS_memset(&context, 0, sizeof(PARSE_CONTEXT));
      if (!pRef->m_pObjList)
        return 0;

      CPDF_Object* pObj =
          pRef->m_pObjList->GetIndirectObject(pRef->GetRefObjNum(), &context);
      return pObj ? pObj->GetInteger() : 0;
    }
  }
  return 0;
}

CPDF_Dictionary* CPDF_Object::GetDict() const {
  switch (m_Type) {
    case PDFOBJ_DICTIONARY:
      // The method should be made non-const if we want to not be const.
      // See bug #234.
      return const_cast<CPDF_Dictionary*>(AsDictionary());
    case PDFOBJ_STREAM:
      return AsStream()->GetDict();
    case PDFOBJ_REFERENCE: {
      const CPDF_Reference* pRef = AsReference();
      CPDF_IndirectObjectHolder* pIndirect = pRef->GetObjList();
      if (!pIndirect)
        return nullptr;
      CPDF_Object* pObj =
          pIndirect->GetIndirectObject(pRef->GetRefObjNum(), nullptr);
      if (!pObj || (pObj == this))
        return nullptr;
      return pObj->GetDict();
    }
    default:
      return nullptr;
  }
}

CPDF_Array* CPDF_Object::GetArray() const {
  // The method should be made non-const if we want to not be const.
  // See bug #234.
  return const_cast<CPDF_Array*>(AsArray());
}

void CPDF_Object::SetString(const CFX_ByteString& str) {
  switch (m_Type) {
    case PDFOBJ_BOOLEAN:
      AsBoolean()->m_bValue = (str == "true");
      return;
    case PDFOBJ_NUMBER:
      AsNumber()->SetString(str);
      return;
    case PDFOBJ_STRING:
      AsString()->m_String = str;
      return;
    case PDFOBJ_NAME:
      AsName()->m_Name = str;
      return;
  }
  ASSERT(FALSE);
}
int CPDF_Object::GetDirectType() const {
  const CPDF_Reference* pRef = AsReference();
  if (!pRef)
    return m_Type;
  return pRef->m_pObjList->GetIndirectType(pRef->GetRefObjNum());
}
FX_BOOL CPDF_Object::IsIdentical(CPDF_Object* pOther) const {
  if (this == pOther)
    return TRUE;
  if (!pOther)
    return FALSE;
  if (pOther->m_Type != m_Type) {
    if (IsReference() && GetDirect())
      return GetDirect()->IsIdentical(pOther);
    if (pOther->IsReference())
      return IsIdentical(pOther->GetDirect());
    return FALSE;
  }
  switch (m_Type) {
    case PDFOBJ_BOOLEAN:
      return AsBoolean()->Identical(pOther->AsBoolean());
    case PDFOBJ_NUMBER:
      return AsNumber()->Identical(pOther->AsNumber());
    case PDFOBJ_STRING:
      return AsString()->Identical(pOther->AsString());
    case PDFOBJ_NAME:
      return AsName()->Identical(pOther->AsName());
    case PDFOBJ_ARRAY:
      return AsArray()->Identical(pOther->AsArray());
    case PDFOBJ_DICTIONARY:
      return AsDictionary()->Identical(pOther->AsDictionary());
    case PDFOBJ_NULL:
      return TRUE;
    case PDFOBJ_STREAM:
      return AsStream()->Identical(pOther->AsStream());
    case PDFOBJ_REFERENCE:
      return AsReference()->Identical(pOther->AsReference());
  }
  return FALSE;
}

CPDF_Object* CPDF_Object::GetDirect() const {
  const CPDF_Reference* pRef = AsReference();
  if (!pRef)
    return const_cast<CPDF_Object*>(this);
  if (!pRef->m_pObjList)
    return nullptr;
  return pRef->m_pObjList->GetIndirectObject(pRef->GetRefObjNum(), nullptr);
}

CPDF_Object* CPDF_Object::Clone(FX_BOOL bDirect) const {
  std::set<FX_DWORD> visited;
  return CloneInternal(bDirect, &visited);
}
CPDF_Object* CPDF_Object::CloneInternal(FX_BOOL bDirect,
                                        std::set<FX_DWORD>* visited) const {
  switch (m_Type) {
    case PDFOBJ_BOOLEAN:
      return new CPDF_Boolean(AsBoolean()->m_bValue);
    case PDFOBJ_NUMBER: {
      const CPDF_Number* pThis = AsNumber();
      return new CPDF_Number(pThis->m_bInteger ? pThis->m_Integer
                                               : pThis->m_Float);
    }
    case PDFOBJ_STRING: {
      const CPDF_String* pString = AsString();
      return new CPDF_String(pString->m_String, pString->IsHex());
    }
    case PDFOBJ_NAME:
      return new CPDF_Name(AsName()->m_Name);
    case PDFOBJ_ARRAY: {
      CPDF_Array* pCopy = new CPDF_Array();
      const CPDF_Array* pThis = AsArray();
      int n = pThis->GetCount();
      for (int i = 0; i < n; i++) {
        CPDF_Object* value = pThis->m_Objects.GetAt(i);
        pCopy->m_Objects.Add(value->CloneInternal(bDirect, visited));
      }
      return pCopy;
    }
    case PDFOBJ_DICTIONARY: {
      CPDF_Dictionary* pCopy = new CPDF_Dictionary();
      const CPDF_Dictionary* pThis = AsDictionary();
      for (const auto& it : *pThis) {
        pCopy->m_Map.insert(std::make_pair(
            it.first, it.second->CloneInternal(bDirect, visited)));
      }
      return pCopy;
    }
    case PDFOBJ_NULL: {
      return new CPDF_Null;
    }
    case PDFOBJ_STREAM: {
      const CPDF_Stream* pThis = AsStream();
      CPDF_StreamAcc acc;
      acc.LoadAllData(pThis, TRUE);
      FX_DWORD streamSize = acc.GetSize();
      CPDF_Dictionary* pDict = pThis->GetDict();
      if (pDict) {
        pDict = ToDictionary(pDict->CloneInternal(bDirect, visited));
      }
      return new CPDF_Stream(acc.DetachData(), streamSize, pDict);
    }
    case PDFOBJ_REFERENCE: {
      const CPDF_Reference* pRef = AsReference();
      FX_DWORD obj_num = pRef->GetRefObjNum();
      if (bDirect && !pdfium::ContainsKey(*visited, obj_num)) {
        visited->insert(obj_num);
        auto* pDirect = pRef->GetDirect();
        return pDirect ? pDirect->CloneInternal(TRUE, visited) : nullptr;
      }
      return new CPDF_Reference(pRef->m_pObjList, obj_num);
    }
  }
  return NULL;
}
CPDF_Object* CPDF_Object::CloneRef(CPDF_IndirectObjectHolder* pDoc) const {
  if (m_ObjNum) {
    return new CPDF_Reference(pDoc, m_ObjNum);
  }
  return Clone();
}
CFX_WideString CPDF_Object::GetUnicodeText(CFX_CharMap* pCharMap) const {
  if (const CPDF_String* pString = AsString())
    return PDF_DecodeText(pString->m_String, pCharMap);

  if (const CPDF_Stream* pStream = AsStream()) {
    CPDF_StreamAcc stream;
    stream.LoadAllData(pStream, FALSE);
    CFX_WideString result =
        PDF_DecodeText(stream.GetData(), stream.GetSize(), pCharMap);
    return result;
  }
  if (const CPDF_Name* pName = AsName())
    return PDF_DecodeText(pName->m_Name, pCharMap);
  return CFX_WideString();
}
void CPDF_Object::SetUnicodeText(const FX_WCHAR* pUnicodes, int len) {
  if (CPDF_String* pString = AsString()) {
    pString->m_String = PDF_EncodeText(pUnicodes, len);
  } else if (CPDF_Stream* pStream = AsStream()) {
    CFX_ByteString result = PDF_EncodeText(pUnicodes, len);
    pStream->SetData((uint8_t*)result.c_str(), result.GetLength(), FALSE,
                     FALSE);
  }
}

CPDF_Array* CPDF_Object::AsArray() {
  return IsArray() ? static_cast<CPDF_Array*>(this) : nullptr;
}

const CPDF_Array* CPDF_Object::AsArray() const {
  return IsArray() ? static_cast<const CPDF_Array*>(this) : nullptr;
}

CPDF_Boolean* CPDF_Object::AsBoolean() {
  return IsBoolean() ? static_cast<CPDF_Boolean*>(this) : nullptr;
}

const CPDF_Boolean* CPDF_Object::AsBoolean() const {
  return IsBoolean() ? static_cast<const CPDF_Boolean*>(this) : nullptr;
}

CPDF_Dictionary* CPDF_Object::AsDictionary() {
  return IsDictionary() ? static_cast<CPDF_Dictionary*>(this) : nullptr;
}

const CPDF_Dictionary* CPDF_Object::AsDictionary() const {
  return IsDictionary() ? static_cast<const CPDF_Dictionary*>(this) : nullptr;
}

CPDF_Name* CPDF_Object::AsName() {
  return IsName() ? static_cast<CPDF_Name*>(this) : nullptr;
}

const CPDF_Name* CPDF_Object::AsName() const {
  return IsName() ? static_cast<const CPDF_Name*>(this) : nullptr;
}

CPDF_Number* CPDF_Object::AsNumber() {
  return IsNumber() ? static_cast<CPDF_Number*>(this) : nullptr;
}

const CPDF_Number* CPDF_Object::AsNumber() const {
  return IsNumber() ? static_cast<const CPDF_Number*>(this) : nullptr;
}

CPDF_Reference* CPDF_Object::AsReference() {
  return IsReference() ? static_cast<CPDF_Reference*>(this) : nullptr;
}

const CPDF_Reference* CPDF_Object::AsReference() const {
  return IsReference() ? static_cast<const CPDF_Reference*>(this) : nullptr;
}

CPDF_Stream* CPDF_Object::AsStream() {
  return IsStream() ? static_cast<CPDF_Stream*>(this) : nullptr;
}

const CPDF_Stream* CPDF_Object::AsStream() const {
  return IsStream() ? static_cast<const CPDF_Stream*>(this) : nullptr;
}

CPDF_String* CPDF_Object::AsString() {
  return IsString() ? static_cast<CPDF_String*>(this) : nullptr;
}

const CPDF_String* CPDF_Object::AsString() const {
  return IsString() ? static_cast<const CPDF_String*>(this) : nullptr;
}

CPDF_Number::CPDF_Number(int value)
    : CPDF_Object(PDFOBJ_NUMBER), m_bInteger(TRUE), m_Integer(value) {}

CPDF_Number::CPDF_Number(FX_FLOAT value)
    : CPDF_Object(PDFOBJ_NUMBER), m_bInteger(FALSE), m_Float(value) {}

CPDF_Number::CPDF_Number(const CFX_ByteStringC& str)
    : CPDF_Object(PDFOBJ_NUMBER) {
  FX_atonum(str, m_bInteger, &m_Integer);
}

void CPDF_Number::SetString(const CFX_ByteStringC& str) {
  FX_atonum(str, m_bInteger, &m_Integer);
}
FX_BOOL CPDF_Number::Identical(CPDF_Number* pOther) const {
  return m_bInteger == pOther->m_bInteger && m_Integer == pOther->m_Integer;
}
CFX_ByteString CPDF_Number::GetString() const {
  return m_bInteger ? CFX_ByteString::FormatInteger(m_Integer, FXFORMAT_SIGNED)
                    : CFX_ByteString::FormatFloat(m_Float);
}
void CPDF_Number::SetNumber(FX_FLOAT value) {
  m_bInteger = FALSE;
  m_Float = value;
}
CPDF_String::CPDF_String(const CFX_WideString& str)
    : CPDF_Object(PDFOBJ_STRING), m_bHex(FALSE) {
  m_String = PDF_EncodeText(str);
}
CPDF_Array::~CPDF_Array() {
  int size = m_Objects.GetSize();
  CPDF_Object** pList = m_Objects.GetData();
  for (int i = 0; i < size; i++) {
    if (pList[i])
      pList[i]->Release();
  }
}
CFX_FloatRect CPDF_Array::GetRect() {
  CFX_FloatRect rect;
  if (!IsArray() || m_Objects.GetSize() != 4)
    return rect;

  rect.left = GetNumber(0);
  rect.bottom = GetNumber(1);
  rect.right = GetNumber(2);
  rect.top = GetNumber(3);
  return rect;
}
CFX_Matrix CPDF_Array::GetMatrix() {
  CFX_Matrix matrix;
  if (!IsArray() || m_Objects.GetSize() != 6)
    return matrix;

  matrix.Set(GetNumber(0), GetNumber(1), GetNumber(2), GetNumber(3),
             GetNumber(4), GetNumber(5));
  return matrix;
}
CPDF_Object* CPDF_Array::GetElement(FX_DWORD i) const {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return nullptr;
  return m_Objects.GetAt(i);
}
CPDF_Object* CPDF_Array::GetElementValue(FX_DWORD i) const {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return nullptr;
  return m_Objects.GetAt(i)->GetDirect();
}
CFX_ByteString CPDF_Array::GetString(FX_DWORD i) const {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return CFX_ByteString();
  return m_Objects.GetAt(i)->GetString();
}
CFX_ByteStringC CPDF_Array::GetConstString(FX_DWORD i) const {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return CFX_ByteStringC();
  return m_Objects.GetAt(i)->GetConstString();
}
int CPDF_Array::GetInteger(FX_DWORD i) const {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return 0;
  return m_Objects.GetAt(i)->GetInteger();
}
FX_FLOAT CPDF_Array::GetNumber(FX_DWORD i) const {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return 0;
  return m_Objects.GetAt(i)->GetNumber();
}
CPDF_Dictionary* CPDF_Array::GetDict(FX_DWORD i) const {
  CPDF_Object* p = GetElementValue(i);
  if (!p)
    return NULL;
  if (CPDF_Dictionary* pDict = p->AsDictionary())
    return pDict;
  if (CPDF_Stream* pStream = p->AsStream())
    return pStream->GetDict();
  return NULL;
}
CPDF_Stream* CPDF_Array::GetStream(FX_DWORD i) const {
  return ToStream(GetElementValue(i));
}
CPDF_Array* CPDF_Array::GetArray(FX_DWORD i) const {
  return ToArray(GetElementValue(i));
}
void CPDF_Array::RemoveAt(FX_DWORD i, int nCount) {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return;

  if (nCount <= 0 || nCount > m_Objects.GetSize() - i)
    return;

  for (int j = 0; j < nCount; ++j) {
    if (CPDF_Object* p = m_Objects.GetAt(i + j))
      p->Release();
  }
  m_Objects.RemoveAt(i, nCount);
}
void CPDF_Array::SetAt(FX_DWORD i,
                       CPDF_Object* pObj,
                       CPDF_IndirectObjectHolder* pObjs) {
  ASSERT(IsArray());
  ASSERT(i < (FX_DWORD)m_Objects.GetSize());
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return;
  if (CPDF_Object* pOld = m_Objects.GetAt(i))
    pOld->Release();
  if (pObj->GetObjNum()) {
    ASSERT(pObjs);
    pObj = new CPDF_Reference(pObjs, pObj->GetObjNum());
  }
  m_Objects.SetAt(i, pObj);
}
void CPDF_Array::InsertAt(FX_DWORD index,
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
  CPDF_Number* pNumber = new CPDF_Number;
  pNumber->SetNumber(f);
  Add(pNumber);
}
void CPDF_Array::AddReference(CPDF_IndirectObjectHolder* pDoc,
                              FX_DWORD objnum) {
  ASSERT(IsArray());
  Add(new CPDF_Reference(pDoc, objnum));
}
FX_BOOL CPDF_Array::Identical(CPDF_Array* pOther) const {
  if (m_Objects.GetSize() != pOther->m_Objects.GetSize()) {
    return FALSE;
  }
  for (int i = 0; i < m_Objects.GetSize(); i++) {
    if (!m_Objects[i]->IsIdentical(pOther->m_Objects[i]))
      return FALSE;
  }
  return TRUE;
}
CPDF_Dictionary::~CPDF_Dictionary() {
  for (const auto& it : m_Map) {
    it.second->Release();
  }
}
CPDF_Object* CPDF_Dictionary::GetElement(const CFX_ByteStringC& key) const {
  auto it = m_Map.find(key);
  if (it == m_Map.end())
    return nullptr;
  return it->second;
}
CPDF_Object* CPDF_Dictionary::GetElementValue(
    const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElement(key);
  return p ? p->GetDirect() : nullptr;
}
CFX_ByteString CPDF_Dictionary::GetString(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetString();
  }
  return CFX_ByteString();
}
CFX_ByteStringC CPDF_Dictionary::GetConstString(
    const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetConstString();
  }
  return CFX_ByteStringC();
}
CFX_WideString CPDF_Dictionary::GetUnicodeText(const CFX_ByteStringC& key,
                                               CFX_CharMap* pCharMap) const {
  CPDF_Object* p = GetElement(key);
  if (CPDF_Reference* pRef = ToReference(p))
    p = pRef->GetDirect();
  return p ? p->GetUnicodeText(pCharMap) : CFX_WideString();
}
CFX_ByteString CPDF_Dictionary::GetString(const CFX_ByteStringC& key,
                                          const CFX_ByteStringC& def) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetString();
  }
  return CFX_ByteString(def);
}
CFX_ByteStringC CPDF_Dictionary::GetConstString(
    const CFX_ByteStringC& key,
    const CFX_ByteStringC& def) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetConstString();
  }
  return CFX_ByteStringC(def);
}
int CPDF_Dictionary::GetInteger(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetInteger();
  }
  return 0;
}
int CPDF_Dictionary::GetInteger(const CFX_ByteStringC& key, int def) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetInteger();
  }
  return def;
}
FX_FLOAT CPDF_Dictionary::GetNumber(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetNumber();
  }
  return 0;
}
FX_BOOL CPDF_Dictionary::GetBoolean(const CFX_ByteStringC& key,
                                    FX_BOOL bDefault) const {
  CPDF_Object* p = GetElement(key);
  if (ToBoolean(p))
    return p->GetInteger();
  return bDefault;
}
CPDF_Dictionary* CPDF_Dictionary::GetDict(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElementValue(key);
  if (!p)
    return nullptr;
  if (CPDF_Dictionary* pDict = p->AsDictionary())
    return pDict;
  if (CPDF_Stream* pStream = p->AsStream())
    return pStream->GetDict();
  return nullptr;
}
CPDF_Array* CPDF_Dictionary::GetArray(const CFX_ByteStringC& key) const {
  return ToArray(GetElementValue(key));
}
CPDF_Stream* CPDF_Dictionary::GetStream(const CFX_ByteStringC& key) const {
  return ToStream(GetElementValue(key));
}
CFX_FloatRect CPDF_Dictionary::GetRect(const CFX_ByteStringC& key) const {
  CFX_FloatRect rect;
  CPDF_Array* pArray = GetArray(key);
  if (pArray)
    rect = pArray->GetRect();
  return rect;
}
CFX_Matrix CPDF_Dictionary::GetMatrix(const CFX_ByteStringC& key) const {
  CFX_Matrix matrix;
  CPDF_Array* pArray = GetArray(key);
  if (pArray)
    matrix = pArray->GetMatrix();
  return matrix;
}
FX_BOOL CPDF_Dictionary::KeyExist(const CFX_ByteStringC& key) const {
  return pdfium::ContainsKey(m_Map, key);
}

void CPDF_Dictionary::SetAt(const CFX_ByteStringC& key, CPDF_Object* pObj) {
  ASSERT(IsDictionary());
  // Avoid 2 constructions of CFX_ByteString.
  CFX_ByteString key_bytestring = key;
  auto it = m_Map.find(key_bytestring);
  if (it == m_Map.end()) {
    if (pObj) {
      m_Map.insert(std::make_pair(key_bytestring, pObj));
    }
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
  ASSERT(m_Type == PDFOBJ_DICTIONARY);
  auto it = m_Map.find(key);
  if (it == m_Map.end())
    return;

  it->second->Release();
  m_Map.erase(it);
}
void CPDF_Dictionary::ReplaceKey(const CFX_ByteStringC& oldkey,
                                 const CFX_ByteStringC& newkey) {
  ASSERT(m_Type == PDFOBJ_DICTIONARY);
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
FX_BOOL CPDF_Dictionary::Identical(CPDF_Dictionary* pOther) const {
  if (!pOther) {
    return FALSE;
  }
  if (m_Map.size() != pOther->m_Map.size()) {
    return FALSE;
  }
  for (const auto& it : m_Map) {
    const CFX_ByteString& key = it.first;
    if (!it.second->IsIdentical(pOther->GetElement(key)))
      return FALSE;
  }
  return TRUE;
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
                                     FX_DWORD objnum) {
  SetAt(key, new CPDF_Reference(pDoc, objnum));
}
void CPDF_Dictionary::AddReference(const CFX_ByteStringC& key,
                                   CPDF_IndirectObjectHolder* pDoc,
                                   FX_DWORD objnum) {
  SetAt(key, new CPDF_Reference(pDoc, objnum));
}
void CPDF_Dictionary::SetAtNumber(const CFX_ByteStringC& key, FX_FLOAT f) {
  CPDF_Number* pNumber = new CPDF_Number;
  pNumber->SetNumber(f);
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
  pArray->AddNumber16(matrix.a);
  pArray->AddNumber16(matrix.b);
  pArray->AddNumber16(matrix.c);
  pArray->AddNumber16(matrix.d);
  pArray->AddNumber(matrix.e);
  pArray->AddNumber(matrix.f);
  SetAt(key, pArray);
}
CPDF_Stream::CPDF_Stream(uint8_t* pData, FX_DWORD size, CPDF_Dictionary* pDict)
    : CPDF_Object(PDFOBJ_STREAM),
      m_pDict(pDict),
      m_dwSize(size),
      m_GenNum(kMemoryBasedGenNum),
      m_pDataBuf(pData) {}

CPDF_Stream::~CPDF_Stream() {
  if (IsMemoryBased())
    FX_Free(m_pDataBuf);

  if (m_pDict)
    m_pDict->Release();
}

void CPDF_Stream::InitStreamInternal(CPDF_Dictionary* pDict) {
  if (pDict) {
    if (m_pDict)
      m_pDict->Release();
    m_pDict = pDict;
  }
  if (IsMemoryBased())
    FX_Free(m_pDataBuf);

  m_GenNum = 0;
  m_pFile = nullptr;
}

void CPDF_Stream::InitStream(uint8_t* pData,
                             FX_DWORD size,
                             CPDF_Dictionary* pDict) {
  InitStreamInternal(pDict);
  m_GenNum = kMemoryBasedGenNum;
  m_pDataBuf = FX_Alloc(uint8_t, size);
  if (pData) {
    FXSYS_memcpy(m_pDataBuf, pData, size);
  }
  m_dwSize = size;
  if (m_pDict) {
    m_pDict->SetAtInteger("Length", size);
  }
}
void CPDF_Stream::SetData(const uint8_t* pData,
                          FX_DWORD size,
                          FX_BOOL bCompressed,
                          FX_BOOL bKeepBuf) {
  if (IsMemoryBased())
    FX_Free(m_pDataBuf);
  m_GenNum = kMemoryBasedGenNum;

  if (bKeepBuf) {
    m_pDataBuf = (uint8_t*)pData;
  } else {
    m_pDataBuf = FX_Alloc(uint8_t, size);
    if (pData) {
      FXSYS_memcpy(m_pDataBuf, pData, size);
    }
  }
  m_dwSize = size;
  if (!m_pDict)
    m_pDict = new CPDF_Dictionary;
  m_pDict->SetAtInteger("Length", size);
  if (!bCompressed) {
    m_pDict->RemoveAt("Filter");
    m_pDict->RemoveAt("DecodeParms");
  }
}
FX_BOOL CPDF_Stream::ReadRawData(FX_FILESIZE offset,
                                 uint8_t* buf,
                                 FX_DWORD size) const {
  if (!IsMemoryBased() && m_pFile)
    return m_pFile->ReadBlock(buf, offset, size);

  if (m_pDataBuf)
    FXSYS_memcpy(buf, m_pDataBuf + offset, size);
  return TRUE;
}
void CPDF_Stream::InitStreamFromFile(IFX_FileRead* pFile,
                                     CPDF_Dictionary* pDict) {
  InitStreamInternal(pDict);
  m_pFile = pFile;
  m_dwSize = (FX_DWORD)pFile->GetSize();
  if (m_pDict) {
    m_pDict->SetAtInteger("Length", m_dwSize);
  }
}

FX_BOOL CPDF_Stream::Identical(CPDF_Stream* pOther) const {
  if (!m_pDict)
    return !pOther->m_pDict;

  if (!m_pDict->Identical(pOther->m_pDict))
    return FALSE;

  if (m_dwSize != pOther->m_dwSize)
    return FALSE;

  if (!IsMemoryBased() && !pOther->IsMemoryBased()) {
    if (m_pFile == pOther->m_pFile && !m_pFile)
      return TRUE;

    if (!m_pFile || !pOther->m_pFile)
      return FALSE;

    uint8_t srcBuf[kBlockSize];
    uint8_t destBuf[kBlockSize];
    FX_DWORD size = m_dwSize;
    if (m_pFile == pOther->m_pFile)
      return TRUE;

    FX_DWORD offset = 0;
    while (size > 0) {
      FX_DWORD actualSize = std::min(size, kBlockSize);
      m_pFile->ReadBlock(srcBuf, offset, actualSize);
      pOther->m_pFile->ReadBlock(destBuf, offset, actualSize);
      if (FXSYS_memcmp(srcBuf, destBuf, actualSize) != 0)
        return FALSE;

      size -= actualSize;
      offset += actualSize;
    }
    return TRUE;
  }

  if (!IsMemoryBased() || !pOther->IsMemoryBased()) {
    IFX_FileRead* pFile = nullptr;
    uint8_t* pBuf = nullptr;
    if (!pOther->IsMemoryBased()) {
      pFile = pOther->m_pFile;
      pBuf = m_pDataBuf;
    } else if (!IsMemoryBased()) {
      pFile = m_pFile;
      pBuf = pOther->m_pDataBuf;
    }
    if (!pBuf)
      return FALSE;

    uint8_t srcBuf[kBlockSize];
    FX_DWORD size = m_dwSize;
    FX_DWORD offset = 0;
    while (size > 0) {
      FX_DWORD actualSize = std::min(size, kBlockSize);
      pFile->ReadBlock(srcBuf, offset, actualSize);
      if (FXSYS_memcmp(srcBuf, pBuf, actualSize) != 0)
        return FALSE;

      pBuf += actualSize;
      size -= actualSize;
      offset += actualSize;
    }
    return TRUE;
  }
  return FXSYS_memcmp(m_pDataBuf, pOther->m_pDataBuf, m_dwSize) == 0;
}

CPDF_StreamAcc::CPDF_StreamAcc() {
  m_bNewBuf = FALSE;
  m_pData = NULL;
  m_dwSize = 0;
  m_pImageParam = NULL;
  m_pStream = NULL;
  m_pSrcData = NULL;
}
void CPDF_StreamAcc::LoadAllData(const CPDF_Stream* pStream,
                                 FX_BOOL bRawAccess,
                                 FX_DWORD estimated_size,
                                 FX_BOOL bImageAcc) {
  if (!pStream)
    return;

  m_pStream = pStream;
  if (pStream->IsMemoryBased() &&
      (!pStream->GetDict()->KeyExist("Filter") || bRawAccess)) {
    m_dwSize = pStream->m_dwSize;
    m_pData = (uint8_t*)pStream->m_pDataBuf;
    return;
  }
  uint8_t* pSrcData;
  FX_DWORD dwSrcSize = pStream->m_dwSize;
  if (dwSrcSize == 0)
    return;

  if (!pStream->IsMemoryBased()) {
    pSrcData = m_pSrcData = FX_Alloc(uint8_t, dwSrcSize);
    if (!pStream->ReadRawData(0, pSrcData, dwSrcSize))
      return;
  } else {
    pSrcData = pStream->m_pDataBuf;
  }
  uint8_t* pDecryptedData = pSrcData;
  FX_DWORD dwDecryptedSize = dwSrcSize;
  if (!pStream->GetDict()->KeyExist("Filter") || bRawAccess) {
    m_pData = pDecryptedData;
    m_dwSize = dwDecryptedSize;
  } else {
    FX_BOOL bRet = PDF_DataDecode(
        pDecryptedData, dwDecryptedSize, m_pStream->GetDict(), m_pData,
        m_dwSize, m_ImageDecoder, m_pImageParam, estimated_size, bImageAcc);
    if (!bRet) {
      m_pData = pDecryptedData;
      m_dwSize = dwDecryptedSize;
    }
  }
  if (pSrcData != pStream->m_pDataBuf && pSrcData != m_pData) {
    FX_Free(pSrcData);
  }
  if (pDecryptedData != pSrcData && pDecryptedData != m_pData) {
    FX_Free(pDecryptedData);
  }
  m_pSrcData = NULL;
  m_bNewBuf = m_pData != pStream->m_pDataBuf;
}
CPDF_StreamAcc::~CPDF_StreamAcc() {
  if (m_bNewBuf) {
    FX_Free(m_pData);
  }
  FX_Free(m_pSrcData);
}
const uint8_t* CPDF_StreamAcc::GetData() const {
  if (m_bNewBuf) {
    return m_pData;
  }
  if (!m_pStream) {
    return NULL;
  }
  return m_pStream->m_pDataBuf;
}
FX_DWORD CPDF_StreamAcc::GetSize() const {
  if (m_bNewBuf) {
    return m_dwSize;
  }
  if (!m_pStream) {
    return 0;
  }
  return m_pStream->m_dwSize;
}
uint8_t* CPDF_StreamAcc::DetachData() {
  if (m_bNewBuf) {
    uint8_t* p = m_pData;
    m_pData = NULL;
    m_dwSize = 0;
    return p;
  }
  uint8_t* p = FX_Alloc(uint8_t, m_dwSize);
  FXSYS_memcpy(p, m_pData, m_dwSize);
  return p;
}
void CPDF_Reference::SetRef(CPDF_IndirectObjectHolder* pDoc, FX_DWORD objnum) {
  m_pObjList = pDoc;
  m_RefObjNum = objnum;
}
CPDF_IndirectObjectHolder::CPDF_IndirectObjectHolder(CPDF_Parser* pParser)
    : m_pParser(pParser), m_LastObjNum(0) {
  if (pParser)
    m_LastObjNum = m_pParser->GetLastObjNum();
}
CPDF_IndirectObjectHolder::~CPDF_IndirectObjectHolder() {
  for (const auto& pair : m_IndirectObjs) {
    pair.second->Destroy();
  }
}
CPDF_Object* CPDF_IndirectObjectHolder::GetIndirectObject(
    FX_DWORD objnum,
    PARSE_CONTEXT* pContext) {
  if (objnum == 0)
    return nullptr;

  auto it = m_IndirectObjs.find(objnum);
  if (it != m_IndirectObjs.end())
    return it->second->GetObjNum() != -1 ? it->second : nullptr;

  if (!m_pParser)
    return nullptr;

  CPDF_Object* pObj = m_pParser->ParseIndirectObject(this, objnum, pContext);
  if (!pObj)
    return nullptr;

  pObj->m_ObjNum = objnum;
  m_LastObjNum = std::max(m_LastObjNum, objnum);
  if (m_IndirectObjs[objnum])
    m_IndirectObjs[objnum]->Destroy();

  m_IndirectObjs[objnum] = pObj;
  return pObj;
}
int CPDF_IndirectObjectHolder::GetIndirectType(FX_DWORD objnum) {
  auto it = m_IndirectObjs.find(objnum);
  if (it != m_IndirectObjs.end())
    return it->second->GetType();

  if (!m_pParser)
    return 0;

  PARSE_CONTEXT context;
  FXSYS_memset(&context, 0, sizeof(PARSE_CONTEXT));
  context.m_Flags = PDFPARSE_TYPEONLY;
  return (int)(uintptr_t)m_pParser->ParseIndirectObject(this, objnum, &context);
}
FX_DWORD CPDF_IndirectObjectHolder::AddIndirectObject(CPDF_Object* pObj) {
  if (pObj->m_ObjNum) {
    return pObj->m_ObjNum;
  }
  m_LastObjNum++;
  m_IndirectObjs[m_LastObjNum] = pObj;
  pObj->m_ObjNum = m_LastObjNum;
  return m_LastObjNum;
}
void CPDF_IndirectObjectHolder::ReleaseIndirectObject(FX_DWORD objnum) {
  auto it = m_IndirectObjs.find(objnum);
  if (it == m_IndirectObjs.end() || it->second->GetObjNum() == -1)
    return;
  it->second->Destroy();
  m_IndirectObjs.erase(it);
}
FX_BOOL CPDF_IndirectObjectHolder::InsertIndirectObject(FX_DWORD objnum,
                                                        CPDF_Object* pObj) {
  if (!objnum || !pObj)
    return FALSE;
  auto it = m_IndirectObjs.find(objnum);
  if (it != m_IndirectObjs.end()) {
    if (pObj->GetGenNum() <= it->second->GetGenNum()) {
      pObj->Destroy();
      return FALSE;
    }
    it->second->Destroy();
  }
  pObj->m_ObjNum = objnum;
  m_IndirectObjs[objnum] = pObj;
  m_LastObjNum = std::max(m_LastObjNum, objnum);
  return TRUE;
}
