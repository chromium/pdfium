// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXCODEC_JBIG2_JBIG2_BITSTREAM_H_
#define CORE_SRC_FXCODEC_JBIG2_JBIG2_BITSTREAM_H_

#include "core/include/fxcrt/fx_basic.h"

class CPDF_StreamAcc;

class CJBig2_BitStream {
 public:
  explicit CJBig2_BitStream(CPDF_StreamAcc* pSrcStream);
  ~CJBig2_BitStream();

  // TODO(thestig): readFoo() should return bool.
  int32_t readNBits(FX_DWORD nBits, FX_DWORD* dwResult);
  int32_t readNBits(FX_DWORD nBits, int32_t* nResult);
  int32_t read1Bit(FX_DWORD* dwResult);
  int32_t read1Bit(FX_BOOL* bResult);
  int32_t read1Byte(uint8_t* cResult);
  int32_t readInteger(FX_DWORD* dwResult);
  int32_t readShortInteger(FX_WORD* wResult);
  void alignByte();
  uint8_t getCurByte() const;
  void incByteIdx();
  uint8_t getCurByte_arith() const;
  uint8_t getNextByte_arith() const;
  FX_DWORD getOffset() const;
  void setOffset(FX_DWORD dwOffset);
  FX_DWORD getBitPos() const;
  void setBitPos(FX_DWORD dwBitPos);
  const uint8_t* getBuf() const;
  FX_DWORD getLength() const { return m_dwLength; }
  const uint8_t* getPointer() const;
  void offset(FX_DWORD dwOffset);
  FX_DWORD getByteLeft() const;
  FX_DWORD getObjNum() const;

 private:
  void AdvanceBit();
  bool IsInBound() const;
  FX_DWORD LengthInBits() const;

  const uint8_t* m_pBuf;
  FX_DWORD m_dwLength;
  FX_DWORD m_dwByteIdx;
  FX_DWORD m_dwBitIdx;
  const FX_DWORD m_dwObjNum;

  CJBig2_BitStream(const CJBig2_BitStream&) = delete;
  void operator=(const CJBig2_BitStream&) = delete;
};

#endif  // CORE_SRC_FXCODEC_JBIG2_JBIG2_BITSTREAM_H_
