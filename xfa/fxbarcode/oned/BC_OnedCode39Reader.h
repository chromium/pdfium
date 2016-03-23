// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_ONED_BC_ONEDCODE39READER_H_
#define XFA_FXBARCODE_ONED_BC_ONEDCODE39READER_H_

#include "core/fxcrt/include/fx_system.h"
#include "xfa/fxbarcode/oned/BC_OneDReader.h"

class CBC_CommonBitArray;

class CBC_OnedCode39Reader : public CBC_OneDReader {
 public:
  static const FX_CHAR* ALPHABET_STRING;
  static const FX_CHAR* CHECKSUM_STRING;
  static const int32_t CHARACTER_ENCODINGS[44];
  static const int32_t ASTERISK_ENCODING = 0x094;

  CBC_OnedCode39Reader();
  explicit CBC_OnedCode39Reader(FX_BOOL usingCheckDigit);
  CBC_OnedCode39Reader(FX_BOOL usingCheckDigit, FX_BOOL extendedMode);
  virtual ~CBC_OnedCode39Reader();

  CFX_ByteString DecodeRow(int32_t rowNumber,
                           CBC_CommonBitArray* row,
                           int32_t hints,
                           int32_t& e);

 private:
  CFX_Int32Array* FindAsteriskPattern(CBC_CommonBitArray* row, int32_t& e);
  int32_t ToNarrowWidePattern(CFX_Int32Array* counters);
  FX_CHAR PatternToChar(int32_t pattern, int32_t& e);
  CFX_ByteString DecodeExtended(CFX_ByteString& encoded, int32_t& e);

  FX_BOOL m_usingCheckDigit;
  FX_BOOL m_extendedMode;
};

#endif  // XFA_FXBARCODE_ONED_BC_ONEDCODE39READER_H_
