// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_TEXTENCODER_H_
#define _BC_TEXTENCODER_H_
class CBC_TextEncoder;
class CBC_TextEncoder : public CBC_C40Encoder {
 public:
  CBC_TextEncoder();
  virtual ~CBC_TextEncoder();
  int32_t getEncodingMode();
  int32_t encodeChar(FX_WCHAR c, CFX_WideString& sb, int32_t& e);
};
#endif
