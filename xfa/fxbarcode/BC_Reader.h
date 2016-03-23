// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_BC_READER_H_
#define XFA_FXBARCODE_BC_READER_H_

#include "core/fxcrt/include/fx_string.h"

class CBC_BinaryBitmap;

class CBC_Reader {
 public:
  CBC_Reader();
  virtual ~CBC_Reader();
  virtual CFX_ByteString Decode(CBC_BinaryBitmap* image, int32_t& e) = 0;
  virtual CFX_ByteString Decode(CBC_BinaryBitmap* image,
                                int32_t hints,
                                int32_t& e) = 0;
};

#endif  // XFA_FXBARCODE_BC_READER_H_
