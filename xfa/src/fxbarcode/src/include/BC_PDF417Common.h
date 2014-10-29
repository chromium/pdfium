// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417COMMON_H_
#define _BC_PDF417COMMON_H_
class CBC_PDF417Common;
class CBC_PDF417Common : public CFX_Object
{
public:
    CBC_PDF417Common();
    virtual ~CBC_PDF417Common();
    static FX_INT32 getBitCountSum(CFX_Int32Array& moduleBitCount);
    static FX_INT32 getCodeword(FX_DWORD symbol);
    static FX_INT32 NUMBER_OF_CODEWORDS;
    static FX_INT32 MAX_CODEWORDS_IN_BARCODE;
    static FX_INT32 MIN_ROWS_IN_BARCODE;
    static FX_INT32 MAX_ROWS_IN_BARCODE;
    static FX_INT32 MAX_CODEWORDS_IN_ROW;
    static FX_INT32 MODULES_IN_CODEWORD;
    static FX_INT32 MODULES_IN_STOP_PATTERN;
    static FX_INT32 BARS_IN_MODULE;
    static FX_INT32 SYMBOL_TABLE[];
    static FX_INT32 CODEWORD_TABLE[];
private:
    static CFX_Int32Array* EMPTY_INT_ARRAY;
    static FX_INT32 findCodewordIndex(FX_DWORD symbol);
};
#endif
