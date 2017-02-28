// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSDECLARATION_H_
#define XFA_FDE_CSS_CFDE_CSSDECLARATION_H_

#include <memory>
#include <utility>
#include <vector>

#include "xfa/fde/css/fde_cssdatatable.h"

class CFDE_CSSPropertyHolder;
class CFDE_CSSCustomProperty;

class CFDE_CSSDeclaration {
 public:
  using const_prop_iterator =
      std::vector<std::unique_ptr<CFDE_CSSPropertyHolder>>::const_iterator;
  using const_custom_iterator =
      std::vector<std::unique_ptr<CFDE_CSSCustomProperty>>::const_iterator;

  static bool ParseCSSString(const FX_WCHAR* pszValue,
                             int32_t iValueLen,
                             int32_t* iOffset,
                             int32_t* iLength);
  static bool ParseCSSColor(const FX_WCHAR* pszValue,
                            int32_t iValueLen,
                            FX_ARGB* dwColor);

  CFDE_CSSDeclaration();
  ~CFDE_CSSDeclaration();

  CFX_RetainPtr<CFDE_CSSValue> GetProperty(FDE_CSSProperty eProperty,
                                           bool* bImportant) const;

  const_prop_iterator begin() const { return properties_.begin(); }
  const_prop_iterator end() const { return properties_.end(); }

  const_custom_iterator custom_begin() const {
    return custom_properties_.begin();
  }
  const_custom_iterator custom_end() const { return custom_properties_.end(); }

  bool empty() const { return properties_.empty(); }

  void AddProperty(const FDE_CSSPropertyTable* pTable,
                   const CFX_WideStringC& value);
  void AddProperty(const CFX_WideString& prop, const CFX_WideString& value);

  size_t PropertyCountForTesting() const;

  FX_ARGB ParseColorForTest(const FX_WCHAR* pszValue,
                            int32_t iValueLen,
                            FX_ARGB* dwColor) const;

 private:
  void ParseFontProperty(const FX_WCHAR* pszValue,
                         int32_t iValueLen,
                         bool bImportant);
  bool ParseBorderProperty(const FX_WCHAR* pszValue,
                           int32_t iValueLen,
                           CFX_RetainPtr<CFDE_CSSValue>& pWidth) const;
  void ParseValueListProperty(const FDE_CSSPropertyTable* pTable,
                              const FX_WCHAR* pszValue,
                              int32_t iValueLen,
                              bool bImportant);
  void Add4ValuesProperty(const std::vector<CFX_RetainPtr<CFDE_CSSValue>>& list,
                          bool bImportant,
                          FDE_CSSProperty eLeft,
                          FDE_CSSProperty eTop,
                          FDE_CSSProperty eRight,
                          FDE_CSSProperty eBottom);
  CFX_RetainPtr<CFDE_CSSValue> ParseNumber(const FX_WCHAR* pszValue,
                                           int32_t iValueLen);
  CFX_RetainPtr<CFDE_CSSValue> ParseEnum(const FX_WCHAR* pszValue,
                                         int32_t iValueLen);
  CFX_RetainPtr<CFDE_CSSValue> ParseColor(const FX_WCHAR* pszValue,
                                          int32_t iValueLen);
  CFX_RetainPtr<CFDE_CSSValue> ParseString(const FX_WCHAR* pszValue,
                                           int32_t iValueLen);
  void AddPropertyHolder(FDE_CSSProperty eProperty,
                         CFX_RetainPtr<CFDE_CSSValue> pValue,
                         bool bImportant);

  std::vector<std::unique_ptr<CFDE_CSSPropertyHolder>> properties_;
  std::vector<std::unique_ptr<CFDE_CSSCustomProperty>> custom_properties_;
};

#endif  // XFA_FDE_CSS_CFDE_CSSDECLARATION_H_
