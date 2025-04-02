// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2006-2007 Jeremias Maerki.
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

#include "fxbarcode/datamatrix/BC_EncoderContext.h"

#include <utility>

#include "core/fxcrt/fx_string.h"
#include "fxbarcode/datamatrix/BC_Encoder.h"
#include "fxbarcode/datamatrix/BC_SymbolInfo.h"

CBC_EncoderContext::CBC_EncoderContext(const WideString& msg) {
  ByteString dststr = msg.ToUTF8();
  size_t c = dststr.GetLength();
  WideString sb;
  sb.Reserve(c);
  for (size_t i = 0; i < c; i++) {
    wchar_t ch = static_cast<wchar_t>(dststr[i] & 0xff);
    if (ch == '?' && dststr[i] != '?') {
      has_characters_outside_iso88591_encoding_ = true;
    }
    sb += ch;
  }
  msg_ = std::move(sb);
  codewords_.Reserve(msg_.GetLength());
}

CBC_EncoderContext::~CBC_EncoderContext() = default;

void CBC_EncoderContext::setSkipAtEnd(int32_t count) {
  skip_at_end_ = count;
}
wchar_t CBC_EncoderContext::getCurrentChar() {
  return msg_[pos_];
}
wchar_t CBC_EncoderContext::getCurrent() {
  return msg_[pos_];
}

void CBC_EncoderContext::writeCodewords(const WideString& codewords) {
  codewords_ += codewords;
}

void CBC_EncoderContext::writeCodeword(wchar_t codeword) {
  codewords_ += codeword;
}

size_t CBC_EncoderContext::getCodewordCount() {
  return codewords_.GetLength();
}

void CBC_EncoderContext::SignalEncoderChange(
    CBC_HighLevelEncoder::Encoding encoding) {
  new_encoding_ = encoding;
}

void CBC_EncoderContext::ResetEncoderSignal() {
  new_encoding_ = CBC_HighLevelEncoder::Encoding::UNKNOWN;
}

bool CBC_EncoderContext::hasMoreCharacters() {
  return pos_ < getTotalMessageCharCount();
}

size_t CBC_EncoderContext::getRemainingCharacters() {
  return getTotalMessageCharCount() - pos_;
}

bool CBC_EncoderContext::UpdateSymbolInfo() {
  return UpdateSymbolInfo(getCodewordCount());
}

bool CBC_EncoderContext::UpdateSymbolInfo(size_t len) {
  if (!symbol_info_ || len > symbol_info_->data_capacity()) {
    symbol_info_ = CBC_SymbolInfo::Lookup(len, allow_rectangular_);
    if (!symbol_info_) {
      return false;
    }
  }
  return true;
}

void CBC_EncoderContext::resetSymbolInfo() {
  allow_rectangular_ = true;
}

size_t CBC_EncoderContext::getTotalMessageCharCount() {
  return msg_.GetLength() - skip_at_end_;
}
