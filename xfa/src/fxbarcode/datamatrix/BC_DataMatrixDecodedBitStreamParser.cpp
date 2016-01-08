// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2008 ZXing authors
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
#include "xfa/src/fxbarcode/common/BC_CommonDecoderResult.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitSource.h"
#include "BC_DataMatrixDecodedBitStreamParser.h"
const FX_CHAR CBC_DataMatrixDecodedBitStreamParser::C40_BASIC_SET_CHARS[] = {
    '*', '*', '*', ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
const FX_CHAR CBC_DataMatrixDecodedBitStreamParser::C40_SHIFT2_SET_CHARS[] = {
    '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*',  '+', ',', '-', '.',
    '/', ':', ';', '<', '=', '>', '?',  '@', '[', '\\', ']', '^', '_'};
const FX_CHAR CBC_DataMatrixDecodedBitStreamParser::TEXT_BASIC_SET_CHARS[] = {
    '*', '*', '*', ' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
const FX_CHAR CBC_DataMatrixDecodedBitStreamParser::TEXT_SHIFT3_SET_CHARS[] = {
    '\'', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',         'J',
    'K',  'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',         'U',
    'V',  'W', 'X', 'Y', 'Z', '{', '|', '}', '~', (FX_CHAR)127};
const int32_t CBC_DataMatrixDecodedBitStreamParser::PAD_ENCODE = 0;
const int32_t CBC_DataMatrixDecodedBitStreamParser::ASCII_ENCODE = 1;
const int32_t CBC_DataMatrixDecodedBitStreamParser::C40_ENCODE = 2;
const int32_t CBC_DataMatrixDecodedBitStreamParser::TEXT_ENCODE = 3;
const int32_t CBC_DataMatrixDecodedBitStreamParser::ANSIX12_ENCODE = 4;
const int32_t CBC_DataMatrixDecodedBitStreamParser::EDIFACT_ENCODE = 5;
const int32_t CBC_DataMatrixDecodedBitStreamParser::BASE256_ENCODE = 6;
CBC_DataMatrixDecodedBitStreamParser::CBC_DataMatrixDecodedBitStreamParser() {}
CBC_DataMatrixDecodedBitStreamParser::~CBC_DataMatrixDecodedBitStreamParser() {}
CBC_CommonDecoderResult* CBC_DataMatrixDecodedBitStreamParser::Decode(
    CFX_ByteArray& bytes,
    int32_t& e) {
  CBC_CommonBitSource bits(&bytes);
  CFX_ByteString result;
  CFX_ByteString resultTrailer;
  CFX_Int32Array byteSegments;
  int32_t mode = ASCII_ENCODE;
  do {
    if (mode == 1) {
      mode = DecodeAsciiSegment(&bits, result, resultTrailer, e);
      BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    } else {
      switch (mode) {
        case 2:
          DecodeC40Segment(&bits, result, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
          break;
        case 3:
          DecodeTextSegment(&bits, result, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
          break;
        case 4:
          DecodeAnsiX12Segment(&bits, result, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
          break;
        case 5:
          DecodeEdifactSegment(&bits, result, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
          break;
        case 6:
          DecodeBase256Segment(&bits, result, byteSegments, e);
          BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
          break;
        default:
          NULL;
          e = BCExceptionFormatException;
          return NULL;
      }
      mode = ASCII_ENCODE;
    }
  } while (mode != PAD_ENCODE && bits.Available() > 0);
  if (resultTrailer.GetLength() > 0) {
    result += resultTrailer;
  }
  CBC_CommonDecoderResult* tempCp = new CBC_CommonDecoderResult();
  tempCp->Init(bytes, result,
               (byteSegments.GetSize() <= 0) ? CFX_Int32Array() : byteSegments,
               NULL, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return tempCp;
}
int32_t CBC_DataMatrixDecodedBitStreamParser::DecodeAsciiSegment(
    CBC_CommonBitSource* bits,
    CFX_ByteString& result,
    CFX_ByteString& resultTrailer,
    int32_t& e) {
  FX_CHAR buffer[128];
  FX_BOOL upperShift = FALSE;
  do {
    int32_t oneByte = bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, 0);
    if (oneByte == 0) {
      e = BCExceptionFormatException;
      return 0;
    } else if (oneByte <= 128) {
      oneByte = upperShift ? oneByte + 128 : oneByte;
      upperShift = FALSE;
      result += ((FX_CHAR)(oneByte - 1));
      return ASCII_ENCODE;
    } else if (oneByte == 129) {
      return PAD_ENCODE;
    } else if (oneByte <= 229) {
      int32_t value = oneByte - 130;
#if defined(_FX_WINAPI_PARTITION_APP_)
      memset(buffer, 0, sizeof(FX_CHAR) * 128);
      _itoa_s(value, buffer, 128, 10);
#else
      FXSYS_itoa(value, buffer, 10);
#endif
      if (value < 10) {
        result += '0';
        buffer[1] = '\0';
      } else {
        buffer[2] = '\0';
      }
      result += buffer;
    } else if (oneByte == 230) {
      return C40_ENCODE;
    } else if (oneByte == 231) {
      return BASE256_ENCODE;
    } else if (oneByte == 232 || oneByte == 233 || oneByte == 234) {
    } else if (oneByte == 235) {
      upperShift = TRUE;
    } else if (oneByte == 236) {
      result += "[)>";
      result += 0x1E;
      result += "05";
      result += 0x1D;
      resultTrailer.Insert(0, 0x1E);
      resultTrailer.Insert(0 + 1, 0x04);
    } else if (oneByte == 237) {
      result += "[)>";
      result += 0x1E;
      result += "06";
      result += 0x1D;
      resultTrailer.Insert(0, 0x1E);
      resultTrailer.Insert(0 + 1, 0x04);
    } else if (oneByte == 238) {
      return ANSIX12_ENCODE;
    } else if (oneByte == 239) {
      return TEXT_ENCODE;
    } else if (oneByte == 240) {
      return EDIFACT_ENCODE;
    } else if (oneByte == 241) {
    } else if (oneByte >= 242) {
      if (oneByte == 254 && bits->Available() == 0) {
      } else {
        e = BCExceptionFormatException;
        return 0;
      }
    }
  } while (bits->Available() > 0);
  return ASCII_ENCODE;
}
void CBC_DataMatrixDecodedBitStreamParser::DecodeC40Segment(
    CBC_CommonBitSource* bits,
    CFX_ByteString& result,
    int32_t& e) {
  FX_BOOL upperShift = FALSE;
  CFX_Int32Array cValues;
  cValues.SetSize(3);
  do {
    if (bits->Available() == 8) {
      return;
    }
    int32_t firstByte = bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    if (firstByte == 254) {
      return;
    }
    int32_t tempp = bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    ParseTwoBytes(firstByte, tempp, cValues);
    int32_t shift = 0;
    int32_t i;
    for (i = 0; i < 3; i++) {
      int32_t cValue = cValues[i];
      switch (shift) {
        case 0:
          if (cValue < 3) {
            shift = cValue + 1;
          } else if (cValue < 27) {
            FX_CHAR c40char = C40_BASIC_SET_CHARS[cValue];
            if (upperShift) {
              result += (FX_CHAR)(c40char + 128);
              upperShift = FALSE;
            } else {
              result += c40char;
            }
          } else {
            e = BCExceptionFormatException;
            return;
          }
          break;
        case 1:
          if (upperShift) {
            result += (FX_CHAR)(cValue + 128);
            upperShift = FALSE;
          } else {
            result += cValue;
          }
          shift = 0;
          break;
        case 2:
          if (cValue < 27) {
            FX_CHAR c40char = C40_SHIFT2_SET_CHARS[cValue];
            if (upperShift) {
              result += (FX_CHAR)(c40char + 128);
              upperShift = FALSE;
            } else {
              result += c40char;
            }
          } else if (cValue == 27) {
            e = BCExceptionFormatException;
            return;
          } else if (cValue == 30) {
            upperShift = TRUE;
          } else {
            e = BCExceptionFormatException;
            return;
          }
          shift = 0;
          break;
        case 3:
          if (upperShift) {
            result += (FX_CHAR)(cValue + 224);
            upperShift = FALSE;
          } else {
            result += (FX_CHAR)(cValue + 96);
          }
          shift = 0;
          break;
        default:
          break;
          e = BCExceptionFormatException;
          return;
      }
    }
  } while (bits->Available() > 0);
}
void CBC_DataMatrixDecodedBitStreamParser::DecodeTextSegment(
    CBC_CommonBitSource* bits,
    CFX_ByteString& result,
    int32_t& e) {
  FX_BOOL upperShift = FALSE;
  CFX_Int32Array cValues;
  cValues.SetSize(3);
  int32_t shift = 0;
  do {
    if (bits->Available() == 8) {
      return;
    }
    int32_t firstByte = bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    if (firstByte == 254) {
      return;
    }
    int32_t inTp = bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    ParseTwoBytes(firstByte, inTp, cValues);
    for (int32_t i = 0; i < 3; i++) {
      int32_t cValue = cValues[i];
      switch (shift) {
        case 0:
          if (cValue < 3) {
            shift = cValue + 1;
          } else if (cValue < 40) {
            FX_CHAR textChar = TEXT_BASIC_SET_CHARS[cValue];
            if (upperShift) {
              result += (FX_CHAR)(textChar + 128);
              upperShift = FALSE;
            } else {
              result += textChar;
            }
          } else {
            e = BCExceptionFormatException;
            return;
          }
          break;
        case 1:
          if (upperShift) {
            result += (FX_CHAR)(cValue + 128);
            upperShift = FALSE;
          } else {
            result += cValue;
          }
          shift = 0;
          break;
        case 2:
          if (cValue < 27) {
            FX_CHAR c40char = C40_SHIFT2_SET_CHARS[cValue];
            if (upperShift) {
              result += (FX_CHAR)(c40char + 128);
              upperShift = FALSE;
            } else {
              result += c40char;
            }
          } else if (cValue == 27) {
            e = BCExceptionFormatException;
            return;
          } else if (cValue == 30) {
            upperShift = TRUE;
          } else {
            e = BCExceptionFormatException;
            return;
          }
          shift = 0;
          break;
        case 3:
          if (cValue < 19) {
            FX_CHAR textChar = TEXT_SHIFT3_SET_CHARS[cValue];
            if (upperShift) {
              result += (FX_CHAR)(textChar + 128);
              upperShift = FALSE;
            } else {
              result += textChar;
            }
            shift = 0;
          } else {
            e = BCExceptionFormatException;
            return;
          }
          break;
        default:
          break;
          e = BCExceptionFormatException;
          return;
      }
    }
  } while (bits->Available() > 0);
}
void CBC_DataMatrixDecodedBitStreamParser::DecodeAnsiX12Segment(
    CBC_CommonBitSource* bits,
    CFX_ByteString& result,
    int32_t& e) {
  CFX_Int32Array cValues;
  cValues.SetSize(3);
  do {
    if (bits->Available() == 8) {
      return;
    }
    int32_t firstByte = bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    if (firstByte == 254) {
      return;
    }
    int32_t iTemp1 = bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    ParseTwoBytes(firstByte, iTemp1, cValues);
    int32_t i;
    for (i = 0; i < 3; i++) {
      int32_t cValue = cValues[i];
      if (cValue == 0) {
        BC_FX_ByteString_Append(result, 1, '\r');
      } else if (cValue == 1) {
        BC_FX_ByteString_Append(result, 1, '*');
      } else if (cValue == 2) {
        BC_FX_ByteString_Append(result, 1, '>');
      } else if (cValue == 3) {
        BC_FX_ByteString_Append(result, 1, ' ');
      } else if (cValue < 14) {
        BC_FX_ByteString_Append(result, 1, (FX_CHAR)(cValue + 44));
      } else if (cValue < 40) {
        BC_FX_ByteString_Append(result, 1, (FX_CHAR)(cValue + 51));
      } else {
        e = BCExceptionFormatException;
        return;
      }
    }
  } while (bits->Available() > 0);
}
void CBC_DataMatrixDecodedBitStreamParser::ParseTwoBytes(
    int32_t firstByte,
    int32_t secondByte,
    CFX_Int32Array& result) {
  int32_t fullBitValue = (firstByte << 8) + secondByte - 1;
  int32_t temp = fullBitValue / 1600;
  result[0] = temp;
  fullBitValue -= temp * 1600;
  temp = fullBitValue / 40;
  result[1] = temp;
  result[2] = fullBitValue - temp * 40;
}
void CBC_DataMatrixDecodedBitStreamParser::DecodeEdifactSegment(
    CBC_CommonBitSource* bits,
    CFX_ByteString& result,
    int32_t& e) {
  FX_CHAR buffer[128];
  FX_BOOL unlatch = FALSE;
  do {
    if (bits->Available() <= 16) {
      return;
    }
    int32_t i;
    for (i = 0; i < 4; i++) {
      int32_t edifactValue = bits->ReadBits(6, e);
      BC_EXCEPTION_CHECK_ReturnVoid(e);
      if (edifactValue == 0x1F) {
        unlatch = TRUE;
      }
      if (!unlatch) {
        if ((edifactValue & 32) == 0) {
          edifactValue |= 64;
        }
#if defined(_FX_WINAPI_PARTITION_APP_)
        memset(buffer, 0, sizeof(FX_CHAR) * 128);
        _itoa_s(edifactValue, buffer, 128, 10);
        result += buffer;
#else
        result += FXSYS_itoa(edifactValue, buffer, 10);
#endif
      }
    }
  } while (!unlatch && bits->Available() > 0);
}
void CBC_DataMatrixDecodedBitStreamParser::DecodeBase256Segment(
    CBC_CommonBitSource* bits,
    CFX_ByteString& result,
    CFX_Int32Array& byteSegments,
    int32_t& e) {
  int32_t codewordPosition = 1 + bits->getByteOffset();
  int32_t iTmp = bits->ReadBits(8, e);
  BC_EXCEPTION_CHECK_ReturnVoid(e);
  int32_t d1 = Unrandomize255State(iTmp, codewordPosition++);
  int32_t count;
  if (d1 == 0) {
    count = bits->Available() / 8;
  } else if (d1 < 250) {
    count = d1;
  } else {
    int32_t iTmp3 = bits->ReadBits(8, e);
    BC_EXCEPTION_CHECK_ReturnVoid(e);
    count = 250 * (d1 - 249) + Unrandomize255State(iTmp3, codewordPosition++);
  }
  if (count < 0) {
    e = BCExceptionFormatException;
    return;
  }
  CFX_ByteArray* bytes = new CFX_ByteArray();
  bytes->SetSize(count);
  int32_t i;
  for (i = 0; i < count; i++) {
    if (bits->Available() < 8) {
      e = BCExceptionFormatException;
      delete bytes;
      return;
    }
    int32_t iTemp5 = bits->ReadBits(8, e);
    if (e != BCExceptionNO) {
      delete bytes;
      return;
    }
    bytes->SetAt(i, Unrandomize255State(iTemp5, codewordPosition++));
  }
  BC_FX_ByteString_Append(result, *bytes);
  delete bytes;
}
uint8_t CBC_DataMatrixDecodedBitStreamParser::Unrandomize255State(
    int32_t randomizedBase256Codeword,
    int32_t base256CodewordPosition) {
  int32_t pseudoRandomNumber = ((149 * base256CodewordPosition) % 255) + 1;
  int32_t tempVariable = randomizedBase256Codeword - pseudoRandomNumber;
  return (uint8_t)(tempVariable >= 0 ? tempVariable : tempVariable + 256);
}
