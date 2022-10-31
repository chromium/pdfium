// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_QRCODE_BC_QRCODERBITVECTOR_H_
#define FXBARCODE_QRCODE_BC_QRCODERBITVECTOR_H_

#include <stddef.h>
#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "third_party/base/span.h"

class CBC_QRCoderBitVector {
 public:
  CBC_QRCoderBitVector();
  ~CBC_QRCoderBitVector();

  pdfium::span<const uint8_t> GetArray() const;
  int32_t At(size_t index) const;
  size_t Size() const;
  size_t sizeInBytes() const;

  void AppendBit(int32_t bit);
  void AppendBits(int32_t value, int32_t numBits);
  void AppendBitVector(const CBC_QRCoderBitVector* bits);
  bool XOR(const CBC_QRCoderBitVector* other);

 private:
  void AppendByte(int8_t value);

  size_t m_sizeInBits = 0;
  DataVector<uint8_t> m_array;
};

#endif  // FXBARCODE_QRCODE_BC_QRCODERBITVECTOR_H_
