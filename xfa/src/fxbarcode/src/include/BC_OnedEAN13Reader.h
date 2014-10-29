// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDEAN13READER_H_
#define _BC_ONEDEAN13READER_H_
class CBC_OneDimReader;
class CBC_CommonBitArray;
class CBC_OnedEAN13Reader;
class CBC_OnedEAN13Reader : public CBC_OneDimReader
{
public:
    const static FX_INT32 FIRST_DIGIT_ENCODINGS[10];
    CBC_OnedEAN13Reader();
    virtual ~CBC_OnedEAN13Reader();
private:
    void DetermineFirstDigit(CFX_ByteString &result,  FX_INT32 lgPatternFound, FX_INT32 &e);
protected:
    FX_INT32 DecodeMiddle(CBC_CommonBitArray *row, CFX_Int32Array *startRange, CFX_ByteString &resultString, FX_INT32 &e);
    friend class CBC_OnedUPCAReader;
};
#endif
