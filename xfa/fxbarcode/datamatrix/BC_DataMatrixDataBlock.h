// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXDATABLOCK_H_
#define XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXDATABLOCK_H_

#include "core/fxcrt/include/fx_basic.h"

class CBC_DataMatrixVersion;

class CBC_DataMatrixDataBlock {
 public:
  virtual ~CBC_DataMatrixDataBlock();

  int32_t GetNumDataCodewords();
  CFX_ByteArray* GetCodewords();

  static CFX_PtrArray* GetDataBlocks(CFX_ByteArray* rawCodewords,
                                     CBC_DataMatrixVersion* version,
                                     int32_t& e);

 private:
  int32_t m_numDataCodewords;
  CFX_ByteArray m_codewords;

  CBC_DataMatrixDataBlock(int32_t numDataCodewords, CFX_ByteArray* codewords);
};

#endif  // XFA_FXBARCODE_DATAMATRIX_BC_DATAMATRIXDATABLOCK_H_
