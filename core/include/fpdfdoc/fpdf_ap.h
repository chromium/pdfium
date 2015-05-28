// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFDOC_FPDF_AP_H_
#define CORE_INCLUDE_FPDFDOC_FPDF_AP_H_

#include "../fxcrt/fx_basic.h"
#include "../fpdfapi/fpdf_parser.h"
#include "fpdf_vt.h"

class IPVT_FontMap
{
public:
    virtual ~IPVT_FontMap() { }
    virtual CPDF_Font*						GetPDFFont(FX_INT32 nFontIndex) = 0;
    virtual CFX_ByteString					GetPDFFontAlias(FX_INT32 nFontIndex) = 0;
};
struct CPVT_Dash {

    CPVT_Dash(FX_INT32 dash, FX_INT32 gap, FX_INT32 phase) : nDash(dash), nGap(gap), nPhase(phase)
    {}

    FX_INT32			nDash;

    FX_INT32			nGap;

    FX_INT32			nPhase;
};
#define CT_TRANSPARENT		0
#define	CT_GRAY				1
#define	CT_RGB				2
#define	CT_CMYK				3
struct CPVT_Color {

    CPVT_Color(FX_INT32 type = 0, FX_FLOAT color1 = 0.0f, FX_FLOAT color2 = 0.0f, FX_FLOAT color3 = 0.0f, FX_FLOAT color4 = 0.0f)
        : nColorType(type), fColor1(color1), fColor2(color2), fColor3(color3), fColor4(color4)
    {}

    FX_INT32			nColorType;
    FX_FLOAT			fColor1;
    FX_FLOAT			fColor2;
    FX_FLOAT			fColor3;
    FX_FLOAT			fColor4;
};
class CPVT_Provider : public IPDF_VariableText_Provider
{
public:

    CPVT_Provider(IPVT_FontMap * pFontMap);

    virtual ~CPVT_Provider();

    FX_INT32						GetCharWidth(FX_INT32 nFontIndex, FX_WORD word, FX_INT32 nWordStyle);

    FX_INT32						GetTypeAscent(FX_INT32 nFontIndex);

    FX_INT32						GetTypeDescent(FX_INT32 nFontIndex);

    FX_INT32						GetWordFontIndex(FX_WORD word, FX_INT32 charset, FX_INT32 nFontIndex);

    FX_BOOL							IsLatinWord(FX_WORD word);

    FX_INT32						GetDefaultFontIndex();
private:

    IPVT_FontMap *	m_pFontMap;
};
#define PBS_SOLID			0
#define PBS_DASH			1
#define PBS_BEVELED			2
#define PBS_INSET			3
#define PBS_UNDERLINED		4
class CPVT_GenerateAP
{
public:

    static FX_BOOL							GenerateTextFieldAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict);

    static FX_BOOL							GenerateComboBoxAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict);

    static FX_BOOL							GenerateListBoxAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict);

    static CFX_ByteString					GenerateEditAP(IPVT_FontMap * pFontMap, IPDF_VariableText_Iterator * pIterator,
            const CPDF_Point & ptOffset, FX_BOOL bContinuous, FX_WORD SubWord = 0, const CPVT_WordRange * pVisible = NULL);

    static CFX_ByteString					GenerateBorderAP(const CPDF_Rect & rect, FX_FLOAT fWidth,
            const CPVT_Color & color, const CPVT_Color & crLeftTop, const CPVT_Color & crRightBottom,
            FX_INT32 nStyle, const CPVT_Dash & dash);

    static CFX_ByteString					GenerateColorAP(const CPVT_Color & color, const FX_BOOL & bFillOrStroke);
};

#endif  // CORE_INCLUDE_FPDFDOC_FPDF_AP_H_
