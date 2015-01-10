// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDCODABARREADER_H_
#define _BC_ONEDCODABARREADER_H_
class CBC_CommonBitArray;
class CBC_OneDReader;
class CBC_OnedCodaBarReader;
class CBC_OnedCodaBarReader : public CBC_OneDReader
{
public:
    CBC_OnedCodaBarReader();
    virtual ~CBC_OnedCodaBarReader();
    CFX_ByteString DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, FX_INT32 hints, FX_INT32 &e);
    CFX_Int32Array *FindAsteriskPattern(CBC_CommonBitArray *row, FX_INT32 &e);
    FX_BOOL ArrayContains(const FX_CHAR array[], FX_CHAR key);
    FX_CHAR ToNarrowWidePattern(CFX_Int32Array *counter);
    static FX_LPCSTR ALPHABET_STRING;


    const static FX_INT32 CHARACTER_ENCODINGS[22];

    const static FX_INT32 minCharacterLength;

    const static FX_CHAR STARTEND_ENCODING[8];
};
#endif
