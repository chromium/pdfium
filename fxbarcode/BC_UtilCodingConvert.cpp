// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxbarcode/BC_UtilCodingConvert.h"

CBC_UtilCodingConvert::CBC_UtilCodingConvert() {}

CBC_UtilCodingConvert::~CBC_UtilCodingConvert() {}

void CBC_UtilCodingConvert::UnicodeToLocale(const CFX_WideString& src,
                                            CFX_ByteString& dst) {
  dst = CFX_ByteString::FromUnicode(src);
}

void CBC_UtilCodingConvert::LocaleToUtf8(const CFX_ByteString& src,
                                         CFX_ByteString& dst) {
  CFX_WideString unicode = CFX_WideString::FromLocal(src.AsStringC());
  dst = unicode.UTF8Encode();
}

void CBC_UtilCodingConvert::LocaleToUtf8(const CFX_ByteString& src,
                                         std::vector<uint8_t>& dst) {
  CFX_WideString unicode = CFX_WideString::FromLocal(src.AsStringC());
  CFX_ByteString utf8 = unicode.UTF8Encode();
  dst = std::vector<uint8_t>(utf8.begin(), utf8.end());
}

void CBC_UtilCodingConvert::Utf8ToLocale(const std::vector<uint8_t>& src,
                                         CFX_ByteString& dst) {
  CFX_ByteString utf8;
  for (uint8_t value : src)
    utf8 += value;

  CFX_WideString unicode = CFX_WideString::FromUTF8(utf8.AsStringC());
  dst = CFX_ByteString::FromUnicode(unicode);
}

void CBC_UtilCodingConvert::Utf8ToLocale(const uint8_t* src,
                                         int32_t count,
                                         CFX_ByteString& dst) {
  CFX_WideString unicode =
      CFX_WideString::FromUTF8(CFX_ByteStringC(src, count));
  dst = CFX_ByteString::FromUnicode(unicode);
}

void CBC_UtilCodingConvert::UnicodeToUTF8(const CFX_WideString& src,
                                          CFX_ByteString& dst) {
  dst = src.UTF8Encode();
}
