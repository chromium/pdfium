// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_UNICODE_
#define _FX_UNICODE_
enum FX_CHARBREAKPROP {
    FX_CBP_OP = 0,
    FX_CBP_CL = 1,
    FX_CBP_QU = 2,
    FX_CBP_GL = 3,
    FX_CBP_NS = 4,
    FX_CBP_EX = 5,
    FX_CBP_SY = 6,
    FX_CBP_IS = 7,
    FX_CBP_PR = 8,
    FX_CBP_PO = 9,
    FX_CBP_NU = 10,
    FX_CBP_AL = 11,
    FX_CBP_ID = 12,
    FX_CBP_IN = 13,
    FX_CBP_HY = 14,
    FX_CBP_BA = 15,
    FX_CBP_BB = 16,
    FX_CBP_B2 = 17,
    FX_CBP_ZW = 18,
    FX_CBP_CM = 19,
    FX_CBP_WJ = 20,
    FX_CBP_H2 = 21,
    FX_CBP_H3 = 22,
    FX_CBP_JL = 23,
    FX_CBP_JV = 24,
    FX_CBP_JT = 25,

    FX_CBP_BK = 26,
    FX_CBP_CR = 27,
    FX_CBP_LF = 28,
    FX_CBP_NL = 29,
    FX_CBP_SA = 30,
    FX_CBP_SG = 31,
    FX_CBP_CB = 32,
    FX_CBP_XX = 33,
    FX_CBP_AI = 34,
    FX_CBP_SP = 35,
    FX_CBP_TB = 37,
    FX_CBP_NONE = 36,
};
#define FX_BIDICLASSBITS		6
#define FX_BIDICLASSBITSMASK	(31 << FX_BIDICLASSBITS)
enum FX_BIDICLASS {
    FX_BIDICLASS_ON		= 0,
    FX_BIDICLASS_L		= 1,
    FX_BIDICLASS_R		= 2,
    FX_BIDICLASS_AN		= 3,
    FX_BIDICLASS_EN		= 4,
    FX_BIDICLASS_AL		= 5,
    FX_BIDICLASS_NSM	= 6,
    FX_BIDICLASS_CS		= 7,
    FX_BIDICLASS_ES		= 8,
    FX_BIDICLASS_ET		= 9,
    FX_BIDICLASS_BN		= 10,
    FX_BIDICLASS_S		= 11,
    FX_BIDICLASS_WS		= 12,
    FX_BIDICLASS_B		= 13,
    FX_BIDICLASS_RLO	= 14,
    FX_BIDICLASS_RLE	= 15,
    FX_BIDICLASS_LRO	= 16,
    FX_BIDICLASS_LRE	= 17,
    FX_BIDICLASS_PDF	= 18,
    FX_BIDICLASS_N		= FX_BIDICLASS_ON,
};
#define FX_CHARTYPEBITS		11
#define FX_CHARTYPEBITSMASK	(15 << FX_CHARTYPEBITS)
enum FX_CHARTYPE {
    FX_CHARTYPE_Unknown				= 0,
    FX_CHARTYPE_Tab					= (1 << FX_CHARTYPEBITS),
    FX_CHARTYPE_Space				= (2 << FX_CHARTYPEBITS),
    FX_CHARTYPE_Control				= (3 << FX_CHARTYPEBITS),
    FX_CHARTYPE_Combination			= (4 << FX_CHARTYPEBITS),
    FX_CHARTYPE_Numeric				= (5 << FX_CHARTYPEBITS),
    FX_CHARTYPE_Normal				= (6 << FX_CHARTYPEBITS),
    FX_CHARTYPE_ArabicAlef			= (7 << FX_CHARTYPEBITS),
    FX_CHARTYPE_ArabicSpecial		= (8 << FX_CHARTYPEBITS),
    FX_CHARTYPE_ArabicDistortion	= (9 << FX_CHARTYPEBITS),
    FX_CHARTYPE_ArabicNormal		= (10 << FX_CHARTYPEBITS),
    FX_CHARTYPE_ArabicForm			= (11 << FX_CHARTYPEBITS),
    FX_CHARTYPE_Arabic				= (12 << FX_CHARTYPEBITS),
};
typedef struct _FX_CHARPROPERTIES {
    union FX_CHARPROPERTIES_UNION{
        struct FX_CHARPROPERTIES_BIT{
            FX_DWORD	dwBreakType		: 6;
            FX_DWORD	dwBidiClass		: 5;
            FX_DWORD	dwCharType		: 4;
            FX_DWORD	dwRotation		: 1;
            FX_DWORD	dwCJKSpecial	: 1;
            FX_DWORD	dwVertIndex		: 6;
            FX_DWORD	dwBidiIndex		: 9;
        };
        FX_DWORD	dwCharProps;
    };
} FX_CHARPROPERTIES;
FX_DWORD FX_GetUnicodeProperties(FX_WCHAR wch);
FX_BOOL	FX_IsCtrlCode(FX_WCHAR ch);
FX_BOOL	FX_IsRotationCode(FX_WCHAR ch);
FX_BOOL FX_IsCombinationChar(FX_WCHAR wch);
FX_BOOL	FX_IsBidiChar(FX_WCHAR wch);
FX_WCHAR FX_GetMirrorChar(FX_WCHAR wch, FX_BOOL bRTL, FX_BOOL bVertical);
FX_WCHAR FX_GetMirrorChar(FX_WCHAR wch, FX_DWORD dwProps, FX_BOOL bRTL, FX_BOOL bVertical);
class CFX_Char : public CFX_Object
{
public:
    CFX_Char() : m_wCharCode(0)
        , m_nBreakType(0)
        , m_nRotation(0)
        , m_dwCharProps(0)
        , m_dwCharStyles(0)
        , m_iCharWidth(0)
        , m_iHorizontalScale(100)
        , m_iVertialScale(100)
    {
    }
    CFX_Char(FX_WORD wCharCode, FX_DWORD dwCharProps)
        : m_wCharCode(wCharCode)
        , m_nBreakType(0)
        , m_nRotation(0)
        , m_dwCharProps(dwCharProps)
        , m_dwCharStyles(0)
        , m_iCharWidth(0)
        , m_iHorizontalScale(100)
        , m_iVertialScale(100)
    {
    }
    FX_DWORD	GetCharType() const
    {
        return m_dwCharProps & FX_CHARTYPEBITSMASK;
    }
    FX_WORD		m_wCharCode;
    FX_BYTE		m_nBreakType;
    FX_INT8		m_nRotation;
    FX_DWORD	m_dwCharProps;
    FX_DWORD	m_dwCharStyles;
    FX_INT32	m_iCharWidth;
    FX_INT32	m_iHorizontalScale;
    FX_INT32	m_iVertialScale;
};
typedef CFX_ArrayTemplate<CFX_Char>	CFX_CharArray;
class CFX_TxtChar : public CFX_Char
{
public:
    CFX_TxtChar() : CFX_Char()
        , m_dwStatus(0)
        , m_iBidiClass(0)
        , m_iBidiLevel(0)
        , m_iBidiPos(0)
        , m_iBidiOrder(0)
        , m_pUserData(NULL)
    {
    }
    FX_DWORD			m_dwStatus;
    FX_INT16			m_iBidiClass;
    FX_INT16			m_iBidiLevel;
    FX_INT16			m_iBidiPos;
    FX_INT16			m_iBidiOrder;
    FX_LPVOID			m_pUserData;
};
typedef CFX_ArrayTemplate<CFX_TxtChar>	CFX_TxtCharArray;
class CFX_RTFChar : public CFX_Char
{
public:
    CFX_RTFChar() : CFX_Char()
        , m_dwStatus(0)
        , m_iFontSize(0)
        , m_iFontHeight(0)
        , m_iBidiClass(0)
        , m_iBidiLevel(0)
        , m_iBidiPos(0)
        , m_dwLayoutStyles(0)
        , m_dwIdentity(0)
        , m_pUserData(NULL)
    {
    }
    FX_DWORD			m_dwStatus;
    FX_INT32			m_iFontSize;
    FX_INT32			m_iFontHeight;
    FX_INT16			m_iBidiClass;
    FX_INT16			m_iBidiLevel;
    FX_INT16			m_iBidiPos;
    FX_INT16			m_iBidiOrder;
    FX_DWORD			m_dwLayoutStyles;
    FX_DWORD			m_dwIdentity;
    IFX_Unknown			*m_pUserData;
};
typedef CFX_ArrayTemplate<CFX_RTFChar>	CFX_RTFCharArray;
#endif
