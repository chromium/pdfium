// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONSTANTS_ANNOTATION_COMMON_H_
#define CONSTANTS_ANNOTATION_COMMON_H_

namespace pdfium {
namespace annotation {

// PDF 1.7 spec, table 8.15.
// Entries common to all annotation dictionaries.
inline constexpr char kType[] = "Type";
inline constexpr char kSubtype[] = "Subtype";
inline constexpr char kRect[] = "Rect";
inline constexpr char kContents[] = "Contents";
inline constexpr char kP[] = "P";
inline constexpr char kNM[] = "NM";
inline constexpr char kM[] = "M";
inline constexpr char kF[] = "F";
inline constexpr char kAP[] = "AP";
inline constexpr char kAS[] = "AS";
inline constexpr char kBorder[] = "Border";
inline constexpr char kC[] = "C";
inline constexpr char kStructParent[] = "StructParent";
inline constexpr char kOC[] = "OC";

// Entries for polygon and polyline annotations.
inline constexpr char kVertices[] = "Vertices";

// Entries for ink annotations
inline constexpr char kInkList[] = "InkList";

// Entries for line annotations
inline constexpr char kL[] = "L";

}  // namespace annotation
}  // namespace pdfium

#endif  // CONSTANTS_ANNOTATION_COMMON_H_
