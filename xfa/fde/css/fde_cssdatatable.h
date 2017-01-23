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

struct FDE_CSSPropertyValueTable {
  FDE_CSSPropertyValue eName;
  const FX_WCHAR* pszName;
  uint32_t dwHash;
};

struct FDE_CSSLengthUnitTable {
  uint16_t wHash;
  FDE_CSSNumberType wValue;
};

struct FDE_CSSCOLORTABLE {
  uint32_t dwHash;
  FX_ARGB dwValue;
};

const FDE_CSSPropertyTable* FDE_GetCSSPropertyByName(
    const CFX_WideStringC& wsName);
const FDE_CSSPropertyTable* FDE_GetCSSPropertyByEnum(FDE_CSSProperty eName);

const FDE_CSSPropertyValueTable* FDE_GetCSSPropertyValueByName(
    const CFX_WideStringC& wsName);

const FDE_CSSLengthUnitTable* FDE_GetCSSLengthUnitByName(
    const CFX_WideStringC& wsName);

const FDE_CSSCOLORTABLE* FDE_GetCSSColorByName(const CFX_WideStringC& wsName);

#endif  // XFA_FDE_CSS_FDE_CSSDATATABLE_H_
