// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BASE256ENCODER_H_
#define _BC_BASE256ENCODER_H_
class CBC_Encoder;
class CBC_Base256Encoder;
class CBC_Base256Encoder : public CBC_Encoder {
 public:
  CBC_Base256Encoder();
  virtual ~CBC_Base256Encoder();
  int32_t getEncodingMode();
  void Encode(CBC_EncoderContext& context, int32_t& e);

 private:
  static FX_WCHAR randomize255State(FX_WCHAR ch, int32_t codewordPosition);
};
#endif
