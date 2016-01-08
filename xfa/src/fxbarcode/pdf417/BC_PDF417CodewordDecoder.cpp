// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2013 ZXing authors
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
#include "BC_PDF417Common.h"
#include "BC_PDF417CodewordDecoder.h"
#define SYMBOL_TABLE_Length 2787
#define Float_MAX_VALUE 2147483647
FX_FLOAT CBC_PDF417CodewordDecoder::RATIOS_TABLE[2787][8] = {{0}};
CBC_PDF417CodewordDecoder::CBC_PDF417CodewordDecoder() {}
CBC_PDF417CodewordDecoder::~CBC_PDF417CodewordDecoder() {}
void CBC_PDF417CodewordDecoder::Initialize() {
  for (int32_t i = 0; i < SYMBOL_TABLE_Length; i++) {
    int32_t currentSymbol = CBC_PDF417Common::SYMBOL_TABLE[i];
    int32_t currentBit = currentSymbol & 0x1;
    for (int32_t j = 0; j < CBC_PDF417Common::BARS_IN_MODULE; j++) {
      FX_FLOAT size = 0.0f;
      while ((currentSymbol & 0x1) == currentBit) {
        size += 1.0f;
        currentSymbol >>= 1;
      }
      currentBit = currentSymbol & 0x1;
      RATIOS_TABLE[i][CBC_PDF417Common::BARS_IN_MODULE - j - 1] =
          size / CBC_PDF417Common::MODULES_IN_CODEWORD;
    }
  }
}
void CBC_PDF417CodewordDecoder::Finalize() {}
int32_t CBC_PDF417CodewordDecoder::getDecodedValue(
    CFX_Int32Array& moduleBitCount) {
  CFX_Int32Array* array = sampleBitCounts(moduleBitCount);
  int32_t decodedValue = getDecodedCodewordValue(*array);
  delete array;
  if (decodedValue != -1) {
    return decodedValue;
  }
  return getClosestDecodedValue(moduleBitCount);
}
CFX_Int32Array* CBC_PDF417CodewordDecoder::sampleBitCounts(
    CFX_Int32Array& moduleBitCount) {
  FX_FLOAT bitCountSum =
      (FX_FLOAT)CBC_PDF417Common::getBitCountSum(moduleBitCount);
  CFX_Int32Array* bitCount = new CFX_Int32Array();
  bitCount->SetSize(CBC_PDF417Common::BARS_IN_MODULE);
  int32_t bitCountIndex = 0;
  int32_t sumPreviousBits = 0;
  for (int32_t i = 0; i < CBC_PDF417Common::MODULES_IN_CODEWORD; i++) {
    FX_FLOAT sampleIndex =
        bitCountSum / (2 * CBC_PDF417Common::MODULES_IN_CODEWORD) +
        (i * bitCountSum) / CBC_PDF417Common::MODULES_IN_CODEWORD;
    if (sumPreviousBits + moduleBitCount.GetAt(bitCountIndex) <= sampleIndex) {
      sumPreviousBits += moduleBitCount.GetAt(bitCountIndex);
      bitCountIndex++;
    }
    bitCount->SetAt(bitCountIndex, bitCount->GetAt(bitCountIndex) + 1);
  }
  return bitCount;
}
int32_t CBC_PDF417CodewordDecoder::getDecodedCodewordValue(
    CFX_Int32Array& moduleBitCount) {
  int32_t decodedValue = getBitValue(moduleBitCount);
  return CBC_PDF417Common::getCodeword(decodedValue) == -1 ? -1 : decodedValue;
}
int32_t CBC_PDF417CodewordDecoder::getBitValue(CFX_Int32Array& moduleBitCount) {
  int64_t result = 0;
  for (int32_t i = 0; i < moduleBitCount.GetSize(); i++) {
    for (int32_t bit = 0; bit < moduleBitCount.GetAt(i); bit++) {
      result = (result << 1) | (i % 2 == 0 ? 1 : 0);
    }
  }
  return (int32_t)result;
}
int32_t CBC_PDF417CodewordDecoder::getClosestDecodedValue(
    CFX_Int32Array& moduleBitCount) {
  int32_t bitCountSum = CBC_PDF417Common::getBitCountSum(moduleBitCount);
  CFX_FloatArray bitCountRatios;
  bitCountRatios.SetSize(CBC_PDF417Common::BARS_IN_MODULE);
  for (int32_t i = 0; i < bitCountRatios.GetSize(); i++) {
    bitCountRatios[i] = moduleBitCount.GetAt(i) / (FX_FLOAT)bitCountSum;
  }
  FX_FLOAT bestMatchError = (FX_FLOAT)Float_MAX_VALUE;
  int32_t bestMatch = -1;
  for (int32_t j = 0; j < SYMBOL_TABLE_Length; j++) {
    FX_FLOAT error = 0.0f;
    for (int32_t k = 0; k < CBC_PDF417Common::BARS_IN_MODULE; k++) {
      FX_FLOAT diff = RATIOS_TABLE[j][k] - bitCountRatios[k];
      error += diff * diff;
    }
    if (error < bestMatchError) {
      bestMatchError = error;
      bestMatch = CBC_PDF417Common::SYMBOL_TABLE[j];
    }
  }
  return bestMatch;
}
