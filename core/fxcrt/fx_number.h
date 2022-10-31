// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_NUMBER_H_
#define CORE_FXCRT_FX_NUMBER_H_

#include <stdint.h>

#include "core/fxcrt/bytestring.h"

class FX_Number {
 public:
  FX_Number();
  explicit FX_Number(uint32_t value) = delete;
  explicit FX_Number(int32_t value);
  explicit FX_Number(float value);
  explicit FX_Number(ByteStringView str);

  bool IsInteger() const { return m_bIsInteger; }
  bool IsSigned() const { return m_bIsSigned; }

  int32_t GetSigned() const;  // Underflow/Overflow possible.
  float GetFloat() const;

 private:
  bool m_bIsInteger;  // One of the two integers vs. float type.
  bool m_bIsSigned;   // Only valid if |m_bInteger|.
  union {
    uint32_t m_UnsignedValue;
    int32_t m_SignedValue;
    float m_FloatValue;
  };
};

#endif  // CORE_FXCRT_FX_NUMBER_H_
