// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRDECODEDBITSTREAMPARSER_H_
#define _BC_QRDECODEDBITSTREAMPARSER_H_
class CBC_CommonDecoderResult;
class CBC_QRCoderErrorCorrectionLevel;
class CBC_CommonBitSource;
class CBC_QRCoderVersion;
class CBC_CommonCharacterSetECI;
class CBC_QRDecodedBitStreamParser;
class CBC_QRDecodedBitStreamParser  : public CFX_Object
{
private:
    const static FX_CHAR ALPHANUMERIC_CHARS[45];
    static FX_LPCSTR UTF_8;
    CBC_QRDecodedBitStreamParser();
public:
    virtual ~CBC_QRDecodedBitStreamParser();
    static CBC_CommonDecoderResult* Decode(CFX_ByteArray *bytes, CBC_QRCoderVersion *version,
                                           CBC_QRCoderErrorCorrectionLevel* ecLevel, FX_INT32 byteModeDecode, FX_INT32 &e);

    static const CFX_ByteString GuessEncoding(CFX_ByteArray* bytes);
    static FX_INT32 ParseECIValue(CBC_CommonBitSource* bits, FX_INT32 &e);
    static void DecodeGBKSegment(CBC_CommonBitSource* bits, CFX_ByteString &result, FX_INT32 count, FX_INT32 &e);
    static void DecodeKanjiSegment(CBC_CommonBitSource* bits, CFX_ByteString &result, FX_INT32 count, FX_INT32 &e);
    static void DecodeNumericSegment(CBC_CommonBitSource* bits, CFX_ByteString &result, FX_INT32 count, FX_INT32 &e);
    static void DecodeAlphanumericSegment(CBC_CommonBitSource* bits, CFX_ByteString &result, FX_INT32 count, FX_BOOL fac1InEffect, FX_INT32 &e);
    static void DecodeByteSegment(CBC_CommonBitSource* bits, CFX_ByteString &result, FX_INT32 count, CBC_CommonCharacterSetECI *currentCharacterSetECI,
                                  CFX_Int32Array *byteSegments, FX_INT32 byteModeDecode, FX_INT32 &e);
};
#endif
