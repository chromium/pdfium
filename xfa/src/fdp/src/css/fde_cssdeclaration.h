// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_CSSDECLARATION
#define _FDE_CSSDECLARATION
#include "fde_cssdatatable.h"
typedef struct _FDE_CSSPROPERTYHOLDER : public CFX_Target {
  int16_t eProperty;
  int16_t bImportant;
  IFDE_CSSValue* pValue;
  _FDE_CSSPROPERTYHOLDER* pNext;
} FDE_CSSPROPERTYHOLDER, *FDE_LPCSSPROPERTYHOLDER;
typedef struct _FDE_CSSCUSTOMPROPERTY : public CFX_Target {
  const FX_WCHAR* pwsName;
  const FX_WCHAR* pwsValue;
  _FDE_CSSCUSTOMPROPERTY* pNext;
} FDE_CSSCUSTOMPROPERTY, *FDE_LPCSSCUSTOMPROPERTY;
typedef struct _FDE_CSSPROPERTYARGS : public CFX_Target {
  IFX_MEMAllocator* pStaticStore;
  CFX_MapPtrToPtr* pStringCache;
  FDE_LPCCSSPROPERTYTABLE pProperty;
} FDE_CSSPROPERTYARGS;
typedef FDE_CSSPROPERTYARGS const* FDE_LPCCSSPROPERTYARGS;
class CFDE_CSSDeclaration : public IFDE_CSSDeclaration, public CFX_Target {
 public:
  CFDE_CSSDeclaration()
      : m_pFirstProperty(NULL),
        m_pLastProperty(NULL),
        m_pFirstCustom(NULL),
        m_pLastCustom(NULL) {}
  virtual IFDE_CSSValue* GetProperty(FDE_CSSPROPERTY eProperty,
                                     FX_BOOL& bImportant) const;
  virtual FX_POSITION GetStartPosition() const;
  virtual void GetNextProperty(FX_POSITION& pos,
                               FDE_CSSPROPERTY& eProperty,
                               IFDE_CSSValue*& pValue,
                               FX_BOOL& bImportant) const;
  virtual FX_POSITION GetStartCustom() const;
  virtual void GetNextCustom(FX_POSITION& pos,
                             CFX_WideString& wsName,
                             CFX_WideString& wsValue) const;
  FX_BOOL AddProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                      const FX_WCHAR* pszValue,
                      int32_t iValueLen);
  FX_BOOL AddProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                      const FX_WCHAR* pszName,
                      int32_t iNameLen,
                      const FX_WCHAR* pszValue,
                      int32_t iValueLen);

 protected:
  FX_BOOL ParseTextEmphasisProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                    const FX_WCHAR* pszValue,
                                    int32_t iValueLen,
                                    FX_BOOL bImportant);
  FX_BOOL ParseColumnsProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                               const FX_WCHAR* pszValue,
                               int32_t iValueLen,
                               FX_BOOL bImportant);
  FX_BOOL ParseColumnRuleProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                  const FX_WCHAR* pszValue,
                                  int32_t iValueLen,
                                  FX_BOOL bImportant);
  FX_BOOL ParseOverflowProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                const FX_WCHAR* pszValue,
                                int32_t iValueLen,
                                FX_BOOL bImportant);
  FX_BOOL ParseFontProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                            const FX_WCHAR* pszValue,
                            int32_t iValueLen,
                            FX_BOOL bImportant);
  FX_BOOL ParseBackgroundProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                  const FX_WCHAR* pszValue,
                                  int32_t iValueLen,
                                  FX_BOOL bImportant);
  FX_BOOL ParseListStyleProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                 const FX_WCHAR* pszValue,
                                 int32_t iValueLen,
                                 FX_BOOL bImportant);
  FX_BOOL ParseBorderPropoerty(IFX_MEMAllocator* pStaticStore,
                               const FX_WCHAR* pszValue,
                               int32_t iValueLen,
                               IFDE_CSSValue*& pColor,
                               IFDE_CSSValue*& pStyle,
                               IFDE_CSSValue*& pWidth) const;
  void AddBorderProperty(IFX_MEMAllocator* pStaticStore,
                         IFDE_CSSValue* pColor,
                         IFDE_CSSValue* pStyle,
                         IFDE_CSSValue* pWidth,
                         FX_BOOL bImportant,
                         FDE_CSSPROPERTY eColor,
                         FDE_CSSPROPERTY eStyle,
                         FDE_CSSPROPERTY eWidth);
  FX_BOOL ParseContentProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                               const FX_WCHAR* pszValue,
                               int32_t iValueLen,
                               FX_BOOL bImportant);
  FX_BOOL ParseCounterProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                               const FX_WCHAR* pszValue,
                               int32_t iValueLen,
                               FX_BOOL bImportant);
  FX_BOOL ParseValueListProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                 const FX_WCHAR* pszValue,
                                 int32_t iValueLen,
                                 FX_BOOL bImportant);
  FX_BOOL Add4ValuesProperty(IFX_MEMAllocator* pStaticStore,
                             const CFDE_CSSValueArray& list,
                             FX_BOOL bImportant,
                             FDE_CSSPROPERTY eLeft,
                             FDE_CSSPROPERTY eTop,
                             FDE_CSSPROPERTY eRight,
                             FDE_CSSPROPERTY eBottom);
  IFDE_CSSValue* ParseNumber(FDE_LPCCSSPROPERTYARGS pArgs,
                             const FX_WCHAR* pszValue,
                             int32_t iValueLen);
  IFDE_CSSValue* ParseEnum(FDE_LPCCSSPROPERTYARGS pArgs,
                           const FX_WCHAR* pszValue,
                           int32_t iValueLen);
  IFDE_CSSValue* ParseColor(FDE_LPCCSSPROPERTYARGS pArgs,
                            const FX_WCHAR* pszValue,
                            int32_t iValueLen);
  IFDE_CSSValue* ParseURI(FDE_LPCCSSPROPERTYARGS pArgs,
                          const FX_WCHAR* pszValue,
                          int32_t iValueLen);
  IFDE_CSSValue* ParseString(FDE_LPCCSSPROPERTYARGS pArgs,
                             const FX_WCHAR* pszValue,
                             int32_t iValueLen);
  IFDE_CSSValue* ParseFunction(FDE_LPCCSSPROPERTYARGS pArgs,
                               const FX_WCHAR* pszValue,
                               int32_t iValueLen);
  const FX_WCHAR* CopyToLocal(FDE_LPCCSSPROPERTYARGS pArgs,
                              const FX_WCHAR* pszValue,
                              int32_t iValueLen);
  void AddPropertyHolder(IFX_MEMAllocator* pStaticStore,
                         FDE_CSSPROPERTY eProperty,
                         IFDE_CSSValue* pValue,
                         FX_BOOL bImportant);
  IFDE_CSSPrimitiveValue* NewNumberValue(IFX_MEMAllocator* pStaticStore,
                                         FDE_CSSPRIMITIVETYPE eUnit,
                                         FX_FLOAT fValue) const;
  IFDE_CSSPrimitiveValue* NewEnumValue(IFX_MEMAllocator* pStaticStore,
                                       FDE_CSSPROPERTYVALUE eValue) const;
  FDE_LPCSSPROPERTYHOLDER m_pFirstProperty;
  FDE_LPCSSPROPERTYHOLDER m_pLastProperty;
  FDE_LPCSSCUSTOMPROPERTY m_pFirstCustom;
  FDE_LPCSSCUSTOMPROPERTY m_pLastCustom;
};
#endif
