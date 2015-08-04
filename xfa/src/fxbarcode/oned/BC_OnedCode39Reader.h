// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ONEDCODA39READER_H_
#define _BC_ONEDCODA39READER_H_
class CBC_OneDReader;
class CBC_CommonBitArray;
class CBC_OnedCoda39Reader;
class CBC_OnedCode39Reader : public CBC_OneDReader {
 public:
  static const FX_CHAR* ALPHABET_STRING;
  static const FX_CHAR* CHECKSUM_STRING;
  const static int32_t CHARACTER_ENCODINGS[44];
  const static int32_t ASTERISK_ENCODING;
  CBC_OnedCode39Reader();
  CBC_OnedCode39Reader(FX_BOOL usingCheckDigit);
  CBC_OnedCode39Reader(FX_BOOL usingCheckDigit, FX_BOOL extendedMode);
  virtual ~CBC_OnedCode39Reader();
  CFX_ByteString DecodeRow(int32_t rowNumber,
                           CBC_CommonBitArray* row,
                           int32_t hints,
                           int32_t& e);

 private:
  FX_BOOL m_usingCheckDigit;
  FX_BOOL m_extendedMode;
  CFX_Int32Array* FindAsteriskPattern(CBC_CommonBitArray* row, int32_t& e);
  int32_t ToNarrowWidePattern(CFX_Int32Array* counters);
  FX_CHAR PatternToChar(int32_t pattern, int32_t& e);
  CFX_ByteString DecodeExtended(CFX_ByteString& encoded, int32_t& e);
};
#endif
