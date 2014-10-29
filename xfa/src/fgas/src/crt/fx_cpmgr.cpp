// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../fgas_base.h"
#include "fx_codepage.h"
#ifdef _FXCP
#ifdef __cplusplus
extern "C"
{
#endif
extern const FX_CODEPAGE_HEADER g_CP936_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP932_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP949_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP950_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP874_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP1250_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP1251_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP1252_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP1253_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP1254_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP1255_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP1256_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP1257_MapHeader;
extern const FX_CODEPAGE_HEADER g_CP1258_MapHeader;
extern const FX_CPCU_MAPINFO g_CP936_CUMap;
extern const FX_CPCU_MAPINFO g_CP932_CUMap;
extern const FX_CPCU_MAPINFO g_CP949_CUMap;
extern const FX_CPCU_MAPINFO g_CP950_CUMap;
extern const FX_CPCU_MAPINFO g_CP874_CUMap;
extern const FX_CPCU_MAPINFO g_CP1250_CUMap;
extern const FX_CPCU_MAPINFO g_CP1251_CUMap;
extern const FX_CPCU_MAPINFO g_CP1252_CUMap;
extern const FX_CPCU_MAPINFO g_CP1253_CUMap;
extern const FX_CPCU_MAPINFO g_CP1254_CUMap;
extern const FX_CPCU_MAPINFO g_CP1255_CUMap;
extern const FX_CPCU_MAPINFO g_CP1256_CUMap;
extern const FX_CPCU_MAPINFO g_CP1257_CUMap;
extern const FX_CPCU_MAPINFO g_CP1258_CUMap;
FX_LPCCODEPAGE FX_GetCodePage(FX_WORD wCodePage)
{
    FX_INT32 iEnd = sizeof(g_FXCodePageMgr) / sizeof(FX_CODEPAGE) - 1;
    FXSYS_assert(iEnd >= 0);
    FX_INT32 iStart = 0, iMid;
    FX_UINT16 uCPID;
    do {
        iMid = (iStart + iEnd) / 2;
        const FX_CODEPAGE &cp = g_FXCodePageMgr[iMid];
        uCPID = cp.pCPHeader->uCPID;
        if (wCodePage == uCPID) {
            return g_FXCodePageMgr + iMid;
        } else if (wCodePage < uCPID) {
            iEnd = iMid - 1;
        } else {
            iStart = iMid + 1;
        }
    } while (iStart <= iEnd);
    return NULL;
}
#ifdef __cplusplus
}
#endif
#endif
