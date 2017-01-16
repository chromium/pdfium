// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSSTYLESELECTOR_H_
#define XFA_FDE_CSS_FDE_CSSSTYLESELECTOR_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_ext.h"
#include "xfa/fde/css/fde_css.h"
#include "xfa/fde/css/fde_csscache.h"
#include "xfa/fde/css/fde_cssdeclaration.h"

class CFDE_CSSAccelerator;
class CFDE_CSSComputedStyle;
class CFDE_CSSRule;
class CFDE_CSSSelector;
class CFDE_CSSStyleSheet;
class CXFA_CSSTagProvider;

class FDE_CSSRuleData {
 public:
  FDE_CSSRuleData(CFDE_CSSSelector* pSel,
                  CFDE_CSSDeclaration* pDecl,
                  uint32_t dwPos);

  CFDE_CSSSelector* const pSelector;
  CFDE_CSSDeclaration* const pDeclaration;
  uint32_t dwPriority;
  FDE_CSSRuleData* pNext;
};

class CFDE_CSSRuleCollection {
 public:
  CFDE_CSSRuleCollection();
  ~CFDE_CSSRuleCollection();

  void AddRulesFrom(const CFX_ArrayTemplate<CFDE_CSSStyleSheet*>& sheets,
                    uint32_t dwMediaList,
                    CFGAS_FontMgr* pFontMgr);
  void Clear();
  int32_t CountSelectors() const { return m_iSelectors; }

  FDE_CSSRuleData* GetIDRuleData(uint32_t dwIDHash) {
    auto it = m_IDRules.find(dwIDHash);
    return it != m_IDRules.end() ? it->second : nullptr;
  }

  FDE_CSSRuleData* GetTagRuleData(uint32_t dwTagHash) {
    auto it = m_TagRules.find(dwTagHash);
    return it != m_TagRules.end() ? it->second : nullptr;
  }

  FDE_CSSRuleData* GetClassRuleData(uint32_t dwIDHash) {
    auto it = m_ClassRules.find(dwIDHash);
    return it != m_ClassRules.end() ? it->second : nullptr;
  }

  FDE_CSSRuleData* GetUniversalRuleData() { return m_pUniversalRules; }
  FDE_CSSRuleData* GetPseudoRuleData() { return m_pPseudoRules; }

 protected:
  void AddRulesFrom(CFDE_CSSStyleSheet* pStyleSheet,
                    CFDE_CSSRule* pRule,
                    uint32_t dwMediaList,
                    CFGAS_FontMgr* pFontMgr);
  void AddRuleTo(std::map<uint32_t, FDE_CSSRuleData*>* pMap,
                 uint32_t dwKey,
                 CFDE_CSSSelector* pSel,
                 CFDE_CSSDeclaration* pDecl);
  bool AddRuleTo(FDE_CSSRuleData** pList, FDE_CSSRuleData* pData);
  FDE_CSSRuleData* NewRuleData(CFDE_CSSSelector* pSel,
                               CFDE_CSSDeclaration* pDecl);

  std::map<uint32_t, FDE_CSSRuleData*> m_IDRules;
  std::map<uint32_t, FDE_CSSRuleData*> m_TagRules;
  std::map<uint32_t, FDE_CSSRuleData*> m_ClassRules;
  FDE_CSSRuleData* m_pUniversalRules;
  FDE_CSSRuleData* m_pPseudoRules;
  int32_t m_iSelectors;
};

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
  void MatchRules(FDE_CSSTagCache* pCache,
                  FDE_CSSRuleData* pList,
                  FDE_CSSPseudo ePseudoType);
  bool MatchSelector(FDE_CSSTagCache* pCache,
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

  FX_FLOAT ApplyNumber(FDE_CSSPrimitiveType eUnit,
                       FX_FLOAT fValue,
                       FX_FLOAT fPercentBase);
  bool SetLengthWithPercent(FDE_CSSLength& width,
                            FDE_CSSPrimitiveType eType,
                            CFDE_CSSPrimitiveValue* pPrimitive,
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
  std::vector<FDE_CSSRuleData*> m_MatchedRules;
};

class CFDE_CSSInheritedData {
 public:
  CFDE_CSSInheritedData();

  FDE_CSSLength m_LetterSpacing;
  FDE_CSSLength m_WordSpacing;
  FDE_CSSLength m_TextIndent;
  CFDE_CSSValueList* m_pFontFamily;
  FX_FLOAT m_fFontSize;
  FX_FLOAT m_fLineHeight;
  FX_ARGB m_dwFontColor;
  uint16_t m_wFontWeight;
  FDE_CSSFontVariant m_eFontVariant;
  FDE_CSSFontStyle m_eFontStyle;
  FDE_CSSTextAlign m_eTextAlign;
};

class CFDE_CSSNonInheritedData {
 public:
  CFDE_CSSNonInheritedData();

  FDE_CSSRect m_MarginWidth;
  FDE_CSSRect m_BorderWidth;
  FDE_CSSRect m_PaddingWidth;
  FDE_CSSLength m_Top;
  FDE_CSSLength m_Bottom;
  FDE_CSSLength m_Left;
  FDE_CSSLength m_Right;
  FX_FLOAT m_fVerticalAlign;
  FDE_CSSDisplay m_eDisplay;
  FDE_CSSVerticalAlign m_eVerticalAlign;
  uint8_t m_dwTextDecoration;
  bool m_bHasMargin;
  bool m_bHasBorder;
  bool m_bHasPadding;
};

class CFDE_CSSComputedStyle : public IFX_Retainable {
 public:
  CFDE_CSSComputedStyle();
  ~CFDE_CSSComputedStyle() override;

  // IFX_Retainable
  uint32_t Retain() override;
  uint32_t Release() override;

  int32_t CountFontFamilies() const;
  const FX_WCHAR* GetFontFamily(int32_t index) const;
  uint16_t GetFontWeight() const;
  FDE_CSSFontVariant GetFontVariant() const;
  FDE_CSSFontStyle GetFontStyle() const;
  FX_FLOAT GetFontSize() const;
  FX_ARGB GetColor() const;
  void SetFontWeight(uint16_t wFontWeight);
  void SetFontVariant(FDE_CSSFontVariant eFontVariant);
  void SetFontStyle(FDE_CSSFontStyle eFontStyle);
  void SetFontSize(FX_FLOAT fFontSize);
  void SetColor(FX_ARGB dwFontColor);

  const FDE_CSSRect* GetBorderWidth() const;
  const FDE_CSSRect* GetMarginWidth() const;
  const FDE_CSSRect* GetPaddingWidth() const;
  void SetMarginWidth(const FDE_CSSRect& rect);
  void SetPaddingWidth(const FDE_CSSRect& rect);

  FDE_CSSDisplay GetDisplay() const;

  FX_FLOAT GetLineHeight() const;
  const FDE_CSSLength& GetTextIndent() const;
  FDE_CSSTextAlign GetTextAlign() const;
  FDE_CSSVerticalAlign GetVerticalAlign() const;
  FX_FLOAT GetNumberVerticalAlign() const;
  uint32_t GetTextDecoration() const;
  const FDE_CSSLength& GetLetterSpacing() const;
  void SetLineHeight(FX_FLOAT fLineHeight);
  void SetTextIndent(const FDE_CSSLength& textIndent);
  void SetTextAlign(FDE_CSSTextAlign eTextAlign);
  void SetNumberVerticalAlign(FX_FLOAT fAlign);
  void SetTextDecoration(uint32_t dwTextDecoration);
  void SetLetterSpacing(const FDE_CSSLength& letterSpacing);
  void AddCustomStyle(const CFX_WideString& wsName,
                      const CFX_WideString& wsValue);

  bool GetCustomStyle(const CFX_WideStringC& wsName,
                      CFX_WideString& wsValue) const;

  CFDE_CSSInheritedData m_InheritedData;
  CFDE_CSSNonInheritedData m_NonInheritedData;

 private:
  uint32_t m_dwRefCount;
  std::vector<CFX_WideString> m_CustomProperties;
};

#endif  // XFA_FDE_CSS_FDE_CSSSTYLESELECTOR_H_
