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
// Get the embedded attachment at |index| in |document|. Note that the returned
// attachment handle is only valid while |document| is open.
//
//   document - handle to a document.
//   index    - the index of the requested embedded file.
//
// Returns the handle to the attachment object, or NULL on failure.
DLLEXPORT FPDF_ATTACHMENT STDCALL FPDFDoc_GetAttachment(FPDF_DOCUMENT document,
                                                        int index);

// Experimental API.
// Get the name of the |attachment| file. |buffer| is only modified if |buflen|
// is longer than the length of the file name. On errors, |buffer| is unmodified
// and the returned length is 0.
//
//   attachment - handle to an attachment.
//   buffer     - buffer for holding the file name, encoded in UTF16-LE.
//   buflen     - length of the buffer.
//
// Returns the length of the file name.
DLLEXPORT unsigned long STDCALL
FPDFAttachment_GetName(FPDF_ATTACHMENT attachment,
                       void* buffer,
                       unsigned long buflen);

// Experimental API.
// Check if the params dictionary of |attachment| has |key| as a key.
//
//   attachment - handle to an attachment.
//   key        - the key to look for.
//
// Returns true if |key| exists.
DLLEXPORT FPDF_BOOL STDCALL FPDFAttachment_HasKey(FPDF_ATTACHMENT attachment,
                                                  FPDF_WIDESTRING key);

// Experimental API.
// Get the type of the value corresponding to |key| in the params dictionary of
// the embedded |attachment|.
//
//   attachment - handle to an attachment.
//   key        - the key to look for.
//
// Returns the type of the dictionary value.
DLLEXPORT FPDF_OBJECT_TYPE STDCALL
FPDFAttachment_GetValueType(FPDF_ATTACHMENT attachment, FPDF_WIDESTRING key);

// Experimental API.
// Get the string value corresponding to |key| in the params dictionary of the
// embedded file |attachment|. |buffer| is only modified if |buflen| is longer
// than the length of the string value. Note that if |key| does not exist in the
// dictionary or if |key|'s corresponding value in the dictionary is not a
// string (i.e. the value is not of type FPDF_OBJECT_STRING or
// FPDF_OBJECT_NAME), then an empty string would be copied to |buffer| and the
// return value would be 2. On other errors, nothing would be added to |buffer|
// and the return value would be 0.
//
//   attachment - handle to an attachment.
//   key        - the key to the requested string value.
//   buffer     - buffer for holding the file's date string encoded in UTF16-LE.
//   buflen     - length of the buffer.
//
// Returns the length of the dictionary value string.
DLLEXPORT unsigned long STDCALL
FPDFAttachment_GetStringValue(FPDF_ATTACHMENT attachment,
                              FPDF_WIDESTRING key,
                              void* buffer,
                              unsigned long buflen);

// Experimental API.
// Get the file data of |attachment|. |buffer| is only modified if |buflen| is
// longer than the length of the file. On errors, |buffer| is unmodified and the
// returned length is 0.
//
//   attachment - handle to an attachment.
//   buffer     - buffer for holding the file's data in raw bytes.
//   buflen     - length of the buffer.
//
// Returns the length of the file.
DLLEXPORT unsigned long STDCALL
FPDFAttachment_GetFile(FPDF_ATTACHMENT attachment,
                       void* buffer,
                       unsigned long buflen);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // PUBLIC_FPDF_ATTACHMENT_H_
