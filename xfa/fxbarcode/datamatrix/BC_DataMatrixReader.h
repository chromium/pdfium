// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXREADER_H_
#define XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXREADER_H_

#include "xfa/fxbarcode/BC_Reader.h"

class CBC_BinaryBitmap;
class CBC_DataMatrixDecoder;

class CBC_DataMatrixReader : public CBC_Reader {
 public:
  CBC_DataMatrixReader();
  virtual ~CBC_DataMatrixReader();
  CFX_ByteString Decode(CBC_BinaryBitmap* image, int32_t& e);
  CFX_ByteString Decode(CBC_BinaryBitmap* image, int hints, int32_t& e);

  virtual void Init();

 private:
  CBC_DataMatrixDecoder* m_decoder;
};

#endif  // XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXREADER_H_
