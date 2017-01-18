// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSDATATABLE_H_
#define XFA_FDE_CSS_FDE_CSSDATATABLE_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "xfa/fde/css/cfde_cssnumbervalue.h"
#include "xfa/fde/css/cfde_cssvalue.h"
#include "xfa/fde/css/fde_css.h"

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
  FDE_CSSNumberType wValue;
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
                        FDE_CSSNumberType& eUnit);
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
