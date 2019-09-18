// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_JBIG2_JBIG2_BITSTREAM_H_
#define CORE_FXCODEC_JBIG2_JBIG2_BITSTREAM_H_

#include "core/fxcrt/retain_ptr.h"
#include "third_party/base/span.h"

class CJBig2_BitStream {
 public:
  CJBig2_BitStream(pdfium::span<const uint8_t> pSrcStream, uint32_t dwObjNum);
  CJBig2_BitStream(const CJBig2_BitStream&) = delete;
  CJBig2_BitStream& operator=(const CJBig2_BitStream&) = delete;
  ~CJBig2_BitStream();

  // TODO(thestig): readFoo() should return bool.
  int32_t readNBits(uint32_t dwBits, uint32_t* dwResult);
  int32_t readNBits(uint32_t dwBits, int32_t* nResult);
  int32_t read1Bit(uint32_t* dwResult);
  int32_t read1Bit(bool* bResult);
  int32_t read1Byte(uint8_t* cResult);
  int32_t readInteger(uint32_t* dwResult);
  int32_t readShortInteger(uint16_t* wResult);
  void alignByte();
  uint8_t getCurByte() const;
  void incByteIdx();
  uint8_t getCurByte_arith() const;
  uint8_t getNextByte_arith() const;
  uint32_t getOffset() const;
  void setOffset(uint32_t dwOffset);
  uint32_t getBitPos() const;
  void setBitPos(uint32_t dwBitPos);
  const uint8_t* getBuf() const;
  uint32_t getLength() const { return m_Span.size(); }
  const uint8_t* getPointer() const;
  void offset(uint32_t dwOffset);
  uint32_t getByteLeft() const;
  uint32_t getObjNum() const;
  bool IsInBounds() const;

 private:
  void AdvanceBit();
  uint32_t LengthInBits() const;

  const pdfium::span<const uint8_t> m_Span;
  uint32_t m_dwByteIdx = 0;
  uint32_t m_dwBitIdx = 0;
  const uint32_t m_dwObjNum;
};

#endif  // CORE_FXCODEC_JBIG2_JBIG2_BITSTREAM_H_
