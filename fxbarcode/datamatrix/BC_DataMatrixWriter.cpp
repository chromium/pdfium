// Copyright 2014 The PDFium Authors
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

#include "fxbarcode/datamatrix/BC_DataMatrixWriter.h"

#include <stdint.h>

#include <memory>

#include "core/fxcrt/data_vector.h"
#include "fxbarcode/BC_TwoDimWriter.h"
#include "fxbarcode/BC_Writer.h"
#include "fxbarcode/common/BC_CommonBitMatrix.h"
#include "fxbarcode/common/BC_CommonByteMatrix.h"
#include "fxbarcode/datamatrix/BC_ASCIIEncoder.h"
#include "fxbarcode/datamatrix/BC_Base256Encoder.h"
#include "fxbarcode/datamatrix/BC_C40Encoder.h"
#include "fxbarcode/datamatrix/BC_DataMatrixSymbolInfo144.h"
#include "fxbarcode/datamatrix/BC_DefaultPlacement.h"
#include "fxbarcode/datamatrix/BC_EdifactEncoder.h"
#include "fxbarcode/datamatrix/BC_Encoder.h"
#include "fxbarcode/datamatrix/BC_EncoderContext.h"
#include "fxbarcode/datamatrix/BC_ErrorCorrection.h"
#include "fxbarcode/datamatrix/BC_HighLevelEncoder.h"
#include "fxbarcode/datamatrix/BC_SymbolInfo.h"
#include "fxbarcode/datamatrix/BC_TextEncoder.h"
#include "fxbarcode/datamatrix/BC_X12Encoder.h"
#include "third_party/base/check.h"

namespace {

std::unique_ptr<CBC_CommonByteMatrix> EncodeLowLevel(
    CBC_DefaultPlacement* placement,
    const CBC_SymbolInfo* symbolInfo) {
  int32_t symbolWidth = symbolInfo->GetSymbolDataWidth();
  DCHECK(symbolWidth);
  int32_t symbolHeight = symbolInfo->GetSymbolDataHeight();
  DCHECK(symbolHeight);
  int32_t width = symbolInfo->GetSymbolWidth();
  DCHECK(width);
  int32_t height = symbolInfo->GetSymbolHeight();
  DCHECK(height);

  auto matrix = std::make_unique<CBC_CommonByteMatrix>(width, height);
  int32_t matrixY = 0;
  for (int32_t y = 0; y < symbolHeight; y++) {
    int32_t matrixX;
    if ((y % symbolInfo->matrix_height()) == 0) {
      matrixX = 0;
      for (int32_t x = 0; x < width; x++) {
        matrix->Set(matrixX, matrixY, x % 2 == 0);
        matrixX++;
      }
      matrixY++;
    }
    matrixX = 0;
    for (int32_t x = 0; x < symbolWidth; x++) {
      if (x % symbolInfo->matrix_width() == 0) {
        matrix->Set(matrixX, matrixY, true);
        matrixX++;
      }
      matrix->Set(matrixX, matrixY, placement->GetBit(x, y));
      matrixX++;
      if (x % symbolInfo->matrix_width() == symbolInfo->matrix_width() - 1) {
        matrix->Set(matrixX, matrixY, y % 2 == 0);
        matrixX++;
      }
    }
    matrixY++;
    if (y % symbolInfo->matrix_height() == symbolInfo->matrix_height() - 1) {
      matrixX = 0;
      for (int32_t x = 0; x < width; x++) {
        matrix->Set(matrixX, matrixY, true);
        matrixX++;
      }
      matrixY++;
    }
  }
  return matrix;
}

}  // namespace

CBC_DataMatrixWriter::CBC_DataMatrixWriter() : CBC_TwoDimWriter(true) {}

CBC_DataMatrixWriter::~CBC_DataMatrixWriter() = default;

bool CBC_DataMatrixWriter::SetErrorCorrectionLevel(int32_t level) {
  set_error_correction_level(level);
  return true;
}

DataVector<uint8_t> CBC_DataMatrixWriter::Encode(const WideString& contents,
                                                 int32_t* pOutWidth,
                                                 int32_t* pOutHeight) {
  WideString encoded = CBC_HighLevelEncoder::EncodeHighLevel(contents);
  if (encoded.IsEmpty())
    return DataVector<uint8_t>();

  const CBC_SymbolInfo* pSymbolInfo =
      CBC_SymbolInfo::Lookup(encoded.GetLength(), false);
  if (!pSymbolInfo)
    return DataVector<uint8_t>();

  WideString codewords =
      CBC_ErrorCorrection::EncodeECC200(encoded, pSymbolInfo);
  if (codewords.IsEmpty())
    return DataVector<uint8_t>();

  int32_t width = pSymbolInfo->GetSymbolDataWidth();
  DCHECK(width);
  int32_t height = pSymbolInfo->GetSymbolDataHeight();
  DCHECK(height);

  auto placement =
      std::make_unique<CBC_DefaultPlacement>(codewords, width, height);
  auto bytematrix = EncodeLowLevel(placement.get(), pSymbolInfo);
  DCHECK(bytematrix);

  *pOutWidth = bytematrix->GetWidth();
  *pOutHeight = bytematrix->GetHeight();
  return bytematrix->TakeArray();
}
