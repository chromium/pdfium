// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../../include/fpdfapi/fpdf_resource.h"
#include "../../../../include/fpdfapi/fpdf_module.h"
#include "../../fpdf_font/font_int.h"
#include "../cmap_int.h"
extern const FX_WORD g_FXCMAP_KSC_EUC_H_0[];
extern const FX_WORD g_FXCMAP_KSC_EUC_V_0[];
extern const FX_WORD g_FXCMAP_KSCms_UHC_H_1[];
extern const FX_WORD g_FXCMAP_KSCms_UHC_V_1[];
extern const FX_WORD g_FXCMAP_KSCms_UHC_HW_H_1[];
extern const FX_WORD g_FXCMAP_KSCms_UHC_HW_V_1[];
extern const FX_WORD g_FXCMAP_KSCpc_EUC_H_0[];
extern const FX_WORD g_FXCMAP_UniKS_UCS2_H_1[];
extern const FX_WORD g_FXCMAP_UniKS_UCS2_V_1[];
extern const FX_WORD g_FXCMAP_UniKS_UTF16_H_0[];
extern const FX_WORD g_FXCMAP_Korea1CID2Unicode_2[];
const FXCMAP_CMap g_FXCMAP_Korea1_cmaps[] = {
    { "KSC-EUC-H", FXCMAP_CMap::Range, g_FXCMAP_KSC_EUC_H_0, 467, FXCMAP_CMap::None, NULL, 0, 0 },
    { "KSC-EUC-V", FXCMAP_CMap::Range, g_FXCMAP_KSC_EUC_V_0, 16, FXCMAP_CMap::None, NULL, 0, -1 },
    { "KSCms-UHC-H", FXCMAP_CMap::Range, g_FXCMAP_KSCms_UHC_H_1, 675, FXCMAP_CMap::None, NULL, 0, -2 },
    { "KSCms-UHC-V", FXCMAP_CMap::Range, g_FXCMAP_KSCms_UHC_V_1, 16, FXCMAP_CMap::None, NULL, 0, -1 },
    { "KSCms-UHC-HW-H", FXCMAP_CMap::Range, g_FXCMAP_KSCms_UHC_HW_H_1, 675, FXCMAP_CMap::None, NULL, 0, 0 },
    { "KSCms-UHC-HW-V", FXCMAP_CMap::Range, g_FXCMAP_KSCms_UHC_HW_V_1, 16, FXCMAP_CMap::None, NULL, 0, -1 },
    { "KSCpc-EUC-H", FXCMAP_CMap::Range, g_FXCMAP_KSCpc_EUC_H_0, 509, FXCMAP_CMap::None, NULL, 0, -6 },
    { "UniKS-UCS2-H", FXCMAP_CMap::Range, g_FXCMAP_UniKS_UCS2_H_1, 8394, FXCMAP_CMap::None, NULL, 0, 0 },
    { "UniKS-UCS2-V", FXCMAP_CMap::Range, g_FXCMAP_UniKS_UCS2_V_1, 18, FXCMAP_CMap::None, NULL, 0, -1 },
    { "UniKS-UTF16-H", FXCMAP_CMap::Single, g_FXCMAP_UniKS_UTF16_H_0, 158, FXCMAP_CMap::None, NULL, 0, -2 },
    { "UniKS-UTF16-V", FXCMAP_CMap::Range, g_FXCMAP_UniKS_UCS2_V_1, 18, FXCMAP_CMap::None, NULL, 0, -1 },
};
void CPDF_ModuleMgr::LoadEmbeddedKorea1CMaps()
{
    CPDF_FontGlobals* pFontGlobals = CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals();
    pFontGlobals->m_EmbeddedCharsets[CIDSET_KOREA1].m_pMapList = g_FXCMAP_Korea1_cmaps;
    pFontGlobals->m_EmbeddedCharsets[CIDSET_KOREA1].m_Count = sizeof g_FXCMAP_Korea1_cmaps / sizeof (FXCMAP_CMap);
    pFontGlobals->m_EmbeddedToUnicodes[CIDSET_KOREA1].m_pMap = g_FXCMAP_Korea1CID2Unicode_2;
    pFontGlobals->m_EmbeddedToUnicodes[CIDSET_KOREA1].m_Count = 18352;
}
