// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PUBLIC_FPDF_ATTACHMENT_H_
#define PUBLIC_FPDF_ATTACHMENT_H_

// NOLINTNEXTLINE(build/include)
#include "fpdfview.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Experimental API.
// Get the number of embedded files in |document|.
//
//   document - handle to a document.
//
// Returns the number of embedded files in |document|.
DLLEXPORT int STDCALL FPDFDoc_GetAttachmentCount(FPDF_DOCUMENT document);

// Experimental API.
// Get the name of the embedded file at |index| in |document|. |buffer| is
// only modified if |buflen| is longer than the length of the file name. On
// errors, |buffer| is unmodified and the returned length is 0.
//
//   document - handle to a document.
//   index    - the index of the requested embedded file.
//   buffer   - buffer for holding the file name, encoded in UTF16-LE.
//   buflen   - length of the buffer.
//
// Returns the length of the file name.
DLLEXPORT unsigned long STDCALL
FPDFDoc_GetAttachmentName(FPDF_DOCUMENT document,
                          int index,
                          void* buffer,
                          unsigned long buflen);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_ATTACHMENT_H_
