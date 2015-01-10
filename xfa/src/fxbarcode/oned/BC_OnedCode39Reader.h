// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDCODA39READER_H_
#define _BC_ONEDCODA39READER_H_
class CBC_OneDReader;
class CBC_CommonBitArray;
class CBC_OnedCoda39Reader;
class CBC_OnedCode39Reader : public CBC_OneDReader
{
public:
    static FX_LPCSTR ALPHABET_STRING;
    static FX_LPCSTR CHECKSUM_STRING;
    const static FX_INT32 CHARACTER_ENCODINGS[44];
    const static FX_INT32 ASTERISK_ENCODING;
    CBC_OnedCode39Reader();
    CBC_OnedCode39Reader(FX_BOOL usingCheckDigit);
    CBC_OnedCode39Reader(FX_BOOL usingCheckDigit, FX_BOOL extendedMode);
    virtual ~CBC_OnedCode39Reader();
    CFX_ByteString DecodeRow(FX_INT32 rowNumber, CBC_CommonBitArray *row, FX_INT32 hints, FX_INT32 &e);
private:
    FX_BOOL m_usingCheckDigit;
    FX_BOOL m_extendedMode;
    CFX_Int32Array *FindAsteriskPattern(CBC_CommonBitArray *row, FX_INT32 &e);
    FX_INT32 ToNarrowWidePattern(CFX_Int32Array *counters);
    FX_CHAR PatternToChar(FX_INT32 pattern, FX_INT32 &e);
    CFX_ByteString DecodeExtended(CFX_ByteString &encoded, FX_INT32 &e);
};
#endif
