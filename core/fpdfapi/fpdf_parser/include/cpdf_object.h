// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_OBJECT_H_
#define CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_OBJECT_H_

#include <memory>
#include <set>

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_Array;
class CPDF_Boolean;
class CPDF_Dictionary;
class CPDF_Name;
class CPDF_Null;
class CPDF_Number;
class CPDF_Reference;
class CPDF_Stream;
class CPDF_String;

class CPDF_Object {
 public:
  static const uint32_t kInvalidObjNum = static_cast<uint32_t>(-1);
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
  uint32_t GetObjNum() const { return m_ObjNum; }
  uint32_t GetGenNum() const { return m_GenNum; }

  // Create a deep copy of the object.
  virtual CPDF_Object* Clone() const = 0;
  // Create a deep copy of the object except any reference object be
  // copied to the object it points to directly.
  virtual CPDF_Object* CloneDirectObject() const;
  virtual CPDF_Object* GetDirect() const;

  void Release();

  virtual CFX_ByteString GetString() const;
  virtual CFX_WideString GetUnicodeText() const;
  virtual FX_FLOAT GetNumber() const;
  virtual int GetInteger() const;
  virtual CPDF_Dictionary* GetDict() const;

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
  friend class CPDF_Array;
  friend class CPDF_Dictionary;
  friend class CPDF_Document;
  friend class CPDF_IndirectObjectHolder;
  friend class CPDF_Parser;
  friend class CPDF_Reference;
  friend class CPDF_Stream;
  friend struct std::default_delete<CPDF_Object>;

  CPDF_Object() : m_ObjNum(0), m_GenNum(0) {}
  virtual ~CPDF_Object();

  CPDF_Object* CloneObjectNonCyclic(bool bDirect) const;

  // Create a deep copy of the object with the option to either
  // copy a reference object or directly copy the object it refers to
  // when |bDirect| is true.
  // Also check cyclic reference against |pVisited|, no copy if it is found.
  // Complex objects should implement their own CloneNonCyclic()
  // function to properly check for possible loop.
  virtual CPDF_Object* CloneNonCyclic(
      bool bDirect,
      std::set<const CPDF_Object*>* pVisited) const;

  uint32_t m_ObjNum;
  uint32_t m_GenNum;

 private:
  CPDF_Object(const CPDF_Object& src) {}
};

inline CPDF_Boolean* ToBoolean(CPDF_Object* obj) {
  return obj ? obj->AsBoolean() : nullptr;
}

inline const CPDF_Boolean* ToBoolean(const CPDF_Object* obj) {
  return obj ? obj->AsBoolean() : nullptr;
}

inline CPDF_Number* ToNumber(CPDF_Object* obj) {
  return obj ? obj->AsNumber() : nullptr;
}

inline const CPDF_Number* ToNumber(const CPDF_Object* obj) {
  return obj ? obj->AsNumber() : nullptr;
}

inline CPDF_String* ToString(CPDF_Object* obj) {
  return obj ? obj->AsString() : nullptr;
}

inline const CPDF_String* ToString(const CPDF_Object* obj) {
  return obj ? obj->AsString() : nullptr;
}

inline CPDF_Name* ToName(CPDF_Object* obj) {
  return obj ? obj->AsName() : nullptr;
}

inline const CPDF_Name* ToName(const CPDF_Object* obj) {
  return obj ? obj->AsName() : nullptr;
}

inline CPDF_Array* ToArray(CPDF_Object* obj) {
  return obj ? obj->AsArray() : nullptr;
}

inline const CPDF_Array* ToArray(const CPDF_Object* obj) {
  return obj ? obj->AsArray() : nullptr;
}

inline CPDF_Dictionary* ToDictionary(CPDF_Object* obj) {
  return obj ? obj->AsDictionary() : nullptr;
}

inline const CPDF_Dictionary* ToDictionary(const CPDF_Object* obj) {
  return obj ? obj->AsDictionary() : nullptr;
}
inline CPDF_Reference* ToReference(CPDF_Object* obj) {
  return obj ? obj->AsReference() : nullptr;
}

inline const CPDF_Reference* ToReference(const CPDF_Object* obj) {
  return obj ? obj->AsReference() : nullptr;
}

inline CPDF_Stream* ToStream(CPDF_Object* obj) {
  return obj ? obj->AsStream() : nullptr;
}

inline const CPDF_Stream* ToStream(const CPDF_Object* obj) {
  return obj ? obj->AsStream() : nullptr;
}

#endif  // CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_OBJECT_H_
