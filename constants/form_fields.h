// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONSTANTS_FORM_FIELDS_H_
#define CONSTANTS_FORM_FIELDS_H_

namespace pdfium {
namespace form_fields {

// PDF 1.7 spec, table 8.69.
// Entries common to all field dictionaries.
constexpr char kFT[] = "FT";
constexpr char kParent[] = "Parent";
constexpr char kKids[] = "Kids";
constexpr char kT[] = "T";
constexpr char kTU[] = "TU";
constexpr char kTM[] = "TM";
constexpr char kFf[] = "Ff";
constexpr char kV[] = "V";
constexpr char kDV[] = "DV";
constexpr char kAA[] = "AA";

// FT values from PDF 1.7 spec, table 8.69.
constexpr char kBtn[] = "Btn";
constexpr char kTx[] = "Tx";
constexpr char kCh[] = "Ch";
constexpr char kSig[] = "Sig";

}  // namespace form_fields
}  // namespace pdfium

#endif  // CONSTANTS_FORM_FIELDS_H_
