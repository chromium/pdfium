// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSSTYLESELECTOR_H_
#define XFA_FDE_CSS_CFDE_CSSSTYLESELECTOR_H_

#include <memory>
#include <vector>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fde/css/cfde_cssrulecollection.h"
#include "xfa/fde/css/fde_css.h"

class CFDE_CSSAccelerator;
class CFDE_CSSComputedStyle;
class CFDE_CSSDeclaration;
class CFDE_CSSSelector;
class CFDE_CSSStyleSheet;
class CFDE_CSSTagCache;
class CFDE_CSSValue;
class CFDE_CSSValueList;
class CFGAS_FontMgr;
class CXFA_CSSTagProvider;

class CFDE_CSSStyleSelector {
 public:
  explicit CFDE_CSSStyleSelector(CFGAS_FontMgr* pFontMgr);
  ~CFDE_CSSStyleSelector();

  void SetDefFontSize(FX_FLOAT fFontSize);

  bool SetStyleSheet(FDE_CSSStyleSheetGroup eType, CFDE_CSSStyleSheet* pSheet);
  bool SetStyleSheets(FDE_CSSStyleSheetGroup eType,
                      const CFX_ArrayTemplate<CFDE_CSSStyleSheet*>* pArray);
  void SetStylePriority(FDE_CSSStyleSheetGroup eType,
                        FDE_CSSStyleSheetPriority ePriority);
  void UpdateStyleIndex(uint32_t dwMediaList);
  CFDE_CSSAccelerator* InitAccelerator();
  CFDE_CSSComputedStyle* CreateComputedStyle(
      CFDE_CSSComputedStyle* pParentStyle);
  int32_t MatchDeclarations(
      CXFA_CSSTagProvider* pTag,
      CFX_ArrayTemplate<CFDE_CSSDeclaration*>& matchedDecls,
      FDE_CSSPseudo ePseudoType = FDE_CSSPseudo::NONE);
  void ComputeStyle(CXFA_CSSTagProvider* pTag,
                    const CFDE_CSSDeclaration** ppDeclArray,
                    int32_t iDeclCount,
                    CFDE_CSSComputedStyle* pDestStyle);

 protected:
  void Reset();
  void MatchRules(CFDE_CSSTagCache* pCache,
                  CFDE_CSSRuleCollection::Data* pList,
                  FDE_CSSPseudo ePseudoType);
  bool MatchSelector(CFDE_CSSTagCache* pCache,
                     CFDE_CSSSelector* pSel,
                     FDE_CSSPseudo ePseudoType);
  void AppendInlineStyle(CFDE_CSSDeclaration* pDecl,
                         const FX_WCHAR* psz,
                         int32_t iLen);
  void ApplyDeclarations(bool bPriority,
                         const CFDE_CSSDeclaration** ppDeclArray,
                         int32_t iDeclCount,
                         CFDE_CSSComputedStyle* pDestStyle);
  void ApplyProperty(FDE_CSSProperty eProperty,
                     CFDE_CSSValue* pValue,
                     CFDE_CSSComputedStyle* pComputedStyle);

  bool SetLengthWithPercent(FDE_CSSLength& width,
                            FDE_CSSPrimitiveType eType,
                            CFDE_CSSValue* pValue,
                            FX_FLOAT fFontSize);
  FX_FLOAT ToFontSize(FDE_CSSPropertyValue eValue, FX_FLOAT fCurFontSize);
  FDE_CSSDisplay ToDisplay(FDE_CSSPropertyValue eValue);
  FDE_CSSTextAlign ToTextAlign(FDE_CSSPropertyValue eValue);
  uint16_t ToFontWeight(FDE_CSSPropertyValue eValue);
  FDE_CSSFontStyle ToFontStyle(FDE_CSSPropertyValue eValue);
  FDE_CSSVerticalAlign ToVerticalAlign(FDE_CSSPropertyValue eValue);
  uint32_t ToTextDecoration(CFDE_CSSValueList* pList);
  FDE_CSSFontVariant ToFontVariant(FDE_CSSPropertyValue eValue);

  CFGAS_FontMgr* const m_pFontMgr;
  FX_FLOAT m_fDefFontSize;
  CFX_ArrayTemplate<CFDE_CSSStyleSheet*> m_SheetGroups[3];
  CFDE_CSSRuleCollection m_RuleCollection[3];
  FDE_CSSStyleSheetGroup m_ePriorities[3];
  std::unique_ptr<CFDE_CSSAccelerator> m_pAccelerator;
  std::vector<CFDE_CSSRuleCollection::Data*> m_MatchedRules;
};

#endif  // XFA_FDE_CSS_CFDE_CSSSTYLESELECTOR_H_
