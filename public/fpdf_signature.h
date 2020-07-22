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

// Experimental API.
// Function: FPDFSignatureObj_GetContents
//          Get the contents of a signature object.
// Parameters:
//          signature   -   Handle to the signature object. Returned by
//                          FPDF_GetSignatureObject().
//          buffer      -   The address of a buffer that receives the contents.
//          length      -   The size, in bytes, of |buffer|.
// Return value:
//          Returns the number of bytes in the contents on success, 0 on error.
//
// For public-key signatures, |buffer| is either a DER-encoded PKCS#1 binary or
// a DER-encoded PKCS#7 binary. If |length| is less than the returned length, or
// |buffer| is NULL, |buffer| will not be modified.
FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFSignatureObj_GetContents(FPDF_SIGNATURE signature,
                             void* buffer,
                             unsigned long length);

// Experimental API.
// Function: FPDFSignatureObj_GetByteRange
//          Get the byte range of a signature object.
// Parameters:
//          signature   -   Handle to the signature object. Returned by
//                          FPDF_GetSignatureObject().
//          buffer      -   The address of a buffer that receives the
//                          byte range.
//          length      -   The size, in ints, of |buffer|.
// Return value:
//          Returns the number of ints in the byte range on
//          success, 0 on error.
//
// |buffer| is an array of pairs of integers (starting byte offset,
// length in bytes) that describes the exact byte range for the digest
// calculation. If |length| is less than the returned length, or
// |buffer| is NULL, |buffer| will not be modified.
FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFSignatureObj_GetByteRange(FPDF_SIGNATURE signature,
                              int* buffer,
                              unsigned long length);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_SIGNATURE_H_
