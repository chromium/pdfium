// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DATAMATRIXSYMBOLINFO144_H_
#define _BC_DATAMATRIXSYMBOLINFO144_H_
class CBC_SymbolInfo;
class CBC_DataMatrixSymbolInfo144;
class CBC_DataMatrixSymbolInfo144 : public CBC_SymbolInfo {
 public:
  CBC_DataMatrixSymbolInfo144();
  virtual ~CBC_DataMatrixSymbolInfo144();
  int32_t getInterleavedBlockCount();
  int32_t getDataLengthForInterleavedBlock(int32_t index);
};
#endif
