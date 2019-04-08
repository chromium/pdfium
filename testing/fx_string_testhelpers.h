// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_FX_STRING_TESTHELPERS_H_
#define TESTING_FX_STRING_TESTHELPERS_H_

#include <memory>
#include <ostream>
#include <string>
#include <vector>

#include "core/fxcrt/cfx_datetime.h"
#include "public/fpdfview.h"
#include "testing/free_deleter.h"

// Output stream operator so GTEST macros work with CFX_DateTime objects.
std::ostream& operator<<(std::ostream& os, const CFX_DateTime& dt);

std::vector<std::string> StringSplit(const std::string& str, char delimiter);

// Converts a FPDF_WIDESTRING to a std::string.
// Deals with differences between UTF16LE and UTF8.
std::string GetPlatformString(FPDF_WIDESTRING wstr);

// Converts a FPDF_WIDESTRING to a std::wstring.
// Deals with differences between UTF16LE and wchar_t.
std::wstring GetPlatformWString(FPDF_WIDESTRING wstr);

using ScopedFPDFWideString = std::unique_ptr<FPDF_WCHAR, pdfium::FreeDeleter>;

// Returns a newly allocated FPDF_WIDESTRING.
// Deals with differences between UTF16LE and wchar_t.
ScopedFPDFWideString GetFPDFWideString(const std::wstring& wstr);

// Returns a FPDF_WCHAR vector of |length_bytes| bytes. |length_bytes| must be a
// multiple of sizeof(FPDF_WCHAR).
std::vector<FPDF_WCHAR> GetFPDFWideStringBuffer(size_t length_bytes);

#endif  // TESTING_FX_STRING_TESTHELPERS_H_
