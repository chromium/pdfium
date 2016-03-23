// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_ONED_BC_ONEDEAN13READER_H_
#define XFA_FXBARCODE_ONED_BC_ONEDEAN13READER_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "xfa/fxbarcode/oned/BC_OneDimReader.h"

class CBC_CommonBitArray;
class CBC_OnedUPCAReader;

class CBC_OnedEAN13Reader : public CBC_OneDimReader {
 public:
  static const int32_t FIRST_DIGIT_ENCODINGS[10];

  CBC_OnedEAN13Reader();
  virtual ~CBC_OnedEAN13Reader();

 private:
  void DetermineFirstDigit(CFX_ByteString& result,
                           int32_t lgPatternFound,
                           int32_t& e);

 protected:
  friend class CBC_OnedUPCAReader;

  int32_t DecodeMiddle(CBC_CommonBitArray* row,
                       CFX_Int32Array* startRange,
                       CFX_ByteString& resultString,
                       int32_t& e);
};

#endif  // XFA_FXBARCODE_ONED_BC_ONEDEAN13READER_H_
