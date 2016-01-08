// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_CSSDATATABLE
#define _FDE_CSSDATATABLE
class CFDE_CSSFunction : public CFX_Target {
 public:
  CFDE_CSSFunction(const FX_WCHAR* pszFuncName, IFDE_CSSValueList* pArgList)
      : m_pArgList(pArgList), m_pszFuncName(pszFuncName) {
    FXSYS_assert(pArgList != NULL);
  }
  int32_t CountArgs() const { return m_pArgList->CountValues(); }
  IFDE_CSSValue* GetArgs(int32_t index) const {
    return m_pArgList->GetValue(index);
  }
  const FX_WCHAR* GetFuncName() const { return m_pszFuncName; };

 protected:
  IFDE_CSSValueList* m_pArgList;
  const FX_WCHAR* m_pszFuncName;
};
class CFDE_CSSPrimitiveValue : public IFDE_CSSPrimitiveValue,
                               public CFX_Target {
 public:
  CFDE_CSSPrimitiveValue(const CFDE_CSSPrimitiveValue& src) { *this = src; }
  CFDE_CSSPrimitiveValue(FX_ARGB color)
      : m_eType(FDE_CSSPRIMITIVETYPE_RGB), m_dwColor(color) {}
  CFDE_CSSPrimitiveValue(FDE_CSSPROPERTYVALUE eValue)
      : m_eType(FDE_CSSPRIMITIVETYPE_Enum), m_eEnum(eValue) {}
  CFDE_CSSPrimitiveValue(FDE_CSSPRIMITIVETYPE eType, FX_FLOAT fValue)
      : m_eType(eType), m_fNumber(fValue) {}
  CFDE_CSSPrimitiveValue(FDE_CSSPRIMITIVETYPE eType, const FX_WCHAR* pValue)
      : m_eType(eType), m_pString(pValue) {
    FXSYS_assert(m_pString != NULL);
  }
  CFDE_CSSPrimitiveValue(CFDE_CSSFunction* pFunction)
      : m_eType(FDE_CSSPRIMITIVETYPE_Function), m_pFunction(pFunction) {}

  virtual FDE_CSSPRIMITIVETYPE GetPrimitiveType() const { return m_eType; }

  virtual FX_ARGB GetRGBColor() const {
    FXSYS_assert(m_eType == FDE_CSSPRIMITIVETYPE_RGB);
    return m_dwColor;
  }
  virtual FX_FLOAT GetFloat() const {
    FXSYS_assert(m_eType >= FDE_CSSPRIMITIVETYPE_Number &&
                 m_eType <= FDE_CSSPRIMITIVETYPE_PC);
    return m_fNumber;
  }
  virtual const FX_WCHAR* GetString(int32_t& iLength) const {
    FXSYS_assert(m_eType >= FDE_CSSPRIMITIVETYPE_String &&
                 m_eType <= FDE_CSSPRIMITIVETYPE_URI);
    iLength = FXSYS_wcslen(m_pString);
    return m_pString;
  }
  virtual FDE_CSSPROPERTYVALUE GetEnum() const {
    FXSYS_assert(m_eType == FDE_CSSPRIMITIVETYPE_Enum);
    return m_eEnum;
  }
  virtual const FX_WCHAR* GetFuncName() const {
    FXSYS_assert(m_eType == FDE_CSSPRIMITIVETYPE_Function);
    return m_pFunction->GetFuncName();
  }
  virtual int32_t CountArgs() const {
    FXSYS_assert(m_eType == FDE_CSSPRIMITIVETYPE_Function);
    return m_pFunction->CountArgs();
  }
  virtual IFDE_CSSValue* GetArgs(int32_t index) const {
    FXSYS_assert(m_eType == FDE_CSSPRIMITIVETYPE_Function);
    return m_pFunction->GetArgs(index);
  }

  FDE_CSSPRIMITIVETYPE m_eType;
  union {
    FX_ARGB m_dwColor;
    FX_FLOAT m_fNumber;
    const FX_WCHAR* m_pString;
    FDE_CSSPROPERTYVALUE m_eEnum;
    CFDE_CSSFunction* m_pFunction;
  };
};
typedef CFX_ArrayTemplate<IFDE_CSSPrimitiveValue*> CFDE_CSSPrimitiveArray;
typedef CFX_ArrayTemplate<IFDE_CSSValue*> CFDE_CSSValueArray;
class CFDE_CSSValueList : public IFDE_CSSValueList, public CFX_Target {
 public:
  CFDE_CSSValueList(IFX_MEMAllocator* pStaticStore,
                    const CFDE_CSSValueArray& list);
  virtual int32_t CountValues() const { return m_iCount; }
  virtual IFDE_CSSValue* GetValue(int32_t index) const {
    return m_ppList[index];
  }

 protected:
  IFDE_CSSValue** m_ppList;
  int32_t m_iCount;
};
class CFDE_CSSValueListParser : public CFX_Target {
 public:
  CFDE_CSSValueListParser(const FX_WCHAR* psz, int32_t iLen, FX_WCHAR separator)
      : m_Separator(separator), m_pCur(psz), m_pEnd(psz + iLen) {
    FXSYS_assert(psz != NULL && iLen > 0);
  }
  FX_BOOL NextValue(FDE_CSSPRIMITIVETYPE& eType,
                    const FX_WCHAR*& pStart,
                    int32_t& iLength);
  FX_WCHAR m_Separator;

 protected:
  int32_t SkipTo(FX_WCHAR wch,
                 FX_BOOL bWSSeparator = FALSE,
                 FX_BOOL bBrContinue = FALSE);
  const FX_WCHAR* m_pCur;
  const FX_WCHAR* m_pEnd;
};

#define FDE_CSSVALUETYPE_MaybeNumber 0x0100
#define FDE_CSSVALUETYPE_MaybeEnum 0x0200
#define FDE_CSSVALUETYPE_MaybeURI 0x0400
#define FDE_CSSVALUETYPE_MaybeString 0x0800
#define FDE_CSSVALUETYPE_MaybeColor 0x1000
#define FDE_CSSVALUETYPE_MaybeFunction 0x2000
#define FDE_IsOnlyValue(type, enum) \
  (((type) & ~(enum)) == FDE_CSSVALUETYPE_Primitive)
struct FDE_CSSPROPERTYTABLE {
  FDE_CSSPROPERTY eName;
  const FX_WCHAR* pszName;
  FX_DWORD dwHash;
  FX_DWORD dwType;
};
typedef FDE_CSSPROPERTYTABLE const* FDE_LPCCSSPROPERTYTABLE;

FDE_LPCCSSPROPERTYTABLE FDE_GetCSSPropertyByName(const FX_WCHAR* pszName,
                                                 int32_t iLength);
FDE_LPCCSSPROPERTYTABLE FDE_GetCSSPropertyByEnum(FDE_CSSPROPERTY eName);
struct FDE_CSSPROPERTYVALUETABLE {
  FDE_CSSPROPERTYVALUE eName;
  const FX_WCHAR* pszName;
  FX_DWORD dwHash;
};
typedef FDE_CSSPROPERTYVALUETABLE const* FDE_LPCCSSPROPERTYVALUETABLE;

FDE_LPCCSSPROPERTYVALUETABLE FDE_GetCSSPropertyValueByName(
    const FX_WCHAR* pszName,
    int32_t iLength);
FDE_LPCCSSPROPERTYVALUETABLE FDE_GetCSSPropertyValueByEnum(
    FDE_CSSPROPERTYVALUE eName);
struct FDE_CSSMEDIATYPETABLE {
  FX_WORD wHash;
  FX_WORD wValue;
};
typedef FDE_CSSMEDIATYPETABLE const* FDE_LPCCSSMEDIATYPETABLE;
FDE_LPCCSSMEDIATYPETABLE FDE_GetCSSMediaTypeByName(const FX_WCHAR* pszName,
                                                   int32_t iLength);
struct FDE_CSSLENGTHUNITTABLE {
  FX_WORD wHash;
  FX_WORD wValue;
};
typedef FDE_CSSLENGTHUNITTABLE const* FDE_LPCCSSLENGTHUNITTABLE;
FDE_LPCCSSLENGTHUNITTABLE FDE_GetCSSLengthUnitByName(const FX_WCHAR* pszName,
                                                     int32_t iLength);
struct FDE_CSSCOLORTABLE {
  FX_DWORD dwHash;
  FX_ARGB dwValue;
};
typedef FDE_CSSCOLORTABLE const* FDE_LPCCSSCOLORTABLE;
FDE_LPCCSSCOLORTABLE FDE_GetCSSColorByName(const FX_WCHAR* pszName,
                                           int32_t iLength);
struct FDE_CSSPERSUDOTABLE {
  FDE_CSSPERSUDO eName;
  const FX_WCHAR* pszName;
  FX_DWORD dwHash;
};
typedef FDE_CSSPERSUDOTABLE const* FDE_LPCCSSPERSUDOTABLE;

FDE_LPCCSSPERSUDOTABLE FDE_GetCSSPersudoByEnum(FDE_CSSPERSUDO ePersudo);
FX_BOOL FDE_ParseCSSNumber(const FX_WCHAR* pszValue,
                           int32_t iValueLen,
                           FX_FLOAT& fValue,
                           FDE_CSSPRIMITIVETYPE& eUnit);
FX_BOOL FDE_ParseCSSString(const FX_WCHAR* pszValue,
                           int32_t iValueLen,
                           int32_t& iOffset,
                           int32_t& iLength);
FX_BOOL FDE_ParseCSSColor(const FX_WCHAR* pszValue,
                          int32_t iValueLen,
                          FX_ARGB& dwColor);
FX_BOOL FDE_ParseCSSURI(const FX_WCHAR* pszValue,
                        int32_t iValueLen,
                        int32_t& iOffset,
                        int32_t& iLength);

#endif
