// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DICTIONARY_H_
#define CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DICTIONARY_H_

#include <map>

#include "core/fpdfapi/fpdf_parser/include/cpdf_object.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_string.h"

class CPDF_IndirectObjectHolder;

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
  CPDF_Object* GetObjectBy(const CFX_ByteStringC& key) const;
  CPDF_Object* GetDirectObjectBy(const CFX_ByteStringC& key) const;
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
  bool IsSignatureDict() const;

  // Set* functions invalidate iterators for the element with the key |key|.
  void SetAt(const CFX_ByteStringC& key, CPDF_Object* pObj);
  void SetAtName(const CFX_ByteStringC& key, const CFX_ByteString& name);
  void SetAtString(const CFX_ByteStringC& key, const CFX_ByteString& str);
  void SetAtInteger(const CFX_ByteStringC& key, int i);
  void SetAtNumber(const CFX_ByteStringC& key, FX_FLOAT f);
  void SetAtReference(const CFX_ByteStringC& key,
                      CPDF_IndirectObjectHolder* pDoc,
                      uint32_t objnum);
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
                    uint32_t objnum);

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

#endif  // CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DICTIONARY_H_
