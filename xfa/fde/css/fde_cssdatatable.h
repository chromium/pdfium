// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSDATATABLE_H_
#define XFA_FDE_CSS_FDE_CSSDATATABLE_H_

#include "core/fxcrt/fx_system.h"
#include "xfa/fde/css/fde_css.h"

class CFDE_CSSFunction {
 public:
  CFDE_CSSFunction(const FX_WCHAR* pszFuncName, IFDE_CSSValueList* pArgList)
      : m_pArgList(pArgList), m_pszFuncName(pszFuncName) {
    ASSERT(pArgList);
  }
  int32_t CountArgs() const { return m_pArgList->CountValues(); }
  IFDE_CSSValue* GetArgs(int32_t index) const {
    return m_pArgList->GetValue(index);
  }
  const FX_WCHAR* GetFuncName() const { return m_pszFuncName; }

 protected:
  IFDE_CSSValueList* m_pArgList;
  const FX_WCHAR* m_pszFuncName;
};

class CFDE_CSSPrimitiveValue : public IFDE_CSSPrimitiveValue {
 public:
  explicit CFDE_CSSPrimitiveValue(FX_ARGB color);
  explicit CFDE_CSSPrimitiveValue(FDE_CSSPropertyValue eValue);
  explicit CFDE_CSSPrimitiveValue(CFDE_CSSFunction* pFunction);
  CFDE_CSSPrimitiveValue(FDE_CSSPrimitiveType eType, FX_FLOAT fValue);
  CFDE_CSSPrimitiveValue(FDE_CSSPrimitiveType eType, const FX_WCHAR* pValue);
  CFDE_CSSPrimitiveValue(const CFDE_CSSPrimitiveValue& src);

  // IFDE_CSSPrimitiveValue
  FDE_CSSPrimitiveType GetPrimitiveType() const override;
  FX_ARGB GetRGBColor() const override;
  FX_FLOAT GetFloat() const override;
  const FX_WCHAR* GetString(int32_t& iLength) const override;
  FDE_CSSPropertyValue GetEnum() const override;
  const FX_WCHAR* GetFuncName() const override;
  int32_t CountArgs() const override;
  IFDE_CSSValue* GetArgs(int32_t index) const override;

  FDE_CSSPrimitiveType m_eType;
  union {
    FX_ARGB m_dwColor;
    FX_FLOAT m_fNumber;
    const FX_WCHAR* m_pString;
    FDE_CSSPropertyValue m_eEnum;
    CFDE_CSSFunction* m_pFunction;
  };
};

typedef CFX_ArrayTemplate<IFDE_CSSPrimitiveValue*> CFDE_CSSPrimitiveArray;
typedef CFX_ArrayTemplate<IFDE_CSSValue*> CFDE_CSSValueArray;

class CFDE_CSSValueList : public IFDE_CSSValueList {
 public:
  explicit CFDE_CSSValueList(const CFDE_CSSValueArray& list);

  // IFDE_CSSValueList
  int32_t CountValues() const override;
  IFDE_CSSValue* GetValue(int32_t index) const override;

 protected:
  IFDE_CSSValue** m_ppList;
  int32_t m_iCount;
};

class CFDE_CSSValueListParser {
 public:
  CFDE_CSSValueListParser(const FX_WCHAR* psz, int32_t iLen, FX_WCHAR separator)
      : m_Separator(separator), m_pCur(psz), m_pEnd(psz + iLen) {
    ASSERT(psz && iLen > 0);
  }
  bool NextValue(FDE_CSSPrimitiveType& eType,
                 const FX_WCHAR*& pStart,
                 int32_t& iLength);
  FX_WCHAR m_Separator;

 protected:
  int32_t SkipTo(FX_WCHAR wch,
                 bool bWSSeparator = false,
                 bool bBrContinue = false);

  const FX_WCHAR* m_pCur;
  const FX_WCHAR* m_pEnd;
};

#define FDE_IsOnlyValue(type, enum) \
  (((type) & ~(enum)) == FDE_CSSVALUETYPE_Primitive)

struct FDE_CSSPropertyTable {
  FDE_CSSProperty eName;
  const FX_WCHAR* pszName;
  uint32_t dwHash;
  uint32_t dwType;
};

const FDE_CSSPropertyTable* FDE_GetCSSPropertyByName(
    const CFX_WideStringC& wsName);
const FDE_CSSPropertyTable* FDE_GetCSSPropertyByEnum(FDE_CSSProperty eName);

struct FDE_CSSPropertyValueTable {
  FDE_CSSPropertyValue eName;
  const FX_WCHAR* pszName;
  uint32_t dwHash;
};

const FDE_CSSPropertyValueTable* FDE_GetCSSPropertyValueByName(
    const CFX_WideStringC& wsName);
const FDE_CSSPropertyValueTable* FDE_GetCSSPropertyValueByEnum(
    FDE_CSSPropertyValue eName);

struct FDE_CSSMEDIATYPETABLE {
  uint16_t wHash;
  uint16_t wValue;
};

const FDE_CSSMEDIATYPETABLE* FDE_GetCSSMediaTypeByName(
    const CFX_WideStringC& wsName);

struct FDE_CSSLengthUnitTable {
  uint16_t wHash;
  FDE_CSSPrimitiveType wValue;
};

const FDE_CSSLengthUnitTable* FDE_GetCSSLengthUnitByName(
    const CFX_WideStringC& wsName);

struct FDE_CSSCOLORTABLE {
  uint32_t dwHash;
  FX_ARGB dwValue;
};

const FDE_CSSCOLORTABLE* FDE_GetCSSColorByName(const CFX_WideStringC& wsName);

struct FDE_CSSPseudoTable {
  FDE_CSSPseudo eName;
  const FX_WCHAR* pszName;
  uint32_t dwHash;
};

const FDE_CSSPseudoTable* FDE_GetCSSPseudoByEnum(FDE_CSSPseudo ePseudo);

bool FDE_ParseCSSNumber(const FX_WCHAR* pszValue,
                        int32_t iValueLen,
                        FX_FLOAT& fValue,
                        FDE_CSSPrimitiveType& eUnit);
bool FDE_ParseCSSString(const FX_WCHAR* pszValue,
                        int32_t iValueLen,
                        int32_t* iOffset,
                        int32_t* iLength);
bool FDE_ParseCSSColor(const FX_WCHAR* pszValue,
                       int32_t iValueLen,
                       FX_ARGB& dwColor);
bool FDE_ParseCSSURI(const FX_WCHAR* pszValue,
                     int32_t* iOffset,
                     int32_t* iLength);

#endif  // XFA_FDE_CSS_FDE_CSSDATATABLE_H_
