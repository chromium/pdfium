// Copyright 2014 The PDFium Authors
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

#include <array>
#include <iterator>

#include "core/fxcrt/notreached.h"
#include "fxbarcode/datamatrix/BC_DataMatrixSymbolInfo144.h"
#include "fxbarcode/datamatrix/BC_Encoder.h"

namespace {

constexpr size_t kSymbolsCount = 30;
constexpr size_t kSymbolDataSize = kSymbolsCount - 1;

std::array<CBC_SymbolInfo*, kSymbolsCount> g_symbols = {};

constexpr std::array<CBC_SymbolInfo::Data, kSymbolDataSize> kSymbolData = {
    {{3, 5, 3, 5, 8, 8, 1},           {5, 7, 5, 7, 10, 10, 1},
     {5, 7, 5, 7, 16, 6, 1},          {8, 10, 8, 10, 12, 12, 1},
     {10, 11, 10, 11, 14, 6, 2},      {12, 12, 12, 12, 14, 14, 1},
     {16, 14, 16, 14, 24, 10, 1},     {18, 14, 18, 14, 16, 16, 1},
     {22, 18, 22, 18, 18, 18, 1},     {22, 18, 22, 18, 16, 10, 2},
     {30, 20, 30, 20, 20, 20, 1},     {32, 24, 32, 24, 16, 14, 2},
     {36, 24, 36, 24, 22, 22, 1},     {44, 28, 44, 28, 24, 24, 1},
     {49, 28, 49, 28, 22, 14, 2},     {62, 36, 62, 36, 14, 14, 4},
     {86, 42, 86, 42, 16, 16, 4},     {114, 48, 114, 48, 18, 18, 4},
     {144, 56, 144, 56, 20, 20, 4},   {174, 68, 174, 68, 22, 22, 4},
     {204, 84, 102, 42, 24, 24, 4},   {280, 112, 140, 56, 14, 14, 16},
     {368, 144, 92, 36, 16, 16, 16},  {456, 192, 114, 48, 18, 18, 16},
     {576, 224, 144, 56, 20, 20, 16}, {696, 272, 174, 68, 22, 22, 16},
     {816, 336, 136, 56, 24, 24, 16}, {1050, 408, 175, 68, 18, 18, 36},
     {1304, 496, 163, 62, 20, 20, 36}}};

}  // namespace

// static
void CBC_SymbolInfo::Initialize() {
  for (size_t i = 0; i < kSymbolDataSize; ++i)
    g_symbols[i] = new CBC_SymbolInfo(&kSymbolData[i]);
  g_symbols[kSymbolDataSize] = new CBC_DataMatrixSymbolInfo144();
}

// static
void CBC_SymbolInfo::Finalize() {
  for (size_t i = 0; i < kSymbolsCount; ++i) {
    delete g_symbols[i];
    g_symbols[i] = nullptr;
  }
}

CBC_SymbolInfo::CBC_SymbolInfo(const Data* data) : data_(data) {}

CBC_SymbolInfo::~CBC_SymbolInfo() = default;

const CBC_SymbolInfo* CBC_SymbolInfo::Lookup(size_t data_codewords,
                                             bool allow_rectangular) {
  for (size_t i = 0; i < kSymbolsCount; ++i) {
    CBC_SymbolInfo* symbol = g_symbols[i];
    if (symbol->is_rectangular() && !allow_rectangular)
      continue;

    if (data_codewords <= symbol->data_capacity())
      return symbol;
  }
  return nullptr;
}

int32_t CBC_SymbolInfo::GetHorizontalDataRegions() const {
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
      NOTREACHED_NORETURN();
  }
}

int32_t CBC_SymbolInfo::GetVerticalDataRegions() const {
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
      NOTREACHED_NORETURN();
  }
}

int32_t CBC_SymbolInfo::GetSymbolDataWidth() const {
  return GetHorizontalDataRegions() * data_->matrix_width;
}

int32_t CBC_SymbolInfo::GetSymbolDataHeight() const {
  return GetVerticalDataRegions() * data_->matrix_height;
}

int32_t CBC_SymbolInfo::GetSymbolWidth() const {
  return GetSymbolDataWidth() + (GetHorizontalDataRegions() * 2);
}

int32_t CBC_SymbolInfo::GetSymbolHeight() const {
  return GetSymbolDataHeight() + (GetVerticalDataRegions() * 2);
}

size_t CBC_SymbolInfo::GetInterleavedBlockCount() const {
  return data_->data_capacity / data_->rs_block_data;
}

size_t CBC_SymbolInfo::GetDataLengthForInterleavedBlock() const {
  return data_->rs_block_data;
}

size_t CBC_SymbolInfo::GetErrorLengthForInterleavedBlock() const {
  return data_->rs_block_error;
}
