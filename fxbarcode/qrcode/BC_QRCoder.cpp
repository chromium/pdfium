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

#include <utility>

#include "core/fxcrt/numerics/safe_conversions.h"
#include "fxbarcode/common/BC_CommonByteMatrix.h"
#include "fxbarcode/qrcode/BC_QRCoder.h"
#include "fxbarcode/qrcode/BC_QRCoderErrorCorrectionLevel.h"
#include "fxbarcode/qrcode/BC_QRCoderMode.h"

CBC_QRCoder::CBC_QRCoder() = default;

CBC_QRCoder::~CBC_QRCoder() = default;

const CBC_QRCoderErrorCorrectionLevel* CBC_QRCoder::GetECLevel() const {
  return ec_level_;
}

int32_t CBC_QRCoder::GetVersion() const {
  return version_;
}

int32_t CBC_QRCoder::GetMatrixWidth() const {
  return matrix_width_;
}

int32_t CBC_QRCoder::GetMaskPattern() const {
  return mask_pattern_;
}

int32_t CBC_QRCoder::GetNumTotalBytes() const {
  return num_total_bytes_;
}

int32_t CBC_QRCoder::GetNumDataBytes() const {
  return num_data_bytes_;
}

int32_t CBC_QRCoder::GetNumRSBlocks() const {
  return num_rsblocks_;
}

std::unique_ptr<CBC_CommonByteMatrix> CBC_QRCoder::TakeMatrix() {
  return std::move(matrix_);
}

bool CBC_QRCoder::IsValid() const {
  return ec_level_ && version_ != -1 && matrix_width_ != -1 &&
         mask_pattern_ != -1 && num_total_bytes_ != -1 &&
         num_data_bytes_ != -1 && num_ecbytes_ != -1 && num_rsblocks_ != -1 &&
         IsValidMaskPattern(mask_pattern_) &&
         num_total_bytes_ == num_data_bytes_ + num_ecbytes_ && matrix_ &&
         matrix_width_ == pdfium::checked_cast<int32_t>(matrix_->GetWidth()) &&
         matrix_->GetWidth() == matrix_->GetHeight();
}

void CBC_QRCoder::SetECLevel(const CBC_QRCoderErrorCorrectionLevel* ecLevel) {
  ec_level_ = ecLevel;
}

void CBC_QRCoder::SetVersion(int32_t version) {
  version_ = version;
}

void CBC_QRCoder::SetMatrixWidth(int32_t width) {
  matrix_width_ = width;
}

void CBC_QRCoder::SetMaskPattern(int32_t pattern) {
  mask_pattern_ = pattern;
}

void CBC_QRCoder::SetNumDataBytes(int32_t bytes) {
  num_data_bytes_ = bytes;
}

void CBC_QRCoder::SetNumTotalBytes(int32_t value) {
  num_total_bytes_ = value;
}

void CBC_QRCoder::SetNumRSBlocks(int32_t block) {
  num_rsblocks_ = block;
}

void CBC_QRCoder::SetNumECBytes(int32_t value) {
  num_ecbytes_ = value;
}

bool CBC_QRCoder::IsValidMaskPattern(int32_t maskPattern) {
  return maskPattern >= 0 && maskPattern < kNumMaskPatterns;
}

void CBC_QRCoder::SetMatrix(std::unique_ptr<CBC_CommonByteMatrix> pMatrix) {
  matrix_ = std::move(pMatrix);
}
