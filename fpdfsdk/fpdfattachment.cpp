// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_attachment.h"

#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfdoc/cpdf_filespec.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "fpdfsdk/fsdk_define.h"

DLLEXPORT int STDCALL FPDFDoc_GetAttachmentCount(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return 0;

  return CPDF_NameTree(pDoc, "EmbeddedFiles").GetCount();
}

DLLEXPORT unsigned long STDCALL
FPDFDoc_GetAttachmentName(FPDF_DOCUMENT document,
                          int index,
                          void* buffer,
                          unsigned long buflen) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc || index < 0)
    return 0;

  CPDF_NameTree nameTree(pDoc, "EmbeddedFiles");
  if (static_cast<size_t>(index) >= nameTree.GetCount())
    return 0;

  CFX_ByteString csName;
  CPDF_Object* pFile = nameTree.LookupValueAndName(index, &csName);
  if (!pFile)
    return 0;

  CFX_WideString name;
  CPDF_FileSpec filespec(pFile);
  filespec.GetFileName(&name);
  CFX_ByteString encodedName = name.UTF16LE_Encode();
  unsigned long len = encodedName.GetLength();
  if (buffer && buflen >= len)
    memcpy(buffer, encodedName.c_str(), len);

  return len;
}
