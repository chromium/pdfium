// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DICTIONARY_H_
#define CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DICTIONARY_H_

#include <map>
#include <set>

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
  CPDF_Object* Clone() const override;
  CPDF_Dictionary* GetDict() const override;
  bool IsDictionary() const override;
  CPDF_Dictionary* AsDictionary() override;
  const CPDF_Dictionary* AsDictionary() const override;

  size_t GetCount() const { return m_Map.size(); }
  CPDF_Object* GetObjectFor(const CFX_ByteString& key) const;
  CPDF_Object* GetDirectObjectFor(const CFX_ByteString& key) const;
  CFX_ByteString GetStringFor(const CFX_ByteString& key) const;
  CFX_ByteString GetStringFor(const CFX_ByteString& key,
                              const CFX_ByteString& default_str) const;
  CFX_WideString GetUnicodeTextFor(const CFX_ByteString& key) const;
  int GetIntegerFor(const CFX_ByteString& key) const;
  int GetIntegerFor(const CFX_ByteString& key, int default_int) const;
  bool GetBooleanFor(const CFX_ByteString& key, bool bDefault = false) const;
  FX_FLOAT GetNumberFor(const CFX_ByteString& key) const;
  CPDF_Dictionary* GetDictFor(const CFX_ByteString& key) const;
  CPDF_Stream* GetStreamFor(const CFX_ByteString& key) const;
  CPDF_Array* GetArrayFor(const CFX_ByteString& key) const;
  CFX_FloatRect GetRectFor(const CFX_ByteString& key) const;
  CFX_Matrix GetMatrixFor(const CFX_ByteString& key) const;
  FX_FLOAT GetFloatFor(const CFX_ByteString& key) const {
    return GetNumberFor(key);
  }

  FX_BOOL KeyExist(const CFX_ByteString& key) const;
  bool IsSignatureDict() const;

  // Set* functions invalidate iterators for the element with the key |key|.
  void SetFor(const CFX_ByteString& key, CPDF_Object* pObj);
  void SetNameFor(const CFX_ByteString& key, const CFX_ByteString& name);
  void SetStringFor(const CFX_ByteString& key, const CFX_ByteString& str);
  void SetIntegerFor(const CFX_ByteString& key, int i);
  void SetNumberFor(const CFX_ByteString& key, FX_FLOAT f);
  void SetReferenceFor(const CFX_ByteString& key,
                       CPDF_IndirectObjectHolder* pDoc,
                       uint32_t objnum);
  void SetReferenceFor(const CFX_ByteString& key,
                       CPDF_IndirectObjectHolder* pDoc,
                       CPDF_Object* obj) {
    SetReferenceFor(key, pDoc, obj->GetObjNum());
  }
  void SetRectFor(const CFX_ByteString& key, const CFX_FloatRect& rect);
  void SetMatrixFor(const CFX_ByteString& key, const CFX_Matrix& matrix);
  void SetBooleanFor(const CFX_ByteString& key, bool bValue);

  // Invalidates iterators for the element with the key |key|.
  void RemoveFor(const CFX_ByteString& key);

  // Invalidates iterators for the element with the key |oldkey|.
  void ReplaceKey(const CFX_ByteString& oldkey, const CFX_ByteString& newkey);

  iterator begin() { return m_Map.begin(); }
  iterator end() { return m_Map.end(); }
  const_iterator begin() const { return m_Map.begin(); }
  const_iterator end() const { return m_Map.end(); }

 protected:
  ~CPDF_Dictionary() override;

  CPDF_Object* CloneNonCyclic(
      bool bDirect,
      std::set<const CPDF_Object*>* visited) const override;

  std::map<CFX_ByteString, CPDF_Object*> m_Map;
};

#endif  // CORE_FPDFAPI_FPDF_PARSER_INCLUDE_CPDF_DICTIONARY_H_
