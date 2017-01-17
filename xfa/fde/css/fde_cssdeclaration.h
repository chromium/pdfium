// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSDECLARATION_H_
#define XFA_FDE_CSS_FDE_CSSDECLARATION_H_

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "xfa/fde/css/fde_cssdatatable.h"

class FDE_CSSPropertyHolder {
 public:
  FDE_CSSPropertyHolder();
  ~FDE_CSSPropertyHolder();

  FDE_CSSProperty eProperty;
  bool bImportant;
  CFX_RetainPtr<CFDE_CSSValue> pValue;
};

class FDE_CSSCustomProperty {
 public:
  const FX_WCHAR* pwsName;
  const FX_WCHAR* pwsValue;
};

struct FDE_CSSPropertyArgs {
  std::unordered_map<uint32_t, FX_WCHAR*>* pStringCache;
  const FDE_CSSPropertyTable* pProperty;
};

class CFDE_CSSDeclaration {
 public:
  using const_prop_iterator =
      std::vector<std::unique_ptr<FDE_CSSPropertyHolder>>::const_iterator;
  using const_custom_iterator =
      std::vector<std::unique_ptr<FDE_CSSCustomProperty>>::const_iterator;

  CFDE_CSSDeclaration();
  ~CFDE_CSSDeclaration();

  CFDE_CSSValue* GetProperty(FDE_CSSProperty eProperty, bool& bImportant) const;

  const_prop_iterator begin() const { return properties_.begin(); }
  const_prop_iterator end() const { return properties_.end(); }

  const_custom_iterator custom_begin() const {
    return custom_properties_.begin();
  }
  const_custom_iterator custom_end() const { return custom_properties_.end(); }

  bool empty() const { return properties_.empty(); }

  void AddProperty(const FDE_CSSPropertyArgs* pArgs,
                   const FX_WCHAR* pszValue,
                   int32_t iValueLen);
  void AddProperty(const FDE_CSSPropertyArgs* pArgs,
                   const FX_WCHAR* pszName,
                   int32_t iNameLen,
                   const FX_WCHAR* pszValue,
                   int32_t iValueLen);

  size_t PropertyCountForTesting() const;

 protected:
  void ParseFontProperty(const FDE_CSSPropertyArgs* pArgs,
                         const FX_WCHAR* pszValue,
                         int32_t iValueLen,
                         bool bImportant);
  bool ParseBorderProperty(const FX_WCHAR* pszValue,
                           int32_t iValueLen,
                           CFX_RetainPtr<CFDE_CSSValue>& pWidth) const;
  void ParseValueListProperty(const FDE_CSSPropertyArgs* pArgs,
                              const FX_WCHAR* pszValue,
                              int32_t iValueLen,
                              bool bImportant);
  void Add4ValuesProperty(const std::vector<CFX_RetainPtr<CFDE_CSSValue>>& list,
                          bool bImportant,
                          FDE_CSSProperty eLeft,
                          FDE_CSSProperty eTop,
                          FDE_CSSProperty eRight,
                          FDE_CSSProperty eBottom);
  CFX_RetainPtr<CFDE_CSSValue> ParseNumber(const FDE_CSSPropertyArgs* pArgs,
                                           const FX_WCHAR* pszValue,
                                           int32_t iValueLen);
  CFX_RetainPtr<CFDE_CSSValue> ParseEnum(const FDE_CSSPropertyArgs* pArgs,
                                         const FX_WCHAR* pszValue,
                                         int32_t iValueLen);
  CFX_RetainPtr<CFDE_CSSValue> ParseColor(const FDE_CSSPropertyArgs* pArgs,
                                          const FX_WCHAR* pszValue,
                                          int32_t iValueLen);
  CFX_RetainPtr<CFDE_CSSValue> ParseURI(const FDE_CSSPropertyArgs* pArgs,
                                        const FX_WCHAR* pszValue,
                                        int32_t iValueLen);
  CFX_RetainPtr<CFDE_CSSValue> ParseString(const FDE_CSSPropertyArgs* pArgs,
                                           const FX_WCHAR* pszValue,
                                           int32_t iValueLen);
  CFX_RetainPtr<CFDE_CSSValue> ParseFunction(const FDE_CSSPropertyArgs* pArgs,
                                             const FX_WCHAR* pszValue,
                                             int32_t iValueLen);
  const FX_WCHAR* CopyToLocal(const FDE_CSSPropertyArgs* pArgs,
                              const FX_WCHAR* pszValue,
                              int32_t iValueLen);
  void AddPropertyHolder(FDE_CSSProperty eProperty,
                         CFX_RetainPtr<CFDE_CSSValue> pValue,
                         bool bImportant);
  CFX_RetainPtr<CFDE_CSSPrimitiveValue> NewNumberValue(
      FDE_CSSPrimitiveType eUnit,
      FX_FLOAT fValue) const;
  CFX_RetainPtr<CFDE_CSSPrimitiveValue> NewEnumValue(
      FDE_CSSPropertyValue eValue) const;

  std::vector<std::unique_ptr<FDE_CSSPropertyHolder>> properties_;
  std::vector<std::unique_ptr<FDE_CSSCustomProperty>> custom_properties_;
};

#endif  // XFA_FDE_CSS_FDE_CSSDECLARATION_H_
