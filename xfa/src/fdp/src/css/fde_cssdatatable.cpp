// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "fde_cssdatatable.h"
#ifdef _cplusplus
extern "C" {
#endif
    FX_BOOL FDE_CSSLengthToFloat(const FDE_CSSLENGTH &len, FX_FLOAT fPercentBase, FX_FLOAT &fResult)
    {
        switch (len.GetUnit()) {
            case FDE_CSSLENGTHUNIT_Point:
                fResult = len.GetValue();
                return TRUE;
            case FDE_CSSLENGTHUNIT_Percent:
                fResult = len.GetValue() * fPercentBase;
                return TRUE;
            default:
                return FALSE;
        }
    }
    CFX_FloatRect FDE_CSSBoundaryToRect(IFDE_CSSBoundaryStyle *pBoundStyle, FX_FLOAT fContainerWidth, FX_BOOL bPadding, FX_BOOL bBorder, FX_BOOL bMargin)
    {
        FXSYS_assert(pBoundStyle != NULL);
        FX_FLOAT fResult;
        const FDE_CSSRECT *pRect;
        CFX_FloatRect rect(0, 0, 0, 0);
        if (bPadding) {
            pRect = pBoundStyle->GetPaddingWidth();
            if (pRect != NULL) {
                if (FDE_CSSLengthToFloat(pRect->left, fContainerWidth, fResult)) {
                    rect.left += fResult;
                }
                if (FDE_CSSLengthToFloat(pRect->top, fContainerWidth, fResult)) {
                    rect.top += fResult;
                }
                if (FDE_CSSLengthToFloat(pRect->right, fContainerWidth, fResult)) {
                    rect.right += fResult;
                }
                if (FDE_CSSLengthToFloat(pRect->bottom, fContainerWidth, fResult)) {
                    rect.bottom += fResult;
                }
            }
        }
        if (bBorder) {
            pRect = pBoundStyle->GetBorderWidth();
            if (pRect != NULL) {
                if (FDE_CSSLengthToFloat(pRect->left, fContainerWidth, fResult)) {
                    rect.left += fResult;
                }
                if (FDE_CSSLengthToFloat(pRect->top, fContainerWidth, fResult)) {
                    rect.top += fResult;
                }
                if (FDE_CSSLengthToFloat(pRect->right, fContainerWidth, fResult)) {
                    rect.right += fResult;
                }
                if (FDE_CSSLengthToFloat(pRect->bottom, fContainerWidth, fResult)) {
                    rect.bottom += fResult;
                }
            }
        }
        if (bMargin) {
            pRect = pBoundStyle->GetMarginWidth();
            if (pRect != NULL) {
                if (FDE_CSSLengthToFloat(pRect->left, fContainerWidth, fResult)) {
                    rect.left += fResult;
                }
                if (FDE_CSSLengthToFloat(pRect->top, fContainerWidth, fResult)) {
                    rect.top += fResult;
                }
                if (FDE_CSSLengthToFloat(pRect->right, fContainerWidth, fResult)) {
                    rect.right += fResult;
                }
                if (FDE_CSSLengthToFloat(pRect->bottom, fContainerWidth, fResult)) {
                    rect.bottom += fResult;
                }
            }
        }
        return rect;
    }
    FX_DWORD FDE_CSSFontStyleToFDE(IFDE_CSSFontStyle *pFontStyle)
    {
        FXSYS_assert(pFontStyle != NULL);
        FX_DWORD dwFontStyle = FX_FONTSTYLE_Normal;
        if (pFontStyle->GetFontStyle() == FDE_CSSFONTSTYLE_Italic) {
            dwFontStyle |= FX_FONTSTYLE_Italic;
        }
        if (pFontStyle->GetFontWeight() >= 700) {
            dwFontStyle |= FX_FONTSTYLE_Bold;
        }
        return dwFontStyle;
    }
    static const FDE_CSSPROPERTYTABLE g_FDE_CSSProperties[] = {
        {FDE_CSSPROPERTY_WritingMode,	(FX_LPCWSTR)L"writing-mode",	0x01878076,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_ColumnRuleWidth,	(FX_LPCWSTR)L"column-rule-width",	0x0200FB00,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_BorderLeft,	(FX_LPCWSTR)L"border-left",	0x04080036,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_ColumnRule,	(FX_LPCWSTR)L"column-rule",	0x04C83DF3,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_Height,	(FX_LPCWSTR)L"height",	0x05A5C519,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_CounterReset,	(FX_LPCWSTR)L"counter-reset",	0x0894F9B0,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber | FDE_CSSVALUETYPE_MaybeString},
        {FDE_CSSPROPERTY_Content,	(FX_LPCWSTR)L"content",	0x097BE91B,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeURI | FDE_CSSVALUETYPE_MaybeString},
        {FDE_CSSPROPERTY_RubyPosition,	(FX_LPCWSTR)L"ruby-position",	0x09ACD024,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BackgroundColor,	(FX_LPCWSTR)L"background-color",	0x09E8E8AC,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeColor},
        {FDE_CSSPROPERTY_Width,	(FX_LPCWSTR)L"width",	0x0A8A8F80,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_Src,	(FX_LPCWSTR)L"src",	0x0BD37048,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeURI},
        {FDE_CSSPROPERTY_Top,	(FX_LPCWSTR)L"top",	0x0BEDAF33,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_Margin,	(FX_LPCWSTR)L"margin",	0x0CB016BE,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_BorderColor,	(FX_LPCWSTR)L"border-color",	0x0CBB528A,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeColor},
        {FDE_CSSPROPERTY_Widows,	(FX_LPCWSTR)L"widows",	0x1026C59D,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_BorderBottomColor,	(FX_LPCWSTR)L"border-bottom-color",	0x121E22EC,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeColor},
        {FDE_CSSPROPERTY_TextIndent,	(FX_LPCWSTR)L"text-indent",	0x169ADB74,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_Right,	(FX_LPCWSTR)L"right",	0x193ADE3E,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_TextEmphasisStyle,	(FX_LPCWSTR)L"text-emphasis-style",	0x20DBAF4A,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeString},
        {FDE_CSSPROPERTY_PaddingLeft,	(FX_LPCWSTR)L"padding-left",	0x228CF02F,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_ColumnWidth,	(FX_LPCWSTR)L"column-width",	0x24C9AC9B,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_MarginLeft,	(FX_LPCWSTR)L"margin-left",	0x297C5656,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_Border,	(FX_LPCWSTR)L"border",	0x2A23349E,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_BorderTop,	(FX_LPCWSTR)L"border-top",	0x2B866ADE,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_RubyOverhang,	(FX_LPCWSTR)L"ruby-overhang",	0x2CCA0D89,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_PageBreakBefore,	(FX_LPCWSTR)L"page-break-before",	0x3119B36F,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_MaxHeight,	(FX_LPCWSTR)L"max-height",	0x343597EC,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_MinWidth,	(FX_LPCWSTR)L"min-width",	0x35832871,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_BorderLeftColor,	(FX_LPCWSTR)L"border-left-color",	0x35C64022,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeColor},
        {FDE_CSSPROPERTY_Bottom,	(FX_LPCWSTR)L"bottom",	0x399F02B5,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_Quotes,	(FX_LPCWSTR)L"quotes",	0x3D8C6A01,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeString},
        {FDE_CSSPROPERTY_MaxWidth,	(FX_LPCWSTR)L"max-width",	0x3EA274F3,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_PaddingRight,	(FX_LPCWSTR)L"padding-right",	0x3F616AC2,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_ListStyleImage,	(FX_LPCWSTR)L"list-style-image",	0x42A8A86A,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeURI},
        {FDE_CSSPROPERTY_WhiteSpace,	(FX_LPCWSTR)L"white-space",	0x42F0429A,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BorderBottom,	(FX_LPCWSTR)L"border-bottom",	0x452CE780,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_ListStyleType,	(FX_LPCWSTR)L"list-style-type",	0x48094789,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_WordBreak,	(FX_LPCWSTR)L"word-break",	0x4D74A3CE,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_OverflowX,	(FX_LPCWSTR)L"overflow-x",	0x4ECEBF99,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_OverflowY,	(FX_LPCWSTR)L"overflow-y",	0x4ECEBF9A,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BorderTopColor,	(FX_LPCWSTR)L"border-top-color",	0x5109B8CA,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeColor},
        {FDE_CSSPROPERTY_FontFamily,	(FX_LPCWSTR)L"font-family",	0x574686E6,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeString},
        {FDE_CSSPROPERTY_Cursor,	(FX_LPCWSTR)L"cursor",	0x59DFCA5E,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeString},
        {FDE_CSSPROPERTY_RubyAlign,	(FX_LPCWSTR)L"ruby-align",	0x6077BDFA,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_ColumnRuleColor,	(FX_LPCWSTR)L"column-rule-color",	0x65DDFD9F,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeColor},
        {FDE_CSSPROPERTY_FontWeight,	(FX_LPCWSTR)L"font-weight",	0x6692F60C,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_BorderRightStyle,	(FX_LPCWSTR)L"border-right-style",	0x6920DDA7,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_MinHeight,	(FX_LPCWSTR)L"min-height",	0x6AAE312A,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_Color,	(FX_LPCWSTR)L"color",	0x6E67921F,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeColor},
        {FDE_CSSPROPERTY_LetterSpacing,	(FX_LPCWSTR)L"letter-spacing",	0x70536102,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_EmptyCells,	(FX_LPCWSTR)L"empty-cells",	0x7531528F,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_TextAlign,	(FX_LPCWSTR)L"text-align",	0x7553F1BD,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_RubySpan,	(FX_LPCWSTR)L"ruby-span",	0x76FCFCE1,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeFunction},
        {FDE_CSSPROPERTY_Position,	(FX_LPCWSTR)L"position",	0x814F82B5,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BorderStyle,	(FX_LPCWSTR)L"border-style",	0x82A4CD5C,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BorderBottomStyle,	(FX_LPCWSTR)L"border-bottom-style",	0x88079DBE,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BorderCollapse,	(FX_LPCWSTR)L"border-collapse",	0x8883C7FE,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_ColumnCount,	(FX_LPCWSTR)L"column-count",	0x89936A64,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_BorderRightWidth,	(FX_LPCWSTR)L"border-right-width",	0x8F5A6036,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_UnicodeBidi,	(FX_LPCWSTR)L"unicode-bidi",	0x91670F6C,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_VerticalAlign,	(FX_LPCWSTR)L"vertical-align",	0x934A87D2,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_PaddingTop,	(FX_LPCWSTR)L"padding-top",	0x959D22B7,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_Columns,	(FX_LPCWSTR)L"columns",	0x96FA5D81,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_Overflow,	(FX_LPCWSTR)L"overflow",	0x97B76B54,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_TableLayout,	(FX_LPCWSTR)L"table-layout",	0x9B1CB4B3,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_FontVariant,	(FX_LPCWSTR)L"font-variant",	0x9C785779,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_ListStyle,	(FX_LPCWSTR)L"list-style",	0x9E6C471A,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_BackgroundPosition,	(FX_LPCWSTR)L"background-position",	0xA8846D22,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_BorderWidth,	(FX_LPCWSTR)L"border-width",	0xA8DE4FEB,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_TextEmphasisColor,	(FX_LPCWSTR)L"text-emphasis-color",	0xAAF23478,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeColor},
        {FDE_CSSPROPERTY_BorderLeftStyle,	(FX_LPCWSTR)L"border-left-style",	0xABAFBAF4,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_PageBreakInside,	(FX_LPCWSTR)L"page-break-inside",	0xACB695F8,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_TextEmphasis,	(FX_LPCWSTR)L"text-emphasis",	0xAD0E580C,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_BorderBottomWidth,	(FX_LPCWSTR)L"border-bottom-width",	0xAE41204D,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_ColumnGap,	(FX_LPCWSTR)L"column-gap",	0xB5C1BA73,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_Orphans,	(FX_LPCWSTR)L"orphans",	0xB716467B,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_BorderRight,	(FX_LPCWSTR)L"border-right",	0xB78E9EA9,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_FontSize,	(FX_LPCWSTR)L"font-size",	0xB93956DF,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_PageBreakAfter,	(FX_LPCWSTR)L"page-break-after",	0xBC358AEE,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_CaptionSide,	(FX_LPCWSTR)L"caption-side",	0xC03F3560,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BackgroundRepeat,	(FX_LPCWSTR)L"background-repeat",	0xC2C2FDCE,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BorderTopStyle,	(FX_LPCWSTR)L"border-top-style",	0xC6F3339C,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BorderSpacing,	(FX_LPCWSTR)L"border-spacing",	0xC72030F0,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_TextTransform,	(FX_LPCWSTR)L"text-transform",	0xC88EEA6E,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_FontStyle,	(FX_LPCWSTR)L"font-style",	0xCB1950F5,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_Font,	(FX_LPCWSTR)L"font",	0xCD308B77,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_LineHeight,	(FX_LPCWSTR)L"line-height",	0xCFCACE2E,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_MarginRight,	(FX_LPCWSTR)L"margin-right",	0xD13C58C9,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_Float,	(FX_LPCWSTR)L"float",	0xD1532876,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BorderLeftWidth,	(FX_LPCWSTR)L"border-left-width",	0xD1E93D83,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_Display,	(FX_LPCWSTR)L"display",	0xD4224C36,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_Clear,	(FX_LPCWSTR)L"clear",	0xD8ED1467,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_ColumnRuleStyle,	(FX_LPCWSTR)L"column-rule-style",	0xDBC77871,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_TextCombine,	(FX_LPCWSTR)L"text-combine",	0xDC5207CF,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_ListStylePosition,	(FX_LPCWSTR)L"list-style-position",	0xE1A1DE3C,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_Visibility,	(FX_LPCWSTR)L"visibility",	0xE29F5168,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_PaddingBottom,	(FX_LPCWSTR)L"padding-bottom",	0xE555B3B9,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_BackgroundAttachment,	(FX_LPCWSTR)L"background-attachment",	0xE77981F6,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_BackgroundImage,	(FX_LPCWSTR)L"background-image",	0xE9AEB710,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeURI},
        {FDE_CSSPROPERTY_LineBreak,	(FX_LPCWSTR)L"line-break",	0xEA2D1D9A,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_Background,	(FX_LPCWSTR)L"background",	0xEB49DD40,	FDE_CSSVALUETYPE_Shorthand},
        {FDE_CSSPROPERTY_BorderTopWidth,	(FX_LPCWSTR)L"border-top-width",	0xED2CB62B,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_WordSpacing,	(FX_LPCWSTR)L"word-spacing",	0xEDA63BAE,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_BorderRightColor,	(FX_LPCWSTR)L"border-right-color",	0xF33762D5,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeColor},
        {FDE_CSSPROPERTY_CounterIncrement,	(FX_LPCWSTR)L"counter-increment",	0xF4CFB1B2,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber | FDE_CSSVALUETYPE_MaybeString},
        {FDE_CSSPROPERTY_Left,	(FX_LPCWSTR)L"left",	0xF5AD782B,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_TextDecoration,	(FX_LPCWSTR)L"text-decoration",	0xF7C634BA,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_Padding,	(FX_LPCWSTR)L"padding",	0xF8C373F7,	FDE_CSSVALUETYPE_List | FDE_CSSVALUETYPE_MaybeNumber},
        {FDE_CSSPROPERTY_MarginBottom,	(FX_LPCWSTR)L"margin-bottom",	0xF93485A0,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_MarginTop,	(FX_LPCWSTR)L"margin-top",	0xFE51DCFE,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeNumber | FDE_CSSVALUETYPE_MaybeEnum},
        {FDE_CSSPROPERTY_Direction,	(FX_LPCWSTR)L"direction",	0xFE746E61,	FDE_CSSVALUETYPE_Primitive | FDE_CSSVALUETYPE_MaybeEnum},
    };
    static const FDE_CSSPROPERTYVALUETABLE g_FDE_CSSPropertyValues[] = {
        {FDE_CSSPROPERTYVALUE_Bolder,	(FX_LPCWSTR)L"bolder",	0x009F1058},
        {FDE_CSSPROPERTYVALUE_LowerLatin,	(FX_LPCWSTR)L"lower-latin",	0x016014CE},
        {FDE_CSSPROPERTYVALUE_Lowercase,	(FX_LPCWSTR)L"lowercase",	0x02ACB805},
        {FDE_CSSPROPERTYVALUE_LowerGreek,	(FX_LPCWSTR)L"lower-greek",	0x03D81D64},
        {FDE_CSSPROPERTYVALUE_Sesame,	(FX_LPCWSTR)L"sesame",	0x0432ECDE},
        {FDE_CSSPROPERTYVALUE_None,	(FX_LPCWSTR)L"none",	0x048B6670},
        {FDE_CSSPROPERTYVALUE_NwResize,	(FX_LPCWSTR)L"nw-resize",	0x054B4BE4},
        {FDE_CSSPROPERTYVALUE_WResize,	(FX_LPCWSTR)L"w-resize",	0x0A2F8D76},
        {FDE_CSSPROPERTYVALUE_Dot,	(FX_LPCWSTR)L"dot",	0x0A48CB27},
        {FDE_CSSPROPERTYVALUE_End,	(FX_LPCWSTR)L"end",	0x0A631437},
        {FDE_CSSPROPERTYVALUE_Ltr,	(FX_LPCWSTR)L"ltr",	0x0B1B56D2},
        {FDE_CSSPROPERTYVALUE_Pre,	(FX_LPCWSTR)L"pre",	0x0B848587},
        {FDE_CSSPROPERTYVALUE_Rtl,	(FX_LPCWSTR)L"rtl",	0x0BB92C52},
        {FDE_CSSPROPERTYVALUE_Sub,	(FX_LPCWSTR)L"sub",	0x0BD37FAA},
        {FDE_CSSPROPERTYVALUE_Top,	(FX_LPCWSTR)L"top",	0x0BEDAF33},
        {FDE_CSSPROPERTYVALUE_Visible,	(FX_LPCWSTR)L"visible",	0x0F55D7EE},
        {FDE_CSSPROPERTYVALUE_Filled,	(FX_LPCWSTR)L"filled",	0x10827DD0},
        {FDE_CSSPROPERTYVALUE_SwResize,	(FX_LPCWSTR)L"sw-resize",	0x10B548E9},
        {FDE_CSSPROPERTYVALUE_NoRepeat,	(FX_LPCWSTR)L"no-repeat",	0x1235C18B},
        {FDE_CSSPROPERTYVALUE_Default,	(FX_LPCWSTR)L"default",	0x14DA2125},
        {FDE_CSSPROPERTYVALUE_Transparent,	(FX_LPCWSTR)L"transparent",	0x17B64DB2},
        {FDE_CSSPROPERTYVALUE_Ridge,	(FX_LPCWSTR)L"ridge",	0x18EBEE4B},
        {FDE_CSSPROPERTYVALUE_Right,	(FX_LPCWSTR)L"right",	0x193ADE3E},
        {FDE_CSSPROPERTYVALUE_HorizontalTb,	(FX_LPCWSTR)L"horizontal-tb",	0x1A66A86D},
        {FDE_CSSPROPERTYVALUE_DistributeLetter,	(FX_LPCWSTR)L"distribute-letter",	0x1EDBD75C},
        {FDE_CSSPROPERTYVALUE_DoubleCircle,	(FX_LPCWSTR)L"double-circle",	0x1FF082BA},
        {FDE_CSSPROPERTYVALUE_Ruby,	(FX_LPCWSTR)L"ruby",	0x20D66C02},
        {FDE_CSSPROPERTYVALUE_Collapse,	(FX_LPCWSTR)L"collapse",	0x2128D673},
        {FDE_CSSPROPERTYVALUE_Normal,	(FX_LPCWSTR)L"normal",	0x247CF3E9},
        {FDE_CSSPROPERTYVALUE_Avoid,	(FX_LPCWSTR)L"avoid",	0x24E684B3},
        {FDE_CSSPROPERTYVALUE_UpperRoman,	(FX_LPCWSTR)L"upper-roman",	0x28BAC2B6},
        {FDE_CSSPROPERTYVALUE_Auto,	(FX_LPCWSTR)L"auto",	0x2B35B6D9},
        {FDE_CSSPROPERTYVALUE_Text,	(FX_LPCWSTR)L"text",	0x2D08AF85},
        {FDE_CSSPROPERTYVALUE_XSmall,	(FX_LPCWSTR)L"x-small",	0x2D2FCAFE},
        {FDE_CSSPROPERTYVALUE_Thin,	(FX_LPCWSTR)L"thin",	0x2D574D53},
        {FDE_CSSPROPERTYVALUE_Repeat,	(FX_LPCWSTR)L"repeat",	0x306614A1},
        {FDE_CSSPROPERTYVALUE_Small,	(FX_LPCWSTR)L"small",	0x316A3739},
        {FDE_CSSPROPERTYVALUE_NeResize,	(FX_LPCWSTR)L"ne-resize",	0x31FD5E12},
        {FDE_CSSPROPERTYVALUE_NoContent,	(FX_LPCWSTR)L"no-content",	0x33A1C545},
        {FDE_CSSPROPERTYVALUE_Outside,	(FX_LPCWSTR)L"outside",	0x36DF693D},
        {FDE_CSSPROPERTYVALUE_EResize,	(FX_LPCWSTR)L"e-resize",	0x36E19FA4},
        {FDE_CSSPROPERTYVALUE_TableRow,	(FX_LPCWSTR)L"table-row",	0x3912A02D},
        {FDE_CSSPROPERTYVALUE_Bottom,	(FX_LPCWSTR)L"bottom",	0x399F02B5},
        {FDE_CSSPROPERTYVALUE_Underline,	(FX_LPCWSTR)L"underline",	0x3A0273A6},
        {FDE_CSSPROPERTYVALUE_CjkIdeographic,	(FX_LPCWSTR)L"cjk-ideographic",	0x3A641CC4},
        {FDE_CSSPROPERTYVALUE_SeResize,	(FX_LPCWSTR)L"se-resize",	0x3D675B17},
        {FDE_CSSPROPERTYVALUE_Fixed,	(FX_LPCWSTR)L"fixed",	0x3D7DEB10},
        {FDE_CSSPROPERTYVALUE_Double,	(FX_LPCWSTR)L"double",	0x3D98515B},
        {FDE_CSSPROPERTYVALUE_Solid,	(FX_LPCWSTR)L"solid",	0x40623B5B},
        {FDE_CSSPROPERTYVALUE_RubyBaseGroup,	(FX_LPCWSTR)L"ruby-base-group",	0x41014E84},
        {FDE_CSSPROPERTYVALUE_OpenQuote,	(FX_LPCWSTR)L"open-quote",	0x44A41E8D},
        {FDE_CSSPROPERTYVALUE_Lighter,	(FX_LPCWSTR)L"lighter",	0x45BEB7AF},
        {FDE_CSSPROPERTYVALUE_LowerRoman,	(FX_LPCWSTR)L"lower-roman",	0x5044D253},
        {FDE_CSSPROPERTYVALUE_Strict,	(FX_LPCWSTR)L"strict",	0x52F4EBD9},
        {FDE_CSSPROPERTYVALUE_TableCaption,	(FX_LPCWSTR)L"table-caption",	0x5325CD63},
        {FDE_CSSPROPERTYVALUE_Oblique,	(FX_LPCWSTR)L"oblique",	0x53EBDDB1},
        {FDE_CSSPROPERTYVALUE_Decimal,	(FX_LPCWSTR)L"decimal",	0x54034C2F},
        {FDE_CSSPROPERTYVALUE_Loose,	(FX_LPCWSTR)L"loose",	0x54D3A1E2},
        {FDE_CSSPROPERTYVALUE_Hebrew,	(FX_LPCWSTR)L"hebrew",	0x565792DD},
        {FDE_CSSPROPERTYVALUE_Hidden,	(FX_LPCWSTR)L"hidden",	0x573CB40C},
        {FDE_CSSPROPERTYVALUE_Dashed,	(FX_LPCWSTR)L"dashed",	0x58A3DD29},
        {FDE_CSSPROPERTYVALUE_Embed,	(FX_LPCWSTR)L"embed",	0x59C8F27D},
        {FDE_CSSPROPERTYVALUE_TableRowGroup,	(FX_LPCWSTR)L"table-row-group",	0x5A43BD07},
        {FDE_CSSPROPERTYVALUE_TableColumn,	(FX_LPCWSTR)L"table-column",	0x5E705DA3},
        {FDE_CSSPROPERTYVALUE_Static,	(FX_LPCWSTR)L"static",	0x5E7555E8},
        {FDE_CSSPROPERTYVALUE_Outset,	(FX_LPCWSTR)L"outset",	0x61236164},
        {FDE_CSSPROPERTYVALUE_DecimalLeadingZero,	(FX_LPCWSTR)L"decimal-leading-zero",	0x61DFC55D},
        {FDE_CSSPROPERTYVALUE_KeepWords,	(FX_LPCWSTR)L"keep-words",	0x63964801},
        {FDE_CSSPROPERTYVALUE_KatakanaIroha,	(FX_LPCWSTR)L"katakana-iroha",	0x65D7C91C},
        {FDE_CSSPROPERTYVALUE_Super,	(FX_LPCWSTR)L"super",	0x6A4F842F},
        {FDE_CSSPROPERTYVALUE_Center,	(FX_LPCWSTR)L"center",	0x6C51AFC1},
        {FDE_CSSPROPERTYVALUE_TableHeaderGroup,	(FX_LPCWSTR)L"table-header-group",	0x706103D8},
        {FDE_CSSPROPERTYVALUE_Inside,	(FX_LPCWSTR)L"inside",	0x709CB0FC},
        {FDE_CSSPROPERTYVALUE_XxLarge,	(FX_LPCWSTR)L"xx-large",	0x70BB1508},
        {FDE_CSSPROPERTYVALUE_Triangle,	(FX_LPCWSTR)L"triangle",	0x7524EDF6},
        {FDE_CSSPROPERTYVALUE_RubyTextGroup,	(FX_LPCWSTR)L"ruby-text-group",	0x78C2B98E},
        {FDE_CSSPROPERTYVALUE_Circle,	(FX_LPCWSTR)L"circle",	0x7ABEC0D2},
        {FDE_CSSPROPERTYVALUE_Hiragana,	(FX_LPCWSTR)L"hiragana",	0x7BF5E25B},
        {FDE_CSSPROPERTYVALUE_RepeatX,	(FX_LPCWSTR)L"repeat-x",	0x7C8F3226},
        {FDE_CSSPROPERTYVALUE_RepeatY,	(FX_LPCWSTR)L"repeat-y",	0x7C8F3227},
        {FDE_CSSPROPERTYVALUE_Move,	(FX_LPCWSTR)L"move",	0x7DA03417},
        {FDE_CSSPROPERTYVALUE_HiraganaIroha,	(FX_LPCWSTR)L"hiragana-iroha",	0x7EE863FB},
        {FDE_CSSPROPERTYVALUE_RubyBase,	(FX_LPCWSTR)L"ruby-base",	0x7FD1B1EA},
        {FDE_CSSPROPERTYVALUE_Scroll,	(FX_LPCWSTR)L"scroll",	0x84787AEF},
        {FDE_CSSPROPERTYVALUE_Smaller,	(FX_LPCWSTR)L"smaller",	0x849769F0},
        {FDE_CSSPROPERTYVALUE_TableFooterGroup,	(FX_LPCWSTR)L"table-footer-group",	0x85BDD97E},
        {FDE_CSSPROPERTYVALUE_Baseline,	(FX_LPCWSTR)L"baseline",	0x87436BA3},
        {FDE_CSSPROPERTYVALUE_Separate,	(FX_LPCWSTR)L"separate",	0x877C66B5},
        {FDE_CSSPROPERTYVALUE_Armenian,	(FX_LPCWSTR)L"armenian",	0x889BE4EB},
        {FDE_CSSPROPERTYVALUE_Open,	(FX_LPCWSTR)L"open",	0x8B90E1F2},
        {FDE_CSSPROPERTYVALUE_Relative,	(FX_LPCWSTR)L"relative",	0x8C995B5C},
        {FDE_CSSPROPERTYVALUE_Thick,	(FX_LPCWSTR)L"thick",	0x8CC35EB3},
        {FDE_CSSPROPERTYVALUE_Justify,	(FX_LPCWSTR)L"justify",	0x8D269CAE},
        {FDE_CSSPROPERTYVALUE_Middle,	(FX_LPCWSTR)L"middle",	0x947FA00F},
        {FDE_CSSPROPERTYVALUE_Always,	(FX_LPCWSTR)L"always",	0x959AB231},
        {FDE_CSSPROPERTYVALUE_DistributeSpace,	(FX_LPCWSTR)L"distribute-space",	0x97A20E58},
        {FDE_CSSPROPERTYVALUE_LineEdge,	(FX_LPCWSTR)L"line-edge",	0x9A845D2A},
        {FDE_CSSPROPERTYVALUE_PreWrap,	(FX_LPCWSTR)L"pre-wrap",	0x9D59588E},
        {FDE_CSSPROPERTYVALUE_Medium,	(FX_LPCWSTR)L"medium",	0xA084A381},
        {FDE_CSSPROPERTYVALUE_NResize,	(FX_LPCWSTR)L"n-resize",	0xA088968D},
        {FDE_CSSPROPERTYVALUE_ListItem,	(FX_LPCWSTR)L"list-item",	0xA32382B8},
        {FDE_CSSPROPERTYVALUE_Show,	(FX_LPCWSTR)L"show",	0xA66C10C1},
        {FDE_CSSPROPERTYVALUE_Currentcolor,	(FX_LPCWSTR)L"currentColor",	0xA7883922},
        {FDE_CSSPROPERTYVALUE_NoCloseQuote,	(FX_LPCWSTR)L"no-close-quote",	0xA79CBFFB},
        {FDE_CSSPROPERTYVALUE_VerticalLr,	(FX_LPCWSTR)L"vertical-lr",	0xA8673F65},
        {FDE_CSSPROPERTYVALUE_VerticalRl,	(FX_LPCWSTR)L"vertical-rl",	0xA8675E25},
        {FDE_CSSPROPERTYVALUE_Pointer,	(FX_LPCWSTR)L"pointer",	0xA90929C1},
        {FDE_CSSPROPERTYVALUE_XxSmall,	(FX_LPCWSTR)L"xx-small",	0xADE1FC76},
        {FDE_CSSPROPERTYVALUE_Bold,	(FX_LPCWSTR)L"bold",	0xB18313A1},
        {FDE_CSSPROPERTYVALUE_Both,	(FX_LPCWSTR)L"both",	0xB1833CAD},
        {FDE_CSSPROPERTYVALUE_SmallCaps,	(FX_LPCWSTR)L"small-caps",	0xB299428D},
        {FDE_CSSPROPERTYVALUE_Katakana,	(FX_LPCWSTR)L"katakana",	0xB421A4BC},
        {FDE_CSSPROPERTYVALUE_After,	(FX_LPCWSTR)L"after",	0xB6B44172},
        {FDE_CSSPROPERTYVALUE_Horizontal,	(FX_LPCWSTR)L"horizontal",	0xB7732DEA},
        {FDE_CSSPROPERTYVALUE_Dotted,	(FX_LPCWSTR)L"dotted",	0xB88652A4},
        {FDE_CSSPROPERTYVALUE_Disc,	(FX_LPCWSTR)L"disc",	0xBEBC18C3},
        {FDE_CSSPROPERTYVALUE_Georgian,	(FX_LPCWSTR)L"georgian",	0xBEF99E8C},
        {FDE_CSSPROPERTYVALUE_Inline,	(FX_LPCWSTR)L"inline",	0xC02D649F},
        {FDE_CSSPROPERTYVALUE_Overline,	(FX_LPCWSTR)L"overline",	0xC0EC9FA4},
        {FDE_CSSPROPERTYVALUE_Wait,	(FX_LPCWSTR)L"wait",	0xC1613BB5},
        {FDE_CSSPROPERTYVALUE_BreakAll,	(FX_LPCWSTR)L"break-all",	0xC3145BAB},
        {FDE_CSSPROPERTYVALUE_UpperAlpha,	(FX_LPCWSTR)L"upper-alpha",	0xC52D4A9F},
        {FDE_CSSPROPERTYVALUE_Capitalize,	(FX_LPCWSTR)L"capitalize",	0xC5321D46},
        {FDE_CSSPROPERTYVALUE_Nowrap,	(FX_LPCWSTR)L"nowrap",	0xC7994417},
        {FDE_CSSPROPERTYVALUE_TextBottom,	(FX_LPCWSTR)L"text-bottom",	0xC7D08D87},
        {FDE_CSSPROPERTYVALUE_NoOpenQuote,	(FX_LPCWSTR)L"no-open-quote",	0xC8CD7877},
        {FDE_CSSPROPERTYVALUE_Groove,	(FX_LPCWSTR)L"groove",	0xCB24A412},
        {FDE_CSSPROPERTYVALUE_Progress,	(FX_LPCWSTR)L"progress",	0xCD1D9835},
        {FDE_CSSPROPERTYVALUE_Larger,	(FX_LPCWSTR)L"larger",	0xCD3C409D},
        {FDE_CSSPROPERTYVALUE_CloseQuote,	(FX_LPCWSTR)L"close-quote",	0xCF8696D1},
        {FDE_CSSPROPERTYVALUE_TableCell,	(FX_LPCWSTR)L"table-cell",	0xCFB5E595},
        {FDE_CSSPROPERTYVALUE_PreLine,	(FX_LPCWSTR)L"pre-line",	0xD04FEDBC},
        {FDE_CSSPROPERTYVALUE_Absolute,	(FX_LPCWSTR)L"absolute",	0xD0B2D55F},
        {FDE_CSSPROPERTYVALUE_InlineTable,	(FX_LPCWSTR)L"inline-table",	0xD131F494},
        {FDE_CSSPROPERTYVALUE_BidiOverride,	(FX_LPCWSTR)L"bidi-override",	0xD161FDE5},
        {FDE_CSSPROPERTYVALUE_InlineBlock,	(FX_LPCWSTR)L"inline-block",	0xD26A8BD7},
        {FDE_CSSPROPERTYVALUE_Inset,	(FX_LPCWSTR)L"inset",	0xD6F23243},
        {FDE_CSSPROPERTYVALUE_Crosshair,	(FX_LPCWSTR)L"crosshair",	0xD6F8018E},
        {FDE_CSSPROPERTYVALUE_UpperLatin,	(FX_LPCWSTR)L"upper-latin",	0xD9D60531},
        {FDE_CSSPROPERTYVALUE_Help,	(FX_LPCWSTR)L"help",	0xDA002969},
        {FDE_CSSPROPERTYVALUE_Hide,	(FX_LPCWSTR)L"hide",	0xDA69395A},
        {FDE_CSSPROPERTYVALUE_Uppercase,	(FX_LPCWSTR)L"uppercase",	0xDAD595A8},
        {FDE_CSSPROPERTYVALUE_SResize,	(FX_LPCWSTR)L"s-resize",	0xDB3AADF2},
        {FDE_CSSPROPERTYVALUE_Table,	(FX_LPCWSTR)L"table",	0xDB9BE968},
        {FDE_CSSPROPERTYVALUE_Blink,	(FX_LPCWSTR)L"blink",	0xDC36E390},
        {FDE_CSSPROPERTYVALUE_Block,	(FX_LPCWSTR)L"block",	0xDCD480AB},
        {FDE_CSSPROPERTYVALUE_Start,	(FX_LPCWSTR)L"start",	0xE1D9D5AE},
        {FDE_CSSPROPERTYVALUE_TableColumnGroup,	(FX_LPCWSTR)L"table-column-group",	0xE2258EFD},
        {FDE_CSSPROPERTYVALUE_Italic,	(FX_LPCWSTR)L"italic",	0xE31D5396},
        {FDE_CSSPROPERTYVALUE_LineThrough,	(FX_LPCWSTR)L"line-through",	0xE4C5A276},
        {FDE_CSSPROPERTYVALUE_KeepAll,	(FX_LPCWSTR)L"keep-all",	0xE704A72B},
        {FDE_CSSPROPERTYVALUE_LowerAlpha,	(FX_LPCWSTR)L"lower-alpha",	0xECB75A3C},
        {FDE_CSSPROPERTYVALUE_RunIn,	(FX_LPCWSTR)L"run-in",	0xEEC930B9},
        {FDE_CSSPROPERTYVALUE_Square,	(FX_LPCWSTR)L"square",	0xEF85D351},
        {FDE_CSSPROPERTYVALUE_XLarge,	(FX_LPCWSTR)L"x-large",	0xF008E390},
        {FDE_CSSPROPERTYVALUE_Large,	(FX_LPCWSTR)L"large",	0xF4434FCB},
        {FDE_CSSPROPERTYVALUE_Before,	(FX_LPCWSTR)L"before",	0xF4FFCE73},
        {FDE_CSSPROPERTYVALUE_Left,	(FX_LPCWSTR)L"left",	0xF5AD782B},
        {FDE_CSSPROPERTYVALUE_TextTop,	(FX_LPCWSTR)L"text-top",	0xFCB58D45},
        {FDE_CSSPROPERTYVALUE_RubyText,	(FX_LPCWSTR)L"ruby-text",	0xFCC77174},
        {FDE_CSSPROPERTYVALUE_NoDisplay,	(FX_LPCWSTR)L"no-display",	0xFE482860},
    };
    static const FDE_CSSMEDIATYPETABLE g_FDE_CSSMediaTypes[] = {
        {0xF09, 0x02},
        {0x4880, 0x20},
        {0x536A, 0x80},
        {0x741D, 0x10},
        {0x76ED, 0x08},
        {0x7CFB, 0x01},
        {0x9578, 0x04},
        {0xC8E1, 0x40},
        {0xD0F9, 0xFF},
    };
    static const FDE_CSSLENGTHUNITTABLE g_FDE_CSSLengthUnits[] = {
        {0x0672, 3},
        {0x067D, 4},
        {0x1AF7, 8},
        {0x2F7A, 7},
        {0x3ED3, 10},
        {0x3EE4, 9},
        {0x3EE8, 5},
        {0xFC30, 6},
    };
    static const FDE_CSSCOLORTABLE g_FDE_CSSColors[] = {
        {0x031B47FE, 0xff000080},
        {0x0BB8DF5B, 0xffff0000},
        {0x0D82A78C, 0xff800000},
        {0x2ACC82E8, 0xff00ffff},
        {0x2D083986, 0xff008080},
        {0x4A6A6195, 0xffc0c0c0},
        {0x546A8EF3, 0xff808080},
        {0x65C9169C, 0xffffa500},
        {0x8422BB61, 0xffffffff},
        {0x9271A558, 0xff800080},
        {0xA65A3EE3, 0xffff00ff},
        {0xB1345708, 0xff0000ff},
        {0xB6D2CF1F, 0xff808000},
        {0xD19B5E1C, 0xffffff00},
        {0xDB64391D, 0xff000000},
        {0xF616D507, 0xff00ff00},
        {0xF6EFFF31, 0xff008000},
    };
    static const FDE_CSSPERSUDOTABLE g_FDE_CSSPersudoType[] = {
        {FDE_CSSPERSUDO_After,	(FX_LPCWSTR)L":after",	0x16EE1FEC},
        {FDE_CSSPERSUDO_Before,	(FX_LPCWSTR)L":before",	0x7DCDDE2D},
    };
    FDE_LPCCSSPERSUDOTABLE FDE_GetCSSPersudoByEnum(FDE_CSSPERSUDO ePersudo)
    {
        return (ePersudo < FDE_CSSPERSUDO_NONE) ? (g_FDE_CSSPersudoType + ePersudo) : NULL;
    }
    FDE_LPCCSSPROPERTYTABLE FDE_GetCSSPropertyByName(FX_LPCWSTR pszName, FX_INT32 iLength)
    {
        FXSYS_assert(pszName != NULL && iLength > 0);
        FX_DWORD dwHash = FX_HashCode_String_GetW(pszName, iLength, TRUE);
        FX_INT32 iEnd = FDE_CSSPROPERTY_MAX - 1;
        FX_INT32 iMid, iStart = 0;
        FX_DWORD dwMid;
        do {
            iMid = (iStart + iEnd) / 2;
            dwMid = g_FDE_CSSProperties[iMid].dwHash;
            if (dwHash == dwMid) {
                return g_FDE_CSSProperties + iMid;
            } else if (dwHash > dwMid) {
                iStart = iMid + 1;
            } else {
                iEnd = iMid - 1;
            }
        } while (iStart <= iEnd);
        return NULL;
    }
    FDE_LPCCSSPROPERTYTABLE FDE_GetCSSPropertyByEnum(FDE_CSSPROPERTY eName)
    {
        return (eName < FDE_CSSPROPERTY_MAX) ? (g_FDE_CSSProperties + eName) : NULL;
    }
    FDE_LPCCSSPROPERTYVALUETABLE FDE_GetCSSPropertyValueByName(FX_LPCWSTR pszName, FX_INT32 iLength)
    {
        FXSYS_assert(pszName != NULL && iLength > 0);
        FX_DWORD dwHash = FX_HashCode_String_GetW(pszName, iLength, TRUE);
        FX_INT32 iEnd = FDE_CSSPROPERTYVALUE_MAX - 1;
        FX_INT32 iMid, iStart = 0;
        FX_DWORD dwMid;
        do {
            iMid = (iStart + iEnd) / 2;
            dwMid = g_FDE_CSSPropertyValues[iMid].dwHash;
            if (dwHash == dwMid) {
                return g_FDE_CSSPropertyValues + iMid;
            } else if (dwHash > dwMid) {
                iStart = iMid + 1;
            } else {
                iEnd = iMid - 1;
            }
        } while (iStart <= iEnd);
        return NULL;
    }
    FDE_LPCCSSPROPERTYVALUETABLE FDE_GetCSSPropertyValueByEnum(FDE_CSSPROPERTYVALUE eName)
    {
        return (eName < FDE_CSSPROPERTYVALUE_MAX) ? (g_FDE_CSSPropertyValues + eName) : NULL;
    }
    FDE_LPCCSSMEDIATYPETABLE FDE_GetCSSMediaTypeByName(FX_LPCWSTR pszName, FX_INT32 iLength)
    {
        FXSYS_assert(pszName != NULL && iLength > 0);
        FX_WORD wHash = (FX_WORD)FX_HashCode_String_GetW(pszName, iLength, TRUE);
        FX_INT32 iEnd = sizeof(g_FDE_CSSMediaTypes) / sizeof(FDE_CSSMEDIATYPETABLE) - 1;
        FX_INT32 iMid, iStart = 0;
        FX_WORD uMid;
        do {
            iMid = (iStart + iEnd) / 2;
            uMid = g_FDE_CSSMediaTypes[iMid].wHash;
            if (wHash == uMid) {
                return g_FDE_CSSMediaTypes + iMid;
            } else if (wHash > uMid) {
                iStart = iMid + 1;
            } else {
                iEnd = iMid - 1;
            }
        } while (iStart <= iEnd);
        return NULL;
    }
    FDE_LPCCSSLENGTHUNITTABLE FDE_GetCSSLengthUnitByName(FX_LPCWSTR pszName, FX_INT32 iLength)
    {
        FXSYS_assert(pszName != NULL && iLength > 0);
        FX_WORD wHash = (FX_WORD)FX_HashCode_String_GetW(pszName, iLength, TRUE);
        FX_INT32 iEnd = sizeof(g_FDE_CSSLengthUnits) / sizeof(FDE_CSSLENGTHUNITTABLE) - 1;
        FX_INT32 iMid, iStart = 0;
        FX_WORD wMid;
        do {
            iMid = (iStart + iEnd) / 2;
            wMid = g_FDE_CSSLengthUnits[iMid].wHash;
            if (wHash == wMid) {
                return g_FDE_CSSLengthUnits + iMid;
            } else if (wHash > wMid) {
                iStart = iMid + 1;
            } else {
                iEnd = iMid - 1;
            }
        } while (iStart <= iEnd);
        return NULL;
    }
    FDE_LPCCSSCOLORTABLE FDE_GetCSSColorByName(FX_LPCWSTR pszName, FX_INT32 iLength)
    {
        FXSYS_assert(pszName != NULL && iLength > 0);
        FX_DWORD dwHash = FX_HashCode_String_GetW(pszName, iLength, TRUE);
        FX_INT32 iEnd = sizeof(g_FDE_CSSColors) / sizeof(FDE_CSSCOLORTABLE) - 1;
        FX_INT32 iMid, iStart = 0;
        FX_DWORD dwMid;
        do {
            iMid = (iStart + iEnd) / 2;
            dwMid = g_FDE_CSSColors[iMid].dwHash;
            if (dwHash == dwMid) {
                return g_FDE_CSSColors + iMid;
            } else if (dwHash > dwMid) {
                iStart = iMid + 1;
            } else {
                iEnd = iMid - 1;
            }
        } while (iStart <= iEnd);
        return NULL;
    }
    FX_BOOL FDE_ParseCSSNumber(FX_LPCWSTR pszValue, FX_INT32 iValueLen, FX_FLOAT &fValue, FDE_CSSPRIMITIVETYPE &eUnit)
    {
        FXSYS_assert(pszValue != NULL && iValueLen > 0);
        FX_INT32 iUsedLen = 0;
        fValue = FX_wcstof(pszValue, iValueLen, &iUsedLen);
        if (iUsedLen <= 0) {
            return FALSE;
        }
        iValueLen -= iUsedLen;
        pszValue += iUsedLen;
        eUnit = FDE_CSSPRIMITIVETYPE_Number;
        if (iValueLen >= 1 && *pszValue == '%') {
            eUnit = FDE_CSSPRIMITIVETYPE_Percent;
        } else if (iValueLen == 2) {
            FDE_LPCCSSLENGTHUNITTABLE pUnit = FDE_GetCSSLengthUnitByName(pszValue, 2);
            if (pUnit != NULL) {
                eUnit = (FDE_CSSPRIMITIVETYPE)pUnit->wValue;
            }
        }
        return TRUE;
    }

    FX_BOOL FDE_ParseCSSString(FX_LPCWSTR pszValue, FX_INT32 iValueLen, FX_INT32 &iOffset, FX_INT32 &iLength)
    {
        FXSYS_assert(pszValue != NULL && iValueLen > 0);
        iOffset = 0;
        iLength = iValueLen;
        if (iValueLen >= 2) {
            FX_WCHAR first = pszValue[0], last = pszValue[iValueLen - 1];
            if ((first == '\"' && last == '\"') || (first == '\'' && last == '\'')) {
                iOffset = 1, iLength -= 2;
            }
        }
        return iValueLen > 0;
    }

    FX_BOOL FDE_ParseCSSURI(FX_LPCWSTR pszValue, FX_INT32 iValueLen, FX_INT32 &iOffset, FX_INT32 &iLength)
    {
        FXSYS_assert(pszValue != NULL && iValueLen > 0);
        if (iValueLen < 6 || pszValue[iValueLen - 1] != ')' || FX_wcsnicmp((FX_LPCWSTR)L"url(", pszValue, 4)) {
            return FALSE;
        }
        if (FDE_ParseCSSString(pszValue + 4, iValueLen - 5, iOffset, iLength)) {
            iOffset += 4;
            return TRUE;
        }
        return FALSE;
    }

    FX_BOOL FDE_ParseCSSColor(FX_LPCWSTR pszValue, FX_INT32 iValueLen, FX_ARGB &dwColor)
    {
        FXSYS_assert(pszValue != NULL && iValueLen > 0);
        if (*pszValue == '#') {
            switch (iValueLen) {
                case 4: {
                        FX_BYTE red = FX_Hex2Dec((FX_BYTE)pszValue[1], (FX_BYTE)pszValue[1]);
                        FX_BYTE green = FX_Hex2Dec((FX_BYTE)pszValue[2], (FX_BYTE)pszValue[2]);
                        FX_BYTE blue = FX_Hex2Dec((FX_BYTE)pszValue[3], (FX_BYTE)pszValue[3]);
                        dwColor = ArgbEncode(255, red, green, blue);
                    }
                    return TRUE;
                case 7: {
                        FX_BYTE red = FX_Hex2Dec((FX_BYTE)pszValue[1], (FX_BYTE)pszValue[2]);
                        FX_BYTE green = FX_Hex2Dec((FX_BYTE)pszValue[3], (FX_BYTE)pszValue[4]);
                        FX_BYTE blue = FX_Hex2Dec((FX_BYTE)pszValue[5], (FX_BYTE)pszValue[6]);
                        dwColor = ArgbEncode(255, red, green, blue);
                    }
                    return TRUE;
            }
        } else if (iValueLen >= 10) {
            if (pszValue[iValueLen - 1] != ')' || FX_wcsnicmp((FX_LPCWSTR)L"rgb(", pszValue, 4)) {
                return FALSE;
            }
            FX_BYTE rgb[3] = {0};
            FX_FLOAT fValue;
            FDE_CSSPRIMITIVETYPE eType;
            CFDE_CSSValueListParser list(pszValue + 4, iValueLen - 5, ',');
            for (FX_INT32 i = 0; i < 3; ++i) {
                if (!list.NextValue(eType, pszValue, iValueLen)) {
                    return FALSE;
                }
                if (eType != FDE_CSSPRIMITIVETYPE_Number) {
                    return FALSE;
                }
                if (!FDE_ParseCSSNumber(pszValue, iValueLen, fValue, eType)) {
                    return FALSE;
                }
                rgb[i] = eType == FDE_CSSPRIMITIVETYPE_Percent ? FXSYS_round(fValue * 2.55f) : FXSYS_round(fValue);
            }
            dwColor = ArgbEncode(255, rgb[0], rgb[1], rgb[2]);
            return TRUE;
        } else {
            FDE_LPCCSSCOLORTABLE pColor = FDE_GetCSSColorByName(pszValue, iValueLen);
            if (pColor != NULL) {
                dwColor = pColor->dwValue;
                return TRUE;
            }
        }
        return FALSE;
    }
#ifdef _cplusplus
};
#endif
CFDE_CSSValueList::CFDE_CSSValueList(IFX_MEMAllocator *pStaticStore, const CFDE_CSSValueArray &list)
{
    m_iCount = list.GetSize();
    FX_INT32 iByteCount = m_iCount * sizeof(IFDE_CSSValue*);
    m_ppList = (IFDE_CSSValue**)pStaticStore->Alloc(iByteCount);
    FXSYS_memcpy(m_ppList, list.GetData(), iByteCount);
}
FX_BOOL CFDE_CSSValueListParser::NextValue(FDE_CSSPRIMITIVETYPE &eType, FX_LPCWSTR &pStart, FX_INT32 &iLength)
{
    while (m_pCur < m_pEnd && (*m_pCur <= ' ' || *m_pCur == m_Separator)) {
        ++m_pCur;
    }
    if (m_pCur >= m_pEnd) {
        return FALSE;
    }
    eType = FDE_CSSPRIMITIVETYPE_Unknown;
    pStart = m_pCur;
    iLength = 0;
    FX_WCHAR wch = *m_pCur;
    if (wch == '#') {
        iLength = SkipTo(' ');
        if (iLength == 4 || iLength == 7) {
            eType = FDE_CSSPRIMITIVETYPE_RGB;
        }
    } else if ((wch >= '0' && wch <= '9') || wch == '.' || wch == '-' || wch == '+') {
        while (m_pCur < m_pEnd && (*m_pCur > ' ' && *m_pCur != m_Separator)) {
            ++m_pCur;
        }
        iLength = m_pCur - pStart;
        if (iLength > 0) {
            eType = FDE_CSSPRIMITIVETYPE_Number;
        }
    } else if (wch == '\"' || wch == '\'') {
        pStart++;
        iLength = SkipTo(wch) - 1;
        m_pCur++;
        eType = FDE_CSSPRIMITIVETYPE_String;
    } else if (m_pEnd - m_pCur > 5 && m_pCur[3] == '(') {
        if (FX_wcsnicmp((FX_LPCWSTR)L"url", m_pCur, 3) == 0) {
            wch = m_pCur[4];
            if (wch == '\"' || wch == '\'') {
                pStart += 5;
                iLength = SkipTo(wch) - 6;
                m_pCur += 2;
            } else {
                pStart += 4;
                iLength = SkipTo(')') - 4;
                m_pCur++;
            }
            eType = FDE_CSSPRIMITIVETYPE_URI;
        } else if (FX_wcsnicmp((FX_LPCWSTR)L"rgb", m_pCur, 3) == 0) {
            iLength = SkipTo(')') + 1;
            m_pCur++;
            eType = FDE_CSSPRIMITIVETYPE_RGB;
        }
    } else {
        iLength = SkipTo(m_Separator, TRUE, TRUE);
        eType = FDE_CSSPRIMITIVETYPE_String;
    }
    return m_pCur <= m_pEnd && iLength > 0;
}
FX_INT32 CFDE_CSSValueListParser::SkipTo(FX_WCHAR wch, FX_BOOL bWSSeparator , FX_BOOL bBrContinue )
{
    FX_LPCWSTR pStart = m_pCur;
    if (!bBrContinue) {
        if (bWSSeparator)
            while (++m_pCur < m_pEnd && *m_pCur != wch && *m_pCur > ' ') ;
        else
            while (++m_pCur < m_pEnd && *m_pCur != wch) ;
    } else {
        FX_INT32 iBracketCount = 0;
        if (bWSSeparator) {
            while (m_pCur < m_pEnd && *m_pCur != wch && *m_pCur > ' ') {
                if (*m_pCur == '(') {
                    iBracketCount++;
                } else if (*m_pCur == ')') {
                    iBracketCount--;
                }
                m_pCur++;
            }
        } else {
            while (m_pCur < m_pEnd && *m_pCur != wch) {
                if (*m_pCur == '(') {
                    iBracketCount++;
                } else if (*m_pCur == ')') {
                    iBracketCount--;
                }
                m_pCur++;
            }
        }
        while (iBracketCount > 0 && m_pCur < m_pEnd) {
            if (*m_pCur == ')') {
                iBracketCount--;
            }
            m_pCur++;
        }
    }
    return m_pCur - pStart;
}
