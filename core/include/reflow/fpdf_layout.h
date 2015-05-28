// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_REFLOW_FPDF_LAYOUT_H_
#define CORE_INCLUDE_REFLOW_FPDF_LAYOUT_H_

class IPDF_LayoutElement;
class IPDF_LayoutProcessor;
class IPDF_LayoutProvider;
typedef enum {
    LayoutUnknown,
    LayoutArifact,
    LayoutDocument,
    LayoutPart,
    LayoutArt,
    LayoutSect,
    LayoutDiv,
    LayoutBlockQuote,
    LayoutCaption,
    LayoutTOC,
    LayoutTOCI,
    LayoutIndex,
    LayoutNonStruct,
    LayoutPrivate,
    LayoutParagraph,
    LayoutHeading,
    LayoutHeading1,
    LayoutHeading2,
    LayoutHeading3,
    LayoutHeading4,
    LayoutHeading5,
    LayoutHeading6,
    LayoutList,
    LayoutListItem,
    LayoutListLabel,
    LayoutListBody,
    LayoutTable,
    LayoutTableRow,
    LayoutTableHeaderCell,
    LayoutTableDataCell,
    LayoutTableHeaderGroup,
    LayoutTableBodyGroup,
    LayoutTableFootGroup,
    LayoutSpan,
    LayoutQuote,
    LayoutNote,
    LayoutReference,
    LayoutBibEntry,
    LayoutCode,
    LayoutLink,
    LayoutAnnot,
    LayoutRuby,
    LayoutRubyBase,
    LayoutRubyAnnot,
    LayoutRubyPunc,
    LayoutWarichu,
    LayoutWarichuText,
    LayoutWarichuPunc,
    LayoutFigure,
    LayoutFormula,
    LayoutForm,
} LayoutType;
typedef enum {
    LayoutArtifactType,
    LayoutArtifactAttached,
    LayoutArtifactSubType,
    LayoutPlacement,
    LayoutWritingMode,
    LayoutBackgroundColor,
    LayoutBorderColor,
    LayoutBorderStyle,
    LayoutBorderThickness,
    LayoutPadding,
    LayoutColor,
    LayoutSpaceBefore,
    LayoutSpaceAfter,
    LayoutStartIndent,
    LayoutEndIndent,
    LayoutTextIndent,
    LayoutTextAlign,
    LayoutBBox,
    LayoutWidth,
    LayoutHeight,
    LayoutBlockAlign,
    LayoutInlineAlign,
    LayoutTBorderStyle,
    LayoutTPadding,
    LayoutBaselineShift,
    LayoutLineHeight,
    LayoutTextDecorationColor,
    LayoutTextDecorationThickness,
    LayoutTextDecorationType,
    LayoutRubyAlign,
    LayoutRubyPosition,
    LayoutGlyphOrientationVertical,
    LayoutColumnCount,
    LayoutColumnGap,
    LayoutColumnWidths,
    LayoutListNumbering,
    LayoutFieldRole,
    LayoutFieldChecked,
    LayoutFieldDesc,
    LayoutRowSpan,
    LayoutColSpan,
    LayoutTableHeaders,
    LayoutTableHeaderScope,
    LayoutTableSummary,
} LayoutAttr;
typedef enum {
    LayoutInvalid = 0,
    LayoutBlock,
    LayoutInline,
    LayoutBefore,
    LayoutAfter,
    LayoutStart,
    LayoutEnd,
    LayoutLrTb,
    LayoutRlTb,
    LayoutTbRl,
    LayoutNone,
    LayoutHidden,
    LayoutDotted,
    LayoutDashed,
    LayoutSolid,
    LayoutDouble,
    LayoutGroove,
    LayoutRidge,
    LayoutInset,
    LayoutOutset,
    LayoutNormal,
    LayoutAuto,
    LayoutCenter,
    LayoutJustify,
    LayoutMiddle,
    LayoutUnderline,
    LayoutOverline,
    LayoutLineThrough,
    LayoutDistribute,
    LayoutMinus90Degree,
    LayoutZeroDegree,
    Layout90Degree,
    Layout180Degree,
    Layout270Degree,
    LayoutDisc,
    LayoutCircle,
    LayoutSquare,
    LayoutDecimal,
    LayoutUpperRoman,
    LayoutLowerRoman,
    LayoutUpperAlpha,
    LayoutLowerAlpha,
    LayoutRB,
    LayoutCB,
    LayoutPB,
    LayoutTV,
    LayoutOn,
    LayoutOff,
    LayoutNeutral,
    LayoutRow,
    LayoutColumn,
    LayoutBoth,
    LayoutLeft,
    LayoutTop,
    LayoutBottom,
    LayoutRight,
    LayoutPagination,
    LayoutLayout,
    LayoutPage,
    LayoutBackground,
    LayoutHeader,
    LayoutFooter,
    LayoutWatermark,
} LayoutEnum;
class IPDF_LayoutElement
{
public:

    virtual ~IPDF_LayoutElement() {};


    virtual LayoutType	GetType() = 0;

    virtual int		CountAttrValues(LayoutAttr attr_type) = 0;


    virtual LayoutEnum	GetEnumAttr(LayoutAttr attr_type, int index = 0) = 0;

    virtual FX_FLOAT	GetNumberAttr(LayoutAttr attr_type, int index = 0) = 0;

    virtual FX_COLORREF	GetColorAttr(LayoutAttr attr_type, int index = 0) = 0;


    virtual int		CountChildren() = 0;


    virtual IPDF_LayoutElement* GetChild(int index) = 0;


    virtual IPDF_LayoutElement* GetParent() = 0;


    virtual int		CountObjects() = 0;

    virtual CPDF_PageObject*	GetObject(int index) = 0;
};
typedef enum {
    LayoutReady,
    LayoutFinished,
    LayoutToBeContinued,
    LayoutError
} LayoutStatus;
#define RF_PARSER_IMAGE		0x1
#define RF_PARSER_DEBUGINFO	0x2
#define RF_PARSER_PAGEMODE	0x4
#define RF_PARSER_READERORDER	0x8
class IPDF_LayoutProcessor
{
public:

    virtual ~IPDF_LayoutProcessor() {};

    static IPDF_LayoutProcessor* Create_LayoutProcessor_Reflow(FX_FLOAT TopIndent, FX_FLOAT fWidth, FX_FLOAT fHeight, void* pReflowedPage, int flags, FX_FLOAT lineSpace = 0);

    static IPDF_LayoutProcessor* Create_LayoutProcessor_2HTML(FX_LPCSTR fileName);

    virtual LayoutStatus	StartProcess(IPDF_LayoutElement* pElement, IFX_Pause* pPause, const CFX_AffineMatrix* pPDFMatrix = NULL) = 0;

    virtual LayoutStatus	Continue() = 0;
    virtual int				GetPosition() = 0;
};
#define LP_Lang_Unknow	 		0x0
#define LP_Lang_English	 		0x1
#define LP_Lang_French			0x2
#define LP_Lang_Italian			0x4
#define LP_Lang_German			0x8
#define LP_Lang_Spanish			0x10
#define LP_Lang_Polish			0x20
#define LP_Lang_Russian			0x40
#define LP_Lang_ChinesePRC		0x80
#define LP_Lang_ChineseTaiwan	0x100
#define LP_Lang_Japanese		0x200
#define LP_Lang_Korean			0x400
#define LP_Lang_Portuguese		0x800
#define LP_Lang_Turkish			0x1000
#define LP_Lang_Dutch			0x2000
typedef struct _LayoutProviderStyle {
    _LayoutProviderStyle()
    {
        m_Language = LP_Lang_Unknow;
        m_bIgnoreInvisibleText = TRUE;
    }
    FX_INT32	m_Language;
    FX_BOOL		m_bIgnoreInvisibleText;
} LAYOUTPROVIDER_STYLE;
class IPDF_LayoutProvider
{
public:

    virtual ~IPDF_LayoutProvider() {};

    static IPDF_LayoutProvider* Create_LayoutProvider_TaggedPDF(CPDF_PageObjects* pPageObjs);

    static IPDF_LayoutProvider* Create_LayoutProvider_AutoReflow(CPDF_PageObjects* pPageObjs, FX_BOOL bReadOrder);

    virtual void			SetLayoutProviderStyle(LAYOUTPROVIDER_STYLE Style) = 0;




    virtual LayoutStatus	StartLoad(IFX_Pause* pPause = NULL) = 0;

    virtual LayoutStatus	Continue() = 0;
    virtual int				GetPosition() = 0;


    virtual IPDF_LayoutElement* GetRoot() = 0;
};
#endif  // CORE_INCLUDE_REFLOW_FPDF_LAYOUT_H_
