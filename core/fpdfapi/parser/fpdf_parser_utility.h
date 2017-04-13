// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_FPDF_PARSER_UTILITY_H_
#define CORE_FPDFAPI_PARSER_FPDF_PARSER_UTILITY_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_basic.h"

class IFX_SeekableReadStream;
class CPDF_Dictionary;
class CPDF_Object;

// Use the accessors below instead of directly accessing PDF_CharType.
extern const char PDF_CharType[256];

inline bool PDFCharIsWhitespace(uint8_t c) {
  return PDF_CharType[c] == 'W';
}
inline bool PDFCharIsNumeric(uint8_t c) {
  return PDF_CharType[c] == 'N';
}
inline bool PDFCharIsDelimiter(uint8_t c) {
  return PDF_CharType[c] == 'D';
}
inline bool PDFCharIsOther(uint8_t c) {
  return PDF_CharType[c] == 'R';
}

inline bool PDFCharIsLineEnding(uint8_t c) {
  return c == '\r' || c == '\n';
}

int32_t GetHeaderOffset(const CFX_RetainPtr<IFX_SeekableReadStream>& pFile);
int32_t GetDirectInteger(CPDF_Dictionary* pDict, const CFX_ByteString& key);

CFX_ByteTextBuf& operator<<(CFX_ByteTextBuf& buf, const CPDF_Object* pObj);

#endif  // CORE_FPDFAPI_PARSER_FPDF_PARSER_UTILITY_H_
