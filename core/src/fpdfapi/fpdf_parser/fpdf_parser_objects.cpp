// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfapi/fpdf_objects.h"

#include <algorithm>

#include "core/include/fpdfapi/fpdf_parser.h"
#include "core/include/fxcrt/fx_string.h"
#include "third_party/base/stl_util.h"

void CPDF_Object::Release() {
  if (m_ObjNum) {
    return;
  }
  Destroy();
}

CPDF_Number::CPDF_Number(const CFX_ByteStringC& str) {
  FX_atonum(str, m_bInteger, &m_Integer);
}

void CPDF_Number::SetString(const CFX_ByteString& str) {
  FX_atonum(str, m_bInteger, &m_Integer);
}

CFX_ByteString CPDF_Number::GetString() const {
  return m_bInteger ? CFX_ByteString::FormatInteger(m_Integer, FXFORMAT_SIGNED)
                    : CFX_ByteString::FormatFloat(m_Float);
}

CPDF_String::CPDF_String(const CFX_WideString& str) : m_bHex(FALSE) {
  m_String = PDF_EncodeText(str);
}

CFX_WideString CPDF_String::GetUnicodeText() const {
  return PDF_DecodeText(m_String);
}

CFX_WideString CPDF_Name::GetUnicodeText() const {
  return PDF_DecodeText(m_Name);
}

CPDF_Array::~CPDF_Array() {
  int size = m_Objects.GetSize();
  CPDF_Object** pList = m_Objects.GetData();
  for (int i = 0; i < size; i++) {
    if (pList[i])
      pList[i]->Release();
  }
}

CPDF_Object* CPDF_Array::Clone(FX_BOOL bDirect) const {
  CPDF_Array* pCopy = new CPDF_Array();
  for (int i = 0; i < GetCount(); i++) {
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

CFX_ByteString CPDF_Array::GetStringAt(FX_DWORD i) const {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return CFX_ByteString();
  return m_Objects.GetAt(i)->GetString();
}

CFX_ByteStringC CPDF_Array::GetConstStringAt(FX_DWORD i) const {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return CFX_ByteStringC();
  return m_Objects.GetAt(i)->GetConstString();
}

int CPDF_Array::GetIntegerAt(FX_DWORD i) const {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return 0;
  return m_Objects.GetAt(i)->GetInteger();
}

FX_FLOAT CPDF_Array::GetNumberAt(FX_DWORD i) const {
  if (i >= (FX_DWORD)m_Objects.GetSize())
    return 0;
  return m_Objects.GetAt(i)->GetNumber();
}

CPDF_Dictionary* CPDF_Array::GetDictAt(FX_DWORD i) const {
  CPDF_Object* p = GetElementValue(i);
  if (!p)
    return NULL;
  if (CPDF_Dictionary* pDict = p->AsDictionary())
    return pDict;
  if (CPDF_Stream* pStream = p->AsStream())
    return pStream->GetDict();
  return NULL;
}

CPDF_Stream* CPDF_Array::GetStreamAt(FX_DWORD i) const {
  return ToStream(GetElementValue(i));
}

CPDF_Array* CPDF_Array::GetArrayAt(FX_DWORD i) const {
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
  CPDF_Number* pNumber = new CPDF_Number(f);
  Add(pNumber);
}

void CPDF_Array::AddReference(CPDF_IndirectObjectHolder* pDoc,
                              FX_DWORD objnum) {
  ASSERT(IsArray());
  Add(new CPDF_Reference(pDoc, objnum));
}

CPDF_Dictionary::~CPDF_Dictionary() {
  for (const auto& it : m_Map) {
    it.second->Release();
  }
}

CPDF_Object* CPDF_Dictionary::Clone(FX_BOOL bDirect) const {
  CPDF_Dictionary* pCopy = new CPDF_Dictionary();
  for (const auto& it : *this)
    pCopy->m_Map.insert(std::make_pair(it.first, it.second->Clone(bDirect)));
  return pCopy;
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

CFX_ByteString CPDF_Dictionary::GetStringBy(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetString();
  }
  return CFX_ByteString();
}

CFX_ByteStringC CPDF_Dictionary::GetConstStringBy(
    const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetConstString();
  }
  return CFX_ByteStringC();
}

CFX_WideString CPDF_Dictionary::GetUnicodeTextBy(
    const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElement(key);
  if (CPDF_Reference* pRef = ToReference(p))
    p = pRef->GetDirect();
  return p ? p->GetUnicodeText() : CFX_WideString();
}

CFX_ByteString CPDF_Dictionary::GetStringBy(const CFX_ByteStringC& key,
                                            const CFX_ByteStringC& def) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetString();
  }
  return CFX_ByteString(def);
}

CFX_ByteStringC CPDF_Dictionary::GetConstStringBy(
    const CFX_ByteStringC& key,
    const CFX_ByteStringC& def) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetConstString();
  }
  return CFX_ByteStringC(def);
}

int CPDF_Dictionary::GetIntegerBy(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetInteger();
  }
  return 0;
}

int CPDF_Dictionary::GetIntegerBy(const CFX_ByteStringC& key, int def) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetInteger();
  }
  return def;
}

FX_FLOAT CPDF_Dictionary::GetNumberBy(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElement(key);
  if (p) {
    return p->GetNumber();
  }
  return 0;
}

FX_BOOL CPDF_Dictionary::GetBooleanBy(const CFX_ByteStringC& key,
                                      FX_BOOL bDefault) const {
  CPDF_Object* p = GetElement(key);
  if (ToBoolean(p))
    return p->GetInteger();
  return bDefault;
}

CPDF_Dictionary* CPDF_Dictionary::GetDictBy(const CFX_ByteStringC& key) const {
  CPDF_Object* p = GetElementValue(key);
  if (!p)
    return nullptr;
  if (CPDF_Dictionary* pDict = p->AsDictionary())
    return pDict;
  if (CPDF_Stream* pStream = p->AsStream())
    return pStream->GetDict();
  return nullptr;
}

CPDF_Array* CPDF_Dictionary::GetArrayBy(const CFX_ByteStringC& key) const {
  return ToArray(GetElementValue(key));
}

CPDF_Stream* CPDF_Dictionary::GetStreamBy(const CFX_ByteStringC& key) const {
  return ToStream(GetElementValue(key));
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
                                     FX_DWORD objnum) {
  SetAt(key, new CPDF_Reference(pDoc, objnum));
}

void CPDF_Dictionary::AddReference(const CFX_ByteStringC& key,
                                   CPDF_IndirectObjectHolder* pDoc,
                                   FX_DWORD objnum) {
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

CPDF_Stream::CPDF_Stream(uint8_t* pData, FX_DWORD size, CPDF_Dictionary* pDict)
    : m_pDict(pDict),
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

CPDF_Object* CPDF_Stream::Clone(FX_BOOL bDirect) const {
  CPDF_StreamAcc acc;
  acc.LoadAllData(this, TRUE);
  FX_DWORD streamSize = acc.GetSize();
  CPDF_Dictionary* pDict = GetDict();
  if (pDict) {
    pDict = ToDictionary(pDict->Clone(bDirect));
  }
  return new CPDF_Stream(acc.DetachData(), streamSize, pDict);
}

void CPDF_Stream::SetData(const uint8_t* pData,
                          FX_DWORD size,
                          FX_BOOL bCompressed,
                          FX_BOOL bKeepBuf) {
  if (IsMemoryBased())
    FX_Free(m_pDataBuf);
  m_GenNum = kMemoryBasedGenNum;

  if (bKeepBuf) {
    m_pDataBuf = const_cast<uint8_t*>(pData);
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

CFX_WideString CPDF_Stream::GetUnicodeText() const {
  CPDF_StreamAcc stream;
  stream.LoadAllData(this, FALSE);
  return PDF_DecodeText(stream.GetData(), stream.GetSize());
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
    m_dwSize = pStream->GetRawSize();
    m_pData = pStream->GetRawData();
    return;
  }
  uint8_t* pSrcData;
  FX_DWORD dwSrcSize = pStream->GetRawSize();
  if (dwSrcSize == 0)
    return;

  if (!pStream->IsMemoryBased()) {
    pSrcData = m_pSrcData = FX_Alloc(uint8_t, dwSrcSize);
    if (!pStream->ReadRawData(0, pSrcData, dwSrcSize))
      return;
  } else {
    pSrcData = pStream->GetRawData();
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
  if (pSrcData != pStream->GetRawData() && pSrcData != m_pData) {
    FX_Free(pSrcData);
  }
  if (pDecryptedData != pSrcData && pDecryptedData != m_pData) {
    FX_Free(pDecryptedData);
  }
  m_pSrcData = NULL;
  m_bNewBuf = m_pData != pStream->GetRawData();
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
  return m_pStream->GetRawData();
}

FX_DWORD CPDF_StreamAcc::GetSize() const {
  if (m_bNewBuf) {
    return m_dwSize;
  }
  if (!m_pStream) {
    return 0;
  }
  return m_pStream->GetRawSize();
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

CPDF_Object* CPDF_Reference::Clone(FX_BOOL bDirect) const {
  if (bDirect) {
    auto* pDirect = GetDirect();
    return pDirect ? pDirect->Clone(TRUE) : nullptr;
  }
  return new CPDF_Reference(m_pObjList, m_RefObjNum);
}

void CPDF_Reference::SetRef(CPDF_IndirectObjectHolder* pDoc, FX_DWORD objnum) {
  m_pObjList = pDoc;
  m_RefObjNum = objnum;
}

CPDF_Object* CPDF_Reference::GetDirect() const {
  return m_pObjList ? m_pObjList->GetIndirectObject(m_RefObjNum) : nullptr;
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

CPDF_Object* CPDF_IndirectObjectHolder::GetIndirectObject(FX_DWORD objnum) {
  if (objnum == 0)
    return nullptr;

  auto it = m_IndirectObjs.find(objnum);
  if (it != m_IndirectObjs.end())
    return it->second->GetObjNum() != -1 ? it->second : nullptr;

  if (!m_pParser)
    return nullptr;

  CPDF_Object* pObj = m_pParser->ParseIndirectObject(this, objnum);
  if (!pObj)
    return nullptr;

  pObj->m_ObjNum = objnum;
  m_LastObjNum = std::max(m_LastObjNum, objnum);
  if (m_IndirectObjs[objnum])
    m_IndirectObjs[objnum]->Destroy();

  m_IndirectObjs[objnum] = pObj;
  return pObj;
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
