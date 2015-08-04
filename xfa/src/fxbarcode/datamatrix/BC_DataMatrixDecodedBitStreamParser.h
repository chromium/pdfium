// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DATAMATRIXDECODEDBITSTREAMPARSER_H_
#define _BC_DATAMATRIXDECODEDBITSTREAMPARSER_H_
class CBC_CommonDecoderResult;
class CBC_CommonBitSource;
class CBC_DataMatrixDecodedBitStreamParser {
 public:
  CBC_DataMatrixDecodedBitStreamParser();
  virtual ~CBC_DataMatrixDecodedBitStreamParser();
  static CBC_CommonDecoderResult* Decode(CFX_ByteArray& bytes, int32_t& e);

 private:
  static int32_t DecodeAsciiSegment(CBC_CommonBitSource* bits,
                                    CFX_ByteString& result,
                                    CFX_ByteString& resultTrailer,
                                    int32_t& e);
  static void DecodeC40Segment(CBC_CommonBitSource* bits,
                               CFX_ByteString& result,
                               int32_t& e);
  static void DecodeTextSegment(CBC_CommonBitSource* bits,
                                CFX_ByteString& result,
                                int32_t& e);
  static void DecodeAnsiX12Segment(CBC_CommonBitSource* bits,
                                   CFX_ByteString& result,
                                   int32_t& e);
  static void ParseTwoBytes(int32_t firstByte,
                            int32_t secondByte,
                            CFX_Int32Array& result);
  static void DecodeEdifactSegment(CBC_CommonBitSource* bits,
                                   CFX_ByteString& result,
                                   int32_t& e);
  static void DecodeBase256Segment(CBC_CommonBitSource* bits,
                                   CFX_ByteString& result,
                                   CFX_Int32Array& byteSegments,
                                   int32_t& e);
  static uint8_t Unrandomize255State(int32_t randomizedBase256Codeword,
                                     int32_t base256CodewordPosition);

  const static FX_CHAR C40_BASIC_SET_CHARS[];
  const static FX_CHAR C40_SHIFT2_SET_CHARS[];

  const static FX_CHAR TEXT_BASIC_SET_CHARS[];
  const static FX_CHAR TEXT_SHIFT3_SET_CHARS[];
  const static int32_t PAD_ENCODE;
  const static int32_t ASCII_ENCODE;
  const static int32_t C40_ENCODE;
  const static int32_t TEXT_ENCODE;
  const static int32_t ANSIX12_ENCODE;
  const static int32_t EDIFACT_ENCODE;
  const static int32_t BASE256_ENCODE;
};
#endif
