// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "fde_cssdeclaration.h"
IFDE_CSSValue* CFDE_CSSDeclaration::GetProperty(FDE_CSSPROPERTY eProperty,
                                                FX_BOOL& bImportant) const {
  for (FDE_LPCSSPROPERTYHOLDER pHolder = m_pFirstProperty; pHolder;
       pHolder = pHolder->pNext) {
    if (pHolder->eProperty == eProperty) {
      bImportant = pHolder->bImportant;
      return pHolder->pValue;
    }
  }
  return NULL;
}
FX_POSITION CFDE_CSSDeclaration::GetStartPosition() const {
  return (FX_POSITION)m_pFirstProperty;
}
void CFDE_CSSDeclaration::GetNextProperty(FX_POSITION& pos,
                                          FDE_CSSPROPERTY& eProperty,
                                          IFDE_CSSValue*& pValue,
                                          FX_BOOL& bImportant) const {
  FDE_LPCSSPROPERTYHOLDER pHolder = (FDE_LPCSSPROPERTYHOLDER)pos;
  FXSYS_assert(pHolder != NULL);
  bImportant = pHolder->bImportant;
  eProperty = (FDE_CSSPROPERTY)pHolder->eProperty;
  pValue = pHolder->pValue;
  pos = (FX_POSITION)pHolder->pNext;
}
FX_POSITION CFDE_CSSDeclaration::GetStartCustom() const {
  return (FX_POSITION)m_pFirstCustom;
}
void CFDE_CSSDeclaration::GetNextCustom(FX_POSITION& pos,
                                        CFX_WideString& wsName,
                                        CFX_WideString& wsValue) const {
  FDE_LPCSSCUSTOMPROPERTY pProperty = (FDE_LPCSSCUSTOMPROPERTY)pos;
  if (pProperty == NULL) {
    return;
  }
  wsName = pProperty->pwsName;
  wsValue = pProperty->pwsValue;
  pos = (FX_POSITION)pProperty->pNext;
}
const FX_WCHAR* CFDE_CSSDeclaration::CopyToLocal(FDE_LPCCSSPROPERTYARGS pArgs,
                                                 const FX_WCHAR* pszValue,
                                                 int32_t iValueLen) {
  FXSYS_assert(iValueLen > 0);
  CFX_MapPtrToPtr* pCache = pArgs->pStringCache;
  void* pKey = NULL;
  if (pCache) {
    void* pszCached = NULL;
    pKey =
        (void*)(uintptr_t)FX_HashCode_String_GetW(pszValue, iValueLen, FALSE);
    if (pCache->Lookup(pKey, pszCached)) {
      return (const FX_WCHAR*)pszCached;
    }
  }
  FX_WCHAR* psz =
      (FX_WCHAR*)pArgs->pStaticStore->Alloc((iValueLen + 1) * sizeof(FX_WCHAR));
  if (psz == NULL) {
    return NULL;
  }
  FXSYS_wcsncpy(psz, pszValue, iValueLen);
  psz[iValueLen] = '\0';
  if (pCache) {
    pCache->SetAt(pKey, psz);
  }
  return psz;
}
IFDE_CSSPrimitiveValue* CFDE_CSSDeclaration::NewNumberValue(
    IFX_MEMAllocator* pStaticStore,
    FDE_CSSPRIMITIVETYPE eUnit,
    FX_FLOAT fValue) const {
  static CFDE_CSSPrimitiveValue s_ZeroValue(FDE_CSSPRIMITIVETYPE_Number, 0.0f);
  if (eUnit == FDE_CSSPRIMITIVETYPE_Number && FXSYS_fabs(fValue) < 0.001f) {
    return &s_ZeroValue;
  }
  return FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(eUnit, fValue);
}
inline IFDE_CSSPrimitiveValue* CFDE_CSSDeclaration::NewEnumValue(
    IFX_MEMAllocator* pStaticStore,
    FDE_CSSPROPERTYVALUE eValue) const {
  return FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(eValue);
}
void CFDE_CSSDeclaration::AddPropertyHolder(IFX_MEMAllocator* pStaticStore,
                                            FDE_CSSPROPERTY eProperty,
                                            IFDE_CSSValue* pValue,
                                            FX_BOOL bImportant) {
  FDE_LPCSSPROPERTYHOLDER pHolder =
      FDE_NewWith(pStaticStore) FDE_CSSPROPERTYHOLDER;
  pHolder->bImportant = bImportant;
  pHolder->eProperty = eProperty;
  pHolder->pValue = pValue;
  pHolder->pNext = NULL;
  if (m_pLastProperty == NULL) {
    m_pLastProperty = m_pFirstProperty = pHolder;
  } else {
    m_pLastProperty->pNext = pHolder;
    m_pLastProperty = pHolder;
  }
}
FX_BOOL CFDE_CSSDeclaration::AddProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                         const FX_WCHAR* pszValue,
                                         int32_t iValueLen) {
  FXSYS_assert(iValueLen > 0);
  FX_BOOL bImportant = FALSE;
  if (iValueLen >= 10 && pszValue[iValueLen - 10] == '!' &&
      FX_wcsnicmp(L"important", pszValue + iValueLen - 9, 9) == 0) {
    if ((iValueLen -= 10) == 0) {
      return FALSE;
    }
    bImportant = TRUE;
  }
  const FX_DWORD dwType = pArgs->pProperty->dwType;
  switch (dwType & 0x0F) {
    case FDE_CSSVALUETYPE_Primitive: {
      static const FX_DWORD g_ValueGuessOrder[] = {
          FDE_CSSVALUETYPE_MaybeNumber,   FDE_CSSVALUETYPE_MaybeEnum,
          FDE_CSSVALUETYPE_MaybeColor,    FDE_CSSVALUETYPE_MaybeURI,
          FDE_CSSVALUETYPE_MaybeFunction, FDE_CSSVALUETYPE_MaybeString,
      };
      static const int32_t g_ValueGuessCount =
          sizeof(g_ValueGuessOrder) / sizeof(FX_DWORD);
      for (int32_t i = 0; i < g_ValueGuessCount; ++i) {
        const FX_DWORD dwMatch = dwType & g_ValueGuessOrder[i];
        if (dwMatch == 0) {
          continue;
        }
        IFDE_CSSValue* pCSSValue = NULL;
        switch (dwMatch) {
          case FDE_CSSVALUETYPE_MaybeFunction:
            pCSSValue = ParseFunction(pArgs, pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeNumber:
            pCSSValue = ParseNumber(pArgs, pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeEnum:
            pCSSValue = ParseEnum(pArgs, pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeColor:
            pCSSValue = ParseColor(pArgs, pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeURI:
            pCSSValue = ParseURI(pArgs, pszValue, iValueLen);
            break;
          case FDE_CSSVALUETYPE_MaybeString:
            pCSSValue = ParseString(pArgs, pszValue, iValueLen);
            break;
          default:
            break;
        }
        if (pCSSValue != NULL) {
          AddPropertyHolder(pArgs->pStaticStore, pArgs->pProperty->eName,
                            pCSSValue, bImportant);
          return TRUE;
        }
        if (FDE_IsOnlyValue(dwType, g_ValueGuessOrder[i])) {
          return FALSE;
        }
      }
    } break;
    case FDE_CSSVALUETYPE_Shorthand: {
      IFX_MEMAllocator* pStaticStore = pArgs->pStaticStore;
      IFDE_CSSValue *pColor, *pStyle, *pWidth;
      switch (pArgs->pProperty->eName) {
        case FDE_CSSPROPERTY_Font:
          return ParseFontProperty(pArgs, pszValue, iValueLen, bImportant);
        case FDE_CSSPROPERTY_Background:
          return ParseBackgroundProperty(pArgs, pszValue, iValueLen,
                                         bImportant);
        case FDE_CSSPROPERTY_ListStyle:
          return ParseListStyleProperty(pArgs, pszValue, iValueLen, bImportant);
        case FDE_CSSPROPERTY_Border:
          if (ParseBorderPropoerty(pStaticStore, pszValue, iValueLen, pColor,
                                   pStyle, pWidth)) {
            AddBorderProperty(pStaticStore, pColor, pStyle, pWidth, bImportant,
                              FDE_CSSPROPERTY_BorderLeftColor,
                              FDE_CSSPROPERTY_BorderLeftStyle,
                              FDE_CSSPROPERTY_BorderLeftWidth);
            AddBorderProperty(pStaticStore, pColor, pStyle, pWidth, bImportant,
                              FDE_CSSPROPERTY_BorderTopColor,
                              FDE_CSSPROPERTY_BorderTopStyle,
                              FDE_CSSPROPERTY_BorderTopWidth);
            AddBorderProperty(pStaticStore, pColor, pStyle, pWidth, bImportant,
                              FDE_CSSPROPERTY_BorderRightColor,
                              FDE_CSSPROPERTY_BorderRightStyle,
                              FDE_CSSPROPERTY_BorderRightWidth);
            AddBorderProperty(pStaticStore, pColor, pStyle, pWidth, bImportant,
                              FDE_CSSPROPERTY_BorderBottomColor,
                              FDE_CSSPROPERTY_BorderBottomStyle,
                              FDE_CSSPROPERTY_BorderBottomWidth);
            return TRUE;
          }
          break;
        case FDE_CSSPROPERTY_BorderLeft:
          if (ParseBorderPropoerty(pStaticStore, pszValue, iValueLen, pColor,
                                   pStyle, pWidth)) {
            AddBorderProperty(pStaticStore, pColor, pStyle, pWidth, bImportant,
                              FDE_CSSPROPERTY_BorderLeftColor,
                              FDE_CSSPROPERTY_BorderLeftStyle,
                              FDE_CSSPROPERTY_BorderLeftWidth);
            return TRUE;
          }
          break;
        case FDE_CSSPROPERTY_BorderTop:
          if (ParseBorderPropoerty(pStaticStore, pszValue, iValueLen, pColor,
                                   pStyle, pWidth)) {
            AddBorderProperty(pStaticStore, pColor, pStyle, pWidth, bImportant,
                              FDE_CSSPROPERTY_BorderTopColor,
                              FDE_CSSPROPERTY_BorderTopStyle,
                              FDE_CSSPROPERTY_BorderTopWidth);
            return TRUE;
          }
          break;
        case FDE_CSSPROPERTY_BorderRight:
          if (ParseBorderPropoerty(pStaticStore, pszValue, iValueLen, pColor,
                                   pStyle, pWidth)) {
            AddBorderProperty(pStaticStore, pColor, pStyle, pWidth, bImportant,
                              FDE_CSSPROPERTY_BorderRightColor,
                              FDE_CSSPROPERTY_BorderRightStyle,
                              FDE_CSSPROPERTY_BorderRightWidth);
            return TRUE;
          }
          break;
        case FDE_CSSPROPERTY_BorderBottom:
          if (ParseBorderPropoerty(pStaticStore, pszValue, iValueLen, pColor,
                                   pStyle, pWidth)) {
            AddBorderProperty(pStaticStore, pColor, pStyle, pWidth, bImportant,
                              FDE_CSSPROPERTY_BorderBottomColor,
                              FDE_CSSPROPERTY_BorderBottomStyle,
                              FDE_CSSPROPERTY_BorderBottomWidth);
            return TRUE;
          }
          break;
        case FDE_CSSPROPERTY_Overflow:
          return ParseOverflowProperty(pArgs, pszValue, iValueLen, bImportant);
        case FDE_CSSPROPERTY_ColumnRule:
          return ParseColumnRuleProperty(pArgs, pszValue, iValueLen,
                                         bImportant);
        default:
          break;
      }
    } break;
    case FDE_CSSVALUETYPE_List:
      switch (pArgs->pProperty->eName) {
        case FDE_CSSPROPERTY_CounterIncrement:
        case FDE_CSSPROPERTY_CounterReset:
          return ParseCounterProperty(pArgs, pszValue, iValueLen, bImportant);
        case FDE_CSSPROPERTY_Content:
          return ParseContentProperty(pArgs, pszValue, iValueLen, bImportant);
        default:
          return ParseValueListProperty(pArgs, pszValue, iValueLen, bImportant);
      }
    default:
      FXSYS_assert(FALSE);
      break;
  }
  return FALSE;
}
FX_BOOL CFDE_CSSDeclaration::AddProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                         const FX_WCHAR* pszName,
                                         int32_t iNameLen,
                                         const FX_WCHAR* pszValue,
                                         int32_t iValueLen) {
  FDE_LPCSSCUSTOMPROPERTY pProperty =
      FDE_NewWith(pArgs->pStaticStore) FDE_CSSCUSTOMPROPERTY;
  pProperty->pwsName = CopyToLocal(pArgs, pszName, iNameLen);
  pProperty->pwsValue = CopyToLocal(pArgs, pszValue, iValueLen);
  pProperty->pNext = NULL;
  if (m_pLastCustom == NULL) {
    m_pLastCustom = m_pFirstCustom = pProperty;
  } else {
    m_pLastCustom->pNext = pProperty;
    m_pLastCustom = pProperty;
  }
  return TRUE;
}
IFDE_CSSValue* CFDE_CSSDeclaration::ParseNumber(FDE_LPCCSSPROPERTYARGS pArgs,
                                                const FX_WCHAR* pszValue,
                                                int32_t iValueLen) {
  FX_FLOAT fValue;
  FDE_CSSPRIMITIVETYPE eUnit;
  if (!FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eUnit)) {
    return NULL;
  }
  return NewNumberValue(pArgs->pStaticStore, eUnit, fValue);
}
IFDE_CSSValue* CFDE_CSSDeclaration::ParseEnum(FDE_LPCCSSPROPERTYARGS pArgs,
                                              const FX_WCHAR* pszValue,
                                              int32_t iValueLen) {
  FDE_LPCCSSPROPERTYVALUETABLE pValue =
      FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
  return pValue ? NewEnumValue(pArgs->pStaticStore, pValue->eName) : NULL;
}
IFDE_CSSValue* CFDE_CSSDeclaration::ParseColor(FDE_LPCCSSPROPERTYARGS pArgs,
                                               const FX_WCHAR* pszValue,
                                               int32_t iValueLen) {
  FX_ARGB dwColor;
  if (!FDE_ParseCSSColor(pszValue, iValueLen, dwColor)) {
    return NULL;
  }
  return FDE_NewWith(pArgs->pStaticStore) CFDE_CSSPrimitiveValue(dwColor);
}
IFDE_CSSValue* CFDE_CSSDeclaration::ParseURI(FDE_LPCCSSPROPERTYARGS pArgs,
                                             const FX_WCHAR* pszValue,
                                             int32_t iValueLen) {
  int32_t iOffset;
  if (!FDE_ParseCSSURI(pszValue, iValueLen, iOffset, iValueLen)) {
    return NULL;
  }
  if (iValueLen <= 0) {
    return NULL;
  }
  pszValue = CopyToLocal(pArgs, pszValue + iOffset, iValueLen);
  return pszValue
             ? FDE_NewWith(pArgs->pStaticStore)
                   CFDE_CSSPrimitiveValue(FDE_CSSPRIMITIVETYPE_URI, pszValue)
             : NULL;
}
IFDE_CSSValue* CFDE_CSSDeclaration::ParseString(FDE_LPCCSSPROPERTYARGS pArgs,
                                                const FX_WCHAR* pszValue,
                                                int32_t iValueLen) {
  int32_t iOffset;
  if (!FDE_ParseCSSString(pszValue, iValueLen, iOffset, iValueLen)) {
    return NULL;
  }
  if (iValueLen <= 0) {
    return NULL;
  }
  pszValue = CopyToLocal(pArgs, pszValue + iOffset, iValueLen);
  return pszValue
             ? FDE_NewWith(pArgs->pStaticStore)
                   CFDE_CSSPrimitiveValue(FDE_CSSPRIMITIVETYPE_String, pszValue)
             : NULL;
}
IFDE_CSSValue* CFDE_CSSDeclaration::ParseFunction(FDE_LPCCSSPROPERTYARGS pArgs,
                                                  const FX_WCHAR* pszValue,
                                                  int32_t iValueLen) {
  if (pszValue[iValueLen - 1] != ')') {
    return NULL;
  }
  int32_t iStartBracket = 0;
  while (pszValue[iStartBracket] != '(') {
    if (iStartBracket < iValueLen) {
      iStartBracket++;
    } else {
      return NULL;
    }
  }
  if (iStartBracket == 0) {
    return NULL;
  }
  const FX_WCHAR* pszFuncName = CopyToLocal(pArgs, pszValue, iStartBracket);
  pszValue += (iStartBracket + 1);
  iValueLen -= (iStartBracket + 2);
  CFDE_CSSValueArray argumentArr;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ',');
  FDE_CSSPRIMITIVETYPE ePrimitiveType;
  while (parser.NextValue(ePrimitiveType, pszValue, iValueLen)) {
    switch (ePrimitiveType) {
      case FDE_CSSPRIMITIVETYPE_String: {
        FDE_LPCCSSPROPERTYVALUETABLE pPropertyValue =
            FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
        if (pPropertyValue != NULL) {
          argumentArr.Add(
              NewEnumValue(pArgs->pStaticStore, pPropertyValue->eName));
          continue;
        }
        IFDE_CSSValue* pFunctionValue =
            ParseFunction(pArgs, pszValue, iValueLen);
        if (pFunctionValue != NULL) {
          argumentArr.Add(pFunctionValue);
          continue;
        }
        argumentArr.Add(FDE_NewWith(pArgs->pStaticStore) CFDE_CSSPrimitiveValue(
            FDE_CSSPRIMITIVETYPE_String,
            CopyToLocal(pArgs, pszValue, iValueLen)));
      } break;
      case FDE_CSSPRIMITIVETYPE_Number: {
        FX_FLOAT fValue;
        if (FDE_ParseCSSNumber(pszValue, iValueLen, fValue, ePrimitiveType)) {
          argumentArr.Add(
              NewNumberValue(pArgs->pStaticStore, ePrimitiveType, fValue));
        }
      } break;
      default:
        argumentArr.Add(FDE_NewWith(pArgs->pStaticStore) CFDE_CSSPrimitiveValue(
            FDE_CSSPRIMITIVETYPE_String,
            CopyToLocal(pArgs, pszValue, iValueLen)));
        break;
    }
  }
  IFDE_CSSValueList* pArgumentList = FDE_NewWith(pArgs->pStaticStore)
      CFDE_CSSValueList(pArgs->pStaticStore, argumentArr);
  CFDE_CSSFunction* pFunction = FDE_NewWith(pArgs->pStaticStore)
      CFDE_CSSFunction(pszFuncName, pArgumentList);
  return FDE_NewWith(pArgs->pStaticStore) CFDE_CSSPrimitiveValue(pFunction);
}
FX_BOOL CFDE_CSSDeclaration::ParseContentProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                                  const FX_WCHAR* pszValue,
                                                  int32_t iValueLen,
                                                  FX_BOOL bImportant) {
  IFX_MEMAllocator* pStaticStore = (IFX_MEMAllocator*)pArgs->pStaticStore;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  FDE_CSSPRIMITIVETYPE eType;
  CFDE_CSSValueArray list;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPRIMITIVETYPE_URI:
        list.Add(FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(
            eType, CopyToLocal(pArgs, pszValue, iValueLen)));
        break;
      case FDE_CSSPRIMITIVETYPE_Number:
        return FALSE;
      case FDE_CSSPRIMITIVETYPE_String: {
        FDE_LPCCSSPROPERTYVALUETABLE pValue =
            FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
        if (pValue != NULL) {
          switch (pValue->eName) {
            case FDE_CSSPROPERTYVALUE_Normal:
            case FDE_CSSPROPERTYVALUE_None: {
              if (list.GetSize() == 0) {
                list.Add(NewEnumValue(pStaticStore, pValue->eName));
              } else {
                return FALSE;
              }
            } break;
            case FDE_CSSPROPERTYVALUE_OpenQuote:
            case FDE_CSSPROPERTYVALUE_CloseQuote:
            case FDE_CSSPROPERTYVALUE_NoOpenQuote:
            case FDE_CSSPROPERTYVALUE_NoCloseQuote:
              list.Add(NewEnumValue(pStaticStore, pValue->eName));
              break;
            default:
              return FALSE;
          }
          continue;
        }
        IFDE_CSSValue* pFunction = ParseFunction(pArgs, pszValue, iValueLen);
        if (pFunction != NULL) {
          list.Add(pFunction);
          continue;
        }
        list.Add(FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(
            eType, CopyToLocal(pArgs, pszValue, iValueLen)));
      } break;
      case FDE_CSSPRIMITIVETYPE_RGB:
        return FALSE;
      default:
        break;
    }
  }
  if (list.GetSize() == 0) {
    return FALSE;
  }
  AddPropertyHolder(pStaticStore, pArgs->pProperty->eName,
                    FDE_NewWith(pStaticStore)
                        CFDE_CSSValueList(pStaticStore, list),
                    bImportant);
  return TRUE;
}
FX_BOOL CFDE_CSSDeclaration::ParseCounterProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                                  const FX_WCHAR* pszValue,
                                                  int32_t iValueLen,
                                                  FX_BOOL bImportant) {
  IFX_MEMAllocator* pStaticStore = pArgs->pStaticStore;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  CFDE_CSSValueArray list;
  CFDE_CSSValueArray listFull;
  FDE_CSSPRIMITIVETYPE eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPRIMITIVETYPE_Number: {
        FX_FLOAT fValue;
        if (FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eType)) {
          if (list.GetSize() == 1) {
            list.Add(NewNumberValue(pStaticStore, eType, fValue));
            listFull.Add(FDE_NewWith(pStaticStore)
                             CFDE_CSSValueList(pStaticStore, list));
            list.RemoveAll();
          } else {
            return FALSE;
          }
        }
      } break;
      case FDE_CSSPRIMITIVETYPE_String: {
        if (list.GetSize() == 0) {
          pszValue = CopyToLocal(pArgs, pszValue, iValueLen);
          list.Add(FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(
              FDE_CSSPRIMITIVETYPE_String, pszValue));
        } else {
          listFull.Add(FDE_NewWith(pStaticStore)
                           CFDE_CSSValueList(pStaticStore, list));
          list.RemoveAll();
          pszValue = CopyToLocal(pArgs, pszValue, iValueLen);
          list.Add(FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(
              FDE_CSSPRIMITIVETYPE_String, pszValue));
        }
      } break;
      default:
        break;
    }
  }
  if (list.GetSize() == 1) {
    listFull.Add(FDE_NewWith(pStaticStore)
                     CFDE_CSSValueList(pStaticStore, list));
  }
  if (listFull.GetSize() == 0) {
    return FALSE;
  }
  AddPropertyHolder(pStaticStore, pArgs->pProperty->eName,
                    FDE_NewWith(pStaticStore)
                        CFDE_CSSValueList(pStaticStore, listFull),
                    bImportant);
  return TRUE;
}
FX_BOOL CFDE_CSSDeclaration::ParseValueListProperty(
    FDE_LPCCSSPROPERTYARGS pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen,
    FX_BOOL bImportant) {
  IFX_MEMAllocator* pStaticStore = pArgs->pStaticStore;
  FX_WCHAR separator =
      (pArgs->pProperty->eName == FDE_CSSPROPERTY_FontFamily) ? ',' : ' ';
  CFDE_CSSValueListParser parser(pszValue, iValueLen, separator);
  const FX_DWORD dwType = pArgs->pProperty->dwType;
  FDE_CSSPRIMITIVETYPE eType;
  CFDE_CSSValueArray list;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPRIMITIVETYPE_Number:
        if (dwType & FDE_CSSVALUETYPE_MaybeNumber) {
          FX_FLOAT fValue;
          if (FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eType)) {
            list.Add(NewNumberValue(pStaticStore, eType, fValue));
          }
        }
        break;
      case FDE_CSSPRIMITIVETYPE_String:
        if (dwType & FDE_CSSVALUETYPE_MaybeColor) {
          FX_ARGB dwColor;
          if (FDE_ParseCSSColor(pszValue, iValueLen, dwColor)) {
            list.Add(FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(dwColor));
            continue;
          }
        }
        if (dwType & FDE_CSSVALUETYPE_MaybeEnum) {
          FDE_LPCCSSPROPERTYVALUETABLE pValue =
              FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
          if (pValue != NULL) {
            list.Add(NewEnumValue(pStaticStore, pValue->eName));
            continue;
          }
        }
        if (dwType & FDE_CSSVALUETYPE_MaybeString) {
          pszValue = CopyToLocal(pArgs, pszValue, iValueLen);
          list.Add(FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(
              FDE_CSSPRIMITIVETYPE_String, pszValue));
        }
        break;
      case FDE_CSSPRIMITIVETYPE_RGB:
        if (dwType & FDE_CSSVALUETYPE_MaybeColor) {
          FX_ARGB dwColor;
          if (FDE_ParseCSSColor(pszValue, iValueLen, dwColor)) {
            list.Add(FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(dwColor));
          }
        }
        break;
      default:
        break;
    }
  }
  if (list.GetSize() == 0) {
    return FALSE;
  }
  switch (pArgs->pProperty->eName) {
    case FDE_CSSPROPERTY_BorderColor:
      return Add4ValuesProperty(
          pStaticStore, list, bImportant, FDE_CSSPROPERTY_BorderLeftColor,
          FDE_CSSPROPERTY_BorderTopColor, FDE_CSSPROPERTY_BorderRightColor,
          FDE_CSSPROPERTY_BorderBottomColor);
    case FDE_CSSPROPERTY_BorderStyle:
      return Add4ValuesProperty(
          pStaticStore, list, bImportant, FDE_CSSPROPERTY_BorderLeftStyle,
          FDE_CSSPROPERTY_BorderTopStyle, FDE_CSSPROPERTY_BorderRightStyle,
          FDE_CSSPROPERTY_BorderBottomStyle);
    case FDE_CSSPROPERTY_BorderWidth:
      return Add4ValuesProperty(
          pStaticStore, list, bImportant, FDE_CSSPROPERTY_BorderLeftWidth,
          FDE_CSSPROPERTY_BorderTopWidth, FDE_CSSPROPERTY_BorderRightWidth,
          FDE_CSSPROPERTY_BorderBottomWidth);
    case FDE_CSSPROPERTY_Margin:
      return Add4ValuesProperty(
          pStaticStore, list, bImportant, FDE_CSSPROPERTY_MarginLeft,
          FDE_CSSPROPERTY_MarginTop, FDE_CSSPROPERTY_MarginRight,
          FDE_CSSPROPERTY_MarginBottom);
    case FDE_CSSPROPERTY_Padding:
      return Add4ValuesProperty(
          pStaticStore, list, bImportant, FDE_CSSPROPERTY_PaddingLeft,
          FDE_CSSPROPERTY_PaddingTop, FDE_CSSPROPERTY_PaddingRight,
          FDE_CSSPROPERTY_PaddingBottom);
    default: {
      CFDE_CSSValueList* pList =
          FDE_NewWith(pStaticStore) CFDE_CSSValueList(pStaticStore, list);
      AddPropertyHolder(pStaticStore, pArgs->pProperty->eName, pList,
                        bImportant);
      return TRUE;
    } break;
  }
  return FALSE;
}
FX_BOOL CFDE_CSSDeclaration::Add4ValuesProperty(IFX_MEMAllocator* pStaticStore,
                                                const CFDE_CSSValueArray& list,
                                                FX_BOOL bImportant,
                                                FDE_CSSPROPERTY eLeft,
                                                FDE_CSSPROPERTY eTop,
                                                FDE_CSSPROPERTY eRight,
                                                FDE_CSSPROPERTY eBottom) {
  switch (list.GetSize()) {
    case 1:
      AddPropertyHolder(pStaticStore, eLeft, list[0], bImportant);
      AddPropertyHolder(pStaticStore, eTop, list[0], bImportant);
      AddPropertyHolder(pStaticStore, eRight, list[0], bImportant);
      AddPropertyHolder(pStaticStore, eBottom, list[0], bImportant);
      return TRUE;
    case 2:
      AddPropertyHolder(pStaticStore, eLeft, list[1], bImportant);
      AddPropertyHolder(pStaticStore, eTop, list[0], bImportant);
      AddPropertyHolder(pStaticStore, eRight, list[1], bImportant);
      AddPropertyHolder(pStaticStore, eBottom, list[0], bImportant);
      return TRUE;
    case 3:
      AddPropertyHolder(pStaticStore, eLeft, list[1], bImportant);
      AddPropertyHolder(pStaticStore, eTop, list[0], bImportant);
      AddPropertyHolder(pStaticStore, eRight, list[1], bImportant);
      AddPropertyHolder(pStaticStore, eBottom, list[2], bImportant);
      return TRUE;
    case 4:
      AddPropertyHolder(pStaticStore, eLeft, list[3], bImportant);
      AddPropertyHolder(pStaticStore, eTop, list[0], bImportant);
      AddPropertyHolder(pStaticStore, eRight, list[1], bImportant);
      AddPropertyHolder(pStaticStore, eBottom, list[2], bImportant);
      return TRUE;
    default:
      break;
  }
  return FALSE;
}
FX_BOOL CFDE_CSSDeclaration::ParseBorderPropoerty(
    IFX_MEMAllocator* pStaticStore,
    const FX_WCHAR* pszValue,
    int32_t iValueLen,
    IFDE_CSSValue*& pColor,
    IFDE_CSSValue*& pStyle,
    IFDE_CSSValue*& pWidth) const {
  pColor = pStyle = pWidth = NULL;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  FDE_CSSPRIMITIVETYPE eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPRIMITIVETYPE_Number:
        if (pWidth == NULL) {
          FX_FLOAT fValue;
          if (FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eType)) {
            pWidth = NewNumberValue(pStaticStore, eType, fValue);
          }
        }
        break;
      case FDE_CSSPRIMITIVETYPE_RGB:
        if (pColor == NULL) {
          FX_ARGB dwColor;
          if (FDE_ParseCSSColor(pszValue, iValueLen, dwColor)) {
            pColor = FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(dwColor);
          }
        }
        break;
      case FDE_CSSPRIMITIVETYPE_String: {
        FDE_LPCCSSCOLORTABLE pColorItem =
            FDE_GetCSSColorByName(pszValue, iValueLen);
        if (pColorItem != NULL) {
          if (pColor == NULL) {
            pColor = FDE_NewWith(pStaticStore)
                CFDE_CSSPrimitiveValue(pColorItem->dwValue);
          }
          continue;
        }
        FDE_LPCCSSPROPERTYVALUETABLE pValue =
            FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
        if (pValue == NULL) {
          continue;
        }
        switch (pValue->eName) {
          case FDE_CSSPROPERTYVALUE_Transparent:
            if (pColor == NULL) {
              pColor =
                  FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue((FX_ARGB)0);
            }
            break;
          case FDE_CSSPROPERTYVALUE_Thin:
          case FDE_CSSPROPERTYVALUE_Thick:
          case FDE_CSSPROPERTYVALUE_Medium:
            if (pWidth == NULL) {
              pWidth = NewEnumValue(pStaticStore, pValue->eName);
            }
            break;
          case FDE_CSSPROPERTYVALUE_None:
          case FDE_CSSPROPERTYVALUE_Hidden:
          case FDE_CSSPROPERTYVALUE_Dotted:
          case FDE_CSSPROPERTYVALUE_Dashed:
          case FDE_CSSPROPERTYVALUE_Solid:
          case FDE_CSSPROPERTYVALUE_Double:
          case FDE_CSSPROPERTYVALUE_Groove:
          case FDE_CSSPROPERTYVALUE_Ridge:
          case FDE_CSSPROPERTYVALUE_Inset:
          case FDE_CSSPROPERTYVALUE_Outset:
            if (pStyle == NULL) {
              pStyle = NewEnumValue(pStaticStore, pValue->eName);
            }
            break;
          default:
            break;
        }
      }; break;
      default:
        break;
    }
  }
  if (pColor == NULL) {
    pColor = FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue((FX_ARGB)0);
  }
  if (pStyle == NULL) {
    pStyle = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_None);
  }
  if (pWidth == NULL) {
    pWidth = NewNumberValue(pStaticStore, FDE_CSSPRIMITIVETYPE_Number, 0.0f);
  }
  return TRUE;
}
void CFDE_CSSDeclaration::AddBorderProperty(IFX_MEMAllocator* pStaticStore,
                                            IFDE_CSSValue* pColor,
                                            IFDE_CSSValue* pStyle,
                                            IFDE_CSSValue* pWidth,
                                            FX_BOOL bImportant,
                                            FDE_CSSPROPERTY eColor,
                                            FDE_CSSPROPERTY eStyle,
                                            FDE_CSSPROPERTY eWidth) {
  AddPropertyHolder(pStaticStore, eStyle, pStyle, bImportant);
  AddPropertyHolder(pStaticStore, eWidth, pWidth, bImportant);
  AddPropertyHolder(pStaticStore, eColor, pColor, bImportant);
}
FX_BOOL CFDE_CSSDeclaration::ParseListStyleProperty(
    FDE_LPCCSSPROPERTYARGS pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen,
    FX_BOOL bImportant) {
  IFX_MEMAllocator* pStaticStore = pArgs->pStaticStore;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  IFDE_CSSPrimitiveValue *pType = NULL, *pImage = NULL, *pPosition = NULL;
  FDE_CSSPRIMITIVETYPE eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPRIMITIVETYPE_URI:
        if (pImage == NULL) {
          pImage = FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(
              eType, CopyToLocal(pArgs, pszValue, iValueLen));
        }
        break;
      case FDE_CSSPRIMITIVETYPE_String: {
        FDE_LPCCSSPROPERTYVALUETABLE pValue =
            FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
        if (pValue == NULL) {
          break;
        }
        switch (pValue->eName) {
          case FDE_CSSPROPERTYVALUE_None:
            if (pImage == NULL) {
              pImage = NewEnumValue(pStaticStore, pValue->eName);
            } else if (pType == NULL) {
              pImage = NewEnumValue(pStaticStore, pValue->eName);
            }
            break;
          case FDE_CSSPROPERTYVALUE_Inside:
          case FDE_CSSPROPERTYVALUE_Outside:
            if (pPosition == NULL) {
              pPosition = NewEnumValue(pStaticStore, pValue->eName);
            }
            break;
          case FDE_CSSPROPERTYVALUE_Disc:
          case FDE_CSSPROPERTYVALUE_Circle:
          case FDE_CSSPROPERTYVALUE_Square:
          case FDE_CSSPROPERTYVALUE_Decimal:
          case FDE_CSSPROPERTYVALUE_DecimalLeadingZero:
          case FDE_CSSPROPERTYVALUE_LowerRoman:
          case FDE_CSSPROPERTYVALUE_UpperRoman:
          case FDE_CSSPROPERTYVALUE_LowerGreek:
          case FDE_CSSPROPERTYVALUE_LowerLatin:
          case FDE_CSSPROPERTYVALUE_UpperLatin:
          case FDE_CSSPROPERTYVALUE_Armenian:
          case FDE_CSSPROPERTYVALUE_Georgian:
          case FDE_CSSPROPERTYVALUE_LowerAlpha:
          case FDE_CSSPROPERTYVALUE_UpperAlpha:
            if (pType == NULL) {
              pType = NewEnumValue(pStaticStore, pValue->eName);
            }
            break;
          default:
            break;
        }
      }; break;
      default:
        break;
    }
  }
  if (pPosition == NULL) {
    pPosition = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Outside);
  }
  if (pImage == NULL) {
    pImage = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_None);
  }
  if (pType == NULL) {
    pType = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_None);
  }
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_ListStylePosition, pPosition,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_ListStyleImage, pImage,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_ListStyleType, pType,
                    bImportant);
  return TRUE;
}
FX_BOOL CFDE_CSSDeclaration::ParseBackgroundProperty(
    FDE_LPCCSSPROPERTYARGS pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen,
    FX_BOOL bImportant) {
  IFX_MEMAllocator* pStaticStore = pArgs->pStaticStore;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  IFDE_CSSPrimitiveValue *pColor = NULL, *pImage = NULL, *pRepeat = NULL;
  IFDE_CSSPrimitiveValue *pPosX = NULL, *pPosY = NULL, *pAttachment = NULL;
  FDE_CSSPRIMITIVETYPE eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPRIMITIVETYPE_URI:
        if (pImage == NULL) {
          pImage = FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(
              eType, CopyToLocal(pArgs, pszValue, iValueLen));
        }
        break;
      case FDE_CSSPRIMITIVETYPE_Number: {
        FX_FLOAT fValue;
        if (!FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eType)) {
          break;
        }
        if (pPosX == NULL) {
          pPosX = NewNumberValue(pStaticStore, eType, fValue);
        } else if (pPosY == NULL) {
          pPosY = NewNumberValue(pStaticStore, eType, fValue);
        }
      } break;
      case FDE_CSSPRIMITIVETYPE_String: {
        FDE_LPCCSSPROPERTYVALUETABLE pValue =
            FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
        if (pValue != NULL) {
          switch (pValue->eName) {
            case FDE_CSSPROPERTYVALUE_None:
              if (pImage == NULL) {
                pImage = NewEnumValue(pStaticStore, pValue->eName);
              }
              break;
            case FDE_CSSPROPERTYVALUE_Transparent:
              if (pColor == NULL) {
                pColor = FDE_NewWith(pStaticStore)
                    CFDE_CSSPrimitiveValue((FX_ARGB)0);
              }
              break;
            case FDE_CSSPROPERTYVALUE_Fixed:
            case FDE_CSSPROPERTYVALUE_Scroll:
              if (pAttachment == NULL) {
                pAttachment = NewEnumValue(pStaticStore, pValue->eName);
              }
              break;
            case FDE_CSSPROPERTYVALUE_Repeat:
            case FDE_CSSPROPERTYVALUE_RepeatX:
            case FDE_CSSPROPERTYVALUE_RepeatY:
            case FDE_CSSPROPERTYVALUE_NoRepeat:
              if (pRepeat == NULL) {
                pRepeat = NewEnumValue(pStaticStore, pValue->eName);
              }
              break;
            case FDE_CSSPROPERTYVALUE_Left:
            case FDE_CSSPROPERTYVALUE_Right:
              if (pPosX == NULL) {
                pPosX = NewEnumValue(pStaticStore, pValue->eName);
              }
              break;
            case FDE_CSSPROPERTYVALUE_Top:
            case FDE_CSSPROPERTYVALUE_Bottom:
              if (pPosY == NULL) {
                pPosX = NewEnumValue(pStaticStore, pValue->eName);
              }
              break;
            case FDE_CSSPROPERTYVALUE_Center:
              if (pPosX == NULL) {
                pPosX = NewEnumValue(pStaticStore, pValue->eName);
              } else if (pPosY == NULL) {
                pPosX = NewEnumValue(pStaticStore, pValue->eName);
              }
              break;
            default:
              break;
          }
          break;
        }
        FDE_LPCCSSCOLORTABLE pColorItem =
            FDE_GetCSSColorByName(pszValue, iValueLen);
        if (pColorItem != NULL)
          if (pColor == NULL) {
            pColor = FDE_NewWith(pStaticStore)
                CFDE_CSSPrimitiveValue(pColorItem->dwValue);
          }
      } break;
      case FDE_CSSPRIMITIVETYPE_RGB:
        if (pColor == NULL) {
          FX_ARGB dwColor;
          if (FDE_ParseCSSColor(pszValue, iValueLen, dwColor)) {
            pColor = FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(dwColor);
          }
        }
        break;
      default:
        break;
    }
  }
  if (pColor == NULL) {
    pColor = FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue((FX_ARGB)0);
  }
  if (pImage == NULL) {
    pImage = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_None);
  }
  if (pRepeat == NULL) {
    pRepeat = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Repeat);
  }
  if (pAttachment == NULL) {
    pAttachment = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Scroll);
  }
  if (pPosX == NULL) {
    pPosX = NewNumberValue(pStaticStore, FDE_CSSPRIMITIVETYPE_Number, 0.0f);
    pPosY = NewNumberValue(pStaticStore, FDE_CSSPRIMITIVETYPE_Number, 0.0f);
  } else if (pPosY == NULL) {
    pPosY = NewNumberValue(pStaticStore, FDE_CSSPRIMITIVETYPE_Number, 0.0f);
  }
  CFDE_CSSValueArray position;
  position.Add(pPosX);
  position.Add(pPosY);
  CFDE_CSSValueList* pPosList =
      FDE_NewWith(pStaticStore) CFDE_CSSValueList(pStaticStore, position);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_BackgroundColor, pColor,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_BackgroundImage, pImage,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_BackgroundRepeat, pRepeat,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_BackgroundPosition, pPosList,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_BackgroundAttachment,
                    pAttachment, bImportant);
  return TRUE;
}
FX_BOOL CFDE_CSSDeclaration::ParseFontProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                               const FX_WCHAR* pszValue,
                                               int32_t iValueLen,
                                               FX_BOOL bImportant) {
  IFX_MEMAllocator* pStaticStore = pArgs->pStaticStore;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, '/');
  IFDE_CSSPrimitiveValue *pStyle = NULL, *pVariant = NULL, *pWeight = NULL;
  IFDE_CSSPrimitiveValue *pFontSize = NULL, *pLineHeight = NULL;
  CFDE_CSSValueArray familyList;
  FDE_CSSPRIMITIVETYPE eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPRIMITIVETYPE_String: {
        FDE_LPCCSSPROPERTYVALUETABLE pValue =
            FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
        if (pValue != NULL) {
          switch (pValue->eName) {
            case FDE_CSSPROPERTYVALUE_XxSmall:
            case FDE_CSSPROPERTYVALUE_XSmall:
            case FDE_CSSPROPERTYVALUE_Small:
            case FDE_CSSPROPERTYVALUE_Medium:
            case FDE_CSSPROPERTYVALUE_Large:
            case FDE_CSSPROPERTYVALUE_XLarge:
            case FDE_CSSPROPERTYVALUE_XxLarge:
            case FDE_CSSPROPERTYVALUE_Smaller:
            case FDE_CSSPROPERTYVALUE_Larger:
              if (pFontSize == NULL) {
                pFontSize = NewEnumValue(pStaticStore, pValue->eName);
              }
              continue;
            case FDE_CSSPROPERTYVALUE_Bold:
            case FDE_CSSPROPERTYVALUE_Bolder:
            case FDE_CSSPROPERTYVALUE_Lighter:
              if (pWeight == NULL) {
                pWeight = NewEnumValue(pStaticStore, pValue->eName);
              }
              continue;
            case FDE_CSSPROPERTYVALUE_Italic:
            case FDE_CSSPROPERTYVALUE_Oblique:
              if (pStyle == NULL) {
                pStyle = NewEnumValue(pStaticStore, pValue->eName);
              }
              continue;
            case FDE_CSSPROPERTYVALUE_SmallCaps:
              if (pVariant == NULL) {
                pVariant = NewEnumValue(pStaticStore, pValue->eName);
              }
              continue;
            case FDE_CSSPROPERTYVALUE_Normal:
              if (pStyle == NULL) {
                pStyle = NewEnumValue(pStaticStore, pValue->eName);
              } else if (pVariant == NULL) {
                pVariant = NewEnumValue(pStaticStore, pValue->eName);
              } else if (pWeight == NULL) {
                pWeight = NewEnumValue(pStaticStore, pValue->eName);
              } else if (pFontSize == NULL) {
                pFontSize = NewEnumValue(pStaticStore, pValue->eName);
              } else if (pLineHeight == NULL) {
                pLineHeight = NewEnumValue(pStaticStore, pValue->eName);
              }
              continue;
            default:
              break;
          }
        }
        if (pFontSize != NULL) {
          familyList.Add(FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(
              eType, CopyToLocal(pArgs, pszValue, iValueLen)));
        }
        parser.m_Separator = ',';
      } break;
      case FDE_CSSPRIMITIVETYPE_Number: {
        FX_FLOAT fValue;
        if (!FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eType)) {
          break;
        }
        if (eType == FDE_CSSPRIMITIVETYPE_Number) {
          switch ((int32_t)fValue) {
            case 100:
            case 200:
            case 300:
            case 400:
            case 500:
            case 600:
            case 700:
            case 800:
            case 900:
              if (pWeight == NULL) {
                pWeight = NewNumberValue(pStaticStore,
                                         FDE_CSSPRIMITIVETYPE_Number, fValue);
              }
              continue;
          }
        }
        if (pFontSize == NULL) {
          pFontSize = NewNumberValue(pStaticStore, eType, fValue);
        } else if (pLineHeight == NULL) {
          pLineHeight = NewNumberValue(pStaticStore, eType, fValue);
        }
      } break;
      default:
        break;
    }
  }
  if (pStyle == NULL) {
    pStyle = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Normal);
  }
  if (pVariant == NULL) {
    pVariant = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Normal);
  }
  if (pWeight == NULL) {
    pWeight = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Normal);
  }
  if (pFontSize == NULL) {
    pFontSize = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Medium);
  }
  if (pLineHeight == NULL) {
    pLineHeight = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Normal);
  }
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_FontStyle, pStyle,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_FontVariant, pVariant,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_FontWeight, pWeight,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_FontSize, pFontSize,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_LineHeight, pLineHeight,
                    bImportant);
  if (familyList.GetSize() > 0) {
    CFDE_CSSValueList* pList =
        FDE_NewWith(pStaticStore) CFDE_CSSValueList(pStaticStore, familyList);
    AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_FontFamily, pList,
                      bImportant);
  }
  return TRUE;
}
FX_BOOL CFDE_CSSDeclaration::ParseColumnRuleProperty(
    FDE_LPCCSSPROPERTYARGS pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen,
    FX_BOOL bImportant) {
  IFX_MEMAllocator* pStaticStore = pArgs->pStaticStore;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  IFDE_CSSPrimitiveValue* pColumnRuleWidth = NULL;
  IFDE_CSSPrimitiveValue* pColumnRuleStyle = NULL;
  IFDE_CSSPrimitiveValue* pColumnRuleColor = NULL;
  FDE_CSSPRIMITIVETYPE eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPRIMITIVETYPE_String: {
        FDE_LPCCSSPROPERTYVALUETABLE pValue =
            FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
        if (pValue != NULL) {
          switch (pValue->eName) {
            case FDE_CSSPROPERTYVALUE_None:
            case FDE_CSSPROPERTYVALUE_Hidden:
            case FDE_CSSPROPERTYVALUE_Dotted:
            case FDE_CSSPROPERTYVALUE_Dashed:
            case FDE_CSSPROPERTYVALUE_Solid:
            case FDE_CSSPROPERTYVALUE_Double:
            case FDE_CSSPROPERTYVALUE_Groove:
            case FDE_CSSPROPERTYVALUE_Ridge:
            case FDE_CSSPROPERTYVALUE_Inset:
            case FDE_CSSPROPERTYVALUE_Outset:
              if (pColumnRuleStyle == NULL) {
                pColumnRuleStyle = NewEnumValue(pStaticStore, pValue->eName);
              }
              break;
            case FDE_CSSPROPERTYVALUE_Transparent:
              if (pColumnRuleColor == NULL) {
                pColumnRuleColor = NewEnumValue(pStaticStore, pValue->eName);
              }
              break;
            case FDE_CSSPROPERTYVALUE_Thin:
            case FDE_CSSPROPERTYVALUE_Medium:
            case FDE_CSSPROPERTYVALUE_Thick:
              if (pColumnRuleWidth == NULL) {
                pColumnRuleWidth = NewEnumValue(pStaticStore, pValue->eName);
              }
              break;
            default:
              break;
          }
          continue;
        }
        FX_ARGB dwColor;
        if (FDE_ParseCSSColor(pszValue, iValueLen, dwColor) &&
            pColumnRuleColor == NULL) {
          pColumnRuleColor = FDE_NewWith(pStaticStore)
              CFDE_CSSPrimitiveValue((FX_ARGB)dwColor);
          continue;
        }
      } break;
      case FDE_CSSPRIMITIVETYPE_Number: {
        FX_FLOAT fValue;
        if (FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eType) &&
            pColumnRuleWidth == NULL) {
          pColumnRuleWidth = NewNumberValue(pStaticStore, eType, fValue);
        }
      } break;
      case FDE_CSSPRIMITIVETYPE_RGB: {
        FX_ARGB dwColor;
        if (pColumnRuleColor == NULL &&
            FDE_ParseCSSColor(pszValue, iValueLen, dwColor)) {
          pColumnRuleColor = FDE_NewWith(pStaticStore)
              CFDE_CSSPrimitiveValue((FX_ARGB)dwColor);
        }
      } break;
      default:
        break;
    }
  }
  if (pColumnRuleColor == NULL && pColumnRuleStyle == NULL &&
      pColumnRuleWidth == NULL) {
    return FALSE;
  }
  if (pColumnRuleStyle == NULL) {
    pColumnRuleStyle = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_None);
  }
  if (pColumnRuleWidth == NULL) {
    pColumnRuleWidth = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Medium);
  }
  if (pColumnRuleColor == NULL) {
    pColumnRuleColor =
        FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue((FX_ARGB)0);
  }
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_ColumnRuleStyle,
                    pColumnRuleStyle, bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_ColumnRuleWidth,
                    pColumnRuleWidth, bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_ColumnRuleColor,
                    pColumnRuleColor, bImportant);
  return TRUE;
}
FX_BOOL CFDE_CSSDeclaration::ParseTextEmphasisProperty(
    FDE_LPCCSSPROPERTYARGS pArgs,
    const FX_WCHAR* pszValue,
    int32_t iValueLen,
    FX_BOOL bImportant) {
  IFX_MEMAllocator* pStaticStore = pArgs->pStaticStore;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  CFDE_CSSValueArray arrEmphasisStyle;
  FDE_CSSPRIMITIVETYPE eType;
  IFDE_CSSPrimitiveValue* pEmphasisColor = NULL;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPRIMITIVETYPE_String: {
        FDE_LPCCSSPROPERTYVALUETABLE pValue =
            FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
        if (pValue != NULL) {
          arrEmphasisStyle.Add(NewEnumValue(pStaticStore, pValue->eName));
          continue;
        }
        FX_ARGB dwColor;
        if (FDE_ParseCSSColor(pszValue, iValueLen, dwColor)) {
          pEmphasisColor =
              FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(dwColor);
          continue;
        }
        pszValue = CopyToLocal(pArgs, pszValue, iValueLen);
        arrEmphasisStyle.Add(FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(
            FDE_CSSPRIMITIVETYPE_String, pszValue));
      } break;
      case FDE_CSSPRIMITIVETYPE_RGB: {
        FX_ARGB dwColor;
        if (FDE_ParseCSSColor(pszValue, iValueLen, dwColor)) {
          pEmphasisColor =
              FDE_NewWith(pStaticStore) CFDE_CSSPrimitiveValue(dwColor);
        }
      } break;
      default:
        break;
    }
  }
  if (arrEmphasisStyle.GetSize() != 0) {
    AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_TextEmphasisStyle,
                      FDE_NewWith(pStaticStore)
                          CFDE_CSSValueList(pStaticStore, arrEmphasisStyle),
                      bImportant);
  }
  if (pEmphasisColor != NULL) {
    AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_TextEmphasisColor,
                      pEmphasisColor, bImportant);
  }
  return TRUE;
}
FX_BOOL CFDE_CSSDeclaration::ParseColumnsProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                                  const FX_WCHAR* pszValue,
                                                  int32_t iValueLen,
                                                  FX_BOOL bImportant) {
  IFX_MEMAllocator* pStaticStore = pArgs->pStaticStore;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  IFDE_CSSPrimitiveValue* pColumnWidth = NULL;
  IFDE_CSSPrimitiveValue* pColumnCount = NULL;
  FDE_CSSPRIMITIVETYPE eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    switch (eType) {
      case FDE_CSSPRIMITIVETYPE_String: {
        FDE_LPCCSSPROPERTYVALUETABLE pValue =
            FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
        if (pValue == NULL && pValue->eName == FDE_CSSPROPERTYVALUE_Auto) {
          pColumnWidth = NewEnumValue(pStaticStore, pValue->eName);
        }
      } break;
      case FDE_CSSPRIMITIVETYPE_Number: {
        FX_FLOAT fValue;
        if (FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eType)) {
          switch (eType) {
            case FDE_CSSPRIMITIVETYPE_Number:
              if (pColumnCount == NULL) {
                pColumnCount = NewNumberValue(pStaticStore, eType, fValue);
              }
              break;
            default:
              if (pColumnWidth == NULL) {
                pColumnWidth = NewNumberValue(pStaticStore, eType, fValue);
              }
              break;
          }
        }
      } break;
      default:
        break;
    }
  }
  if (pColumnWidth == NULL && pColumnCount == NULL) {
    return FALSE;
  } else if (pColumnWidth == NULL) {
    pColumnWidth = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Auto);
  } else if (pColumnCount == NULL) {
    pColumnCount = NewEnumValue(pStaticStore, FDE_CSSPROPERTYVALUE_Auto);
  }
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_ColumnWidth, pColumnWidth,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_ColumnCount, pColumnCount,
                    bImportant);
  return TRUE;
}
FX_BOOL CFDE_CSSDeclaration::ParseOverflowProperty(FDE_LPCCSSPROPERTYARGS pArgs,
                                                   const FX_WCHAR* pszValue,
                                                   int32_t iValueLen,
                                                   FX_BOOL bImportant) {
  IFX_MEMAllocator* pStaticStore = pArgs->pStaticStore;
  CFDE_CSSValueListParser parser(pszValue, iValueLen, ' ');
  IFDE_CSSPrimitiveValue* pOverflowX = NULL;
  IFDE_CSSPrimitiveValue* pOverflowY = NULL;
  FDE_CSSPRIMITIVETYPE eType;
  while (parser.NextValue(eType, pszValue, iValueLen)) {
    if (eType == FDE_CSSPRIMITIVETYPE_String) {
      FDE_LPCCSSPROPERTYVALUETABLE pValue =
          FDE_GetCSSPropertyValueByName(pszValue, iValueLen);
      if (pValue != NULL) {
        switch (pValue->eName) {
          case FDE_CSSOVERFLOW_Visible:
          case FDE_CSSOVERFLOW_Hidden:
          case FDE_CSSOVERFLOW_Scroll:
          case FDE_CSSOVERFLOW_Auto:
          case FDE_CSSOVERFLOW_NoDisplay:
          case FDE_CSSOVERFLOW_NoContent:
            if (pOverflowX != NULL && pOverflowY != NULL) {
              return FALSE;
            } else if (pOverflowX == NULL) {
              pOverflowX = NewEnumValue(pStaticStore, pValue->eName);
            } else if (pOverflowY == NULL) {
              pOverflowY = NewEnumValue(pStaticStore, pValue->eName);
            }
            break;
          default:
            break;
        }
      }
    }
  }
  if (pOverflowX == NULL && pOverflowY == NULL) {
    return FALSE;
  } else if (pOverflowY == NULL) {
    pOverflowY = NewEnumValue(pStaticStore, pOverflowX->GetEnum());
  }
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_OverflowX, pOverflowX,
                    bImportant);
  AddPropertyHolder(pStaticStore, FDE_CSSPROPERTY_OverflowY, pOverflowY,
                    bImportant);
  return TRUE;
}
