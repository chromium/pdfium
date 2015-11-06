// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FXBARCODE_PDF417_BC_PDF417DECODEDBITSTREAMPARSER_H_
#define XFA_SRC_FXBARCODE_PDF417_BC_PDF417DECODEDBITSTREAMPARSER_H_

#include "core/include/fxcrt/fx_basic.h"
#include "core/include/fxcrt/fx_string.h"

class CBC_CommonDecoderResult;
class CBC_PDF417ResultMetadata;

class CBC_DecodedBitStreamPaser {
 public:
  CBC_DecodedBitStreamPaser();
  virtual ~CBC_DecodedBitStreamPaser();
  static void Initialize();
  static void Finalize();
  static CBC_CommonDecoderResult* decode(CFX_Int32Array& codewords,
                                         CFX_ByteString ecLevel,
                                         int32_t& e);

 private:
  enum Mode { ALPHA, LOWER, MIXED, PUNCT, ALPHA_SHIFT, PUNCT_SHIFT };
  static int32_t MAX_NUMERIC_CODEWORDS;
  static int32_t PL;
  static int32_t LL;
  static int32_t AS;
  static int32_t ML;
  static int32_t AL;
  static int32_t PS;
  static int32_t PAL;
  static FX_CHAR PUNCT_CHARS[29];
  static FX_CHAR MIXED_CHARS[30];
  static int32_t NUMBER_OF_SEQUENCE_CODEWORDS;
  static int32_t decodeMacroBlock(CFX_Int32Array& codewords,
                                  int32_t codeIndex,
                                  CBC_PDF417ResultMetadata* resultMetadata,
                                  int32_t& e);
  static int32_t textCompaction(CFX_Int32Array& codewords,
                                int32_t codeIndex,
                                CFX_ByteString& result);
  static void decodeTextCompaction(CFX_Int32Array& textCompactionData,
                                   CFX_Int32Array& byteCompactionData,
                                   int32_t length,
                                   CFX_ByteString& result);
  static int32_t byteCompaction(int32_t mode,
                                CFX_Int32Array& codewords,
                                int32_t codeIndex,
                                CFX_ByteString& result);
  static int32_t numericCompaction(CFX_Int32Array& codewords,
                                   int32_t codeIndex,
                                   CFX_ByteString& result,
                                   int32_t& e);
  static CFX_ByteString decodeBase900toBase10(CFX_Int32Array& codewords,
                                              int32_t count,
                                              int32_t& e);
};
#endif  // XFA_SRC_FXBARCODE_PDF417_BC_PDF417DECODEDBITSTREAMPARSER_H_
