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
class CPDF_Dictionary;
class CPDF_IndirectObjectHolder;
class CPDF_Name;
class CPDF_Null;
class CPDF_Number;
class CPDF_Parser;
class CPDF_Reference;
class CPDF_Stream;
class CPDF_StreamAcc;
class CPDF_String;
class IFX_FileRead;

class CPDF_Object {
 public:
  enum Type {
    BOOLEAN = 1,
    NUMBER,
    STRING,
    NAME,
    ARRAY,
    DICTIONARY,
    STREAM,
    NULLOBJ,
    REFERENCE
  };

  virtual Type GetType() const = 0;
  FX_DWORD GetObjNum() const { return m_ObjNum; }
  FX_DWORD GetGenNum() const { return m_GenNum; }

  virtual CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const = 0;
  virtual CPDF_Object* GetDirect() const;

  FX_BOOL IsModified() const { return FALSE; }
  void Release();

  virtual CFX_ByteString GetString() const;
  virtual CFX_ByteStringC GetConstString() const;
  virtual CFX_WideString GetUnicodeText() const;
  virtual FX_FLOAT GetNumber() const;
  virtual int GetInteger() const;
  virtual CPDF_Dictionary* GetDict() const;
  virtual CPDF_Array* GetArray() const;

  virtual void SetString(const CFX_ByteString& str);

  virtual bool IsArray() const;
  virtual bool IsBoolean() const;
  virtual bool IsDictionary() const;
  virtual bool IsName() const;
  virtual bool IsNumber() const;
  virtual bool IsReference() const;
  virtual bool IsStream() const;
  virtual bool IsString() const;

  virtual CPDF_Array* AsArray();
  virtual const CPDF_Array* AsArray() const;
  virtual CPDF_Boolean* AsBoolean();
  virtual const CPDF_Boolean* AsBoolean() const;
  virtual CPDF_Dictionary* AsDictionary();
  virtual const CPDF_Dictionary* AsDictionary() const;
  virtual CPDF_Name* AsName();
  virtual const CPDF_Name* AsName() const;
  virtual CPDF_Number* AsNumber();
  virtual const CPDF_Number* AsNumber() const;
  virtual CPDF_Reference* AsReference();
  virtual const CPDF_Reference* AsReference() const;
  virtual CPDF_Stream* AsStream();
  virtual const CPDF_Stream* AsStream() const;
  virtual CPDF_String* AsString();
  virtual const CPDF_String* AsString() const;

 protected:
  CPDF_Object() : m_ObjNum(0), m_GenNum(0) {}
  virtual ~CPDF_Object();
  void Destroy() { delete this; }

  FX_DWORD m_ObjNum;
  FX_DWORD m_GenNum;

  friend class CPDF_IndirectObjectHolder;
  friend class CPDF_Parser;

 private:
  CPDF_Object(const CPDF_Object& src) {}
};

class CPDF_Boolean : public CPDF_Object {
 public:
  CPDF_Boolean() : m_bValue(false) {}
  explicit CPDF_Boolean(FX_BOOL value) : m_bValue(value) {}

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
  CFX_ByteString GetString() const override;
  int GetInteger() const override;
  void SetString(const CFX_ByteString& str) override;
  bool IsBoolean() const override;
  CPDF_Boolean* AsBoolean() override;
  const CPDF_Boolean* AsBoolean() const override;

 protected:
  ~CPDF_Boolean() override;

  FX_BOOL m_bValue;
};

inline CPDF_Boolean* ToBoolean(CPDF_Object* obj) {
  return obj ? obj->AsBoolean() : nullptr;
}

inline const CPDF_Boolean* ToBoolean(const CPDF_Object* obj) {
  return obj ? obj->AsBoolean() : nullptr;
}

class CPDF_Number : public CPDF_Object {
 public:
  CPDF_Number() : m_bInteger(TRUE), m_Integer(0) {}
  explicit CPDF_Number(int value) : m_bInteger(TRUE), m_Integer(value) {}
  explicit CPDF_Number(FX_FLOAT value) : m_bInteger(FALSE), m_Float(value) {}
  explicit CPDF_Number(const CFX_ByteStringC& str);

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
  CFX_ByteString GetString() const override;
  FX_FLOAT GetNumber() const override;
  int GetInteger() const override;
  void SetString(const CFX_ByteString& str) override;
  bool IsNumber() const override;
  CPDF_Number* AsNumber() override;
  const CPDF_Number* AsNumber() const override;

  FX_BOOL IsInteger() { return m_bInteger; }

 protected:
  ~CPDF_Number() override;

  FX_BOOL m_bInteger;
  union {
    int m_Integer;
    FX_FLOAT m_Float;
  };
};

inline CPDF_Number* ToNumber(CPDF_Object* obj) {
  return obj ? obj->AsNumber() : nullptr;
}

inline const CPDF_Number* ToNumber(const CPDF_Object* obj) {
  return obj ? obj->AsNumber() : nullptr;
}

class CPDF_String : public CPDF_Object {
 public:
  CPDF_String() : m_bHex(FALSE) {}
  CPDF_String(const CFX_ByteString& str, FX_BOOL bHex)
      : m_String(str), m_bHex(bHex) {}
  explicit CPDF_String(const CFX_WideString& str);

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
  CFX_ByteString GetString() const override;
  CFX_ByteStringC GetConstString() const override;
  CFX_WideString GetUnicodeText() const override;
  void SetString(const CFX_ByteString& str) override;
  bool IsString() const override;
  CPDF_String* AsString() override;
  const CPDF_String* AsString() const override;

  FX_BOOL IsHex() const { return m_bHex; }

 protected:
  ~CPDF_String() override;

  CFX_ByteString m_String;
  FX_BOOL m_bHex;
};

inline CPDF_String* ToString(CPDF_Object* obj) {
  return obj ? obj->AsString() : nullptr;
}

inline const CPDF_String* ToString(const CPDF_Object* obj) {
  return obj ? obj->AsString() : nullptr;
}

class CPDF_Name : public CPDF_Object {
 public:
  explicit CPDF_Name(const CFX_ByteString& str) : m_Name(str) {}
  explicit CPDF_Name(const CFX_ByteStringC& str) : m_Name(str) {}
  explicit CPDF_Name(const FX_CHAR* str) : m_Name(str) {}

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
  CFX_ByteString GetString() const override;
  CFX_ByteStringC GetConstString() const override;
  CFX_WideString GetUnicodeText() const override;
  void SetString(const CFX_ByteString& str) override;
  bool IsName() const override;
  CPDF_Name* AsName() override;
  const CPDF_Name* AsName() const override;

 protected:
  ~CPDF_Name() override;

  CFX_ByteString m_Name;
};

inline CPDF_Name* ToName(CPDF_Object* obj) {
  return obj ? obj->AsName() : nullptr;
}

inline const CPDF_Name* ToName(const CPDF_Object* obj) {
  return obj ? obj->AsName() : nullptr;
}

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

  FX_DWORD GetCount() const { return m_Objects.GetSize(); }
  CPDF_Object* GetElement(FX_DWORD index) const;
  CPDF_Object* GetElementValue(FX_DWORD index) const;
  CFX_Matrix GetMatrix();
  CFX_FloatRect GetRect();
  CFX_ByteString GetStringAt(FX_DWORD index) const;
  CFX_ByteStringC GetConstStringAt(FX_DWORD index) const;
  int GetIntegerAt(FX_DWORD index) const;
  FX_FLOAT GetNumberAt(FX_DWORD index) const;
  CPDF_Dictionary* GetDictAt(FX_DWORD index) const;
  CPDF_Stream* GetStreamAt(FX_DWORD index) const;
  CPDF_Array* GetArrayAt(FX_DWORD index) const;
  FX_FLOAT GetFloatAt(FX_DWORD index) const { return GetNumberAt(index); }

  void SetAt(FX_DWORD index,
             CPDF_Object* pObj,
             CPDF_IndirectObjectHolder* pObjs = nullptr);
  void InsertAt(FX_DWORD index,
                CPDF_Object* pObj,
                CPDF_IndirectObjectHolder* pObjs = nullptr);
  void RemoveAt(FX_DWORD index, int nCount = 1);

  void Add(CPDF_Object* pObj, CPDF_IndirectObjectHolder* pObjs = nullptr);
  void AddNumber(FX_FLOAT f);
  void AddInteger(int i);
  void AddString(const CFX_ByteString& str);
  void AddName(const CFX_ByteString& str);
  void AddReference(CPDF_IndirectObjectHolder* pDoc, FX_DWORD objnum);
  void AddReference(CPDF_IndirectObjectHolder* pDoc, CPDF_Object* obj) {
    AddReference(pDoc, obj->GetObjNum());
  }

 protected:
  ~CPDF_Array() override;

  CFX_ArrayTemplate<CPDF_Object*> m_Objects;
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

  CPDF_Dictionary();

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
  CPDF_Dictionary* GetDict() const override;
  bool IsDictionary() const override;
  CPDF_Dictionary* AsDictionary() override;
  const CPDF_Dictionary* AsDictionary() const override;

  size_t GetCount() const { return m_Map.size(); }
  CPDF_Object* GetElement(const CFX_ByteStringC& key) const;
  CPDF_Object* GetElementValue(const CFX_ByteStringC& key) const;
  CFX_ByteString GetStringBy(const CFX_ByteStringC& key) const;
  CFX_ByteStringC GetConstStringBy(const CFX_ByteStringC& key) const;
  CFX_ByteString GetStringBy(const CFX_ByteStringC& key,
                             const CFX_ByteStringC& default_str) const;
  CFX_ByteStringC GetConstStringBy(const CFX_ByteStringC& key,
                                   const CFX_ByteStringC& default_str) const;
  CFX_WideString GetUnicodeTextBy(const CFX_ByteStringC& key) const;
  int GetIntegerBy(const CFX_ByteStringC& key) const;
  int GetIntegerBy(const CFX_ByteStringC& key, int default_int) const;
  FX_BOOL GetBooleanBy(const CFX_ByteStringC& key,
                       FX_BOOL bDefault = FALSE) const;
  FX_FLOAT GetNumberBy(const CFX_ByteStringC& key) const;
  CPDF_Dictionary* GetDictBy(const CFX_ByteStringC& key) const;
  CPDF_Stream* GetStreamBy(const CFX_ByteStringC& key) const;
  CPDF_Array* GetArrayBy(const CFX_ByteStringC& key) const;
  CFX_FloatRect GetRectBy(const CFX_ByteStringC& key) const;
  CFX_Matrix GetMatrixBy(const CFX_ByteStringC& key) const;
  FX_FLOAT GetFloatBy(const CFX_ByteStringC& key) const {
    return GetNumberBy(key);
  }

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
  void SetAtRect(const CFX_ByteStringC& key, const CFX_FloatRect& rect);
  void SetAtMatrix(const CFX_ByteStringC& key, const CFX_Matrix& matrix);
  void SetAtBoolean(const CFX_ByteStringC& key, FX_BOOL bValue);

  void AddReference(const CFX_ByteStringC& key,
                    CPDF_IndirectObjectHolder* pDoc,
                    FX_DWORD objnum);

  // Invalidates iterators for the element with the key |key|.
  void RemoveAt(const CFX_ByteStringC& key);

  // Invalidates iterators for the element with the key |oldkey|.
  void ReplaceKey(const CFX_ByteStringC& oldkey, const CFX_ByteStringC& newkey);

  iterator begin() { return m_Map.begin(); }
  iterator end() { return m_Map.end(); }
  const_iterator begin() const { return m_Map.begin(); }
  const_iterator end() const { return m_Map.end(); }

 protected:
  ~CPDF_Dictionary() override;

  std::map<CFX_ByteString, CPDF_Object*> m_Map;
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

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
  CPDF_Dictionary* GetDict() const override;
  CFX_WideString GetUnicodeText() const override;
  bool IsStream() const override;
  CPDF_Stream* AsStream() override;
  const CPDF_Stream* AsStream() const override;

  FX_DWORD GetRawSize() const { return m_dwSize; }
  uint8_t* GetRawData() const { return m_pDataBuf; }

  void SetData(const uint8_t* pData,
               FX_DWORD size,
               FX_BOOL bCompressed,
               FX_BOOL bKeepBuf);

  void InitStream(uint8_t* pData, FX_DWORD size, CPDF_Dictionary* pDict);
  void InitStreamFromFile(IFX_FileRead* pFile, CPDF_Dictionary* pDict);

  FX_BOOL ReadRawData(FX_FILESIZE start_pos,
                      uint8_t* pBuf,
                      FX_DWORD buf_size) const;

  bool IsMemoryBased() const { return m_GenNum == kMemoryBasedGenNum; }

 protected:
  static const FX_DWORD kMemoryBasedGenNum = (FX_DWORD)-1;

  ~CPDF_Stream() override;

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
  const CFX_ByteString& GetImageDecoder() const { return m_ImageDecoder; }
  const CPDF_Dictionary* GetImageParam() const { return m_pImageParam; }

  uint8_t* DetachData();

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
  CPDF_Null() {}

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
};

class CPDF_Reference : public CPDF_Object {
 public:
  CPDF_Reference(CPDF_IndirectObjectHolder* pDoc, int objnum)
      : m_pObjList(pDoc), m_RefObjNum(objnum) {}

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
  CPDF_Object* GetDirect() const override;
  CFX_ByteString GetString() const override;
  CFX_ByteStringC GetConstString() const override;
  FX_FLOAT GetNumber() const override;
  int GetInteger() const override;
  CPDF_Dictionary* GetDict() const override;

  // TODO(weili): check whether GetUnicodeText() and GetArray() are needed.
  bool IsReference() const override;
  CPDF_Reference* AsReference() override;
  const CPDF_Reference* AsReference() const override;

  CPDF_IndirectObjectHolder* GetObjList() const { return m_pObjList; }
  FX_DWORD GetRefObjNum() const { return m_RefObjNum; }

  void SetRef(CPDF_IndirectObjectHolder* pDoc, FX_DWORD objnum);

 protected:
  ~CPDF_Reference() override;
  CPDF_Object* SafeGetDirect() const {
    CPDF_Object* obj = GetDirect();
    if (!obj || obj->IsReference())
      return nullptr;
    return obj;
  }

  CPDF_IndirectObjectHolder* m_pObjList;
  FX_DWORD m_RefObjNum;
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

  CPDF_Object* GetIndirectObject(FX_DWORD objnum);
  FX_DWORD AddIndirectObject(CPDF_Object* pObj);
  void ReleaseIndirectObject(FX_DWORD objnum);

  // Takes ownership of |pObj|.
  FX_BOOL InsertIndirectObject(FX_DWORD objnum, CPDF_Object* pObj);

  FX_DWORD GetLastObjNum() const { return m_LastObjNum; }
  iterator begin() { return m_IndirectObjs.begin(); }
  const_iterator begin() const { return m_IndirectObjs.begin(); }
  iterator end() { return m_IndirectObjs.end(); }
  const_iterator end() const { return m_IndirectObjs.end(); }

 protected:
  CPDF_Parser* m_pParser;
  FX_DWORD m_LastObjNum;
  std::map<FX_DWORD, CPDF_Object*> m_IndirectObjs;
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_OBJECTS_H_
