// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fx_string_testhelpers.h"

#include <iomanip>
#include <ios>

#include "core/fxcrt/fx_string.h"

std::ostream& operator<<(std::ostream& os, const CFX_DateTime& dt) {
  os << dt.GetYear() << "-" << std::to_string(dt.GetMonth()) << "-"
     << std::to_string(dt.GetDay()) << " " << std::to_string(dt.GetHour())
     << ":" << std::to_string(dt.GetMinute()) << ":"
     << std::to_string(dt.GetSecond()) << "."
     << std::to_string(dt.GetMillisecond());
  return os;
}

std::vector<std::string> StringSplit(const std::string& str, char delimiter) {
  std::vector<std::string> result;
  size_t pos = 0;
  while (1) {
    size_t found = str.find(delimiter, pos);
    if (found == std::string::npos)
      break;

    result.push_back(str.substr(pos, found - pos));
    pos = found + 1;
  }
  result.push_back(str.substr(pos));
  return result;
}

std::string GetPlatformString(FPDF_WIDESTRING wstr) {
  WideString wide_string =
      WideString::FromUTF16LE(wstr, WideString::WStringLength(wstr));
  return std::string(wide_string.ToUTF8().c_str());
}

std::wstring GetPlatformWString(FPDF_WIDESTRING wstr) {
  if (!wstr)
    return nullptr;

  size_t characters = 0;
  while (wstr[characters])
    ++characters;

  std::wstring platform_string(characters, L'\0');
  for (size_t i = 0; i < characters + 1; ++i) {
    const unsigned char* ptr = reinterpret_cast<const unsigned char*>(&wstr[i]);
    platform_string[i] = ptr[0] + 256 * ptr[1];
  }
  return platform_string;
}

std::unique_ptr<unsigned short, pdfium::FreeDeleter> GetFPDFWideString(
    const std::wstring& wstr) {
  size_t length = sizeof(uint16_t) * (wstr.length() + 1);
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> result(
      static_cast<unsigned short*>(malloc(length)));
  char* ptr = reinterpret_cast<char*>(result.get());
  size_t i = 0;
  for (wchar_t w : wstr) {
    ptr[i++] = w & 0xff;
    ptr[i++] = (w >> 8) & 0xff;
  }
  ptr[i++] = 0;
  ptr[i] = 0;
  return result;
}
