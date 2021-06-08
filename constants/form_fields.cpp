// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "constants/form_fields.h"

namespace pdfium {
namespace form_fields {

// PDF 1.7 spec, table 8.69.
// Entries common to all field dictionaries.
const char kFT[] = "FT";
const char kParent[] = "Parent";
const char kKids[] = "Kids";
const char kT[] = "T";
const char kTU[] = "TU";
const char kTM[] = "TM";
const char kFf[] = "Ff";
const char kV[] = "V";
const char kDV[] = "DV";
const char kAA[] = "AA";

// FT values from PDF 1.7 spec, table 8.69.
const char kBtn[] = "Btn";
const char kTx[] = "Tx";
const char kCh[] = "Ch";
const char kSig[] = "Sig";

}  // namespace form_fields
}  // namespace pdfium
