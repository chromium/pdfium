// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2006 Jeremias Maerki
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

#include "fxbarcode/datamatrix/BC_SymbolInfo.h"

#include "core/fxcrt/fx_memory.h"
#include "fxbarcode/common/BC_CommonBitMatrix.h"
#include "fxbarcode/datamatrix/BC_DataMatrixSymbolInfo144.h"
#include "fxbarcode/datamatrix/BC_Encoder.h"

namespace {

constexpr size_t kSymbolsCount = 30;

CBC_SymbolInfo* g_symbols[kSymbolsCount] = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

constexpr CBC_SymbolInfo::Data kSymbolData[] = {
    {3, 5, 8, 8, 1, 3, 5},           {5, 7, 10, 10, 1, 5, 7},
    {5, 7, 16, 6, 1, 5, 7},          {8, 10, 12, 12, 1, 8, 10},
    {10, 11, 14, 6, 2, 10, 11},      {12, 12, 14, 14, 1, 12, 12},
    {16, 14, 24, 10, 1, 16, 14},     {18, 14, 16, 16, 1, 18, 14},
    {22, 18, 18, 18, 1, 22, 18},     {22, 18, 16, 10, 2, 22, 18},
    {30, 20, 20, 20, 1, 30, 20},     {32, 24, 16, 14, 2, 32, 24},
    {36, 24, 22, 22, 1, 36, 24},     {44, 28, 24, 24, 1, 44, 28},
    {49, 28, 22, 14, 2, 49, 28},     {62, 36, 14, 14, 4, 62, 36},
    {86, 42, 16, 16, 4, 86, 42},     {114, 48, 18, 18, 4, 114, 48},
    {144, 56, 20, 20, 4, 144, 56},   {174, 68, 22, 22, 4, 174, 68},
    {204, 84, 24, 24, 4, 102, 42},   {280, 112, 14, 14, 16, 140, 56},
    {368, 144, 16, 16, 16, 92, 36},  {456, 192, 18, 18, 16, 114, 48},
    {576, 224, 20, 20, 16, 144, 56}, {696, 272, 22, 22, 16, 174, 68},
    {816, 336, 24, 24, 16, 136, 56}, {1050, 408, 18, 18, 36, 175, 68},
    {1304, 496, 20, 20, 36, 163, 62}};

constexpr size_t kSymbolDataSize = FX_ArraySize(kSymbolData);
static_assert(kSymbolDataSize + 1 == kSymbolsCount, "Wrong kSymbolDataSize");

}  // namespace

// static
void CBC_SymbolInfo::Initialize() {
  for (size_t i = 0; i < kSymbolDataSize; ++i)
    g_symbols[i] = new CBC_SymbolInfo(&kSymbolData[i]);
  g_symbols[kSymbolDataSize] = new CBC_DataMatrixSymbolInfo144();
}

// static
void CBC_SymbolInfo::Finalize() {
  for (size_t i = 0; i < kSymbolsCount; i++) {
    delete g_symbols[i];
    g_symbols[i] = nullptr;
  }
}

CBC_SymbolInfo::CBC_SymbolInfo(const Data* data)
    : data_(data), rectangular_(data_->matrix_width != data_->matrix_height) {}

CBC_SymbolInfo::~CBC_SymbolInfo() = default;

const CBC_SymbolInfo* CBC_SymbolInfo::Lookup(size_t iDataCodewords,
                                             bool bAllowRectangular) {
  for (size_t i = 0; i < kSymbolsCount; i++) {
    CBC_SymbolInfo* symbol = g_symbols[i];
    if (symbol->rectangular_ && !bAllowRectangular)
      continue;

    if (iDataCodewords <= symbol->dataCapacity())
      return symbol;
  }
  return nullptr;
}

int32_t CBC_SymbolInfo::getHorizontalDataRegions() const {
  switch (data_->data_regions) {
    case 1:
      return 1;
    case 2:
      return 2;
    case 4:
      return 2;
    case 16:
      return 4;
    case 36:
      return 6;
    default:
      NOTREACHED();
      return 0;
  }
}

int32_t CBC_SymbolInfo::getVerticalDataRegions() const {
  switch (data_->data_regions) {
    case 1:
      return 1;
    case 2:
      return 1;
    case 4:
      return 2;
    case 16:
      return 4;
    case 36:
      return 6;
    default:
      NOTREACHED();
      return 0;
  }
}

int32_t CBC_SymbolInfo::getSymbolDataWidth() const {
  return getHorizontalDataRegions() * data_->matrix_width;
}

int32_t CBC_SymbolInfo::getSymbolDataHeight() const {
  return getVerticalDataRegions() * data_->matrix_height;
}

int32_t CBC_SymbolInfo::getSymbolWidth() const {
  return getSymbolDataWidth() + (getHorizontalDataRegions() * 2);
}

int32_t CBC_SymbolInfo::getSymbolHeight() const {
  return getSymbolDataHeight() + (getVerticalDataRegions() * 2);
}

size_t CBC_SymbolInfo::getCodewordCount() const {
  return data_->data_capacity + data_->error_codewords;
}

size_t CBC_SymbolInfo::getInterleavedBlockCount() const {
  return data_->data_capacity / data_->rs_block_data;
}

size_t CBC_SymbolInfo::getDataLengthForInterleavedBlock() const {
  return data_->rs_block_data;
}

size_t CBC_SymbolInfo::getErrorLengthForInterleavedBlock() const {
  return data_->rs_block_error;
}
