// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2009 ZXing authors
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

#include "BC_PDF417DecodedBitStreamParser.h"

#include <stdlib.h>

#include "xfa/src/fxbarcode/BC_DecoderResult.h"
#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/common/BC_CommonDecoderResult.h"
#include "BC_PDF417ResultMetadata.h"
#include "third_party/bigint/BigIntegerLibrary.hh"

#define TEXT_COMPACTION_MODE_LATCH 900
#define BYTE_COMPACTION_MODE_LATCH 901
#define NUMERIC_COMPACTION_MODE_LATCH 902
#define BYTE_COMPACTION_MODE_LATCH_6 924
#define BEGIN_MACRO_PDF417_CONTROL_BLOCK 928
#define BEGIN_MACRO_PDF417_OPTIONAL_FIELD 923
#define MACRO_PDF417_TERMINATOR 922
#define MODE_SHIFT_TO_BYTE_COMPACTION_MODE 913

int32_t CBC_DecodedBitStreamPaser::MAX_NUMERIC_CODEWORDS = 15;
int32_t CBC_DecodedBitStreamPaser::NUMBER_OF_SEQUENCE_CODEWORDS = 2;
int32_t CBC_DecodedBitStreamPaser::PL = 25;
int32_t CBC_DecodedBitStreamPaser::LL = 27;
int32_t CBC_DecodedBitStreamPaser::AS = 27;
int32_t CBC_DecodedBitStreamPaser::ML = 28;
int32_t CBC_DecodedBitStreamPaser::AL = 28;
int32_t CBC_DecodedBitStreamPaser::PS = 29;
int32_t CBC_DecodedBitStreamPaser::PAL = 29;
FX_CHAR CBC_DecodedBitStreamPaser::PUNCT_CHARS[29] = {
    ';', '<',  '>',  '@', '[', '\\', '}', '_', '`', '~',
    '!', '\r', '\t', ',', ':', '\n', '-', '.', '$', '/',
    '"', '|',  '*',  '(', ')', '?',  '{', '}', '\''};
FX_CHAR CBC_DecodedBitStreamPaser::MIXED_CHARS[30] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '&', '\r', '\t',
    ',', ':', '#', '-', '.', '$', '/', '+', '%', '*', '=', '^'};
void CBC_DecodedBitStreamPaser::Initialize() {}
void CBC_DecodedBitStreamPaser::Finalize() {}
CBC_DecodedBitStreamPaser::CBC_DecodedBitStreamPaser() {}
CBC_DecodedBitStreamPaser::~CBC_DecodedBitStreamPaser() {}
CBC_CommonDecoderResult* CBC_DecodedBitStreamPaser::decode(
    CFX_Int32Array& codewords,
    CFX_ByteString ecLevel,
    int32_t& e) {
  CFX_ByteString result;
  int32_t codeIndex = 1;
  int32_t code = codewords.GetAt(codeIndex);
  codeIndex++;
  CBC_PDF417ResultMetadata* resultMetadata = new CBC_PDF417ResultMetadata;
  while (codeIndex < codewords[0]) {
    switch (code) {
      case TEXT_COMPACTION_MODE_LATCH:
        codeIndex = textCompaction(codewords, codeIndex, result);
        break;
      case BYTE_COMPACTION_MODE_LATCH:
        codeIndex = byteCompaction(code, codewords, codeIndex, result);
        break;
      case NUMERIC_COMPACTION_MODE_LATCH:
        codeIndex = numericCompaction(codewords, codeIndex, result, e);
        BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
        break;
      case MODE_SHIFT_TO_BYTE_COMPACTION_MODE:
        codeIndex = byteCompaction(code, codewords, codeIndex, result);
        break;
      case BYTE_COMPACTION_MODE_LATCH_6:
        codeIndex = byteCompaction(code, codewords, codeIndex, result);
        break;
      case BEGIN_MACRO_PDF417_CONTROL_BLOCK:
        codeIndex = decodeMacroBlock(codewords, codeIndex, resultMetadata, e);
        if (e != BCExceptionNO) {
          delete resultMetadata;
          return NULL;
        }
        break;
      default:
        codeIndex--;
        codeIndex = textCompaction(codewords, codeIndex, result);
        break;
    }
    if (codeIndex < codewords.GetSize()) {
      code = codewords[codeIndex++];
    } else {
      e = BCExceptionFormatInstance;
      delete resultMetadata;
      return NULL;
    }
  }
  if (result.GetLength() == 0) {
    e = BCExceptionFormatInstance;
    delete resultMetadata;
    return NULL;
  }
  CFX_ByteArray rawBytes;
  CFX_PtrArray byteSegments;
  CBC_CommonDecoderResult* tempCd = new CBC_CommonDecoderResult();
  tempCd->Init(rawBytes, result, byteSegments, ecLevel, e);
  if (e != BCExceptionNO) {
    delete resultMetadata;
    return NULL;
  }
  tempCd->setOther(resultMetadata);
  return tempCd;
}
int32_t CBC_DecodedBitStreamPaser::decodeMacroBlock(
    CFX_Int32Array& codewords,
    int32_t codeIndex,
    CBC_PDF417ResultMetadata* resultMetadata,
    int32_t& e) {
  if (codeIndex + NUMBER_OF_SEQUENCE_CODEWORDS > codewords[0]) {
    e = BCExceptionFormatInstance;
    return -1;
  }
  CFX_Int32Array segmentIndexArray;
  segmentIndexArray.SetSize(NUMBER_OF_SEQUENCE_CODEWORDS);
  for (int32_t i = 0; i < NUMBER_OF_SEQUENCE_CODEWORDS; i++, codeIndex++) {
    segmentIndexArray.SetAt(i, codewords[codeIndex]);
  }
  CFX_ByteString str =
      decodeBase900toBase10(segmentIndexArray, NUMBER_OF_SEQUENCE_CODEWORDS, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, -1);
  resultMetadata->setSegmentIndex(atoi(str.GetBuffer(str.GetLength())));
  CFX_ByteString fileId;
  codeIndex = textCompaction(codewords, codeIndex, fileId);
  resultMetadata->setFileId(fileId);
  if (codewords[codeIndex] == BEGIN_MACRO_PDF417_OPTIONAL_FIELD) {
    codeIndex++;
    CFX_Int32Array additionalOptionCodeWords;
    additionalOptionCodeWords.SetSize(codewords[0] - codeIndex);
    int32_t additionalOptionCodeWordsIndex = 0;
    FX_BOOL end = FALSE;
    while ((codeIndex < codewords[0]) && !end) {
      int32_t code = codewords[codeIndex++];
      if (code < TEXT_COMPACTION_MODE_LATCH) {
        additionalOptionCodeWords[additionalOptionCodeWordsIndex++] = code;
      } else {
        switch (code) {
          case MACRO_PDF417_TERMINATOR:
            resultMetadata->setLastSegment(TRUE);
            codeIndex++;
            end = TRUE;
            break;
          default:
            e = BCExceptionFormatInstance;
            return -1;
        }
      }
    }
    CFX_Int32Array array;
    array.SetSize(additionalOptionCodeWordsIndex);
    array.Copy(additionalOptionCodeWords);
    resultMetadata->setOptionalData(array);
  } else if (codewords[codeIndex] == MACRO_PDF417_TERMINATOR) {
    resultMetadata->setLastSegment(TRUE);
    codeIndex++;
  }
  return codeIndex;
}
int32_t CBC_DecodedBitStreamPaser::textCompaction(CFX_Int32Array& codewords,
                                                  int32_t codeIndex,
                                                  CFX_ByteString& result) {
  CFX_Int32Array textCompactionData;
  textCompactionData.SetSize((codewords[0] - codeIndex) << 1);
  CFX_Int32Array byteCompactionData;
  byteCompactionData.SetSize((codewords[0] - codeIndex) << 1);
  int32_t index = 0;
  FX_BOOL end = FALSE;
  while ((codeIndex < codewords[0]) && !end) {
    int32_t code = codewords[codeIndex++];
    if (code < TEXT_COMPACTION_MODE_LATCH) {
      textCompactionData[index] = code / 30;
      textCompactionData[index + 1] = code % 30;
      index += 2;
    } else {
      switch (code) {
        case TEXT_COMPACTION_MODE_LATCH:
          textCompactionData[index++] = TEXT_COMPACTION_MODE_LATCH;
          break;
        case BYTE_COMPACTION_MODE_LATCH:
          codeIndex--;
          end = TRUE;
          break;
        case NUMERIC_COMPACTION_MODE_LATCH:
          codeIndex--;
          end = TRUE;
          break;
        case BEGIN_MACRO_PDF417_CONTROL_BLOCK:
          codeIndex--;
          end = TRUE;
          break;
        case BEGIN_MACRO_PDF417_OPTIONAL_FIELD:
          codeIndex--;
          end = TRUE;
          break;
        case MACRO_PDF417_TERMINATOR:
          codeIndex--;
          end = TRUE;
          break;
        case MODE_SHIFT_TO_BYTE_COMPACTION_MODE:
          textCompactionData[index] = MODE_SHIFT_TO_BYTE_COMPACTION_MODE;
          code = codewords[codeIndex++];
          byteCompactionData[index] = code;
          index++;
          break;
        case BYTE_COMPACTION_MODE_LATCH_6:
          codeIndex--;
          end = TRUE;
          break;
      }
    }
  }
  decodeTextCompaction(textCompactionData, byteCompactionData, index, result);
  return codeIndex;
}
void CBC_DecodedBitStreamPaser::decodeTextCompaction(
    CFX_Int32Array& textCompactionData,
    CFX_Int32Array& byteCompactionData,
    int32_t length,
    CFX_ByteString& result) {
  Mode subMode = ALPHA;
  Mode priorToShiftMode = ALPHA;
  int32_t i = 0;
  while (i < length) {
    int32_t subModeCh = textCompactionData[i];
    FX_CHAR ch = 0;
    switch (subMode) {
      case ALPHA:
        if (subModeCh < 26) {
          ch = (FX_CHAR)('A' + subModeCh);
        } else {
          if (subModeCh == 26) {
            ch = ' ';
          } else if (subModeCh == LL) {
            subMode = LOWER;
          } else if (subModeCh == ML) {
            subMode = MIXED;
          } else if (subModeCh == PS) {
            priorToShiftMode = subMode;
            subMode = PUNCT_SHIFT;
          } else if (subModeCh == MODE_SHIFT_TO_BYTE_COMPACTION_MODE) {
            result += (FX_CHAR)byteCompactionData[i];
          } else if (subModeCh == TEXT_COMPACTION_MODE_LATCH) {
            subMode = ALPHA;
          }
        }
        break;
      case LOWER:
        if (subModeCh < 26) {
          ch = (FX_CHAR)('a' + subModeCh);
        } else {
          if (subModeCh == 26) {
            ch = ' ';
          } else if (subModeCh == AS) {
            priorToShiftMode = subMode;
            subMode = ALPHA_SHIFT;
          } else if (subModeCh == ML) {
            subMode = MIXED;
          } else if (subModeCh == PS) {
            priorToShiftMode = subMode;
            subMode = PUNCT_SHIFT;
          } else if (subModeCh == MODE_SHIFT_TO_BYTE_COMPACTION_MODE) {
            result += (FX_CHAR)byteCompactionData[i];
          } else if (subModeCh == TEXT_COMPACTION_MODE_LATCH) {
            subMode = ALPHA;
          }
        }
        break;
      case MIXED:
        if (subModeCh < PL) {
          ch = MIXED_CHARS[subModeCh];
        } else {
          if (subModeCh == PL) {
            subMode = PUNCT;
          } else if (subModeCh == 26) {
            ch = ' ';
          } else if (subModeCh == LL) {
            subMode = LOWER;
          } else if (subModeCh == AL) {
            subMode = ALPHA;
          } else if (subModeCh == PS) {
            priorToShiftMode = subMode;
            subMode = PUNCT_SHIFT;
          } else if (subModeCh == MODE_SHIFT_TO_BYTE_COMPACTION_MODE) {
            result += (FX_CHAR)byteCompactionData[i];
          } else if (subModeCh == TEXT_COMPACTION_MODE_LATCH) {
            subMode = ALPHA;
          }
        }
        break;
      case PUNCT:
        if (subModeCh < PAL) {
          ch = PUNCT_CHARS[subModeCh];
        } else {
          if (subModeCh == PAL) {
            subMode = ALPHA;
          } else if (subModeCh == MODE_SHIFT_TO_BYTE_COMPACTION_MODE) {
            result += (FX_CHAR)byteCompactionData[i];
          } else if (subModeCh == TEXT_COMPACTION_MODE_LATCH) {
            subMode = ALPHA;
          }
        }
        break;
      case ALPHA_SHIFT:
        subMode = priorToShiftMode;
        if (subModeCh < 26) {
          ch = (FX_CHAR)('A' + subModeCh);
        } else {
          if (subModeCh == 26) {
            ch = ' ';
          } else if (subModeCh == TEXT_COMPACTION_MODE_LATCH) {
            subMode = ALPHA;
          }
        }
        break;
      case PUNCT_SHIFT:
        subMode = priorToShiftMode;
        if (subModeCh < PAL) {
          ch = PUNCT_CHARS[subModeCh];
        } else {
          if (subModeCh == PAL) {
            subMode = ALPHA;
          } else if (subModeCh == MODE_SHIFT_TO_BYTE_COMPACTION_MODE) {
            result += (FX_CHAR)byteCompactionData[i];
          } else if (subModeCh == TEXT_COMPACTION_MODE_LATCH) {
            subMode = ALPHA;
          }
        }
        break;
    }
    if (ch != 0) {
      result += ch;
    }
    i++;
  }
}
int32_t CBC_DecodedBitStreamPaser::byteCompaction(int32_t mode,
                                                  CFX_Int32Array& codewords,
                                                  int32_t codeIndex,
                                                  CFX_ByteString& result) {
  if (mode == BYTE_COMPACTION_MODE_LATCH) {
    int32_t count = 0;
    int64_t value = 0;
    FX_WORD* decodedData = FX_Alloc(FX_WORD, 6);
    CFX_Int32Array byteCompactedCodewords;
    byteCompactedCodewords.SetSize(6);
    FX_BOOL end = FALSE;
    int32_t nextCode = codewords[codeIndex++];
    while ((codeIndex < codewords[0]) && !end) {
      byteCompactedCodewords[count++] = nextCode;
      value = 900 * value + nextCode;
      nextCode = codewords[codeIndex++];
      if (nextCode == TEXT_COMPACTION_MODE_LATCH ||
          nextCode == BYTE_COMPACTION_MODE_LATCH ||
          nextCode == NUMERIC_COMPACTION_MODE_LATCH ||
          nextCode == BYTE_COMPACTION_MODE_LATCH_6 ||
          nextCode == BEGIN_MACRO_PDF417_CONTROL_BLOCK ||
          nextCode == BEGIN_MACRO_PDF417_OPTIONAL_FIELD ||
          nextCode == MACRO_PDF417_TERMINATOR) {
        codeIndex--;
        end = TRUE;
      } else {
        if ((count % 5 == 0) && (count > 0)) {
          int32_t j = 0;
          for (; j < 6; ++j) {
            decodedData[5 - j] = (FX_WORD)(value % 256);
            value >>= 8;
          }
          for (j = 0; j < 6; ++j) {
            result += (FX_CHAR)decodedData[j];
          }
          count = 0;
        }
      }
    }
    FX_Free(decodedData);
    if (codeIndex == codewords[0] && nextCode < TEXT_COMPACTION_MODE_LATCH) {
      byteCompactedCodewords[count++] = nextCode;
    }
    for (int32_t i = 0; i < count; i++) {
      result += (FX_CHAR)(FX_WORD)byteCompactedCodewords[i];
    }
  } else if (mode == BYTE_COMPACTION_MODE_LATCH_6) {
    int32_t count = 0;
    int64_t value = 0;
    FX_BOOL end = FALSE;
    while (codeIndex < codewords[0] && !end) {
      int32_t code = codewords[codeIndex++];
      if (code < TEXT_COMPACTION_MODE_LATCH) {
        count++;
        value = 900 * value + code;
      } else {
        if (code == TEXT_COMPACTION_MODE_LATCH ||
            code == BYTE_COMPACTION_MODE_LATCH ||
            code == NUMERIC_COMPACTION_MODE_LATCH ||
            code == BYTE_COMPACTION_MODE_LATCH_6 ||
            code == BEGIN_MACRO_PDF417_CONTROL_BLOCK ||
            code == BEGIN_MACRO_PDF417_OPTIONAL_FIELD ||
            code == MACRO_PDF417_TERMINATOR) {
          codeIndex--;
          end = TRUE;
        }
      }
      if ((count % 5 == 0) && (count > 0)) {
        FX_WORD* decodedData = FX_Alloc(FX_WORD, 6);
        int32_t j = 0;
        for (; j < 6; ++j) {
          decodedData[5 - j] = (FX_WORD)(value & 0xFF);
          value >>= 8;
        }
        for (j = 0; j < 6; ++j) {
          result += (FX_CHAR)decodedData[j];
        }
        count = 0;
        FX_Free(decodedData);
      }
    }
  }
  return codeIndex;
}
int32_t CBC_DecodedBitStreamPaser::numericCompaction(CFX_Int32Array& codewords,
                                                     int32_t codeIndex,
                                                     CFX_ByteString& result,
                                                     int32_t& e) {
  int32_t count = 0;
  FX_BOOL end = FALSE;
  CFX_Int32Array numericCodewords;
  numericCodewords.SetSize(MAX_NUMERIC_CODEWORDS);
  while (codeIndex < codewords[0] && !end) {
    int32_t code = codewords[codeIndex++];
    if (codeIndex == codewords[0]) {
      end = TRUE;
    }
    if (code < TEXT_COMPACTION_MODE_LATCH) {
      numericCodewords[count] = code;
      count++;
    } else {
      if (code == TEXT_COMPACTION_MODE_LATCH ||
          code == BYTE_COMPACTION_MODE_LATCH ||
          code == BYTE_COMPACTION_MODE_LATCH_6 ||
          code == BEGIN_MACRO_PDF417_CONTROL_BLOCK ||
          code == BEGIN_MACRO_PDF417_OPTIONAL_FIELD ||
          code == MACRO_PDF417_TERMINATOR) {
        codeIndex--;
        end = TRUE;
      }
    }
    if (count % MAX_NUMERIC_CODEWORDS == 0 ||
        code == NUMERIC_COMPACTION_MODE_LATCH || end) {
      CFX_ByteString s = decodeBase900toBase10(numericCodewords, count, e);
      BC_EXCEPTION_CHECK_ReturnValue(e, -1);
      result += s;
      count = 0;
    }
  }
  return codeIndex;
}
CFX_ByteString CBC_DecodedBitStreamPaser::decodeBase900toBase10(
    CFX_Int32Array& codewords,
    int32_t count,
    int32_t& e) {
  BigInteger result = 0;
  BigInteger nineHundred(900);
  for (int32_t i = 0; i < count; i++) {
    result = result * nineHundred + BigInteger(codewords[i]);
  }
  CFX_ByteString resultString(bigIntegerToString(result).c_str());
  if (resultString.GetAt(0) != '1') {
    e = BCExceptionFormatInstance;
    return ' ';
  }
  return resultString.Mid(1, resultString.GetLength() - 1);
}
