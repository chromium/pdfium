// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSDECLARATION_H_
#define XFA_FDE_CSS_FDE_CSSDECLARATION_H_

#include <unordered_map>

#include "xfa/fde/css/fde_cssdatatable.h"

class FDE_CSSPropertyHolder {
 public:
  FDE_CSSProperty eProperty;
  bool bImportant;
  IFDE_CSSValue* pValue;
  FDE_CSSPropertyHolder* pNext;
};

class FDE_CSSCustomProperty {
 public:
  const FX_WCHAR* pwsName;
  const FX_WCHAR* pwsValue;
  FDE_CSSCustomProperty* pNext;
};

struct FDE_CSSPropertyArgs {
  std::unordered_map<uint32_t, FX_WCHAR*>* pStringCache;
  const FDE_CSSPropertyTable* pProperty;
};

class CFDE_CSSDeclaration {
 public:
  CFDE_CSSDeclaration()
      : m_pFirstProperty(nullptr),
        m_pLastProperty(nullptr),
        m_pFirstCustom(nullptr),
        m_pLastCustom(nullptr) {}

  IFDE_CSSValue* GetProperty(FDE_CSSProperty eProperty, bool& bImportant) const;
  FX_POSITION GetStartPosition() const;
  void GetNextProperty(FX_POSITION& pos,
                       FDE_CSSProperty& eProperty,
                       IFDE_CSSValue*& pValue,
                       bool& bImportant) const;
  FX_POSITION GetStartCustom() const;
  void GetNextCustom(FX_POSITION& pos,
                     CFX_WideString& wsName,
                     CFX_WideString& wsValue) const;
  bool AddProperty(const FDE_CSSPropertyArgs* pArgs,
                   const FX_WCHAR* pszValue,
                   int32_t iValueLen);
  bool AddProperty(const FDE_CSSPropertyArgs* pArgs,
                   const FX_WCHAR* pszName,
                   int32_t iNameLen,
                   const FX_WCHAR* pszValue,
                   int32_t iValueLen);

 protected:
  bool ParseFontProperty(const FDE_CSSPropertyArgs* pArgs,
                         const FX_WCHAR* pszValue,
                         int32_t iValueLen,
                         bool bImportant);
  bool ParseBorderProperty(const FX_WCHAR* pszValue,
                           int32_t iValueLen,
                           IFDE_CSSValue*& pWidth) const;
  bool ParseValueListProperty(const FDE_CSSPropertyArgs* pArgs,
                              const FX_WCHAR* pszValue,
                              int32_t iValueLen,
                              bool bImportant);
  bool Add4ValuesProperty(const CFDE_CSSValueArray& list,
                          bool bImportant,
                          FDE_CSSProperty eLeft,
                          FDE_CSSProperty eTop,
                          FDE_CSSProperty eRight,
                          FDE_CSSProperty eBottom);
  IFDE_CSSValue* ParseNumber(const FDE_CSSPropertyArgs* pArgs,
                             const FX_WCHAR* pszValue,
                             int32_t iValueLen);
  IFDE_CSSValue* ParseEnum(const FDE_CSSPropertyArgs* pArgs,
                           const FX_WCHAR* pszValue,
                           int32_t iValueLen);
  IFDE_CSSValue* ParseColor(const FDE_CSSPropertyArgs* pArgs,
                            const FX_WCHAR* pszValue,
                            int32_t iValueLen);
  IFDE_CSSValue* ParseURI(const FDE_CSSPropertyArgs* pArgs,
                          const FX_WCHAR* pszValue,
                          int32_t iValueLen);
  IFDE_CSSValue* ParseString(const FDE_CSSPropertyArgs* pArgs,
                             const FX_WCHAR* pszValue,
                             int32_t iValueLen);
  IFDE_CSSValue* ParseFunction(const FDE_CSSPropertyArgs* pArgs,
                               const FX_WCHAR* pszValue,
                               int32_t iValueLen);
  const FX_WCHAR* CopyToLocal(const FDE_CSSPropertyArgs* pArgs,
                              const FX_WCHAR* pszValue,
                              int32_t iValueLen);
  void AddPropertyHolder(FDE_CSSProperty eProperty,
                         IFDE_CSSValue* pValue,
                         bool bImportant);
  IFDE_CSSPrimitiveValue* NewNumberValue(FDE_CSSPrimitiveType eUnit,
                                         FX_FLOAT fValue) const;
  IFDE_CSSPrimitiveValue* NewEnumValue(FDE_CSSPropertyValue eValue) const;

  FDE_CSSPropertyHolder* m_pFirstProperty;
  FDE_CSSPropertyHolder* m_pLastProperty;
  FDE_CSSCustomProperty* m_pFirstCustom;
  FDE_CSSCustomProperty* m_pLastCustom;
};

#endif  // XFA_FDE_CSS_FDE_CSSDECLARATION_H_
