// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ERRORCORRECTION_H_
#define _BC_ERRORCORRECTION_H_
class CBC_SymbolInfo;
class CBC_ErrorCorrection;
class CBC_ErrorCorrection : public CFX_Object
{
public:
    CBC_ErrorCorrection();
    virtual ~CBC_ErrorCorrection();
    static void Initialize();
    static void Finalize();
    static CFX_WideString encodeECC200(CFX_WideString codewords, CBC_SymbolInfo* symbolInfo, FX_INT32 &e);
private:
    static FX_INT32 FACTOR_SETS[];
    static FX_INT32 FACTORS[][100];
    static FX_INT32 MODULO_VALUE;
    static FX_INT32 LOG[256];
    static FX_INT32 ALOG[256];
private:
    static CFX_WideString createECCBlock(CFX_WideString codewords, FX_INT32 numECWords, FX_INT32 &e);
    static CFX_WideString createECCBlock(CFX_WideString codewords, FX_INT32 start, FX_INT32 len, FX_INT32 numECWords, FX_INT32 &e);
};
#endif
