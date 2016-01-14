// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_OBJECTS_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_OBJECTS_H_

#include <map>
#include <set>

#include "core/include/fxcrt/fx_coordinates.h"
#include "core/include/fxcrt/fx_system.h"

class CPDF_Array;
class CPDF_Boolean;
class CPDF_CryptoHandler;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_IndirectObjectHolder;
class CPDF_Name;
class CPDF_Null;
class CPDF_Number;
class CPDF_Parser;
class CPDF_Reference;
class CPDF_Stream;
class CPDF_StreamAcc;
class CPDF_StreamFilter;
class CPDF_String;
class IFX_FileRead;
struct PARSE_CONTEXT;

#define PDFOBJ_INVALID 0
#define PDFOBJ_BOOLEAN 1
#define PDFOBJ_NUMBER 2
#define PDFOBJ_STRING 3
#define PDFOBJ_NAME 4
#define PDFOBJ_ARRAY 5
#define PDFOBJ_DICTIONARY 6
#define PDFOBJ_STREAM 7
#define PDFOBJ_NULL 8
#define PDFOBJ_REFERENCE 9

class CPDF_Object {
 public:
  int GetType() const { return m_Type; }

  FX_DWORD GetObjNum() const { return m_ObjNum; }

  FX_DWORD GetGenNum() const { return m_GenNum; }

  FX_BOOL IsIdentical(CPDF_Object* pObj) const;

  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const;

  CPDF_Object* CloneRef(CPDF_IndirectObjectHolder* pObjs) const;

  CPDF_Object* GetDirect() const;

  void Release();

  CFX_ByteString GetString() const;

  CFX_ByteStringC GetConstString() const;

  CFX_WideString GetUnicodeText(CFX_CharMap* pCharMap = NULL) const;
  FX_FLOAT GetNumber() const;

  FX_FLOAT GetNumber16() const;

  int GetInteger() const;

  CPDF_Dictionary* GetDict() const;

  CPDF_Array* GetArray() const;

  void SetString(const CFX_ByteString& str);

  void SetUnicodeText(const FX_WCHAR* pUnicodes, int len = -1);

  int GetDirectType() const;

  FX_BOOL IsModified() const { return FALSE; }

  bool IsArray() const { return m_Type == PDFOBJ_ARRAY; }
  bool IsBoolean() const { return m_Type == PDFOBJ_BOOLEAN; }
  bool IsDictionary() const { return m_Type == PDFOBJ_DICTIONARY; }
  bool IsName() const { return m_Type == PDFOBJ_NAME; }
  bool IsNumber() const { return m_Type == PDFOBJ_NUMBER; }
  bool IsReference() const { return m_Type == PDFOBJ_REFERENCE; }
  bool IsStream() const { return m_Type == PDFOBJ_STREAM; }
  bool IsString() const { return m_Type == PDFOBJ_STRING; }

  CPDF_Array* AsArray();
  const CPDF_Array* AsArray() const;

  CPDF_Boolean* AsBoolean();
  const CPDF_Boolean* AsBoolean() const;

  CPDF_Dictionary* AsDictionary();
  const CPDF_Dictionary* AsDictionary() const;

  CPDF_Name* AsName();
  const CPDF_Name* AsName() const;

  CPDF_Number* AsNumber();
  const CPDF_Number* AsNumber() const;

  CPDF_Reference* AsReference();
  const CPDF_Reference* AsReference() const;

  CPDF_Stream* AsStream();
  const CPDF_Stream* AsStream() const;

  CPDF_String* AsString();
  const CPDF_String* AsString() const;

 protected:
  explicit CPDF_Object(FX_DWORD type)
      : m_Type(type), m_ObjNum(0), m_GenNum(0) {}
  ~CPDF_Object() {}
  void Destroy();

  static const int kObjectRefMaxDepth = 128;
  static int s_nCurRefDepth;
  FX_DWORD m_Type;
  FX_DWORD m_ObjNum;
  FX_DWORD m_GenNum;

  friend class CPDF_IndirectObjectHolder;
  friend class CPDF_Parser;
  friend class CPDF_SyntaxParser;

 private:
  CPDF_Object(const CPDF_Object& src) {}
  CPDF_Object* CloneInternal(FX_BOOL bDirect,
                             std::set<FX_DWORD>* visited) const;
};
class CPDF_Boolean : public CPDF_Object {
 public:
  CPDF_Boolean() : CPDF_Object(PDFOBJ_BOOLEAN), m_bValue(false) {}
  explicit CPDF_Boolean(FX_BOOL value)
      : CPDF_Object(PDFOBJ_BOOLEAN), m_bValue(value) {}

  FX_BOOL Identical(CPDF_Boolean* pOther) const {
    return m_bValue == pOther->m_bValue;
  }

 protected:
  FX_BOOL m_bValue;
  friend class CPDF_Object;
};
inline CPDF_Boolean* ToBoolean(CPDF_Object* obj) {
  return obj ? obj->AsBoolean() : nullptr;
}
inline const CPDF_Boolean* ToBoolean(const CPDF_Object* obj) {
  return obj ? obj->AsBoolean() : nullptr;
}

class CPDF_Number : public CPDF_Object {
 public:
  CPDF_Number() : CPDF_Object(PDFOBJ_NUMBER), m_bInteger(TRUE), m_Integer(0) {}

  explicit CPDF_Number(int value);

  explicit CPDF_Number(FX_FLOAT value);

  explicit CPDF_Number(const CFX_ByteStringC& str);

  FX_BOOL Identical(CPDF_Number* pOther) const;

  CFX_ByteString GetString() const;

  void SetString(const CFX_ByteStringC& str);

  FX_BOOL IsInteger() const { return m_bInteger; }

  int GetInteger() const { return m_bInteger ? m_Integer : (int)m_Float; }

  FX_FLOAT GetNumber() const {
    return m_bInteger ? (FX_FLOAT)m_Integer : m_Float;
  }

  void SetNumber(FX_FLOAT value);

  FX_FLOAT GetNumber16() const { return GetNumber(); }

  FX_FLOAT GetFloat() const {
    return m_bInteger ? (FX_FLOAT)m_Integer : m_Float;
  }

 protected:
  FX_BOOL m_bInteger;

  union {
    int m_Integer;

    FX_FLOAT m_Float;
  };
  friend class CPDF_Object;
};
inline CPDF_Number* ToNumber(CPDF_Object* obj) {
  return obj ? obj->AsNumber() : nullptr;
}
inline const CPDF_Number* ToNumber(const CPDF_Object* obj) {
  return obj ? obj->AsNumber() : nullptr;
}

class CPDF_String : public CPDF_Object {
 public:
  CPDF_String() : CPDF_Object(PDFOBJ_STRING), m_bHex(FALSE) {}

  CPDF_String(const CFX_ByteString& str, FX_BOOL bHex)
      : CPDF_Object(PDFOBJ_STRING), m_String(str), m_bHex(bHex) {}

  explicit CPDF_String(const CFX_WideString& str);

  CFX_ByteString& GetString() { return m_String; }

  FX_BOOL Identical(CPDF_String* pOther) const {
    return m_String == pOther->m_String;
  }

  FX_BOOL IsHex() const { return m_bHex; }

 protected:
  CFX_ByteString m_String;

  FX_BOOL m_bHex;
  friend class CPDF_Object;
};
inline CPDF_String* ToString(CPDF_Object* obj) {
  return obj ? obj->AsString() : nullptr;
}
inline const CPDF_String* ToString(const CPDF_Object* obj) {
  return obj ? obj->AsString() : nullptr;
}

class CPDF_Name : public CPDF_Object {
 public:
  explicit CPDF_Name(const CFX_ByteString& str)
      : CPDF_Object(PDFOBJ_NAME), m_Name(str) {}
  explicit CPDF_Name(const CFX_ByteStringC& str)
      : CPDF_Object(PDFOBJ_NAME), m_Name(str) {}
  explicit CPDF_Name(const FX_CHAR* str)
      : CPDF_Object(PDFOBJ_NAME), m_Name(str) {}

  CFX_ByteString& GetString() { return m_Name; }

  FX_BOOL Identical(CPDF_Name* pOther) const {
    return m_Name == pOther->m_Name;
  }

 protected:
  CFX_ByteString m_Name;
  friend class CPDF_Object;
};
inline CPDF_Name* ToName(CPDF_Object* obj) {
  return obj ? obj->AsName() : nullptr;
}
inline const CPDF_Name* ToName(const CPDF_Object* obj) {
  return obj ? obj->AsName() : nullptr;
}

class CPDF_Array : public CPDF_Object {
 public:
  CPDF_Array() : CPDF_Object(PDFOBJ_ARRAY) {}

  FX_DWORD GetCount() const { return m_Objects.GetSize(); }

  CPDF_Object* GetElement(FX_DWORD index) const;

  CPDF_Object* GetElementValue(FX_DWORD index) const;

  CFX_Matrix GetMatrix();

  CFX_FloatRect GetRect();

  CFX_ByteString GetString(FX_DWORD index) const;

  CFX_ByteStringC GetConstString(FX_DWORD index) const;

  int GetInteger(FX_DWORD index) const;

  FX_FLOAT GetNumber(FX_DWORD index) const;

  CPDF_Dictionary* GetDict(FX_DWORD index) const;

  CPDF_Stream* GetStream(FX_DWORD index) const;

  CPDF_Array* GetArray(FX_DWORD index) const;

  FX_FLOAT GetFloat(FX_DWORD index) const { return GetNumber(index); }

  void SetAt(FX_DWORD index,
             CPDF_Object* pObj,
             CPDF_IndirectObjectHolder* pObjs = NULL);

  void InsertAt(FX_DWORD index,
                CPDF_Object* pObj,
                CPDF_IndirectObjectHolder* pObjs = NULL);

  void RemoveAt(FX_DWORD index, int nCount = 1);

  void Add(CPDF_Object* pObj, CPDF_IndirectObjectHolder* pObjs = NULL);

  void AddNumber(FX_FLOAT f);

  void AddInteger(int i);

  void AddString(const CFX_ByteString& str);

  void AddName(const CFX_ByteString& str);

  void AddReference(CPDF_IndirectObjectHolder* pDoc, FX_DWORD objnum);

  void AddReference(CPDF_IndirectObjectHolder* pDoc, CPDF_Object* obj) {
    AddReference(pDoc, obj->GetObjNum());
  }

  FX_FLOAT GetNumber16(FX_DWORD index) const { return GetNumber(index); }

  void AddNumber16(FX_FLOAT value) { AddNumber(value); }

  FX_BOOL Identical(CPDF_Array* pOther) const;

 protected:
  ~CPDF_Array();

  CFX_ArrayTemplate<CPDF_Object*> m_Objects;
  friend class CPDF_Object;
};
inline CPDF_Array* ToArray(CPDF_Object* obj) {
  return obj ? obj->AsArray() : nullptr;
}
inline const CPDF_Array* ToArray(const CPDF_Object* obj) {
  return obj ? obj->AsArray() : nullptr;
}

class CPDF_Dictionary : public CPDF_Object {
 public:
  using iterator = std::map<CFX_ByteString, CPDF_Object*>::iterator;
  using const_iterator = std::map<CFX_ByteString, CPDF_Object*>::const_iterator;

  CPDF_Dictionary() : CPDF_Object(PDFOBJ_DICTIONARY) {}

  CPDF_Object* GetElement(const CFX_ByteStringC& key) const;

  CPDF_Object* GetElementValue(const CFX_ByteStringC& key) const;

  CFX_ByteString GetString(const CFX_ByteStringC& key) const;

  CFX_ByteStringC GetConstString(const CFX_ByteStringC& key) const;

  CFX_ByteString GetString(const CFX_ByteStringC& key,
                           const CFX_ByteStringC& default_str) const;

  CFX_ByteStringC GetConstString(const CFX_ByteStringC& key,
                                 const CFX_ByteStringC& default_str) const;

  CFX_WideString GetUnicodeText(const CFX_ByteStringC& key,
                                CFX_CharMap* pCharMap = NULL) const;

  int GetInteger(const CFX_ByteStringC& key) const;

  int GetInteger(const CFX_ByteStringC& key, int default_int) const;

  FX_BOOL GetBoolean(const CFX_ByteStringC& key,
                     FX_BOOL bDefault = FALSE) const;

  FX_FLOAT GetNumber(const CFX_ByteStringC& key) const;

  CPDF_Dictionary* GetDict(const CFX_ByteStringC& key) const;

  CPDF_Stream* GetStream(const CFX_ByteStringC& key) const;

  CPDF_Array* GetArray(const CFX_ByteStringC& key) const;

  CFX_FloatRect GetRect(const CFX_ByteStringC& key) const;

  CFX_Matrix GetMatrix(const CFX_ByteStringC& key) const;

  FX_FLOAT GetFloat(const CFX_ByteStringC& key) const { return GetNumber(key); }

  FX_BOOL KeyExist(const CFX_ByteStringC& key) const;

  // Set* functions invalidate iterators for the element with the key |key|.
  void SetAt(const CFX_ByteStringC& key, CPDF_Object* pObj);

  void SetAtName(const CFX_ByteStringC& key, const CFX_ByteString& name);

  void SetAtString(const CFX_ByteStringC& key, const CFX_ByteString& string);

  void SetAtInteger(const CFX_ByteStringC& key, int i);

  void SetAtNumber(const CFX_ByteStringC& key, FX_FLOAT f);

  void SetAtReference(const CFX_ByteStringC& key,
                      CPDF_IndirectObjectHolder* pDoc,
                      FX_DWORD objnum);

  void SetAtReference(const CFX_ByteStringC& key,
                      CPDF_IndirectObjectHolder* pDoc,
                      CPDF_Object* obj) {
    SetAtReference(key, pDoc, obj->GetObjNum());
  }

  void AddReference(const CFX_ByteStringC& key,
                    CPDF_IndirectObjectHolder* pDoc,
                    FX_DWORD objnum);

  void SetAtRect(const CFX_ByteStringC& key, const CFX_FloatRect& rect);

  void SetAtMatrix(const CFX_ByteStringC& key, const CFX_Matrix& matrix);

  void SetAtBoolean(const CFX_ByteStringC& key, FX_BOOL bValue);

  // Invalidates iterators for the element with the key |key|.
  void RemoveAt(const CFX_ByteStringC& key);

  // Invalidates iterators for the element with the key |oldkey|.
  void ReplaceKey(const CFX_ByteStringC& oldkey, const CFX_ByteStringC& newkey);

  FX_BOOL Identical(CPDF_Dictionary* pDict) const;

  size_t GetCount() const { return m_Map.size(); }

  iterator begin() { return m_Map.begin(); }

  iterator end() { return m_Map.end(); }

  const_iterator begin() const { return m_Map.begin(); }

  const_iterator end() const { return m_Map.end(); }

 protected:
  ~CPDF_Dictionary();

  std::map<CFX_ByteString, CPDF_Object*> m_Map;

  friend class CPDF_Object;
};
inline CPDF_Dictionary* ToDictionary(CPDF_Object* obj) {
  return obj ? obj->AsDictionary() : nullptr;
}
inline const CPDF_Dictionary* ToDictionary(const CPDF_Object* obj) {
  return obj ? obj->AsDictionary() : nullptr;
}

class CPDF_Stream : public CPDF_Object {
 public:
  CPDF_Stream(uint8_t* pData, FX_DWORD size, CPDF_Dictionary* pDict);

  CPDF_Dictionary* GetDict() const { return m_pDict; }

  void SetData(const uint8_t* pData,
               FX_DWORD size,
               FX_BOOL bCompressed,
               FX_BOOL bKeepBuf);

  void InitStream(uint8_t* pData, FX_DWORD size, CPDF_Dictionary* pDict);

  void InitStreamFromFile(IFX_FileRead* pFile, CPDF_Dictionary* pDict);

  FX_BOOL Identical(CPDF_Stream* pOther) const;

  FX_DWORD GetRawSize() const { return m_dwSize; }

  FX_BOOL ReadRawData(FX_FILESIZE start_pos,
                      uint8_t* pBuf,
                      FX_DWORD buf_size) const;

  FX_BOOL IsMemoryBased() const { return m_GenNum == kMemoryBasedGenNum; }

 protected:
  friend class CPDF_Object;
  friend class CPDF_StreamAcc;

  static const FX_DWORD kMemoryBasedGenNum = (FX_DWORD)-1;
  ~CPDF_Stream();

  void InitStreamInternal(CPDF_Dictionary* pDict);

  CPDF_Dictionary* m_pDict;

  FX_DWORD m_dwSize;

  FX_DWORD m_GenNum;

  union {
    uint8_t* m_pDataBuf;

    IFX_FileRead* m_pFile;
  };
};
inline CPDF_Stream* ToStream(CPDF_Object* obj) {
  return obj ? obj->AsStream() : nullptr;
}
inline const CPDF_Stream* ToStream(const CPDF_Object* obj) {
  return obj ? obj->AsStream() : nullptr;
}

class CPDF_StreamAcc {
 public:
  CPDF_StreamAcc();

  ~CPDF_StreamAcc();

  void LoadAllData(const CPDF_Stream* pStream,
                   FX_BOOL bRawAccess = FALSE,
                   FX_DWORD estimated_size = 0,
                   FX_BOOL bImageAcc = FALSE);

  const CPDF_Stream* GetStream() const { return m_pStream; }

  CPDF_Dictionary* GetDict() const {
    return m_pStream ? m_pStream->GetDict() : nullptr;
  }

  const uint8_t* GetData() const;

  FX_DWORD GetSize() const;

  uint8_t* DetachData();

  const CFX_ByteString& GetImageDecoder() { return m_ImageDecoder; }

  const CPDF_Dictionary* GetImageParam() { return m_pImageParam; }

 protected:
  uint8_t* m_pData;

  FX_DWORD m_dwSize;

  FX_BOOL m_bNewBuf;

  CFX_ByteString m_ImageDecoder;

  CPDF_Dictionary* m_pImageParam;

  const CPDF_Stream* m_pStream;

  uint8_t* m_pSrcData;
};

class CPDF_Null : public CPDF_Object {
 public:
  CPDF_Null() : CPDF_Object(PDFOBJ_NULL) {}
};

class CPDF_Reference : public CPDF_Object {
 public:
  CPDF_Reference(CPDF_IndirectObjectHolder* pDoc, int objnum)
      : CPDF_Object(PDFOBJ_REFERENCE), m_pObjList(pDoc), m_RefObjNum(objnum) {}

  CPDF_IndirectObjectHolder* GetObjList() const { return m_pObjList; }

  FX_DWORD GetRefObjNum() const { return m_RefObjNum; }

  void SetRef(CPDF_IndirectObjectHolder* pDoc, FX_DWORD objnum);

  FX_BOOL Identical(CPDF_Reference* pOther) const {
    return m_RefObjNum == pOther->m_RefObjNum;
  }

 protected:
  CPDF_IndirectObjectHolder* m_pObjList;

  FX_DWORD m_RefObjNum;
  friend class CPDF_Object;
};
inline CPDF_Reference* ToReference(CPDF_Object* obj) {
  return obj ? obj->AsReference() : nullptr;
}
inline const CPDF_Reference* ToReference(const CPDF_Object* obj) {
  return obj ? obj->AsReference() : nullptr;
}

class CPDF_IndirectObjectHolder {
 public:
  using iterator = std::map<FX_DWORD, CPDF_Object*>::iterator;
  using const_iterator = std::map<FX_DWORD, CPDF_Object*>::const_iterator;

  explicit CPDF_IndirectObjectHolder(CPDF_Parser* pParser);
  ~CPDF_IndirectObjectHolder();

  int GetIndirectType(FX_DWORD objnum);
  CPDF_Object* GetIndirectObject(FX_DWORD objnum, PARSE_CONTEXT* pContext);
  FX_DWORD AddIndirectObject(CPDF_Object* pObj);
  void ReleaseIndirectObject(FX_DWORD objnum);

  // Takes ownership of |pObj|.
  FX_BOOL InsertIndirectObject(FX_DWORD objnum, CPDF_Object* pObj);

  FX_DWORD GetLastObjNum() const { return m_LastObjNum; }
  iterator begin() { return m_IndirectObjs.begin(); }
  const_iterator begin() const { return m_IndirectObjs.cbegin(); }
  iterator end() { return m_IndirectObjs.end(); }
  const_iterator end() const { return m_IndirectObjs.end(); }

 protected:
  CPDF_Parser* m_pParser;
  FX_DWORD m_LastObjNum;
  std::map<FX_DWORD, CPDF_Object*> m_IndirectObjs;
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_OBJECTS_H_
