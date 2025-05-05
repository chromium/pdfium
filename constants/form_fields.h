// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONSTANTS_FORM_FIELDS_H_
#define CONSTANTS_FORM_FIELDS_H_

namespace pdfium {
namespace form_fields {

// ISO 32000-1:2008 table 220.
// Entries common to all field dictionaries.
inline constexpr char kFT[] = "FT";
inline constexpr char kParent[] = "Parent";
inline constexpr char kKids[] = "Kids";
inline constexpr char kT[] = "T";
inline constexpr char kTU[] = "TU";
inline constexpr char kTM[] = "TM";
inline constexpr char kFf[] = "Ff";
inline constexpr char kV[] = "V";
inline constexpr char kDV[] = "DV";
inline constexpr char kAA[] = "AA";

// ISO 32000-1:2008 table 220.
// Values for FT keyword.
inline constexpr char kBtn[] = "Btn";
inline constexpr char kTx[] = "Tx";
inline constexpr char kCh[] = "Ch";
inline constexpr char kSig[] = "Sig";

// ISO 32000-1:2008 table 222.
// Entries common to fields containing variable text.
inline constexpr char kDA[] = "DA";
inline constexpr char kQ[] = "Q";
inline constexpr char kDS[] = "DS";
inline constexpr char kRV[] = "RV";

}  // namespace form_fields
}  // namespace pdfium

#endif  // CONSTANTS_FORM_FIELDS_H_
