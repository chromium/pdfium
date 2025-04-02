// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_DATAMATRIX_BC_ENCODERCONTEXT_H_
#define FXBARCODE_DATAMATRIX_BC_ENCODERCONTEXT_H_

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "fxbarcode/datamatrix/BC_HighLevelEncoder.h"

class CBC_SymbolInfo;

class CBC_EncoderContext {
 public:
  explicit CBC_EncoderContext(const WideString& msg);
  ~CBC_EncoderContext();

  void setSkipAtEnd(int32_t count);
  wchar_t getCurrentChar();
  wchar_t getCurrent();
  void writeCodewords(const WideString& codewords);
  void writeCodeword(wchar_t codeword);
  size_t getCodewordCount();
  void SignalEncoderChange(CBC_HighLevelEncoder::Encoding encoding);
  void ResetEncoderSignal();
  bool hasMoreCharacters();
  size_t getRemainingCharacters();
  bool UpdateSymbolInfo();
  bool UpdateSymbolInfo(size_t len);
  void resetSymbolInfo();

  bool HasCharactersOutsideISO88591Encoding() const {
    return has_characters_outside_iso88591_encoding_;
  }

  WideString msg_;
  WideString codewords_;
  size_t pos_ = 0;
  CBC_HighLevelEncoder::Encoding new_encoding_ =
      CBC_HighLevelEncoder::Encoding::UNKNOWN;
  UnownedPtr<const CBC_SymbolInfo> symbol_info_;

 private:
  size_t getTotalMessageCharCount();

  bool allow_rectangular_ = false;  // Force square when false.
  bool has_characters_outside_iso88591_encoding_ = false;
  size_t skip_at_end_ = 0;
};

#endif  // FXBARCODE_DATAMATRIX_BC_ENCODERCONTEXT_H_
