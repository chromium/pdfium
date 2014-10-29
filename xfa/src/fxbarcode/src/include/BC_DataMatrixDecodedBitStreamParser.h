// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DATAMATRIXDECODEDBITSTREAMPARSER_H_
#define _BC_DATAMATRIXDECODEDBITSTREAMPARSER_H_
class CBC_CommonDecoderResult;
class CBC_CommonBitSource;
class CBC_DataMatrixDecodedBitStreamParser;
class CBC_DataMatrixDecodedBitStreamParser : public CFX_Object
{
public:
    CBC_DataMatrixDecodedBitStreamParser();
    virtual ~CBC_DataMatrixDecodedBitStreamParser();
    static CBC_CommonDecoderResult *Decode(CFX_ByteArray &bytes, FX_INT32 &e);

private:
    static FX_INT32 DecodeAsciiSegment(CBC_CommonBitSource *bits, CFX_ByteString &result, CFX_ByteString &resultTrailer, FX_INT32 &e);
    static void DecodeC40Segment(CBC_CommonBitSource *bits, CFX_ByteString &result, FX_INT32 &e);
    static void DecodeTextSegment(CBC_CommonBitSource *bits, CFX_ByteString &result, FX_INT32 &e);
    static void DecodeAnsiX12Segment(CBC_CommonBitSource *bits, CFX_ByteString &result, FX_INT32 &e);
    static void ParseTwoBytes(FX_INT32 firstByte, FX_INT32 secondByte, CFX_Int32Array &result);
    static void DecodeEdifactSegment(CBC_CommonBitSource *bits, CFX_ByteString &result, FX_INT32 &e);
    static void DecodeBase256Segment(CBC_CommonBitSource *bits, CFX_ByteString &result, CFX_Int32Array &byteSegments, FX_INT32 &e);
    static FX_BYTE Unrandomize255State(FX_INT32 randomizedBase256Codeword, FX_INT32 base256CodewordPosition);

    const static FX_CHAR C40_BASIC_SET_CHARS[];
    const static FX_CHAR C40_SHIFT2_SET_CHARS[];


    const static FX_CHAR TEXT_BASIC_SET_CHARS[];
    const static FX_CHAR TEXT_SHIFT3_SET_CHARS[];
    const static FX_INT32 PAD_ENCODE;
    const static FX_INT32 ASCII_ENCODE;
    const static FX_INT32 C40_ENCODE;
    const static FX_INT32 TEXT_ENCODE;
    const static FX_INT32 ANSIX12_ENCODE;
    const static FX_INT32 EDIFACT_ENCODE;
    const static FX_INT32 BASE256_ENCODE;
};
#endif
