// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_DATAMATRIX_BC_ERRORCORRECTION_H_
#define FXBARCODE_DATAMATRIX_BC_ERRORCORRECTION_H_

#include "core/fxcrt/widestring.h"
#include "third_party/base/optional.h"

class CBC_SymbolInfo;

class CBC_ErrorCorrection {
 public:
  CBC_ErrorCorrection() = delete;
  ~CBC_ErrorCorrection() = delete;

  static void Initialize();
  static void Finalize();
  static Optional<WideString> EncodeECC200(const WideString& codewords,
                                           const CBC_SymbolInfo* symbolInfo);

 private:
  static const int32_t MODULO_VALUE = 0x12D;

  static int32_t LOG[256];
  static int32_t ALOG[256];

  static Optional<WideString> CreateECCBlock(const WideString& codewords,
                                             int32_t numECWords);
};

#endif  // FXBARCODE_DATAMATRIX_BC_ERRORCORRECTION_H_
