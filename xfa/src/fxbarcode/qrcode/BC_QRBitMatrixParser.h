// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRABITMATRIXPARSER_H_
#define _BC_QRABITMATRIXPARSER_H_
class CBC_CommonBitMatrix;
class CBC_QRCoderVersion;
class CBC_QRCoderFormatInformation;
class CBC_QRDataMask;
class CBC_QRBitMatrixParser {
 private:
  CBC_CommonBitMatrix* m_bitMatrix;
  CBC_CommonBitMatrix* m_tempBitMatrix;
  CBC_QRCoderVersion* m_version;
  CBC_QRCoderFormatInformation* m_parsedFormatInfo;
  int32_t m_dimension;

 public:
  CBC_QRBitMatrixParser();
  virtual ~CBC_QRBitMatrixParser();
  CBC_QRCoderFormatInformation* ReadFormatInformation(int32_t& e);
  CBC_QRCoderVersion* ReadVersion(int32_t& e);
  int32_t CopyBit(int32_t i, int32_t j, int32_t versionBits);
  CFX_ByteArray* ReadCodewords(int32_t& e);
  virtual void Init(CBC_CommonBitMatrix* bitMatrix, int32_t& e);
};
#endif
