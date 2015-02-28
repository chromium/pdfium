// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_SymbolDict.h"
CJBig2_SymbolDict::CJBig2_SymbolDict()
{
    SDNUMEXSYMS = 0;
    SDEXSYMS = NULL;
    m_bContextRetained = FALSE;
    m_gbContext = m_grContext = NULL;
}

CJBig2_SymbolDict *CJBig2_SymbolDict::DeepCopy()
{
    CJBig2_SymbolDict *dst = NULL;
    CJBig2_SymbolDict *src = this;
    if (src->m_bContextRetained ||
        src->m_gbContext ||
        src->m_grContext) {
        return NULL;
    }
    JBIG2_ALLOC(dst, CJBig2_SymbolDict());
    dst->SDNUMEXSYMS = src->SDNUMEXSYMS;
    dst->SDEXSYMS = (CJBig2_Image**)m_pModule->JBig2_Malloc2(
        sizeof(CJBig2_Image*), src->SDNUMEXSYMS);
    for(FX_DWORD i = 0; i < src->SDNUMEXSYMS; i++) {
        if (src->SDEXSYMS[i]) {
            JBIG2_ALLOC(dst->SDEXSYMS[i],
                        CJBig2_Image(*(src->SDEXSYMS[i])));
        } else {
            dst->SDEXSYMS[i] = NULL;
        }
    }
    return dst;
}

CJBig2_SymbolDict::~CJBig2_SymbolDict()
{
    if(SDEXSYMS) {
        for(FX_DWORD i = 0; i < SDNUMEXSYMS; i++) {
            if(SDEXSYMS[i]) {
                delete SDEXSYMS[i];
            }
        }
        m_pModule->JBig2_Free(SDEXSYMS);
    }
    if(m_bContextRetained) {
        if(m_gbContext) {
            m_pModule->JBig2_Free(m_gbContext);
        }
        if(m_grContext) {
            m_pModule->JBig2_Free(m_grContext);
        }
    }
}
