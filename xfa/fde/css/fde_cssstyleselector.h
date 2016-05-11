// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_FDE_CSSSTYLESELECTOR_H_
#define XFA_FDE_CSS_FDE_CSSSTYLESELECTOR_H_

#include <vector>

#include "core/fxcrt/include/fx_ext.h"
#include "xfa/fde/css/fde_css.h"
#include "xfa/fde/css/fde_csscache.h"
#include "xfa/fde/css/fde_cssdeclaration.h"
#include "xfa/fgas/crt/fgas_memory.h"
#include "xfa/fgas/crt/fgas_system.h"

class CFDE_CSSAccelerator;
class CFDE_CSSComputedStyle;
class CXFA_CSSTagProvider;

class FDE_CSSRuleData : public CFX_Target {
 public:
  FDE_CSSRuleData(CFDE_CSSSelector* pSel,
                  CFDE_CSSDeclaration* pDecl,
                  uint32_t dwPos);

  CFDE_CSSSelector* pSelector;
  CFDE_CSSDeclaration* pDeclaration;
  uint32_t dwPriority;
  FDE_CSSRuleData* pNext;
};

class CFDE_CSSRuleCollection : public CFX_Target {
 public:
  CFDE_CSSRuleCollection()
      : m_pStaticStore(nullptr),
        m_pUniversalRules(nullptr),
        m_pPersudoRules(nullptr),
        m_iSelectors(0) {}
  ~CFDE_CSSRuleCollection() { Clear(); }

  void AddRulesFrom(const CFDE_CSSStyleSheetArray& sheets,
                    uint32_t dwMediaList,
                    IFX_FontMgr* pFontMgr);
  void Clear();

  int32_t CountSelectors() const { return m_iSelectors; }
  FDE_CSSRuleData* GetIDRuleData(uint32_t dwIDHash) {
    void* pData;
    return m_IDRules.Lookup((void*)(uintptr_t)dwIDHash, pData)
               ? (FDE_CSSRuleData*)pData
               : NULL;
  }
  FDE_CSSRuleData* GetTagRuleData(uint32_t dwTagHasn) {
    void* pData;
    return m_TagRules.Lookup((void*)(uintptr_t)dwTagHasn, pData)
               ? (FDE_CSSRuleData*)pData
               : NULL;
  }
  FDE_CSSRuleData* GetClassRuleData(uint32_t dwIDHash) {
    void* pData;
    return m_ClassRules.Lookup((void*)(uintptr_t)dwIDHash, pData)
               ? (FDE_CSSRuleData*)pData
               : NULL;
  }
  FDE_CSSRuleData* GetUniversalRuleData() { return m_pUniversalRules; }
  FDE_CSSRuleData* GetPersudoRuleData() { return m_pPersudoRules; }

  IFX_MemoryAllocator* m_pStaticStore;

 protected:
  void AddRulesFrom(IFDE_CSSStyleSheet* pStyleSheet,
                    IFDE_CSSRule* pRule,
                    uint32_t dwMediaList,
                    IFX_FontMgr* pFontMgr);
  void AddRuleTo(CFX_MapPtrToPtr& map,
                 uint32_t dwKey,
                 CFDE_CSSSelector* pSel,
                 CFDE_CSSDeclaration* pDecl);
  FX_BOOL AddRuleTo(FDE_CSSRuleData*& pList, FDE_CSSRuleData* pData);
  FDE_CSSRuleData* NewRuleData(CFDE_CSSSelector* pSel,
                               CFDE_CSSDeclaration* pDecl);
  CFX_MapPtrToPtr m_IDRules;
  CFX_MapPtrToPtr m_TagRules;
  CFX_MapPtrToPtr m_ClassRules;
  FDE_CSSRuleData* m_pUniversalRules;
  FDE_CSSRuleData* m_pPersudoRules;
  int32_t m_iSelectors;
};

class CFDE_CSSStyleSelector : public CFX_Target {
 public:
  CFDE_CSSStyleSelector();
  ~CFDE_CSSStyleSelector() override;

  void SetFontMgr(IFX_FontMgr* pFontMgr);
  void SetDefFontSize(FX_FLOAT fFontSize);

  FX_BOOL SetStyleSheet(FDE_CSSSTYLESHEETGROUP eType,
                        IFDE_CSSStyleSheet* pSheet);
  FX_BOOL SetStyleSheets(FDE_CSSSTYLESHEETGROUP eType,
                         const CFDE_CSSStyleSheetArray* pArray);
  void SetStylePriority(FDE_CSSSTYLESHEETGROUP eType,
                        FDE_CSSSTYLESHEETPRIORITY ePriority);
  void UpdateStyleIndex(uint32_t dwMediaList);
  CFDE_CSSAccelerator* InitAccelerator();
  IFDE_CSSComputedStyle* CreateComputedStyle(
      IFDE_CSSComputedStyle* pParentStyle);
  int32_t MatchDeclarations(CXFA_CSSTagProvider* pTag,
                            CFDE_CSSDeclarationArray& matchedDecls,
                            FDE_CSSPERSUDO ePersudoType = FDE_CSSPERSUDO_NONE);
  void ComputeStyle(CXFA_CSSTagProvider* pTag,
                    const CFDE_CSSDeclaration** ppDeclArray,
                    int32_t iDeclCount,
                    IFDE_CSSComputedStyle* pDestStyle);

 protected:
  void Reset();
  void MatchRules(FDE_CSSTagCache* pCache,
                  FDE_CSSRuleData* pList,
                  FDE_CSSPERSUDO ePersudoType);
  FX_BOOL MatchSelector(FDE_CSSTagCache* pCache,
                        CFDE_CSSSelector* pSel,
                        FDE_CSSPERSUDO ePersudoType);
  void AppendInlineStyle(CFDE_CSSDeclaration* pDecl,
                         const FX_WCHAR* psz,
                         int32_t iLen);
  void ApplyDeclarations(FX_BOOL bPriority,
                         const CFDE_CSSDeclaration** ppDeclArray,
                         int32_t iDeclCount,
                         IFDE_CSSComputedStyle* pDestStyle);
  void ApplyProperty(FDE_CSSPROPERTY eProperty,
                     IFDE_CSSValue* pValue,
                     CFDE_CSSComputedStyle* pComputedStyle);

  FX_FLOAT ApplyNumber(FDE_CSSPRIMITIVETYPE eUnit,
                       FX_FLOAT fValue,
                       FX_FLOAT fPercentBase);
  FX_BOOL SetLengthWithPercent(FDE_CSSLENGTH& width,
                               FDE_CSSPRIMITIVETYPE eType,
                               IFDE_CSSPrimitiveValue* pPrimitive,
                               FX_FLOAT fFontSize);
  FX_FLOAT ToFontSize(FDE_CSSPROPERTYVALUE eValue, FX_FLOAT fCurFontSize);
  FDE_CSSDISPLAY ToDisplay(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSTEXTALIGN ToTextAlign(FDE_CSSPROPERTYVALUE eValue);
  uint16_t ToFontWeight(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSFONTSTYLE ToFontStyle(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSBORDERSTYLE ToBorderStyle(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSVERTICALALIGN ToVerticalAlign(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSLISTSTYLETYPE ToListStyleType(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSLISTSTYLEPOSITION ToListStylePosition(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSVISIBILITY ToVisibility(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSWHITESPACE ToWhiteSpace(FDE_CSSPROPERTYVALUE eValue);
  uint32_t ToTextDecoration(IFDE_CSSValueList* pList);
  FDE_CSSTEXTTRANSFORM ToTextTransform(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSFONTVARIANT ToFontVariant(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSFLOAT ToFloat(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSCLEAR ToClear(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSWRITINGMODE ToWritingMode(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSWORDBREAK ToWordBreak(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSPAGEBREAK ToPageBreak(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSOVERFLOW ToOverflow(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSLINEBREAK ToLineBreak(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSTEXTCOMBINE ToTextCombine(FDE_CSSPROPERTYVALUE eValue);
  FX_BOOL ToTextEmphasisMark(FDE_CSSPROPERTYVALUE eValue,
                             FDE_CSSTEXTEMPHASISMARK& eMark);
  FX_BOOL ToTextEmphasisFill(FDE_CSSPROPERTYVALUE eValue,
                             FDE_CSSTEXTEMPHASISFILL& eFill);
  FDE_CSSCURSOR ToCursor(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSPOSITION ToPosition(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSCAPTIONSIDE ToCaptionSide(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSBKGREPEAT ToBKGRepeat(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSBKGATTACHMENT ToBKGAttachment(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSRUBYALIGN ToRubyAlign(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSRUBYOVERHANG ToRubyOverhang(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSRUBYPOSITION ToRubyPosition(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSRUBYSPAN ToRubySpan(FDE_CSSPROPERTYVALUE eValue);

  IFX_FontMgr* m_pFontMgr;
  FX_FLOAT m_fDefFontSize;
  IFX_MemoryAllocator* m_pRuleDataStore;
  CFDE_CSSStyleSheetArray m_SheetGroups[FDE_CSSSTYLESHEETGROUP_MAX];
  CFDE_CSSRuleCollection m_RuleCollection[FDE_CSSSTYLESHEETGROUP_MAX];
  FDE_CSSSTYLESHEETGROUP m_ePriorities[FDE_CSSSTYLESHEETPRIORITY_MAX];
  IFX_MemoryAllocator* m_pInlineStyleStore;
  IFX_MemoryAllocator* m_pFixedStyleStore;
  CFDE_CSSAccelerator* m_pAccelerator;
  std::vector<FDE_CSSRuleData*> m_MatchedRules;
};

struct FDE_CSSCOUNTERDATA {
 public:
  FDE_CSSCOUNTERDATA() { FXSYS_memset(this, 0, sizeof(FDE_CSSCOUNTERDATA)); }
  FX_BOOL GetCounterIncrement(int32_t& iValue) {
    iValue = m_iIncVal;
    return m_bIncrement;
  }
  FX_BOOL GetCounterReset(int32_t& iValue) {
    iValue = m_iResetVal;
    return m_bReset;
  }

  const FX_WCHAR* m_pszIdent;
  FX_BOOL m_bIncrement;
  FX_BOOL m_bReset;
  int32_t m_iIncVal;
  int32_t m_iResetVal;
};

class CFDE_CSSCounterStyle {
 public:
  CFDE_CSSCounterStyle() : m_pCounterInc(nullptr), m_pCounterReset(nullptr) {}

  void SetCounterIncrementList(IFDE_CSSValueList* pList) {
    m_pCounterInc = pList;
    m_bIndexDirty = TRUE;
  }

  void SetCounterResetList(IFDE_CSSValueList* pList) {
    m_pCounterReset = pList;
    m_bIndexDirty = TRUE;
  }

  int32_t CountCounters() {
    UpdateIndex();
    return m_arrCounterData.GetSize();
  }

  FX_BOOL GetCounterIncrement(int32_t index, int32_t& iValue) {
    UpdateIndex();
    return m_arrCounterData.ElementAt(index).GetCounterIncrement(iValue);
  }

  FX_BOOL GetCounterReset(int32_t index, int32_t& iValue) {
    UpdateIndex();
    return m_arrCounterData.ElementAt(index).GetCounterReset(iValue);
  }

  const FX_WCHAR* GetCounterIdentifier(int32_t index) {
    UpdateIndex();
    return m_arrCounterData.ElementAt(index).m_pszIdent;
  }

 protected:
  void UpdateIndex();
  void DoUpdateIndex(IFDE_CSSValueList* pList);
  int32_t FindIndex(const FX_WCHAR* pszIdentifier);

  IFDE_CSSValueList* m_pCounterInc;
  IFDE_CSSValueList* m_pCounterReset;
  CFX_ArrayTemplate<FDE_CSSCOUNTERDATA> m_arrCounterData;
  FX_BOOL m_bIndexDirty;
};

class CFDE_CSSInheritedData {
 public:
  void Reset() {
    FXSYS_memset(this, 0, sizeof(CFDE_CSSInheritedData));
    m_LetterSpacing.Set(FDE_CSSLENGTHUNIT_Normal);
    m_WordSpacing.Set(FDE_CSSLENGTHUNIT_Normal);
    m_TextIndent.Set(FDE_CSSLENGTHUNIT_Point, 0);
    m_fFontSize = 12.0f;
    m_fLineHeight = 14.0f;
    m_wFontWeight = 400;
    m_dwFontColor = 0xFF000000;
    m_iWidows = 2;
    m_bTextEmphasisColorCurrent = TRUE;
    m_iOrphans = 2;
  }

  const FX_WCHAR* m_pszListStyleImage;
  FDE_CSSLENGTH m_LetterSpacing;
  FDE_CSSLENGTH m_WordSpacing;
  FDE_CSSLENGTH m_TextIndent;
  IFDE_CSSValueList* m_pFontFamily;
  IFDE_CSSValueList* m_pQuotes;
  IFDE_CSSValueList* m_pCursorUris;
  FDE_CSSCURSOR m_eCursor;
  FX_FLOAT m_fFontSize;
  FX_FLOAT m_fLineHeight;
  FX_ARGB m_dwFontColor;
  FX_ARGB m_dwTextEmphasisColor;
  uint16_t m_wFontWeight;
  int32_t m_iWidows;
  int32_t m_iOrphans;
  const FX_WCHAR* m_pszTextEmphasisCustomMark;
  uint32_t m_eFontVariant : 1;
  uint32_t m_eFontStyle : 1;
  uint32_t m_bTextEmphasisColorCurrent : 1;
  uint32_t m_eTextAligh : 2;
  uint32_t m_eVisibility : 2;
  uint32_t m_eWhiteSpace : 3;
  uint32_t m_eTextTransform : 2;
  uint32_t m_eWritingMode : 2;
  uint32_t m_eWordBreak : 2;
  uint32_t m_eLineBreak : 2;
  uint32_t m_eTextEmphasisFill : 1;
  uint32_t m_eTextEmphasisMark : 3;
  uint32_t m_eCaptionSide : 3;
  uint8_t m_eRubyAlign : 4;
  uint8_t m_eRubyOverhang : 2;
  uint8_t m_eRubyPosition : 2;
};

class CFDE_CSSNonInheritedData {
 public:
  void Reset() {
    FXSYS_memset(this, 0, sizeof(CFDE_CSSNonInheritedData));
    m_MarginWidth = m_BorderWidth =
        m_PaddingWidth.Set(FDE_CSSLENGTHUNIT_Point, 0);
    m_MinBoxSize.Set(FDE_CSSLENGTHUNIT_Point, 0);
    m_MaxBoxSize.Set(FDE_CSSLENGTHUNIT_None);
    m_eDisplay = FDE_CSSDISPLAY_Inline;
    m_fVerticalAlign = 0.0f;
    m_ColumnCount.Set(FDE_CSSLENGTHUNIT_Auto);
    m_ColumnGap.Set(FDE_CSSLENGTHUNIT_Normal);
    m_bColumnRuleColorSame = TRUE;
    m_ColumnWidth.Set(FDE_CSSLENGTHUNIT_Auto);
    m_ColumnRuleWidth.Set(FDE_CSSLENGTHUNIT_Auto);
    m_eTextCombine = FDE_CSSTEXTCOMBINE_None;
  }

  IFDE_CSSValueList* m_pContentList;
  CFDE_CSSCounterStyle* m_pCounterStyle;
  FDE_CSSRECT m_MarginWidth;
  FDE_CSSRECT m_BorderWidth;
  FDE_CSSRECT m_PaddingWidth;
  FDE_CSSSIZE m_BoxSize;
  FDE_CSSSIZE m_MinBoxSize;
  FDE_CSSSIZE m_MaxBoxSize;
  FDE_CSSPOINT m_BKGPosition;
  const FX_WCHAR* m_pszBKGImage;
  FX_ARGB m_dwBKGColor;
  FX_ARGB m_dwBDRLeftColor;
  FX_ARGB m_dwBDRTopColor;
  FX_ARGB m_dwBDRRightColor;
  FX_ARGB m_dwBDRBottomColor;
  IFDE_CSSValue* m_pRubySpan;
  FDE_CSSLENGTH m_ColumnCount;
  FDE_CSSLENGTH m_ColumnGap;
  FDE_CSSLENGTH m_ColumnRuleWidth;
  FDE_CSSLENGTH m_ColumnWidth;
  FX_ARGB m_dwColumnRuleColor;
  FDE_CSSLENGTH m_Top;
  FDE_CSSLENGTH m_Bottom;
  FDE_CSSLENGTH m_Left;
  FDE_CSSLENGTH m_Right;

  FX_FLOAT m_fVerticalAlign;
  FX_FLOAT m_fTextCombineNumber;
  uint32_t m_eBDRLeftStyle : 4;
  uint32_t m_eBDRTopStyle : 4;
  uint32_t m_eBDRRightStyle : 4;
  uint32_t m_eBDRBottomStyle : 4;
  uint32_t m_eDisplay : 5;
  uint32_t m_eVerticalAlign : 4;
  uint32_t m_eListStyleType : 5;
  uint32_t m_eColumnRuleStyle : 4;
  uint32_t m_ePageBreakInside : 3;
  uint32_t m_ePageBreakAfter : 3;
  uint32_t m_ePageBreakBefore : 3;
  uint32_t m_ePosition : 2;
  uint32_t m_eBKGRepeat : 2;
  uint32_t m_eFloat : 2;
  uint32_t m_eClear : 2;
  uint32_t m_eOverflowX : 3;
  uint32_t m_eOverflowY : 3;
  uint32_t m_eListStylePosition : 1;
  uint32_t m_eBKGAttachment : 1;
  uint32_t m_bHasMargin : 1;
  uint32_t m_bHasBorder : 1;
  uint32_t m_bHasPadding : 1;
  uint32_t m_dwTextDecoration : 5;
  uint32_t m_eTextCombine : 1;
  uint32_t m_bColumnRuleColorSame : 1;
  uint32_t m_bHasTextCombineNumber : 1;
};

class CFDE_CSSComputedStyle : public IFDE_CSSComputedStyle,
                              public IFDE_CSSBoundaryStyle,
                              public IFDE_CSSFontStyle,
                              public IFDE_CSSPositionStyle,
                              public IFDE_CSSParagraphStyle,
                              public CFX_Target {
 public:
  CFDE_CSSComputedStyle(IFX_MemoryAllocator* pAlloc)
      : m_dwRefCount(1), m_pAllocator(pAlloc) {}

  ~CFDE_CSSComputedStyle() {}

  // IFX_Unknown:
  uint32_t AddRef() override { return ++m_dwRefCount; }

  uint32_t Release() override {
    uint32_t dwRefCount = --m_dwRefCount;
    if (dwRefCount == 0) {
      delete m_NonInheritedData.m_pCounterStyle;
      FXTARGET_DeleteWith(CFDE_CSSComputedStyle, m_pAllocator, this);
    }
    return dwRefCount;
  }

  // IFDE_CSSComputedStyle:
  void Reset() override {
    m_InheritedData.Reset();
    m_NonInheritedData.Reset();
  }

  IFDE_CSSFontStyle* GetFontStyles() override {
    return static_cast<IFDE_CSSFontStyle*>(this);
  }

  IFDE_CSSBoundaryStyle* GetBoundaryStyles() override {
    return static_cast<IFDE_CSSBoundaryStyle*>(this);
  }

  IFDE_CSSPositionStyle* GetPositionStyles() override {
    return static_cast<IFDE_CSSPositionStyle*>(this);
  }

  IFDE_CSSParagraphStyle* GetParagraphStyles() override {
    return static_cast<IFDE_CSSParagraphStyle*>(this);
  }

  FX_BOOL GetCustomStyle(const CFX_WideStringC& wsName,
                         CFX_WideString& wsValue) const override {
    for (int32_t i = m_CustomProperties.GetSize() - 2; i > -1; i -= 2) {
      if (wsName == m_CustomProperties[i]) {
        wsValue = m_CustomProperties[i + 1];
        return TRUE;
      }
    }
    return FALSE;
  }

  // IFDE_CSSFontStyle:
  int32_t CountFontFamilies() const override {
    return m_InheritedData.m_pFontFamily
               ? m_InheritedData.m_pFontFamily->CountValues()
               : 0;
  }

  const FX_WCHAR* GetFontFamily(int32_t index) const override {
    return (static_cast<IFDE_CSSPrimitiveValue*>(
                m_InheritedData.m_pFontFamily->GetValue(index)))
        ->GetString(index);
  }

  uint16_t GetFontWeight() const override {
    return m_InheritedData.m_wFontWeight;
  }

  FDE_CSSFONTVARIANT GetFontVariant() const override {
    return static_cast<FDE_CSSFONTVARIANT>(m_InheritedData.m_eFontVariant);
  }

  FDE_CSSFONTSTYLE GetFontStyle() const override {
    return static_cast<FDE_CSSFONTSTYLE>(m_InheritedData.m_eFontStyle);
  }

  FX_FLOAT GetFontSize() const override { return m_InheritedData.m_fFontSize; }

  FX_ARGB GetColor() const override { return m_InheritedData.m_dwFontColor; }

  void SetFontWeight(uint16_t wFontWeight) override {
    m_InheritedData.m_wFontWeight = wFontWeight;
  }

  void SetFontVariant(FDE_CSSFONTVARIANT eFontVariant) override {
    m_InheritedData.m_eFontVariant = eFontVariant;
  }

  void SetFontStyle(FDE_CSSFONTSTYLE eFontStyle) override {
    m_InheritedData.m_eFontStyle = eFontStyle;
  }

  void SetFontSize(FX_FLOAT fFontSize) override {
    m_InheritedData.m_fFontSize = fFontSize;
  }

  void SetColor(FX_ARGB dwFontColor) override {
    m_InheritedData.m_dwFontColor = dwFontColor;
  }

  // IFDE_CSSBoundaryStyle:
  const FDE_CSSRECT* GetBorderWidth() const override {
    return m_NonInheritedData.m_bHasBorder ? &(m_NonInheritedData.m_BorderWidth)
                                           : nullptr;
  }

  const FDE_CSSRECT* GetMarginWidth() const override {
    return m_NonInheritedData.m_bHasMargin ? &(m_NonInheritedData.m_MarginWidth)
                                           : nullptr;
  }

  const FDE_CSSRECT* GetPaddingWidth() const override {
    return m_NonInheritedData.m_bHasPadding
               ? &(m_NonInheritedData.m_PaddingWidth)
               : nullptr;
  }

  void SetMarginWidth(const FDE_CSSRECT& rect) override {
    m_NonInheritedData.m_MarginWidth = rect;
    m_NonInheritedData.m_bHasMargin = TRUE;
  }

  void SetPaddingWidth(const FDE_CSSRECT& rect) override {
    m_NonInheritedData.m_PaddingWidth = rect;
    m_NonInheritedData.m_bHasPadding = TRUE;
  }

  // IFDE_CSSPositionStyle:
  FDE_CSSDISPLAY GetDisplay() const override {
    return static_cast<FDE_CSSDISPLAY>(m_NonInheritedData.m_eDisplay);
  }

  // IFDE_CSSParagraphStyle:
  FX_FLOAT GetLineHeight() const override {
    return m_InheritedData.m_fLineHeight;
  }

  const FDE_CSSLENGTH& GetTextIndent() const override {
    return m_InheritedData.m_TextIndent;
  }

  FDE_CSSTEXTALIGN GetTextAlign() const override {
    return static_cast<FDE_CSSTEXTALIGN>(m_InheritedData.m_eTextAligh);
  }

  FDE_CSSVERTICALALIGN GetVerticalAlign() const override {
    return static_cast<FDE_CSSVERTICALALIGN>(
        m_NonInheritedData.m_eVerticalAlign);
  }

  FX_FLOAT GetNumberVerticalAlign() const override {
    return m_NonInheritedData.m_fVerticalAlign;
  }

  uint32_t GetTextDecoration() const override {
    return m_NonInheritedData.m_dwTextDecoration;
  }

  const FDE_CSSLENGTH& GetLetterSpacing() const override {
    return m_InheritedData.m_LetterSpacing;
  }

  void SetLineHeight(FX_FLOAT fLineHeight) override {
    m_InheritedData.m_fLineHeight = fLineHeight;
  }

  void SetTextIndent(const FDE_CSSLENGTH& textIndent) override {
    m_InheritedData.m_TextIndent = textIndent;
  }

  void SetTextAlign(FDE_CSSTEXTALIGN eTextAlign) override {
    m_InheritedData.m_eTextAligh = eTextAlign;
  }

  void SetNumberVerticalAlign(FX_FLOAT fAlign) override {
    m_NonInheritedData.m_eVerticalAlign = FDE_CSSVERTICALALIGN_Number,
    m_NonInheritedData.m_fVerticalAlign = fAlign;
  }

  void SetTextDecoration(uint32_t dwTextDecoration) override {
    m_NonInheritedData.m_dwTextDecoration = dwTextDecoration;
  }

  void SetLetterSpacing(const FDE_CSSLENGTH& letterSpacing) override {
    m_InheritedData.m_LetterSpacing = letterSpacing;
  }

  void AddCustomStyle(const CFX_WideString& wsName,
                      const CFX_WideString& wsValue) {
    m_CustomProperties.Add(wsName);
    m_CustomProperties.Add(wsValue);
  }

  uint32_t m_dwRefCount;
  IFX_MemoryAllocator* m_pAllocator;
  CFDE_CSSInheritedData m_InheritedData;
  CFDE_CSSNonInheritedData m_NonInheritedData;
  CFX_WideStringArray m_CustomProperties;
};

#endif  // XFA_FDE_CSS_FDE_CSSSTYLESELECTOR_H_
