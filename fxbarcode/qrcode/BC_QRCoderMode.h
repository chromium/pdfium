// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_QRCODE_BC_QRCODERMODE_H_
#define FXBARCODE_QRCODE_BC_QRCODERMODE_H_

#include <stdint.h>

#include <vector>

class CBC_QRCoderMode final {
 public:
  ~CBC_QRCoderMode();

  static void Initialize();
  static void Finalize();

  int32_t GetCharacterCountBits(int32_t number) const;
  int32_t GetBits() const;

  static CBC_QRCoderMode* sBYTE;
  static CBC_QRCoderMode* sNUMERIC;
  static CBC_QRCoderMode* sALPHANUMERIC;

 private:
  CBC_QRCoderMode(std::vector<int32_t> charCountBits, int32_t bits);

  std::vector<int32_t> character_count_bits_for_versions_;
  const int32_t bits_;
};

#endif  // FXBARCODE_QRCODE_BC_QRCODERMODE_H_
