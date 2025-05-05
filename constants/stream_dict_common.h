// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONSTANTS_STREAM_DICT_COMMON_H_
#define CONSTANTS_STREAM_DICT_COMMON_H_

namespace pdfium {
namespace stream {

// PDF 1.7 spec, table 3.4.
// Entries common to all stream dictionaries.
//
// TODO(https://crbug.com/pdfium/1049): Examine all usages of "Length",
// "Filter", and "F".
inline constexpr char kLength[] = "Length";
inline constexpr char kFilter[] = "Filter";
inline constexpr char kDecodeParms[] = "DecodeParms";
inline constexpr char kF[] = "F";
inline constexpr char kDL[] = "DL";

}  // namespace stream
}  // namespace pdfium

#endif  // CONSTANTS_STREAM_DICT_COMMON_H_
