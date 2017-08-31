// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <vector>

#include "core/fxcrt/fx_string.h"

namespace {

class CFX_UTF8Encoder {
 public:
  CFX_UTF8Encoder() {}
  ~CFX_UTF8Encoder() {}

  void Input(wchar_t unicodeAsWchar) {
    uint32_t unicode = static_cast<uint32_t>(unicodeAsWchar);
    if (unicode < 0x80) {
      m_Buffer.push_back(unicode);
    } else {
      if (unicode >= 0x80000000)
        return;

      int nbytes = 0;
      if (unicode < 0x800)
        nbytes = 2;
      else if (unicode < 0x10000)
        nbytes = 3;
      else if (unicode < 0x200000)
        nbytes = 4;
      else if (unicode < 0x4000000)
        nbytes = 5;
      else
        nbytes = 6;

      static uint8_t prefix[] = {0xc0, 0xe0, 0xf0, 0xf8, 0xfc};
      int order = 1 << ((nbytes - 1) * 6);
      int code = unicodeAsWchar;
      m_Buffer.push_back(prefix[nbytes - 2] | (code / order));
      for (int i = 0; i < nbytes - 1; i++) {
        code = code % order;
        order >>= 6;
        m_Buffer.push_back(0x80 | (code / order));
      }
    }
  }

  // The data returned by GetResult() is invalidated when this is modified by
  // appending any data.
  CFX_ByteStringC GetResult() const {
    return CFX_ByteStringC(m_Buffer.data(), m_Buffer.size());
  }

 private:
  std::vector<uint8_t> m_Buffer;
};

}  // namespace

CFX_ByteString FX_UTF8Encode(const CFX_WideStringC& wsStr) {
  FX_STRSIZE len = wsStr.GetLength();
  const wchar_t* pStr = wsStr.unterminated_c_str();
  CFX_UTF8Encoder encoder;
  while (len-- > 0)
    encoder.Input(*pStr++);

  return CFX_ByteString(encoder.GetResult());
}
