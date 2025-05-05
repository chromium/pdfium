// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONSTANTS_PAGE_OBJECT_H_
#define CONSTANTS_PAGE_OBJECT_H_

namespace pdfium {
namespace page_object {

// PDF 1.7 spec, table 3.27.
// Entries in a page object.
const char kType[] = "Type";
const char kParent[] = "Parent";
const char kResources[] = "Resources";
const char kMediaBox[] = "MediaBox";
const char kCropBox[] = "CropBox";
const char kBleedBox[] = "BleedBox";
const char kTrimBox[] = "TrimBox";
const char kArtBox[] = "ArtBox";
const char kContents[] = "Contents";
const char kRotate[] = "Rotate";

}  // namespace page_object
}  // namespace pdfium

#endif  // CONSTANTS_PAGE_OBJECT_H_
