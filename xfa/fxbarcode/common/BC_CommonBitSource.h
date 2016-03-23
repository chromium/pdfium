// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_COMMON_BC_COMMONBITSOURCE_H_
#define XFA_FXBARCODE_COMMON_BC_COMMONBITSOURCE_H_

#include "core/fxcrt/include/fx_basic.h"
#include "xfa/fxbarcode/utils.h"

class CBC_CommonBitSource {
 public:
  CBC_CommonBitSource(CFX_ByteArray* bytes);
  virtual ~CBC_CommonBitSource();
  int32_t ReadBits(int32_t numBits, int32_t& e);
  int32_t Available();
  int32_t getByteOffset();

 private:
  CFX_ByteArray m_bytes;
  int32_t m_byteOffset;
  int32_t m_bitOffset;
};

#endif  // XFA_FXBARCODE_COMMON_BC_COMMONBITSOURCE_H_
