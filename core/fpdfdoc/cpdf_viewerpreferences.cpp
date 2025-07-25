// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_viewerpreferences.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"

CPDF_ViewerPreferences::CPDF_ViewerPreferences(const CPDF_Document* doc)
    : doc_(doc) {}

CPDF_ViewerPreferences::~CPDF_ViewerPreferences() = default;

bool CPDF_ViewerPreferences::IsDirectionR2L() const {
  RetainPtr<const CPDF_Dictionary> dict = GetViewerPreferences();
  return dict && dict->GetByteStringFor("Direction") == "R2L";
}

bool CPDF_ViewerPreferences::PrintScaling() const {
  RetainPtr<const CPDF_Dictionary> dict = GetViewerPreferences();
  return !dict || dict->GetByteStringFor("PrintScaling") != "None";
}

int32_t CPDF_ViewerPreferences::NumCopies() const {
  RetainPtr<const CPDF_Dictionary> dict = GetViewerPreferences();
  return dict ? dict->GetIntegerFor("NumCopies") : 1;
}

RetainPtr<const CPDF_Array> CPDF_ViewerPreferences::PrintPageRange() const {
  RetainPtr<const CPDF_Dictionary> dict = GetViewerPreferences();
  return dict ? dict->GetArrayFor("PrintPageRange") : nullptr;
}

ByteString CPDF_ViewerPreferences::Duplex() const {
  RetainPtr<const CPDF_Dictionary> dict = GetViewerPreferences();
  return dict ? dict->GetByteStringFor("Duplex") : ByteString("None");
}

std::optional<ByteString> CPDF_ViewerPreferences::GenericName(
    ByteStringView key) const {
  RetainPtr<const CPDF_Dictionary> dict = GetViewerPreferences();
  if (!dict) {
    return std::nullopt;
  }

  RetainPtr<const CPDF_Name> pName = ToName(dict->GetObjectFor(key));
  if (!pName) {
    return std::nullopt;
  }

  return pName->GetString();
}

RetainPtr<const CPDF_Dictionary> CPDF_ViewerPreferences::GetViewerPreferences()
    const {
  const CPDF_Dictionary* dict = doc_->GetRoot();
  return dict ? dict->GetDictFor("ViewerPreferences") : nullptr;
}
