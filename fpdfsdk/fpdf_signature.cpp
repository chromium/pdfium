// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_signature.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "fpdfsdk/cpdfsdk_helpers.h"

FPDF_EXPORT int FPDF_CALLCONV FPDF_GetSignatureCount(FPDF_DOCUMENT document) {
  auto* doc = CPDFDocumentFromFPDFDocument(document);
  if (!doc)
    return -1;

  CPDF_Dictionary* root = doc->GetRoot();
  if (!root)
    return 0;

  const CPDF_Dictionary* acro_form = root->GetDictFor("AcroForm");
  if (!acro_form)
    return 0;

  const CPDF_Array* fields = acro_form->GetArrayFor("Fields");
  if (!fields)
    return 0;

  int signature_count = 0;
  CPDF_ArrayLocker locker(fields);
  for (const auto& field : locker) {
    const CPDF_Dictionary* field_dict = field->GetDict();
    if (field_dict && field_dict->GetNameFor("FT") == "Sig")
      ++signature_count;
  }

  return signature_count;
}
