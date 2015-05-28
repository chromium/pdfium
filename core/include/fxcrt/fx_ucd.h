// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_UCD_H_
#define CORE_INCLUDE_FXCRT_FX_UCD_H_

#include "fx_system.h"

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
FX_DWORD FX_GetUnicodeProperties(FX_WCHAR wch);
FX_BOOL	FX_IsCtrlCode(FX_WCHAR ch);
FX_BOOL	FX_IsRotationCode(FX_WCHAR ch);
FX_BOOL FX_IsCombinationChar(FX_WCHAR wch);
FX_BOOL	FX_IsBidiChar(FX_WCHAR wch);
FX_WCHAR FX_GetMirrorChar(FX_WCHAR wch, FX_BOOL bRTL, FX_BOOL bVertical);
FX_WCHAR FX_GetMirrorChar(FX_WCHAR wch, FX_DWORD dwProps, FX_BOOL bRTL, FX_BOOL bVertical);

#endif  // CORE_INCLUDE_FXCRT_FX_UCD_H_
