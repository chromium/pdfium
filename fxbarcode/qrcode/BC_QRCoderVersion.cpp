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

#include "fxbarcode/qrcode/BC_QRCoderVersion.h"

#include <memory>
#include <vector>

#include "fxbarcode/common/BC_CommonBitMatrix.h"
#include "fxbarcode/qrcode/BC_QRCoderBitVector.h"
#include "fxbarcode/qrcode/BC_QRCoderECBlocksData.h"
#include "fxbarcode/qrcode/BC_QRCoderErrorCorrectionLevel.h"

namespace {

std::vector<std::unique_ptr<CBC_QRCoderVersion>>* g_VERSION = nullptr;

}  // namespace

CBC_QRCoderVersion::CBC_QRCoderVersion(int32_t versionNumber,
                                       const CBC_QRCoderECBlockData data[4])
    : m_versionNumber(versionNumber) {
  m_ecBlocksArray[0] = std::make_unique<CBC_QRCoderECBlocks>(data[0]);
  m_ecBlocksArray[1] = std::make_unique<CBC_QRCoderECBlocks>(data[1]);
  m_ecBlocksArray[2] = std::make_unique<CBC_QRCoderECBlocks>(data[2]);
  m_ecBlocksArray[3] = std::make_unique<CBC_QRCoderECBlocks>(data[3]);
  m_totalCodeWords = m_ecBlocksArray[0]->GetTotalDataCodeWords();
}

CBC_QRCoderVersion::~CBC_QRCoderVersion() = default;

// static
void CBC_QRCoderVersion::Initialize() {
  g_VERSION = new std::vector<std::unique_ptr<CBC_QRCoderVersion>>();
}

// static
void CBC_QRCoderVersion::Finalize() {
  delete g_VERSION;
  g_VERSION = nullptr;
}

// static
const CBC_QRCoderVersion* CBC_QRCoderVersion::GetVersionForNumber(
    int32_t versionNumber) {
  if (g_VERSION->empty()) {
    for (int i = 0; i < kMaxVersion; ++i) {
      g_VERSION->push_back(
          std::make_unique<CBC_QRCoderVersion>(i + 1, fxbarcode::kECBData[i]));
    }
  }
  if (versionNumber < 1 || versionNumber > kMaxVersion)
    return nullptr;
  return (*g_VERSION)[versionNumber - 1].get();
}

int32_t CBC_QRCoderVersion::GetVersionNumber() const {
  return m_versionNumber;
}

int32_t CBC_QRCoderVersion::GetTotalCodeWords() const {
  return m_totalCodeWords;
}

int32_t CBC_QRCoderVersion::GetDimensionForVersion() const {
  return 17 + 4 * m_versionNumber;
}

const CBC_QRCoderECBlocks* CBC_QRCoderVersion::GetECBlocksForLevel(
    const CBC_QRCoderErrorCorrectionLevel& ecLevel) const {
  return m_ecBlocksArray[ecLevel.Ordinal()].get();
}
