// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_widetextbuf.h"

#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"

size_t CFX_WideTextBuf::GetLength() const {
  return m_DataSize / sizeof(wchar_t);
}

pdfium::span<wchar_t> CFX_WideTextBuf::GetWideSpan() {
  return pdfium::make_span(reinterpret_cast<wchar_t*>(m_pBuffer.get()),
                           GetLength());
}

pdfium::span<const wchar_t> CFX_WideTextBuf::GetWideSpan() const {
  return pdfium::make_span(reinterpret_cast<wchar_t*>(m_pBuffer.get()),
                           GetLength());
}

WideStringView CFX_WideTextBuf::AsStringView() const {
  return WideStringView(GetWideSpan());
}

WideString CFX_WideTextBuf::MakeString() const {
  return WideString(AsStringView());
}

void CFX_WideTextBuf::AppendChar(wchar_t ch) {
  pdfium::span<wchar_t> new_span = ExpandWideBuf(1);
  new_span[0] = ch;
}

void CFX_WideTextBuf::Delete(int start_index, int count) {
  CFX_BinaryBuf::Delete(start_index * sizeof(wchar_t), count * sizeof(wchar_t));
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(ByteStringView ascii) {
  pdfium::span<wchar_t> new_span = ExpandWideBuf(ascii.GetLength());
  for (size_t i = 0; i < ascii.GetLength(); ++i)
    new_span[i] = ascii[i];
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(WideStringView str) {
  AppendBlock(str.unterminated_c_str(), str.GetLength() * sizeof(wchar_t));
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const WideString& str) {
  AppendBlock(str.c_str(), str.GetLength() * sizeof(wchar_t));
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(int i) {
  char buf[32];
  FXSYS_itoa(i, buf, 10);
  size_t len = strlen(buf);
  pdfium::span<wchar_t> new_span = ExpandWideBuf(len);
  for (size_t j = 0; j < len; j++)
    new_span[j] = buf[j];
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(double f) {
  char buf[32];
  size_t len = FloatToString((float)f, buf);
  pdfium::span<wchar_t> new_span = ExpandWideBuf(len);
  for (size_t i = 0; i < len; i++)
    new_span[i] = buf[i];
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const wchar_t* lpsz) {
  AppendBlock(lpsz, wcslen(lpsz) * sizeof(wchar_t));
  return *this;
}

CFX_WideTextBuf& CFX_WideTextBuf::operator<<(const CFX_WideTextBuf& buf) {
  AppendBlock(buf.m_pBuffer.get(), buf.m_DataSize);
  return *this;
}

pdfium::span<wchar_t> CFX_WideTextBuf::ExpandWideBuf(size_t char_count) {
  size_t original_count = GetLength();
  FX_SAFE_SIZE_T safe_bytes = char_count;
  safe_bytes *= sizeof(wchar_t);
  size_t bytes = safe_bytes.ValueOrDie();
  ExpandBuf(bytes);
  m_DataSize += bytes;
  return GetWideSpan().subspan(original_count);
}
