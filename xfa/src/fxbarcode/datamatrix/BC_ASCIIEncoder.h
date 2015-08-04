// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_ASCIIENCODER_H_
#define _BC_ASCIIENCODER_H_
class CBC_Encoder;
class CBC_EncoderContext;
class CBC_ASCIIEncoder;
class CBC_ASCIIEncoder : public CBC_Encoder {
 public:
  CBC_ASCIIEncoder();
  virtual ~CBC_ASCIIEncoder();
  int32_t getEncodingMode();
  void Encode(CBC_EncoderContext& context, int32_t& e);

 private:
  static FX_WCHAR encodeASCIIDigits(FX_WCHAR digit1,
                                    FX_WCHAR digit2,
                                    int32_t& e);
};
#endif
