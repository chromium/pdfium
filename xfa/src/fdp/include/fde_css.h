// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_CSS
#define _FDE_CSS
class IFDE_HTMNotify;
class IFDE_CSSValue;
class IFDE_CSSPrimitiveValue;
class IFDE_CSSValueList;
class IFDE_CSSDeclaration;
class IFDE_CSSSelector;
class IFDE_CSSRule;
class IFDE_CSSStyleRule;
class IFDE_CSSMediaRule;
class IFDE_CSSFontFaceRule;
class IFDE_CSSStyleSheet;
class IFDE_CSSStyleSheetCache;
class IFDE_CSSSyntaxParser;
class IFDE_CSSRubyStyle;
class IFDE_CSSMultiColumnStyle;
class IFDE_CSSGeneratedContentStyle;
class IFDE_CSSFontStyle;
class IFDE_CSSBoundaryStyle;
class IFDE_CSSPositionStyle;
class IFDE_CSSParagraphStyle;
class IFDE_CSSBackgroundStyle;
class IFDE_CSSListStyle;
class IFDE_CSSTableStyle;
class IFDE_CSSVisualStyle;
class IFDE_CSSComputedStyle;
class IFDE_CSSTagProvider;
class IFDE_CSSAccelerator;
class IFDE_CSSStyleSelector;
class IFDE_CSSCounterContext;
class IFDE_CSSCounterManager;
class IFDE_CSSQuoteContext;
class IFDE_CSSContentContext;
class IFDE_CSSMultiColumnContext;
class IFDE_CSSFloatContext;
enum FDE_CSSVALUETYPE {
  FDE_CSSVALUETYPE_Primitive = 1,
  FDE_CSSVALUETYPE_List = 2,
  FDE_CSSVALUETYPE_Shorthand,
};
enum FDE_CSSPRIMITIVETYPE {
  FDE_CSSPRIMITIVETYPE_Unknown = 0,
  FDE_CSSPRIMITIVETYPE_Number = 1,
  FDE_CSSPRIMITIVETYPE_Percent = 2,
  FDE_CSSPRIMITIVETYPE_EMS = 3,
  FDE_CSSPRIMITIVETYPE_EXS = 4,
  FDE_CSSPRIMITIVETYPE_PX = 5,
  FDE_CSSPRIMITIVETYPE_CM = 6,
  FDE_CSSPRIMITIVETYPE_MM = 7,
  FDE_CSSPRIMITIVETYPE_IN = 8,
  FDE_CSSPRIMITIVETYPE_PT = 9,
  FDE_CSSPRIMITIVETYPE_PC = 10,
  FDE_CSSPRIMITIVETYPE_String = 19,
  FDE_CSSPRIMITIVETYPE_URI = 20,
  FDE_CSSPRIMITIVETYPE_RGB = 25,
  FDE_CSSPRIMITIVETYPE_Enum,
  FDE_CSSPRIMITIVETYPE_Function,
};
enum FDE_CSSPROPERTYVALUE {
  FDE_CSSPROPERTYVALUE_Bolder,
  FDE_CSSPROPERTYVALUE_LowerLatin,
  FDE_CSSPROPERTYVALUE_Lowercase,
  FDE_CSSPROPERTYVALUE_LowerGreek,
  FDE_CSSPROPERTYVALUE_Sesame,
  FDE_CSSPROPERTYVALUE_None,
  FDE_CSSPROPERTYVALUE_NwResize,
  FDE_CSSPROPERTYVALUE_WResize,
  FDE_CSSPROPERTYVALUE_Dot,
  FDE_CSSPROPERTYVALUE_End,
  FDE_CSSPROPERTYVALUE_Ltr,
  FDE_CSSPROPERTYVALUE_Pre,
  FDE_CSSPROPERTYVALUE_Rtl,
  FDE_CSSPROPERTYVALUE_Sub,
  FDE_CSSPROPERTYVALUE_Top,
  FDE_CSSPROPERTYVALUE_Visible,
  FDE_CSSPROPERTYVALUE_Filled,
  FDE_CSSPROPERTYVALUE_SwResize,
  FDE_CSSPROPERTYVALUE_NoRepeat,
  FDE_CSSPROPERTYVALUE_Default,
  FDE_CSSPROPERTYVALUE_Transparent,
  FDE_CSSPROPERTYVALUE_Ridge,
  FDE_CSSPROPERTYVALUE_Right,
  FDE_CSSPROPERTYVALUE_HorizontalTb,
  FDE_CSSPROPERTYVALUE_DistributeLetter,
  FDE_CSSPROPERTYVALUE_DoubleCircle,
  FDE_CSSPROPERTYVALUE_Ruby,
  FDE_CSSPROPERTYVALUE_Collapse,
  FDE_CSSPROPERTYVALUE_Normal,
  FDE_CSSPROPERTYVALUE_Avoid,
  FDE_CSSPROPERTYVALUE_UpperRoman,
  FDE_CSSPROPERTYVALUE_Auto,
  FDE_CSSPROPERTYVALUE_Text,
  FDE_CSSPROPERTYVALUE_XSmall,
  FDE_CSSPROPERTYVALUE_Thin,
  FDE_CSSPROPERTYVALUE_Repeat,
  FDE_CSSPROPERTYVALUE_Small,
  FDE_CSSPROPERTYVALUE_NeResize,
  FDE_CSSPROPERTYVALUE_NoContent,
  FDE_CSSPROPERTYVALUE_Outside,
  FDE_CSSPROPERTYVALUE_EResize,
  FDE_CSSPROPERTYVALUE_TableRow,
  FDE_CSSPROPERTYVALUE_Bottom,
  FDE_CSSPROPERTYVALUE_Underline,
  FDE_CSSPROPERTYVALUE_CjkIdeographic,
  FDE_CSSPROPERTYVALUE_SeResize,
  FDE_CSSPROPERTYVALUE_Fixed,
  FDE_CSSPROPERTYVALUE_Double,
  FDE_CSSPROPERTYVALUE_Solid,
  FDE_CSSPROPERTYVALUE_RubyBaseGroup,
  FDE_CSSPROPERTYVALUE_OpenQuote,
  FDE_CSSPROPERTYVALUE_Lighter,
  FDE_CSSPROPERTYVALUE_LowerRoman,
  FDE_CSSPROPERTYVALUE_Strict,
  FDE_CSSPROPERTYVALUE_TableCaption,
  FDE_CSSPROPERTYVALUE_Oblique,
  FDE_CSSPROPERTYVALUE_Decimal,
  FDE_CSSPROPERTYVALUE_Loose,
  FDE_CSSPROPERTYVALUE_Hebrew,
  FDE_CSSPROPERTYVALUE_Hidden,
  FDE_CSSPROPERTYVALUE_Dashed,
  FDE_CSSPROPERTYVALUE_Embed,
  FDE_CSSPROPERTYVALUE_TableRowGroup,
  FDE_CSSPROPERTYVALUE_TableColumn,
  FDE_CSSPROPERTYVALUE_Static,
  FDE_CSSPROPERTYVALUE_Outset,
  FDE_CSSPROPERTYVALUE_DecimalLeadingZero,
  FDE_CSSPROPERTYVALUE_KeepWords,
  FDE_CSSPROPERTYVALUE_KatakanaIroha,
  FDE_CSSPROPERTYVALUE_Super,
  FDE_CSSPROPERTYVALUE_Center,
  FDE_CSSPROPERTYVALUE_TableHeaderGroup,
  FDE_CSSPROPERTYVALUE_Inside,
  FDE_CSSPROPERTYVALUE_XxLarge,
  FDE_CSSPROPERTYVALUE_Triangle,
  FDE_CSSPROPERTYVALUE_RubyTextGroup,
  FDE_CSSPROPERTYVALUE_Circle,
  FDE_CSSPROPERTYVALUE_Hiragana,
  FDE_CSSPROPERTYVALUE_RepeatX,
  FDE_CSSPROPERTYVALUE_RepeatY,
  FDE_CSSPROPERTYVALUE_Move,
  FDE_CSSPROPERTYVALUE_HiraganaIroha,
  FDE_CSSPROPERTYVALUE_RubyBase,
  FDE_CSSPROPERTYVALUE_Scroll,
  FDE_CSSPROPERTYVALUE_Smaller,
  FDE_CSSPROPERTYVALUE_TableFooterGroup,
  FDE_CSSPROPERTYVALUE_Baseline,
  FDE_CSSPROPERTYVALUE_Separate,
  FDE_CSSPROPERTYVALUE_Armenian,
  FDE_CSSPROPERTYVALUE_Open,
  FDE_CSSPROPERTYVALUE_Relative,
  FDE_CSSPROPERTYVALUE_Thick,
  FDE_CSSPROPERTYVALUE_Justify,
  FDE_CSSPROPERTYVALUE_Middle,
  FDE_CSSPROPERTYVALUE_Always,
  FDE_CSSPROPERTYVALUE_DistributeSpace,
  FDE_CSSPROPERTYVALUE_LineEdge,
  FDE_CSSPROPERTYVALUE_PreWrap,
  FDE_CSSPROPERTYVALUE_Medium,
  FDE_CSSPROPERTYVALUE_NResize,
  FDE_CSSPROPERTYVALUE_ListItem,
  FDE_CSSPROPERTYVALUE_Show,
  FDE_CSSPROPERTYVALUE_Currentcolor,
  FDE_CSSPROPERTYVALUE_NoCloseQuote,
  FDE_CSSPROPERTYVALUE_VerticalLr,
  FDE_CSSPROPERTYVALUE_VerticalRl,
  FDE_CSSPROPERTYVALUE_Pointer,
  FDE_CSSPROPERTYVALUE_XxSmall,
  FDE_CSSPROPERTYVALUE_Bold,
  FDE_CSSPROPERTYVALUE_Both,
  FDE_CSSPROPERTYVALUE_SmallCaps,
  FDE_CSSPROPERTYVALUE_Katakana,
  FDE_CSSPROPERTYVALUE_After,
  FDE_CSSPROPERTYVALUE_Horizontal,
  FDE_CSSPROPERTYVALUE_Dotted,
  FDE_CSSPROPERTYVALUE_Disc,
  FDE_CSSPROPERTYVALUE_Georgian,
  FDE_CSSPROPERTYVALUE_Inline,
  FDE_CSSPROPERTYVALUE_Overline,
  FDE_CSSPROPERTYVALUE_Wait,
  FDE_CSSPROPERTYVALUE_BreakAll,
  FDE_CSSPROPERTYVALUE_UpperAlpha,
  FDE_CSSPROPERTYVALUE_Capitalize,
  FDE_CSSPROPERTYVALUE_Nowrap,
  FDE_CSSPROPERTYVALUE_TextBottom,
  FDE_CSSPROPERTYVALUE_NoOpenQuote,
  FDE_CSSPROPERTYVALUE_Groove,
  FDE_CSSPROPERTYVALUE_Progress,
  FDE_CSSPROPERTYVALUE_Larger,
  FDE_CSSPROPERTYVALUE_CloseQuote,
  FDE_CSSPROPERTYVALUE_TableCell,
  FDE_CSSPROPERTYVALUE_PreLine,
  FDE_CSSPROPERTYVALUE_Absolute,
  FDE_CSSPROPERTYVALUE_InlineTable,
  FDE_CSSPROPERTYVALUE_BidiOverride,
  FDE_CSSPROPERTYVALUE_InlineBlock,
  FDE_CSSPROPERTYVALUE_Inset,
  FDE_CSSPROPERTYVALUE_Crosshair,
  FDE_CSSPROPERTYVALUE_UpperLatin,
  FDE_CSSPROPERTYVALUE_Help,
  FDE_CSSPROPERTYVALUE_Hide,
  FDE_CSSPROPERTYVALUE_Uppercase,
  FDE_CSSPROPERTYVALUE_SResize,
  FDE_CSSPROPERTYVALUE_Table,
  FDE_CSSPROPERTYVALUE_Blink,
  FDE_CSSPROPERTYVALUE_Block,
  FDE_CSSPROPERTYVALUE_Start,
  FDE_CSSPROPERTYVALUE_TableColumnGroup,
  FDE_CSSPROPERTYVALUE_Italic,
  FDE_CSSPROPERTYVALUE_LineThrough,
  FDE_CSSPROPERTYVALUE_KeepAll,
  FDE_CSSPROPERTYVALUE_LowerAlpha,
  FDE_CSSPROPERTYVALUE_RunIn,
  FDE_CSSPROPERTYVALUE_Square,
  FDE_CSSPROPERTYVALUE_XLarge,
  FDE_CSSPROPERTYVALUE_Large,
  FDE_CSSPROPERTYVALUE_Before,
  FDE_CSSPROPERTYVALUE_Left,
  FDE_CSSPROPERTYVALUE_TextTop,
  FDE_CSSPROPERTYVALUE_RubyText,
  FDE_CSSPROPERTYVALUE_NoDisplay,
  FDE_CSSPROPERTYVALUE_MAX
};
class IFDE_CSSValue {
 public:
  virtual ~IFDE_CSSValue() {}
  virtual FDE_CSSVALUETYPE GetType() const = 0;
};
class IFDE_CSSPrimitiveValue : public IFDE_CSSValue {
 public:
  virtual FDE_CSSVALUETYPE GetType() const {
    return FDE_CSSVALUETYPE_Primitive;
  }
  virtual FDE_CSSPRIMITIVETYPE GetPrimitiveType() const = 0;
  virtual FX_ARGB GetRGBColor() const = 0;
  virtual FX_FLOAT GetFloat() const = 0;
  virtual const FX_WCHAR* GetString(int32_t& iLength) const = 0;
  virtual FDE_CSSPROPERTYVALUE GetEnum() const = 0;
  virtual const FX_WCHAR* GetFuncName() const = 0;
  virtual int32_t CountArgs() const = 0;
  virtual IFDE_CSSValue* GetArgs(int32_t index) const = 0;
};
class IFDE_CSSValueList : public IFDE_CSSValue {
 public:
  virtual FDE_CSSVALUETYPE GetType() const { return FDE_CSSVALUETYPE_List; }
  virtual int32_t CountValues() const = 0;
  virtual IFDE_CSSValue* GetValue(int32_t index) const = 0;
};
enum FDE_CSSPROPERTY {
  FDE_CSSPROPERTY_WritingMode,
  FDE_CSSPROPERTY_ColumnRuleWidth,
  FDE_CSSPROPERTY_BorderLeft,
  FDE_CSSPROPERTY_ColumnRule,
  FDE_CSSPROPERTY_Height,
  FDE_CSSPROPERTY_CounterReset,
  FDE_CSSPROPERTY_Content,
  FDE_CSSPROPERTY_RubyPosition,
  FDE_CSSPROPERTY_BackgroundColor,
  FDE_CSSPROPERTY_Width,
  FDE_CSSPROPERTY_Src,
  FDE_CSSPROPERTY_Top,
  FDE_CSSPROPERTY_Margin,
  FDE_CSSPROPERTY_BorderColor,
  FDE_CSSPROPERTY_Widows,
  FDE_CSSPROPERTY_BorderBottomColor,
  FDE_CSSPROPERTY_TextIndent,
  FDE_CSSPROPERTY_Right,
  FDE_CSSPROPERTY_TextEmphasisStyle,
  FDE_CSSPROPERTY_PaddingLeft,
  FDE_CSSPROPERTY_ColumnWidth,
  FDE_CSSPROPERTY_MarginLeft,
  FDE_CSSPROPERTY_Border,
  FDE_CSSPROPERTY_BorderTop,
  FDE_CSSPROPERTY_RubyOverhang,
  FDE_CSSPROPERTY_PageBreakBefore,
  FDE_CSSPROPERTY_MaxHeight,
  FDE_CSSPROPERTY_MinWidth,
  FDE_CSSPROPERTY_BorderLeftColor,
  FDE_CSSPROPERTY_Bottom,
  FDE_CSSPROPERTY_Quotes,
  FDE_CSSPROPERTY_MaxWidth,
  FDE_CSSPROPERTY_PaddingRight,
  FDE_CSSPROPERTY_ListStyleImage,
  FDE_CSSPROPERTY_WhiteSpace,
  FDE_CSSPROPERTY_BorderBottom,
  FDE_CSSPROPERTY_ListStyleType,
  FDE_CSSPROPERTY_WordBreak,
  FDE_CSSPROPERTY_OverflowX,
  FDE_CSSPROPERTY_OverflowY,
  FDE_CSSPROPERTY_BorderTopColor,
  FDE_CSSPROPERTY_FontFamily,
  FDE_CSSPROPERTY_Cursor,
  FDE_CSSPROPERTY_RubyAlign,
  FDE_CSSPROPERTY_ColumnRuleColor,
  FDE_CSSPROPERTY_FontWeight,
  FDE_CSSPROPERTY_BorderRightStyle,
  FDE_CSSPROPERTY_MinHeight,
  FDE_CSSPROPERTY_Color,
  FDE_CSSPROPERTY_LetterSpacing,
  FDE_CSSPROPERTY_EmptyCells,
  FDE_CSSPROPERTY_TextAlign,
  FDE_CSSPROPERTY_RubySpan,
  FDE_CSSPROPERTY_Position,
  FDE_CSSPROPERTY_BorderStyle,
  FDE_CSSPROPERTY_BorderBottomStyle,
  FDE_CSSPROPERTY_BorderCollapse,
  FDE_CSSPROPERTY_ColumnCount,
  FDE_CSSPROPERTY_BorderRightWidth,
  FDE_CSSPROPERTY_UnicodeBidi,
  FDE_CSSPROPERTY_VerticalAlign,
  FDE_CSSPROPERTY_PaddingTop,
  FDE_CSSPROPERTY_Columns,
  FDE_CSSPROPERTY_Overflow,
  FDE_CSSPROPERTY_TableLayout,
  FDE_CSSPROPERTY_FontVariant,
  FDE_CSSPROPERTY_ListStyle,
  FDE_CSSPROPERTY_BackgroundPosition,
  FDE_CSSPROPERTY_BorderWidth,
  FDE_CSSPROPERTY_TextEmphasisColor,
  FDE_CSSPROPERTY_BorderLeftStyle,
  FDE_CSSPROPERTY_PageBreakInside,
  FDE_CSSPROPERTY_TextEmphasis,
  FDE_CSSPROPERTY_BorderBottomWidth,
  FDE_CSSPROPERTY_ColumnGap,
  FDE_CSSPROPERTY_Orphans,
  FDE_CSSPROPERTY_BorderRight,
  FDE_CSSPROPERTY_FontSize,
  FDE_CSSPROPERTY_PageBreakAfter,
  FDE_CSSPROPERTY_CaptionSide,
  FDE_CSSPROPERTY_BackgroundRepeat,
  FDE_CSSPROPERTY_BorderTopStyle,
  FDE_CSSPROPERTY_BorderSpacing,
  FDE_CSSPROPERTY_TextTransform,
  FDE_CSSPROPERTY_FontStyle,
  FDE_CSSPROPERTY_Font,
  FDE_CSSPROPERTY_LineHeight,
  FDE_CSSPROPERTY_MarginRight,
  FDE_CSSPROPERTY_Float,
  FDE_CSSPROPERTY_BorderLeftWidth,
  FDE_CSSPROPERTY_Display,
  FDE_CSSPROPERTY_Clear,
  FDE_CSSPROPERTY_ColumnRuleStyle,
  FDE_CSSPROPERTY_TextCombine,
  FDE_CSSPROPERTY_ListStylePosition,
  FDE_CSSPROPERTY_Visibility,
  FDE_CSSPROPERTY_PaddingBottom,
  FDE_CSSPROPERTY_BackgroundAttachment,
  FDE_CSSPROPERTY_BackgroundImage,
  FDE_CSSPROPERTY_LineBreak,
  FDE_CSSPROPERTY_Background,
  FDE_CSSPROPERTY_BorderTopWidth,
  FDE_CSSPROPERTY_WordSpacing,
  FDE_CSSPROPERTY_BorderRightColor,
  FDE_CSSPROPERTY_CounterIncrement,
  FDE_CSSPROPERTY_Left,
  FDE_CSSPROPERTY_TextDecoration,
  FDE_CSSPROPERTY_Padding,
  FDE_CSSPROPERTY_MarginBottom,
  FDE_CSSPROPERTY_MarginTop,
  FDE_CSSPROPERTY_Direction,
  FDE_CSSPROPERTY_MAX
};
class IFDE_CSSDeclaration {
 public:
  virtual ~IFDE_CSSDeclaration() {}
  virtual IFDE_CSSValue* GetProperty(FDE_CSSPROPERTY eProperty,
                                     FX_BOOL& bImportant) const = 0;
  virtual FX_POSITION GetStartPosition() const = 0;
  virtual void GetNextProperty(FX_POSITION& pos,
                               FDE_CSSPROPERTY& eProperty,
                               IFDE_CSSValue*& pValue,
                               FX_BOOL& bImportant) const = 0;
  virtual FX_POSITION GetStartCustom() const = 0;
  virtual void GetNextCustom(FX_POSITION& pos,
                             CFX_WideString& wsName,
                             CFX_WideString& wsValue) const = 0;
};
typedef CFX_ArrayTemplate<IFDE_CSSDeclaration*> CFDE_CSSDeclarationArray;
enum FDE_CSSPERSUDO {
  FDE_CSSPERSUDO_After,
  FDE_CSSPERSUDO_Before,
  FDE_CSSPERSUDO_NONE
};
enum FDE_CSSSELECTORTYPE {
  FDE_CSSSELECTORTYPE_Element,
  FDE_CSSSELECTORTYPE_Descendant,
  FDE_CSSSELECTORTYPE_Class,
  FDE_CSSSELECTORTYPE_Persudo,
  FDE_CSSSELECTORTYPE_ID,
};
class IFDE_CSSSelector {
 public:
  virtual ~IFDE_CSSSelector() {}
  virtual FDE_CSSSELECTORTYPE GetType() const = 0;
  virtual FX_DWORD GetNameHash() const = 0;
  virtual IFDE_CSSSelector* GetNextSelector() const = 0;
};
#define FDE_CSSMEDIATYPE_Braille 0x01
#define FDE_CSSMEDIATYPE_Emboss 0x02
#define FDE_CSSMEDIATYPE_Handheld 0x04
#define FDE_CSSMEDIATYPE_Print 0x08
#define FDE_CSSMEDIATYPE_Projection 0x10
#define FDE_CSSMEDIATYPE_Screen 0x20
#define FDE_CSSMEDIATYPE_TTY 0x40
#define FDE_CSSMEDIATYPE_TV 0x80
#define FDE_CSSMEDIATYPE_ALL 0xFF
enum FDE_CSSRULETYPE {
  FDE_CSSRULETYPE_Unknown = 0,
  FDE_CSSRULETYPE_Style = 1,
  FDE_CSSRULETYPE_Media = 4,
  FDE_CSSRULETYPE_FontFace = 5,
};
class IFDE_CSSRule {
 public:
  virtual ~IFDE_CSSRule() {}
  virtual FDE_CSSRULETYPE GetType() const = 0;
};
typedef CFX_MassArrayTemplate<IFDE_CSSRule*> CFDE_CSSRuleArray;
class IFDE_CSSStyleRule : public IFDE_CSSRule {
 public:
  virtual FDE_CSSRULETYPE GetType() const { return FDE_CSSRULETYPE_Style; }
  virtual int32_t CountSelectorLists() const = 0;
  virtual IFDE_CSSSelector* GetSelectorList(int32_t index) const = 0;
  virtual IFDE_CSSDeclaration* GetDeclaration() const = 0;
};
class IFDE_CSSMediaRule : public IFDE_CSSRule {
 public:
  virtual FDE_CSSRULETYPE GetType() const { return FDE_CSSRULETYPE_Media; }
  virtual FX_DWORD GetMediaList() const = 0;
  virtual int32_t CountRules() const = 0;
  virtual IFDE_CSSRule* GetRule(int32_t index) = 0;
};
class IFDE_CSSFontFaceRule : public IFDE_CSSRule {
 public:
  virtual FDE_CSSRULETYPE GetType() const { return FDE_CSSRULETYPE_FontFace; }
  virtual IFDE_CSSDeclaration* GetDeclaration() const = 0;
};
class IFDE_CSSStyleSheet : public IFX_Unknown {
 public:
  static IFDE_CSSStyleSheet* LoadHTMLStandardStyleSheet();
  static IFDE_CSSStyleSheet* LoadFromStream(
      const CFX_WideString& szUrl,
      IFX_Stream* pStream,
      FX_WORD wCodePage,
      FX_DWORD dwMediaList = FDE_CSSMEDIATYPE_ALL);
  static IFDE_CSSStyleSheet* LoadFromBuffer(
      const CFX_WideString& szUrl,
      const FX_WCHAR* pBuffer,
      int32_t iBufSize,
      FX_WORD wCodePage,
      FX_DWORD dwMediaList = FDE_CSSMEDIATYPE_ALL);
  virtual FX_BOOL GetUrl(CFX_WideString& szUrl) = 0;
  virtual FX_DWORD GetMediaList() const = 0;
  virtual FX_WORD GetCodePage() const = 0;

  virtual int32_t CountRules() const = 0;
  virtual IFDE_CSSRule* GetRule(int32_t index) = 0;
};
typedef CFX_ArrayTemplate<IFDE_CSSStyleSheet*> CFDE_CSSStyleSheetArray;
#define FDE_CSSUSERSTYLESHEET (FX_BSTRC("#USERSHEET"))
#define FDE_CSSUAGENTSTYLESHEET (FX_BSTRC("#AGENTSHEET"))
class IFDE_CSSStyleSheetCache {
 public:
  static IFDE_CSSStyleSheetCache* Create();
  virtual ~IFDE_CSSStyleSheetCache() {}
  virtual void Release() = 0;
  virtual void SetMaxItems(int32_t iMaxCount = 5) = 0;
  virtual void AddStyleSheet(const CFX_ByteStringC& szKey,
                             IFDE_CSSStyleSheet* pStyleSheet) = 0;
  virtual IFDE_CSSStyleSheet* GetStyleSheet(
      const CFX_ByteStringC& szKey) const = 0;
  virtual void RemoveStyleSheet(const CFX_ByteStringC& szKey) = 0;
};
enum FDE_CSSSYNTAXSTATUS {
  FDE_CSSSYNTAXSTATUS_Error,
  FDE_CSSSYNTAXSTATUS_EOS,
  FDE_CSSSYNTAXSTATUS_None,
  FDE_CSSSYNTAXSTATUS_Charset,
  FDE_CSSSYNTAXSTATUS_ImportRule,
  FDE_CSSSYNTAXSTATUS_ImportClose,
  FDE_CSSSYNTAXSTATUS_PageRule,
  FDE_CSSSYNTAXSTATUS_StyleRule,
  FDE_CSSSYNTAXSTATUS_FontFaceRule,
  FDE_CSSSYNTAXSTATUS_MediaRule,
  FDE_CSSSYNTAXSTATUS_MediaType,
  FDE_CSSSYNTAXSTATUS_URI,
  FDE_CSSSYNTAXSTATUS_Selector,
  FDE_CSSSYNTAXSTATUS_DeclOpen,
  FDE_CSSSYNTAXSTATUS_DeclClose,
  FDE_CSSSYNTAXSTATUS_PropertyName,
  FDE_CSSSYNTAXSTATUS_PropertyValue,
};
class IFDE_CSSSyntaxParser {
 public:
  static IFDE_CSSSyntaxParser* Create();
  virtual ~IFDE_CSSSyntaxParser() {}
  virtual void Release() = 0;
  virtual FX_BOOL Init(IFX_Stream* pStream,
                       int32_t iCSSPlaneSize,
                       int32_t iTextDataSize = 32,
                       FX_BOOL bOnlyDeclaration = FALSE) = 0;
  virtual FX_BOOL Init(const FX_WCHAR* pBuffer,
                       int32_t iBufferSize,
                       int32_t iTextDatSize = 32,
                       FX_BOOL bOnlyDeclaration = FALSE) = 0;

  virtual FDE_CSSSYNTAXSTATUS DoSyntaxParse() = 0;
  virtual const FX_WCHAR* GetCurrentString(int32_t& iLength) const = 0;
};
enum FDE_CSSLENGTHUNIT {
  FDE_CSSLENGTHUNIT_Auto,
  FDE_CSSLENGTHUNIT_None,
  FDE_CSSLENGTHUNIT_Normal,
  FDE_CSSLENGTHUNIT_Point,
  FDE_CSSLENGTHUNIT_Percent,
};
#define FDE_CSSUNITBITS (3)
#define FDE_CSSUNITMASK ((1 << FDE_CSSUNITBITS) - 1)
struct FDE_CSSLENGTH {
  FDE_CSSLENGTH& Set(FDE_CSSLENGTHUNIT eUnit) {
    m_iData = eUnit;
    return *this;
  }
  FDE_CSSLENGTH& Set(FDE_CSSLENGTHUNIT eUnit, FX_FLOAT fValue) {
    m_iData = ((intptr_t)(fValue * 1024.0f) << FDE_CSSUNITBITS) | eUnit;
    return *this;
  }
  FDE_CSSLENGTHUNIT GetUnit() const {
    return (FDE_CSSLENGTHUNIT)(m_iData & FDE_CSSUNITMASK);
  }
  FX_FLOAT GetValue() const { return (m_iData >> FDE_CSSUNITBITS) / 1024.0f; }
  FX_BOOL NonZero() const { return (m_iData >> FDE_CSSUNITBITS) != 0; }

 private:
  intptr_t m_iData;
};
struct FDE_CSSPOINT {
  FDE_CSSPOINT& Set(FDE_CSSLENGTHUNIT eUnit) {
    x.Set(eUnit);
    y.Set(eUnit);
    return *this;
  }
  FDE_CSSPOINT& Set(FDE_CSSLENGTHUNIT eUnit, FX_FLOAT fValue) {
    x.Set(eUnit, fValue);
    y.Set(eUnit, fValue);
    return *this;
  }
  FDE_CSSLENGTH x, y;
};
struct FDE_CSSSIZE {
  FDE_CSSSIZE& Set(FDE_CSSLENGTHUNIT eUnit) {
    cx.Set(eUnit);
    cy.Set(eUnit);
    return *this;
  }
  FDE_CSSSIZE& Set(FDE_CSSLENGTHUNIT eUnit, FX_FLOAT fValue) {
    cx.Set(eUnit, fValue);
    cy.Set(eUnit, fValue);
    return *this;
  }
  FDE_CSSLENGTH cx, cy;
};
struct FDE_CSSRECT {
  FDE_CSSRECT& Set(FDE_CSSLENGTHUNIT eUnit) {
    left.Set(eUnit);
    top.Set(eUnit);
    right.Set(eUnit);
    bottom.Set(eUnit);
    return *this;
  }
  FDE_CSSRECT& Set(FDE_CSSLENGTHUNIT eUnit, FX_FLOAT fValue) {
    left.Set(eUnit, fValue);
    top.Set(eUnit, fValue);
    right.Set(eUnit, fValue);
    bottom.Set(eUnit, fValue);
    return *this;
  }

  FDE_CSSLENGTH left, top, right, bottom;
};
enum FDE_CSSBKGATTACHMENT {
  FDE_CSSBKGATTACHMENT_Scroll,
  FDE_CSSBKGATTACHMENT_Fixed,
};
enum FDE_CSSBKGREPEAT {
  FDE_CSSBKGREPEAT_Repeat,
  FDE_CSSBKGREPEAT_RepeatX,
  FDE_CSSBKGREPEAT_RepeatY,
  FDE_CSSBKGREPEAT_NoRepeat,
};
enum FDE_CSSBORDERSTYLE {
  FDE_CSSBORDERSTYLE_None,
  FDE_CSSBORDERSTYLE_Hidden,
  FDE_CSSBORDERSTYLE_Dotted,
  FDE_CSSBORDERSTYLE_Dashed,
  FDE_CSSBORDERSTYLE_Solid,
  FDE_CSSBORDERSTYLE_Double,
  FDE_CSSBORDERSTYLE_Groove,
  FDE_CSSBORDERSTYLE_Ridge,
  FDE_CSSBORDERSTYLE_Inset,
  FDE_CSSBORDERSTYLE_outset,
};
enum FDE_CSSCLEAR {
  FDE_CSSCLEAR_None,
  FDE_CSSCLEAR_Left,
  FDE_CSSCLEAR_Right,
  FDE_CSSCLEAR_Both,
};
enum FDE_CSSDISPLAY {
  FDE_CSSDISPLAY_None,
  FDE_CSSDISPLAY_ListItem,
  FDE_CSSDISPLAY_RunIn,
  FDE_CSSDISPLAY_Block,
  FDE_CSSDISPLAY_Inline,
  FDE_CSSDISPLAY_InlineBlock,
  FDE_CSSDISPLAY_InlineTable,
  FDE_CSSDISPLAY_Table,
  FDE_CSSDISPLAY_TableRow,
  FDE_CSSDISPLAY_TableCell,
  FDE_CSSDISPLAY_TableCaption,
  FDE_CSSDISPLAY_TableColumn,
  FDE_CSSDISPLAY_TableRowGroup,
  FDE_CSSDISPLAY_TableColumnGroup,
  FDE_CSSDISPLAY_TableHeaderGroup,
  FDE_CSSDISPLAY_TableFooterGroup,
  FDE_CSSDISPLAY_Ruby,
  FDE_CSSDISPLAY_RubyBase,
  FDE_CSSDISPLAY_RubyText,
  FDE_CSSDISPLSY_RubyBaseGroup,
  FDE_CSSDISPLAY_RubyTextGroup,
};
enum FDE_CSSVISIBILITY {
  FDE_CSSVISIBILITY_Visible,
  FDE_CSSVISIBILITY_Hidden,
  FDE_CSSVISIBILITY_Collapse,
};
enum FDE_CSSFONTSTYLE {
  FDE_CSSFONTSTYLE_Normal,
  FDE_CSSFONTSTYLE_Italic,
};
enum FDE_CSSFLOAT {
  FDE_CSSFLOAT_None,
  FDE_CSSFLOAT_Left,
  FDE_CSSFLOAT_Right,
};
enum FDE_CSSWRITINGMODE {
  FDE_CSSWRITINGMODE_HorizontalTb,
  FDE_CSSWRITINGMODE_VerticalRl,
  FDE_CSSWRITINGMODE_VerticalLr,
};
enum FDE_CSSWORDBREAK {
  FDE_CSSWORDBREAK_Normal,
  FDE_CSSWORDBREAK_KeepAll,
  FDE_CSSWORDBREAK_BreakAll,
  FDE_CSSWORDBREAK_KeepWords,
};
enum FDE_CSSPAGEBREAK {
  FDE_CSSPAGEBREAK_Auto,
  FDE_CSSPAGEBREAK_Always,
  FDE_CSSPAGEBREAK_Avoid,
  FDE_CSSPAGEBREAK_Left,
  FDE_CSSPAGEBREAK_Right,
};
enum FDE_CSSOVERFLOW {
  FDE_CSSOVERFLOW_Visible,
  FDE_CSSOVERFLOW_Hidden,
  FDE_CSSOVERFLOW_Scroll,
  FDE_CSSOVERFLOW_Auto,
  FDE_CSSOVERFLOW_NoDisplay,
  FDE_CSSOVERFLOW_NoContent,
};
enum FDE_CSSLINEBREAK {
  FDE_CSSLINEBREAK_Auto,
  FDE_CSSLINEBREAK_Loose,
  FDE_CSSLINEBREAK_Normal,
  FDE_CSSLINEBREAK_Strict,
};
enum FDE_CSSTEXTEMPHASISFILL {
  FDE_CSSTEXTEMPHASISFILL_Filled,
  FDE_CSSTEXTEMPHASISFILL_Open,
};
enum FDE_CSSTEXTEMPHASISMARK {
  FDE_CSSTEXTEMPHASISMARK_None,
  FDE_CSSTEXTEMPHASISMARK_Auto,
  FDE_CSSTEXTEMPHASISMARK_Dot,
  FDE_CSSTEXTEMPHASISMARK_Circle,
  FDE_CSSTEXTEMPHASISMARK_DoubleCircle,
  FDE_CSSTEXTEMPHASISMARK_Triangle,
  FDE_CSSTEXTEMPHASISMARK_Sesame,
  FDE_CSSTEXTEMPHASISMARK_Custom,
};
enum FDE_CSSTEXTCOMBINE {
  FDE_CSSTEXTCOMBINE_Horizontal,
  FDE_CSSTEXTCOMBINE_None,
};
enum FDE_CSSCURSOR {
  FDE_CSSCURSOR_Auto,
  FDE_CSSCURSOR_Crosshair,
  FDE_CSSCURSOR_Default,
  FDE_CSSCURSOR_Pointer,
  FDE_CSSCURSOR_Move,
  FDE_CSSCURSOR_EResize,
  FDE_CSSCURSOR_NeResize,
  FDE_CSSCURSOR_NwResize,
  FDE_CSSCURSOR_NResize,
  FDE_CSSCURSOR_SeResize,
  FDE_CSSCURSOR_SwResize,
  FDE_CSSCURSOR_SResize,
  FDE_CSSCURSOR_WResize,
  FDE_CSSCURSOR_Text,
  FDE_CSSCURSOR_Wait,
  FDE_CSSCURSOR_Help,
  FDE_CSSCURSOR_Progress,
};
enum FDE_CSSPOSITION {
  FDE_CSSPOSITION_Static,
  FDE_CSSPOSITION_Relative,
  FDE_CSSPOSITION_Absolute,
  FDE_CSSPOSITION_Fixed,
};
enum FDE_CSSCAPTIONSIDE {
  FDE_CSSCAPTIONSIDE_Top,
  FDE_CSSCAPTIONSIDE_Bottom,
  FDE_CSSCAPTIONSIDE_Left,
  FDE_CSSCAPTIONSIDE_Right,
  FDE_CSSCAPTIONSIDE_Before,
  FDE_CSSCAPTIONSIDE_After,
};
enum FDE_CSSRUBYALIGN {
  FDE_CSSRUBYALIGN_Auto,
  FDE_CSSRUBYALIGN_Start,
  FDE_CSSRUBYALIGN_Left,
  FDE_CSSRUBYALIGN_Center,
  FDE_CSSRUBYALIGN_End,
  FDE_CSSRUBYALIGN_Right,
  FDE_CSSRUBYALIGN_DistributeLetter,
  FDE_CSSRUBYALIGN_DistributeSpace,
  FDE_CSSRUBYALIGN_LineEdge,
};
enum FDE_CSSRUBYOVERHANG {
  FDE_CSSRUBYOVERHANG_Auto,
  FDE_CSSRUBYOVERHANG_Start,
  FDE_CSSRUBYOVERHANG_End,
  FDE_CSSRUBYOVERHANG_None,
};
enum FDE_CSSRUBYPOSITION {
  FDE_CSSRUBYPOSITION_Before,
  FDE_CSSRUBYPOSITION_After,
  FDE_CSSRUBYPOSITION_Right,
  FDE_CSSRUBYPOSITION_Inline,
};
enum FDE_CSSRUBYSPAN {
  FDE_CSSRUBYSPAN_None,
  FDE_CSSRUBYSPAN_Attr,
};
enum FDE_CSSTEXTALIGN {
  FDE_CSSTEXTALIGN_Left,
  FDE_CSSTEXTALIGN_Right,
  FDE_CSSTEXTALIGN_Center,
  FDE_CSSTEXTALIGN_Justify,
  FDE_CSSTEXTALIGN_JustifyAll,
};
enum FDE_CSSVERTICALALIGN {
  FDE_CSSVERTICALALIGN_Baseline,
  FDE_CSSVERTICALALIGN_Sub,
  FDE_CSSVERTICALALIGN_Super,
  FDE_CSSVERTICALALIGN_Top,
  FDE_CSSVERTICALALIGN_TextTop,
  FDE_CSSVERTICALALIGN_Middle,
  FDE_CSSVERTICALALIGN_Bottom,
  FDE_CSSVERTICALALIGN_TextBottom,
  FDE_CSSVERTICALALIGN_Number,
};
enum FDE_CSSLISTSTYLETYPE {
  FDE_CSSLISTSTYLETYPE_Disc,
  FDE_CSSLISTSTYLETYPE_Circle,
  FDE_CSSLISTSTYLETYPE_Square,
  FDE_CSSLISTSTYLETYPE_Decimal,
  FDE_CSSLISTSTYLETYPE_DecimalLeadingZero,
  FDE_CSSLISTSTYLETYPE_LowerRoman,
  FDE_CSSLISTSTYLETYPE_UpperRoman,
  FDE_CSSLISTSTYLETYPE_LowerGreek,
  FDE_CSSLISTSTYLETYPE_LowerLatin,
  FDE_CSSLISTSTYLETYPE_UpperLatin,
  FDE_CSSLISTSTYLETYPE_Armenian,
  FDE_CSSLISTSTYLETYPE_Georgian,
  FDE_CSSLISTSTYLETYPE_LowerAlpha,
  FDE_CSSLISTSTYLETYPE_UpperAlpha,
  FDE_CSSLISTSTYLETYPE_None,
  FDE_CSSLISTSTYLETYPE_CjkIdeographic,
  FDE_CSSLISTSTYLETYPE_Hebrew,
  FDE_CSSLISTSTYLETYPE_Hiragana,
  FDE_CSSLISTSTYLETYPE_HiraganaIroha,
  FDE_CSSLISTSTYLETYPE_Katakana,
  FDE_CSSLISTSTYLETYPE_KatakanaIroha,
};
enum FDE_CSSLISTSTYLEPOSITION {
  FDE_CSSLISTSTYLEPOSITION_Outside,
  FDE_CSSLISTSTYLEPOSITION_Inside,
};
enum FDE_CSSWHITESPACE {
  FDE_CSSWHITESPACE_Normal,
  FDE_CSSWHITESPACE_Pre,
  FDE_CSSWHITESPACE_Nowrap,
  FDE_CSSWHITESPACE_PreWrap,
  FDE_CSSWHITESPACE_PreLine,
};
enum FDE_CSSFONTVARIANT {
  FDE_CSSFONTVARIANT_Normal,
  FDE_CSSFONTVARIANT_SmallCaps,
};
enum FDE_CSSTEXTTRANSFORM {
  FDE_CSSTEXTTRANSFORM_None,
  FDE_CSSTEXTTRANSFORM_Capitalize,
  FDE_CSSTEXTTRANSFORM_UpperCase,
  FDE_CSSTEXTTRANSFORM_LowerCase,
};
enum FDE_CSSTEXTDECORATION {
  FDE_CSSTEXTDECORATION_None = 0,
  FDE_CSSTEXTDECORATION_Underline = 1,
  FDE_CSSTEXTDECORATION_Overline = 2,
  FDE_CSSTEXTDECORATION_LineThrough = 4,
  FDE_CSSTEXTDECORATION_Blink = 8,
  FDE_CSSTEXTDECORATION_Double = 16,
};
class IFDE_CSSRubyStyle {
 public:
  virtual ~IFDE_CSSRubyStyle() {}
  virtual FDE_CSSRUBYALIGN GetRubyAlign() const = 0;
  virtual FDE_CSSRUBYOVERHANG GetRubyOverhang() const = 0;
  virtual FDE_CSSRUBYPOSITION GetRubyPosition() const = 0;
  virtual FDE_CSSRUBYSPAN GetRubySpanType() const = 0;
  virtual IFDE_CSSValue* GetRubySpanAttr() const = 0;
};
class IFDE_CSSMultiColumnStyle {
 public:
  virtual ~IFDE_CSSMultiColumnStyle() {}
  virtual const FDE_CSSLENGTH& GetColumnCount() const = 0;
  virtual const FDE_CSSLENGTH& GetColumnGap() const = 0;
  virtual FX_ARGB GetColumnRuleColor() const = 0;
  virtual FDE_CSSBORDERSTYLE GetColumnRuleStyle() const = 0;
  virtual const FDE_CSSLENGTH& GetColumnRuleWidth() const = 0;
  virtual const FDE_CSSLENGTH& GetColumnWidth() const = 0;
  virtual void SetColumnCount(const FDE_CSSLENGTH& columnCount) = 0;
  virtual void SetColumnGap(const FDE_CSSLENGTH& columnGap) = 0;
  virtual void SetColumnRuleColor(FX_ARGB dwColumnRuleColor) = 0;
  virtual void SetColumnRuleStyle(FDE_CSSBORDERSTYLE eColumnRuleStyle) = 0;
  virtual void SetColumnRuleWidth(const FDE_CSSLENGTH& columnRuleWidth) = 0;
  virtual void SetColumnWidth(const FDE_CSSLENGTH& columnWidth) = 0;
};
class IFDE_CSSGeneratedContentStyle {
 public:
  virtual ~IFDE_CSSGeneratedContentStyle() {}
  virtual int32_t CountCounters() = 0;
  virtual const FX_WCHAR* GetCounterIdentifier(int32_t index) = 0;
  virtual FX_BOOL GetCounterReset(int32_t index, int32_t& iValue) = 0;
  virtual FX_BOOL GetCounterIncrement(int32_t index, int32_t& iValue) = 0;
  virtual IFDE_CSSValueList* GetContent() const = 0;
  virtual int32_t CountQuotes() const = 0;
  virtual const FX_WCHAR* GetQuotes(int32_t index) const = 0;
};
class IFDE_CSSFontStyle {
 public:
  virtual ~IFDE_CSSFontStyle() {}
  virtual int32_t CountFontFamilies() const = 0;
  virtual const FX_WCHAR* GetFontFamily(int32_t index) const = 0;
  virtual FX_WORD GetFontWeight() const = 0;
  virtual FDE_CSSFONTVARIANT GetFontVariant() const = 0;
  virtual FDE_CSSFONTSTYLE GetFontStyle() const = 0;
  virtual FX_FLOAT GetFontSize() const = 0;
  virtual FX_ARGB GetColor() const = 0;
  virtual void SetFontWeight(FX_WORD wFontWeight) = 0;
  virtual void SetFontVariant(FDE_CSSFONTVARIANT eFontVariant) = 0;
  virtual void SetFontStyle(FDE_CSSFONTSTYLE eFontStyle) = 0;
  virtual void SetFontSize(FX_FLOAT fFontSize) = 0;
  virtual void SetColor(FX_ARGB dwFontColor) = 0;
};
class IFDE_CSSBoundaryStyle {
 public:
  virtual ~IFDE_CSSBoundaryStyle() {}
  virtual FX_ARGB GetBorderLeftColor() const = 0;
  virtual FX_ARGB GetBorderTopColor() const = 0;
  virtual FX_ARGB GetBorderRightColor() const = 0;
  virtual FX_ARGB GetBorderBottomColor() const = 0;
  virtual FDE_CSSBORDERSTYLE GetBorderLeftStyle() const = 0;
  virtual FDE_CSSBORDERSTYLE GetBorderTopStyle() const = 0;
  virtual FDE_CSSBORDERSTYLE GetBorderRightStyle() const = 0;
  virtual FDE_CSSBORDERSTYLE GetBorderBottomStyle() const = 0;
  virtual const FDE_CSSRECT* GetBorderWidth() const = 0;
  virtual const FDE_CSSRECT* GetMarginWidth() const = 0;
  virtual const FDE_CSSRECT* GetPaddingWidth() const = 0;
  virtual void SetBorderLeftColor(FX_ARGB dwBorderColor) = 0;
  virtual void SetBorderTopColor(FX_ARGB dwBorderColor) = 0;
  virtual void SetBorderRightColor(FX_ARGB dwBorderColor) = 0;
  virtual void SetBorderBottomColor(FX_ARGB dwBorderColor) = 0;

  virtual void SetBorderLeftStyle(FDE_CSSBORDERSTYLE eBorderStyle) = 0;
  virtual void SetBorderTopStyle(FDE_CSSBORDERSTYLE eBorderStyle) = 0;
  virtual void SetBorderRightStyle(FDE_CSSBORDERSTYLE eBorderStyle) = 0;
  virtual void SetBorderBottomStyle(FDE_CSSBORDERSTYLE eBorderStyle) = 0;

  virtual void SetBorderWidth(const FDE_CSSRECT& rect) = 0;
  virtual void SetMarginWidth(const FDE_CSSRECT& rect) = 0;
  virtual void SetPaddingWidth(const FDE_CSSRECT& rect) = 0;
};
class IFDE_CSSPositionStyle {
 public:
  virtual ~IFDE_CSSPositionStyle() {}
  virtual FDE_CSSDISPLAY GetDisplay() const = 0;
  virtual const FDE_CSSSIZE& GetBoxSize() const = 0;
  virtual const FDE_CSSSIZE& GetMinBoxSize() const = 0;
  virtual const FDE_CSSSIZE& GetMaxBoxSize() const = 0;
  virtual FDE_CSSFLOAT GetFloat() const = 0;
  virtual FDE_CSSCLEAR GetClear() const = 0;
  virtual FDE_CSSPOSITION GetPosition() const = 0;
  virtual FDE_CSSLENGTH GetTop() const = 0;
  virtual FDE_CSSLENGTH GetBottom() const = 0;
  virtual FDE_CSSLENGTH GetLeft() const = 0;
  virtual FDE_CSSLENGTH GetRight() const = 0;
  virtual void SetDisplay(FDE_CSSDISPLAY eDisplay) = 0;
  virtual void SetBoxSize(const FDE_CSSSIZE& boxSize) = 0;
  virtual void SetMinBoxSize(const FDE_CSSSIZE& minBoxSize) = 0;
  virtual void SetMaxBoxSize(const FDE_CSSSIZE& maxBoxSize) = 0;
  virtual void SetFloat(FDE_CSSFLOAT eFloat) = 0;
  virtual void SetClear(FDE_CSSCLEAR eClear) = 0;
};
class IFDE_CSSParagraphStyle {
 public:
  virtual ~IFDE_CSSParagraphStyle() {}
  virtual FX_FLOAT GetLineHeight() const = 0;
  virtual FDE_CSSWHITESPACE GetWhiteSpace() const = 0;
  virtual const FDE_CSSLENGTH& GetTextIndent() const = 0;
  virtual FDE_CSSTEXTALIGN GetTextAlign() const = 0;
  virtual FDE_CSSVERTICALALIGN GetVerticalAlign() const = 0;
  virtual FX_FLOAT GetNumberVerticalAlign() const = 0;
  virtual FDE_CSSTEXTTRANSFORM GetTextTransform() const = 0;
  virtual FX_DWORD GetTextDecoration() const = 0;
  virtual const FDE_CSSLENGTH& GetLetterSpacing() const = 0;
  virtual const FDE_CSSLENGTH& GetWordSpacing() const = 0;
  virtual FDE_CSSWRITINGMODE GetWritingMode() const = 0;
  virtual FDE_CSSWORDBREAK GetWordBreak() const = 0;
  virtual int32_t GetWidows() const = 0;
  virtual FX_ARGB GetTextEmphasisColor() const = 0;
  virtual FDE_CSSPAGEBREAK GetPageBreakBefore() const = 0;
  virtual FDE_CSSPAGEBREAK GetPageBreakAfter() const = 0;
  virtual FDE_CSSPAGEBREAK GetPageBreakInside() const = 0;
  virtual int32_t GetOrphans() const = 0;
  virtual FDE_CSSLINEBREAK GetLineBreak() const = 0;
  virtual FDE_CSSTEXTEMPHASISMARK GetTextEmphasisMark() const = 0;
  virtual FDE_CSSTEXTEMPHASISFILL GetTextEmphasisFill() const = 0;
  virtual const FX_WCHAR* GetTextEmphasisCustom() const = 0;
  virtual FDE_CSSTEXTCOMBINE GetTextCombineType() const = 0;
  virtual FX_BOOL HasTextCombineNumber() const = 0;
  virtual FX_FLOAT GetTextCombineNumber() const = 0;
  virtual void SetLineHeight(FX_FLOAT fLineHeight) = 0;
  virtual void SetWhiteSpace(FDE_CSSWHITESPACE eWhiteSpace) = 0;
  virtual void SetTextIndent(const FDE_CSSLENGTH& textIndent) = 0;
  virtual void SetTextAlign(FDE_CSSTEXTALIGN eTextAlign) = 0;
  virtual void SetVerticalAlign(FDE_CSSVERTICALALIGN eVerticalAlign) = 0;
  virtual void SetNumberVerticalAlign(FX_FLOAT fAlign) = 0;
  virtual void SetTextTransform(FDE_CSSTEXTTRANSFORM eTextTransform) = 0;
  virtual void SetTextDecoration(FX_DWORD dwTextDecoration) = 0;
  virtual void SetLetterSpacing(const FDE_CSSLENGTH& letterSpacing) = 0;
  virtual void SetWordSpacing(const FDE_CSSLENGTH& wordSpacing) = 0;
  virtual void SetWritingMode(FDE_CSSWRITINGMODE eWritingMode) = 0;
  virtual void SetWordBreak(FDE_CSSWORDBREAK eWordBreak) = 0;
  virtual void SetWidows(int32_t iWidows) = 0;
  virtual void SetTextEmphasisColor(FX_ARGB dwTextEmphasisColor) = 0;
  virtual void SetPageBreakBefore(FDE_CSSPAGEBREAK ePageBreakBefore) = 0;
  virtual void SetPageBreakAfter(FDE_CSSPAGEBREAK ePageBreakAfter) = 0;
  virtual void SetPageBreakInside(FDE_CSSPAGEBREAK ePageBreakInside) = 0;
  virtual void SetOrphans(int32_t iOrphans) = 0;
  virtual void SetLineBreak(FDE_CSSLINEBREAK eLineBreak) = 0;
};
class IFDE_CSSBackgroundStyle {
 public:
  virtual ~IFDE_CSSBackgroundStyle() {}
  virtual FX_ARGB GetBKGColor() const = 0;
  virtual const FX_WCHAR* GetBKGImage() const = 0;
  virtual FDE_CSSBKGREPEAT GetBKGRepeat() const = 0;
  virtual FDE_CSSBKGATTACHMENT GetBKGAttachment() const = 0;
  virtual const FDE_CSSPOINT& GetBKGPosition() const = 0;
  virtual void SetBKGColor(FX_ARGB dwBKGColor) = 0;
  virtual void SetBKGPosition(const FDE_CSSPOINT& bkgPosition) = 0;
};
class IFDE_CSSListStyle {
 public:
  virtual ~IFDE_CSSListStyle() {}
  virtual FDE_CSSLISTSTYLETYPE GetListStyleType() const = 0;
  virtual FDE_CSSLISTSTYLEPOSITION GetListStylePosition() const = 0;
  virtual const FX_WCHAR* GetListStyleImage() const = 0;
  virtual void SetListStyleType(FDE_CSSLISTSTYLETYPE eListStyleType) = 0;
  virtual void SetListStylePosition(
      FDE_CSSLISTSTYLEPOSITION eListStylePosition) = 0;
};
class IFDE_CSSTableStyle {
 public:
  virtual ~IFDE_CSSTableStyle() {}
  virtual FDE_CSSCAPTIONSIDE GetCaptionSide() const = 0;
};
class IFDE_CSSVisualStyle {
 public:
  virtual ~IFDE_CSSVisualStyle() {}
  virtual FDE_CSSVISIBILITY GetVisibility() const = 0;
  virtual FDE_CSSOVERFLOW GetOverflowX() const = 0;
  virtual FDE_CSSOVERFLOW GetOverflowY() const = 0;
  virtual void SetVisibility(FDE_CSSVISIBILITY eVisibility) = 0;
};
class IFDE_CSSComputedStyle : public IFX_Unknown {
 public:
  virtual void Reset() = 0;
  virtual IFDE_CSSFontStyle* GetFontStyles() const = 0;
  virtual IFDE_CSSBoundaryStyle* GetBoundaryStyles() const = 0;
  virtual IFDE_CSSPositionStyle* GetPositionStyles() const = 0;
  virtual IFDE_CSSParagraphStyle* GetParagraphStyles() const = 0;
  virtual IFDE_CSSBackgroundStyle* GetBackgroundStyles() const = 0;
  virtual IFDE_CSSVisualStyle* GetVisualStyles() const = 0;
  virtual IFDE_CSSListStyle* GetListStyles() const = 0;
  virtual IFDE_CSSMultiColumnStyle* GetMultiColumnStyle() const = 0;
  virtual IFDE_CSSTableStyle* GetTableStyle() const = 0;
  virtual IFDE_CSSGeneratedContentStyle* GetGeneratedContentStyle() const = 0;
  virtual IFDE_CSSRubyStyle* GetRubyStyle() const = 0;
  virtual FX_BOOL GetCustomStyle(const CFX_WideStringC& wsName,
                                 CFX_WideString& wsValue) const = 0;
};
enum FDE_CSSSTYLESHEETGROUP {
  FDE_CSSSTYLESHEETGROUP_UserAgent,
  FDE_CSSSTYLESHEETGROUP_User,
  FDE_CSSSTYLESHEETGROUP_Author,
  FDE_CSSSTYLESHEETGROUP_MAX,
};
enum FDE_CSSSTYLESHEETPRIORITY {
  FDE_CSSSTYLESHEETPRIORITY_High,
  FDE_CSSSTYLESHEETPRIORITY_Mid,
  FDE_CSSSTYLESHEETPRIORITY_Low,
  FDE_CSSSTYLESHEETPRIORITY_MAX,
};
class IFDE_CSSTagProvider {
 public:
  virtual ~IFDE_CSSTagProvider() {}
  virtual CFX_WideStringC GetTagName() = 0;
  virtual FX_POSITION GetFirstAttribute() = 0;
  virtual void GetNextAttribute(FX_POSITION& pos,
                                CFX_WideStringC& wsAttr,
                                CFX_WideStringC& wsValue) = 0;
};
class IFDE_CSSAccelerator {
 public:
  virtual ~IFDE_CSSAccelerator() {}
  virtual void OnEnterTag(IFDE_CSSTagProvider* pTag) = 0;
  virtual void OnLeaveTag(IFDE_CSSTagProvider* pTag) = 0;
};
class IFDE_CSSStyleSelector {
 public:
  static IFDE_CSSStyleSelector* Create();
  virtual ~IFDE_CSSStyleSelector() {}
  virtual void Release() = 0;
  virtual void SetFontMgr(IFX_FontMgr* pFontMgr) = 0;
  virtual void SetDefFontSize(FX_FLOAT fFontSize) = 0;
  virtual FX_BOOL SetStyleSheet(FDE_CSSSTYLESHEETGROUP eType,
                                IFDE_CSSStyleSheet* pSheet) = 0;
  virtual FX_BOOL SetStyleSheets(FDE_CSSSTYLESHEETGROUP eType,
                                 const CFDE_CSSStyleSheetArray* pArray) = 0;
  virtual void SetStylePriority(FDE_CSSSTYLESHEETGROUP eType,
                                FDE_CSSSTYLESHEETPRIORITY ePriority) = 0;
  virtual void UpdateStyleIndex(FX_DWORD dwMediaList) = 0;
  virtual IFDE_CSSAccelerator* InitAccelerator() = 0;
  virtual IFDE_CSSComputedStyle* CreateComputedStyle(
      IFDE_CSSComputedStyle* pParentStyle) = 0;
  virtual int32_t MatchDeclarations(
      IFDE_CSSTagProvider* pTag,
      CFDE_CSSDeclarationArray& matchedDecls,
      FDE_CSSPERSUDO ePersudoType = FDE_CSSPERSUDO_NONE) = 0;
  virtual void ComputeStyle(IFDE_CSSTagProvider* pTag,
                            const IFDE_CSSDeclaration** ppDeclArray,
                            int32_t iDeclCount,
                            IFDE_CSSComputedStyle* pDestStyle) = 0;
};
#endif
