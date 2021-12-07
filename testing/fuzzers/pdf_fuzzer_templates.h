// Copyright 2021 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// File for holding strings representing PDF templates that are used by fuzzers.

#ifndef TESTING_FUZZERS_PDF_FUZZER_TEMPLATES_H_
#define TESTING_FUZZERS_PDF_FUZZER_TEMPLATES_H_

constexpr char kSimplePdfTemplate[] = R"(%PDF-1.7
1 0 obj
<</Type /Catalog /Pages 2 0 R /AcroForm <</XFA 30 0 R>> /NeedsRendering true>>
endobj
2 0 obj
<</Type /Pages /Kids [3 0 R] /Count 1>>
endobj
3 0 obj
<</Type /Page /Parent 2 0 R /MediaBox [0 0 3 3]>>
endobj
30 0 obj
<</Length $1>>
stream
$2
endstream
endobj
trailer
<</Root 1 0 R /Size 31>>
%%EOF)";

#endif  // TESTING_FUZZERS_PDF_FUZZER_TEMPLATES_H_
