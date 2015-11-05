// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_CSSSTYLESELECTOR
#define _FDE_CSSSTYLESELECTOR
#define FDE_CSSUNIVERSALHASH ('*')
typedef struct _FDE_CSSRULEDATA : public CFX_Target {
 public:
  _FDE_CSSRULEDATA(IFDE_CSSSelector* pSel,
                   IFDE_CSSDeclaration* pDecl,
                   FX_DWORD dwPos);
  IFDE_CSSSelector* pSelector;
  IFDE_CSSDeclaration* pDeclaration;
  FX_DWORD dwPriority;
  _FDE_CSSRULEDATA* pNext;
} FDE_CSSRULEDATA, *FDE_LPCSSRULEDATA;
typedef CFX_ArrayTemplate<FDE_LPCSSRULEDATA> CFDE_CSSRuleDataArray;
class CFDE_CSSRuleCollection : public CFX_Target {
 public:
  CFDE_CSSRuleCollection()
      : m_pStaticStore(nullptr),
        m_pUniversalRules(nullptr),
        m_pPersudoRules(nullptr),
        m_iSelectors(0) {}
  ~CFDE_CSSRuleCollection() { Clear(); }
  void AddRulesFrom(const CFDE_CSSStyleSheetArray& sheets,
                    FX_DWORD dwMediaList,
                    IFX_FontMgr* pFontMgr);
  void Clear();

  int32_t CountSelectors() const { return m_iSelectors; }
  FDE_LPCSSRULEDATA GetIDRuleData(FX_DWORD dwIDHash) {
    void* pData;
    return m_IDRules.Lookup((void*)(uintptr_t)dwIDHash, pData)
               ? (FDE_LPCSSRULEDATA)pData
               : NULL;
  }
  FDE_LPCSSRULEDATA GetTagRuleData(FX_DWORD dwTagHasn) {
    void* pData;
    return m_TagRules.Lookup((void*)(uintptr_t)dwTagHasn, pData)
               ? (FDE_LPCSSRULEDATA)pData
               : NULL;
  }
  FDE_LPCSSRULEDATA GetClassRuleData(FX_DWORD dwIDHash) {
    void* pData;
    return m_ClassRules.Lookup((void*)(uintptr_t)dwIDHash, pData)
               ? (FDE_LPCSSRULEDATA)pData
               : NULL;
  }
  FDE_LPCSSRULEDATA GetUniversalRuleData() { return m_pUniversalRules; }
  FDE_LPCSSRULEDATA GetPersudoRuleData() { return m_pPersudoRules; }
  IFX_MEMAllocator* m_pStaticStore;

 protected:
  void AddRulesFrom(IFDE_CSSStyleSheet* pStyleSheet,
                    IFDE_CSSRule* pRule,
                    FX_DWORD dwMediaList,
                    IFX_FontMgr* pFontMgr);
  void AddRuleTo(CFX_MapPtrToPtr& map,
                 FX_DWORD dwKey,
                 IFDE_CSSSelector* pSel,
                 IFDE_CSSDeclaration* pDecl);
  FX_BOOL AddRuleTo(FDE_LPCSSRULEDATA& pList, FDE_LPCSSRULEDATA pData);
  FDE_LPCSSRULEDATA NewRuleData(IFDE_CSSSelector* pSel,
                                IFDE_CSSDeclaration* pDecl);
  CFX_MapPtrToPtr m_IDRules;
  CFX_MapPtrToPtr m_TagRules;
  CFX_MapPtrToPtr m_ClassRules;
  FDE_LPCSSRULEDATA m_pUniversalRules;
  FDE_LPCSSRULEDATA m_pPersudoRules;
  int32_t m_iSelectors;
};
class CFDE_CSSAccelerator;
class CFDE_CSSComputedStyle;
class CFDE_CSSStyleSelector : public IFDE_CSSStyleSelector, public CFX_Target {
 public:
  CFDE_CSSStyleSelector();
  ~CFDE_CSSStyleSelector();
  virtual void Release() { delete this; }

  virtual void SetFontMgr(IFX_FontMgr* pFontMgr);
  virtual void SetDefFontSize(FX_FLOAT fFontSize);

  virtual FX_BOOL SetStyleSheet(FDE_CSSSTYLESHEETGROUP eType,
                                IFDE_CSSStyleSheet* pSheet);
  virtual FX_BOOL SetStyleSheets(FDE_CSSSTYLESHEETGROUP eType,
                                 const CFDE_CSSStyleSheetArray* pArray);
  virtual void SetStylePriority(FDE_CSSSTYLESHEETGROUP eType,
                                FDE_CSSSTYLESHEETPRIORITY ePriority);
  virtual void UpdateStyleIndex(FX_DWORD dwMediaList);
  virtual IFDE_CSSAccelerator* InitAccelerator();
  virtual IFDE_CSSComputedStyle* CreateComputedStyle(
      IFDE_CSSComputedStyle* pParentStyle);
  virtual int32_t MatchDeclarations(
      IFDE_CSSTagProvider* pTag,
      CFDE_CSSDeclarationArray& matchedDecls,
      FDE_CSSPERSUDO ePersudoType = FDE_CSSPERSUDO_NONE);
  virtual void ComputeStyle(IFDE_CSSTagProvider* pTag,
                            const IFDE_CSSDeclaration** ppDeclArray,
                            int32_t iDeclCount,
                            IFDE_CSSComputedStyle* pDestStyle);

 protected:
  void Reset();
  void MatchRules(FDE_LPCSSTAGCACHE pCache,
                  FDE_LPCSSRULEDATA pList,
                  FDE_CSSPERSUDO ePersudoType);
  void SortRulesTo(CFDE_CSSDeclarationArray& matchDecls);
  FX_BOOL MatchSelector(FDE_LPCSSTAGCACHE pCache,
                        IFDE_CSSSelector* pSel,
                        FDE_CSSPERSUDO ePersudoType);
  void AppendInlineStyle(CFDE_CSSDeclaration* pDecl,
                         const FX_WCHAR* psz,
                         int32_t iLen);
  void ApplyDeclarations(FX_BOOL bPriority,
                         const IFDE_CSSDeclaration** ppDeclArray,
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
  FX_WORD ToFontWeight(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSFONTSTYLE ToFontStyle(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSBORDERSTYLE ToBorderStyle(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSVERTICALALIGN ToVerticalAlign(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSLISTSTYLETYPE ToListStyleType(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSLISTSTYLEPOSITION ToListStylePosition(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSVISIBILITY ToVisibility(FDE_CSSPROPERTYVALUE eValue);
  FDE_CSSWHITESPACE ToWhiteSpace(FDE_CSSPROPERTYVALUE eValue);
  FX_DWORD ToTextDecoration(IFDE_CSSValueList* pList);
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
  IFX_MEMAllocator* m_pRuleDataStore;
  CFDE_CSSStyleSheetArray m_SheetGroups[FDE_CSSSTYLESHEETGROUP_MAX];
  CFDE_CSSRuleCollection m_RuleCollection[FDE_CSSSTYLESHEETGROUP_MAX];
  FDE_CSSSTYLESHEETGROUP m_ePriorities[FDE_CSSSTYLESHEETPRIORITY_MAX];
  IFX_MEMAllocator* m_pInlineStyleStore;
  IFX_MEMAllocator* m_pFixedStyleStore;
  CFDE_CSSAccelerator* m_pAccelerator;
  CFDE_CSSRuleDataArray m_MatchedRules;
};
typedef struct _FDE_CSSCOUNTERDATA {
 public:
  _FDE_CSSCOUNTERDATA() { FX_memset(this, 0, sizeof(_FDE_CSSCOUNTERDATA)); }
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
} FDE_CSSCOUNTERDATA, *FDE_LPCSSCOUNTERDATA;
class CFDE_CSSCounterStyle {
 public:
  CFDE_CSSCounterStyle() : m_pCounterInc(NULL), m_pCounterReset(NULL) {}
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
    FX_memset(this, 0, sizeof(CFDE_CSSInheritedData));
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
  FX_WORD m_wFontWeight;
  int32_t m_iWidows;
  int32_t m_iOrphans;
  const FX_WCHAR* m_pszTextEmphasisCustomMark;
  FX_WORD m_eFontVariant : 1;
  FX_WORD m_eFontStyle : 1;
  FX_WORD m_bTextEmphasisColorCurrent : 1;
  FX_WORD m_eTextAligh : 2;
  FX_WORD m_eVisibility : 2;
  FX_WORD m_eWhiteSpace : 3;
  FX_WORD m_eTextTransform : 2;
  FX_WORD m_eWritingMode : 2;
  FX_WORD m_eWordBreak : 2;
  FX_WORD m_eLineBreak : 2;
  FX_WORD m_eTextEmphasisFill : 1;
  FX_WORD m_eTextEmphasisMark : 3;
  FX_WORD m_eCaptionSide : 3;

  FX_WORD m_eRubyAlign : 4;
  FX_WORD m_eRubyOverhang : 2;
  FX_WORD m_eRubyPosition : 2;
};
class CFDE_CSSNonInheritedData {
 public:
  void Reset() {
    FX_memset(this, 0, sizeof(CFDE_CSSNonInheritedData));
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
  FX_DWORD m_eBDRLeftStyle : 4;
  FX_DWORD m_eBDRTopStyle : 4;
  FX_DWORD m_eBDRRightStyle : 4;
  FX_DWORD m_eBDRBottomStyle : 4;
  FX_DWORD m_eDisplay : 5;
  FX_DWORD m_eVerticalAlign : 4;
  FX_DWORD m_eListStyleType : 5;
  FX_DWORD m_eColumnRuleStyle : 4;
  FX_DWORD m_ePageBreakInside : 3;
  FX_DWORD m_ePageBreakAfter : 3;
  FX_DWORD m_ePageBreakBefore : 3;
  FX_DWORD m_ePosition : 2;
  FX_DWORD m_eBKGRepeat : 2;
  FX_DWORD m_eFloat : 2;
  FX_DWORD m_eClear : 2;
  FX_DWORD m_eOverflowX : 3;
  FX_DWORD m_eOverflowY : 3;
  FX_DWORD m_eListStylePosition : 1;
  FX_DWORD m_eBKGAttachment : 1;
  FX_DWORD m_bHasMargin : 1;
  FX_DWORD m_bHasBorder : 1;
  FX_DWORD m_bHasPadding : 1;
  FX_DWORD m_dwTextDecoration : 5;
  FX_DWORD m_eTextCombine : 1;
  FX_DWORD m_bColumnRuleColorSame : 1;
  FX_DWORD m_bHasTextCombineNumber : 1;
};
class CFDE_CSSComputedStyle : public IFDE_CSSComputedStyle,
                              public IFDE_CSSFontStyle,
                              public IFDE_CSSBoundaryStyle,
                              public IFDE_CSSPositionStyle,
                              public IFDE_CSSParagraphStyle,
                              public IFDE_CSSBackgroundStyle,
                              public IFDE_CSSVisualStyle,
                              public IFDE_CSSListStyle,
                              public IFDE_CSSMultiColumnStyle,
                              public IFDE_CSSGeneratedContentStyle,
                              public IFDE_CSSTableStyle,
                              public IFDE_CSSRubyStyle,
                              public CFX_Target {
 public:
  CFDE_CSSComputedStyle(IFX_MEMAllocator* pAlloc)
      : m_dwRefCount(1), m_pAllocator(pAlloc) {}
  ~CFDE_CSSComputedStyle() {}
  virtual FX_DWORD AddRef() { return ++m_dwRefCount; }
  virtual FX_DWORD Release() {
    FX_DWORD dwRefCount = --m_dwRefCount;
    if (dwRefCount == 0) {
      if (m_NonInheritedData.m_pCounterStyle != NULL) {
        delete m_NonInheritedData.m_pCounterStyle;
      }
      FDE_DeleteWith(CFDE_CSSComputedStyle, m_pAllocator, this);
    }
    return dwRefCount;
  }

  virtual void Reset() {
    m_InheritedData.Reset();
    m_NonInheritedData.Reset();
  }
  virtual IFDE_CSSFontStyle* GetFontStyles() const {
    return (IFDE_CSSFontStyle * const) this;
  }
  virtual IFDE_CSSBoundaryStyle* GetBoundaryStyles() const {
    return (IFDE_CSSBoundaryStyle * const) this;
  }
  virtual IFDE_CSSPositionStyle* GetPositionStyles() const {
    return (IFDE_CSSPositionStyle * const) this;
  }
  virtual IFDE_CSSParagraphStyle* GetParagraphStyles() const {
    return (IFDE_CSSParagraphStyle * const) this;
  }
  virtual IFDE_CSSBackgroundStyle* GetBackgroundStyles() const {
    return (IFDE_CSSBackgroundStyle * const) this;
  }
  virtual IFDE_CSSVisualStyle* GetVisualStyles() const {
    return (IFDE_CSSVisualStyle * const) this;
  }
  virtual IFDE_CSSListStyle* GetListStyles() const {
    return (IFDE_CSSListStyle * const) this;
  }
  virtual IFDE_CSSTableStyle* GetTableStyle() const {
    return (IFDE_CSSTableStyle * const) this;
  }
  virtual IFDE_CSSMultiColumnStyle* GetMultiColumnStyle() const {
    return (IFDE_CSSMultiColumnStyle * const) this;
  }
  virtual IFDE_CSSGeneratedContentStyle* GetGeneratedContentStyle() const {
    return (IFDE_CSSGeneratedContentStyle * const) this;
  }
  virtual IFDE_CSSRubyStyle* GetRubyStyle() const {
    return (IFDE_CSSRubyStyle * const) this;
  }
  virtual FX_BOOL GetCustomStyle(const CFX_WideStringC& wsName,
                                 CFX_WideString& wsValue) const {
    for (int32_t i = m_CustomProperties.GetSize() - 2; i > -1; i -= 2) {
      if (wsName == m_CustomProperties[i]) {
        wsValue = m_CustomProperties[i + 1];
        return TRUE;
      }
    }
    return FALSE;
  }
  virtual FDE_CSSRUBYALIGN GetRubyAlign() const {
    return (FDE_CSSRUBYALIGN)m_InheritedData.m_eRubyAlign;
  }
  virtual FDE_CSSRUBYPOSITION GetRubyPosition() const {
    return (FDE_CSSRUBYPOSITION)m_InheritedData.m_eRubyPosition;
  }
  virtual FDE_CSSRUBYOVERHANG GetRubyOverhang() const {
    return (FDE_CSSRUBYOVERHANG)m_InheritedData.m_eRubyOverhang;
  }
  virtual FDE_CSSRUBYSPAN GetRubySpanType() const {
    return m_NonInheritedData.m_pRubySpan == NULL ? FDE_CSSRUBYSPAN_None
                                                  : FDE_CSSRUBYSPAN_Attr;
  }
  virtual IFDE_CSSValue* GetRubySpanAttr() const {
    return m_NonInheritedData.m_pRubySpan;
  }
  virtual FDE_CSSCAPTIONSIDE GetCaptionSide() const {
    return (FDE_CSSCAPTIONSIDE)m_InheritedData.m_eCaptionSide;
  }
  virtual int32_t CountCounters() {
    return (m_NonInheritedData.m_pCounterStyle == NULL)
               ? 0
               : m_NonInheritedData.m_pCounterStyle->CountCounters();
  }
  virtual const FX_WCHAR* GetCounterIdentifier(int32_t index) {
    return m_NonInheritedData.m_pCounterStyle->GetCounterIdentifier(index);
  }
  virtual FX_BOOL GetCounterReset(int32_t index, int32_t& iValue) {
    return m_NonInheritedData.m_pCounterStyle->GetCounterReset(index, iValue);
  }
  virtual FX_BOOL GetCounterIncrement(int32_t index, int32_t& iValue) {
    return m_NonInheritedData.m_pCounterStyle->GetCounterIncrement(index,
                                                                   iValue);
  }
  virtual IFDE_CSSValueList* GetContent() const {
    return m_NonInheritedData.m_pContentList;
  }
  virtual int32_t CountQuotes() const {
    return m_InheritedData.m_pQuotes == NULL
               ? 0
               : m_InheritedData.m_pQuotes->CountValues();
  }
  virtual const FX_WCHAR* GetQuotes(int32_t index) const {
    FXSYS_assert(m_InheritedData.m_pQuotes != NULL &&
                 m_InheritedData.m_pQuotes->CountValues() > index);
    return ((IFDE_CSSPrimitiveValue*)(m_InheritedData.m_pQuotes->GetValue(
                index)))
        ->GetString(index);
  }
  virtual const FDE_CSSLENGTH& GetColumnCount() const {
    return m_NonInheritedData.m_ColumnCount;
  }
  virtual const FDE_CSSLENGTH& GetColumnGap() const {
    return m_NonInheritedData.m_ColumnGap;
  }
  virtual FX_ARGB GetColumnRuleColor() const {
    return m_NonInheritedData.m_bColumnRuleColorSame
               ? m_InheritedData.m_dwFontColor
               : m_NonInheritedData.m_dwColumnRuleColor;
  }
  virtual FDE_CSSBORDERSTYLE GetColumnRuleStyle() const {
    return (FDE_CSSBORDERSTYLE)m_NonInheritedData.m_eColumnRuleStyle;
  }
  virtual const FDE_CSSLENGTH& GetColumnRuleWidth() const {
    return m_NonInheritedData.m_ColumnRuleWidth;
  }
  virtual const FDE_CSSLENGTH& GetColumnWidth() const {
    return m_NonInheritedData.m_ColumnWidth;
  }
  virtual void SetColumnCount(const FDE_CSSLENGTH& columnCount) {
    m_NonInheritedData.m_ColumnCount = columnCount;
  }
  virtual void SetColumnGap(const FDE_CSSLENGTH& columnGap) {
    m_NonInheritedData.m_ColumnGap = columnGap;
  }
  virtual void SetColumnRuleColor(FX_ARGB dwColumnRuleColor) {
    m_NonInheritedData.m_dwColumnRuleColor = dwColumnRuleColor,
    m_NonInheritedData.m_bColumnRuleColorSame = FALSE;
  }
  virtual void SetColumnRuleStyle(FDE_CSSBORDERSTYLE eColumnRuleStyle) {
    m_NonInheritedData.m_eColumnRuleStyle = eColumnRuleStyle;
  }
  virtual void SetColumnRuleWidth(const FDE_CSSLENGTH& columnRuleWidth) {
    m_NonInheritedData.m_ColumnRuleWidth = columnRuleWidth;
  }
  virtual void SetColumnWidth(const FDE_CSSLENGTH& columnWidth) {
    m_NonInheritedData.m_ColumnWidth = columnWidth;
  }
  virtual int32_t CountFontFamilies() const {
    return m_InheritedData.m_pFontFamily
               ? m_InheritedData.m_pFontFamily->CountValues()
               : 0;
  }
  virtual const FX_WCHAR* GetFontFamily(int32_t index) const {
    return ((IFDE_CSSPrimitiveValue*)(m_InheritedData.m_pFontFamily->GetValue(
                index)))
        ->GetString(index);
  }
  virtual FX_WORD GetFontWeight() const {
    return m_InheritedData.m_wFontWeight;
  }
  virtual FDE_CSSFONTVARIANT GetFontVariant() const {
    return (FDE_CSSFONTVARIANT)m_InheritedData.m_eFontVariant;
  }
  virtual FDE_CSSFONTSTYLE GetFontStyle() const {
    return (FDE_CSSFONTSTYLE)m_InheritedData.m_eFontStyle;
  }
  virtual FX_FLOAT GetFontSize() const { return m_InheritedData.m_fFontSize; }
  virtual FX_ARGB GetColor() const { return m_InheritedData.m_dwFontColor; }
  virtual void SetFontWeight(FX_WORD wFontWeight) {
    m_InheritedData.m_wFontWeight = wFontWeight;
  }
  virtual void SetFontVariant(FDE_CSSFONTVARIANT eFontVariant) {
    m_InheritedData.m_eFontVariant = eFontVariant;
  }
  virtual void SetFontStyle(FDE_CSSFONTSTYLE eFontStyle) {
    m_InheritedData.m_eFontStyle = eFontStyle;
  }
  virtual void SetFontSize(FX_FLOAT fFontSize) {
    m_InheritedData.m_fFontSize = fFontSize;
  }
  virtual void SetColor(FX_ARGB dwFontColor) {
    m_InheritedData.m_dwFontColor = dwFontColor;
  }
  virtual FX_ARGB GetBorderLeftColor() const {
    return m_NonInheritedData.m_dwBDRLeftColor;
  }
  virtual FX_ARGB GetBorderTopColor() const {
    return m_NonInheritedData.m_dwBDRTopColor;
  }
  virtual FX_ARGB GetBorderRightColor() const {
    return m_NonInheritedData.m_dwBDRRightColor;
  }
  virtual FX_ARGB GetBorderBottomColor() const {
    return m_NonInheritedData.m_dwBDRBottomColor;
  }

  virtual FDE_CSSBORDERSTYLE GetBorderLeftStyle() const {
    return (FDE_CSSBORDERSTYLE)m_NonInheritedData.m_eBDRLeftStyle;
  }
  virtual FDE_CSSBORDERSTYLE GetBorderTopStyle() const {
    return (FDE_CSSBORDERSTYLE)m_NonInheritedData.m_eBDRTopStyle;
  }
  virtual FDE_CSSBORDERSTYLE GetBorderRightStyle() const {
    return (FDE_CSSBORDERSTYLE)m_NonInheritedData.m_eBDRRightStyle;
  }
  virtual FDE_CSSBORDERSTYLE GetBorderBottomStyle() const {
    return (FDE_CSSBORDERSTYLE)m_NonInheritedData.m_eBDRBottomStyle;
  }

  virtual const FDE_CSSRECT* GetBorderWidth() const {
    return m_NonInheritedData.m_bHasBorder ? &(m_NonInheritedData.m_BorderWidth)
                                           : NULL;
  }
  virtual const FDE_CSSRECT* GetMarginWidth() const {
    return m_NonInheritedData.m_bHasMargin ? &(m_NonInheritedData.m_MarginWidth)
                                           : NULL;
  }
  virtual const FDE_CSSRECT* GetPaddingWidth() const {
    return m_NonInheritedData.m_bHasPadding
               ? &(m_NonInheritedData.m_PaddingWidth)
               : NULL;
  }
  virtual void SetBorderLeftColor(FX_ARGB dwBorderColor) {
    m_NonInheritedData.m_dwBDRLeftColor = dwBorderColor;
  }
  virtual void SetBorderTopColor(FX_ARGB dwBorderColor) {
    m_NonInheritedData.m_dwBDRTopColor = dwBorderColor;
  }
  virtual void SetBorderRightColor(FX_ARGB dwBorderColor) {
    m_NonInheritedData.m_dwBDRRightColor = dwBorderColor;
  }
  virtual void SetBorderBottomColor(FX_ARGB dwBorderColor) {
    m_NonInheritedData.m_dwBDRBottomColor = dwBorderColor;
  }

  virtual void SetBorderLeftStyle(FDE_CSSBORDERSTYLE eBorderStyle) {
    m_NonInheritedData.m_eBDRLeftStyle = eBorderStyle;
  }
  virtual void SetBorderTopStyle(FDE_CSSBORDERSTYLE eBorderStyle) {
    m_NonInheritedData.m_eBDRTopStyle = eBorderStyle;
  }
  virtual void SetBorderRightStyle(FDE_CSSBORDERSTYLE eBorderStyle) {
    m_NonInheritedData.m_eBDRRightStyle = eBorderStyle;
  }
  virtual void SetBorderBottomStyle(FDE_CSSBORDERSTYLE eBorderStyle) {
    m_NonInheritedData.m_eBDRBottomStyle = eBorderStyle;
  }

  virtual void SetBorderWidth(const FDE_CSSRECT& rect) {
    m_NonInheritedData.m_BorderWidth = rect;
    m_NonInheritedData.m_bHasBorder = TRUE;
  }
  virtual void SetMarginWidth(const FDE_CSSRECT& rect) {
    m_NonInheritedData.m_MarginWidth = rect;
    m_NonInheritedData.m_bHasMargin = TRUE;
  }
  virtual void SetPaddingWidth(const FDE_CSSRECT& rect) {
    m_NonInheritedData.m_PaddingWidth = rect;
    m_NonInheritedData.m_bHasPadding = TRUE;
  }
  virtual FDE_CSSDISPLAY GetDisplay() const {
    return (FDE_CSSDISPLAY)m_NonInheritedData.m_eDisplay;
  }
  virtual const FDE_CSSSIZE& GetBoxSize() const {
    return m_NonInheritedData.m_BoxSize;
  }
  virtual const FDE_CSSSIZE& GetMinBoxSize() const {
    return m_NonInheritedData.m_MinBoxSize;
  }
  virtual const FDE_CSSSIZE& GetMaxBoxSize() const {
    return m_NonInheritedData.m_MaxBoxSize;
  }
  virtual FDE_CSSFLOAT GetFloat() const {
    return (FDE_CSSFLOAT)m_NonInheritedData.m_eFloat;
  }
  virtual FDE_CSSCLEAR GetClear() const {
    return (FDE_CSSCLEAR)m_NonInheritedData.m_eClear;
  }
  virtual FDE_CSSPOSITION GetPosition() const {
    return (FDE_CSSPOSITION)m_NonInheritedData.m_ePosition;
  }
  virtual FDE_CSSLENGTH GetTop() const { return m_NonInheritedData.m_Top; }
  virtual FDE_CSSLENGTH GetBottom() const {
    return m_NonInheritedData.m_Bottom;
  }
  virtual FDE_CSSLENGTH GetLeft() const { return m_NonInheritedData.m_Left; }
  virtual FDE_CSSLENGTH GetRight() const { return m_NonInheritedData.m_Right; }

  virtual void SetDisplay(FDE_CSSDISPLAY eDisplay) {
    m_NonInheritedData.m_eDisplay = eDisplay;
  }
  virtual void SetBoxSize(const FDE_CSSSIZE& size) {
    m_NonInheritedData.m_BoxSize = size;
  }
  virtual void SetMinBoxSize(const FDE_CSSSIZE& size) {
    m_NonInheritedData.m_MinBoxSize = size;
  }
  virtual void SetMaxBoxSize(const FDE_CSSSIZE& size) {
    m_NonInheritedData.m_MaxBoxSize = size;
  }
  virtual void SetFloat(FDE_CSSFLOAT eFloat) {
    m_NonInheritedData.m_eFloat = eFloat;
  }
  virtual void SetClear(FDE_CSSCLEAR eClear) {
    m_NonInheritedData.m_eClear = eClear;
  }
  virtual FX_FLOAT GetLineHeight() const {
    return m_InheritedData.m_fLineHeight;
  }
  virtual FDE_CSSWHITESPACE GetWhiteSpace() const {
    return (FDE_CSSWHITESPACE)m_InheritedData.m_eWhiteSpace;
  }
  virtual const FDE_CSSLENGTH& GetTextIndent() const {
    return m_InheritedData.m_TextIndent;
  }
  virtual FDE_CSSTEXTALIGN GetTextAlign() const {
    return (FDE_CSSTEXTALIGN)m_InheritedData.m_eTextAligh;
  }
  virtual FDE_CSSVERTICALALIGN GetVerticalAlign() const {
    return (FDE_CSSVERTICALALIGN)m_NonInheritedData.m_eVerticalAlign;
  }
  virtual FX_FLOAT GetNumberVerticalAlign() const {
    return m_NonInheritedData.m_fVerticalAlign;
  }
  virtual FDE_CSSTEXTTRANSFORM GetTextTransform() const {
    return (FDE_CSSTEXTTRANSFORM)m_InheritedData.m_eTextTransform;
  }
  virtual FX_DWORD GetTextDecoration() const {
    return m_NonInheritedData.m_dwTextDecoration;
  }
  virtual const FDE_CSSLENGTH& GetLetterSpacing() const {
    return m_InheritedData.m_LetterSpacing;
  }
  virtual const FDE_CSSLENGTH& GetWordSpacing() const {
    return m_InheritedData.m_WordSpacing;
  }
  virtual FDE_CSSWRITINGMODE GetWritingMode() const {
    return (FDE_CSSWRITINGMODE)m_InheritedData.m_eWritingMode;
  }
  virtual FDE_CSSWORDBREAK GetWordBreak() const {
    return (FDE_CSSWORDBREAK)m_InheritedData.m_eWordBreak;
  }
  virtual int32_t GetWidows() const { return m_InheritedData.m_iWidows; }
  virtual FX_ARGB GetTextEmphasisColor() const {
    return m_InheritedData.m_bTextEmphasisColorCurrent
               ? m_InheritedData.m_dwFontColor
               : m_InheritedData.m_dwTextEmphasisColor;
  }
  virtual FDE_CSSPAGEBREAK GetPageBreakBefore() const {
    return (FDE_CSSPAGEBREAK)m_NonInheritedData.m_ePageBreakBefore;
  }
  virtual FDE_CSSPAGEBREAK GetPageBreakAfter() const {
    return (FDE_CSSPAGEBREAK)m_NonInheritedData.m_ePageBreakAfter;
  }
  virtual FDE_CSSPAGEBREAK GetPageBreakInside() const {
    return (FDE_CSSPAGEBREAK)m_NonInheritedData.m_ePageBreakInside;
  }
  virtual int32_t GetOrphans() const { return m_InheritedData.m_iOrphans; }
  virtual FDE_CSSLINEBREAK GetLineBreak() const {
    return (FDE_CSSLINEBREAK)m_InheritedData.m_eLineBreak;
  }
  virtual FDE_CSSTEXTEMPHASISMARK GetTextEmphasisMark() const;
  virtual FDE_CSSTEXTEMPHASISFILL GetTextEmphasisFill() const {
    return (FDE_CSSTEXTEMPHASISFILL)m_InheritedData.m_eTextEmphasisFill;
  }
  virtual const FX_WCHAR* GetTextEmphasisCustom() const {
    FXSYS_assert(m_InheritedData.m_eTextEmphasisMark ==
                 FDE_CSSTEXTEMPHASISMARK_Custom);
    return m_InheritedData.m_pszTextEmphasisCustomMark;
  }
  virtual FDE_CSSTEXTCOMBINE GetTextCombineType() const {
    return (FDE_CSSTEXTCOMBINE)m_NonInheritedData.m_eTextCombine;
  }
  virtual FX_BOOL HasTextCombineNumber() const {
    return m_NonInheritedData.m_bHasTextCombineNumber;
  }
  virtual FX_FLOAT GetTextCombineNumber() const {
    FXSYS_assert(m_NonInheritedData.m_eTextCombine ==
                 FDE_CSSTEXTCOMBINE_Horizontal);
    return m_NonInheritedData.m_fTextCombineNumber;
  }
  virtual void SetLineHeight(FX_FLOAT fLineHeight) {
    m_InheritedData.m_fLineHeight = fLineHeight;
  }
  virtual void SetWhiteSpace(FDE_CSSWHITESPACE eWhiteSpace) {
    m_InheritedData.m_eWhiteSpace = eWhiteSpace;
  }
  virtual void SetTextIndent(const FDE_CSSLENGTH& textIndent) {
    m_InheritedData.m_TextIndent = textIndent;
  }
  virtual void SetTextAlign(FDE_CSSTEXTALIGN eTextAlign) {
    m_InheritedData.m_eTextAligh = eTextAlign;
  }
  virtual void SetVerticalAlign(FDE_CSSVERTICALALIGN eVerticalAlign) {
    m_NonInheritedData.m_eVerticalAlign = eVerticalAlign;
  }
  virtual void SetNumberVerticalAlign(FX_FLOAT fAlign) {
    m_NonInheritedData.m_eVerticalAlign = FDE_CSSVERTICALALIGN_Number,
    m_NonInheritedData.m_fVerticalAlign = fAlign;
  }
  virtual void SetTextTransform(FDE_CSSTEXTTRANSFORM eTextTransform) {
    m_InheritedData.m_eTextTransform = eTextTransform;
  }
  virtual void SetTextDecoration(FX_DWORD dwTextDecoration) {
    m_NonInheritedData.m_dwTextDecoration = dwTextDecoration;
  }
  virtual void SetLetterSpacing(const FDE_CSSLENGTH& letterSpacing) {
    m_InheritedData.m_LetterSpacing = letterSpacing;
  }
  virtual void SetWordSpacing(const FDE_CSSLENGTH& wordSpacing) {
    m_InheritedData.m_WordSpacing = wordSpacing;
  }
  virtual void SetWritingMode(FDE_CSSWRITINGMODE eWritingMode) {
    m_InheritedData.m_eWritingMode = eWritingMode;
  }
  virtual void SetWordBreak(FDE_CSSWORDBREAK eWordBreak) {
    m_InheritedData.m_eWordBreak = eWordBreak;
  }
  virtual void SetWidows(int32_t iWidows) {
    m_InheritedData.m_iWidows = iWidows;
  }
  virtual void SetTextEmphasisColor(FX_ARGB dwTextEmphasisColor) {
    m_InheritedData.m_dwTextEmphasisColor = dwTextEmphasisColor,
    m_InheritedData.m_bTextEmphasisColorCurrent = FALSE;
  }
  virtual void SetPageBreakBefore(FDE_CSSPAGEBREAK ePageBreakBefore) {
    m_NonInheritedData.m_ePageBreakBefore = ePageBreakBefore;
  }
  virtual void SetPageBreakAfter(FDE_CSSPAGEBREAK ePageBreakAfter) {
    m_NonInheritedData.m_ePageBreakAfter = ePageBreakAfter;
  }
  virtual void SetPageBreakInside(FDE_CSSPAGEBREAK ePageBreakInside) {
    m_NonInheritedData.m_ePageBreakInside = ePageBreakInside;
  }
  virtual void SetOrphans(int32_t iOrphans) {
    m_InheritedData.m_iOrphans = iOrphans;
  }
  virtual void SetLineBreak(FDE_CSSLINEBREAK eLineBreak) {
    m_InheritedData.m_eLineBreak = eLineBreak;
  }
  virtual FX_ARGB GetBKGColor() const {
    return m_NonInheritedData.m_dwBKGColor;
  }
  virtual const FX_WCHAR* GetBKGImage() const {
    return m_NonInheritedData.m_pszBKGImage;
  }
  virtual const FDE_CSSPOINT& GetBKGPosition() const {
    return m_NonInheritedData.m_BKGPosition;
  }
  virtual FDE_CSSBKGREPEAT GetBKGRepeat() const {
    return (FDE_CSSBKGREPEAT)m_NonInheritedData.m_eBKGRepeat;
  }
  virtual FDE_CSSBKGATTACHMENT GetBKGAttachment() const {
    return (FDE_CSSBKGATTACHMENT)m_NonInheritedData.m_eBKGAttachment;
  }
  virtual void SetBKGColor(FX_ARGB dwBKGColor) {
    m_NonInheritedData.m_dwBKGColor = dwBKGColor;
  }
  virtual void SetBKGPosition(const FDE_CSSPOINT& bkgPosition) {
    m_NonInheritedData.m_BKGPosition = bkgPosition;
  }
  virtual FDE_CSSVISIBILITY GetVisibility() const {
    return (FDE_CSSVISIBILITY)m_InheritedData.m_eVisibility;
  }
  virtual FDE_CSSOVERFLOW GetOverflowX() const {
    return (FDE_CSSOVERFLOW)m_NonInheritedData.m_eOverflowX;
  }
  virtual FDE_CSSOVERFLOW GetOverflowY() const {
    return (FDE_CSSOVERFLOW)m_NonInheritedData.m_eOverflowY;
  }
  virtual int32_t CountCursorUrls() const {
    return m_InheritedData.m_pCursorUris == NULL
               ? 0
               : m_InheritedData.m_pCursorUris->CountValues();
  }
  virtual const FX_WCHAR* GetCursorUrl(int32_t index) const {
    FXSYS_assert(m_InheritedData.m_pCursorUris != NULL);
    return ((IFDE_CSSPrimitiveValue*)(m_InheritedData.m_pCursorUris->GetValue(
                index)))
        ->GetString(index);
  }
  virtual FDE_CSSCURSOR GetCursorType() const {
    return m_InheritedData.m_eCursor;
  }
  virtual void SetVisibility(FDE_CSSVISIBILITY eVisibility) {
    m_InheritedData.m_eVisibility = eVisibility;
  }
  virtual FDE_CSSLISTSTYLETYPE GetListStyleType() const {
    return (FDE_CSSLISTSTYLETYPE)m_NonInheritedData.m_eListStyleType;
  }
  virtual FDE_CSSLISTSTYLEPOSITION GetListStylePosition() const {
    return (FDE_CSSLISTSTYLEPOSITION)m_NonInheritedData.m_eListStylePosition;
  }
  virtual const FX_WCHAR* GetListStyleImage() const {
    return m_InheritedData.m_pszListStyleImage;
  }
  virtual void SetListStyleType(FDE_CSSLISTSTYLETYPE eListStyleType) {
    m_NonInheritedData.m_eListStyleType = eListStyleType;
  }
  virtual void SetListStylePosition(
      FDE_CSSLISTSTYLEPOSITION eListStylePosition) {
    m_NonInheritedData.m_eListStylePosition = eListStylePosition;
  }
  void AddCustomStyle(const CFX_WideString& wsName,
                      const CFX_WideString& wsValue) {
    m_CustomProperties.Add(wsName);
    m_CustomProperties.Add(wsValue);
  }
  FX_DWORD m_dwRefCount;
  IFX_MEMAllocator* m_pAllocator;
  CFDE_CSSInheritedData m_InheritedData;
  CFDE_CSSNonInheritedData m_NonInheritedData;
  CFX_WideStringArray m_CustomProperties;
};
#endif
