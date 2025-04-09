// Copyright 2014 The PDFium Authors
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

#include "fxbarcode/common/reedsolomon/BC_ReedSolomonGF256.h"

#include <vector>

#include "fxbarcode/common/reedsolomon/BC_ReedSolomonGF256Poly.h"

CBC_ReedSolomonGF256::CBC_ReedSolomonGF256(int32_t primitive) {
  int32_t x = 1;
  for (int32_t j = 0; j < 256; j++) {
    exp_table_[j] = x;
    x <<= 1;
    if (x >= 0x100) {
      x ^= primitive;
    }
  }
  for (int32_t i = 0; i < 255; i++) {
    log_table_[exp_table_[i]] = i;
  }
  log_table_[0] = 0;
}

void CBC_ReedSolomonGF256::Init() {
  zero_ =
      std::make_unique<CBC_ReedSolomonGF256Poly>(this, std::vector<int32_t>{0});
  one_ =
      std::make_unique<CBC_ReedSolomonGF256Poly>(this, std::vector<int32_t>{1});
}

CBC_ReedSolomonGF256::~CBC_ReedSolomonGF256() = default;

std::unique_ptr<CBC_ReedSolomonGF256Poly> CBC_ReedSolomonGF256::BuildMonomial(
    int32_t degree,
    int32_t coefficient) {
  if (degree < 0) {
    return nullptr;
  }

  if (coefficient == 0) {
    return zero_->Clone();
  }

  std::vector<int32_t> coefficients(degree + 1);
  coefficients[0] = coefficient;
  return std::make_unique<CBC_ReedSolomonGF256Poly>(this, coefficients);
}

// static
int32_t CBC_ReedSolomonGF256::AddOrSubtract(int32_t a, int32_t b) {
  return a ^ b;
}

int32_t CBC_ReedSolomonGF256::Exp(int32_t a) {
  return exp_table_[a];
}

std::optional<int32_t> CBC_ReedSolomonGF256::Inverse(int32_t a) {
  if (a == 0) {
    return std::nullopt;
  }
  return exp_table_[255 - log_table_[a]];
}

int32_t CBC_ReedSolomonGF256::Multiply(int32_t a, int32_t b) {
  if (a == 0 || b == 0) {
    return 0;
  }
  if (a == 1) {
    return b;
  }
  if (b == 1) {
    return a;
  }
  return exp_table_[(log_table_[a] + log_table_[b]) % 255];
}
