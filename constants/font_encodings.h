// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONSTANTS_FONT_ENCODINGS_H_
#define CONSTANTS_FONT_ENCODINGS_H_

namespace pdfium {
namespace font_encodings {

// ISO 32000-1:2008 spec, table D1.
inline constexpr char kMacRomanEncoding[] = "MacRomanEncoding";
inline constexpr char kWinAnsiEncoding[] = "WinAnsiEncoding";
inline constexpr char kPDFDocEncoding[] = "PDFDocEncoding";
inline constexpr char kMacExpertEncoding[] = "MacExpertEncoding";

}  // namespace font_encodings
}  // namespace pdfium

#endif  // CONSTANTS_FONT_ENCODINGS_H_
