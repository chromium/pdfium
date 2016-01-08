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

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_UtilCodingConvert.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitSource.h"
#include "xfa/src/fxbarcode/common/BC_CommonECI.h"
#include "xfa/src/fxbarcode/common/BC_CommonCharacterSetECI.h"
#include "xfa/src/fxbarcode/common/BC_CommonDecoderResult.h"
#include "BC_QRCoderMode.h"
#include "BC_QRDecodedBitStreamParser.h"
const FX_CHAR* CBC_QRDecodedBitStreamParser::UTF_8 = "utf8";
const FX_CHAR CBC_QRDecodedBitStreamParser::ALPHANUMERIC_CHARS[45] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
    'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', ' ', '$', '%', '*', '+', '-', '.', '/', ':'};
CBC_QRDecodedBitStreamParser::CBC_QRDecodedBitStreamParser() {}
CBC_QRDecodedBitStreamParser::~CBC_QRDecodedBitStreamParser() {}
CBC_CommonDecoderResult* CBC_QRDecodedBitStreamParser::Decode(
    CFX_ByteArray* bytes,
    CBC_QRCoderVersion* version,
    CBC_QRCoderErrorCorrectionLevel* ecLevel,
    int32_t byteModeDecode,
    int32_t& e) {
  CBC_CommonBitSource bits(bytes);
  CFX_ByteString result;
  CBC_CommonCharacterSetECI* currentCharacterSetECI = NULL;
  FX_BOOL fc1Infact = FALSE;
  CFX_Int32Array byteSegments;
  CBC_QRCoderMode* mode = NULL;
  do {
    if (bits.Available() < 4) {
      mode = CBC_QRCoderMode::sTERMINATOR;
    } else {
      int32_t iTemp1 = bits.ReadBits(4, e);
      BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
      mode = CBC_QRCoderMode::ForBits(iTemp1, e);
      BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
      if (mode == NULL) {
        e = BCExceptionUnSupportMode;
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
      }
    }
    if (!(mode == CBC_QRCoderMode::sTERMINATOR)) {
      if (mode == CBC_QRCoderMode::sFNC1_FIRST_POSITION ||
          mode == CBC_QRCoderMode::sFNC1_SECOND_POSITION) {
        fc1Infact = TRUE;
      } else if (mode == CBC_QRCoderMode::sSTRUCTURED_APPEND) {
        bits.ReadBits(16, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
      } else if (mode == CBC_QRCoderMode::sECI) {
        int32_t value = ParseECIValue(&bits, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        currentCharacterSetECI =
            CBC_CommonCharacterSetECI::GetCharacterSetECIByValue(value);
      } else {
        if (mode == CBC_QRCoderMode::sGBK) {
          bits.ReadBits(4, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        }
        int32_t numBits = mode->GetCharacterCountBits(version, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        int32_t count = bits.ReadBits(numBits, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        if (mode == CBC_QRCoderMode::sNUMERIC) {
          DecodeNumericSegment(&bits, result, count, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        } else if (mode == CBC_QRCoderMode::sALPHANUMERIC) {
          DecodeAlphanumericSegment(&bits, result, count, fc1Infact, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        } else if (mode == CBC_QRCoderMode::sBYTE) {
          DecodeByteSegment(&bits, result, count, currentCharacterSetECI,
                            &byteSegments, byteModeDecode, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        } else if (mode == CBC_QRCoderMode::sGBK) {
          DecodeGBKSegment(&bits, result, count, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        } else if (mode == CBC_QRCoderMode::sKANJI) {
          DecodeKanjiSegment(&bits, result, count, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        } else {
          e = BCExceptionUnSupportMode;
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        }
      }
    }
  } while (!(mode == CBC_QRCoderMode::sTERMINATOR));
  CBC_CommonDecoderResult* tempCd = new CBC_CommonDecoderResult();
  tempCd->Init(*bytes, result, byteSegments, ecLevel, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return tempCd;
}
void CBC_QRDecodedBitStreamParser::DecodeGBKSegment(CBC_CommonBitSource* bits,
                                                    CFX_ByteString& result,
                                                    int32_t count,
                                                    int32_t& e) {
  CFX_ByteString buffer;
  while (count > 0) {
    int32_t twoBytes = bits->ReadBits(13, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    int32_t assembledTwoBytes = ((twoBytes / 0x060) << 8) | (twoBytes % 0x060);
    if (assembledTwoBytes <= 0x0095d) {
      assembledTwoBytes += 0x0a1a1;
    } else {
      assembledTwoBytes += 0x0a6a1;
    }
    buffer += (uint8_t)(assembledTwoBytes >> 8);
    buffer += (uint8_t)assembledTwoBytes;
    count--;
  }
  CBC_UtilCodingConvert::LocaleToUtf8(buffer, result);
}
void CBC_QRDecodedBitStreamParser::DecodeKanjiSegment(CBC_CommonBitSource* bits,
                                                      CFX_ByteString& result,
                                                      int32_t count,
                                                      int32_t& e) {
  CFX_ByteString buffer;
  while (count > 0) {
    int32_t twoBytes = bits->ReadBits(13, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    int32_t assembledTwoBytes = ((twoBytes / 0x0c0) << 8) | (twoBytes % 0x0c0);
    if (assembledTwoBytes <= 0x01f00) {
      assembledTwoBytes += 0x08140;
    } else {
      assembledTwoBytes += 0x0c140;
    }
    buffer += (uint8_t)(assembledTwoBytes >> 8);
    buffer += (uint8_t)assembledTwoBytes;
    count--;
  }
  CBC_UtilCodingConvert::LocaleToUtf8(buffer, result);
}
void CBC_QRDecodedBitStreamParser::DecodeByteSegment(
    CBC_CommonBitSource* bits,
    CFX_ByteString& result,
    int32_t count,
    CBC_CommonCharacterSetECI* currentCharacterSetECI,
    CFX_Int32Array* byteSegments,
    int32_t byteModeDecode,
    int32_t& e) {
  if (count < 0) {
    e = BCExceptionNotFound;
    BC_EXCEPTION_CHECK_ReturnVoid(e);
  }
  if ((count << 3) > bits->Available()) {
    e = BCExceptionRead;
    BC_EXCEPTION_CHECK_ReturnVoid(e);
  }
  uint8_t* readBytes = FX_Alloc(uint8_t, count);
  FXSYS_memset(readBytes, 0x00, count);
  for (int32_t i = 0; i < count; i++) {
    readBytes[i] = (uint8_t)bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
  }
  CFX_ByteString bs(readBytes, count);
  result += bs;
  FX_Free(readBytes);
}
void CBC_QRDecodedBitStreamParser::DecodeAlphanumericSegment(
    CBC_CommonBitSource* bits,
    CFX_ByteString& result,
    int32_t count,
    FX_BOOL fac1InEffect,
    int32_t& e) {
  int32_t start = result.GetLength();
  while (count > 1) {
    int32_t nextTwoCharsBits = bits->ReadBits(11, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    BC_FX_ByteString_Append(result, 1,
                            ALPHANUMERIC_CHARS[nextTwoCharsBits / 45]);
    BC_FX_ByteString_Append(result, 1,
                            ALPHANUMERIC_CHARS[nextTwoCharsBits % 45]);
    count -= 2;
  }
  if (count == 1) {
    int32_t itemp = bits->ReadBits(6, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[itemp]);
  }
  if (fac1InEffect) {
    for (int32_t i = start; i < result.GetLength(); i++) {
      if (result[i] == '%') {
        if ((i < result.GetLength() - 1) && result[i + 1] == '%') {
          result.Delete(i + 1, 1);
        } else {
          result.SetAt(i, (FX_CHAR)0x1d);
        }
      }
    }
  }
}
void CBC_QRDecodedBitStreamParser::DecodeNumericSegment(
    CBC_CommonBitSource* bits,
    CFX_ByteString& result,
    int32_t count,
    int32_t& e) {
  while (count >= 3) {
    int32_t threeDigitsBits = bits->ReadBits(10, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    if (threeDigitsBits >= 1000) {
      e = BCExceptionRead;
      BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    BC_FX_ByteString_Append(result, 1,
                            ALPHANUMERIC_CHARS[threeDigitsBits / 100]);
    BC_FX_ByteString_Append(result, 1,
                            ALPHANUMERIC_CHARS[(threeDigitsBits / 10) % 10]);
    BC_FX_ByteString_Append(result, 1,
                            ALPHANUMERIC_CHARS[threeDigitsBits % 10]);
    count -= 3;
  }
  if (count == 2) {
    int32_t twoDigitBits = bits->ReadBits(7, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    if (twoDigitBits >= 100) {
      e = BCExceptionRead;
      BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[twoDigitBits / 10]);
    BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[twoDigitBits % 10]);
  } else if (count == 1) {
    int32_t digitBits = bits->ReadBits(4, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    if (digitBits >= 10) {
      e = BCExceptionRead;
      BC_EXCEPTION_CHECK_ReturnVoid(e);
    }
    BC_FX_ByteString_Append(result, 1, ALPHANUMERIC_CHARS[digitBits]);
  }
}
const CFX_ByteString CBC_QRDecodedBitStreamParser::GuessEncoding(
    CFX_ByteArray* bytes) {
  return *UTF_8;
}
int32_t CBC_QRDecodedBitStreamParser::ParseECIValue(CBC_CommonBitSource* bits,
                                                    int32_t& e) {
  int32_t firstByte = bits->ReadBits(8, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, 0);
  if ((firstByte & 0x80) == 0) {
    return firstByte & 0x7f;
  } else if ((firstByte & 0xc0) == 0x80) {
    int32_t secondByte = bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    return ((firstByte & 0x3f) << 8) | secondByte;
  } else if ((firstByte & 0xe0) == 0xc0) {
    int32_t secondThirdByte = bits->ReadBits(16, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    return ((firstByte & 0x1f) << 16) | secondThirdByte;
  }
  e = BCExceptionBadECI;
  BC_EXCEPTION_CHECK_ReturnValue(e, 0);
  return 0;
}
