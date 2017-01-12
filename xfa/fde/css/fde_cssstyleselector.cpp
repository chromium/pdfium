// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/css/fde_cssstyleselector.h"

#include <algorithm>
#include <memory>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/css/fde_csscache.h"
#include "xfa/fde/css/fde_cssdeclaration.h"
#include "xfa/fde/css/fde_cssstylesheet.h"
#include "xfa/fde/css/fde_csssyntax.h"
#include "xfa/fxfa/app/cxfa_csstagprovider.h"

#define FDE_CSSUNIVERSALHASH ('*')

FDE_CSSRuleData::FDE_CSSRuleData(CFDE_CSSSelector* pSel,
                                 CFDE_CSSDeclaration* pDecl,
                                 uint32_t dwPos)
    : pSelector(pSel), pDeclaration(pDecl), dwPriority(dwPos), pNext(nullptr) {
  static const uint32_t s_Specific[5] = {0x00010000, 0x00010000, 0x00100000,
                                         0x00100000, 0x01000000};
  for (; pSel; pSel = pSel->GetNextSelector()) {
    FDE_CSSSelectorType eType = pSel->GetType();
    if (eType > FDE_CSSSelectorType::Descendant ||
        pSel->GetNameHash() != FDE_CSSUNIVERSALHASH) {
      dwPriority += s_Specific[static_cast<int>(eType)];
    }
  }
}

CFDE_CSSStyleSelector::CFDE_CSSStyleSelector(CFGAS_FontMgr* pFontMgr)
    : m_pFontMgr(pFontMgr), m_fDefFontSize(12.0f) {
  m_ePriorities[static_cast<int32_t>(FDE_CSSStyleSheetPriority::High)] =
      FDE_CSSStyleSheetGroup::Author;
  m_ePriorities[static_cast<int32_t>(FDE_CSSStyleSheetPriority::Mid)] =
      FDE_CSSStyleSheetGroup::User;
  m_ePriorities[static_cast<int32_t>(FDE_CSSStyleSheetPriority::Low)] =
      FDE_CSSStyleSheetGroup::UserAgent;
}

CFDE_CSSStyleSelector::~CFDE_CSSStyleSelector() {
  Reset();
}

void CFDE_CSSStyleSelector::SetDefFontSize(FX_FLOAT fFontSize) {
  ASSERT(fFontSize > 0);
  m_fDefFontSize = fFontSize;
}

CFDE_CSSAccelerator* CFDE_CSSStyleSelector::InitAccelerator() {
  if (!m_pAccelerator)
    m_pAccelerator = pdfium::MakeUnique<CFDE_CSSAccelerator>();
  m_pAccelerator->Clear();
  return m_pAccelerator.get();
}

IFDE_CSSComputedStyle* CFDE_CSSStyleSelector::CreateComputedStyle(
    IFDE_CSSComputedStyle* pParentStyle) {
  CFDE_CSSComputedStyle* pStyle = new CFDE_CSSComputedStyle();
  if (pParentStyle) {
    pStyle->m_InheritedData =
        static_cast<CFDE_CSSComputedStyle*>(pParentStyle)->m_InheritedData;
  }
  return pStyle;
}

bool CFDE_CSSStyleSelector::SetStyleSheet(FDE_CSSStyleSheetGroup eType,
                                          IFDE_CSSStyleSheet* pSheet) {
  CFX_ArrayTemplate<IFDE_CSSStyleSheet*>& dest =
      m_SheetGroups[static_cast<int32_t>(eType)];
  dest.RemoveAt(0, dest.GetSize());
  if (pSheet)
    dest.Add(pSheet);
  return true;
}

bool CFDE_CSSStyleSelector::SetStyleSheets(
    FDE_CSSStyleSheetGroup eType,
    const CFX_ArrayTemplate<IFDE_CSSStyleSheet*>* pArray) {
  CFX_ArrayTemplate<IFDE_CSSStyleSheet*>& dest =
      m_SheetGroups[static_cast<int32_t>(eType)];
  if (pArray)
    dest.Copy(*pArray);
  else
    dest.RemoveAt(0, dest.GetSize());
  return true;
}

void CFDE_CSSStyleSelector::SetStylePriority(
    FDE_CSSStyleSheetGroup eType,
    FDE_CSSStyleSheetPriority ePriority) {
  m_ePriorities[static_cast<int32_t>(ePriority)] = eType;
}

void CFDE_CSSStyleSelector::UpdateStyleIndex(uint32_t dwMediaList) {
  Reset();

  // TODO(dsinclair): Hard coded size bad. This should probably just be a map.
  for (int32_t iGroup = 0; iGroup < 3; ++iGroup) {
    CFDE_CSSRuleCollection& rules = m_RuleCollection[iGroup];
    rules.AddRulesFrom(m_SheetGroups[iGroup], dwMediaList, m_pFontMgr);
  }
}

void CFDE_CSSStyleSelector::Reset() {
  // TODO(dsinclair): Hard coded size bad. This should probably just be a map.
  for (int32_t iGroup = 0; iGroup < 3; ++iGroup) {
    m_RuleCollection[iGroup].Clear();
  }
}

int32_t CFDE_CSSStyleSelector::MatchDeclarations(
    CXFA_CSSTagProvider* pTag,
    CFX_ArrayTemplate<CFDE_CSSDeclaration*>& matchedDecls,
    FDE_CSSPseudo ePseudoType) {
  ASSERT(pTag);
  FDE_CSSTagCache* pCache = m_pAccelerator->GetTopElement();
  ASSERT(pCache && pCache->GetTag() == pTag);

  matchedDecls.RemoveAt(0, matchedDecls.GetSize());
  // TODO(dsinclair): Hard coded size bad ...
  for (int32_t ePriority = 2; ePriority >= 0; --ePriority) {
    FDE_CSSStyleSheetGroup eGroup = m_ePriorities[ePriority];
    CFDE_CSSRuleCollection& rules =
        m_RuleCollection[static_cast<int32_t>(eGroup)];
    if (rules.CountSelectors() == 0)
      continue;

    if (ePseudoType == FDE_CSSPseudo::NONE) {
      MatchRules(pCache, rules.GetUniversalRuleData(), ePseudoType);
      if (pCache->HashTag()) {
        MatchRules(pCache, rules.GetTagRuleData(pCache->HashTag()),
                   ePseudoType);
      }
      int32_t iCount = pCache->CountHashClass();
      for (int32_t i = 0; i < iCount; i++) {
        pCache->SetClassIndex(i);
        MatchRules(pCache, rules.GetClassRuleData(pCache->HashClass()),
                   ePseudoType);
      }
    } else {
      MatchRules(pCache, rules.GetPseudoRuleData(), ePseudoType);
    }

    std::sort(m_MatchedRules.begin(), m_MatchedRules.end(),
              [](const FDE_CSSRuleData* p1, const FDE_CSSRuleData* p2) {
                return p1->dwPriority < p2->dwPriority;
              });
    for (const auto& rule : m_MatchedRules)
      matchedDecls.Add(rule->pDeclaration);
    m_MatchedRules.clear();
  }
  return matchedDecls.GetSize();
}

void CFDE_CSSStyleSelector::MatchRules(FDE_CSSTagCache* pCache,
                                       FDE_CSSRuleData* pList,
                                       FDE_CSSPseudo ePseudoType) {
  while (pList) {
    if (MatchSelector(pCache, pList->pSelector, ePseudoType))
      m_MatchedRules.push_back(pList);
    pList = pList->pNext;
  }
}

bool CFDE_CSSStyleSelector::MatchSelector(FDE_CSSTagCache* pCache,
                                          CFDE_CSSSelector* pSel,
                                          FDE_CSSPseudo ePseudoType) {
  uint32_t dwHash;
  while (pSel && pCache) {
    switch (pSel->GetType()) {
      case FDE_CSSSelectorType::Descendant:
        dwHash = pSel->GetNameHash();
        while ((pCache = pCache->GetParent()) != nullptr) {
          if (dwHash != FDE_CSSUNIVERSALHASH && dwHash != pCache->HashTag()) {
            continue;
          }
          if (MatchSelector(pCache, pSel->GetNextSelector(), ePseudoType)) {
            return true;
          }
        }
        return false;
      case FDE_CSSSelectorType::ID:
        dwHash = pCache->HashID();
        if (dwHash != pSel->GetNameHash()) {
          return false;
        }
        break;
      case FDE_CSSSelectorType::Class:
        dwHash = pCache->HashClass();
        if (dwHash != pSel->GetNameHash()) {
          return false;
        }
        break;
      case FDE_CSSSelectorType::Element:
        dwHash = pSel->GetNameHash();
        if (dwHash != FDE_CSSUNIVERSALHASH && dwHash != pCache->HashTag()) {
          return false;
        }
        break;
      case FDE_CSSSelectorType::Pseudo:
        dwHash = FDE_GetCSSPseudoByEnum(ePseudoType)->dwHash;
        if (dwHash != pSel->GetNameHash()) {
          return false;
        }
        break;
      default:
        ASSERT(false);
        break;
    }
    pSel = pSel->GetNextSelector();
  }
  return !pSel && pCache;
}

void CFDE_CSSStyleSelector::ComputeStyle(
    CXFA_CSSTagProvider* pTag,
    const CFDE_CSSDeclaration** ppDeclArray,
    int32_t iDeclCount,
    IFDE_CSSComputedStyle* pDestStyle) {
  ASSERT(iDeclCount >= 0);
  ASSERT(pDestStyle);

  static const uint32_t s_dwStyleHash = FX_HashCode_GetW(L"style", true);
  static const uint32_t s_dwAlignHash = FX_HashCode_GetW(L"align", true);

  if (!pTag->empty()) {
    CFDE_CSSDeclaration* pDecl = nullptr;
    for (auto it : *pTag) {
      CFX_WideString wsAttri = it.first;
      CFX_WideString wsValue = it.second;
      uint32_t dwAttriHash = FX_HashCode_GetW(wsAttri.AsStringC(), true);
      if (dwAttriHash == s_dwStyleHash) {
        if (!pDecl)
          pDecl = new CFDE_CSSDeclaration;

        AppendInlineStyle(pDecl, wsValue.c_str(), wsValue.GetLength());
      } else if (dwAttriHash == s_dwAlignHash) {
        if (!pDecl)
          pDecl = new CFDE_CSSDeclaration;

        FDE_CSSPropertyArgs args;
        args.pStringCache = nullptr;
        args.pProperty = FDE_GetCSSPropertyByEnum(FDE_CSSProperty::TextAlign);
        pDecl->AddProperty(&args, wsValue.c_str(), wsValue.GetLength());
      }
    }

    if (pDecl) {
      CFX_ArrayTemplate<CFDE_CSSDeclaration*> decls;
      decls.SetSize(iDeclCount + 1);
      CFDE_CSSDeclaration** ppInline = decls.GetData();
      FXSYS_memcpy(ppInline, ppDeclArray,
                   iDeclCount * sizeof(CFDE_CSSDeclaration*));
      ppInline[iDeclCount++] = pDecl;
      ApplyDeclarations(true, const_cast<const CFDE_CSSDeclaration**>(ppInline),
                        iDeclCount, pDestStyle);
      ApplyDeclarations(false,
                        const_cast<const CFDE_CSSDeclaration**>(ppInline),
                        iDeclCount, pDestStyle);
      return;
    }
  }

  if (iDeclCount > 0) {
    ASSERT(ppDeclArray);

    ApplyDeclarations(true, ppDeclArray, iDeclCount, pDestStyle);
    ApplyDeclarations(false, ppDeclArray, iDeclCount, pDestStyle);
  }
}

void CFDE_CSSStyleSelector::ApplyDeclarations(
    bool bPriority,
    const CFDE_CSSDeclaration** ppDeclArray,
    int32_t iDeclCount,
    IFDE_CSSComputedStyle* pDestStyle) {
  CFDE_CSSComputedStyle* pComputedStyle =
      static_cast<CFDE_CSSComputedStyle*>(pDestStyle);
  IFDE_CSSValue* pVal;
  bool bImportant;
  int32_t i;
  if (bPriority) {
    IFDE_CSSValue* pLastest = nullptr;
    IFDE_CSSValue* pImportant = nullptr;
    for (i = 0; i < iDeclCount; ++i) {
      pVal = ppDeclArray[i]->GetProperty(FDE_CSSProperty::FontSize, bImportant);
      if (!pVal)
        continue;

      if (bImportant)
        pImportant = pVal;
      else
        pLastest = pVal;
    }
    if (pImportant) {
      ApplyProperty(FDE_CSSProperty::FontSize, pImportant, pComputedStyle);
    } else if (pLastest) {
      ApplyProperty(FDE_CSSProperty::FontSize, pLastest, pComputedStyle);
    }
  } else {
    CFX_ArrayTemplate<CFDE_CSSDeclaration*> importants;
    const CFDE_CSSDeclaration* pDecl = nullptr;
    FDE_CSSProperty eProp;
    FX_POSITION pos;
    for (i = 0; i < iDeclCount; ++i) {
      pDecl = ppDeclArray[i];
      pos = pDecl->GetStartPosition();
      while (pos) {
        pDecl->GetNextProperty(pos, eProp, pVal, bImportant);
        if (eProp == FDE_CSSProperty::FontSize) {
          continue;
        } else if (!bImportant) {
          ApplyProperty(eProp, pVal, pComputedStyle);
        } else if (importants.GetSize() == 0 ||
                   importants[importants.GetUpperBound()] != pDecl) {
          importants.Add(const_cast<CFDE_CSSDeclaration*>(pDecl));
        }
      }
    }
    iDeclCount = importants.GetSize();
    for (i = 0; i < iDeclCount; ++i) {
      pDecl = importants[i];
      pos = pDecl->GetStartPosition();
      while (pos) {
        pDecl->GetNextProperty(pos, eProp, pVal, bImportant);
        if (bImportant && eProp != FDE_CSSProperty::FontSize) {
          ApplyProperty(eProp, pVal, pComputedStyle);
        }
      }
    }
    CFX_WideString wsName, wsValue;
    pos = pDecl->GetStartCustom();
    while (pos) {
      pDecl->GetNextCustom(pos, wsName, wsValue);
      pComputedStyle->AddCustomStyle(wsName, wsValue);
    }
  }
}

void CFDE_CSSStyleSelector::AppendInlineStyle(CFDE_CSSDeclaration* pDecl,
                                              const FX_WCHAR* psz,
                                              int32_t iLen) {
  ASSERT(pDecl && psz && iLen > 0);
  auto pSyntax = pdfium::MakeUnique<CFDE_CSSSyntaxParser>();
  if (!pSyntax->Init(psz, iLen, 32, true))
    return;

  int32_t iLen2 = 0;
  const FX_WCHAR* psz2;
  FDE_CSSPropertyArgs args;
  args.pStringCache = nullptr;
  args.pProperty = nullptr;
  CFX_WideString wsName;
  while (1) {
    FDE_CSSSyntaxStatus eStatus = pSyntax->DoSyntaxParse();
    if (eStatus == FDE_CSSSyntaxStatus::PropertyName) {
      psz2 = pSyntax->GetCurrentString(iLen2);
      args.pProperty = FDE_GetCSSPropertyByName(CFX_WideStringC(psz2, iLen2));
      if (!args.pProperty)
        wsName = CFX_WideStringC(psz2, iLen2);
    } else if (eStatus == FDE_CSSSyntaxStatus::PropertyValue) {
      if (args.pProperty) {
        psz2 = pSyntax->GetCurrentString(iLen2);
        if (iLen2 > 0)
          pDecl->AddProperty(&args, psz2, iLen2);
      } else if (iLen2 > 0) {
        psz2 = pSyntax->GetCurrentString(iLen2);
        if (iLen2 > 0) {
          pDecl->AddProperty(&args, wsName.c_str(), wsName.GetLength(), psz2,
                             iLen2);
        }
      }
    } else {
      break;
    }
  }
}

void CFDE_CSSStyleSelector::ApplyProperty(
    FDE_CSSProperty eProperty,
    IFDE_CSSValue* pValue,
    CFDE_CSSComputedStyle* pComputedStyle) {
  if (pValue->GetType() == FDE_CSSVALUETYPE_Primitive) {
    IFDE_CSSPrimitiveValue* pPrimitive =
        static_cast<IFDE_CSSPrimitiveValue*>(pValue);
    FDE_CSSPrimitiveType eType = pPrimitive->GetPrimitiveType();
    switch (eProperty) {
      case FDE_CSSProperty::Display:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_NonInheritedData.m_eDisplay =
              ToDisplay(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::FontSize: {
        FX_FLOAT& fFontSize = pComputedStyle->m_InheritedData.m_fFontSize;
        if (eType >= FDE_CSSPrimitiveType::Number &&
            eType <= FDE_CSSPrimitiveType::Picas) {
          fFontSize = ApplyNumber(eType, pPrimitive->GetFloat(), fFontSize);
        } else if (eType == FDE_CSSPrimitiveType::Enum) {
          fFontSize = ToFontSize(pPrimitive->GetEnum(), fFontSize);
        }
      } break;
      case FDE_CSSProperty::LineHeight:
        if (eType == FDE_CSSPrimitiveType::Number) {
          pComputedStyle->m_InheritedData.m_fLineHeight =
              pPrimitive->GetFloat() *
              pComputedStyle->m_InheritedData.m_fFontSize;
        } else if (eType > FDE_CSSPrimitiveType::Number &&
                   eType <= FDE_CSSPrimitiveType::Picas) {
          pComputedStyle->m_InheritedData.m_fLineHeight =
              ApplyNumber(eType, pPrimitive->GetFloat(),
                          pComputedStyle->m_InheritedData.m_fFontSize);
        }
        break;
      case FDE_CSSProperty::TextAlign:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_eTextAlign =
              ToTextAlign(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::TextIndent:
        SetLengthWithPercent(pComputedStyle->m_InheritedData.m_TextIndent,
                             eType, pPrimitive,
                             pComputedStyle->m_InheritedData.m_fFontSize);
        break;
      case FDE_CSSProperty::FontWeight:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_wFontWeight =
              ToFontWeight(pPrimitive->GetEnum());
        } else if (eType == FDE_CSSPrimitiveType::Number) {
          int32_t iValue = (int32_t)pPrimitive->GetFloat() / 100;
          if (iValue >= 1 && iValue <= 9) {
            pComputedStyle->m_InheritedData.m_wFontWeight = iValue * 100;
          }
        }
        break;
      case FDE_CSSProperty::FontStyle:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_eFontStyle =
              ToFontStyle(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::Color:
        if (eType == FDE_CSSPrimitiveType::RGB) {
          pComputedStyle->m_InheritedData.m_dwFontColor =
              pPrimitive->GetRGBColor();
        }
        break;
      case FDE_CSSProperty::MarginLeft:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_MarginWidth.left, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasMargin = true;
        }
        break;
      case FDE_CSSProperty::MarginTop:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_MarginWidth.top, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasMargin = true;
        }
        break;
      case FDE_CSSProperty::MarginRight:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_MarginWidth.right, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasMargin = true;
        }
        break;
      case FDE_CSSProperty::MarginBottom:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_MarginWidth.bottom, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasMargin = true;
        }
        break;
      case FDE_CSSProperty::PaddingLeft:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_PaddingWidth.left, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasPadding = true;
        }
        break;
      case FDE_CSSProperty::PaddingTop:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_PaddingWidth.top, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasPadding = true;
        }
        break;
      case FDE_CSSProperty::PaddingRight:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_PaddingWidth.right, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasPadding = true;
        }
        break;
      case FDE_CSSProperty::PaddingBottom:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_PaddingWidth.bottom, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasPadding = true;
        }
        break;
      case FDE_CSSProperty::BorderLeftWidth:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_BorderWidth.left, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasBorder = true;
        }
        break;
      case FDE_CSSProperty::BorderTopWidth:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_BorderWidth.top, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasBorder = true;
        }
        break;
      case FDE_CSSProperty::BorderRightWidth:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_BorderWidth.right, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasBorder = true;
        }
        break;
      case FDE_CSSProperty::BorderBottomWidth:
        if (SetLengthWithPercent(
                pComputedStyle->m_NonInheritedData.m_BorderWidth.bottom, eType,
                pPrimitive, pComputedStyle->m_InheritedData.m_fFontSize)) {
          pComputedStyle->m_NonInheritedData.m_bHasBorder = true;
        }
        break;
      case FDE_CSSProperty::VerticalAlign:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_NonInheritedData.m_eVerticalAlign =
              ToVerticalAlign(pPrimitive->GetEnum());
        } else if (eType >= FDE_CSSPrimitiveType::Number &&
                   eType <= FDE_CSSPrimitiveType::Picas) {
          pComputedStyle->m_NonInheritedData.m_eVerticalAlign =
              FDE_CSSVerticalAlign::Number;
          pComputedStyle->m_NonInheritedData.m_fVerticalAlign =
              ApplyNumber(eType, pPrimitive->GetFloat(),
                          pComputedStyle->m_InheritedData.m_fFontSize);
        }
        break;
      case FDE_CSSProperty::FontVariant:
        if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_eFontVariant =
              ToFontVariant(pPrimitive->GetEnum());
        }
        break;
      case FDE_CSSProperty::LetterSpacing:
        if (eType == FDE_CSSPrimitiveType::Percent) {
          break;
        } else if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_LetterSpacing.Set(
              FDE_CSSLengthUnit::Normal);
        } else if (eType >= FDE_CSSPrimitiveType::Number &&
                   eType <= FDE_CSSPrimitiveType::Picas) {
          SetLengthWithPercent(pComputedStyle->m_InheritedData.m_LetterSpacing,
                               eType, pPrimitive,
                               pComputedStyle->m_InheritedData.m_fFontSize);
        }
        break;
      case FDE_CSSProperty::WordSpacing:
        if (eType == FDE_CSSPrimitiveType::Percent) {
          break;
        } else if (eType == FDE_CSSPrimitiveType::Enum) {
          pComputedStyle->m_InheritedData.m_WordSpacing.Set(
              FDE_CSSLengthUnit::Normal);
        } else if (eType >= FDE_CSSPrimitiveType::Number &&
                   eType <= FDE_CSSPrimitiveType::Picas) {
          SetLengthWithPercent(pComputedStyle->m_InheritedData.m_WordSpacing,
                               eType, pPrimitive,
                               pComputedStyle->m_InheritedData.m_fFontSize);
        }
        break;
      case FDE_CSSProperty::Top:
        SetLengthWithPercent(pComputedStyle->m_NonInheritedData.m_Top, eType,
                             pPrimitive,
                             pComputedStyle->m_InheritedData.m_fFontSize);
        break;
      case FDE_CSSProperty::Bottom:
        SetLengthWithPercent(pComputedStyle->m_NonInheritedData.m_Bottom, eType,
                             pPrimitive,
                             pComputedStyle->m_InheritedData.m_fFontSize);
        break;
      case FDE_CSSProperty::Left:
        SetLengthWithPercent(pComputedStyle->m_NonInheritedData.m_Left, eType,
                             pPrimitive,
                             pComputedStyle->m_InheritedData.m_fFontSize);
        break;
      case FDE_CSSProperty::Right:
        SetLengthWithPercent(pComputedStyle->m_NonInheritedData.m_Right, eType,
                             pPrimitive,
                             pComputedStyle->m_InheritedData.m_fFontSize);
        break;
      default:
        break;
    }
  } else if (pValue->GetType() == FDE_CSSVALUETYPE_List) {
    IFDE_CSSValueList* pList = static_cast<IFDE_CSSValueList*>(pValue);
    int32_t iCount = pList->CountValues();
    if (iCount > 0) {
      switch (eProperty) {
        case FDE_CSSProperty::FontFamily:
          pComputedStyle->m_InheritedData.m_pFontFamily = pList;
          break;
        case FDE_CSSProperty::TextDecoration:
          pComputedStyle->m_NonInheritedData.m_dwTextDecoration =
              ToTextDecoration(pList);
          break;
        default:
          break;
      }
    }
  } else {
    ASSERT(false);
  }
}

FX_FLOAT CFDE_CSSStyleSelector::ApplyNumber(FDE_CSSPrimitiveType eUnit,
                                            FX_FLOAT fValue,
                                            FX_FLOAT fPercentBase) {
  switch (eUnit) {
    case FDE_CSSPrimitiveType::Pixels:
    case FDE_CSSPrimitiveType::Number:
      return fValue * 72 / 96;
    case FDE_CSSPrimitiveType::EMS:
    case FDE_CSSPrimitiveType::EXS:
      return fValue * fPercentBase;
    case FDE_CSSPrimitiveType::Percent:
      return fValue * fPercentBase / 100.0f;
    case FDE_CSSPrimitiveType::CentiMeters:
      return fValue * 28.3464f;
    case FDE_CSSPrimitiveType::MilliMeters:
      return fValue * 2.8346f;
    case FDE_CSSPrimitiveType::Inches:
      return fValue * 72.0f;
    case FDE_CSSPrimitiveType::Picas:
      return fValue / 12.0f;
    case FDE_CSSPrimitiveType::Points:
    default:
      return fValue;
  }
}

FDE_CSSDisplay CFDE_CSSStyleSelector::ToDisplay(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Block:
      return FDE_CSSDisplay::Block;
    case FDE_CSSPropertyValue::None:
      return FDE_CSSDisplay::None;
    case FDE_CSSPropertyValue::ListItem:
      return FDE_CSSDisplay::ListItem;
    case FDE_CSSPropertyValue::InlineTable:
      return FDE_CSSDisplay::InlineTable;
    case FDE_CSSPropertyValue::InlineBlock:
      return FDE_CSSDisplay::InlineBlock;
    case FDE_CSSPropertyValue::Inline:
    default:
      return FDE_CSSDisplay::Inline;
  }
}

FDE_CSSTextAlign CFDE_CSSStyleSelector::ToTextAlign(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Center:
      return FDE_CSSTextAlign::Center;
    case FDE_CSSPropertyValue::Right:
      return FDE_CSSTextAlign::Right;
    case FDE_CSSPropertyValue::Justify:
      return FDE_CSSTextAlign::Justify;
    case FDE_CSSPropertyValue::Left:
    default:
      return FDE_CSSTextAlign::Left;
  }
}

uint16_t CFDE_CSSStyleSelector::ToFontWeight(FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Bold:
      return 700;
    case FDE_CSSPropertyValue::Bolder:
      return 900;
    case FDE_CSSPropertyValue::Lighter:
      return 200;
    case FDE_CSSPropertyValue::Normal:
    default:
      return 400;
  }
}

FDE_CSSFontStyle CFDE_CSSStyleSelector::ToFontStyle(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Italic:
    case FDE_CSSPropertyValue::Oblique:
      return FDE_CSSFontStyle::Italic;
    default:
      return FDE_CSSFontStyle::Normal;
  }
}

bool CFDE_CSSStyleSelector::SetLengthWithPercent(
    FDE_CSSLENGTH& width,
    FDE_CSSPrimitiveType eType,
    IFDE_CSSPrimitiveValue* pPrimitive,
    FX_FLOAT fFontSize) {
  if (eType == FDE_CSSPrimitiveType::Percent) {
    width.Set(FDE_CSSLengthUnit::Percent, pPrimitive->GetFloat() / 100.0f);
    return width.NonZero();
  } else if (eType >= FDE_CSSPrimitiveType::Number &&
             eType <= FDE_CSSPrimitiveType::Picas) {
    FX_FLOAT fValue = ApplyNumber(eType, pPrimitive->GetFloat(), fFontSize);
    width.Set(FDE_CSSLengthUnit::Point, fValue);
    return width.NonZero();
  } else if (eType == FDE_CSSPrimitiveType::Enum) {
    switch (pPrimitive->GetEnum()) {
      case FDE_CSSPropertyValue::Auto:
        width.Set(FDE_CSSLengthUnit::Auto);
        return true;
      case FDE_CSSPropertyValue::None:
        width.Set(FDE_CSSLengthUnit::None);
        return true;
      case FDE_CSSPropertyValue::Thin:
        width.Set(FDE_CSSLengthUnit::Point, 2);
        return true;
      case FDE_CSSPropertyValue::Medium:
        width.Set(FDE_CSSLengthUnit::Point, 3);
        return true;
      case FDE_CSSPropertyValue::Thick:
        width.Set(FDE_CSSLengthUnit::Point, 4);
        return true;
      default:
        return false;
    }
  }
  return false;
}

FX_FLOAT CFDE_CSSStyleSelector::ToFontSize(FDE_CSSPropertyValue eValue,
                                           FX_FLOAT fCurFontSize) {
  switch (eValue) {
    case FDE_CSSPropertyValue::XxSmall:
      return m_fDefFontSize / 1.2f / 1.2f / 1.2f;
    case FDE_CSSPropertyValue::XSmall:
      return m_fDefFontSize / 1.2f / 1.2f;
    case FDE_CSSPropertyValue::Small:
      return m_fDefFontSize / 1.2f;
    case FDE_CSSPropertyValue::Medium:
      return m_fDefFontSize;
    case FDE_CSSPropertyValue::Large:
      return m_fDefFontSize * 1.2f;
    case FDE_CSSPropertyValue::XLarge:
      return m_fDefFontSize * 1.2f * 1.2f;
    case FDE_CSSPropertyValue::XxLarge:
      return m_fDefFontSize * 1.2f * 1.2f * 1.2f;
    case FDE_CSSPropertyValue::Larger:
      return fCurFontSize * 1.2f;
    case FDE_CSSPropertyValue::Smaller:
      return fCurFontSize / 1.2f;
    default:
      return fCurFontSize;
  }
}

FDE_CSSVerticalAlign CFDE_CSSStyleSelector::ToVerticalAlign(
    FDE_CSSPropertyValue eValue) {
  switch (eValue) {
    case FDE_CSSPropertyValue::Middle:
      return FDE_CSSVerticalAlign::Middle;
    case FDE_CSSPropertyValue::Bottom:
      return FDE_CSSVerticalAlign::Bottom;
    case FDE_CSSPropertyValue::Super:
      return FDE_CSSVerticalAlign::Super;
    case FDE_CSSPropertyValue::Sub:
      return FDE_CSSVerticalAlign::Sub;
    case FDE_CSSPropertyValue::Top:
      return FDE_CSSVerticalAlign::Top;
    case FDE_CSSPropertyValue::TextTop:
      return FDE_CSSVerticalAlign::TextTop;
    case FDE_CSSPropertyValue::TextBottom:
      return FDE_CSSVerticalAlign::TextBottom;
    case FDE_CSSPropertyValue::Baseline:
    default:
      return FDE_CSSVerticalAlign::Baseline;
  }
}

uint32_t CFDE_CSSStyleSelector::ToTextDecoration(IFDE_CSSValueList* pValue) {
  uint32_t dwDecoration = 0;
  for (int32_t i = pValue->CountValues() - 1; i >= 0; --i) {
    IFDE_CSSPrimitiveValue* pPrimitive =
        static_cast<IFDE_CSSPrimitiveValue*>(pValue->GetValue(i));
    if (pPrimitive->GetPrimitiveType() == FDE_CSSPrimitiveType::Enum) {
      switch (pPrimitive->GetEnum()) {
        case FDE_CSSPropertyValue::Underline:
          dwDecoration |= FDE_CSSTEXTDECORATION_Underline;
          break;
        case FDE_CSSPropertyValue::LineThrough:
          dwDecoration |= FDE_CSSTEXTDECORATION_LineThrough;
          break;
        case FDE_CSSPropertyValue::Overline:
          dwDecoration |= FDE_CSSTEXTDECORATION_Overline;
          break;
        case FDE_CSSPropertyValue::Blink:
          dwDecoration |= FDE_CSSTEXTDECORATION_Blink;
          break;
        case FDE_CSSPropertyValue::Double:
          dwDecoration |= FDE_CSSTEXTDECORATION_Double;
          break;
        default:
          break;
      }
    }
  }
  return dwDecoration;
}

FDE_CSSFontVariant CFDE_CSSStyleSelector::ToFontVariant(
    FDE_CSSPropertyValue eValue) {
  return eValue == FDE_CSSPropertyValue::SmallCaps
             ? FDE_CSSFontVariant::SmallCaps
             : FDE_CSSFontVariant::Normal;
}

CFDE_CSSComputedStyle::CFDE_CSSComputedStyle() : m_dwRefCount(1) {}

CFDE_CSSComputedStyle::~CFDE_CSSComputedStyle() {}

uint32_t CFDE_CSSComputedStyle::Retain() {
  return ++m_dwRefCount;
}

uint32_t CFDE_CSSComputedStyle::Release() {
  uint32_t dwRefCount = --m_dwRefCount;
  if (dwRefCount == 0)
    delete this;
  return dwRefCount;
}

IFDE_CSSFontStyle* CFDE_CSSComputedStyle::GetFontStyles() {
  return static_cast<IFDE_CSSFontStyle*>(this);
}

IFDE_CSSBoundaryStyle* CFDE_CSSComputedStyle::GetBoundaryStyles() {
  return static_cast<IFDE_CSSBoundaryStyle*>(this);
}

IFDE_CSSPositionStyle* CFDE_CSSComputedStyle::GetPositionStyles() {
  return static_cast<IFDE_CSSPositionStyle*>(this);
}

IFDE_CSSParagraphStyle* CFDE_CSSComputedStyle::GetParagraphStyles() {
  return static_cast<IFDE_CSSParagraphStyle*>(this);
}

bool CFDE_CSSComputedStyle::GetCustomStyle(const CFX_WideStringC& wsName,
                                           CFX_WideString& wsValue) const {
  for (int32_t i = pdfium::CollectionSize<int32_t>(m_CustomProperties) - 2;
       i > -1; i -= 2) {
    if (wsName == m_CustomProperties[i]) {
      wsValue = m_CustomProperties[i + 1];
      return true;
    }
  }
  return false;
}

int32_t CFDE_CSSComputedStyle::CountFontFamilies() const {
  return m_InheritedData.m_pFontFamily
             ? m_InheritedData.m_pFontFamily->CountValues()
             : 0;
}

const FX_WCHAR* CFDE_CSSComputedStyle::GetFontFamily(int32_t index) const {
  return (static_cast<IFDE_CSSPrimitiveValue*>(
              m_InheritedData.m_pFontFamily->GetValue(index)))
      ->GetString(index);
}

uint16_t CFDE_CSSComputedStyle::GetFontWeight() const {
  return m_InheritedData.m_wFontWeight;
}

FDE_CSSFontVariant CFDE_CSSComputedStyle::GetFontVariant() const {
  return static_cast<FDE_CSSFontVariant>(m_InheritedData.m_eFontVariant);
}

FDE_CSSFontStyle CFDE_CSSComputedStyle::GetFontStyle() const {
  return static_cast<FDE_CSSFontStyle>(m_InheritedData.m_eFontStyle);
}

FX_FLOAT CFDE_CSSComputedStyle::GetFontSize() const {
  return m_InheritedData.m_fFontSize;
}

FX_ARGB CFDE_CSSComputedStyle::GetColor() const {
  return m_InheritedData.m_dwFontColor;
}

void CFDE_CSSComputedStyle::SetFontWeight(uint16_t wFontWeight) {
  m_InheritedData.m_wFontWeight = wFontWeight;
}

void CFDE_CSSComputedStyle::SetFontVariant(FDE_CSSFontVariant eFontVariant) {
  m_InheritedData.m_eFontVariant = eFontVariant;
}

void CFDE_CSSComputedStyle::SetFontStyle(FDE_CSSFontStyle eFontStyle) {
  m_InheritedData.m_eFontStyle = eFontStyle;
}

void CFDE_CSSComputedStyle::SetFontSize(FX_FLOAT fFontSize) {
  m_InheritedData.m_fFontSize = fFontSize;
}

void CFDE_CSSComputedStyle::SetColor(FX_ARGB dwFontColor) {
  m_InheritedData.m_dwFontColor = dwFontColor;
}

const FDE_CSSRECT* CFDE_CSSComputedStyle::GetBorderWidth() const {
  return m_NonInheritedData.m_bHasBorder ? &(m_NonInheritedData.m_BorderWidth)
                                         : nullptr;
}

const FDE_CSSRECT* CFDE_CSSComputedStyle::GetMarginWidth() const {
  return m_NonInheritedData.m_bHasMargin ? &(m_NonInheritedData.m_MarginWidth)
                                         : nullptr;
}

const FDE_CSSRECT* CFDE_CSSComputedStyle::GetPaddingWidth() const {
  return m_NonInheritedData.m_bHasPadding ? &(m_NonInheritedData.m_PaddingWidth)
                                          : nullptr;
}

void CFDE_CSSComputedStyle::SetMarginWidth(const FDE_CSSRECT& rect) {
  m_NonInheritedData.m_MarginWidth = rect;
  m_NonInheritedData.m_bHasMargin = true;
}

void CFDE_CSSComputedStyle::SetPaddingWidth(const FDE_CSSRECT& rect) {
  m_NonInheritedData.m_PaddingWidth = rect;
  m_NonInheritedData.m_bHasPadding = true;
}

FDE_CSSDisplay CFDE_CSSComputedStyle::GetDisplay() const {
  return static_cast<FDE_CSSDisplay>(m_NonInheritedData.m_eDisplay);
}

FX_FLOAT CFDE_CSSComputedStyle::GetLineHeight() const {
  return m_InheritedData.m_fLineHeight;
}

const FDE_CSSLENGTH& CFDE_CSSComputedStyle::GetTextIndent() const {
  return m_InheritedData.m_TextIndent;
}

FDE_CSSTextAlign CFDE_CSSComputedStyle::GetTextAlign() const {
  return static_cast<FDE_CSSTextAlign>(m_InheritedData.m_eTextAlign);
}

FDE_CSSVerticalAlign CFDE_CSSComputedStyle::GetVerticalAlign() const {
  return static_cast<FDE_CSSVerticalAlign>(m_NonInheritedData.m_eVerticalAlign);
}

FX_FLOAT CFDE_CSSComputedStyle::GetNumberVerticalAlign() const {
  return m_NonInheritedData.m_fVerticalAlign;
}

uint32_t CFDE_CSSComputedStyle::GetTextDecoration() const {
  return m_NonInheritedData.m_dwTextDecoration;
}

const FDE_CSSLENGTH& CFDE_CSSComputedStyle::GetLetterSpacing() const {
  return m_InheritedData.m_LetterSpacing;
}

void CFDE_CSSComputedStyle::SetLineHeight(FX_FLOAT fLineHeight) {
  m_InheritedData.m_fLineHeight = fLineHeight;
}

void CFDE_CSSComputedStyle::SetTextIndent(const FDE_CSSLENGTH& textIndent) {
  m_InheritedData.m_TextIndent = textIndent;
}

void CFDE_CSSComputedStyle::SetTextAlign(FDE_CSSTextAlign eTextAlign) {
  m_InheritedData.m_eTextAlign = eTextAlign;
}

void CFDE_CSSComputedStyle::SetNumberVerticalAlign(FX_FLOAT fAlign) {
  m_NonInheritedData.m_eVerticalAlign = FDE_CSSVerticalAlign::Number,
  m_NonInheritedData.m_fVerticalAlign = fAlign;
}

void CFDE_CSSComputedStyle::SetTextDecoration(uint32_t dwTextDecoration) {
  m_NonInheritedData.m_dwTextDecoration = dwTextDecoration;
}

void CFDE_CSSComputedStyle::SetLetterSpacing(
    const FDE_CSSLENGTH& letterSpacing) {
  m_InheritedData.m_LetterSpacing = letterSpacing;
}

void CFDE_CSSComputedStyle::AddCustomStyle(const CFX_WideString& wsName,
                                           const CFX_WideString& wsValue) {
  m_CustomProperties.push_back(wsName);
  m_CustomProperties.push_back(wsValue);
}

CFDE_CSSInheritedData::CFDE_CSSInheritedData()
    : m_LetterSpacing(FDE_CSSLengthUnit::Normal),
      m_WordSpacing(FDE_CSSLengthUnit::Normal),
      m_TextIndent(FDE_CSSLengthUnit::Point, 0),
      m_pFontFamily(nullptr),
      m_fFontSize(12.0f),
      m_fLineHeight(14.0f),
      m_dwFontColor(0xFF000000),
      m_wFontWeight(400),
      m_eFontVariant(FDE_CSSFontVariant::Normal),
      m_eFontStyle(FDE_CSSFontStyle::Normal),
      m_eTextAlign(FDE_CSSTextAlign::Left) {}

CFDE_CSSNonInheritedData::CFDE_CSSNonInheritedData()
    : m_MarginWidth(FDE_CSSLengthUnit::Point, 0),
      m_BorderWidth(FDE_CSSLengthUnit::Point, 0),
      m_PaddingWidth(FDE_CSSLengthUnit::Point, 0),
      m_fVerticalAlign(0.0f),
      m_eDisplay(FDE_CSSDisplay::Inline),
      m_eVerticalAlign(FDE_CSSVerticalAlign::Baseline),
      m_dwTextDecoration(0),
      m_bHasMargin(false),
      m_bHasBorder(false),
      m_bHasPadding(false) {}
