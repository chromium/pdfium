// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PUBLIC_FPDF_SIGNATURE_H_
#define PUBLIC_FPDF_SIGNATURE_H_

// NOLINTNEXTLINE(build/include)
#include "fpdfview.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Experimental API.
// Function: FPDF_GetSignatureCount
//          Get total number of signatures in the document.
// Parameters:
//          document    -   Handle to document. Returned by FPDF_LoadDocument().
// Return value:
//          Total number of signatures in the document on success, -1 on error.
FPDF_EXPORT int FPDF_CALLCONV FPDF_GetSignatureCount(FPDF_DOCUMENT document);

// Experimental API.
// Function: FPDF_GetSignatureObject
//          Get the Nth signature of the document.
// Parameters:
//          document    -   Handle to document. Returned by FPDF_LoadDocument().
//          index       -   Index into the array of signatures of the document.
// Return value:
//          Returns the handle to the signature, or NULL on failure. The caller
//          does not take ownership of the returned FPDF_SIGNATURE. Instead, it
//          remains valid until FPDF_CloseDocument() is called for the document.
FPDF_EXPORT FPDF_SIGNATURE FPDF_CALLCONV
FPDF_GetSignatureObject(FPDF_DOCUMENT document, int index);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_SIGNATURE_H_
