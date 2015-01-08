// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2007 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "barcode.h"
#include "include/BC_QRCoderMode.h"
#include "include/BC_CommonBitSource.h"
#include "include/BC_CommonECI.h"
#include "include/BC_QRDecodedBitStreamParser.h"
#include "include/BC_CommonCharacterSetECI.h"
#include "include/BC_CommonDecoderResult.h"
#include "include/BC_UtilCodingConvert.h"
FX_LPCSTR CBC_QRDecodedBitStreamParser::UTF_8 = "utf8";
const FX_CHAR CBC_QRDecodedBitStreamParser::ALPHANUMERIC_CHARS[45] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    ' ', '$', '%', '*', '+', '-', '.', '/', ':'
};
CBC_QRDecodedBitStreamParser::CBC_QRDecodedBitStreamParser()
{
}
CBC_QRDecodedBitStreamParser::~CBC_QRDecodedBitStreamParser()
{
}
CBC_CommonDecoderResult* CBC_QRDecodedBitStreamParser::Decode(CFX_ByteArray *bytes, CBC_QRCoderVersion *version,
        CBC_QRCoderErrorCorrectionLevel* ecLevel, FX_INT32 byteModeDecode, FX_INT32 &e)
{
    CBC_CommonBitSource bits(bytes);
    CFX_ByteString result;
    CBC_CommonCharacterSetECI* currentCharacterSetECI = NULL;
    FX_BOOL fc1Infact = FALSE;
    CFX_Int32Array byteSegments;
    CBC_QRCoderMode* mode = NULL;
    do {
        if(bits.Available() < 4) {
            mode = CBC_QRCoderMode::sTERMINATOR;
        } else {
            FX_INT32 iTemp1 = bits.ReadBits(4, e);
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
            mode = CBC_QRCoderMode::ForBits(iTemp1, e);
            BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
            if(mode == NULL) {
                e = BCExceptionUnSupportMode;
                BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
            }
        }
        if(!(mode == CBC_QRCoderMode::sTERMINATOR)) {
            if(mode == CBC_QRCoderMode::sFNC1_FIRST_POSITION || mode == CBC_QRCoderMode::sFNC1_SECOND_POSITION) {
                fc1Infact = TRUE;
            } else if(mode == CBC_QRCoderMode::sSTRUCTURED_APPEND) {
                bits.ReadBits(16, e);
                BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
            } else if(mode == CBC_QRCoderMode::sECI) {
                FX_INT32 value = ParseECIValue(&bits, e);
                BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                currentCharacterSetECI = CBC_CommonCharacterSetECI::GetCharacterSetECIByValue(value);
            } else {
                if(mode == CBC_QRCoderMode::sGBK) {
                    bits.ReadBits(4, e);
                    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                }
                FX_INT32 numBits = mode->GetCharacterCountBits(version, e);
                BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                FX_INT32 count = bits.ReadBits(numBits, e);
                BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                if(mode == CBC_QRCoderMode::sNUMERIC) {
                    DecodeNumericSegment(&bits, result, count, e);
                    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                } else if(mode == CBC_QRCoderMode::sALPHANUMERIC) {
                    DecodeAlphanumericSegment(&bits, result, count, fc1Infact, e);
                    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                } else if(mode == CBC_QRCoderMode::sBYTE) {
                    DecodeByteSegment(&bits, result, count, currentCharacterSetECI, &byteSegments, byteModeDecode, e);
                    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                } else if(mode == CBC_QRCoderMode::sGBK) {
                    DecodeGBKSegment(&bits, result, count, e);
                    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                } else if(mode == CBC_QRCoderMode::sKANJI) {
                    DecodeKanjiSegment(&bits, result, count, e);
                    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                } else {
                    e = BCExceptionUnSupportMode;
                    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
                }
            }
        }
    } while (!(mode == CBC_QRCoderMode::sTERMINATOR));
    CBC_CommonDecoderResult *tempCd = FX_NEW CBC_CommonDecoderResult();
    tempCd->Init(*bytes, result, byteSegments, ecLevel, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return tempCd;
}
void CBC_QRDecodedBitStreamParser::DecodeGBKSegment(CBC_CommonBitSource* bits, CFX_ByteString &result, FX_INT32 count, FX_INT32 &e)
{
    CFX_ByteString buffer;
    FX_INT32 offset = 0;
    while (count > 0) {
        FX_INT32 twoBytes = bits->ReadBits(13, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
        FX_INT32 assembledTwoBytes = ((twoBytes / 0x060) << 8) | (twoBytes % 0x060);
        if (assembledTwoBytes <= 0x0095d) {
            assembledTwoBytes += 0x0a1a1;
        } else {
            assembledTwoBytes += 0x0a6a1;
        }
        buffer += (FX_BYTE) (assembledTwoBytes >> 8);
        buffer += (FX_BYTE) assembledTwoBytes;
        count--;
    }
    CBC_UtilCodingConvert::LocaleToUtf8(buffer, result);
}
void CBC_QRDecodedBitStreamParser::DecodeKanjiSegment(CBC_CommonBitSource* bits, CFX_ByteString &result, FX_INT32 count, FX_INT32 &e)
{
    CFX_ByteString buffer;
    while (count > 0) {
        FX_INT32 twoBytes = bits->ReadBits(13, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
        FX_INT32 assembledTwoBytes = ((twoBytes / 0x0c0) << 8) | (twoBytes % 0x0c0);
        if (assembledTwoBytes <= 0x01f00) {
            assembledTwoBytes += 0x08140;
        } else {
            assembledTwoBytes += 0x0c140;
        }
        buffer += (FX_BYTE) (assembledTwoBytes >> 8);
        buffer += (FX_BYTE) assembledTwoBytes;
        count--;
    }
    CBC_UtilCodingConvert::LocaleToUtf8(buffer, result);
}
void CBC_QRDecodedBitStreamParser::DecodeByteSegment(CBC_CommonBitSource* bits, CFX_ByteString &result, FX_INT32 count,
        CBC_CommonCharacterSetECI *currentCharacterSetECI,
        CFX_Int32Array *byteSegments, FX_INT32 byteModeDecode, FX_INT32 &e)
{
    if(count < 0) {
        e = BCExceptionNotFound;
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    if((count << 3) > bits->Available()) {
        e = BCExceptionRead;
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    FX_BYTE *readBytes = FX_Alloc(FX_BYTE, count);
    FXSYS_memset32(readBytes, 0x00, count);
    for(FX_INT32 i = 0; i < count; i++) {
        readBytes[i] = (FX_BYTE) bits->ReadBits(8, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    CFX_ByteString bs(readBytes, count);
    result += bs;
    FX_Free(readBytes);
}
void CBC_QRDecodedBitStreamParser::DecodeAlphanumericSegment(CBC_CommonBitSource* bits,
        CFX_ByteString &result, FX_INT32 count, FX_BOOL fac1InEffect, FX_INT32 &e)
{
    FX_INT32 start = result.GetLength();
    while(count > 1) {
        FX_INT32 nextTwoCharsBits = bits->ReadBits(11, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
        BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[nextTwoCharsBits / 45]);
        BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[nextTwoCharsBits % 45]);
        count -= 2;
    }
    if(count == 1) {
        FX_INT32 itemp = bits->ReadBits(6, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
        BC_FX_ByteString_Append(result,  1, ALPHANUMERIC_CHARS[itemp]);
    }
    if(fac1InEffect) {
        for(FX_INT32 i = start; i < result.GetLength(); i++) {
            if(result[i] == '%') {
                if((i < result.GetLength() - 1) && result[i + 1] == '%') {
                    result.Delete(i + 1, 1);
                } else {
                    result.SetAt(i, (FX_CHAR)0x1d);
                }
            }
        }
    }
}
void CBC_QRDecodedBitStreamParser::DecodeNumericSegment(CBC_CommonBitSource* bits, CFX_ByteString &result, FX_INT32 count, FX_INT32 &e)
{
    while(count >= 3) {
        FX_INT32 threeDigitsBits = bits->ReadBits(10, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
        if(threeDigitsBits >= 1000) {
            e = BCExceptionRead;
            BC_EXCEPTION_CHECK_ReturnVoid(e);
        }
        BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[threeDigitsBits / 100]);
        BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[(threeDigitsBits / 10) % 10]);
        BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[threeDigitsBits % 10]);
        count -= 3;
    }
    if(count == 2) {
        FX_INT32 twoDigitBits = bits->ReadBits(7, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
        if(twoDigitBits >= 100) {
            e = BCExceptionRead;
            BC_EXCEPTION_CHECK_ReturnVoid(e);
        }
        BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[twoDigitBits / 10]);
        BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[twoDigitBits % 10]);
    } else if(count == 1) {
        FX_INT32 digitBits = bits->ReadBits(4, e);
        BC_EXCEPTION_CHECK_ReturnVoid(e);
        if(digitBits >= 10) {
            e = BCExceptionRead;
            BC_EXCEPTION_CHECK_ReturnVoid(e);
        }
        BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[digitBits]);
    }
}
const CFX_ByteString CBC_QRDecodedBitStreamParser::GuessEncoding(CFX_ByteArray *bytes)
{
    return *UTF_8;
}
FX_INT32 CBC_QRDecodedBitStreamParser::ParseECIValue(CBC_CommonBitSource* bits, FX_INT32 &e)
{
    FX_INT32 firstByte = bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    if((firstByte & 0x80) == 0) {
        return firstByte & 0x7f;
    } else if((firstByte & 0xc0) == 0x80) {
        FX_INT32 secondByte = bits->ReadBits(8, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, 0);
        return ((firstByte & 0x3f) << 8) | secondByte;
    } else if((firstByte & 0xe0) == 0xc0) {
        FX_INT32 secondThirdByte = bits->ReadBits(16, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, 0);
        return ((firstByte & 0x1f) << 16) | secondThirdByte;
    }
    e = BCExceptionBadECI;
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    return 0;
}
