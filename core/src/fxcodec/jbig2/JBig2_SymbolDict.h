// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_SYMBOL_DICT_H_
#define _JBIG2_SYMBOL_DICT_H_
#include "JBig2_Define.h"
#include "JBig2_ArithDecoder.h"
#include "JBig2_Image.h"
class CJBig2_SymbolDict : public CJBig2_Object
{
public:

    CJBig2_SymbolDict();
    CJBig2_SymbolDict *DeepCopy();
    ~CJBig2_SymbolDict();
public:
    FX_DWORD SDNUMEXSYMS;
    CJBig2_Image **SDEXSYMS;
    FX_BOOL m_bContextRetained;
    JBig2ArithCtx *m_gbContext,
                  *m_grContext;
};
#endif
