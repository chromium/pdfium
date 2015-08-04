// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_EDIFACTENCODER_H_
#define _BC_EDIFACTENCODER_H_
class CBC_EncoderContext;
class CBC_EdifactEncoder;
class CBC_EdifactEncoder : public CBC_Encoder {
 public:
  CBC_EdifactEncoder();
  virtual ~CBC_EdifactEncoder();
  int32_t getEncodingMode();
  void Encode(CBC_EncoderContext& context, int32_t& e);

 private:
  static void handleEOD(CBC_EncoderContext& context,
                        CFX_WideString buffer,
                        int32_t& e);
  static void encodeChar(FX_WCHAR c, CFX_WideString& sb, int32_t& e);
  static CFX_WideString encodeToCodewords(CFX_WideString sb,
                                          int32_t startPos,
                                          int32_t& e);
};
#endif
