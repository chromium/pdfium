// Copyright 2020 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_EMBEDDER_TEST_CONSTANTS_H_
#define TESTING_EMBEDDER_TEST_CONSTANTS_H_

namespace pdfium {

// MD5 hash for rendering annotation_stamp_with_ap.pdf with annotations.
extern const char kAnnotationStampWithApChecksum[];

// MD5 hash for rendering a 612x792 blank page.
extern const char kBlankPage612By792Checksum[];

// MD5 hash for rendering bug_890322.pdf.
extern const char kBug890322Checksum[];

// MD5 hash for rendering hello_world.pdf or bug_455199.pdf.
extern const char kHelloWorldChecksum[];

// MD5 hash for rendering hello_world.pdf after removing "Goodbye, world!".
extern const char kHelloWorldRemovedChecksum[];

// MD5 hash for rendering many_rectangles.pdf.
extern const char kManyRectanglesChecksum[];

// MD5 hash for rendering rectangles.pdf.
extern const char kRectanglesChecksum[];

// MD5 hash for rendering text_form.pdf.
extern const char kTextFormChecksum[];

}  // namespace pdfium

#endif  // TESTING_EMBEDDER_TEST_CONSTANTS_H_
