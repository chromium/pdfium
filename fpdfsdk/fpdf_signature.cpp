// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_signature.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "third_party/base/stl_util.h"

namespace {

std::vector<CPDF_Dictionary*> CollectSignatures(CPDF_Document* doc) {
  std::vector<CPDF_Dictionary*> signatures;
  CPDF_Dictionary* root = doc->GetRoot();
  if (!root)
    return signatures;

  const CPDF_Dictionary* acro_form = root->GetDictFor("AcroForm");
  if (!acro_form)
    return signatures;

  const CPDF_Array* fields = acro_form->GetArrayFor("Fields");
  if (!fields)
    return signatures;

  CPDF_ArrayLocker locker(fields);
  for (auto& field : locker) {
    CPDF_Dictionary* field_dict = field->GetDict();
    if (field_dict && field_dict->GetNameFor("FT") == "Sig")
      signatures.push_back(field_dict);
  }
  return signatures;
}

}  // namespace

FPDF_EXPORT int FPDF_CALLCONV FPDF_GetSignatureCount(FPDF_DOCUMENT document) {
  auto* doc = CPDFDocumentFromFPDFDocument(document);
  if (!doc)
    return -1;

  return pdfium::CollectionSize<int>(CollectSignatures(doc));
}

FPDF_EXPORT FPDF_SIGNATURE FPDF_CALLCONV
FPDF_GetSignatureObject(FPDF_DOCUMENT document, int index) {
  auto* doc = CPDFDocumentFromFPDFDocument(document);
  if (!doc)
    return nullptr;

  std::vector<CPDF_Dictionary*> signatures = CollectSignatures(doc);
  if (!pdfium::IndexInBounds(signatures, index))
    return nullptr;

  return FPDFSignatureFromCPDFDictionary(signatures[index]);
}
