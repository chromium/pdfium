// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef PUBLIC_FPDF_FLATTEN_H_
#define PUBLIC_FPDF_FLATTEN_H_

#include "fpdfview.h"

// Result codes.
#define FLATTEN_FAIL            0   // Flatten operation failed.
#define FLATTEN_SUCCESS         1   // Flatten operation succeed.
#define FLATTEN_NOTHINGTODO     2   // There is nothing to be flattened.

// Flags.
#define FLAT_NORMALDISPLAY     0
#define FLAT_PRINT             1

#ifdef __cplusplus
extern "C" {
#endif

// Function: FPDFPage_Flatten
//          Make annotations and form fields become part of the page contents itself.
// Parameters:
//          page  - Handle to the page, as returned by FPDF_LoadPage().
//          nFlag - Intended use of the flattened result: 0 for normal display, 1 for printing.
// Return value:
//          Either FLATTEN_FAIL, FLATTEN_SUCCESS, or FLATTEN_NOTHINGTODO (see above).
// Comments:
//          Currently, all failures return FLATTEN_FAIL, with no indication for the reason
//          for the failure.
DLLEXPORT int STDCALL FPDFPage_Flatten(FPDF_PAGE page, int nFlag);

#ifdef __cplusplus
}
#endif

#endif  // PUBLIC_FPDF_FLATTEN_H_
