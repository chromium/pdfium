// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONSTANTS_ANNOTATION_COMMON_H_
#define CONSTANTS_ANNOTATION_COMMON_H_

namespace pdfium {
namespace annotation {

// PDF 1.7 spec, table 8.15.
// Entries common to all annotation dictionaries.

constexpr char kType[] = "Type";
constexpr char kSubtype[] = "Subtype";
constexpr char kRect[] = "Rect";
constexpr char kContents[] = "Contents";
constexpr char kP[] = "P";
constexpr char kNM[] = "NM";
constexpr char kM[] = "M";
constexpr char kF[] = "F";
constexpr char kAP[] = "AP";
constexpr char kAS[] = "AS";
constexpr char kBorder[] = "Border";
constexpr char kC[] = "C";
constexpr char kStructParent[] = "StructParent";
constexpr char kOC[] = "OC";

}  // namespace annotation
}  // namespace pdfium

#endif  // CONSTANTS_ANNOTATION_COMMON_H_
