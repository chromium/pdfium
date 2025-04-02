// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
/*
 * Copyright 2011 ZXing authors
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

#include "fxbarcode/cbc_codebase.h"

#include <utility>

#include "fxbarcode/BC_Writer.h"

CBC_CodeBase::CBC_CodeBase(std::unique_ptr<CBC_Writer> pWriter)
    : bc_writer_(std::move(pWriter)) {}

CBC_CodeBase::~CBC_CodeBase() = default;

void CBC_CodeBase::SetTextLocation(BC_TEXT_LOC location) {
  bc_writer_->SetTextLocation(location);
}

bool CBC_CodeBase::SetWideNarrowRatio(int8_t ratio) {
  return bc_writer_->SetWideNarrowRatio(ratio);
}

bool CBC_CodeBase::SetStartChar(char start) {
  return bc_writer_->SetStartChar(start);
}

bool CBC_CodeBase::SetEndChar(char end) {
  return bc_writer_->SetEndChar(end);
}

bool CBC_CodeBase::SetErrorCorrectionLevel(int32_t level) {
  return bc_writer_->SetErrorCorrectionLevel(level);
}

bool CBC_CodeBase::SetModuleHeight(int32_t moduleHeight) {
  return bc_writer_->SetModuleHeight(moduleHeight);
}

bool CBC_CodeBase::SetModuleWidth(int32_t moduleWidth) {
  return bc_writer_->SetModuleWidth(moduleWidth);
}

void CBC_CodeBase::SetHeight(int32_t height) {
  return bc_writer_->SetHeight(height);
}

void CBC_CodeBase::SetWidth(int32_t width) {
  return bc_writer_->SetWidth(width);
}
