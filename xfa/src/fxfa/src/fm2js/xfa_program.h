// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_FM_PROGRAM_H
#define _XFA_FM_PROGRAM_H
class CXFA_FMProgram : public CFX_Object
{
public:
    CXFA_FMProgram();
    ~CXFA_FMProgram();
    FX_INT32 Init(FX_WSTR wsFormcalc);
    FX_INT32 ParseProgram();
    FX_INT32 TranslateProgram(CFX_WideTextBuf &wsJavaScript);
    CXFA_FMErrorInfo& GetError()
    {
        return m_pErrorInfo;
    }
private:
    CXFA_FMErrorInfo m_pErrorInfo;
    CXFA_FMParse	m_parse;
    CXFA_FMFunctionDefinition *m_globalFunction;
};
#endif
