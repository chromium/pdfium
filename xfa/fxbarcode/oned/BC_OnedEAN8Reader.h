// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_ONED_BC_ONEDEAN8READER_H_
#define XFA_FXBARCODE_ONED_BC_ONEDEAN8READER_H_

#include "xfa/fxbarcode/oned/BC_OneDimReader.h"

class CBC_CommonBitArray;

class CBC_OnedEAN8Reader : public CBC_OneDimReader {
 public:
  CBC_OnedEAN8Reader();
  virtual ~CBC_OnedEAN8Reader();

 protected:
  int32_t DecodeMiddle(CBC_CommonBitArray*,
                       CFX_Int32Array* startRange,
                       CFX_ByteString& result,
                       int32_t& e);
};

#endif  // XFA_FXBARCODE_ONED_BC_ONEDEAN8READER_H_
