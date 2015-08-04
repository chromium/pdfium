// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_DATAMATRIXBITMATRIXPARSER_H_
#define _BC_DATAMATRIXBITMATRIXPARSER_H_
class CBC_CommonBitMatrix;
class CBC_DataMatrixVersion;
class CBC_DataMatrixBitMatrixParser {
 public:
  CBC_DataMatrixBitMatrixParser();
  virtual ~CBC_DataMatrixBitMatrixParser();
  CBC_DataMatrixVersion* GetVersion();
  CFX_ByteArray* ReadCodewords(int32_t& e);
  FX_BOOL ReadModule(int32_t row,
                     int32_t column,
                     int32_t numRows,
                     int32_t numColumns);
  int32_t ReadUtah(int32_t row,
                   int32_t column,
                   int32_t numRows,
                   int32_t numColumns);
  int32_t ReadCorner1(int32_t numRows, int32_t numColumns);
  int32_t ReadCorner2(int32_t numRows, int32_t numColumns);
  int32_t ReadCorner3(int32_t numRows, int32_t numColumns);
  int32_t ReadCorner4(int32_t numRows, int32_t numColumns);
  CBC_CommonBitMatrix* ExtractDataRegion(CBC_CommonBitMatrix* bitMatrix,
                                         int32_t& e);
  virtual void Init(CBC_CommonBitMatrix* bitMatrix, int32_t& e);

 private:
  static CBC_DataMatrixVersion* ReadVersion(CBC_CommonBitMatrix* bitMatrix,
                                            int32_t& e);
  CBC_CommonBitMatrix* m_mappingBitMatrix;
  CBC_CommonBitMatrix* m_readMappingMatrix;
  CBC_DataMatrixVersion* m_version;
};
#endif
