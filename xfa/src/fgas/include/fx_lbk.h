// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_LINEBREAK
#define _FX_LINEBREAK
enum FX_LINEBREAKTYPE {
    FX_LBT_UNKNOWN				= 0x00,
    FX_LBT_DIRECT_BRK			= 0x1A,
    FX_LBT_INDIRECT_BRK			= 0x2B,
    FX_LBT_COM_INDIRECT_BRK		= 0x3C,
    FX_LBT_COM_PROHIBITED_BRK	= 0x4D,
    FX_LBT_PROHIBITED_BRK		= 0x5E,
    FX_LBT_HANGUL_SPACE_BRK		= 0x6F,
};
#define FX_LBUN	FX_LBT_UNKNOWN
#define FX_LBDB	FX_LBT_DIRECT_BRK
#define FX_LBIB	FX_LBT_INDIRECT_BRK
#define FX_LBCB	FX_LBT_COM_INDIRECT_BRK
#define FX_LBCP	FX_LBT_COM_PROHIBITED_BRK
#define FX_LBPB	FX_LBT_PROHIBITED_BRK
#define FX_LBHS	FX_LBT_HANGUL_SPACE_BRK
void	FX_GetLineBreakPositions(FX_LPCWSTR pwsText, FX_LINEBREAKTYPE *pBrkType, FX_INT32 iLength);
void	FX_GetLineBreakPositions(FX_LPCWSTR pwsText, FX_INT32 iLength, CFX_Int32MassArray &bp);
#endif
